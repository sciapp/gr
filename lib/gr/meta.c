#ifdef __unix__
#define _XOPEN_SOURCE 600
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "gks.h"
#include "gr.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
/* allow the use of posix functions on windows with msvc */
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#define strdup _strdup
#define snprintf(buf, len, format, ...) _snprintf_s(buf, len, len, format, __VA_ARGS__)
#endif

/* ######################### private interface ###################################################################### */

/* ========================= macros ================================================================================= */

/* ------------------------- error handling ------------------------------------------------------------------------- */

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
#ifdef _WIN32
#define psocketerror(prefix_message)                                                                                 \
  do {                                                                                                               \
    char *message = NULL;                                                                                            \
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, \
                  WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message, 0, NULL);           \
    fprintf(stderr, "%s: %s", prefix_message, message);                                                              \
    LocalFree(message);                                                                                              \
  } while (0)
#else
#define psocketerror(prefix_message) perror(prefix_message)
#endif
#else
#define debug_print_error(error_message_arguments)
#define psocketerror(prefix_message)
#endif
#define debug_print_malloc_error()                                                                                    \
  do {                                                                                                                \
    if (isatty(fileno(stderr))) {                                                                                     \
      debug_print_error(("\033[96m%s\033[0m:\033[93m%d\033[0m: Memory allocation failed -> out of virtual memory.\n", \
                         __FILE__, __LINE__));                                                                        \
    } else {                                                                                                          \
      debug_print_error(("%s:%d: Memory allocation failed -> out of virtual memory.\n", __FILE__, __LINE__));         \
    }                                                                                                                 \
  } while (0)


/* ------------------------- dynamic args array --------------------------------------------------------------------- */

#define DYNAMIC_ARGS_ARRAY_INITIAL_SIZE 10
#define DYNAMIC_ARGS_ARRAY_SIZE_INCREMENT 10


/* ------------------------- general -------------------------------------------------------------------------------- */

#ifndef DBL_DECIMAL_DIG
#define DBL_DECIMAL_DIG 17
#endif

/* ------------------------- json deserializer ---------------------------------------------------------------------- */

#define NEXT_VALUE_TYPE_SIZE 80


/* ------------------------- logging -------------------------------------------------------------------------------- */

#ifndef NDEBUG
#ifdef _WIN32
#define logger(logger_arguments)                    \
  do {                                              \
    fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); \
    fprintf logger_arguments;                       \
  } while (0)
#else
#define logger(logger_arguments)                                                    \
  do {                                                                              \
    if (isatty(fileno(stderr))) {                                                   \
      fprintf(stderr, "\033[96m%s\033[0m:\033[93m%d\033[0m: ", __FILE__, __LINE__); \
    } else {                                                                        \
      fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);                               \
    }                                                                               \
    fprintf logger_arguments;                                                       \
  } while (0)
#endif
#else
#define logger(logger_arguments)
#endif


/* ------------------------- memwriter ------------------------------------------------------------------------------ */

#define MEMWRITER_INITIAL_SIZE 32768
#define MEMWRITER_SIZE_INCREMENT 32768

#define ETB '\027'


/* ------------------------- plot ----------------------------------------------------------------------------------- */

#define PLOT_DEFAULT_WIDTH 600.0
#define PLOT_DEFAULT_HEIGHT 450.0
#define PLOT_DEFAULT_KIND "line"
#define PLOT_DEFAULT_SPEC ""
#define PLOT_DEFAULT_CLEAR 1
#define PLOT_DEFAULT_UPDATE 1
#define PLOT_DEFAULT_LOCATION 1
#define PLOT_DEFAULT_SUBPLOT_MIN_X 0.0
#define PLOT_DEFAULT_SUBPLOT_MAX_X 1.0
#define PLOT_DEFAULT_SUBPLOT_MIN_Y 0.0
#define PLOT_DEFAULT_SUBPLOT_MAX_Y 1.0
#define PLOT_DEFAULT_XLOG 0
#define PLOT_DEFAULT_YLOG 0
#define PLOT_DEFAULT_ZLOG 0
#define PLOT_DEFAULT_XFLIP 0
#define PLOT_DEFAULT_YFLIP 0
#define PLOT_DEFAULT_ZFLIP 0
#define PLOT_DEFAULT_ADJUST_XLIM 1
#define PLOT_DEFAULT_ADJUST_YLIM 1
#define PLOT_DEFAULT_ADJUST_ZLIM 1
#define PLOT_DEFAULT_COLORMAP 44 /* VIRIDIS */
#define PLOT_DEFAULT_ROTATION 40
#define PLOT_DEFAULT_TILT 70
#define PLOT_DEFAULT_XLABEL ""
#define PLOT_DEFAULT_YLABEL ""
#define PLOT_DEFAULT_ZLABEL ""
#define PLOT_DEFAULT_STEP_WHERE "mid"
#define PLOT_DEFAULT_CONTOUR_LEVELS 20
#define PLOT_DEFAULT_HEXBIN_NBINS 40
#define PLOT_DEFAULT_TRICONT_LEVELS 20
#define SERIES_DEFAULT_SPEC ""
#define PLOT_POLAR_AXES_TEXT_BUFFER 40
#define PLOT_CONTOUR_GRIDIT_N 200
#define PLOT_WIREFRAME_GRIDIT_N 50
#define PLOT_SURFACE_GRIDIT_N 200


/* ------------------------- receiver / sender----------------------------------------------------------------------- */

#define SOCKET_RECV_BUF_SIZE (MEMWRITER_INITIAL_SIZE - 1)


/* ------------------------- sender --------------------------------------------------------------------------------- */

#define SENDMETA_REF_FORMAT_MAX_LENGTH 100
#define PORT_MAX_STRING_LENGTH 80


/* ------------------------- util ----------------------------------------------------------------------------------- */

#define is_string_delimiter(char_ptr, str) ((*(char_ptr) == '"') && (((char_ptr) == (str)) || *((char_ptr)-1) != '\\'))

#ifndef min
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef max
#define max(x, y) (((x) > (y)) ? (x) : (y))
#endif

/* test macros which can be used like `assert` */
#define return_error_if(condition, error_value) \
  do {                                          \
    if (condition) {                            \
      return (error_value);                     \
    }                                           \
  } while (0)
#define goto_if(condition, goto_label) \
  do {                                 \
    if (condition) {                   \
      goto goto_label;                 \
    }                                  \
  } while (0)
#define cleanup_if(condition) goto_if((condition), cleanup)
#define error_cleanup_if(condition) goto_if((condition), error_cleanup)
#define goto_and_set_error_if(condition, error_value, goto_label) \
  do {                                                            \
    if (condition) {                                              \
      error = (error_value);                                      \
      goto goto_label;                                            \
    }                                                             \
  } while (0)
#define cleanup_and_set_error_if(condition, error_value) goto_and_set_error_if((condition), (error_value), cleanup)
#define error_cleanup_and_set_error_if(condition, error_value) \
  goto_and_set_error_if((condition), (error_value), error_cleanup)

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
  int next_is_array;
  size_t default_array_length;
  ssize_t next_array_length;
  int dataslot_count;
};
typedef struct _argparse_state_t argparse_state_t;

typedef void (*read_param_t)(argparse_state_t *);
typedef void (*delete_value_t)(void *);


/* ------------------------- argument container --------------------------------------------------------------------- */

typedef struct _args_node_t {
  arg_t *arg;
  struct _args_node_t *next;
} args_node_t;

struct _gr_meta_args_t {
  args_node_t *kwargs_head;
  args_node_t *kwargs_tail;
  unsigned int count;
};
typedef gr_meta_args_t *gr_meta_args_ptr_t;


/* ------------------------- argument iterator ---------------------------------------------------------------------- */

typedef struct {
  const args_node_t *next_node;
  const args_node_t *end;
} args_iterator_private_t;

typedef struct _args_iterator_t {
  arg_t *(*next)(struct _args_iterator_t *);
  arg_t *arg;
  args_iterator_private_t *priv;
} args_iterator_t;


/* ------------------------- dynamic args array --------------------------------------------------------------------- */

typedef struct {
  gr_meta_args_t **buf;
  size_t size;
  size_t capacity;
} dynamic_args_array_t;


/* ------------------------- args / dynamic args array stack -------------------------------------------------------- */

#define DEFINE_STACK_DATATYPES(prefix, type) \
  typedef struct _##prefix##_stack_node_t {  \
    type value;                              \
    struct _##prefix##_stack_node_t *next;   \
  } prefix##_stack_node_t;                   \
                                             \
  typedef struct {                           \
    prefix##_stack_node_t *head;             \
  } prefix##_stack_t;

DEFINE_STACK_DATATYPES(args, gr_meta_args_t *)
DEFINE_STACK_DATATYPES(dynamic_args_array, dynamic_args_array_t *)
DEFINE_STACK_DATATYPES(string, const char *)

#undef DEFINE_STACK_DATATYPES


/* ------------------------- error handling ------------------------------------------------------------------------- */

typedef enum {
#ifndef _WIN32 /* Windows uses `NO_ERROR` (= 0) for its own error codes */
  NO_ERROR = 0,
#endif
  ERROR_UNSPECIFIED = 1,
  ERROR_MALLOC,
  ERROR_UNSUPPORTED_OPERATION,
  ERROR_UNSUPPORTED_DATATYPE,
  ERROR_PARSE_NULL,
  ERROR_PARSE_BOOL,
  ERROR_PARSE_INT,
  ERROR_PARSE_DOUBLE,
  ERROR_PARSE_STRING,
  ERROR_PARSE_ARRAY,
  ERROR_PARSE_OBJECT,
  ERROR_PARSE_UNKNOWN_DATATYPE,
  ERROR_PARSE_INVALID_DELIMITER,
  ERROR_PARSE_INCOMPLETE_STRING,
  ERROR_PARSE_MISSING_OBJECT_CONTAINER,
  ERROR_NETWORK_WINSOCK_INIT,
  ERROR_NETWORK_SOCKET_CREATION,
  ERROR_NETWORK_SOCKET_BIND,
  ERROR_NETWORK_SOCKET_LISTEN,
  ERROR_NETWORK_CONNECTION_ACCEPT,
  ERROR_NETWORK_HOSTNAME_RESOLUTION,
  ERROR_NETWORK_CONNECT,
  ERROR_NETWORK_RECV,
  ERROR_NETWORK_SEND,
  ERROR_NETWORK_SOCKET_CLOSE,
  ERROR_NETWORK_WINSOCK_CLEANUP,
  ERROR_CUSTOM_RECV,
  ERROR_CUSTOM_SEND,
  ERROR_PLOT_NORMALIZATION,
  ERROR_PLOT_UNKNOWN_KIND,
  ERROR_PLOT_MISSING_DATA,
  ERROR_PLOT_COMPONENT_LENGTH_MISMATCH,
  ERROR_PLOT_MISSING_LABELS,
  ERROR_NOT_IMPLEMENTED
} error_t;


/* ------------------------- value iterator ------------------------------------------------------------------------- */

typedef struct {
  void *value_buffer;
  const char *value_format;
} args_value_iterator_private_t;

typedef struct _gr_meta_args_value_iterator_t {
  void *(*next)(struct _gr_meta_args_value_iterator_t *);
  void *value_ptr;
  char format;
  int is_array;
  size_t array_length;
  args_value_iterator_private_t *priv;
} args_value_iterator_t;


/* ------------------------- memwriter ------------------------------------------------------------------------------ */

struct _memwriter_t {
  char *buf;
  size_t size;
  size_t capacity;
};
typedef struct _memwriter_t memwriter_t;


/* ------------------------- json deserializer ---------------------------------------------------------------------- */

typedef enum {
  JSON_DATATYPE_UNKNOWN,
  JSON_DATATYPE_NULL,
  JSON_DATATYPE_BOOL,
  JSON_DATATYPE_NUMBER,
  JSON_DATATYPE_STRING,
  JSON_DATATYPE_ARRAY,
  JSON_DATATYPE_OBJECT
} fromjson_datatype_t;

typedef struct {
  char *json_ptr;
  int parsed_any_value_before;
} fromjson_shared_state_t;

typedef struct {
  fromjson_datatype_t datatype;
  int parsing_object;
  void *value_buffer;
  int value_buffer_pointer_level;
  void *next_value_memory;
  char *next_value_type;
  gr_meta_args_t *args;
  fromjson_shared_state_t *shared_state;
} fromjson_state_t;


/* ------------------------- json serializer ------------------------------------------------------------------------ */

typedef error_t (*tojson_post_processing_callback_t)(memwriter_t *, unsigned int, const char *);

enum { member_name, data_type };

typedef enum {
  /* 0 is unknown / not set */
  complete = 1,
  incomplete,
  incomplete_at_struct_beginning
} tojson_serialization_result_t;

typedef struct {
  int apply_padding;
  size_t array_length;
  int read_length_from_string;
  const void *data_ptr;
  va_list *vl;
  int data_offset;
  int wrote_output;
  int add_data;
  tojson_serialization_result_t serial_result;
  unsigned int struct_nested_level;
} tojson_shared_state_t;

typedef struct {
  memwriter_t *memwriter;
  char *data_type_ptr;
  char current_data_type;
  char *additional_type_info;
  int is_type_info_incomplete;
  int add_data_without_separator;
  tojson_shared_state_t *shared;
} tojson_state_t;

typedef struct {
  tojson_serialization_result_t serial_result;
  unsigned int struct_nested_level;
} tojson_permanent_state_t;


/* ------------------------- receiver / sender ---------------------------------------------------------------------- */

struct _metahandle_t;
typedef struct _metahandle_t metahandle_t;

typedef error_t (*recv_callback_t)(void *);
typedef error_t (*send_callback_t)(void *);
typedef const char *(*custom_recv_callback_t)(const char *, unsigned int);
typedef int (*custom_send_callback_t)(const char *, unsigned int, const char *);
typedef error_t (*finalize_callback_t)(metahandle_t *);

struct _metahandle_t {
  int is_receiver;
  union {
    struct {
      memwriter_t *memwriter;
      size_t message_size;
      recv_callback_t recv;
      union {
        struct {
          custom_recv_callback_t recv;
          const char *name;
          unsigned int id;
        } custom;
        struct {
          int client_socket;
          int server_socket;
        } socket;
      } comm;
    } receiver;
    struct {
      memwriter_t *memwriter;
      send_callback_t send;
      union {
        struct {
          custom_send_callback_t send;
          const char *name;
          unsigned int id;
        } custom;
        struct {
          int client_socket;
          struct sockaddr_in server_address;
        } socket;
      } comm;
    } sender;
  } sender_receiver;
  finalize_callback_t finalize;
};


/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind to func ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

typedef error_t (*plot_func_t)(gr_meta_args_t *args);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

typedef enum {
  GR_OPTION_X_LOG = 1 << 0,
  GR_OPTION_Y_LOG = 1 << 1,
  GR_OPTION_Z_LOG = 1 << 2,
  GR_OPTION_FLIP_X = 1 << 3,
  GR_OPTION_FLIP_Y = 1 << 4,
  GR_OPTION_FLIP_Z = 1 << 5,
  GR_OPTION_LINES = 0,
  GR_OPTION_MESH = 1,
  GR_OPTION_FILLED_MESH = 2,
  GR_OPTION_Z_SHADED_MESH = 3,
  GR_OPTION_COLORED_MESH = 4,
  GR_OPTION_CELL_ARRAY = 5,
  GR_OPTION_SHADED_MESH = 6
} gr_option_t;


/* ------------------------- string-to-generic map ------------------------------------------------------------------ */

#define DECLARE_MAP_TYPE(prefix, value_type) \
  typedef struct {                           \
    const char *key;                         \
    value_type value;                        \
  } prefix##_map_entry_t;                    \
                                             \
  typedef struct {                           \
    prefix##_map_entry_t *map;               \
    size_t capacity;                         \
    size_t size;                             \
  } prefix##_map_t;

DECLARE_MAP_TYPE(fmt, const char *)
DECLARE_MAP_TYPE(plot_func, plot_func_t)

#undef DECLARE_MAP_TYPE


/* ========================= functions ============================================================================== */

/* ------------------------- argument parsing ----------------------------------------------------------------------- */

static void *argparse_read_params(const char *format, const void *buffer, va_list *vl, int apply_padding,
                                  char **new_format);
static void argparse_read_int(argparse_state_t *state);
static void argparse_read_double(argparse_state_t *state);
static void argparse_read_char(argparse_state_t *state);
static void argparse_read_string(argparse_state_t *state);
static void argparse_read_default_array_length(argparse_state_t *state);
static void argparse_read_char_array(argparse_state_t *state, int store_array_length);
static void argparse_init_static_variables(void);
static size_t argparse_calculate_needed_buffer_size(const char *format);
static size_t argparse_calculate_needed_padding(void *buffer, char current_format);
static void argparse_read_next_option(argparse_state_t *state, char **format);
static const char *argparse_skip_option(const char *format);
static char *argparse_convert_to_array(argparse_state_t *state);


/* ------------------------- argument container --------------------------------------------------------------------- */

static arg_t *args_create_args(const char *key, const char *value_format, const void *buffer, va_list *vl,
                               int apply_padding);
static int args_validate_format_string(const char *format);
static const char *args_skip_option(const char *format);
static void args_copy_format_string_for_arg(char *dst, const char *format);
static void args_copy_format_string_for_parsing(char *dst, const char *format);
static int args_check_format_compatibility(arg_t *arg, const char *compatible_format);
static void args_decrease_arg_reference_count(args_node_t *args_node);


/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ general ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static error_t plot_init_static_variables(void);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot arguments ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static error_t plot_merge_and_normalize_args(gr_meta_args_t *normalized_args, const gr_meta_args_t *user_args);
static void plot_set_plot_attribute_defaults(gr_meta_args_t *subplot_args);
static void plot_pre_plot(gr_meta_args_t *subplot_args);
static void plot_process_colormap(gr_meta_args_t *subplot_args);
static void plot_process_viewport(gr_meta_args_t *subplot_args);
static void plot_process_window(gr_meta_args_t *subplot_args);
static void plot_store_coordinate_ranges(gr_meta_args_t *subplot_args);
static void plot_post_plot(gr_meta_args_t *subplot_args);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plotting ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static error_t plot_line(gr_meta_args_t *subplot_args);
static error_t plot_step(gr_meta_args_t *subplot_args);
static error_t plot_scatter(gr_meta_args_t *subplot_args);
static error_t plot_quiver(gr_meta_args_t *subplot_args);
static error_t plot_stem(gr_meta_args_t *subplot_args);
static error_t plot_hist(gr_meta_args_t *subplot_args);
static error_t plot_contour(gr_meta_args_t *subplot_args);
static error_t plot_contourf(gr_meta_args_t *subplot_args);
static error_t plot_hexbin(gr_meta_args_t *subplot_args);
static error_t plot_heatmap(gr_meta_args_t *subplot_args);
static error_t plot_wireframe(gr_meta_args_t *subplot_args);
static error_t plot_surface(gr_meta_args_t *subplot_args);
static error_t plot_plot3(gr_meta_args_t *subplot_args);
static error_t plot_scatter3(gr_meta_args_t *subplot_args);
static error_t plot_imshow(gr_meta_args_t *subplot_args);
static error_t plot_isosurface(gr_meta_args_t *subplot_args);
static error_t plot_polar(gr_meta_args_t *subplot_args);
static error_t plot_trisurf(gr_meta_args_t *subplot_args);
static error_t plot_tricont(gr_meta_args_t *subplot_args);
static error_t plot_shade(gr_meta_args_t *subplot_args);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ auxiliary drawing functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static error_t plot_draw_axes(gr_meta_args_t *args, unsigned int pass);
static error_t plot_draw_polar_axes(gr_meta_args_t *args);
static error_t plot_draw_legend(gr_meta_args_t *args);
static error_t plot_draw_colorbar(gr_meta_args_t *args, double off, unsigned int colors);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static double find_max_step(unsigned int n, const double *x);
static const char *next_fmt_key(const char *fmt);


/* ------------------------- util ----------------------------------------------------------------------------------- */

static size_t djb2_hash(const char *str);
static int is_int_number(const char *str);
static unsigned int str_to_uint(const char *str, int *was_successful);
static int int_equals_any(int number, unsigned int n, ...);
static int str_equals_any(const char *str, unsigned int n, ...);
static int uppercase_count(const char *str);


/* ========================= methods ================================================================================ */

/* ------------------------- argument ------------------------------------------------------------------------------- */

static args_value_iterator_t *arg_value_iter(const arg_t *arg);


/* ------------------------- argument container --------------------------------------------------------------------- */

static void args_init(gr_meta_args_t *args);
static void args_finalize(gr_meta_args_t *args);

static gr_meta_args_t *args_flatcopy(const gr_meta_args_t *args);
static gr_meta_args_t *args_copy(const gr_meta_args_t *copy_args, const char **keys_copy_as_array);

static error_t args_push_common(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                                va_list *vl, int apply_padding);
static error_t args_push_vl(gr_meta_args_t *args, const char *key, const char *value_format, va_list *vl);
static error_t args_update_many(gr_meta_args_t *args, const gr_meta_args_t *update_args);
static error_t args_merge(gr_meta_args_t *args, const gr_meta_args_t *merge_args, const char **merge_keys);
static error_t args_setdefault_common(gr_meta_args_t *args, const char *key, const char *value_format,
                                      const void *buffer, va_list *vl, int apply_padding);
static error_t args_setdefault(gr_meta_args_t *args, const char *key, const char *value_format, ...);
static error_t args_setdefault_buf(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                                   int apply_padding);
static error_t args_setdefault_vl(gr_meta_args_t *args, const char *key, const char *value_format, va_list *vl);

static unsigned int args_count(const gr_meta_args_t *args);

static arg_t *args_at(const gr_meta_args_t *args, const char *keyword);
static int args_first_value(const gr_meta_args_t *args, const char *keyword, const char *first_value_format,
                            void *first_value, unsigned int *array_length);
#define args_first_value(args, keyword, first_value_format, first_value, array_length) \
  args_first_value(args, keyword, first_value_format, (void *)first_value, array_length)
static int args_values(const gr_meta_args_t *args, const char *keyword, const char *expected_format, ...);

static args_node_t *args_find_node(const gr_meta_args_t *args, const char *keyword);
static int args_find_previous_node(const gr_meta_args_t *args, const char *keyword, args_node_t **previous_node);

static args_iterator_t *args_iter(const gr_meta_args_t *args);


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


/* ------------------------- args / dynamic args array stack -------------------------------------------------------- */

#define DECLARE_STACK_METHODS(prefix, type)                                          \
  static prefix##_stack_t *prefix##_stack_new(void);                                 \
  static void prefix##_stack_delete(prefix##_stack_t *prefix##_stack);               \
  static void prefix##_stack_delete_with_elements(prefix##_stack_t *prefix##_stack); \
  static error_t prefix##_stack_push(prefix##_stack_t *prefix##_stack, type value);  \
  static type prefix##_stack_pop(prefix##_stack_t *prefix##_stack);                  \
  static int prefix##_stack_empty(prefix##_stack_t *prefix##_stack);

DECLARE_STACK_METHODS(args, gr_meta_args_t *)
DECLARE_STACK_METHODS(dynamic_args_array, dynamic_args_array_t *)
DECLARE_STACK_METHODS(string, const char *)

#undef DECLARE_STACK_METHODS


/* ------------------------- dynamic args array --------------------------------------------------------------------- */

static dynamic_args_array_t *dynamic_args_array_new(void);
static void dynamic_args_array_delete(dynamic_args_array_t *args_array);
static void dynamic_args_array_delete_with_elements(dynamic_args_array_t *args_array);
static error_t dynamic_args_array_push_back(dynamic_args_array_t *args_array, gr_meta_args_t *args);


/* ------------------------- json deserializer ---------------------------------------------------------------------- */

int gr_readmeta(gr_meta_args_t *args, const char *json_string);
static error_t fromjson_read(gr_meta_args_t *args, const char *json_string);

static error_t fromjson_parse(gr_meta_args_t *args, const char *json_string, fromjson_shared_state_t *shared_state);
static error_t fromjson_parse_null(fromjson_state_t *state);
static error_t fromjson_parse_bool(fromjson_state_t *state);
static error_t fromjson_parse_number(fromjson_state_t *state);
static error_t fromjson_parse_int(fromjson_state_t *state);
static error_t fromjson_parse_double(fromjson_state_t *state);
static error_t fromjson_parse_string(fromjson_state_t *state);
static error_t fromjson_parse_array(fromjson_state_t *state);
static error_t fromjson_parse_object(fromjson_state_t *state);

static fromjson_datatype_t fromjson_check_type(const fromjson_state_t *state);
static error_t fromjson_copy_and_filter_json_string(char **dest, const char *src);
static int fromjson_find_next_delimiter(const char **delim_ptr, const char *src, int include_start,
                                        int exclude_nested_structures);
static size_t fromjson_get_outer_array_length(const char *str);
static double fromjson_str_to_double(const char **str, int *was_successful);
static int fromjson_str_to_int(const char **str, int *was_successful);


/* ------------------------- json serializer ------------------------------------------------------------------------ */

#define DECLARE_STRINGIFY(name, type)                                    \
  static error_t tojson_stringify_##name(tojson_state_t *state);         \
  static error_t tojson_stringify_##name##_array(tojson_state_t *state); \
  static error_t tojson_stringify_##name##_value(memwriter_t *memwriter, type value);

static error_t tojson_read_array_length(tojson_state_t *state);
static error_t tojson_skip_bytes(tojson_state_t *state);
DECLARE_STRINGIFY(int, int)
DECLARE_STRINGIFY(double, double)
DECLARE_STRINGIFY(char, char)
DECLARE_STRINGIFY(string, char *)
DECLARE_STRINGIFY(bool, int)
static error_t tojson_stringify_object(tojson_state_t *state);
DECLARE_STRINGIFY(args, gr_meta_args_t *)
static error_t tojson_close_object(tojson_state_t *state);

#undef DECLARE_STRINGIFY_SINGLE
#undef DECLARE_STRINGIFY_MULTI

static int tojson_get_member_count(const char *data_desc);
static int tojson_is_json_array_needed(const char *data_desc);
static void tojson_read_datatype(tojson_state_t *state);
static error_t tojson_unzip_membernames_and_datatypes(char *mixed_ptr, char ***member_name_ptr, char ***data_type_ptr);
static error_t tojson_escape_special_chars(char **escaped_string, const char *unescaped_string, unsigned int *length);
static error_t tojson_serialize(memwriter_t *memwriter, char *data_desc, const void *data, va_list *vl,
                                int apply_padding, int add_data, int add_data_without_separator,
                                unsigned int *struct_nested_level, tojson_serialization_result_t *serial_result,
                                tojson_shared_state_t *shared_state);
static void tojson_init_static_variables(void);
static error_t tojson_init_variables(int *add_data, int *add_data_without_separator, char **_data_desc,
                                     const char *data_desc);
static error_t tojson_write_vl(memwriter_t *memwriter, const char *data_desc, va_list *vl);
static error_t tojson_write_buf(memwriter_t *memwriter, const char *data_desc, const void *buffer, int apply_padding);
static error_t tojson_write_arg(memwriter_t *memwriter, const arg_t *arg);
static error_t tojson_write_args(memwriter_t *memwriter, const gr_meta_args_t *args);
static int tojson_is_complete(void);
static int tojson_struct_nested_level(void);


/* ------------------------- memwriter ------------------------------------------------------------------------------ */

static memwriter_t *memwriter_new(void);
static void memwriter_delete(memwriter_t *memwriter);
static void memwriter_clear(memwriter_t *memwriter);
static error_t memwriter_replace(memwriter_t *memwriter, int index, int count, const char *replacement_str);
static error_t memwriter_erase(memwriter_t *memwriter, int index, int count);
static error_t memwriter_insert(memwriter_t *memwriter, int index, const char *str);
static error_t memwriter_enlarge_buf(memwriter_t *memwriter, size_t size_increment);
static error_t memwriter_ensure_buf(memwriter_t *memwriter, size_t needed_additional_size);
static error_t memwriter_printf(memwriter_t *memwriter, const char *format, ...);
static error_t memwriter_puts(memwriter_t *memwriter, const char *s);
static error_t memwriter_putc(memwriter_t *memwriter, char c);
static char *memwriter_buf(const memwriter_t *memwriter);
static size_t memwriter_size(const memwriter_t *memwriter);


/* ------------------------- receiver ------------------------------------------------------------------------------- */

static error_t receiver_init_for_socket(metahandle_t *handle, const char *hostname, unsigned int port);
static error_t receiver_init_for_custom(metahandle_t *handle, const char *name, unsigned int id,
                                        const char *(*custom_recv)(const char *, unsigned int));
static error_t receiver_finalize_for_socket(metahandle_t *handle);
static error_t receiver_finalize_for_custom(metahandle_t *handle);
static error_t receiver_recv_for_socket(metahandle_t *handle);
static error_t receiver_recv_for_custom(metahandle_t *handle);


/* ------------------------- sender --------------------------------------------------------------------------------- */

static error_t sender_init_for_socket(metahandle_t *handle, const char *hostname, unsigned int port);
static error_t sender_init_for_custom(metahandle_t *handle, const char *name, unsigned int id,
                                      int (*custom_send)(const char *, unsigned int, const char *));
static error_t sender_finalize_for_socket(metahandle_t *handle);
static error_t sender_finalize_for_custom(metahandle_t *handle);
static error_t sender_send_for_socket(metahandle_t *handle);
static error_t sender_send_for_custom(metahandle_t *handle);


/* ------------------------- string-to-generic map ------------------------------------------------------------------ */

#define DECLARE_MAP_METHODS(prefix, value_type)                                                    \
  static prefix##_map_t *prefix##_map_new(size_t capacity);                                        \
  static prefix##_map_t *prefix##_map_new_with_data(size_t count, prefix##_map_entry_t *entries);  \
  static void prefix##_map_delete(prefix##_map_t *prefix##_map);                                   \
  static int prefix##_map_insert(prefix##_map_t *prefix##_map, const char *key, value_type value); \
  static value_type prefix##_map_at(prefix##_map_t *prefix##_map, const char *key);                \
  static ssize_t prefix##_map_index(prefix##_map_t *prefix##_map, const char *key);

DECLARE_MAP_METHODS(fmt, const char *)
DECLARE_MAP_METHODS(plot_func, plot_func_t)

#undef DECLARE_MAP_METHODS


/* ========================= static variables ======================================================================= */

/* ------------------------- argument parsing ----------------------------------------------------------------------- */

static int argparse_valid_format_specifier[128];
static read_param_t argparse_format_specifier_to_read_callback[128];
static delete_value_t argparse_format_specifier_to_delete_callback[128];
static size_t argparse_format_specifier_to_size[128];
static int argparse_static_variables_initialized = 0;


/* ------------------------- argument container --------------------------------------------------------------------- */

static const char *const ARGS_VALID_FORMAT_SPECIFIERS = "niIdDcCsSaA";
static const char *const ARGS_VALID_DATA_FORMAT_SPECIFIERS = "idcsa"; /* Each specifier is also valid in upper case */


/* ------------------------- json deserializer ---------------------------------------------------------------------- */

static const char FROMJSON_VALID_DELIMITERS[] = ",]}";

static error_t (*fromjson_datatype_to_func[])(fromjson_state_t *) = {NULL,
                                                                     fromjson_parse_null,
                                                                     fromjson_parse_bool,
                                                                     fromjson_parse_number,
                                                                     fromjson_parse_string,
                                                                     fromjson_parse_array,
                                                                     fromjson_parse_object};

static const char *const fromjson_datatype_to_string[] = {"unknown", "null",  "bool",  "number",
                                                          "string",  "array", "object"};


/* ------------------------- json serializer ------------------------------------------------------------------------ */

static error_t (*tojson_datatype_to_func[128])(tojson_state_t *);
static int tojson_static_variables_initialized = 0;
static tojson_permanent_state_t tojson_permanent_state = {complete, 0};


/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ general ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static int plot_static_variables_initialized = 0;


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ args ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static gr_meta_args_t *subplots_args = NULL;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind to fmt ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* TODO: Check format of: "heatmap", "hist", "isosurface", "imshow"  */
static fmt_map_entry_t kind_to_fmt[] = {
  {"line", "xys"},     {"hexbin", "xys"},     {"polar", "xys"},     {"shade", "xys"},    {"stem", "xys"},
  {"step", "xys"},     {"contour", "xyzc"},   {"contourf", "xyzc"}, {"tricont", "xyzc"}, {"trisurf", "xyzc"},
  {"surface", "xyzc"}, {"wireframe", "xyzc"}, {"plot3", "xyac"},    {"scatter", "xyac"}, {"scatter3", "xyac"},
  {"quiver", "xyuv"},  {"heatmap", "x"},      {"hist", "x"},        {"isosurface", "x"}, {"imshow", ""}};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind to func ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static plot_func_map_entry_t kind_to_func[] = {
  {"line", plot_line},     {"step", plot_step},         {"scatter", plot_scatter},     {"quiver", plot_quiver},
  {"stem", plot_stem},     {"hist", plot_hist},         {"contour", plot_contour},     {"contourf", plot_contourf},
  {"hexbin", plot_hexbin}, {"heatmap", plot_heatmap},   {"wireframe", plot_wireframe}, {"surface", plot_surface},
  {"plot3", plot_plot3},   {"scatter3", plot_scatter3}, {"imshow", plot_imshow},       {"isosurface", plot_isosurface},
  {"polar", plot_polar},   {"trisurf", plot_trisurf},   {"tricont", plot_tricont},     {"shade", plot_shade},
};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ maps ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static fmt_map_t *fmt_map = NULL;
static plot_func_map_t *plot_func_map = NULL;


/* ######################### public implementation ################################################################## */

/* ========================= methods ================================================================================ */

/* ------------------------- argument container --------------------------------------------------------------------- */

gr_meta_args_t *gr_newmeta() {
  gr_meta_args_t *args = malloc(sizeof(gr_meta_args_t));
  if (args == NULL) {
    debug_print_malloc_error();
    return NULL;
  }
  args_init(args);
  return args;
}

void gr_deletemeta(gr_meta_args_t *args) {
  args_finalize(args);
  free(args);
}

void gr_finalizemeta(void) {
  if (plot_static_variables_initialized) {
    gr_deletemeta(subplots_args);
    subplots_args = NULL;
    fmt_map_delete(fmt_map);
    fmt_map = NULL;
    plot_func_map_delete(plot_func_map);
    plot_func_map = NULL;
    plot_static_variables_initialized = 0;
  }
}

int gr_meta_args_push(gr_meta_args_t *args, const char *key, const char *value_format, ...) {
  va_list vl;
  error_t error;

  va_start(vl, value_format);

  error = args_push_vl(args, key, value_format, &vl);

  va_end(vl);

  return error == NO_ERROR;
}

int gr_meta_args_push_buf(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                          int apply_padding) {
  error_t error;

  error = args_push_common(args, key, value_format, buffer, NULL, apply_padding);

  return error == NO_ERROR;
}

int gr_meta_args_contains(const gr_meta_args_t *args, const char *keyword) {
  return args_at(args, keyword) != NULL;
}

void gr_meta_args_clear(gr_meta_args_t *args) {
  args_node_t *current_node, *next_node;

  current_node = args->kwargs_head;
  while (current_node != NULL) {
    next_node = current_node->next;
    args_decrease_arg_reference_count(current_node);
    free(current_node);
    current_node = next_node;
  }
  args_init(args);
}

void gr_meta_args_remove(gr_meta_args_t *args, const char *key) {
  args_node_t *tmp_node, *previous_node_by_keyword;

  if (args_find_previous_node(args, key, &previous_node_by_keyword)) {
    if (previous_node_by_keyword == NULL) {
      tmp_node = args->kwargs_head->next;
      args_decrease_arg_reference_count(args->kwargs_head);
      free(args->kwargs_head);
      args->kwargs_head = tmp_node;
      if (tmp_node == NULL) {
        args->kwargs_tail = NULL;
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
    --(args->count);
  }
}


/* ------------------------- plot ----------------------------------------------------------------------------------- */

int gr_plotmeta(const gr_meta_args_t *args) {
  gr_meta_args_t **current_subplot_args;
  plot_func_t plot_func;
  const char *kind = NULL;

  if (plot_init_static_variables() != NO_ERROR) {
    return 0;
  }
  if (plot_merge_and_normalize_args(subplots_args, args) != NO_ERROR) {
    return 0;
  }
  args_first_value(subplots_args, "subplots", "A", &current_subplot_args, NULL);
  while (*current_subplot_args != NULL) {
    plot_set_plot_attribute_defaults(*current_subplot_args);
    plot_pre_plot(*current_subplot_args);
    args_first_value(*current_subplot_args, "kind", "s", &kind, NULL);
    logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));
    if ((plot_func = plot_func_map_at(plot_func_map, kind)) == NULL) {
      return 0;
    }
    if (plot_func(*current_subplot_args) != NO_ERROR) {
      return 0;
    };
    plot_post_plot(*current_subplot_args);
    ++current_subplot_args;
  }

  return 1;
}


/* ------------------------- receiver / sender ---------------------------------------------------------------------- */

void *gr_openmeta(int is_receiver, const char *name, unsigned int id,
                  const char *(*custom_recv)(const char *, unsigned int),
                  int (*custom_send)(const char *, unsigned int, const char *)) {
  metahandle_t *handle;
  error_t error = NO_ERROR;

  handle = malloc(sizeof(metahandle_t));
  if (handle == NULL) {
    return NULL;
  }
  handle->is_receiver = is_receiver;
  handle->sender_receiver.receiver.comm.custom.recv = custom_recv;
  if (is_receiver) {
    if (custom_recv != NULL) {
      error = receiver_init_for_custom(handle, name, id, custom_recv);
    } else {
      error = receiver_init_for_socket(handle, name, id);
    }
  } else {
    if (custom_send != NULL) {
      error = sender_init_for_custom(handle, name, id, custom_send);
    } else {
      error = sender_init_for_socket(handle, name, id);
    }
  }

  if (error != NO_ERROR) {
    if (error != ERROR_NETWORK_WINSOCK_INIT) {
      handle->finalize(handle);
    }
    free(handle);
    handle = NULL;
  }

  return (void *)handle;
}

void gr_closemeta(const void *p) {
  metahandle_t *handle = (metahandle_t *)p;

  handle->finalize(handle);
  free(handle);
}


/* ------------------------- receiver ------------------------------------------------------------------------------- */

gr_meta_args_t *gr_recvmeta(const void *p, gr_meta_args_t *args) {
  metahandle_t *handle = (metahandle_t *)p;
  int created_args = 0;

  if (args == NULL) {
    args = gr_newmeta();
    if (args == NULL) {
      goto error_cleanup;
    }
    created_args = 1;
  }

  if (handle->sender_receiver.receiver.recv(handle) != NO_ERROR) {
    goto error_cleanup;
  }
  if (fromjson_read(args, memwriter_buf(handle->sender_receiver.receiver.memwriter)) != NO_ERROR) {
    goto error_cleanup;
  }

  if (memwriter_erase(handle->sender_receiver.receiver.memwriter, 0, handle->sender_receiver.receiver.message_size) !=
      NO_ERROR) {
    goto error_cleanup;
  }

  return args;

error_cleanup:
  if (created_args) {
    gr_deletemeta(args);
  }

  return NULL;
}


/* ------------------------- sender --------------------------------------------------------------------------------- */

int gr_sendmeta(const void *p, const char *data_desc, ...) {
  metahandle_t *handle = (metahandle_t *)p;
  va_list vl;
  error_t error;

  va_start(vl, data_desc);
  error = tojson_write_vl(handle->sender_receiver.sender.memwriter, data_desc, &vl);
  if (error == NO_ERROR && tojson_is_complete() && handle->sender_receiver.sender.send != NULL) {
    error = handle->sender_receiver.sender.send(handle);
  }
  va_end(vl);

  return error == NO_ERROR;
}

int gr_sendmeta_buf(const void *p, const char *data_desc, const void *buffer, int apply_padding) {
  metahandle_t *handle = (metahandle_t *)p;
  error_t error;

  error = tojson_write_buf(handle->sender_receiver.sender.memwriter, data_desc, buffer, apply_padding);
  if (error == NO_ERROR && tojson_is_complete() && handle->sender_receiver.sender.send != NULL) {
    error = handle->sender_receiver.sender.send(handle);
  }

  return error == NO_ERROR;
}

int gr_sendmeta_ref(const void *p, const char *key, char format, const void *ref, int len) {
  static const char VALID_OPENING_BRACKETS[] = "([{";
  static const char VALID_CLOSING_BRACKETS[] = ")]}";
  static const char VALID_SEPARATOR[] = ",";
  static gr_meta_args_t *current_args = NULL;
  static dynamic_args_array_t *current_args_array = NULL;
  const char *_key = NULL;
  metahandle_t *handle = (metahandle_t *)p;
  char format_string[SENDMETA_REF_FORMAT_MAX_LENGTH];
  error_t error = NO_ERROR;

  if (tojson_struct_nested_level() == 0) {
    gr_sendmeta(handle, "o(");
  }
  if (strchr("soO", format) == NULL) {
    /* handle general cases (values and arrays) */
    if (islower(format)) {
      if (current_args_array == NULL) {
        snprintf(format_string, SENDMETA_REF_FORMAT_MAX_LENGTH, "%s:%c,", key, format);
        error = gr_sendmeta_buf(handle, format_string, ref, 1);
      } else {
        snprintf(format_string, SENDMETA_REF_FORMAT_MAX_LENGTH, "%c", format);
        /* TODO: add error return value to `gr_meta_args_push_arg` (?) */
        gr_meta_args_push_buf(current_args, key, format_string, ref, 1);
      }
    } else {
      if (current_args_array == NULL) {
        snprintf(format_string, SENDMETA_REF_FORMAT_MAX_LENGTH, "%s:n%c,", key, format);
        error = gr_sendmeta(handle, format_string, len, ref);
      } else {
        snprintf(format_string, SENDMETA_REF_FORMAT_MAX_LENGTH, "n%c", format);
        gr_meta_args_push(current_args, key, format_string, len, ref);
      }
    }
  } else {
    static args_stack_t *args_stack = NULL;
    static dynamic_args_array_stack_t *args_array_stack = NULL;
    static string_stack_t *key_stack = NULL;
    /* handle special cases (strings, objects and arrays of objects) */
    switch (format) {
    case 's':
      if (current_args_array == NULL) {
        snprintf(format_string, SENDMETA_REF_FORMAT_MAX_LENGTH, "%s:s,", key);
        error = gr_sendmeta(handle, format_string, ref);
      } else {
        gr_meta_args_push(current_args, key, "s", ref);
      }
      break;
    case 'o':
      if (strchr(VALID_OPENING_BRACKETS, *(const char *)ref)) {
        if (current_args_array == NULL) {
          snprintf(format_string, SENDMETA_REF_FORMAT_MAX_LENGTH, "%s:o(,", key);
          gr_sendmeta(handle, format_string);
        } else {
          if ((_key = strdup(key)) == NULL) {
            error = ERROR_MALLOC;
            break;
          }
          if (args_stack == NULL) {
            args_stack = args_stack_new();
            if (args_stack == NULL) {
              error = ERROR_MALLOC;
              break;
            }
          }
          if (key_stack == NULL) {
            key_stack = string_stack_new();
            if (key_stack == NULL) {
              error = ERROR_MALLOC;
              break;
            }
          }
          if ((error = args_stack_push(args_stack, current_args)) != NO_ERROR) {
            break;
          }
          if ((error = string_stack_push(key_stack, _key)) != NO_ERROR) {
            break;
          }
          _key = NULL; /* avoid deletion at the end of this function */
          current_args = gr_newmeta();
          if (current_args == NULL) {
            error = ERROR_MALLOC;
            break;
          }
        }
      } else if (strchr(VALID_CLOSING_BRACKETS, *(const char *)ref)) {
        if (current_args_array == NULL) {
          gr_sendmeta(handle, ")");
        } else {
          gr_meta_args_t *previous_args = args_stack_pop(args_stack);
          _key = string_stack_pop(key_stack);
          gr_meta_args_push(previous_args, _key, "a", current_args);
          current_args = previous_args;
          if (args_stack_empty(args_stack)) {
            args_stack_delete(args_stack);
            args_stack = NULL;
          }
          if (string_stack_empty(key_stack)) {
            string_stack_delete(key_stack);
            key_stack = NULL;
          }
        }
      }
      break;
    case 'O':
      if (strchr(VALID_OPENING_BRACKETS, *(const char *)ref)) {
        if ((_key = strdup(key)) == NULL) {
          error = ERROR_MALLOC;
          break;
        }
        if (current_args_array != NULL) {
          if (args_array_stack == NULL) {
            args_array_stack = dynamic_args_array_stack_new();
            if (args_array_stack == NULL) {
              error = ERROR_MALLOC;
              break;
            }
          }
          if ((error = dynamic_args_array_stack_push(args_array_stack, current_args_array)) != NO_ERROR) {
            break;
          }
        }
        if (current_args != NULL) {
          if (args_stack == NULL) {
            args_stack = args_stack_new();
            if (args_stack == NULL) {
              error = ERROR_MALLOC;
              break;
            }
          }
          if ((error = args_stack_push(args_stack, current_args)) != NO_ERROR) {
            break;
          }
        }
        if (key_stack == NULL) {
          key_stack = string_stack_new();
          if (key_stack == NULL) {
            error = ERROR_MALLOC;
            break;
          }
        }
        if ((error = string_stack_push(key_stack, _key)) != NO_ERROR) {
          break;
        }
        _key = NULL;
        current_args_array = dynamic_args_array_new();
        if (current_args_array == NULL) {
          error = ERROR_MALLOC;
          break;
        }
        current_args = gr_newmeta();
        if (current_args == NULL) {
          error = ERROR_MALLOC;
          break;
        }
        if ((error = dynamic_args_array_push_back(current_args_array, current_args)) != NO_ERROR) {
          break;
        }
      } else if (strchr(VALID_SEPARATOR, *(const char *)ref)) {
        current_args = gr_newmeta();
        if (current_args == NULL) {
          error = ERROR_MALLOC;
          break;
        }
        assert(current_args_array != NULL);
        if ((error = dynamic_args_array_push_back(current_args_array, current_args)) != NO_ERROR) {
          break;
        }
      } else if (strchr(VALID_CLOSING_BRACKETS, *(const char *)ref)) {
        assert(key_stack != NULL);
        _key = string_stack_pop(key_stack);
        if (args_array_stack != NULL) {
          current_args = args_stack_pop(args_stack);
          gr_meta_args_push(current_args, _key, "nA", current_args_array->size, current_args_array->buf);
          dynamic_args_array_delete(current_args_array);
          current_args_array = dynamic_args_array_stack_pop(args_array_stack);
          if (dynamic_args_array_stack_empty(args_array_stack)) {
            dynamic_args_array_stack_delete(args_array_stack);
            args_array_stack = NULL;
          }
        } else {
          snprintf(format_string, SENDMETA_REF_FORMAT_MAX_LENGTH, "%s:nA,", _key);
          gr_sendmeta(handle, format_string, current_args_array->size, current_args_array->buf);
          dynamic_args_array_delete_with_elements(current_args_array);
          current_args_array = NULL;
          current_args = NULL;
        }
        if (string_stack_empty(key_stack)) {
          string_stack_delete(key_stack);
          key_stack = NULL;
        }
      }
      break;
    case '\0':
      gr_sendmeta(handle, ")");
      break;
    default:
      break;
    }
  }

  free((void *)_key);

  return error == NO_ERROR;
}

int gr_sendmeta_args(const void *p, const gr_meta_args_t *args) {
  metahandle_t *handle = (metahandle_t *)p;
  error_t error;

  error = tojson_write_args(handle->sender_receiver.sender.memwriter, args);
  if (error == NO_ERROR && tojson_is_complete() && handle->sender_receiver.sender.send != NULL) {
    error = handle->sender_receiver.sender.send(handle);
  }

  return error == NO_ERROR;
}


/* ######################### private implementation ################################################################# */

/* ========================= functions ============================================================================== */

/* ------------------------- argument parsing ----------------------------------------------------------------------- */

void *argparse_read_params(const char *format, const void *buffer, va_list *vl, int apply_padding, char **new_format) {
  char *fmt, *current_format, first_format_char;
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
  if (needed_buffer_size > 0) {
    save_buffer = malloc(needed_buffer_size);
    if (save_buffer == NULL) {
      debug_print_malloc_error();
      free(fmt);
      return NULL;
    }
  } else {
    save_buffer = NULL;
  }

  /* initialize state object */
  state.vl = vl;
  state.in_buffer = buffer;
  state.apply_padding = apply_padding;
  state.data_offset = 0;
  state.save_buffer = save_buffer;
  state.next_is_array = 0;
  state.default_array_length = 1;
  state.next_array_length = -1;
  state.dataslot_count = 0;

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
    state.next_array_length = -1;
    if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, tolower(*current_format)) != NULL) {
      ++state.dataslot_count;
      if (state.dataslot_count == 1) {
        first_format_char = *current_format;
      }
    }
    ++current_format;
  }

  /* Reset the save buffer since it was shifted during the parsing process */
  state.save_buffer = save_buffer;
  if (state.dataslot_count > 1 && islower(first_format_char) && new_format != NULL) {
    *new_format = argparse_convert_to_array(&state);
  }

  /* cleanup */
  free(fmt);

  return state.save_buffer;
}

#define CHECK_PADDING(type)                                           \
  do {                                                                \
    if (state->in_buffer != NULL && state->apply_padding) {           \
      ptrdiff_t needed_padding = state->data_offset % sizeof(type);   \
      state->in_buffer = ((char *)state->in_buffer) + needed_padding; \
      state->data_offset += needed_padding;                           \
    }                                                                 \
  } while (0)

#define READ_TYPE(type, terminate_array)                                                                               \
  void argparse_read_##type(argparse_state_t *state) {                                                                 \
    size_t *size_t_typed_buffer;                                                                                       \
    type *typed_buffer, **pointer_typed_buffer, *src_ptr;                                                              \
    size_t current_array_length;                                                                                       \
                                                                                                                       \
    if (state->next_is_array) {                                                                                        \
      current_array_length = (state->next_array_length >= 0) ? state->next_array_length : state->default_array_length; \
      size_t_typed_buffer = state->save_buffer;                                                                        \
      *size_t_typed_buffer = current_array_length;                                                                     \
      pointer_typed_buffer = (type **)++size_t_typed_buffer;                                                           \
      if (current_array_length + (terminate_array ? 1 : 0) > 0) {                                                      \
        *pointer_typed_buffer = malloc((current_array_length + (terminate_array ? 1 : 0)) * sizeof(type));             \
      } else {                                                                                                         \
        *pointer_typed_buffer = NULL;                                                                                  \
      }                                                                                                                \
      if (current_array_length > 0) {                                                                                  \
        if (state->in_buffer != NULL) {                                                                                \
          CHECK_PADDING(type **);                                                                                      \
          src_ptr = *(type **)state->in_buffer;                                                                        \
        } else {                                                                                                       \
          src_ptr = va_arg(*state->vl, type *);                                                                        \
        }                                                                                                              \
        if (*pointer_typed_buffer != NULL) {                                                                           \
          memcpy(*pointer_typed_buffer, src_ptr, current_array_length * sizeof(type));                                 \
          if (terminate_array) {                                                                                       \
            /* cast to `type ***` instead of `type **` to ensure that `= NULL` is always a syntactical                 \
             * valid statement (it wouldn't for primary types like `int` and `double`);                                \
             * for non-pointer types the following statement is never executed, so it should be fine                   \
             */                                                                                                        \
            (*(type ***)pointer_typed_buffer)[current_array_length] = NULL; /* array terminator */                     \
          }                                                                                                            \
        } else {                                                                                                       \
          debug_print_malloc_error();                                                                                  \
        }                                                                                                              \
        if (state->in_buffer != NULL) {                                                                                \
          state->in_buffer = ((type **)state->in_buffer) + 1;                                                          \
          state->data_offset += sizeof(type *);                                                                        \
        }                                                                                                              \
        state->save_buffer = ++pointer_typed_buffer;                                                                   \
      }                                                                                                                \
    } else {                                                                                                           \
      typed_buffer = state->save_buffer;                                                                               \
      if (state->in_buffer != NULL) {                                                                                  \
        CHECK_PADDING(type);                                                                                           \
        *typed_buffer = *((type *)state->in_buffer);                                                                   \
        state->in_buffer = ((type *)state->in_buffer) + 1;                                                             \
        state->data_offset += sizeof(type);                                                                            \
      } else {                                                                                                         \
        *typed_buffer = va_arg(*state->vl, type);                                                                      \
      }                                                                                                                \
      state->save_buffer = ++typed_buffer;                                                                             \
    }                                                                                                                  \
  }

READ_TYPE(int, 0)
READ_TYPE(double, 0)
READ_TYPE(gr_meta_args_ptr_t, 1)

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

    current_array_length = (state->next_array_length >= 0) ? state->next_array_length : state->default_array_length;
    if (state->in_buffer != NULL) {
      CHECK_PADDING(char **);
      src_ptr = *(const char ***)state->in_buffer;
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
    const size_t *typed_buffer = state->in_buffer;
    CHECK_PADDING(size_t);
    state->default_array_length = *typed_buffer;
    state->in_buffer = typed_buffer + 1;
    state->data_offset += sizeof(size_t);
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
    src_ptr = *(char **)state->in_buffer;
  } else {
    src_ptr = va_arg(*state->vl, char *);
  }
  current_array_length = (state->next_array_length >= 0) ? state->next_array_length : (int)strlen(src_ptr);
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
    argparse_valid_format_specifier['a'] = 1;
    argparse_valid_format_specifier['A'] = 1;

    argparse_format_specifier_to_read_callback['i'] = argparse_read_int;
    argparse_format_specifier_to_read_callback['d'] = argparse_read_double;
    argparse_format_specifier_to_read_callback['c'] = argparse_read_char;
    argparse_format_specifier_to_read_callback['s'] = argparse_read_string;
    argparse_format_specifier_to_read_callback['a'] = argparse_read_gr_meta_args_ptr_t;
    argparse_format_specifier_to_read_callback['n'] = argparse_read_default_array_length;

    argparse_format_specifier_to_delete_callback['s'] = free;
    argparse_format_specifier_to_delete_callback['a'] = (delete_value_t)gr_deletemeta;

    argparse_format_specifier_to_size['i'] = sizeof(int);
    argparse_format_specifier_to_size['I'] = sizeof(int *);
    argparse_format_specifier_to_size['d'] = sizeof(double);
    argparse_format_specifier_to_size['D'] = sizeof(double *);
    argparse_format_specifier_to_size['c'] = sizeof(char);
    argparse_format_specifier_to_size['C'] = sizeof(char *);
    argparse_format_specifier_to_size['s'] = sizeof(char *);
    argparse_format_specifier_to_size['S'] = sizeof(char **);
    argparse_format_specifier_to_size['a'] = sizeof(gr_meta_args_t *);
    argparse_format_specifier_to_size['A'] = sizeof(gr_meta_args_t **);
    argparse_format_specifier_to_size['n'] = 0; /* size for array length is reserved by an array call itself */
    argparse_format_specifier_to_size['#'] = sizeof(size_t); /* only used internally */

    argparse_static_variables_initialized = 1;
  }
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

char *argparse_convert_to_array(argparse_state_t *state) {
  void *new_save_buffer = NULL;
  size_t *size_t_typed_buffer;
  void **general_typed_buffer;
  char *new_format = NULL;

  new_save_buffer = malloc(sizeof(size_t) + sizeof(void *));
  if (new_save_buffer == NULL) {
    goto cleanup;
  }
  size_t_typed_buffer = new_save_buffer;
  *size_t_typed_buffer = state->dataslot_count;
  general_typed_buffer = (void **)(size_t_typed_buffer + 1);
  *general_typed_buffer = state->save_buffer;
  state->save_buffer = new_save_buffer;
  new_format = malloc(2 * sizeof(char));
  new_format[0] = toupper(state->current_format);
  new_format[1] = '\0';
  if (new_format == NULL) {
    goto cleanup;
  }
  return new_format;

cleanup:
  free(new_save_buffer);
  free(new_format);
  debug_print_malloc_error();
  return NULL;
}


/* ------------------------- argument container --------------------------------------------------------------------- */

arg_t *args_create_args(const char *key, const char *value_format, const void *buffer, va_list *vl, int apply_padding) {
  arg_t *arg;
  char *parsing_format;
  char *new_format = NULL;

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
  arg->value_format = malloc(2 * strlen(value_format) + 1);
  if (arg->value_format == NULL) {
    debug_print_malloc_error();
    free((char *)arg->key);
    free(arg);
    return NULL;
  }
  parsing_format = malloc(strlen(value_format) + 1);
  if (parsing_format == NULL) {
    debug_print_malloc_error();
    free((char *)arg->key);
    free((char *)arg->value_format);
    free(arg);
    return NULL;
  }
  args_copy_format_string_for_parsing(parsing_format, value_format);
  arg->value_ptr = argparse_read_params(parsing_format, buffer, vl, apply_padding, &new_format);
  if (new_format == NULL) {
    args_copy_format_string_for_arg((char *)arg->value_format, value_format);
  } else {
    args_copy_format_string_for_arg((char *)arg->value_format, new_format);
    free(new_format);
  }
  free(parsing_format);
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
  char *first_format_char;
  char *previous_char;
  char *current_char;
  char *option_start;
  int is_valid;

  if (format == NULL) {
    return 0;
  }
  fmt = strdup(format);
  if (fmt == NULL) {
    debug_print_malloc_error();
    return 0;
  }

  first_format_char = NULL;
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
                ("The option \"%s\" in the format string \"%s\" in no valid number.\n", option_start, format));
            }
          } else {
            is_valid = 0;
            --current_char;
            debug_print_error(("Option \"%s\" in the format string \"%s\" is not terminated.\n", option_start, format));
          }
        } else {
          is_valid = 0;
          debug_print_error(
            ("Specifier '%c' in the format string \"%s\" cannot have any options.\n", *previous_char, format));
        }
      } else {
        is_valid = 0;
        debug_print_error(
          ("The format string \"%s\" is invalid: Format strings must not start with an option.\n", format));
      }
    } else {
      if (strchr(ARGS_VALID_FORMAT_SPECIFIERS, *current_char) == NULL) {
        is_valid = 0;
        debug_print_error(("Invalid specifier '%c' in the format string \"%s\".\n", *current_char, format));
      } else if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, *current_char) != NULL) {
        if (first_format_char != NULL && *current_char != *first_format_char) {
          is_valid = 0;
          debug_print_error(("The format string \"%s\" consists of different types which is not allowed.\n", format));
        }
        if (first_format_char == NULL) {
          first_format_char = current_char;
        }
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

void args_copy_format_string_for_parsing(char *dst, const char *format) {
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

void args_copy_format_string_for_arg(char *dst, const char *format) {
  /* `dst` should have twice as much memory as `format` to ensure that no buffer overun can occur */
  while (*format) {
    if (*format == 'n') {
      /* Skip `n` since it is added for arrays anyway when needed */
      ++format;
      continue;
    }
    if (isupper(*format)) {
      /* all array formats get an internal size value */
      *dst++ = 'n';
    }
    if (*format == 'C') {
      /* char arrays and strings are the same -> store them as strings for unified data handling */
      *dst++ = 's';
      ++format;
    } else {
      *dst++ = *format++;
    }
    /* Skip an optional array length since it already saved in the argument buffer itself (-> `n` format) */
    format = args_skip_option(format);
  }
  *dst = '\0';
}

int args_check_format_compatibility(arg_t *arg, const char *compatible_format) {
  char first_compatible_format_char, first_value_format_char;
  const char *current_format_ptr;
  char *compatible_format_for_arg;
  int dataslot_count;

  /* First, check if the compatible format itself is valid (-> known format, homogeneous, no options) */
  first_compatible_format_char = *compatible_format;
  if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, tolower(first_compatible_format_char)) == NULL) {
    return 0;
  }
  current_format_ptr = compatible_format;
  while (*current_format_ptr != '\0') {
    if (*current_format_ptr != first_compatible_format_char) {
      return 0;
    }
    ++current_format_ptr;
  }

  /* Second, check if original and compatible format are identical */
  /* within an argument, formats are stored **with** array length slots, so we need `nD` instead of `D` for example
   * -> convert the format before comparison */
  compatible_format_for_arg = malloc(2 * strlen(compatible_format) + 1);
  if (compatible_format_for_arg == NULL) {
    debug_print_malloc_error();
    return 0;
  }
  args_copy_format_string_for_arg(compatible_format_for_arg, compatible_format);
  if (strcmp(arg->value_format, compatible_format_for_arg) == 0) {
    free(compatible_format_for_arg);
    return 2;
  }
  free(compatible_format_for_arg);

  /* Otherwise, check if the format is compatible */
  /* Compatibility is only possible for single array types -> the original format string (ignoring `n`!) must not be
   * longer than 1 */
  dataslot_count = 0;
  current_format_ptr = arg->value_format;
  while (*current_format_ptr != '\0' && dataslot_count < 2) {
    if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, tolower(*current_format_ptr)) != NULL) {
      ++dataslot_count;
      if (dataslot_count == 1) {
        first_value_format_char = *current_format_ptr;
      }
    }
    ++current_format_ptr;
  }
  if (dataslot_count > 1) {
    return 0;
  }
  /* Check if the single format character is an uppercase varsion of the given compatible format */
  if (first_value_format_char != toupper(first_compatible_format_char)) {
    return 0;
  }

  return 1;
}

void args_decrease_arg_reference_count(args_node_t *args_node) {
  if (--(args_node->arg->priv->reference_count) == 0) {
    args_value_iterator_t *value_it = arg_value_iter(args_node->arg);
    while (value_it->next(value_it) != NULL) {
      /* use a char pointer since chars have no memory alignment restrictions */
      if (value_it->is_array) {
        if (argparse_format_specifier_to_delete_callback[(int)value_it->format] != NULL) {
          char **current_value_ptr = *(char ***)value_it->value_ptr;
          while (*current_value_ptr != NULL) {
            argparse_format_specifier_to_delete_callback[(int)value_it->format](*current_value_ptr);
            /* cast to (char *) to allow pointer increment by bytes */
            current_value_ptr =
              (char **)((char *)current_value_ptr + argparse_format_specifier_to_size[(int)value_it->format]);
          }
        }
        free(*(char ***)value_it->value_ptr);
      } else if (argparse_format_specifier_to_delete_callback[(int)value_it->format] != NULL) {
        argparse_format_specifier_to_delete_callback[(int)value_it->format](*(char **)value_it->value_ptr);
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


/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ general ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

error_t plot_init_static_variables(void) {
  if (!plot_static_variables_initialized) {
    logger((stderr, "Initializing static plot variables\n"));
    subplots_args = gr_newmeta();
    if (subplots_args == NULL) {
      debug_print_malloc_error();
      goto error_cleanup;
    }
    fmt_map = fmt_map_new_with_data(sizeof(kind_to_fmt) / sizeof(kind_to_fmt[0]), kind_to_fmt);
    if (fmt_map == NULL) {
      debug_print_malloc_error();
      goto error_cleanup;
    }
    plot_func_map = plot_func_map_new_with_data(sizeof(kind_to_func) / sizeof(kind_to_func[0]), kind_to_func);
    if (plot_func_map == NULL) {
      debug_print_malloc_error();
      goto error_cleanup;
    }
    plot_static_variables_initialized = 1;
  }
  return NO_ERROR;

error_cleanup:
  if (subplots_args != NULL) {
    gr_deletemeta(subplots_args);
    subplots_args = NULL;
  }
  if (fmt_map != NULL) {
    fmt_map_delete(fmt_map);
    fmt_map = NULL;
  }
  if (plot_func_map != NULL) {
    plot_func_map_delete(plot_func_map);
    plot_func_map = NULL;
  }
  return ERROR_MALLOC;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot arguments ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

error_t plot_merge_and_normalize_args(gr_meta_args_t *normalized_args, const gr_meta_args_t *user_args) {
  gr_meta_args_t *copied_args = NULL, **subplot_args_array = NULL;
  const char *keys_copy_as_array[] = {"subplots", "series", NULL};
  const char *merge_keys[] = {"subplots", NULL};
  error_t error = NO_ERROR;

  logger((stderr, "Normalize plot args\n"));

  copied_args = args_copy(user_args, keys_copy_as_array);
  return_error_if(copied_args == NULL, ERROR_MALLOC);
  if ((gr_meta_args_contains(user_args, "subplots"))) {
    error = args_merge(normalized_args, copied_args, merge_keys);
  } else if ((gr_meta_args_contains(user_args, "series"))) {
    if ((args_values(normalized_args, "subplots", "A", &subplot_args_array))) {
      error = args_merge(*subplot_args_array, copied_args, merge_keys);
    } else {
      error = gr_meta_args_push(normalized_args, "subplots", "A(1)", &copied_args) ? NO_ERROR : ERROR_MALLOC;
      cleanup_if(error != NO_ERROR);
    }
    copied_args = NULL;
  } else {
    gr_meta_args_t *subplot_args = gr_newmeta();
    cleanup_and_set_error_if(subplot_args == NULL, ERROR_MALLOC);
    error = gr_meta_args_push(subplot_args, "series", "A(1)", &copied_args) ? NO_ERROR : ERROR_MALLOC;
    cleanup_if(error != NO_ERROR);
    error = gr_meta_args_push(normalized_args, "subplots", "A(1)", &subplot_args) ? NO_ERROR : ERROR_MALLOC;
    cleanup_if(error != NO_ERROR);
    copied_args = NULL;
  }

cleanup:
  if (copied_args != NULL) {
    gr_deletemeta(copied_args);
  }

  return error;
}

void plot_set_plot_attribute_defaults(gr_meta_args_t *subplot_args) {
  const char *kind;
  gr_meta_args_t **current_series;

  logger((stderr, "Set plot attribute defaults\n"));

  args_setdefault(subplot_args, "kind", "s", PLOT_DEFAULT_KIND);
  args_first_value(subplot_args, "kind", "s", &kind, NULL);
  if (!gr_meta_args_contains(subplot_args, "figsize")) {
    args_setdefault(subplot_args, "size", "dd", PLOT_DEFAULT_WIDTH, PLOT_DEFAULT_HEIGHT);
  }
  args_setdefault(subplot_args, "clear", "i", PLOT_DEFAULT_CLEAR);
  args_setdefault(subplot_args, "update", "i", PLOT_DEFAULT_UPDATE);
  if (gr_meta_args_contains(subplot_args, "labels")) {
    args_setdefault(subplot_args, "location", "i", PLOT_DEFAULT_LOCATION);
  }
  args_setdefault(subplot_args, "subplot", "dddd", PLOT_DEFAULT_SUBPLOT_MIN_X, PLOT_DEFAULT_SUBPLOT_MAX_X,
                  PLOT_DEFAULT_SUBPLOT_MIN_Y, PLOT_DEFAULT_SUBPLOT_MAX_Y);
  args_setdefault(subplot_args, "xlog", "i", PLOT_DEFAULT_XLOG);
  args_setdefault(subplot_args, "ylog", "i", PLOT_DEFAULT_YLOG);
  args_setdefault(subplot_args, "zlog", "i", PLOT_DEFAULT_ZLOG);
  args_setdefault(subplot_args, "xflip", "i", PLOT_DEFAULT_XFLIP);
  args_setdefault(subplot_args, "yflip", "i", PLOT_DEFAULT_YFLIP);
  args_setdefault(subplot_args, "zflip", "i", PLOT_DEFAULT_ZFLIP);
  args_setdefault(subplot_args, "adjust_xlim", "i", PLOT_DEFAULT_ADJUST_XLIM);
  args_setdefault(subplot_args, "adjust_ylim", "i", PLOT_DEFAULT_ADJUST_YLIM);
  args_setdefault(subplot_args, "adjust_zlim", "i", PLOT_DEFAULT_ADJUST_ZLIM);
  args_setdefault(subplot_args, "colormap", "i", PLOT_DEFAULT_COLORMAP);
  args_setdefault(subplot_args, "rotation", "i", PLOT_DEFAULT_ROTATION);
  args_setdefault(subplot_args, "tilt", "i", PLOT_DEFAULT_TILT);

  if (strcmp(kind, "step") == 0) {
    args_setdefault(subplot_args, "step_where", "s", PLOT_DEFAULT_STEP_WHERE);
  } else if (str_equals_any(kind, 2, "contour", "contourf")) {
    args_setdefault(subplot_args, "levels", "i", PLOT_DEFAULT_CONTOUR_LEVELS);
  } else if (strcmp(kind, "hexbin") == 0) {
    args_setdefault(subplot_args, "nbins", "i", PLOT_DEFAULT_HEXBIN_NBINS);
  } else if (strcmp(kind, "tricont") == 0) {
    args_setdefault(subplot_args, "levels", "i", PLOT_DEFAULT_TRICONT_LEVELS);
  }

  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    args_setdefault(*current_series, "spec", "s", SERIES_DEFAULT_SPEC);
    ++current_series;
  }
}

void plot_pre_plot(gr_meta_args_t *subplot_args) {
  int clear;
  const char *kind;
  double alpha;

  logger((stderr, "Pre plot processing\n"));

  args_first_value(subplot_args, "clear", "i", &clear, NULL);
  logger((stderr, "Got keyword \"clear\" with value %d\n", clear));
  if (clear) {
    gr_clearws();
  }
  args_first_value(subplot_args, "kind", "s", &kind, NULL);
  logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));
  if (str_equals_any(kind, 2, "imshow", "isosurface")) {
    plot_process_viewport(subplot_args);
  } else {
    plot_process_viewport(subplot_args);
    plot_store_coordinate_ranges(subplot_args);
    plot_process_window(subplot_args);
    if (str_equals_any(kind, 1, "polar")) {
      plot_draw_polar_axes(subplot_args);
    } else {
      plot_draw_axes(subplot_args, 1);
    }
  }

  plot_process_colormap(subplot_args);
  gr_uselinespec(" ");

  gr_savestate();
  if (args_first_value(subplot_args, "alpha", "d", &alpha, NULL)) {
    gr_settransparency(alpha);
  }
}

void plot_process_colormap(gr_meta_args_t *subplot_args) {
  int colormap;

  if (args_first_value(subplot_args, "colormap", "i", &colormap, NULL)) {
    gr_setcolormap(colormap);
  }
  /* TODO: Implement other datatypes for `colormap` */
}

void plot_process_viewport(gr_meta_args_t *subplot_args) {
  const char *kind;
  double metric_width, metric_height;
  int pixel_width, pixel_height;
  double size[2];
  double width, height;
  const double *subplot;
  double viewport[4] = {0.0, 0.0, 0.0, 0.0};
  double wsviewport[4] = {0.0, 0.0, 0.0, 0.0};
  double wswindow[4] = {0.0, 0.0, 0.0, 0.0};
  double vp[4];
  double aspect_ratio;
  double metric_size;
  int background_color_index;

  args_first_value(subplot_args, "kind", "s", &kind, NULL);
  args_first_value(subplot_args, "subplot", "D", &subplot, NULL);
#ifdef __EMSCRIPTEN__
  metric_width = 0.16384;
  metric_height = 0.12288;
  pixel_width = 640;
  pixel_height = 480;
#else
  gr_inqdspsize(&metric_width, &metric_height, &pixel_width, &pixel_height);
#endif
  if (args_values(subplot_args, "figsize", "dd", &size[0], &size[1])) {
    size[0] *= pixel_width * 0.0254 / metric_width;
    size[1] *= pixel_height * 0.0254 / metric_height;
  } else if (args_values(subplot_args, "size", "dd", &size[0], &size[1])) {
    double dpi = pixel_width / metric_width * 0.0254;
    if (dpi > 200) {
      int i;
      for (i = 0; i < 2; ++i) {
        size[i] *= dpi / 100.0;
      }
    }
  } else {
    size[0] = 480;
    size[1] = 480;
  }
  width = size[0];
  height = size[1];
  logger((stderr, "Using size: %lf, %lf\n", width, height));

  memcpy(vp, subplot, sizeof(vp));
  if (width > height) {
    aspect_ratio = height / width;
    metric_size = metric_width * width / pixel_width;
    wsviewport[1] = metric_size;
    wsviewport[3] = metric_size * aspect_ratio;
    wswindow[1] = 1.0;
    wswindow[3] = aspect_ratio;
    vp[2] *= aspect_ratio;
    vp[3] *= aspect_ratio;
  } else {
    aspect_ratio = width / height;
    metric_size = metric_height * height / pixel_height;
    wsviewport[1] = metric_size * aspect_ratio;
    wsviewport[3] = metric_size;
    wswindow[1] = aspect_ratio;
    wswindow[3] = 1.0;
    vp[0] *= aspect_ratio;
    vp[1] *= aspect_ratio;
  }

  if (str_equals_any(kind, 5, "wireframe", "surface", "plot3", "scatter3", "trisurf")) {
    double tmp_vp[4];
    double extent;

    if (str_equals_any(kind, 2, "surface", "trisurf")) {
      extent = min(vp[1] - vp[0] - 0.1, vp[3] - vp[2]);
    } else {
      extent = min(vp[1] - vp[0], vp[3] - vp[2]);
    }
    tmp_vp[0] = 0.5 * (vp[0] + vp[1] - extent);
    tmp_vp[1] = 0.5 * (vp[0] + vp[1] + extent);
    tmp_vp[2] = 0.5 * (vp[2] + vp[3] - extent);
    tmp_vp[3] = 0.5 * (vp[2] + vp[3] + extent);
    memcpy(vp, tmp_vp, 4 * sizeof(double));
  }

  viewport[0] = vp[0] + 0.125 * (vp[1] - vp[0]);
  viewport[1] = vp[0] + 0.925 * (vp[1] - vp[0]);
  viewport[2] = vp[2] + 0.125 * (vp[3] - vp[2]);
  viewport[3] = vp[2] + 0.925 * (vp[3] - vp[2]);

  if (width > height) {
    viewport[2] += (1 - (subplot[3] - subplot[2]) * (subplot[3] - subplot[2])) * 0.02;
  }
  if (str_equals_any(kind, 5, "contour", "contourf", "heatmap", "hexbin", "quiver")) {
    viewport[1] -= 0.1;
  }

  if (args_first_value(subplot_args, "backgroundcolor", "i", &background_color_index, NULL)) {
    gr_savestate();
    gr_selntran(0);
    gr_setfillintstyle(GKS_K_INTSTYLE_SOLID);
    gr_setfillcolorind(background_color_index);
    if (width > height) {
      gr_fillrect(subplot[0], subplot[1], subplot[2] * aspect_ratio, subplot[3] * aspect_ratio);
    } else {
      gr_fillrect(subplot[0] * aspect_ratio, subplot[1] * aspect_ratio, subplot[2], subplot[3]);
    }
    gr_selntran(1);
    gr_restorestate();
  }

  if (strcmp(kind, "polar") == 0) {
    double x_center, y_center, r;

    x_center = 0.5 * (viewport[0] + viewport[1]);
    y_center = 0.5 * (viewport[2] + viewport[3]);
    r = 0.5 * min(viewport[1] - viewport[0], viewport[3] - viewport[2]);
    viewport[0] = x_center - r;
    viewport[1] = x_center + r;
    viewport[2] = y_center - r;
    viewport[3] = y_center + r;
  }

  gr_setviewport(viewport[0], viewport[1], viewport[2], viewport[3]);
  gr_setwsviewport(wsviewport[0], wsviewport[1], wsviewport[2], wsviewport[3]);
  gr_setwswindow(wswindow[0], wswindow[1], wswindow[2], wswindow[3]);

  gr_meta_args_push(subplot_args, "viewport", "dddd", viewport[0], viewport[1], viewport[2], viewport[3]);
  logger((stderr, "Stored viewport (%f, %f, %f, %f)\n", viewport[0], viewport[1], viewport[2], viewport[3]));
  gr_meta_args_push(subplot_args, "vp", "dddd", vp[0], vp[1], vp[2], vp[3]);
  gr_meta_args_push(subplot_args, "ratio", "d", aspect_ratio);
}

void plot_process_window(gr_meta_args_t *subplot_args) {
  int scale = 0;
  const char *kind;
  int xlog, ylog, zlog;
  int xflip, yflip, zflip;
  int major_count, x_major_count, y_major_count;
  double x_min, x_max, y_min, y_max, z_min, z_max;
  double x, y, zoom;
  int adjust_xlim, adjust_ylim, adjust_zlim;
  double x_tick, y_tick;
  double x_org_low, x_org_high, y_org_low, y_org_high;
  int reset_ranges = 0;

  args_first_value(subplot_args, "kind", "s", &kind, NULL);
  args_first_value(subplot_args, "xlog", "i", &xlog, NULL);
  args_first_value(subplot_args, "ylog", "i", &ylog, NULL);
  args_first_value(subplot_args, "zlog", "i", &zlog, NULL);
  args_first_value(subplot_args, "xflip", "i", &xflip, NULL);
  args_first_value(subplot_args, "yflip", "i", &yflip, NULL);
  args_first_value(subplot_args, "zflip", "i", &zflip, NULL);

  if (strcmp(kind, "polar") != 0) {
    scale |= xlog ? GR_OPTION_X_LOG : 0;
    scale |= ylog ? GR_OPTION_Y_LOG : 0;
    scale |= zlog ? GR_OPTION_Z_LOG : 0;
    scale |= xflip ? GR_OPTION_FLIP_X : 0;
    scale |= yflip ? GR_OPTION_FLIP_Y : 0;
    scale |= zflip ? GR_OPTION_FLIP_Z : 0;
  }

  args_values(subplot_args, "xrange", "dd", &x_min, &x_max);
  args_values(subplot_args, "yrange", "dd", &y_min, &y_max);
  if (args_values(subplot_args, "reset_ranges", "i", &reset_ranges) && reset_ranges) {
    if (args_values(subplot_args, "original_xrange", "dd", &x_min, &x_max) &&
        args_values(subplot_args, "original_yrange", "dd", &y_min, &y_max) &&
        args_values(subplot_args, "original_adjust_xlim", "i", &adjust_xlim) &&
        args_values(subplot_args, "original_adjust_ylim", "i", &adjust_ylim)) {
      gr_meta_args_push(subplot_args, "xrange", "dd", x_min, x_max);
      gr_meta_args_push(subplot_args, "yrange", "dd", y_min, y_max);
      gr_meta_args_push(subplot_args, "adjust_xlim", "i", adjust_xlim);
      gr_meta_args_push(subplot_args, "adjust_ylim", "i", adjust_ylim);
      gr_meta_args_remove(subplot_args, "original_xrange");
      gr_meta_args_remove(subplot_args, "original_yrange");
      gr_meta_args_remove(subplot_args, "original_adjust_xlim");
      gr_meta_args_remove(subplot_args, "original_adjust_ylim");
    }
    gr_meta_args_remove(subplot_args, "reset_ranges");
  }
  if (gr_meta_args_contains(subplot_args, "panzoom")) {
    if (!gr_meta_args_contains(subplot_args, "original_xrange")) {
      gr_meta_args_push(subplot_args, "original_xrange", "dd", x_min, x_max);
      args_values(subplot_args, "adjust_xlim", "i", &adjust_xlim);
      gr_meta_args_push(subplot_args, "original_adjust_xlim", "i", adjust_xlim);
      gr_meta_args_push(subplot_args, "adjust_xlim", "i", 0);
    }
    if (!gr_meta_args_contains(subplot_args, "original_yrange")) {
      gr_meta_args_push(subplot_args, "original_yrange", "dd", y_min, y_max);
      args_values(subplot_args, "adjust_ylim", "i", &adjust_ylim);
      gr_meta_args_push(subplot_args, "original_adjust_ylim", "i", adjust_ylim);
      gr_meta_args_push(subplot_args, "adjust_ylim", "i", 0);
    }
    args_values(subplot_args, "panzoom", "ddd", &x, &y, &zoom);
    logger((stderr, "Window before `gr_panzoom` (%f, %f, %f, %f)\n", x_min, x_max, y_min, y_max));
    gr_panzoom(x, y, zoom, &x_min, &x_max, &y_min, &y_max);
    logger((stderr, "Window after `gr_panzoom` (%f, %f, %f, %f)\n", x_min, x_max, y_min, y_max));
    gr_meta_args_push(subplot_args, "xrange", "dd", x_min, x_max);
    gr_meta_args_push(subplot_args, "yrange", "dd", y_min, y_max);
    gr_meta_args_remove(subplot_args, "panzoom");
  }

  if (str_equals_any(kind, 6, "wireframe", "surface", "plot3", "scatter3", "polar", "trisurf")) {
    major_count = 2;
  } else {
    major_count = 5;
  }

  if (!(scale & GR_OPTION_X_LOG)) {
    args_values(subplot_args, "adjust_xlim", "i", &adjust_xlim);
    if (adjust_xlim) {
      logger((stderr, "xrange before \"gr_adjustlimits\": (%f, %f)\n", x_min, x_max));
      gr_adjustlimits(&x_min, &x_max);
      logger((stderr, "xrange after \"gr_adjustlimits\": (%f, %f)\n", x_min, x_max));
    }
    x_major_count = major_count;
    x_tick = gr_tick(x_min, x_max) / x_major_count;
  } else {
    x_tick = x_major_count = 1;
  }
  if (!(scale & GR_OPTION_FLIP_X)) {
    x_org_low = x_min;
    x_org_high = x_max;
  } else {
    x_org_low = x_max;
    x_org_high = x_min;
  }
  gr_meta_args_push(subplot_args, "xtick", "d", x_tick);
  gr_meta_args_push(subplot_args, "xorg", "dd", x_org_low, x_org_high);
  gr_meta_args_push(subplot_args, "xmajor", "i", x_major_count);

  if (str_equals_any(kind, 2, "hist", "stem") && !gr_meta_args_contains(subplot_args, "ylim")) {
    y_min = 0;
  }
  if (!(scale & GR_OPTION_Y_LOG)) {
    args_values(subplot_args, "adjust_ylim", "i", &adjust_ylim);
    if (adjust_ylim) {
      logger((stderr, "yrange before \"gr_adjustlimits\": (%f, %f)\n", y_min, y_max));
      gr_adjustlimits(&y_min, &y_max);
      logger((stderr, "yrange after \"gr_adjustlimits\": (%f, %f)\n", y_min, y_max));
    }
    y_major_count = major_count;
    y_tick = gr_tick(y_min, y_max) / y_major_count;
  } else {
    y_tick = y_major_count = 1;
  }
  if (!(scale & GR_OPTION_FLIP_Y)) {
    y_org_low = y_min;
    y_org_high = y_max;
  } else {
    y_org_low = y_max;
    y_org_high = y_min;
  }
  gr_meta_args_push(subplot_args, "ytick", "d", y_tick);
  gr_meta_args_push(subplot_args, "yorg", "dd", y_org_low, y_org_high);
  gr_meta_args_push(subplot_args, "ymajor", "i", y_major_count);

  logger((stderr, "Storing window (%f, %f, %f, %f)\n", x_min, x_max, y_min, y_max));
  gr_meta_args_push(subplot_args, "window", "dddd", x_min, x_max, y_min, y_max);
  if (strcmp(kind, "polar") != 0) {
    gr_setwindow(x_min, x_max, y_min, y_max);
  } else {
    gr_setwindow(-1.0, 1.0, -1.0, 1.0);
  }

  if (str_equals_any(kind, 5, "wireframe", "surface", "plot3", "scatter3", "trisurf")) {
    double z_major_count;
    double z_tick;
    double z_org_low, z_org_high;
    int rotation, tilt;

    args_values(subplot_args, "zrange", "dd", &z_min, &z_max);
    if (!(scale & GR_OPTION_Z_LOG)) {
      args_values(subplot_args, "adjust_zlim", "i", &adjust_zlim);
      if (adjust_zlim) {
        logger((stderr, "zrange before \"gr_adjustlimits\": (%f, %f)\n", z_min, z_max));
        gr_adjustlimits(&z_min, &z_max);
        logger((stderr, "zrange after \"gr_adjustlimits\": (%f, %f)\n", z_min, z_max));
      }
      z_major_count = major_count;
      z_tick = gr_tick(z_min, z_max) / z_major_count;
    } else {
      z_tick = z_major_count = 1;
    }
    if (!(scale & GR_OPTION_FLIP_Z)) {
      z_org_low = z_min;
      z_org_high = z_max;
    } else {
      z_org_low = z_max;
      z_org_high = z_min;
    }
    gr_meta_args_push(subplot_args, "ztick", "d", z_tick);
    gr_meta_args_push(subplot_args, "zorg", "dd", z_org_low, z_org_high);
    gr_meta_args_push(subplot_args, "zmajor", "i", z_major_count);

    args_values(subplot_args, "rotation", "i", &rotation);
    args_values(subplot_args, "tilt", "i", &tilt);
    gr_setspace(z_min, z_max, rotation, tilt);
  }

  gr_meta_args_push(subplot_args, "scale", "i", scale);
  gr_setscale(scale);
}

void plot_store_coordinate_ranges(gr_meta_args_t *subplot_args) {
  const char *kind;
  const char *fmt;
  gr_meta_args_t **current_series;
  unsigned int series_count;
  const char *data_component_names[] = {"x", "y", "z", NULL};
  const char **current_component_name;
  double *current_component = NULL;
  unsigned int point_count = 0;
  const char *range_keys[][2] = {{"xlim", "xrange"}, {"ylim", "yrange"}, {"zlim", "zrange"}};
  const char *(*current_range_keys)[2];
  unsigned int i;

  logger((stderr, "Storing coordinate ranges\n"));
  /* TODO: support that single `lim` values are `null` / unset! */

  args_first_value(subplot_args, "kind", "s", &kind, NULL);
  fmt = fmt_map_at(fmt_map, kind);
  current_range_keys = range_keys;
  current_component_name = data_component_names;
  while (*current_component_name != NULL) {
    double min_component = DBL_MAX;
    double max_component = -DBL_MAX;
    double step = -DBL_MAX;
    if (strchr(fmt, **current_component_name) == NULL || gr_meta_args_contains(subplot_args, (*current_range_keys)[1])) {
      ++current_range_keys;
      ++current_component_name;
      continue;
    }
    if (!gr_meta_args_contains(subplot_args, (*current_range_keys)[0])) {
      args_first_value(subplot_args, "series", "A", &current_series, &series_count);
      while (*current_series != NULL) {
        if (args_first_value(*current_series, *current_component_name, "D", &current_component, &point_count)) {
          for (i = 0; i < point_count; i++) {
            min_component = min(current_component[i], min_component);
            max_component = max(current_component[i], max_component);
          }
        }
        ++current_series;
      }
      if (strcmp(kind, "quiver") == 0) {
        step = max(find_max_step(point_count, current_component), step);
        if (step > 0.0) {
          min_component -= step;
          max_component += step;
        }
      }
    } else {
      args_values(subplot_args, (*current_range_keys)[0], "dd", &min_component, &max_component);
    }
    gr_meta_args_push(subplot_args, (*current_range_keys)[1], "dd", min_component, max_component);
    ++current_range_keys;
    ++current_component_name;
  }
  /* For quiver plots use u^2 + v^2 as z value */
  if (strcmp(kind, "quiver") == 0) {
    double min_component = DBL_MAX;
    double max_component = -DBL_MAX;
    if (!gr_meta_args_contains(subplot_args, "zlim")) {
      double *u, *v;
      /* TODO: Support more than one series? */
      /* TODO: `ERROR_PLOT_COMPONENT_LENGTH_MISMATCH` */
      args_first_value(subplot_args, "series", "A", &current_series, NULL);
      args_first_value(*current_series, "u", "D", &u, &point_count);
      args_first_value(*current_series, "v", "D", &v, NULL);
      for (i = 0; i < point_count; i++) {
        double z = u[i] * u[i] + v[i] * v[i];
        min_component = min(z, min_component);
        max_component = max(z, max_component);
      }
      min_component = sqrt(min_component);
      max_component = sqrt(max_component);
    } else {
      args_values(subplot_args, "zlim", "dd", &min_component, &max_component);
    }
    gr_meta_args_push(subplot_args, "zrange", "dd", min_component, max_component);
  }
}

void plot_post_plot(gr_meta_args_t *subplot_args) {
  int update;
  const char *kind;

  logger((stderr, "Post plot processsing\n"));

  gr_restorestate();
  args_first_value(subplot_args, "kind", "s", &kind, NULL);
  logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));
  if (str_equals_any(kind, 4, "line", "step", "scatter", "stem") && gr_meta_args_contains(subplot_args, "labels")) {
    plot_draw_legend(subplot_args);
  }
  args_first_value(subplot_args, "update", "i", &update, NULL);
  logger((stderr, "Got keyword \"update\" with value %d\n", update));
  if (update) {
    gr_updatews();
  }
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plotting ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

error_t plot_line(gr_meta_args_t *subplot_args) {
  gr_meta_args_t **current_series;

  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *x, *y;
    unsigned int x_length, y_length;
    char *spec;
    int mask;
    return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
    args_first_value(*current_series, "spec", "s", &spec, NULL); /* `spec` is always set */
    mask = gr_uselinespec(spec);
    if (int_equals_any(mask, 5, 0, 1, 3, 4, 5)) {
      gr_polyline(x_length, x, y);
    }
    if (mask & 2) {
      gr_polymarker(x_length, x, y);
    }
    ++current_series;
  }

  return NO_ERROR;
}

error_t plot_step(gr_meta_args_t *subplot_args) {
  gr_meta_args_t **current_series;

  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *x, *y;
    unsigned int x_length, y_length;
    char *spec;
    int mask;
    return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
    args_first_value(*current_series, "spec", "s", &spec, NULL); /* `spec` is always set */
    mask = gr_uselinespec(spec);
    if (int_equals_any(mask, 5, 0, 1, 3, 4, 5)) {
      const char *where;
      args_first_value(*current_series, "step_where", "s", &where, NULL); /* `spec` is always set */
      if (strcmp(where, "pre") == 0) {
      }
      /* TODO: implement this! */
      /*
       *     n = len(x)
       *     x_step_boundaries = np.zeros(2 * n - 1)
       *     y_step_values = np.zeros(2 * n - 1)
       *     x_step_boundaries[0] = x[0]
       *     x_step_boundaries[1::2] = x[:-1]
       *     x_step_boundaries[2::2] = x[1:]
       *     y_step_values[0] = y[0]
       *     y_step_values[1::2] = y[1:]
       *     y_step_values[2::2] = y[1:]
       * }
       * elif where == 'post':
       *     n = len(x)
       *     x_step_boundaries = np.zeros(2 * n - 1)
       *     y_step_values = np.zeros(2 * n - 1)
       *     x_step_boundaries[0::2] = x
       *     x_step_boundaries[1::2] = x[1:]
       *     x_step_boundaries[-1] = x[-1]
       *     y_step_values[0::2] = y
       *     y_step_values[1::2] = y[:-1]
       *     y_step_values[-1] = y[-1]
       * else:
       *     n = len(x)
       *     x_step_boundaries = np.zeros(2 * n)
       *     x_step_boundaries[0] = x[0]
       *     x_step_boundaries[1:-1][0::2] = (x[1:] + x[:-1]) / 2
       *     x_step_boundaries[1:-1][1::2] = (x[1:] + x[:-1]) / 2
       *     x_step_boundaries[-1] = x[-1]
       *     y_step_values = np.zeros(2 * n)
       *     y_step_values[0::2] = y
       *     y_step_values[1::2] = y
       * gr.polyline(x_step_boundaries, y_step_values)
       */
    }
    if (mask & 2) {
      gr_polymarker(x_length, x, y);
    }
    ++current_series;
  }

  return ERROR_NOT_IMPLEMENTED;
}

error_t plot_scatter(gr_meta_args_t *subplot_args) {
  gr_meta_args_t **current_series;

  gr_setmarkertype(GKS_K_MARKERTYPE_SOLID_CIRCLE);
  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *x = NULL, *y = NULL, *z = NULL, *c = NULL;
    unsigned int x_length, y_length, z_length, c_length;
    args_first_value(*current_series, "x", "D", &x, &x_length);
    args_first_value(*current_series, "y", "D", &y, &y_length);
    args_first_value(*current_series, "z", "D", &z, &z_length);
    args_first_value(*current_series, "c", "D", &c, &c_length);
    return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
    if (z != NULL || c != NULL) {
      if (c != NULL) {
        /* TODO: implement me! */
        /*
         * c_min = c.min()
         * c_ptp = c.ptp()
         */
      }
      /*
       * for i in range(len(x)):
       *     if z is not None:
       *         gr.setmarkersize(z[i] / 100.0)
       *     if c is not None:
       *         c_index = 1000 + int(255 * (c[i] - c_min) / c_ptp)
       *         gr.setmarkercolorind(c_index)
       *     gr.polymarker([x[i]], [y[i]])
       */
    } else {
      gr_polymarker(x_length, x, y);
    }
    ++current_series;
  }

  return ERROR_NOT_IMPLEMENTED;
}

error_t plot_quiver(gr_meta_args_t *subplot_args) {
  gr_meta_args_t **current_series;

  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *x = NULL, *y = NULL, *u = NULL, *v = NULL;
    unsigned int x_length, y_length, u_length, v_length;
    return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "u", "D", &u, &u_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "v", "D", &v, &v_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
    /* TODO: Check length of `u` and `v` */
    gr_quiver(x_length, y_length, x, y, u, v, 1);

    ++current_series;
  }

  return NO_ERROR;
}

error_t plot_stem(gr_meta_args_t *subplot_args) {
  const double *window;
  double base_line_y[2] = {0.0, 0.0};
  double stem_x[2], stem_y[2] = {0.0};
  gr_meta_args_t **current_series;

  args_first_value(subplot_args, "window", "D", &window, NULL);
  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *x, *y;
    unsigned int x_length, y_length;
    char *spec;
    unsigned int i;
    return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
    gr_polyline(2, (double *)window, base_line_y);
    gr_setmarkertype(GKS_K_MARKERTYPE_SOLID_CIRCLE);
    args_first_value(*current_series, "spec", "s", &spec, NULL);
    gr_uselinespec(spec);
    for (i = 0; i < x_length; ++i) {
      stem_x[0] = stem_x[1] = x[i];
      stem_y[1] = y[i];
      gr_polyline(2, stem_x, stem_y);
    }
    gr_polymarker(x_length, x, y);
    ++current_series;
  }

  return NO_ERROR;
}

error_t plot_hist(gr_meta_args_t *subplot_args) {
  const double *window;
  double y_min;
  gr_meta_args_t **current_series;

  args_first_value(subplot_args, "window", "D", &window, NULL);
  y_min = window[2];
  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *x, *y;
    unsigned int x_length, y_length;
    unsigned int i;
    return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
    for (i = 0; i <= y_length; ++i) {
      gr_setfillcolorind(989);
      gr_setfillintstyle(GKS_K_INTSTYLE_SOLID);
      gr_fillrect(x[i - 1], x[i], y_min, y[i - 1]);
      gr_setfillcolorind(1);
      gr_setfillintstyle(GKS_K_INTSTYLE_HOLLOW);
      gr_fillrect(x[i - 1], x[i], y_min, y[i - 1]);
    }
    ++current_series;
  }

  return NO_ERROR;
}

error_t plot_contour(gr_meta_args_t *subplot_args) {
  double z_min, z_max;
  int num_levels;
  double *h;
  double *gridit_x = NULL, *gridit_y = NULL, *gridit_z = NULL;
  gr_meta_args_t **current_series;
  int i;
  error_t error = NO_ERROR;

  args_values(subplot_args, "zrange", "dd", &z_min, &z_max);
  gr_setspace(z_min, z_max, 0, 90);
  args_values(subplot_args, "levels", "i", &num_levels);
  h = malloc(num_levels * sizeof(double));
  if (h == NULL) {
    debug_print_malloc_error();
    error = ERROR_MALLOC;
    goto cleanup;
  }
  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *x, *y, *z;
    unsigned int x_length, y_length, z_length;
    args_first_value(*current_series, "x", "D", &x, &x_length);
    args_first_value(*current_series, "y", "D", &y, &y_length);
    args_first_value(*current_series, "z", "D", &z, &z_length);
    if (x_length == y_length && x_length == z_length) {
      if (gridit_x == NULL) {
        gridit_x = malloc(PLOT_CONTOUR_GRIDIT_N * sizeof(double));
        gridit_y = malloc(PLOT_CONTOUR_GRIDIT_N * sizeof(double));
        gridit_z = malloc(PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N * sizeof(double));
        if (gridit_x == NULL || gridit_y == NULL || gridit_z == NULL) {
          debug_print_malloc_error();
          error = ERROR_MALLOC;
          goto cleanup;
        }
      }
      gr_gridit(x_length, x, y, z, PLOT_CONTOUR_GRIDIT_N, PLOT_CONTOUR_GRIDIT_N, gridit_x, gridit_y, gridit_z);
      for (i = 0; i < PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N; i++) {
        z_min = min(gridit_z[i], z_min);
        z_max = max(gridit_z[i], z_max);
      }
      for (i = 0; i < num_levels; ++i) {
        h[i] = z_min + (1.0 * i) / num_levels * (z_max - z_min);
      }
      gr_contour(PLOT_CONTOUR_GRIDIT_N, PLOT_CONTOUR_GRIDIT_N, num_levels, gridit_x, gridit_y, h, gridit_z, 1000);
    } else {
      if (x_length * y_length != z_length) {
        error = ERROR_PLOT_COMPONENT_LENGTH_MISMATCH;
        goto cleanup;
      }
      for (i = 0; i < num_levels; ++i) {
        h[i] = z_min + (1.0 * i) / num_levels * (z_max - z_min);
      }
      gr_contour(x_length, y_length, num_levels, x, y, h, z, 1000);
    }
    ++current_series;
  }
  if ((error = plot_draw_colorbar(subplot_args, 0.0, num_levels)) != NO_ERROR) {
    goto cleanup;
  }

cleanup:
  free(h);
  free(gridit_x);
  free(gridit_y);
  free(gridit_z);

  return error;
}

error_t plot_contourf(gr_meta_args_t *subplot_args) {
  double z_min, z_max;
  int num_levels, scale;
  double *h;
  double *gridit_x = NULL, *gridit_y = NULL, *gridit_z = NULL;
  gr_meta_args_t **current_series;
  int i;
  error_t error = NO_ERROR;

  args_values(subplot_args, "zrange", "dd", &z_min, &z_max);
  gr_setspace(z_min, z_max, 0, 90);
  args_values(subplot_args, "levels", "i", &num_levels);
  h = malloc(num_levels * sizeof(double));
  if (h == NULL) {
    debug_print_malloc_error();
    error = ERROR_MALLOC;
    goto cleanup;
  }
  args_values(subplot_args, "scale", "i", &scale);
  gr_setscale(scale);
  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *x, *y, *z;
    unsigned int x_length, y_length, z_length;
    args_first_value(*current_series, "x", "D", &x, &x_length);
    args_first_value(*current_series, "y", "D", &y, &y_length);
    args_first_value(*current_series, "z", "D", &z, &z_length);
    if ((error = plot_draw_colorbar(subplot_args, 0.0, num_levels)) != NO_ERROR) {
      goto cleanup;
    }
    gr_setlinecolorind(1);
    if (x_length == y_length && x_length == z_length) {
      if (gridit_x == NULL) {
        gridit_x = malloc(PLOT_CONTOUR_GRIDIT_N * sizeof(double));
        gridit_y = malloc(PLOT_CONTOUR_GRIDIT_N * sizeof(double));
        gridit_z = malloc(PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N * sizeof(double));
        if (gridit_x == NULL || gridit_y == NULL || gridit_z == NULL) {
          debug_print_malloc_error();
          error = ERROR_MALLOC;
          goto cleanup;
        }
      }
      gr_gridit(x_length, x, y, z, PLOT_CONTOUR_GRIDIT_N, PLOT_CONTOUR_GRIDIT_N, gridit_x, gridit_y, gridit_z);
      for (i = 0; i < PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N; i++) {
        z_min = min(gridit_z[i], z_min);
        z_max = max(gridit_z[i], z_max);
      }
      for (i = 0; i < num_levels; ++i) {
        h[i] = z_min + (1.0 * i) / num_levels * (z_max - z_min);
      }
      gr_contourf(PLOT_CONTOUR_GRIDIT_N, PLOT_CONTOUR_GRIDIT_N, num_levels, gridit_x, gridit_y, h, gridit_z, 1000);
    } else {
      if (x_length * y_length != z_length) {
        error = ERROR_PLOT_COMPONENT_LENGTH_MISMATCH;
        goto cleanup;
      }
      for (i = 0; i < num_levels; ++i) {
        h[i] = z_min + (1.0 * i) / num_levels * (z_max - z_min);
      }
      gr_contourf(x_length, y_length, num_levels, x, y, h, z, 1000);
    }
    ++current_series;
  }

cleanup:
  free(h);
  free(gridit_x);
  free(gridit_y);
  free(gridit_z);

  return error;
}

error_t plot_hexbin(gr_meta_args_t *subplot_args) {
  int nbins;
  gr_meta_args_t **current_series;

  args_values(subplot_args, "nbins", "i", &nbins);
  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *x, *y;
    unsigned int x_length, y_length;
    int cntmax;
    return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
    cntmax = gr_hexbin(x_length, x, y, nbins);
    /* TODO: return an error in the else case? */
    if (cntmax > 0) {
      gr_meta_args_push(subplot_args, "zrange", "dd", 0.0, 1.0 * cntmax);
      plot_draw_colorbar(subplot_args, 0.0, 256);
    }
    ++current_series;
  }

  return NO_ERROR;
}

error_t plot_heatmap(gr_meta_args_t *subplot_args) {
  gr_meta_args_t **current_series;

  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {

    /* TODO: Implement me! */
    /* TODO: How to deal with z.shape? */
    /*
     * x_min, x_max, y_min, y_max = _plt.kwargs['window']
     * height, width = z.shape
     * cmap = _colormap()
     * icmap = np.zeros(256, np.uint32)
     * for i in range(256):
     *     r, g, b, a = cmap[i]
     *     icmap[i] = (int(r * 255) << 0) + (int(g * 255) << 8) + (int(b * 255) << 16) + (int(a * 255) << 24)
     * z_min, z_max = _plt.kwargs.get('zlim', (np.min(z), np.max(z)))
     * if z_max < z_min:
     *     z_max, z_min = z_min, z_max
     * if z_max > z_min:
     *     data = (z - z_min) / (z_max - z_min) * 255
     * else:
     *     data = np.zeros((height, width))
     * rgba = np.zeros((height, width), np.uint32)
     * for x in range(width):
     *     for y in range(height):
     *         rgba[y, x] = icmap[int(data[y, x])]
     * gr.drawimage(x_min, x_max, y_min, y_max, width, height, rgba)
     * _colorbar()
     */

    ++current_series;
  }

  return ERROR_NOT_IMPLEMENTED;
}

error_t plot_wireframe(gr_meta_args_t *subplot_args) {
  double *gridit_x = NULL, *gridit_y = NULL, *gridit_z = NULL;
  gr_meta_args_t **current_series;
  error_t error = NO_ERROR;

  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *x, *y, *z;
    unsigned int x_length, y_length, z_length;
    args_first_value(*current_series, "x", "D", &x, &x_length);
    args_first_value(*current_series, "y", "D", &y, &y_length);
    args_first_value(*current_series, "z", "D", &z, &z_length);
    gr_setfillcolorind(0);
    if (x_length == y_length && x_length == z_length) {
      if (gridit_x == NULL) {
        gridit_x = malloc(PLOT_WIREFRAME_GRIDIT_N * sizeof(double));
        gridit_y = malloc(PLOT_WIREFRAME_GRIDIT_N * sizeof(double));
        gridit_z = malloc(PLOT_WIREFRAME_GRIDIT_N * PLOT_WIREFRAME_GRIDIT_N * sizeof(double));
        if (gridit_x == NULL || gridit_y == NULL || gridit_z == NULL) {
          debug_print_malloc_error();
          error = ERROR_MALLOC;
          goto cleanup;
        }
      }
      gr_gridit(x_length, x, y, z, PLOT_WIREFRAME_GRIDIT_N, PLOT_WIREFRAME_GRIDIT_N, gridit_x, gridit_y, gridit_z);
      gr_surface(PLOT_WIREFRAME_GRIDIT_N, PLOT_WIREFRAME_GRIDIT_N, gridit_x, gridit_y, gridit_z, GR_OPTION_FILLED_MESH);
    } else {
      if (x_length * y_length != z_length) {
        error = ERROR_PLOT_COMPONENT_LENGTH_MISMATCH;
        goto cleanup;
      }
      gr_surface(x_length, y_length, x, y, z, GR_OPTION_FILLED_MESH);
    }
    ++current_series;
  }
  plot_draw_axes(subplot_args, 2);

cleanup:
  free(gridit_x);
  free(gridit_y);
  free(gridit_z);

  return error;
}

error_t plot_surface(gr_meta_args_t *subplot_args) {
  double *gridit_x = NULL, *gridit_y = NULL, *gridit_z = NULL;
  gr_meta_args_t **current_series;
  error_t error = NO_ERROR;

  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *x, *y, *z;
    unsigned int x_length, y_length, z_length;
    args_first_value(*current_series, "x", "D", &x, &x_length);
    args_first_value(*current_series, "y", "D", &y, &y_length);
    args_first_value(*current_series, "z", "D", &z, &z_length);
    /* TODO: add support for GR3 */
    if (x_length == y_length && x_length == z_length) {
      if (gridit_x == NULL) {
        gridit_x = malloc(PLOT_SURFACE_GRIDIT_N * sizeof(double));
        gridit_y = malloc(PLOT_SURFACE_GRIDIT_N * sizeof(double));
        gridit_z = malloc(PLOT_SURFACE_GRIDIT_N * PLOT_SURFACE_GRIDIT_N * sizeof(double));
        if (gridit_x == NULL || gridit_y == NULL || gridit_z == NULL) {
          debug_print_malloc_error();
          error = ERROR_MALLOC;
          goto cleanup;
        }
      }
      gr_gridit(x_length, x, y, z, PLOT_SURFACE_GRIDIT_N, PLOT_SURFACE_GRIDIT_N, gridit_x, gridit_y, gridit_z);
      gr_surface(PLOT_SURFACE_GRIDIT_N, PLOT_SURFACE_GRIDIT_N, gridit_x, gridit_y, gridit_z, GR_OPTION_COLORED_MESH);
    } else {
      if (x_length * y_length != z_length) {
        error = ERROR_PLOT_COMPONENT_LENGTH_MISMATCH;
        goto cleanup;
      }
      gr_surface(x_length, y_length, x, y, z, GR_OPTION_COLORED_MESH);
    }
    ++current_series;
  }
  plot_draw_axes(subplot_args, 2);
  plot_draw_colorbar(subplot_args, 0.05, 256);

cleanup:
  free(gridit_x);
  free(gridit_y);
  free(gridit_z);

  return error;
}

error_t plot_plot3(gr_meta_args_t *subplot_args) {
  gr_meta_args_t **current_series;

  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *x, *y, *z;
    unsigned int x_length, y_length, z_length;
    return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "z", "D", &z, &z_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(x_length != y_length || x_length != z_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
    gr_polyline3d(x_length, x, y, z);
    ++current_series;
  }
  plot_draw_axes(subplot_args, 2);

  return NO_ERROR;
}

error_t plot_scatter3(gr_meta_args_t *subplot_args) {
  gr_meta_args_t **current_series;

  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    /* TODO: Implement me! */
    /*
     * gr.setmarkertype(gr.MARKERTYPE_SOLID_CIRCLE)
     * if c is not None:
     *     c_min = c.min()
     *     c_ptp = c.ptp()
     *     for i in range(len(x)):
     *         c_index = 1000 + int(255 * (c[i] - c_min) / c_ptp)
     *         gr.setmarkercolorind(c_index)
     *         gr.polymarker3d([x[i]], [y[i]], [z[i]])
     * else:
     *     gr.polymarker3d(x, y, z)
     */
    ++current_series;
  }
  plot_draw_axes(subplot_args, 2);

  return ERROR_NOT_IMPLEMENTED;
}

error_t plot_imshow(gr_meta_args_t *subplot_args) {
  gr_meta_args_t **current_series;

  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    /* TODO: Implement me! */
    ++current_series;
  }

  return ERROR_NOT_IMPLEMENTED;
}

error_t plot_isosurface(gr_meta_args_t *subplot_args) {
  gr_meta_args_t **current_series;

  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    /* TODO: Implement me! */
    ++current_series;
  }

  return ERROR_NOT_IMPLEMENTED;
}

error_t plot_polar(gr_meta_args_t *subplot_args) {
  const double *window;
  double r_min, r_max, tick;
  int n;
  gr_meta_args_t **current_series;

  args_first_value(subplot_args, "window", "D", &window, NULL);
  r_min = window[2];
  r_max = window[3];
  tick = 0.5 * gr_tick(r_min, r_max);
  n = (int)ceil((r_max - r_min) / tick);
  r_max = r_min + n * tick;
  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *rho, *theta, *x, *y;
    unsigned int rho_length, theta_length;
    char *spec;
    unsigned int i;
    return_error_if(!args_first_value(*current_series, "x", "D", &theta, &theta_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "y", "D", &rho, &rho_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(rho_length != theta_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
    x = malloc(rho_length * sizeof(double));
    y = malloc(rho_length * sizeof(double));
    if (x == NULL || y == NULL) {
      debug_print_malloc_error();
      free(x);
      free(y);
      return ERROR_MALLOC;
    }
    for (i = 0; i < rho_length; ++i) {
      double current_rho = (rho[i] - r_min) / (r_max - r_min);
      x[i] = current_rho * cos(theta[i]);
      y[i] = current_rho * sin(theta[i]);
    }
    args_first_value(*current_series, "spec", "s", &spec, NULL); /* `spec` is always set */
    gr_uselinespec(spec);
    gr_polyline(rho_length, x, y);
    free(x);
    free(y);
    ++current_series;
  }

  return NO_ERROR;
}

error_t plot_trisurf(gr_meta_args_t *subplot_args) {
  gr_meta_args_t **current_series;

  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *x, *y, *z;
    unsigned int x_length, y_length, z_length;
    return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "z", "D", &z, &z_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(x_length != y_length || x_length != z_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
    gr_trisurface(x_length, x, y, z);
    ++current_series;
  }
  plot_draw_axes(subplot_args, 2);
  plot_draw_colorbar(subplot_args, 0.05, 256);

  return NO_ERROR;
}

error_t plot_tricont(gr_meta_args_t *subplot_args) {
  double z_min, z_max;
  double *levels;
  int num_levels;
  gr_meta_args_t **current_series;
  int i;

  args_values(subplot_args, "zrange", "dd", &z_min, &z_max);
  args_values(subplot_args, "levels", "i", &num_levels);
  levels = malloc(num_levels * sizeof(double));
  if (levels == NULL) {
    debug_print_malloc_error();
    return ERROR_MALLOC;
  }
  for (i = 0; i < num_levels; ++i) {
    levels[i] = z_min + ((1.0 * i) / (num_levels - 1)) * (z_max - z_min);
  }
  args_first_value(subplot_args, "series", "A", &current_series, NULL);
  while (*current_series != NULL) {
    double *x, *y, *z;
    unsigned int x_length, y_length, z_length;
    return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(!args_first_value(*current_series, "z", "D", &z, &z_length), ERROR_PLOT_MISSING_DATA);
    return_error_if(x_length != y_length || x_length != z_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
    gr_tricontour(x_length, x, y, z, num_levels, levels);
    ++current_series;
  }
  plot_draw_axes(subplot_args, 2);
  plot_draw_colorbar(subplot_args, 0.05, 256);
  free(levels);

  return NO_ERROR;
}

error_t plot_shade(gr_meta_args_t *subplot_args) {
  gr_meta_args_t **current_shader;
  const char *data_component_names[] = {"x", "y", NULL};
  double *components[2];
  char *spec = ""; /* TODO: read spec from data! */
  int xform, width, height;
  double **current_component = components;
  const char **current_component_name = data_component_names;
  unsigned int point_count;

  args_first_value(subplot_args, "series", "A", &current_shader, NULL);
  while (*current_component_name != NULL) {
    args_first_value(*current_shader, *current_component_name, "D", current_component, &point_count);
    ++current_component_name;
    ++current_component;
  }
  if (!args_first_value(subplot_args, "xform", "i", &xform, NULL)) {
    xform = 1;
  }
  if (!args_first_value(subplot_args, "width", "i", &width, NULL)) {
    width = 100;
  }
  if (!args_first_value(subplot_args, "height", "i", &height, NULL)) {
    height = 100;
  }
  gr_shadepoints(point_count, components[0], components[1], xform, width, height);

  return NO_ERROR;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ auxiliary drawing functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

error_t plot_draw_axes(gr_meta_args_t *args, unsigned int pass) {
  const char *kind = NULL;
  const double *viewport, *vp;
  double x_tick;
  double x_org_low, x_org_high;
  int x_major_count;
  double y_tick;
  double y_org_low, y_org_high;
  int y_major_count;
  double z_tick;
  double z_org_low, z_org_high;
  int z_major_count;
  double diag;
  double charheight;
  double ticksize;
  char *title;
  char *x_label, *y_label, *z_label;

  args_first_value(args, "kind", "s", &kind, NULL);
  args_first_value(args, "viewport", "D", &viewport, NULL);
  args_first_value(args, "vp", "D", &vp, NULL);
  args_values(args, "xtick", "d", &x_tick);
  args_values(args, "xorg", "dd", &x_org_low, &x_org_high);
  args_values(args, "xmajor", "i", &x_major_count);
  args_values(args, "ytick", "d", &y_tick);
  args_values(args, "yorg", "dd", &y_org_low, &y_org_high);
  args_values(args, "ymajor", "i", &y_major_count);

  gr_setlinecolorind(1);
  gr_setlinewidth(1);
  diag = sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
              (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));
  charheight = max(0.018 * diag, 0.012);
  gr_setcharheight(charheight);
  ticksize = 0.0075 * diag;
  if (str_equals_any(kind, 5, "wireframe", "surface", "plot3", "scatter3", "trisurf")) {
    args_values(args, "ztick", "d", &z_tick);
    args_values(args, "zorg", "dd", &z_org_low, &z_org_high);
    args_values(args, "zmajor", "i", &z_major_count);
    if (pass == 1) {
      gr_grid3d(x_tick, 0, z_tick, x_org_low, y_org_high, z_org_low, 2, 0, 2);
      gr_grid3d(0, y_tick, 0, x_org_low, y_org_high, z_org_low, 0, 2, 0);
    } else {
      gr_axes3d(x_tick, 0, z_tick, x_org_low, y_org_low, z_org_low, x_major_count, 0, z_major_count, -ticksize);
      gr_axes3d(0, y_tick, 0, x_org_high, y_org_low, z_org_low, 0, y_major_count, 0, ticksize);
    }
  } else {
    if (str_equals_any(kind, 2, "heatmap", "shade")) {
      ticksize = -ticksize;
    } else {
      gr_grid(x_tick, y_tick, 0, 0, x_major_count, y_major_count);
    }
    gr_axes(x_tick, y_tick, x_org_low, y_org_low, x_major_count, y_major_count, ticksize);
    gr_axes(x_tick, y_tick, x_org_high, y_org_high, -x_major_count, -y_major_count, -ticksize);
  }

  if (args_first_value(args, "title", "s", &title, NULL)) {
    gr_savestate();
    gr_settextalign(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);
    gr_textext(0.5 * (viewport[0] + viewport[1]), vp[3], title);
    gr_restorestate();
  }

  if (str_equals_any(kind, 5, "wireframe", "surface", "plot3", "scatter3", "trisurf")) {
    if (args_first_value(args, "xlabel", "s", &x_label, NULL) &&
        args_first_value(args, "ylabel", "s", &y_label, NULL) &&
        args_first_value(args, "zlabel", "s", &z_label, NULL)) {
      gr_titles3d(x_label, y_label, z_label);
    }
  } else {
    if (args_first_value(args, "xlabel", "s", &x_label, NULL)) {
      gr_savestate();
      gr_settextalign(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_BOTTOM);
      gr_textext(0.5 * (viewport[0] + viewport[1]), vp[2] + 0.5 * charheight, x_label);
      gr_restorestate();
    }
    if (args_first_value(args, "ylabel", "s", &y_label, NULL)) {
      gr_savestate();
      gr_settextalign(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);
      gr_setcharup(-1, 0);
      gr_textext(vp[0] + 0.5 * charheight, 0.5 * (viewport[2] + viewport[3]), y_label);
      gr_restorestate();
    }
  }

  return NO_ERROR;
}

error_t plot_draw_polar_axes(gr_meta_args_t *args) {
  const double *window, *viewport;
  double diag;
  double charheight;
  double r_min, r_max;
  double tick;
  double x[2], y[2];
  int i, n, alpha;
  char text_buffer[PLOT_POLAR_AXES_TEXT_BUFFER];

  args_first_value(args, "viewport", "D", &viewport, NULL);
  diag = sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
              (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));
  charheight = max(0.018 * diag, 0.012);

  args_first_value(args, "window", "D", &window, NULL);
  r_min = window[2];
  r_max = window[3];

  gr_savestate();
  gr_setcharheight(charheight);
  gr_setlinetype(GKS_K_LINETYPE_SOLID);

  tick = 0.5 * gr_tick(r_min, r_max);
  n = (int)ceil((r_max - r_min) / tick);
  for (i = 0; i <= n; i++) {
    int r = i / n;
    if (i % 2 == 0) {
      gr_setlinecolorind(88);
      if (i > 0) {
        gr_drawarc(-r, r, -r, r, 0, 180);
        gr_drawarc(-r, r, -r, r, 180, 360);
      }
      gr_settextalign(GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);
      x[0] = 0.05;
      y[0] = r;
      gr_wctondc(x, y);
      snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%g", r_min + i * tick);
      gr_text(x[0], y[0], text_buffer);
    } else {
      gr_setlinecolorind(90);
      gr_drawarc(-r, r, -r, r, 0, 180);
      gr_drawarc(-r, r, -r, r, 180, 360);
    }
  }
  for (alpha = 0; alpha < 360; alpha += 45) {
    x[0] = sin((alpha + 90) * M_PI / 180.0);
    y[0] = cos((alpha + 90) * M_PI / 180.0);
    x[1] = 0.0;
    y[1] = 0.0;
    gr_polyline(2, x, y);
    gr_settextalign(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_HALF);
    x[0] *= 1.1;
    y[0] *= 1.1;
    gr_wctondc(x, y);
    snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%d\xb0", alpha);
    gr_textext(x[0], y[0], text_buffer);
  }
  gr_restorestate();

  return NO_ERROR;
}

error_t plot_draw_legend(gr_meta_args_t *subplot_args) {
  const char **labels, **current_label;
  unsigned int num_labels, num_series;
  gr_meta_args_t **current_series;
  const double *viewport;
  int location;
  double px, py, w, h;
  double tbx[4], tby[4];
  double legend_symbol_x[2], legend_symbol_y[2];


  return_error_if(!args_first_value(subplot_args, "labels", "S", &labels, &num_labels), ERROR_PLOT_MISSING_LABELS);
  logger((stderr, "Draw a legend with %d labels\n", num_labels));
  args_first_value(subplot_args, "series", "A", &current_series, &num_series);
  args_first_value(subplot_args, "viewport", "D", &viewport, NULL);
  args_values(subplot_args, "location", "i", &location);
  gr_savestate();
  gr_selntran(0);
  gr_setscale(0);
  w = 0;
  for (current_label = labels; *current_label != NULL; ++current_label) {
    gr_inqtextext(0, 0, *(char **)current_label, tbx, tby);
    w = max(w, tbx[2]);
  }

  h = (num_series + 1) * 0.03;
  if (int_equals_any(location, 3, 8, 9, 10)) {
    px = 0.5 * (viewport[0] + viewport[1] - w);
  } else if (int_equals_any(location, 3, 2, 3, 6)) {
    px = viewport[0] + 0.11;
  } else {
    px = viewport[1] - 0.05 - w;
  }
  if (int_equals_any(location, 4, 5, 6, 7, 10)) {
    py = 0.5 * (viewport[2] + viewport[3] + h) - 0.03;
  } else if (int_equals_any(location, 3, 3, 4, 8)) {
    py = viewport[2] + h;
  } else {
    py = viewport[3] - 0.06;
  }

  gr_setfillintstyle(GKS_K_INTSTYLE_SOLID);
  gr_setfillcolorind(0);
  gr_fillrect(px - 0.08, px + w + 0.02, py + 0.03, py - 0.03 * num_series);
  gr_setlinetype(GKS_K_INTSTYLE_SOLID);
  gr_setlinecolorind(1);
  gr_setlinewidth(1);
  gr_drawrect(px - 0.08, px + w + 0.02, py + 0.03, py - 0.03 * num_series);
  gr_uselinespec(" ");
  current_label = labels;
  while (*current_series != NULL) {
    char *spec;
    int mask;

    gr_savestate();
    args_first_value(*current_series, "spec", "s", &spec, NULL); /* `spec` is always set */
    mask = gr_uselinespec(spec);
    if (int_equals_any(mask, 5, 0, 1, 3, 4, 5)) {
      legend_symbol_x[0] = px - 0.07;
      legend_symbol_x[1] = px - 0.01;
      legend_symbol_y[0] = py;
      legend_symbol_y[1] = py;
      gr_polyline(2, legend_symbol_x, legend_symbol_y);
    }
    if (mask & 2) {
      legend_symbol_x[0] = px - 0.06;
      legend_symbol_x[1] = px - 0.02;
      legend_symbol_y[0] = py;
      legend_symbol_y[1] = py;
      gr_polymarker(2, legend_symbol_x, legend_symbol_y);
    }
    gr_restorestate();
    gr_settextalign(GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);
    if (*current_label != NULL) {
      gr_textext(px, py, (char *)*current_label);
      ++current_label;
    }
    py -= 0.03;
    ++current_series;
  }
  gr_selntran(1);
  gr_restorestate();

  return NO_ERROR;
}

error_t plot_draw_colorbar(gr_meta_args_t *args, double off, unsigned int colors) {
  const double *viewport;
  double z_min, z_max;
  int *data;
  double diag, charheight;
  int scale;
  unsigned int i;

  gr_savestate();
  args_first_value(args, "viewport", "D", &viewport, NULL);
  args_values(args, "zrange", "dd", &z_min, &z_max);
  data = malloc(colors * sizeof(int));
  if (data == NULL) {
    debug_print_malloc_error();
    return ERROR_MALLOC;
  }
  for (i = 0; i < colors; ++i) {
    data[i] = 1000 + 255 * i / (colors - 1);
  }
  gr_setwindow(0.0, 1.0, z_min, z_max);
  gr_setviewport(viewport[1] + 0.02 + off, viewport[1] + 0.05 + off, viewport[2], viewport[3]);
  gr_cellarray(0, 1, z_max, z_min, 1, colors, 1, 1, 1, colors, data);
  diag = sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
              (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));
  charheight = max(0.016 * diag, 0.012);
  gr_setcharheight(charheight);
  args_values(args, "scale", "i", &scale);
  if (scale & GR_OPTION_Z_LOG) {
    gr_setscale(GR_OPTION_Y_LOG);
    gr_axes(0, 2, 1, z_min, 0, 1, 0.005);
  } else {
    double z_tick = 0.5 * gr_tick(z_min, z_max);
    gr_axes(0, z_tick, 1, z_min, 0, 1, 0.005);
  }
  free(data);
  gr_restorestate();

  return NO_ERROR;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

double find_max_step(unsigned int n, const double *x) {
  double max_step = 0.0;
  unsigned int i;

  if (n < 2) {
    return 0.0;
  }
  for (i = 1; i < n; ++i) {
    max_step = max(x[i] - x[i - 1], max_step);
  }

  return max_step;
}

const char *next_fmt_key(const char *kind) {
  static const char *saved_fmt = NULL;
  static char fmt_key[2] = {0, 0};

  if (kind != NULL) {
    saved_fmt = fmt_map_at(fmt_map, kind);
  }
  if (saved_fmt == NULL) {
    return NULL;
  }
  fmt_key[0] = *saved_fmt;
  if (*saved_fmt != '\0') {
    ++saved_fmt;
  }

  return fmt_key;
}


/* ------------------------- util ----------------------------------------------------------------------------------- */

size_t djb2_hash(const char *str) {
  /* String hash function by Dan Bernstein, see http://www.cse.yorku.ca/~oz/hash.html */
  size_t hash = 5381;
  char c;

  while ((c = *str++) != '\0') {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }

  return hash;
}

int is_int_number(const char *str) {
  return strchr(FROMJSON_VALID_DELIMITERS, str[strspn(str, "0123456789-+")]) != NULL;
}

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

int int_equals_any(int number, unsigned int n, ...) {
  va_list vl;
  int current_number;
  int any_is_equal;
  unsigned int i;

  va_start(vl, n);
  any_is_equal = 0;
  for (i = 0; i < n; i++) {
    current_number = va_arg(vl, int);
    if (number == current_number) {
      any_is_equal = 1;
      break;
    }
  }
  va_end(vl);

  return any_is_equal;
}

int str_equals_any(const char *str, unsigned int n, ...) {
  va_list vl;
  char *current_str;
  int any_is_equal;
  unsigned int i;

  va_start(vl, n);
  any_is_equal = 0;
  for (i = 0; i < n; i++) {
    current_str = va_arg(vl, char *);
    if (!strcmp(str, current_str)) {
      any_is_equal = 1;
      break;
    }
  }
  va_end(vl);

  return any_is_equal;
}
int uppercase_count(const char *str) {
  int uppercase_count = 0;

  while (*str) {
    if (isupper(*str)) {
      ++uppercase_count;
    }
    ++str;
  }
  return uppercase_count;
}


/* ========================= methods ================================================================================ */

/* ------------------------- argument ------------------------------------------------------------------------------- */

args_value_iterator_t *arg_value_iter(const arg_t *arg) {
  return args_value_iterator_new(arg);
}


/* ------------------------- argument container --------------------------------------------------------------------- */

void args_init(gr_meta_args_t *args) {
  args->kwargs_head = NULL;
  args->kwargs_tail = NULL;
  args->count = 0;
}

void args_finalize(gr_meta_args_t *args) {
  gr_meta_args_clear(args);
}

gr_meta_args_t *args_flatcopy(const gr_meta_args_t *copy_args) {
  /* Clone the linked list but share the referenced values */
  gr_meta_args_t *args = NULL;
  args_iterator_t *it = NULL;
  args_node_t *args_node;
  arg_t *copy_arg;

  args = gr_newmeta();
  if (args == NULL) {
    debug_print_malloc_error();
    goto error_cleanup;
  }
  it = args_iter(copy_args);
  while ((copy_arg = it->next(it)) != NULL) {
    ++(copy_arg->priv->reference_count);
    args_node = malloc(sizeof(args_node_t));
    if (args_node == NULL) {
      debug_print_malloc_error();
      goto error_cleanup;
    }
    args_node->arg = copy_arg;
    args_node->next = NULL;

    if (args->kwargs_head == NULL) {
      args->kwargs_head = args_node;
    } else {
      args->kwargs_tail->next = args_node;
    }
    args->kwargs_tail = args_node;
    ++(args->count);
  }
  args_iterator_delete(it);

  return args;

error_cleanup:
  if (args != NULL) {
    gr_deletemeta(args);
  }
  if (it != NULL) {
    args_iterator_delete(it);
  }

  return NULL;
}

gr_meta_args_t *args_copy(const gr_meta_args_t *copy_args, const char **keys_copy_as_array) {
  /* Clone the linked list and all values that are argument containers as well. Share all other values (-> **no deep
   * copy!**).
   * `keys_copy_as_array` can be used to always copy values of the specified keys as an array. It is only read for
   * values which are argument containers. The array must be terminated with a NULL pointer. */
  gr_meta_args_t *args = NULL, **args_array = NULL, *copied_args = NULL, **copied_args_array = NULL,
                 **current_args_copy = NULL;
  args_iterator_t *it = NULL;
  args_value_iterator_t *value_it = NULL;
  args_node_t *args_node;
  arg_t *copy_arg;
  const char **current_key_ptr;
  int copy_as_array;

  args = gr_newmeta();
  if (args == NULL) {
    debug_print_malloc_error();
    goto error_cleanup;
  }
  it = args_iter(copy_args);
  error_cleanup_if(it == NULL);
  while ((copy_arg = it->next(it)) != NULL) {
    if (strncmp(copy_arg->value_format, "a", 1) == 0 || strncmp(copy_arg->value_format, "nA", 2) == 0) {
      value_it = arg_value_iter(copy_arg);
      error_cleanup_if(value_it == NULL);
      /* Do not support two dimensional argument arrays like `nAnA`) -> a loop would be needed with more memory
       * management */
      error_cleanup_if(value_it->next(value_it) == NULL);
      if (value_it->is_array) {
        args_array = *(gr_meta_args_t ***)value_it->value_ptr;
        copied_args_array = malloc(value_it->array_length * sizeof(gr_meta_args_t *));
        error_cleanup_if(copied_args_array == NULL);
        current_args_copy = copied_args_array;
        while (*args_array != NULL) {
          *current_args_copy = args_copy(*args_array, keys_copy_as_array);
          error_cleanup_if(*current_args_copy == NULL);
          ++args_array;
          ++current_args_copy;
        }
        current_args_copy = NULL;
        gr_meta_args_push(args, it->arg->key, "nA", value_it->array_length, copied_args_array);
      } else {
        copied_args = args_copy(*(gr_meta_args_t **)value_it->value_ptr, keys_copy_as_array);
        error_cleanup_if(copied_args == NULL);
        copy_as_array = 0;
        if (keys_copy_as_array != NULL) {
          current_key_ptr = keys_copy_as_array;
          while (*current_key_ptr != NULL) {
            if (strcmp(it->arg->key, *current_key_ptr) == 0) {
              copy_as_array = 1;
              break;
            }
            ++current_key_ptr;
          }
        }
        if (copy_as_array) {
          gr_meta_args_push(args, it->arg->key, "A(1)", &copied_args);
        } else {
          gr_meta_args_push(args, it->arg->key, "a", copied_args);
        }
        copied_args = NULL;
      }
    } else {
      ++(copy_arg->priv->reference_count);
      args_node = malloc(sizeof(args_node_t));
      if (args_node == NULL) {
        debug_print_malloc_error();
        goto error_cleanup;
      }
      args_node->arg = copy_arg;
      args_node->next = NULL;

      if (args->kwargs_head == NULL) {
        args->kwargs_head = args_node;
      } else {
        args->kwargs_tail->next = args_node;
      }
      args->kwargs_tail = args_node;
      ++(args->count);
    }
  }
  goto cleanup;

error_cleanup:
  if (args != NULL) {
    gr_deletemeta(args);
    args = NULL;
  }
cleanup:
  if (current_args_copy != NULL) {
    while (current_args_copy != copied_args_array) {
      if (*current_args_copy != NULL) {
        gr_deletemeta(*current_args_copy);
      }
      --current_args_copy;
    }
  }
  if (copied_args_array != NULL) {
    free(copied_args_array);
  }
  if (it != NULL) {
    args_iterator_delete(it);
  }
  if (value_it != NULL) {
    args_value_iterator_delete(value_it);
  }

  return args;
}

error_t args_push_common(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                         va_list *vl, int apply_padding) {
  arg_t *arg;
  args_node_t *args_node;

  if ((arg = args_create_args(key, value_format, buffer, vl, apply_padding)) == NULL) {
    return ERROR_MALLOC;
  }

  if ((args_node = args_find_node(args, key)) != NULL) {
    args_decrease_arg_reference_count(args_node);
    args_node->arg = arg;
  } else {
    args_node = malloc(sizeof(args_node_t));
    if (args_node == NULL) {
      debug_print_malloc_error();
      free((char *)arg->key);
      free((char *)arg->value_format);
      free(arg->priv);
      free(arg);
      return ERROR_MALLOC;
    }
    args_node->arg = arg;
    args_node->next = NULL;
    if (args->kwargs_head == NULL) {
      args->kwargs_head = args_node;
      args->kwargs_tail = args_node;
    } else {
      args->kwargs_tail->next = args_node;
      args->kwargs_tail = args_node;
    }
    ++(args->count);
  }

  return NO_ERROR;
}

error_t args_push_vl(gr_meta_args_t *args, const char *key, const char *value_format, va_list *vl) {
  return args_push_common(args, key, value_format, NULL, vl, 0);
}

error_t args_update_many(gr_meta_args_t *args, const gr_meta_args_t *update_args) {
  return args_merge(args, update_args, NULL);
}

error_t args_merge(gr_meta_args_t *args, const gr_meta_args_t *merge_args, const char **merge_keys) {
  args_iterator_t *it = NULL;
  args_value_iterator_t *value_it = NULL, *merge_value_it = NULL;
  args_node_t *args_node, *previous_node_by_keyword;
  arg_t *update_arg, *current_arg;
  gr_meta_args_t **args_array, **merge_args_array;
  const char **current_key_ptr;
  int merge, i;
  error_t error = NO_ERROR;

  it = args_iter(merge_args);
  cleanup_and_set_error_if(it == NULL, ERROR_MALLOC);
  while ((update_arg = it->next(it)) != NULL) {
    merge = 0;
    if (merge_keys != NULL) {
      current_key_ptr = merge_keys;
      while (*current_key_ptr != NULL) {
        if (strcmp(it->arg->key, *current_key_ptr) == 0) {
          merge = 1;
          break;
        }
        ++current_key_ptr;
      }
    }
    if (merge && (current_arg = args_at(args, update_arg->key)) != NULL) {
      value_it = arg_value_iter(current_arg);
      merge_value_it = arg_value_iter(update_arg);
      cleanup_and_set_error_if(value_it == NULL, ERROR_MALLOC);
      cleanup_and_set_error_if(merge_value_it == NULL, ERROR_MALLOC);
      /* Do not support two dimensional argument arrays like `nAnA`) -> a loop would be needed with more memory
       * management */
      cleanup_and_set_error_if(value_it->next(value_it) == NULL, ERROR_MALLOC);
      cleanup_and_set_error_if(merge_value_it->next(merge_value_it) == NULL, ERROR_MALLOC);
      if (value_it->is_array) {
        args_array = *(gr_meta_args_t ***)value_it->value_ptr;
      } else {
        args_array = (gr_meta_args_t **)value_it->value_ptr;
      }
      if (merge_value_it->is_array) {
        merge_args_array = *(gr_meta_args_t ***)merge_value_it->value_ptr;
      } else {
        merge_args_array = (gr_meta_args_t **)merge_value_it->value_ptr;
      }
      for (i = 0; i < value_it->array_length && i < merge_value_it->array_length; ++i) {
        error = args_merge(args_array[i], merge_args_array[i], merge_keys);
        cleanup_if(error != NO_ERROR);
      }
    } else {
      ++(update_arg->priv->reference_count);
      args_node = malloc(sizeof(args_node_t));
      if (args_node == NULL) {
        debug_print_malloc_error();
        error = ERROR_MALLOC;
        goto cleanup;
      }
      args_node->arg = update_arg;
      args_node->next = NULL;

      if (args->kwargs_head == NULL) {
        args->kwargs_head = args_node;
        args->kwargs_tail = args_node;
        ++(args->count);
      } else if (args_find_previous_node(args, update_arg->key, &previous_node_by_keyword)) {
        if (previous_node_by_keyword == NULL) {
          args_node->next = args->kwargs_head->next;
          if (args->kwargs_head == args->kwargs_tail) {
            args->kwargs_tail = args_node;
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
        ++(args->count);
      }
    }
  }

cleanup:
  if (it != NULL) {
    args_iterator_delete(it);
  }
  if (value_it != NULL) {
    args_value_iterator_delete(value_it);
  }
  if (merge_value_it != NULL) {
    args_value_iterator_delete(merge_value_it);
  }

  return error;
}

error_t args_setdefault_common(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                               va_list *vl, int apply_padding) {
  if (!gr_meta_args_contains(args, key)) {
    return args_push_common(args, key, value_format, buffer, vl, apply_padding);
  }
  return NO_ERROR;
}

error_t args_setdefault(gr_meta_args_t *args, const char *key, const char *value_format, ...) {
  error_t error;
  va_list vl;
  va_start(vl, value_format);

  error = args_setdefault_vl(args, key, value_format, &vl);

  va_end(vl);

  return error;
}

error_t args_setdefault_buf(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                            int apply_padding) {
  return args_setdefault_common(args, key, value_format, buffer, NULL, apply_padding);
}

error_t args_setdefault_vl(gr_meta_args_t *args, const char *key, const char *value_format, va_list *vl) {
  return args_setdefault_common(args, key, value_format, NULL, vl, 0);
}

unsigned int args_count(const gr_meta_args_t *args) {
  return args->count;
}

arg_t *args_at(const gr_meta_args_t *args, const char *keyword) {
  args_node_t *current_node;

  current_node = args_find_node(args, keyword);

  if (current_node != NULL) {
    return current_node->arg;
  }
  return NULL;
}

int(args_first_value)(const gr_meta_args_t *args, const char *keyword, const char *first_value_format,
                      void *first_value, unsigned int *array_length) {
  arg_t *arg;
  char *transformed_first_value_format;
  char first_value_type;
  size_t *size_t_typed_value_ptr;
  void *value_ptr;

  arg = args_at(args, keyword);
  if (arg == NULL) {
    return 0;
  }
  transformed_first_value_format = malloc(2 * strlen(first_value_format) + 1);
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
  first_value_type = (arg->value_format[0] != 'n') ? arg->value_format[0] : arg->value_format[1];
  if (islower(first_value_type)) {
    value_ptr = arg->value_ptr;
  } else {
    size_t_typed_value_ptr = arg->value_ptr;
    if (array_length != NULL) {
      *array_length = *size_t_typed_value_ptr;
    }
    value_ptr = (size_t_typed_value_ptr + 1);
  }
  if (first_value != NULL) {
    if (isupper(first_value_type)) {
      /* if the first value is an array simple store the pointer; the type the pointer is pointing to is unimportant
       * in this case so use a void pointer conversion to shorten the code */
      *(void **)first_value = *(void **)value_ptr;
    } else {
      switch (first_value_type) {
      case 'i':
        *(int *)first_value = *(int *)value_ptr;
        break;
      case 'd':
        *(double *)first_value = *(double *)value_ptr;
        break;
      case 'c':
        *(char *)first_value = *(char *)value_ptr;
        break;
      case 's':
        *(char **)first_value = *(char **)value_ptr;
        break;
      case 'a':
        *(gr_meta_args_t **)first_value = *(gr_meta_args_t **)value_ptr;
        break;
      default:
        return 0;
      }
    }
  }
  return 1;
}

int args_values(const gr_meta_args_t *args, const char *keyword, const char *expected_format, ...) {
  va_list vl;
  arg_t *arg;
  args_value_iterator_t *value_it = NULL;
  const char *current_va_format;
  int formats_are_equal = 0;
  int data_offset = 0;
  int was_successful = 0;

  va_start(vl, expected_format);

  arg = args_at(args, keyword);
  if (arg == NULL) {
    goto cleanup;
  }
  if (!(formats_are_equal = args_check_format_compatibility(arg, expected_format))) {
    goto cleanup;
  }
  formats_are_equal = (formats_are_equal == 2);

  current_va_format = expected_format;
  value_it = arg_value_iter(arg);
  if (value_it->next(value_it) == NULL) {
    goto cleanup;
  }
  while (*current_va_format != '\0') {
    void *current_value_ptr;
    current_value_ptr = va_arg(vl, void *);
    if (value_it->is_array && isupper(*current_va_format)) {
      /* If an array is stored and an array format is given by the user, simply assign a pointer to the data. The
       * datatype itself is unimportant in this case. */
      *(void **)current_value_ptr = *(void **)value_it->value_ptr;
    } else {
      switch (value_it->format) {
      case 'i':
        if (value_it->is_array) {
          /* The data is stored as an array but the user wants to assign the values to single variables (the array case
           * was handled before by the void pointer). -> Assign value by value by incrementing a data offset in each
           * step. */
          *(int *)current_value_ptr = (*(int **)value_it->value_ptr)[data_offset++];
        } else {
          /* The data is stored as a single value. Assign that value to the variable given by the user. */
          *(int *)current_value_ptr = *(int *)value_it->value_ptr;
        }
        break;
      case 'd':
        if (value_it->is_array) {
          *(double *)current_value_ptr = (*(double **)value_it->value_ptr)[data_offset++];
        } else {
          *(double *)current_value_ptr = *(double *)value_it->value_ptr;
        }
        break;
      case 'c':
        if (value_it->is_array) {
          *(char *)current_value_ptr = (*(char **)value_it->value_ptr)[data_offset++];
        } else {
          *(char *)current_value_ptr = *(char *)value_it->value_ptr;
        }
        break;
      case 's':
        if (value_it->is_array) {
          *(char **)current_value_ptr = (*(char ***)value_it->value_ptr)[data_offset++];
        } else {
          *(char **)current_value_ptr = *(char **)value_it->value_ptr;
        }
        break;
      case 'a':
        if (value_it->is_array) {
          *(gr_meta_args_t **)current_value_ptr = (*(gr_meta_args_t ***)value_it->value_ptr)[data_offset++];
        } else {
          *(gr_meta_args_t **)current_value_ptr = *(gr_meta_args_t **)value_it->value_ptr;
        }
        break;
      default:
        goto cleanup;
      }
    }
    if (formats_are_equal) {
      /* Only iterate if the format given by the user is equal to the stored internal format. In the other case, the
       * user requests to store **one** array to single variables -> values are read from one pointer, no iteration is
       * needed. */
      value_it->next(value_it);
      data_offset = 0;
    }
    ++current_va_format;
  }
  was_successful = 1;

cleanup:
  if (value_it != NULL) {
    args_value_iterator_delete(value_it);
  }
  va_end(vl);

  return was_successful;
}

args_node_t *args_find_node(const gr_meta_args_t *args, const char *keyword) {
  args_node_t *current_node;

  current_node = args->kwargs_head;
  while (current_node != NULL && strcmp(current_node->arg->key, keyword) != 0) {
    current_node = current_node->next;
  }

  return current_node;
}

int args_find_previous_node(const gr_meta_args_t *args, const char *keyword, args_node_t **previous_node) {
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
      STEP_VALUE_BUFFER_BY_TYPE('a', gr_meta_args_t *)
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


/* ------------------------- args / dynamic args array stack -------------------------------------------------------- */

#define DEFINE_STACK_METHODS(prefix, type, free_elem_func)                         \
  prefix##_stack_t *prefix##_stack_new(void) {                                     \
    prefix##_stack_t *prefix##_stack;                                              \
                                                                                   \
    prefix##_stack = malloc(sizeof(prefix##_stack_t));                             \
    if (prefix##_stack == NULL) {                                                  \
      return NULL;                                                                 \
    }                                                                              \
    prefix##_stack->head = NULL;                                                   \
                                                                                   \
    return prefix##_stack;                                                         \
  }                                                                                \
                                                                                   \
  void prefix##_stack_delete(prefix##_stack_t *prefix##_stack) {                   \
    prefix##_stack_node_t *current_node = prefix##_stack->head, *next_node = NULL; \
                                                                                   \
    while (current_node != NULL) {                                                 \
      next_node = current_node->next;                                              \
      free(current_node);                                                          \
      current_node = next_node;                                                    \
    }                                                                              \
    free(prefix##_stack);                                                          \
  }                                                                                \
                                                                                   \
  void prefix##_stack_delete_with_elements(prefix##_stack_t *prefix##_stack) {     \
    prefix##_stack_node_t *current_node = prefix##_stack->head, *next_node = NULL; \
                                                                                   \
    while (current_node != NULL) {                                                 \
      next_node = current_node->next;                                              \
      free_elem_func(next_node->value);                                            \
      free(current_node);                                                          \
      current_node = next_node;                                                    \
    }                                                                              \
    free(prefix##_stack);                                                          \
  }                                                                                \
                                                                                   \
  error_t prefix##_stack_push(prefix##_stack_t *prefix##_stack, type value) {      \
    prefix##_stack_node_t *prefix##_stack_node;                                    \
                                                                                   \
    if ((prefix##_stack_node = malloc(sizeof(prefix##_stack_node_t))) == NULL) {   \
      return ERROR_MALLOC;                                                         \
    }                                                                              \
    prefix##_stack_node->value = value;                                            \
    prefix##_stack_node->next = prefix##_stack->head;                              \
    prefix##_stack->head = prefix##_stack_node;                                    \
                                                                                   \
    return NO_ERROR;                                                               \
  }                                                                                \
                                                                                   \
  type prefix##_stack_pop(prefix##_stack_t *prefix##_stack) {                      \
    prefix##_stack_node_t *head_node;                                              \
    type value = NULL;                                                             \
                                                                                   \
    head_node = prefix##_stack->head;                                              \
    if (head_node != NULL) {                                                       \
      value = head_node->value;                                                    \
      prefix##_stack->head = head_node->next;                                      \
      free(head_node);                                                             \
    }                                                                              \
                                                                                   \
    return value;                                                                  \
  }                                                                                \
                                                                                   \
  int prefix##_stack_empty(prefix##_stack_t *prefix##_stack) { return (prefix##_stack->head == NULL); }

DEFINE_STACK_METHODS(args, gr_meta_args_t *, gr_deletemeta)
DEFINE_STACK_METHODS(dynamic_args_array, dynamic_args_array_t *, dynamic_args_array_delete)
DEFINE_STACK_METHODS(string, const char *, free)

#undef DEFINE_STACK_METHODS


/* ------------------------- dynamic args array --------------------------------------------------------------------- */

dynamic_args_array_t *dynamic_args_array_new(void) {
  dynamic_args_array_t *args_array;

  args_array = malloc(sizeof(dynamic_args_array_t));
  if (args_array == NULL) {
    return NULL;
  }
  args_array->buf = malloc(DYNAMIC_ARGS_ARRAY_INITIAL_SIZE * sizeof(gr_meta_args_t *));
  if (args_array->buf == NULL) {
    free(args_array);
    return NULL;
  }
  args_array->capacity = DYNAMIC_ARGS_ARRAY_INITIAL_SIZE;
  args_array->size = 0;

  return args_array;
}

void dynamic_args_array_delete(dynamic_args_array_t *args_array) {
  free(args_array->buf);
  free(args_array);
}

void dynamic_args_array_delete_with_elements(dynamic_args_array_t *args_array) {
  size_t i;
  for (i = 0; i < args_array->size; ++i) {
    gr_deletemeta(args_array->buf[i]);
  }
  dynamic_args_array_delete(args_array);
}

error_t dynamic_args_array_push_back(dynamic_args_array_t *args_array, gr_meta_args_t *args) {
  if (args_array->size == args_array->capacity) {
    gr_meta_args_t **enlarged_buf =
      realloc(args_array->buf, (args_array->capacity + DYNAMIC_ARGS_ARRAY_SIZE_INCREMENT) * sizeof(gr_meta_args_t *));
    if (enlarged_buf == NULL) {
      return ERROR_MALLOC;
    }
    args_array->buf = enlarged_buf;
    args_array->capacity += DYNAMIC_ARGS_ARRAY_SIZE_INCREMENT;
  }
  args_array->buf[args_array->size] = args;
  ++args_array->size;

  return NO_ERROR;
}


/* ------------------------- json deserializer ---------------------------------------------------------------------- */

int gr_readmeta(gr_meta_args_t *args, const char *json_string) {
  return (fromjson_read(args, json_string) == NO_ERROR);
}

static error_t fromjson_read(gr_meta_args_t *args, const char *json_string) {
  return fromjson_parse(args, json_string, NULL);
}

error_t fromjson_parse(gr_meta_args_t *args, const char *json_string, fromjson_shared_state_t *shared_state) {
  char *filtered_json_string = NULL;
  fromjson_state_t state;
  int allocated_shared_state_mem = 0;
  error_t error = NO_ERROR;

  state.datatype = JSON_DATATYPE_UNKNOWN;
  state.value_buffer = NULL;
  state.value_buffer_pointer_level = 0;
  state.next_value_memory = NULL;
  state.next_value_type = malloc(NEXT_VALUE_TYPE_SIZE);
  if (state.next_value_type == NULL) {
    debug_print_malloc_error();
    return ERROR_MALLOC;
  }
  state.args = args;
  if (shared_state == NULL) {
    shared_state = malloc(sizeof(fromjson_shared_state_t));
    if (shared_state == NULL) {
      free(state.next_value_type);
      debug_print_malloc_error();
      return ERROR_MALLOC;
    }
    if ((error = fromjson_copy_and_filter_json_string(&filtered_json_string, json_string)) != NO_ERROR) {
      free(state.next_value_type);
      free(shared_state);
      return error;
    }
    shared_state->json_ptr = filtered_json_string;
    shared_state->parsed_any_value_before = 0;
    allocated_shared_state_mem = 1;
  }
  state.shared_state = shared_state;
  state.parsing_object = (*state.shared_state->json_ptr == '{');
  if (state.parsing_object) {
    ++state.shared_state->json_ptr;
  }

  while (strchr("}", *state.shared_state->json_ptr) == NULL) {
    const char *current_key = NULL;

    if (state.parsing_object) {
      fromjson_parse_string(&state);
      current_key = *(const char **)state.value_buffer;
      free(state.value_buffer);
      state.value_buffer = NULL;
      ++(state.shared_state->json_ptr);
    }
    state.datatype = fromjson_check_type(&state);
    if (state.datatype) {
      if ((error = fromjson_datatype_to_func[state.datatype](&state)) != NO_ERROR) {
        break;
      }
      if (state.parsing_object) {
        gr_meta_args_push_buf(args, current_key, state.next_value_type, state.value_buffer, 0);
      } else {
        /* parsing values without an outer object (-> missing key) is not supported by the argument container */
        error = ERROR_PARSE_MISSING_OBJECT_CONTAINER;
        break;
      }
      if (strchr(FROMJSON_VALID_DELIMITERS, *state.shared_state->json_ptr) != NULL) {
        if (*state.shared_state->json_ptr == ',') {
          ++state.shared_state->json_ptr;
        }
      } else {
        error = ERROR_PARSE_INVALID_DELIMITER;
        break;
      }
    } else {
      error = ERROR_PARSE_UNKNOWN_DATATYPE;
      break;
    }
    if (state.value_buffer_pointer_level > 1) {
      int i, outer_array_length = uppercase_count(state.next_value_type);
      for (i = 0; i < outer_array_length; ++i) {
        free(((char **)state.value_buffer)[i]);
      }
    }
    free(state.value_buffer);
    state.value_buffer = NULL;
    state.value_buffer_pointer_level = 0;
  }
  if (state.parsing_object && *state.shared_state->json_ptr == '\0') {
    error = ERROR_PARSE_INCOMPLETE_STRING;
  }
  if (*state.shared_state->json_ptr) {
    ++state.shared_state->json_ptr;
  }

  free(state.value_buffer);
  free(filtered_json_string);
  free(state.next_value_type);

  if (allocated_shared_state_mem) {
    free(shared_state);
  }

  return error;
}

#define CHECK_AND_ALLOCATE_MEMORY(type, length)            \
  do {                                                     \
    if (state->value_buffer == NULL) {                     \
      state->value_buffer = malloc(length * sizeof(type)); \
      if (state->value_buffer == NULL) {                   \
        debug_print_malloc_error();                        \
        return 0;                                          \
      }                                                    \
      state->value_buffer_pointer_level = 1;               \
      state->next_value_memory = state->value_buffer;      \
    }                                                      \
  } while (0)

error_t fromjson_parse_null(fromjson_state_t *state) {
  if (strncmp(state->shared_state->json_ptr, "null", 4) != 0) {
    return ERROR_PARSE_NULL;
  }
  strcpy(state->next_value_type, "");
  state->shared_state->json_ptr += 4;
  return NO_ERROR;
}

error_t fromjson_parse_bool(fromjson_state_t *state) {
  int bool_value;

  if (strncmp(state->shared_state->json_ptr, "true", 4) == 0) {
    bool_value = 1;
  } else if (strncmp(state->shared_state->json_ptr, "false", 5) == 0) {
    bool_value = 0;
  } else {
    return ERROR_PARSE_BOOL;
  }
  CHECK_AND_ALLOCATE_MEMORY(int, 1);
  *((int *)state->next_value_memory) = bool_value;
  strcpy(state->next_value_type, "i");
  state->shared_state->json_ptr += bool_value ? 4 : 5;
  return NO_ERROR;
}

error_t fromjson_parse_number(fromjson_state_t *state) {
  error_t error;

  if (is_int_number(state->shared_state->json_ptr)) {
    error = fromjson_parse_int(state);
  } else {
    error = fromjson_parse_double(state);
  }
  return error;
}

error_t fromjson_parse_int(fromjson_state_t *state) {
  int was_successful;
  int int_value;

  int_value = fromjson_str_to_int((const char **)&state->shared_state->json_ptr, &was_successful);
  if (!was_successful) {
    return ERROR_PARSE_INT;
  }
  CHECK_AND_ALLOCATE_MEMORY(int, 1);
  *((int *)state->next_value_memory) = int_value;
  strcpy(state->next_value_type, "i");
  return NO_ERROR;
}

error_t fromjson_parse_double(fromjson_state_t *state) {
  int was_successful;
  double double_value;

  double_value = fromjson_str_to_double((const char **)&state->shared_state->json_ptr, &was_successful);
  if (!was_successful) {
    return ERROR_PARSE_DOUBLE;
  }
  CHECK_AND_ALLOCATE_MEMORY(double, 1);
  *((double *)state->next_value_memory) = double_value;
  strcpy(state->next_value_type, "d");
  return NO_ERROR;
}

error_t fromjson_parse_string(fromjson_state_t *state) {
  char *string_value;
  char *json_ptr;
  const char *src_ptr;
  char *dest_ptr;
  int string_is_complete;
  int skipped_char;

  CHECK_AND_ALLOCATE_MEMORY(char *, 1);
  json_ptr = state->shared_state->json_ptr;
  string_value = ++json_ptr;
  while (*json_ptr && !is_string_delimiter(json_ptr, string_value)) {
    ++json_ptr;
  }
  string_is_complete = (*json_ptr != '\0');
  *json_ptr = '\0';
  /* Unescape '"' and '\' (since '\' is the escape character) */
  src_ptr = dest_ptr = string_value;
  skipped_char = 0;
  while (*src_ptr) {
    if (*src_ptr == '\\' && !skipped_char) {
      ++src_ptr;
      skipped_char = 1;
    } else {
      *dest_ptr = *src_ptr;
      ++src_ptr;
      ++dest_ptr;
      skipped_char = 0;
    }
  }
  *dest_ptr = '\0';

  *((const char **)state->next_value_memory) = string_value;
  strcpy(state->next_value_type, "s");
  ++json_ptr;
  state->shared_state->json_ptr = json_ptr;

  return string_is_complete ? NO_ERROR : ERROR_PARSE_STRING;
}

error_t fromjson_parse_array(fromjson_state_t *state) {
  fromjson_datatype_t json_datatype;
  const char *next_delim_ptr;

#define PARSE_VALUES(parse_suffix, c_type)                                                                       \
  do {                                                                                                           \
    c_type *values;                                                                                              \
    c_type *current_value_ptr;                                                                                   \
    CHECK_AND_ALLOCATE_MEMORY(c_type *, 1);                                                                      \
    values = malloc(array_length * sizeof(c_type));                                                              \
    if (values == NULL) {                                                                                        \
      debug_print_malloc_error();                                                                                \
      return ERROR_MALLOC;                                                                                       \
    }                                                                                                            \
    current_value_ptr = values;                                                                                  \
    *(c_type **)state->next_value_memory = values;                                                               \
    state->value_buffer_pointer_level = 2;                                                                       \
    state->next_value_memory = values;                                                                           \
    --state->shared_state->json_ptr;                                                                             \
    while (!error && strchr("]", *state->shared_state->json_ptr) == NULL) {                                      \
      ++state->shared_state->json_ptr;                                                                           \
      error = fromjson_parse_##parse_suffix(state);                                                              \
      ++current_value_ptr;                                                                                       \
      state->next_value_memory = current_value_ptr;                                                              \
    }                                                                                                            \
    snprintf(array_type + strlen(array_type), NEXT_VALUE_TYPE_SIZE, "%c(%lu)", toupper(*state->next_value_type), \
             array_length);                                                                                      \
  } while (0)

  if (strchr("]", *(state->shared_state->json_ptr + 1)) == NULL) {
    char array_type[NEXT_VALUE_TYPE_SIZE];
    size_t outer_array_length = 0;
    int is_nested_array = (*(state->shared_state->json_ptr + 1) == '[');
    unsigned int current_outer_array_index = 0;
    if (is_nested_array) {
      outer_array_length = fromjson_get_outer_array_length(state->shared_state->json_ptr);
      /* `char *` is only used as a generic type since all pointers to values have the same storage size */
      CHECK_AND_ALLOCATE_MEMORY(char *, outer_array_length);
    }
    array_type[0] = '\0';
    do {
      error_t error = NO_ERROR;
      size_t array_length = 0;
      state->shared_state->json_ptr += is_nested_array ? 2 : 1;
      next_delim_ptr = state->shared_state->json_ptr;
      while (*next_delim_ptr != ']' &&
             fromjson_find_next_delimiter(&next_delim_ptr, next_delim_ptr, array_length == 0, 1)) {
        ++array_length;
      }
      if (*next_delim_ptr != ']') {
        state->shared_state->json_ptr -= is_nested_array ? 2 : 1;
        return ERROR_PARSE_INCOMPLETE_STRING;
      }
      assert(array_length > 0);
      json_datatype = fromjson_check_type(state);
      switch (json_datatype) {
      case JSON_DATATYPE_NUMBER:
        if (is_int_number(state->shared_state->json_ptr)) {
          PARSE_VALUES(int, int);
        } else {
          PARSE_VALUES(double, double);
        }
        break;
      case JSON_DATATYPE_STRING:
        PARSE_VALUES(string, char *);
        break;
      case JSON_DATATYPE_OBJECT:
        PARSE_VALUES(object, gr_meta_args_t *);
        break;
      case JSON_DATATYPE_ARRAY:
        debug_print_error(("Arrays only support one level of nesting!\n"));
        state->shared_state->json_ptr -= is_nested_array ? 2 : 1;
        return ERROR_PARSE_ARRAY;
        break;
      default:
        debug_print_error(
          ("The datatype \"%s\" is currently not supported in arrays!\n", fromjson_datatype_to_string[json_datatype]));
        state->shared_state->json_ptr -= is_nested_array ? 2 : 1;
        return ERROR_PARSE_ARRAY;
        break;
      }
      ++(state->shared_state->json_ptr);
      if (is_nested_array) {
        ++current_outer_array_index;
        /* `char **` is only used as a generic type since all pointers to values have the same storage size */
        state->next_value_memory = ((char **)state->value_buffer) + current_outer_array_index;
      }
    } while (is_nested_array && current_outer_array_index < outer_array_length &&
             strchr("]", *state->shared_state->json_ptr) == NULL);
    if (is_nested_array && *state->shared_state->json_ptr == ']') {
      ++state->shared_state->json_ptr;
    }
    strcpy(state->next_value_type, array_type);
  } else {
    strcpy(state->next_value_type, "I(0)");
  }

  return NO_ERROR;

#undef PARSE_VALUES
}

error_t fromjson_parse_object(fromjson_state_t *state) {
  gr_meta_args_t *args;
  error_t error;

  CHECK_AND_ALLOCATE_MEMORY(gr_meta_args_t *, 1);
  args = gr_newmeta();
  error = fromjson_parse(args, state->shared_state->json_ptr, state->shared_state);
  *((gr_meta_args_t **)state->next_value_memory) = args;
  strcpy(state->next_value_type, "a");
  return error;
}

#undef CHECK_AND_ALLOCATE_MEMORY

fromjson_datatype_t fromjson_check_type(const fromjson_state_t *state) {
  fromjson_datatype_t datatype;

  datatype = JSON_DATATYPE_UNKNOWN;
  switch (*state->shared_state->json_ptr) {
  case '"':
    datatype = JSON_DATATYPE_STRING;
    break;
  case '[':
    datatype = JSON_DATATYPE_ARRAY;
    break;
  case '{':
    datatype = JSON_DATATYPE_OBJECT;
    break;
  default:
    break;
  }
  if (datatype == JSON_DATATYPE_UNKNOWN) {
    if (*state->shared_state->json_ptr == 'n') {
      datatype = JSON_DATATYPE_NULL;
    } else if (strchr("ft", *state->shared_state->json_ptr) != NULL) {
      datatype = JSON_DATATYPE_BOOL;
    } else {
      datatype = JSON_DATATYPE_NUMBER;
    }
  }
  return datatype;
}

error_t fromjson_copy_and_filter_json_string(char **dest, const char *src) {
  const char *src_ptr;
  char *dest_buffer, *dest_ptr;
  int in_string;

  src_ptr = src;
  dest_buffer = malloc(strlen(src) + 1);
  if (dest_buffer == NULL) {
    debug_print_malloc_error();
    return ERROR_MALLOC;
  }
  dest_ptr = dest_buffer;

  in_string = 0;
  while (*src_ptr) {
    if (is_string_delimiter(src_ptr, src)) {
      in_string = !in_string;
    }
    if (in_string || !isspace(*src_ptr)) {
      *dest_ptr = *src_ptr;
      ++dest_ptr;
    }
    ++src_ptr;
  }
  *dest_ptr = '\0';

  *dest = dest_buffer;

  return NO_ERROR;
}

int fromjson_find_next_delimiter(const char **delim_ptr, const char *src, int include_start,
                                 int exclude_nested_structures) {
  if (*src == '\0') {
    return 0;
  }
  if (!include_start) {
    ++src;
  }
  if (exclude_nested_structures) {
    const char *src_ptr = src;
    int nested_level = 0;

    while (*src_ptr) {
      if (strchr("[{", *src_ptr)) {
        ++nested_level;
      } else if (strchr("]}", *src_ptr)) {
        if (nested_level > 0) {
          --nested_level;
        } else {
          break;
        }
      } else if (*src_ptr == ',' && nested_level == 0) {
        break;
      }
      ++src_ptr;
    }
    if (*src_ptr != '\0') {
      *delim_ptr = src_ptr;
      return 1;
    }
  } else {
    size_t segment_length = strcspn(src, FROMJSON_VALID_DELIMITERS);
    if (*(src + segment_length) != '\0') {
      *delim_ptr = src + segment_length;
      return 1;
    }
  }
  return 0;
}

size_t fromjson_get_outer_array_length(const char *str) {
  size_t outer_array_length = 0;
  int current_array_level = 1;

  if (*str != '[') {
    return outer_array_length;
  }
  ++str;
  while (current_array_level > 0 && *str) {
    switch (*str) {
    case '[':
      ++current_array_level;
      break;
    case ']':
      --current_array_level;
      break;
    case ',':
      if (current_array_level == 1) {
        ++outer_array_length;
      }
      break;
    default:
      break;
    }
    ++str;
  }
  ++outer_array_length;
  return outer_array_length;
}

double fromjson_str_to_double(const char **str, int *was_successful) {
  char *conversion_end = NULL;
  double conversion_result;
  int success = 0;
  const char *next_delim_ptr = NULL;

  errno = 0;
  if (*str != NULL) {
    conversion_result = strtod(*str, &conversion_end);
  } else {
    conversion_result = 0.0;
  }
  if (conversion_end == NULL) {
    debug_print_error(("No number conversion was executed (the string is NULL)!\n"));
  } else if (*str == conversion_end || strchr(FROMJSON_VALID_DELIMITERS, *conversion_end) == NULL) {
    fromjson_find_next_delimiter(&next_delim_ptr, *str, 1, 0);
    debug_print_error(("The parameter \"%.*s\" is not a valid number!\n", next_delim_ptr - *str, *str));
  } else if (errno == ERANGE) {
    fromjson_find_next_delimiter(&next_delim_ptr, *str, 1, 0);
    if (conversion_result == HUGE_VAL || conversion_result == -HUGE_VAL) {
      debug_print_error(("The parameter \"%.*s\" caused an overflow, the number has been clamped to \"%lf\"\n",
                         next_delim_ptr - *str, *str, conversion_result));
    } else {
      debug_print_error(("The parameter \"%.*s\" caused an underflow, the number has been clamped to \"%lf\"\n",
                         next_delim_ptr - *str, *str, conversion_result));
    }
  } else {
    success = 1;
    *str = conversion_end;
  }
  if (was_successful != NULL) {
    *was_successful = success;
  }

  return conversion_result;
}

int fromjson_str_to_int(const char **str, int *was_successful) {
  char *conversion_end = NULL;
  long conversion_result;
  int success = 0;
  const char *next_delim_ptr = NULL;

  errno = 0;
  if (*str != NULL) {
    conversion_result = strtol(*str, &conversion_end, 10);
  } else {
    conversion_result = 0;
  }
  if (conversion_end == NULL) {
    debug_print_error(("No number conversion was executed (the string is NULL)!\n"));
  } else if (*str == conversion_end || strchr(FROMJSON_VALID_DELIMITERS, *conversion_end) == NULL) {
    fromjson_find_next_delimiter(&next_delim_ptr, *str, 1, 0);
    debug_print_error(("The parameter \"%.*s\" is not a valid number!\n", next_delim_ptr - *str, *str));
  } else if (errno == ERANGE || conversion_result > INT_MAX || conversion_result < INT_MIN) {
    fromjson_find_next_delimiter(&next_delim_ptr, *str, 1, 0);
    if (conversion_result > INT_MAX) {
      debug_print_error(("The parameter \"%.*s\" is too big, the number has been clamped to \"%d\"\n",
                         next_delim_ptr - *str, *str, INT_MAX));
      conversion_result = INT_MAX;
    } else {
      debug_print_error(("The parameter \"%.*s\" is too small, the number has been clamped to \"%d\"\n",
                         next_delim_ptr - *str, *str, INT_MIN));
      conversion_result = INT_MIN;
    }
  } else {
    success = 1;
    *str = conversion_end;
  }
  if (was_successful != NULL) {
    *was_successful = success;
  }

  return (int)conversion_result;
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

#define INIT_MULTI_VALUE(vars, type)             \
  do {                                           \
    if (state->shared->data_ptr != NULL) {       \
      CHECK_PADDING(type *);                     \
      vars = *(type **)state->shared->data_ptr;  \
    } else {                                     \
      vars = va_arg(*state->shared->vl, type *); \
    }                                            \
  } while (0)

#define FIN_MULTI_VALUE(type)                                           \
  do {                                                                  \
    if (state->shared->data_ptr != NULL) {                              \
      state->shared->data_ptr = ((type **)state->shared->data_ptr) + 1; \
      state->shared->data_offset += sizeof(type *);                     \
    }                                                                   \
  } while (0)

#define DEFINE_STRINGIFY_VALUE(name, type, format_specifier)                    \
  error_t tojson_stringify_##name##_value(memwriter_t *memwriter, type value) { \
    return memwriter_printf(memwriter, format_specifier, value);                \
  }

#define DEFINE_STRINGIFY_SINGLE(name, type, promoted_type)                                \
  error_t tojson_stringify_##name(tojson_state_t *state) {                                \
    type value;                                                                           \
    error_t error = NO_ERROR;                                                             \
    RETRIEVE_SINGLE_VALUE(value, type, promoted_type);                                    \
    if ((error = tojson_stringify_##name##_value(state->memwriter, value)) != NO_ERROR) { \
      return error;                                                                       \
    }                                                                                     \
    state->shared->wrote_output = 1;                                                      \
    return error;                                                                         \
  }

#define DEFINE_STRINGIFY_MULTI(name, type)                                                                          \
  error_t tojson_stringify_##name##_array(tojson_state_t *state) {                                                  \
    type *values;                                                                                                   \
    type current_value;                                                                                             \
    int length;                                                                                                     \
    int remaining_elements;                                                                                         \
    error_t error = NO_ERROR;                                                                                       \
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
    if ((error = memwriter_putc(state->memwriter, '[')) != NO_ERROR) {                                              \
      return error;                                                                                                 \
    }                                                                                                               \
    /* write array content */                                                                                       \
    while (remaining_elements) {                                                                                    \
      current_value = *values++;                                                                                    \
      if ((error = tojson_stringify_##name##_value(state->memwriter, current_value)) != NO_ERROR) {                 \
        return error;                                                                                               \
      }                                                                                                             \
      if (remaining_elements > 1) {                                                                                 \
        if ((error = memwriter_putc(state->memwriter, ',')) != NO_ERROR) {                                          \
          return error;                                                                                             \
        }                                                                                                           \
      }                                                                                                             \
      --remaining_elements;                                                                                         \
    }                                                                                                               \
    /* write array end */                                                                                           \
    if ((error = memwriter_putc(state->memwriter, ']')) != NO_ERROR) {                                              \
      return error;                                                                                                 \
    }                                                                                                               \
    FIN_MULTI_VALUE(type);                                                                                          \
    state->shared->wrote_output = 1;                                                                                \
    return error;                                                                                                   \
  }

DEFINE_STRINGIFY_SINGLE(int, int, int)
DEFINE_STRINGIFY_MULTI(int, int)
DEFINE_STRINGIFY_VALUE(int, int, "%d")
DEFINE_STRINGIFY_SINGLE(double, double, double)
DEFINE_STRINGIFY_MULTI(double, double)
DEFINE_STRINGIFY_SINGLE(char, char, int)
DEFINE_STRINGIFY_VALUE(char, char, "%c")
DEFINE_STRINGIFY_SINGLE(string, char *, char *)
DEFINE_STRINGIFY_MULTI(string, char *)
DEFINE_STRINGIFY_SINGLE(bool, int, int)
DEFINE_STRINGIFY_MULTI(bool, int)
DEFINE_STRINGIFY_SINGLE(args, gr_meta_args_t *, gr_meta_args_t *)
DEFINE_STRINGIFY_MULTI(args, gr_meta_args_t *)

#undef DEFINE_STRINGIFY_SINGLE
#undef DEFINE_STRINGIFY_MULTI
#undef DEFINE_STRINGIFY_VALUE

#define STR(x) #x
#define XSTR(x) STR(x)

error_t tojson_stringify_double_value(memwriter_t *memwriter, double value) {
  error_t error;
  size_t string_start_index;
  const char *unprocessed_string;

  string_start_index = memwriter_size(memwriter);
  if ((error = memwriter_printf(memwriter, "%." XSTR(DBL_DECIMAL_DIG) "g", value)) != NO_ERROR) {
    return error;
  }
  unprocessed_string = memwriter_buf(memwriter) + string_start_index;
  if (strspn(unprocessed_string, "0123456789-") == memwriter_size(memwriter) - string_start_index) {
    if ((error = memwriter_putc(memwriter, '.')) != NO_ERROR) {
      return error;
    }
  }
  return NO_ERROR;
}

#undef XSTR
#undef STR

error_t tojson_stringify_char_array(tojson_state_t *state) {
  char *chars;
  char *escaped_chars = NULL;
  unsigned int length;
  error_t error = NO_ERROR;

  INIT_MULTI_VALUE(chars, char);

  if (state->additional_type_info != NULL) {
    int was_successful;
    length = str_to_uint(state->additional_type_info, &was_successful);
    if (!was_successful) {
      debug_print_error(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                         state->additional_type_info));
      goto cleanup;
    }
  } else {
    if (state->shared->read_length_from_string) {
      length = 0;
    } else {
      length = state->shared->array_length;
    }
  }
  if ((error = tojson_escape_special_chars(&escaped_chars, chars, &length)) != NO_ERROR) {
    goto cleanup;
  }
  if ((error = memwriter_printf(state->memwriter, "\"%.*s\"", length, escaped_chars)) != NO_ERROR) {
    goto cleanup;
  }
  state->shared->wrote_output = 1;

  FIN_MULTI_VALUE(char);

cleanup:
  free(escaped_chars);
  return error;
}

error_t tojson_stringify_string_value(memwriter_t *memwriter, char *value) {
  char *escaped_chars = NULL;
  unsigned int length = 0;
  error_t error = NO_ERROR;

  if ((error = tojson_escape_special_chars(&escaped_chars, value, &length))) {
    goto cleanup;
  }
  if ((error = memwriter_printf(memwriter, "\"%s\"", escaped_chars)) != NO_ERROR) {
    goto cleanup;
  }

cleanup:
  free(escaped_chars);
  return error;
}

error_t tojson_stringify_bool_value(memwriter_t *memwriter, int value) {
  return memwriter_puts(memwriter, value ? "true" : "false");
}

error_t tojson_stringify_object(tojson_state_t *state) {
  char **member_names = NULL;
  char **data_types = NULL;
  char **member_name_ptr;
  char **data_type_ptr;
  int has_members;
  error_t error = NO_ERROR;

  /* IMPORTANT: additional_type_info is altered after the unzip call! */
  if ((error = tojson_unzip_membernames_and_datatypes(state->additional_type_info, &member_names, &data_types)) !=
      NO_ERROR) {
    goto cleanup;
  }
  member_name_ptr = member_names;
  data_type_ptr = data_types;

  has_members =
    (member_name_ptr != NULL && *member_name_ptr != NULL && data_type_ptr != NULL && *data_type_ptr != NULL);
  /* write object start */
  if (!state->add_data_without_separator) {
    if (state->shared->add_data && has_members) {
      if ((error = memwriter_putc(state->memwriter, ',')) != NO_ERROR) {
        goto cleanup;
      }
    } else if (!state->shared->add_data) {
      if ((error = memwriter_putc(state->memwriter, '{')) != NO_ERROR) {
        goto cleanup;
      }
      ++(state->shared->struct_nested_level);
    }
  }
  /* `add_data` is only relevant for the first object start; reset it to default afterwards since nested objects can
   * follow*/
  state->shared->add_data = 0;
  if (has_members) {
    /* write object content */
    int serialized_all_members = 0;
    while (!serialized_all_members) {
      if ((error = memwriter_printf(state->memwriter, "\"%s\":", *member_name_ptr)) != NO_ERROR) {
        goto cleanup;
      }
      if ((error = tojson_serialize(state->memwriter, *data_type_ptr, NULL, NULL, -1, -1, 0, NULL, NULL,
                                    state->shared)) != NO_ERROR) {
        goto cleanup;
      }
      ++member_name_ptr;
      ++data_type_ptr;
      if (*member_name_ptr != NULL && *data_type_ptr != NULL) {
        /* write JSON separator */
        if ((error = memwriter_putc(state->memwriter, ',')) != NO_ERROR) {
          goto cleanup;
        }
      } else {
        serialized_all_members = 1;
      }
    }
  }
  /* write object end if the type info is complete */
  if (!state->is_type_info_incomplete) {
    --(state->shared->struct_nested_level);
    if ((error = memwriter_putc(state->memwriter, '}')) != NO_ERROR) {
      goto cleanup;
    }
  }
  /* Only set serial result if not set before */
  if (state->shared->serial_result == 0 && state->is_type_info_incomplete) {
    state->shared->serial_result = has_members ? incomplete : incomplete_at_struct_beginning;
  }

cleanup:
  free(member_names);
  free(data_types);
  if (error != NO_ERROR) {
    return error;
  }

  state->shared->wrote_output = 1;

  return NO_ERROR;
}

error_t tojson_stringify_args_value(memwriter_t *memwriter, gr_meta_args_t *args) {
  error_t error = NO_ERROR;

  if ((error = memwriter_putc(memwriter, '{')) != NO_ERROR) {
    return error;
  }
  tojson_permanent_state.serial_result = incomplete_at_struct_beginning;
  if ((error = tojson_write_args(memwriter, args)) != NO_ERROR) {
    return error;
  }

  return NO_ERROR;
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
    if (*data_desc == '(') {
      ++nested_level;
    } else if (*data_desc == ')') {
      --nested_level;
    } else if (nested_level == 0 && strchr(relevant_data_types, *data_desc)) {
      ++count_relevant_data_types;
    }
    ++data_desc;
  }
  return count_relevant_data_types >= 2;
}

void tojson_read_datatype(tojson_state_t *state) {
  char *additional_type_info = NULL;
  state->current_data_type = *state->data_type_ptr;
  ++(state->data_type_ptr);
  if (*state->data_type_ptr == '(') {
    int nested_level = 1;
    additional_type_info = ++(state->data_type_ptr);
    while (*state->data_type_ptr != 0 && nested_level > 0) {
      if (*state->data_type_ptr == '(') {
        ++nested_level;
      } else if (*state->data_type_ptr == ')') {
        --nested_level;
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
}

error_t tojson_skip_bytes(tojson_state_t *state) {
  int count;

  if (state->shared->data_ptr == NULL) {
    debug_print_error(("Skipping bytes is not supported when using the variable argument list and is ignored.\n"));
    return NO_ERROR;
  }

  if (state->additional_type_info != NULL) {
    int was_successful;
    count = str_to_uint(state->additional_type_info, &was_successful);
    if (!was_successful) {
      debug_print_error(("Byte skipping with an invalid number -> ignoring.\n"));
      return NO_ERROR;
    }
  } else {
    count = 1;
  }
  state->shared->data_ptr = ((char *)state->shared->data_ptr) + count;
  state->shared->data_offset += count;

  return NO_ERROR;
}

error_t tojson_close_object(tojson_state_t *state) {
  error_t error;
  --(state->shared->struct_nested_level);
  if ((error = memwriter_putc(state->memwriter, '}')) != NO_ERROR) {
    return error;
  }
  return NO_ERROR;
}

error_t tojson_read_array_length(tojson_state_t *state) {
  int value;

  RETRIEVE_SINGLE_VALUE(value, size_t, size_t);
  state->shared->array_length = value;

  return NO_ERROR;
}

error_t tojson_unzip_membernames_and_datatypes(char *mixed_ptr, char ***member_name_ptr, char ***data_type_ptr) {
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
    return ERROR_MALLOC;
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
          if (*mixed_ptr == '(') {
            ++nested_type_level;
          } else if (*mixed_ptr == ')') {
            --nested_type_level;
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
  return NO_ERROR;
}

error_t tojson_escape_special_chars(char **escaped_string, const char *unescaped_string, unsigned int *length) {
  /* characters '\' and '"' must be escaped before written to a json string value */
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
  while (remaining_chars) {
    if (strchr(chars_to_escape, *src_ptr) != NULL) {
      ++needed_memory;
    }
    ++src_ptr;
    --remaining_chars;
  }
  dest_ptr = malloc(needed_memory);
  if (dest_ptr == NULL) {
    return ERROR_MALLOC;
  }
  *escaped_string = dest_ptr;
  src_ptr = unescaped_string;
  remaining_chars = len;
  while (remaining_chars) {
    if (strchr(chars_to_escape, *src_ptr) != NULL) {
      *dest_ptr++ = '\\';
    }
    *dest_ptr++ = *src_ptr++;
    --remaining_chars;
  }
  *dest_ptr = '\0';
  if (length != NULL) {
    *length = needed_memory - 1;
  }

  return NO_ERROR;
}

#undef CHECK_PADDING
#undef RETRIEVE_SINGLE_VALUE
#undef INIT_MULTI_VALUE
#undef FIN_MULTI_VALUE

error_t tojson_serialize(memwriter_t *memwriter, char *data_desc, const void *data, va_list *vl, int apply_padding,
                         int add_data, int add_data_without_separator, unsigned int *struct_nested_level,
                         tojson_serialization_result_t *serial_result, tojson_shared_state_t *shared_state) {
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

  tojson_state_t state;
  int json_array_needed = 0;
  int allocated_shared_state_mem = 0;
  error_t error = NO_ERROR;

  state.memwriter = memwriter;
  state.data_type_ptr = data_desc;
  state.current_data_type = 0;
  state.additional_type_info = NULL;
  state.add_data_without_separator = add_data_without_separator;
  state.is_type_info_incomplete = 0;
  if (shared_state == NULL) {
    shared_state = malloc(sizeof(tojson_shared_state_t));
    if (shared_state == NULL) {
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
    if ((error = memwriter_putc(memwriter, '[')) != NO_ERROR) {
      goto cleanup;
    }
  }
  while (*state.data_type_ptr != 0) {
    shared_state->wrote_output = 0;
    tojson_read_datatype(&state);
    if (tojson_datatype_to_func[state.current_data_type]) {
      error = tojson_datatype_to_func[state.current_data_type](&state);
    } else {
      debug_print_error(("WARNING: '%c' (ASCII code %d) is not a valid type identifier\n", state.current_data_type,
                         state.current_data_type));
      error = ERROR_UNSUPPORTED_DATATYPE;
    }
    if (error != NO_ERROR) {
      goto cleanup;
    }
    if (*state.data_type_ptr != 0 && *state.data_type_ptr != ')' && shared_state->wrote_output) {
      /* write JSON separator, if data was written and the object is not closed in the next step */
      if ((error = memwriter_putc(memwriter, ',')) != NO_ERROR) {
        goto cleanup;
      }
    }
  }
  /* write list tail if needed */
  if (json_array_needed) {
    if ((error = memwriter_putc(memwriter, ']')) != NO_ERROR) {
      goto cleanup;
    }
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

cleanup:
  if (allocated_shared_state_mem) {
    free(shared_state);
  }

  return error;
}

void tojson_init_static_variables(void) {
  if (!tojson_static_variables_initialized) {
    tojson_datatype_to_func['n'] = tojson_read_array_length;
    tojson_datatype_to_func['e'] = tojson_skip_bytes;
    tojson_datatype_to_func['i'] = tojson_stringify_int;
    tojson_datatype_to_func['I'] = tojson_stringify_int_array;
    tojson_datatype_to_func['d'] = tojson_stringify_double;
    tojson_datatype_to_func['D'] = tojson_stringify_double_array;
    tojson_datatype_to_func['c'] = tojson_stringify_char;
    tojson_datatype_to_func['C'] = tojson_stringify_char_array;
    tojson_datatype_to_func['s'] = tojson_stringify_string;
    tojson_datatype_to_func['S'] = tojson_stringify_string_array;
    tojson_datatype_to_func['b'] = tojson_stringify_bool;
    tojson_datatype_to_func['B'] = tojson_stringify_bool_array;
    tojson_datatype_to_func['o'] = tojson_stringify_object;
    tojson_datatype_to_func['a'] = tojson_stringify_args;
    tojson_datatype_to_func['A'] = tojson_stringify_args_array;
    tojson_datatype_to_func[')'] = tojson_close_object;

    tojson_static_variables_initialized = 1;
  }
}

error_t tojson_init_variables(int *add_data, int *add_data_without_separator, char **_data_desc,
                              const char *data_desc) {
  tojson_init_static_variables();
  *add_data = (tojson_permanent_state.serial_result != complete);
  *add_data_without_separator = (tojson_permanent_state.serial_result == incomplete_at_struct_beginning);
  if (*add_data) {
    char *data_desc_ptr;
    int data_desc_len = strlen(data_desc);
    *_data_desc = malloc(data_desc_len + 3);
    if (*_data_desc == NULL) {
      debug_print_malloc_error();
      return ERROR_MALLOC;
    }
    data_desc_ptr = *_data_desc;
    if (strncmp(data_desc, "o(", 2) != 0) {
      memcpy(data_desc_ptr, "o(", 2);
      data_desc_ptr += 2;
    }
    memcpy(data_desc_ptr, data_desc, data_desc_len);
    data_desc_ptr += data_desc_len;
    *data_desc_ptr = '\0';
  } else {
    *_data_desc = strdup(data_desc);
    if (*_data_desc == NULL) {
      debug_print_malloc_error();
      return ERROR_MALLOC;
    }
  }

  return NO_ERROR;
}

error_t tojson_write_vl(memwriter_t *memwriter, const char *data_desc, va_list *vl) {
  int add_data, add_data_without_separator;
  char *_data_desc;
  error_t error;

  error = tojson_init_variables(&add_data, &add_data_without_separator, &_data_desc, data_desc);
  if (!error) {
    error = tojson_serialize(memwriter, _data_desc, NULL, vl, 0, add_data, add_data_without_separator,
                             &tojson_permanent_state.struct_nested_level, &tojson_permanent_state.serial_result, NULL);
  }
  free(_data_desc);

  return error;
}

error_t tojson_write_buf(memwriter_t *memwriter, const char *data_desc, const void *buffer, int apply_padding) {
  int add_data, add_data_without_separator;
  char *_data_desc;
  error_t error;

  error = tojson_init_variables(&add_data, &add_data_without_separator, &_data_desc, data_desc);
  if (!error) {
    error = tojson_serialize(memwriter, _data_desc, buffer, NULL, apply_padding, add_data, add_data_without_separator,
                             &tojson_permanent_state.struct_nested_level, &tojson_permanent_state.serial_result, NULL);
  }
  free(_data_desc);

  return error;
}

error_t tojson_write_arg(memwriter_t *memwriter, const arg_t *arg) {
  error_t error = NO_ERROR;

  if (arg->key == NULL) {
    if ((error = tojson_write_buf(memwriter, arg->value_format, arg->value_ptr, 1)) != NO_ERROR) {
      return error;
    }
  } else {
    char *format, *format_ptr;
    size_t key_length, value_format_length;
    key_length = strlen(arg->key);
    value_format_length = strlen(arg->value_format);
    format = malloc(key_length + value_format_length + 2); /* 2 = 1 ':' + 1 '\0' */
    if (format == NULL) {
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
    if ((error = tojson_write_buf(memwriter, format, arg->value_ptr, 1)) != NO_ERROR) {
      free(format);
      return error;
    }
    free(format);
  }

  return error;
}

error_t tojson_write_args(memwriter_t *memwriter, const gr_meta_args_t *args) {
  args_iterator_t *it;
  arg_t *arg;

  it = args_iter(args);
  if ((arg = it->next(it))) {
    tojson_write_buf(memwriter, "o(", NULL, 1);
    do {
      tojson_write_arg(memwriter, arg);
    } while ((arg = it->next(it)));
    tojson_write_buf(memwriter, ")", NULL, 1);
  }
  args_iterator_delete(it);

  return 0;
}

int tojson_is_complete(void) {
  return tojson_permanent_state.serial_result == complete;
}

int tojson_struct_nested_level(void) {
  return tojson_permanent_state.struct_nested_level;
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

error_t memwriter_replace(memwriter_t *memwriter, int index, int count, const char *replacement_str) {
  int replacement_str_len = (replacement_str != NULL) ? strlen(replacement_str) : 0;
  error_t error = NO_ERROR;

  if ((replacement_str_len > count) &&
      (error = memwriter_ensure_buf(memwriter, replacement_str_len - count)) != NO_ERROR) {
    return error;
  }
  if (count != replacement_str_len) {
    memmove(memwriter->buf + index + replacement_str_len, memwriter->buf + index + count,
            memwriter->size - (index + count));
  }
  if (replacement_str != NULL) {
    memcpy(memwriter->buf + index, replacement_str, replacement_str_len);
  }
  memwriter->size += replacement_str_len - count;

  return error;
}

error_t memwriter_erase(memwriter_t *memwriter, int index, int count) {
  return memwriter_replace(memwriter, index, count, NULL);
}

error_t memwriter_insert(memwriter_t *memwriter, int index, const char *str) {
  return memwriter_replace(memwriter, index, 0, str);
}

error_t memwriter_enlarge_buf(memwriter_t *memwriter, size_t size_increment) {
  void *new_buf;

  if (size_increment == 0) {
    size_increment = MEMWRITER_SIZE_INCREMENT;
  } else {
    /* round up to the next `MEMWRITER_SIZE_INCREMENT` step */
    size_increment = ((size_increment - 1) / MEMWRITER_SIZE_INCREMENT + 1) * MEMWRITER_SIZE_INCREMENT;
  }
  new_buf = realloc(memwriter->buf, memwriter->capacity + size_increment);
  if (new_buf == NULL) {
    debug_print_malloc_error();
    return ERROR_MALLOC;
  }
  memwriter->buf = new_buf;
  memwriter->capacity += size_increment;

  return NO_ERROR;
}

error_t memwriter_ensure_buf(memwriter_t *memwriter, size_t needed_additional_size) {
  if (memwriter->size + needed_additional_size > memwriter->capacity) {
    return memwriter_enlarge_buf(memwriter, memwriter->size + needed_additional_size - memwriter->capacity);
  }
  return NO_ERROR;
}

error_t memwriter_printf(memwriter_t *memwriter, const char *format, ...) {
  va_list vl;
  error_t error = NO_ERROR;

  while (1) {
    int chars_needed;
    va_start(vl, format);
    chars_needed = vsnprintf(&memwriter->buf[memwriter->size], memwriter->capacity - memwriter->size, format, vl);
    va_end(vl);
    if (chars_needed < 0) {
      return ERROR_UNSPECIFIED;
    }
    /* we need one more char because `vsnprintf` does exclude the trailing '\0' character in its calculations */
    if ((size_t)chars_needed < (memwriter->capacity - memwriter->size)) {
      memwriter->size += chars_needed;
      break;
    }
    if ((error = memwriter_ensure_buf(memwriter, chars_needed + 1)) != NO_ERROR) {
      break;
    }
  }

  return error;
}

error_t memwriter_puts(memwriter_t *memwriter, const char *s) {
  return memwriter_printf(memwriter, "%s", s);
}

error_t memwriter_putc(memwriter_t *memwriter, char c) {
  return memwriter_printf(memwriter, "%c", c);
}

char *memwriter_buf(const memwriter_t *memwriter) {
  return memwriter->buf;
}

size_t memwriter_size(const memwriter_t *memwriter) {
  return memwriter->size;
}


/* ------------------------- receiver ------------------------------------------------------------------------------- */

error_t receiver_init_for_custom(metahandle_t *handle, const char *name, unsigned int id,
                                 const char *(*custom_recv)(const char *, unsigned int)) {
  handle->sender_receiver.receiver.comm.custom.recv = custom_recv;
  handle->sender_receiver.receiver.comm.custom.name = name;
  handle->sender_receiver.receiver.comm.custom.id = id;
  handle->sender_receiver.receiver.message_size = 0;
  handle->sender_receiver.receiver.recv = receiver_recv_for_custom;
  handle->finalize = receiver_finalize_for_custom;
  handle->sender_receiver.receiver.memwriter = memwriter_new();
  if (handle->sender_receiver.receiver.memwriter == NULL) {
    return ERROR_MALLOC;
  }

  return NO_ERROR;
}

error_t receiver_init_for_socket(metahandle_t *handle, const char *hostname, unsigned int port) {
  char port_str[PORT_MAX_STRING_LENGTH];
  struct addrinfo *addr_result = NULL, *addr_ptr = NULL, addr_hints;
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  socklen_t client_addrlen = sizeof(client_addr);
  int error;
#ifdef SO_REUSEADDR
  int socket_opt;
#endif
#ifdef _WIN32
  int wsa_startup_error = 0;
  WSADATA wsa_data;
#endif

  snprintf(port_str, PORT_MAX_STRING_LENGTH, "%u", port);

  handle->sender_receiver.receiver.memwriter = NULL;
  handle->sender_receiver.receiver.comm.socket.server_socket = -1;
  handle->sender_receiver.receiver.comm.socket.client_socket = -1;
  handle->sender_receiver.receiver.recv = receiver_recv_for_socket;
  handle->finalize = receiver_finalize_for_socket;

#ifdef _WIN32
  /* Initialize winsock */
  if ((wsa_startup_error = WSAStartup(MAKEWORD(2, 2), &wsa_data))) {
#ifndef NDEBUG
    /* on WSAStartup failure `WSAGetLastError` should not be called (see MSDN), use the error code directly instead
     */
    wchar_t *message = NULL;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                   wsa_startup_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&message, 0, NULL);
    debug_print_error(("winsock initialization failed: %S\n", message));
    LocalFree(message);
#endif
    return ERROR_NETWORK_WINSOCK_INIT;
  }
#endif

  memset(&addr_hints, 0, sizeof(addr_hints));
  addr_hints.ai_family = AF_UNSPEC;
  addr_hints.ai_socktype = SOCK_STREAM;
  addr_hints.ai_protocol = 0;
  addr_hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;

  /* Query a list of ip addresses for the given hostname */
  if ((error = getaddrinfo(hostname, port_str, &addr_hints, &addr_result)) != 0) {
#ifdef _WIN32
    psocketerror("getaddrinfo failed with error");
#else
    if (error == EAI_SYSTEM) {
      perror("getaddrinfo failed with error");
    } else {
      fprintf(stderr, "getaddrinfo failed with error: %s\n", gai_strerror(error));
    }
#endif
    return ERROR_NETWORK_HOSTNAME_RESOLUTION;
  }

  /* Create a socket for listening */
  if ((handle->sender_receiver.receiver.comm.socket.server_socket =
         socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol)) < 0) {
    psocketerror("socket creation failed");
    freeaddrinfo(addr_result);
    return ERROR_NETWORK_SOCKET_CREATION;
  }
  /* Set SO_REUSEADDR if available on this system */
#ifdef SO_REUSEADDR
  socket_opt = 1;
  if (setsockopt(handle->sender_receiver.receiver.comm.socket.server_socket, SOL_SOCKET, SO_REUSEADDR,
                 (char *)&socket_opt, sizeof(socket_opt)) < 0) {
    psocketerror("setting socket options failed");
    freeaddrinfo(addr_result);
    return ERROR_NETWORK_SOCKET_CREATION;
  }
#endif

  /* Bind the socket to given ip address and port */
  if (bind(handle->sender_receiver.receiver.comm.socket.server_socket, addr_result->ai_addr, addr_result->ai_addrlen)) {
    psocketerror("bind failed");
    freeaddrinfo(addr_result);
    return ERROR_NETWORK_SOCKET_BIND;
  }
  freeaddrinfo(addr_result);

  /* Listen for incoming connections */
  if (listen(handle->sender_receiver.receiver.comm.socket.server_socket, 1)) {
    psocketerror("listen failed");
    return ERROR_NETWORK_SOCKET_LISTEN;
  }

  /* Accecpt an incoming connection and get a new socket instance for communication */
  if ((handle->sender_receiver.receiver.comm.socket.client_socket =
         accept(handle->sender_receiver.receiver.comm.socket.server_socket, (struct sockaddr *)&client_addr,
                &client_addrlen)) < 0) {
    psocketerror("accept failed");
    return ERROR_NETWORK_CONNECTION_ACCEPT;
  }

  handle->sender_receiver.receiver.memwriter = memwriter_new();
  if (handle->sender_receiver.receiver.memwriter == NULL) {
    return ERROR_MALLOC;
  }

  return NO_ERROR;
}

error_t receiver_finalize_for_custom(metahandle_t *handle) {
  memwriter_delete(handle->sender_receiver.receiver.memwriter);

  return NO_ERROR;
}

error_t receiver_finalize_for_socket(metahandle_t *handle) {
  error_t error = NO_ERROR;

  memwriter_delete(handle->sender_receiver.receiver.memwriter);
#ifdef _WIN32
  if (handle->sender_receiver.receiver.comm.socket.client_socket >= 0) {
    if (closesocket(handle->sender_receiver.receiver.comm.socket.client_socket)) {
      psocketerror("client socket shutdown failed");
      error = ERROR_NETWORK_SOCKET_CLOSE;
    }
  }
  if (handle->sender_receiver.receiver.comm.socket.server_socket >= 0) {
    if (closesocket(handle->sender_receiver.receiver.comm.socket.server_socket)) {
      psocketerror("server socket shutdown failed");
      error = ERROR_NETWORK_SOCKET_CLOSE;
    }
  }
  if (WSACleanup()) {
    psocketerror("winsock shutdown failed");
    error = ERROR_NETWORK_WINSOCK_CLEANUP;
  }
#else
  if (handle->sender_receiver.receiver.comm.socket.client_socket >= 0) {
    if (close(handle->sender_receiver.receiver.comm.socket.client_socket)) {
      psocketerror("client socket shutdown failed");
      error = ERROR_NETWORK_SOCKET_CLOSE;
    }
  }
  if (handle->sender_receiver.receiver.comm.socket.server_socket >= 0) {
    if (close(handle->sender_receiver.receiver.comm.socket.server_socket)) {
      psocketerror("server socket shutdown failed");
      error = ERROR_NETWORK_SOCKET_CLOSE;
    }
  }
#endif

  return error;
}

error_t receiver_recv_for_socket(metahandle_t *handle) {
  int search_start_index = 0;
  char *end_ptr;
  static char recv_buf[SOCKET_RECV_BUF_SIZE];
  error_t error = NO_ERROR;

  memwriter_clear(handle->sender_receiver.receiver.memwriter);
  while ((end_ptr = memchr(memwriter_buf(handle->sender_receiver.receiver.memwriter) + search_start_index, ETB,
                           memwriter_size(handle->sender_receiver.receiver.memwriter) - search_start_index)) == NULL) {
    int bytes_received;
    search_start_index = memwriter_size(handle->sender_receiver.receiver.memwriter);
    if ((bytes_received =
           recv(handle->sender_receiver.receiver.comm.socket.client_socket, recv_buf, SOCKET_RECV_BUF_SIZE, 0)) < 0) {
      psocketerror("error while receiving data");
      return ERROR_NETWORK_RECV;
    }
    if ((error = memwriter_printf(handle->sender_receiver.receiver.memwriter, "%.*s", bytes_received, recv_buf)) !=
        NO_ERROR) {
      return error;
    }
  }
  *end_ptr = '\0';
  handle->sender_receiver.receiver.message_size = end_ptr - memwriter_buf(handle->sender_receiver.receiver.memwriter);

  return error;
}

error_t receiver_recv_for_custom(metahandle_t *handle) {
  /* TODO: is it really necessary to copy the memory? */
  const char *recv_buf;
  error_t error = NO_ERROR;

  recv_buf = handle->sender_receiver.receiver.comm.custom.recv(handle->sender_receiver.receiver.comm.custom.name,
                                                               handle->sender_receiver.receiver.comm.custom.id);
  if (recv_buf == NULL) {
    return ERROR_CUSTOM_RECV;
  }
  memwriter_clear(handle->sender_receiver.receiver.memwriter);
  if ((error = memwriter_puts(handle->sender_receiver.receiver.memwriter, recv_buf)) != NO_ERROR) {
    return error;
  }
  handle->sender_receiver.receiver.message_size = memwriter_size(handle->sender_receiver.receiver.memwriter);

  return error;
}


/* ------------------------- sender --------------------------------------------------------------------------------- */

error_t sender_init_for_custom(metahandle_t *handle, const char *name, unsigned int id,
                               int (*custom_send)(const char *, unsigned int, const char *)) {
  handle->sender_receiver.sender.comm.custom.send = custom_send;
  handle->sender_receiver.sender.comm.custom.name = name;
  handle->sender_receiver.sender.comm.custom.id = id;
  handle->sender_receiver.sender.send = sender_send_for_custom;
  handle->finalize = sender_finalize_for_custom;
  handle->sender_receiver.sender.memwriter = memwriter_new();
  if (handle->sender_receiver.sender.memwriter == NULL) {
    return ERROR_MALLOC;
  }

  return NO_ERROR;
}

error_t sender_init_for_socket(metahandle_t *handle, const char *hostname, unsigned int port) {
  char port_str[PORT_MAX_STRING_LENGTH];
  struct addrinfo *addr_result = NULL, *addr_ptr = NULL, addr_hints;
  int error;
#ifdef _WIN32
  int wsa_startup_error = 0;
  WSADATA wsa_data;
#endif

  snprintf(port_str, PORT_MAX_STRING_LENGTH, "%u", port);

  handle->sender_receiver.sender.memwriter = NULL;
  handle->sender_receiver.sender.comm.socket.client_socket = -1;
  handle->sender_receiver.sender.send = sender_send_for_socket;
  handle->finalize = sender_finalize_for_socket;

#ifdef _WIN32
  /* Initialize winsock */
  if ((wsa_startup_error = WSAStartup(MAKEWORD(2, 2), &wsa_data))) {
#ifndef NDEBUG
    /* on WSAStartup failure `WSAGetLastError` should not be called (see MSDN), use the error code directly instead
     */
    wchar_t *message = NULL;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                   wsa_startup_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&message, 0, NULL);
    debug_print_error(("winsock initialization failed: %S\n", message));
    LocalFree(message);
#endif
    return ERROR_NETWORK_WINSOCK_INIT;
  }
#endif

  memset(&addr_hints, 0, sizeof(addr_hints));
  addr_hints.ai_family = AF_UNSPEC;
  addr_hints.ai_socktype = SOCK_STREAM;
  addr_hints.ai_protocol = IPPROTO_TCP;

  /* Query a list of ip addresses for the given hostname */
  if ((error = getaddrinfo(hostname, port_str, &addr_hints, &addr_result)) != 0) {
#ifdef _WIN32
    psocketerror("getaddrinfo failed with error");
#else
    if (error == EAI_SYSTEM) {
      perror("getaddrinfo failed with error");
    } else {
      fprintf(stderr, "getaddrinfo failed with error: %s\n", gai_strerror(error));
    }
#endif
    return ERROR_NETWORK_HOSTNAME_RESOLUTION;
  }

  /* Attempt to connect to an address until one succeeds */
  handle->sender_receiver.sender.comm.socket.client_socket = -1;
  for (addr_ptr = addr_result; addr_ptr != NULL && handle->sender_receiver.sender.comm.socket.client_socket < 0;
       addr_ptr = addr_ptr->ai_next) {
    /* Create a socket for connecting to server */
    handle->sender_receiver.sender.comm.socket.client_socket =
      socket(addr_ptr->ai_family, addr_ptr->ai_socktype, addr_ptr->ai_protocol);
    if (handle->sender_receiver.sender.comm.socket.client_socket < 0) {
      psocketerror("socket creation failed");
      return ERROR_NETWORK_SOCKET_CREATION;
    }
    /* Connect to server */
    if (connect(handle->sender_receiver.sender.comm.socket.client_socket, addr_ptr->ai_addr,
                (int)addr_ptr->ai_addrlen)) {
#ifdef _WIN32
      closesocket(handle->sender_receiver.sender.comm.socket.client_socket);
#else
      close(handle->sender_receiver.sender.comm.socket.client_socket);
#endif
      handle->sender_receiver.sender.comm.socket.client_socket = -1;
    }
  }
  freeaddrinfo(addr_result);

  if (handle->sender_receiver.sender.comm.socket.client_socket < 0) {
    fprintf(stderr, "cannot connect to host %s port %u: ", hostname, port);
    psocketerror("");
    return ERROR_NETWORK_CONNECT;
  }

  handle->sender_receiver.sender.memwriter = memwriter_new();
  if (handle->sender_receiver.sender.memwriter == NULL) {
    return ERROR_MALLOC;
  }

  return NO_ERROR;
}

error_t sender_finalize_for_custom(metahandle_t *handle) {
  memwriter_delete(handle->sender_receiver.sender.memwriter);

  return NO_ERROR;
}

error_t sender_finalize_for_socket(metahandle_t *handle) {
  error_t error = NO_ERROR;

  memwriter_delete(handle->sender_receiver.sender.memwriter);
#ifdef _WIN32
  if (handle->sender_receiver.sender.comm.socket.client_socket >= 0) {
    if (closesocket(handle->sender_receiver.sender.comm.socket.client_socket)) {
      psocketerror("client socket shutdown failed");
      error = ERROR_NETWORK_SOCKET_CLOSE;
    }
  }
  if (WSACleanup()) {
    psocketerror("winsock shutdown failed");
    error = ERROR_NETWORK_WINSOCK_CLEANUP;
  }
#else
  if (handle->sender_receiver.sender.comm.socket.client_socket >= 0) {
    if (close(handle->sender_receiver.sender.comm.socket.client_socket)) {
      psocketerror("client socket shutdown failed");
      error = ERROR_NETWORK_SOCKET_CLOSE;
    }
  }
#endif

  return error;
}

error_t sender_send_for_socket(metahandle_t *handle) {
  const char *buf, *send_ptr;
  size_t buf_size;
  int bytes_left;
  error_t error = NO_ERROR;

  if ((error = memwriter_putc(handle->sender_receiver.sender.memwriter, ETB)) != NO_ERROR) {
    return error;
  }

  buf = memwriter_buf(handle->sender_receiver.sender.memwriter);
  buf_size = memwriter_size(handle->sender_receiver.sender.memwriter);

  send_ptr = buf;
  bytes_left = buf_size;
  while (bytes_left) {
    int bytes_sent = send(handle->sender_receiver.sender.comm.socket.client_socket, buf, bytes_left, 0);
    if (bytes_sent < 0) {
      psocketerror("could not send any data");
      return ERROR_NETWORK_SEND;
    }
    send_ptr += bytes_sent;
    bytes_left -= bytes_sent;
  }

  memwriter_clear(handle->sender_receiver.sender.memwriter);

  return error;
}

error_t sender_send_for_custom(metahandle_t *handle) {
  const char *buf;
  error_t error = NO_ERROR;

  buf = memwriter_buf(handle->sender_receiver.sender.memwriter);
  if (!handle->sender_receiver.sender.comm.custom.send(handle->sender_receiver.sender.comm.custom.name,
                                                       handle->sender_receiver.sender.comm.custom.id, buf)) {
    error = ERROR_CUSTOM_SEND;
    return error;
  }
  memwriter_clear(handle->sender_receiver.sender.memwriter);

  return error;
}

#ifndef NDEBUG
void gr_dumpmeta(const gr_meta_args_t *args, FILE *f) {
  args_iterator_t *it;
  args_value_iterator_t *value_it;
  arg_t *arg;
  int i;

  fprintf(f, "=== container contents ===\n");

  it = args_iter(args);
  while ((arg = it->next(it)) != NULL) {
    if (*arg->value_format) {
      value_it = arg_value_iter(arg);
      while (value_it->next(value_it) != NULL) {
        switch (value_it->format) {
        case 'i':
          if (value_it->is_array) {
            fprintf(f, "%s: [", arg->key);
            for (i = 0; i < value_it->array_length; i++) {
              fprintf(f, "%d, ", (*((int **)value_it->value_ptr))[i]);
            }
            if (value_it->array_length > 0) {
              fprintf(f, "\b\b");
            }
            fprintf(f, "]\n");
          } else {
            fprintf(f, "%s: %d\n", arg->key, *((int *)value_it->value_ptr));
          }
          break;
        case 'd':
          if (value_it->is_array) {
            fprintf(f, "%s: [", arg->key);
            for (i = 0; i < value_it->array_length; i++) {
              fprintf(f, "%lf, ", (*((double **)value_it->value_ptr))[i]);
            }
            if (value_it->array_length > 0) {
              fprintf(f, "\b\b");
            }
            fprintf(f, "]\n");
          } else {
            fprintf(f, "%s: %lf\n", arg->key, *((double *)value_it->value_ptr));
          }
          break;
        case 'c':
          fprintf(f, "%s: %c\n", arg->key, *((char *)value_it->value_ptr));
          break;
        case 's':
          if (value_it->is_array) {
            fprintf(f, "%s: [", arg->key);
            for (i = 0; i < value_it->array_length; i++) {
              fprintf(f, "%s, ", (*((char ***)value_it->value_ptr))[i]);
            }
            if (value_it->array_length > 0) {
              fprintf(f, "\b\b");
            }
            fprintf(f, "]\n");
          } else {
            fprintf(f, "%s: %s\n", arg->key, *((char **)value_it->value_ptr));
          }
          break;
        case 'a':
          if (value_it->is_array) {
            fprintf(f, "%s: [\n", arg->key);
            for (i = 0; i < value_it->array_length; i++) {
              gr_dumpmeta((*((gr_meta_args_t ***)value_it->value_ptr))[i], f);
            }
            if (value_it->array_length > 0) {
              fprintf(f, "\b\b");
            }
            fprintf(f, "]\n");
          } else {
            fprintf(f, "%s: container\n", arg->key);
            gr_dumpmeta(*((gr_meta_args_t **)value_it->value_ptr), f);
          }
          break;
        default:
          break;
        }
      }
      args_value_iterator_delete(value_it);
    } else {
      fprintf(f, "%s: (none)\n", arg->key);
    }
  }
  args_iterator_delete(it);

  fprintf(f, "=== container contents end ===\n");
}

void gr_dumpmeta_json(const gr_meta_args_t *args, FILE *f) {
  static memwriter_t *memwriter = NULL;

  if (memwriter == NULL) {
    memwriter = memwriter_new();
  }
  tojson_write_args(memwriter, args);
  if (tojson_is_complete()) {
    memwriter_putc(memwriter, '\0');
    fprintf(f, "%s\n", memwriter_buf(memwriter));
    memwriter_delete(memwriter);
    memwriter = NULL;
  }
}

#ifdef EMSCRIPTEN
FILE *gr_get_stdout() {
  return stdout;
}
#endif
#endif


/* ------------------------- string-to-generic map ------------------------------------------------------------------ */

#define DEFINE_MAP_METHODS(prefix, value_type, invalid_value)                                                 \
                                                                                                              \
  prefix##_map_t *prefix##_map_new(size_t capacity) {                                                         \
    prefix##_map_t *prefix##_map;                                                                             \
    size_t power2_capacity = 1;                                                                               \
                                                                                                              \
    while (power2_capacity < 2 * capacity) {                                                                  \
      power2_capacity <<= 1;                                                                                  \
    }                                                                                                         \
    prefix##_map = malloc(sizeof(prefix##_map_t));                                                            \
    if (prefix##_map == NULL) {                                                                               \
      debug_print_malloc_error();                                                                             \
      return NULL;                                                                                            \
    }                                                                                                         \
    prefix##_map->map = calloc(power2_capacity, sizeof(prefix##_map_entry_t));                                \
    if (prefix##_map->map == NULL) {                                                                          \
      free(prefix##_map);                                                                                     \
      debug_print_malloc_error();                                                                             \
      return NULL;                                                                                            \
    }                                                                                                         \
    prefix##_map->capacity = power2_capacity;                                                                 \
    prefix##_map->size = 0;                                                                                   \
                                                                                                              \
    return prefix##_map;                                                                                      \
  }                                                                                                           \
                                                                                                              \
  prefix##_map_t *prefix##_map_new_with_data(size_t count, prefix##_map_entry_t *entries) {                   \
    prefix##_map_t *prefix##_map;                                                                             \
    size_t i;                                                                                                 \
                                                                                                              \
    prefix##_map = prefix##_map_new(count);                                                                   \
    if (prefix##_map == NULL) {                                                                               \
      return NULL;                                                                                            \
    }                                                                                                         \
    for (i = 0; i < count; ++i) {                                                                             \
      if (!prefix##_map_insert(prefix##_map, entries[i].key, entries[i].value)) {                             \
        prefix##_map_delete(prefix##_map);                                                                    \
        return NULL;                                                                                          \
      }                                                                                                       \
    }                                                                                                         \
                                                                                                              \
    return prefix##_map;                                                                                      \
  }                                                                                                           \
                                                                                                              \
  void prefix##_map_delete(prefix##_map_t *prefix##_map) {                                                    \
    ssize_t i;                                                                                                \
    for (i = 0; i < prefix##_map->capacity; ++i) {                                                            \
      if (prefix##_map->map[i].key != NULL) {                                                                 \
        free((char *)prefix##_map->map[i].key);                                                               \
      }                                                                                                       \
    }                                                                                                         \
    free(prefix##_map->map);                                                                                  \
    free(prefix##_map);                                                                                       \
  }                                                                                                           \
                                                                                                              \
  int prefix##_map_insert(prefix##_map_t *prefix##_map, const char *key, value_type value) {                  \
    ssize_t index;                                                                                            \
                                                                                                              \
    index = prefix##_map_index(prefix##_map, key);                                                            \
    if (index < 0) {                                                                                          \
      return 0;                                                                                               \
    }                                                                                                         \
    if (prefix##_map->map[index].key != NULL) {                                                               \
      free((char *)prefix##_map->map[index].key);                                                             \
    }                                                                                                         \
    prefix##_map->map[index].key = strdup(key);                                                               \
    if (prefix##_map->map[index].key == NULL) {                                                               \
      debug_print_malloc_error();                                                                             \
      return 0;                                                                                               \
    }                                                                                                         \
    prefix##_map->map[index].value = value;                                                                   \
                                                                                                              \
    return 1;                                                                                                 \
  }                                                                                                           \
                                                                                                              \
  value_type prefix##_map_at(prefix##_map_t *prefix##_map, const char *key) {                                 \
    ssize_t index;                                                                                            \
                                                                                                              \
    index = prefix##_map_index(prefix##_map, key);                                                            \
    if (index < 0) {                                                                                          \
      return invalid_value;                                                                                   \
    }                                                                                                         \
    return prefix##_map->map[index].value;                                                                    \
  }                                                                                                           \
                                                                                                              \
  ssize_t prefix##_map_index(prefix##_map_t *prefix##_map, const char *key) {                                 \
    size_t hash;                                                                                              \
    size_t i;                                                                                                 \
                                                                                                              \
    hash = djb2_hash(key);                                                                                    \
    for (i = 0; i < prefix##_map->capacity; ++i) {                                                            \
      /* Quadratic probing that will visit every slot in the hash table if the capacity is a power of 2, see: \
       * http://research.cs.vt.edu/AVresearch/hashing/quadratic.php */                                        \
      size_t next_index = (hash + (i * i + i) / 2) % prefix##_map->capacity;                                  \
      if (prefix##_map->map[next_index].key == NULL || strcmp(prefix##_map->map[next_index].key, key) == 0) { \
        return next_index;                                                                                    \
      }                                                                                                       \
    }                                                                                                         \
    return -1;                                                                                                \
  }

DEFINE_MAP_METHODS(fmt, const char *, NULL)
DEFINE_MAP_METHODS(plot_func, plot_func_t, NULL)

#undef DEFINE_MAP_METHODS
