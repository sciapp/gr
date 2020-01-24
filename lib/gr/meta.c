/* TODO: this is workaround to allow building with mingw and cmake */
#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG 0x00000400
#endif

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
#include <sys/ioctl.h>
#endif
#ifdef _MSC_VER
#include <intrin.h>
#endif

#include "gks.h"
#include "gr.h"
#include "gkscore.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
/* allow the use of posix functions on windows with msvc */
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#define snprintf(buf, len, format, ...) _snprintf_s(buf, len, len, format, __VA_ARGS__)
#endif

/* ######################### private interface ###################################################################### */

/* ========================= macros ================================================================================= */

/* ------------------------- error handling ------------------------------------------------------------------------- */

#ifndef NDEBUG
static void debug_printf(const char *format, ...)
{
  va_list vl;
  va_start(vl, format);
  vfprintf(stderr, format, vl);
  va_end(vl);
}
#define debug_print_error(error_message_arguments) \
  do                                               \
    {                                              \
      debug_printf error_message_arguments;        \
    }                                              \
  while (0)
#ifdef _WIN32
#define psocketerror(prefix_message)                                                                                   \
  do                                                                                                                   \
    {                                                                                                                  \
      char *message = NULL;                                                                                            \
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, \
                    WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message, 0, NULL);           \
      fprintf(stderr, "%s: %s", prefix_message, message);                                                              \
      LocalFree(message);                                                                                              \
    }                                                                                                                  \
  while (0)
#else
#define psocketerror(prefix_message) perror(prefix_message)
#endif
#else
#define debug_print_error(error_message_arguments)
#define psocketerror(prefix_message)
#endif
#define debug_print_malloc_error()                                                                                    \
  do                                                                                                                  \
    {                                                                                                                 \
      if (isatty(fileno(stderr)))                                                                                     \
        {                                                                                                             \
          debug_print_error(                                                                                          \
              ("\033[96m%s\033[0m:\033[93m%d\033[0m: Memory allocation failed -> out of virtual memory.\n", __FILE__, \
               __LINE__));                                                                                            \
        }                                                                                                             \
      else                                                                                                            \
        {                                                                                                             \
          debug_print_error(("%s:%d: Memory allocation failed -> out of virtual memory.\n", __FILE__, __LINE__));     \
        }                                                                                                             \
    }                                                                                                                 \
  while (0)


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
#if defined(_WIN32) || defined(__EMSCRIPTEN__)
#define logger(logger_arguments)                                            \
  do                                                                        \
    {                                                                       \
      fprintf(stderr, "%s:%d(%s): ", __FILE__, __LINE__, CURRENT_FUNCTION); \
      fprintf logger_arguments;                                             \
    }                                                                       \
  while (0)
#else
#define logger(logger_arguments)                                                                          \
  do                                                                                                      \
    {                                                                                                     \
      if (isatty(fileno(stderr)))                                                                         \
        {                                                                                                 \
          fprintf(stderr, "\033[36m%s\033[0m:\033[33m%d\033[0m(\033[34m%s\033[0m): ", __FILE__, __LINE__, \
                  CURRENT_FUNCTION);                                                                      \
        }                                                                                                 \
      else                                                                                                \
        {                                                                                                 \
          fprintf(stderr, "%s:%d(%s): ", __FILE__, __LINE__, CURRENT_FUNCTION);                           \
        }                                                                                                 \
      fprintf logger_arguments;                                                                           \
    }                                                                                                     \
  while (0)
#endif
#else
#define logger(logger_arguments)
#endif


/* ------------------------- memwriter ------------------------------------------------------------------------------ */

#define MEMWRITER_INITIAL_SIZE 32768
#define MEMWRITER_EXPONENTIAL_INCREASE_UNTIL 268435456
#define MEMWRITER_LINEAR_INCREMENT_SIZE 67108864

#define ETB '\027'


/* ------------------------- plot ----------------------------------------------------------------------------------- */

#define ROOT_DEFAULT_APPEND_PLOTS 0
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
#define PLOT_DEFAULT_KEEP_ASPECT_RATIO 0
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


/* ------------------------- user interaction ----------------------------------------------------------------------- */

#define INPUTMETA_ANGLE_DELTA_FACTOR 0.001
#define INPUTMETA_DEFAULT_KEEP_ASPECT_RATIO 1


/* ------------------------- util ----------------------------------------------------------------------------------- */

#define is_string_delimiter(char_ptr, str) ((*(char_ptr) == '"') && (((char_ptr) == (str)) || *((char_ptr)-1) != '\\'))

#ifndef array_size
#define array_size(a) ((sizeof(a) / sizeof(*(a))))
#endif

#ifndef min
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef max
#define max(x, y) (((x) > (y)) ? (x) : (y))
#endif

/* test macros which can be used like `assert` */
#define return_error_if(condition, error_value)                                                    \
  do                                                                                               \
    {                                                                                              \
      if (condition)                                                                               \
        {                                                                                          \
          logger((stderr, "Got error \"%d\" (\"%s\")!\n", error_value, error_names[error_value])); \
          return (error_value);                                                                    \
        }                                                                                          \
    }                                                                                              \
  while (0)
#define return_if_error return_error_if((error) != NO_ERROR, (error))
#define goto_if(condition, goto_label) \
  do                                   \
    {                                  \
      if (condition)                   \
        {                              \
          goto goto_label;             \
        }                              \
    }                                  \
  while (0)
#define cleanup_if(condition) goto_if((condition), cleanup)
#define error_cleanup_if(condition) goto_if((condition), error_cleanup)
#define goto_if_error(goto_label)                                                          \
  do                                                                                       \
    {                                                                                      \
      if ((error) != NO_ERROR)                                                             \
        {                                                                                  \
          logger((stderr, "Got error \"%d\" (\"%s\")!\n", (error), error_names[(error)])); \
          goto goto_label;                                                                 \
        }                                                                                  \
    }                                                                                      \
  while (0)
#define cleanup_if_error goto_if_error(cleanup)
#define error_cleanup_if_error goto_if_error(error_cleanup)
#define goto_and_set_error_if(condition, error_value, goto_label)                                      \
  do                                                                                                   \
    {                                                                                                  \
      if (condition)                                                                                   \
        {                                                                                              \
          error = (error_value);                                                                       \
          if (error == ERROR_MALLOC)                                                                   \
            {                                                                                          \
              debug_print_malloc_error();                                                              \
            }                                                                                          \
          else                                                                                         \
            {                                                                                          \
              logger((stderr, "Got error \"%d\" (\"%s\")!\n", error_value, error_names[error_value])); \
            }                                                                                          \
          goto goto_label;                                                                             \
        }                                                                                              \
    }                                                                                                  \
  while (0)
#define cleanup_and_set_error_if(condition, error_value) goto_and_set_error_if((condition), (error_value), cleanup)
#define error_cleanup_and_set_error_if(condition, error_value) \
  goto_and_set_error_if((condition), (error_value), error_cleanup)

#if (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))) || defined(__clang__)
#define MAYBE_UNUSED __attribute__((unused))
#define UNUSED \
  __attribute__((unused, deprecated("Marked as \"UNUSED\" but used. Please remove the \"UNUSED\" marker.")))
#else
#define MAYBE_UNUSED
#define UNUSED
#endif


#if defined(__GNUC__) || defined(__clang__)
#define CURRENT_FUNCTION __func__
#elif defined(__FUNCTION__)
#define CURRENT_FUNCTION __FUNCTION__
#else
#define CURRENT_FUNCTION "<unknown>"
#endif


/* ========================= datatypes ============================================================================== */

/* ------------------------- argument ------------------------------------------------------------------------------- */

typedef struct _gr_meta_arg_private_t
{
  unsigned int reference_count;
} arg_private_t;

typedef struct
{
  const char *key;
  void *value_ptr;
  const char *value_format;
  arg_private_t *priv;
} arg_t;


/* ------------------------- argument parsing ----------------------------------------------------------------------- */

struct _argparse_state_t
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
typedef struct _argparse_state_t argparse_state_t;

typedef void (*read_param_t)(argparse_state_t *);
typedef void (*delete_value_t)(void *);


/* ------------------------- argument container --------------------------------------------------------------------- */

typedef struct _args_node_t
{
  arg_t *arg;
  struct _args_node_t *next;
} args_node_t;

struct _gr_meta_args_t
{
  args_node_t *kwargs_head;
  args_node_t *kwargs_tail;
  unsigned int count;
};
typedef gr_meta_args_t *gr_meta_args_ptr_t;


/* ------------------------- argument iterator ---------------------------------------------------------------------- */

typedef struct
{
  const args_node_t *next_node;
  const args_node_t *end;
} args_iterator_private_t;

typedef struct _args_iterator_t
{
  arg_t *(*next)(struct _args_iterator_t *);
  arg_t *arg;
  args_iterator_private_t *priv;
} args_iterator_t;


/* ------------------------- dynamic args array --------------------------------------------------------------------- */

typedef struct
{
  gr_meta_args_t **buf;
  size_t size;
  size_t capacity;
} dynamic_args_array_t;


/* ------------------------- error handling ------------------------------------------------------------------------- */

#define ENUM_ELEMENTS(X, Y)                           \
  X(ERROR_UNSPECIFIED, 1)                             \
  X(ERROR_INTERNAL, 2)                                \
  X(ERROR_MALLOC, 3)                                  \
  X(ERROR_UNSUPPORTED_OPERATION, 4)                   \
  X(ERROR_UNSUPPORTED_DATATYPE, 5)                    \
  X(ERROR_ARGS_INVALID_KEY, 6)                        \
  X(ERROR_ARGS_INCREASING_NON_ARRAY_VALUE, 7)         \
  X(ERROR_ARGS_INCREASING_MULTI_DIMENSIONAL_ARRAY, 8) \
  X(ERROR_PARSE_NULL, 9)                              \
  X(ERROR_PARSE_BOOL, 10)                             \
  X(ERROR_PARSE_INT, 11)                              \
  X(ERROR_PARSE_DOUBLE, 12)                           \
  X(ERROR_PARSE_STRING, 13)                           \
  X(ERROR_PARSE_ARRAY, 14)                            \
  X(ERROR_PARSE_OBJECT, 15)                           \
  X(ERROR_PARSE_UNKNOWN_DATATYPE, 16)                 \
  X(ERROR_PARSE_INVALID_DELIMITER, 17)                \
  X(ERROR_PARSE_INCOMPLETE_STRING, 18)                \
  X(ERROR_PARSE_MISSING_OBJECT_CONTAINER, 19)         \
  X(ERROR_NETWORK_WINSOCK_INIT, 20)                   \
  X(ERROR_NETWORK_SOCKET_CREATION, 21)                \
  X(ERROR_NETWORK_SOCKET_BIND, 22)                    \
  X(ERROR_NETWORK_SOCKET_LISTEN, 23)                  \
  X(ERROR_NETWORK_CONNECTION_ACCEPT, 24)              \
  X(ERROR_NETWORK_HOSTNAME_RESOLUTION, 25)            \
  X(ERROR_NETWORK_CONNECT, 26)                        \
  X(ERROR_NETWORK_RECV, 27)                           \
  X(ERROR_NETWORK_RECV_CONNECTION_SHUTDOWN, 28)       \
  X(ERROR_NETWORK_SEND, 29)                           \
  X(ERROR_NETWORK_SOCKET_CLOSE, 30)                   \
  X(ERROR_NETWORK_WINSOCK_CLEANUP, 31)                \
  X(ERROR_CUSTOM_RECV, 32)                            \
  X(ERROR_CUSTOM_SEND, 33)                            \
  X(ERROR_PLOT_NORMALIZATION, 34)                     \
  X(ERROR_PLOT_UNKNOWN_KEY, 35)                       \
  X(ERROR_PLOT_UNKNOWN_KIND, 36)                      \
  X(ERROR_PLOT_MISSING_DATA, 37)                      \
  X(ERROR_PLOT_COMPONENT_LENGTH_MISMATCH, 38)         \
  X(ERROR_PLOT_MISSING_LABELS, 39)                    \
  X(ERROR_PLOT_INVALID_ID, 40)                        \
  Y(ERROR_NOT_IMPLEMENTED, 41)

#define ENUM_VALUE(name, value) name = value,
#define ENUM_LAST_VALUE(name, value) name = value
#define STRING_ARRAY_VALUE(name, value) #name,
#define STRING_ARRAY_LAST_VALUE(name, value) #name

typedef enum
{
#ifndef _WIN32 /* Windows uses `NO_ERROR` (= 0) for its own error codes */
  ENUM_VALUE(NO_ERROR, 0)
#endif
      ENUM_ELEMENTS(ENUM_VALUE, ENUM_LAST_VALUE)
} error_t;

#ifndef NDEBUG
const char *error_names[] = {STRING_ARRAY_VALUE(NO_ERROR, 0)
                                 ENUM_ELEMENTS(STRING_ARRAY_VALUE, STRING_ARRAY_LAST_VALUE)};
#endif

#undef ENUM_ELEMENTS
#undef ENUM_VALUE
#undef STRING_ARRAY_VALUE


/* ------------------------- value iterator ------------------------------------------------------------------------- */

typedef struct
{
  void *value_buffer;
  const char *value_format;
} args_value_iterator_private_t;

typedef struct _gr_meta_args_value_iterator_t
{
  void *(*next)(struct _gr_meta_args_value_iterator_t *);
  void *value_ptr;
  char format;
  int is_array;
  size_t array_length;
  args_value_iterator_private_t *priv;
} args_value_iterator_t;


/* ------------------------- memwriter ------------------------------------------------------------------------------ */

struct _memwriter_t
{
  char *buf;
  size_t size;
  size_t capacity;
};
typedef struct _memwriter_t memwriter_t;


/* ------------------------- json deserializer ---------------------------------------------------------------------- */

typedef enum
{
  JSON_DATATYPE_UNKNOWN,
  JSON_DATATYPE_NULL,
  JSON_DATATYPE_BOOL,
  JSON_DATATYPE_NUMBER,
  JSON_DATATYPE_STRING,
  JSON_DATATYPE_ARRAY,
  JSON_DATATYPE_OBJECT
} fromjson_datatype_t;

typedef struct
{
  char *json_ptr;
  int parsed_any_value_before;
} fromjson_shared_state_t;

typedef struct
{
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

enum
{
  member_name,
  data_type
};

typedef enum
{
  /* 0 is unknown / not set */
  complete = 1,
  incomplete,
  incomplete_at_struct_beginning
} tojson_serialization_result_t;

typedef struct
{
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

typedef struct
{
  memwriter_t *memwriter;
  char *data_type_ptr;
  char current_data_type;
  char *additional_type_info;
  int is_type_info_incomplete;
  int add_data_without_separator;
  tojson_shared_state_t *shared;
} tojson_state_t;

typedef struct
{
  tojson_serialization_result_t serial_result;
  unsigned int struct_nested_level;
} tojson_permanent_state_t;


/* ------------------------- receiver / sender ---------------------------------------------------------------------- */

struct _metahandle_t;
typedef struct _metahandle_t metahandle_t;

typedef error_t (*recv_callback_t)(metahandle_t *);
typedef error_t (*send_callback_t)(metahandle_t *);
typedef const char *(*custom_recv_callback_t)(const char *, unsigned int);
typedef int (*custom_send_callback_t)(const char *, unsigned int, const char *);
typedef error_t (*finalize_callback_t)(metahandle_t *);

struct _metahandle_t
{
  int is_receiver;
  union
  {
    struct
    {
      memwriter_t *memwriter;
      size_t message_size;
      recv_callback_t recv;
      union
      {
        struct
        {
          custom_recv_callback_t recv;
          const char *name;
          unsigned int id;
        } custom;
        struct
        {
          int client_socket;
          int server_socket;
        } socket;
      } comm;
    } receiver;
    struct
    {
      memwriter_t *memwriter;
      send_callback_t send;
      union
      {
        struct
        {
          custom_send_callback_t send;
          const char *name;
          unsigned int id;
        } custom;
        struct
        {
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

typedef enum
{
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


/* ------------------------- generic list --------------------------------------------------------------------------- */

#define DECLARE_LIST_TYPE(prefix, entry_type)                                                                          \
  typedef entry_type prefix##_list_entry_t;                                                                            \
  typedef const entry_type prefix##_list_const_entry_t;                                                                \
  typedef error_t (*prefix##_list_entry_copy_func_t)(prefix##_list_entry_t *, prefix##_list_const_entry_t);            \
  typedef error_t (*prefix##_list_entry_delete_func_t)(prefix##_list_entry_t);                                         \
                                                                                                                       \
  typedef struct                                                                                                       \
  {                                                                                                                    \
    prefix##_list_entry_copy_func_t entry_copy;                                                                        \
    prefix##_list_entry_delete_func_t entry_delete;                                                                    \
  } prefix##_list_vtable_t;                                                                                            \
                                                                                                                       \
  typedef struct _##prefix##_list_node_t                                                                               \
  {                                                                                                                    \
    prefix##_list_entry_t entry;                                                                                       \
    struct _##prefix##_list_node_t *next;                                                                              \
  } prefix##_list_node_t;                                                                                              \
                                                                                                                       \
  typedef struct                                                                                                       \
  {                                                                                                                    \
    const prefix##_list_vtable_t *vt;                                                                                  \
    prefix##_list_node_t *head;                                                                                        \
    prefix##_list_node_t *tail;                                                                                        \
    size_t size;                                                                                                       \
  } prefix##_list_t;                                                                                                   \
                                                                                                                       \
  /* ~~~~~~~~~~~~~~~~~~~~~~~ ref list ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */ \
                                                                                                                       \
  typedef entry_type prefix##_reflist_entry_t;                                                                         \
  typedef error_t (*prefix##_reflist_entry_copy_func_t)(prefix##_reflist_entry_t *, prefix##_reflist_entry_t);         \
  typedef error_t (*prefix##_reflist_entry_delete_func_t)(prefix##_reflist_entry_t);                                   \
                                                                                                                       \
  typedef struct                                                                                                       \
  {                                                                                                                    \
    prefix##_reflist_entry_copy_func_t entry_copy;                                                                     \
    prefix##_reflist_entry_delete_func_t entry_delete;                                                                 \
  } prefix##_reflist_vtable_t;                                                                                         \
                                                                                                                       \
  typedef struct _##prefix##_reflist_node_t                                                                            \
  {                                                                                                                    \
    prefix##_reflist_entry_t entry;                                                                                    \
    struct _##prefix##_reflist_node_t *next;                                                                           \
  } prefix##_reflist_node_t;                                                                                           \
                                                                                                                       \
  typedef struct                                                                                                       \
  {                                                                                                                    \
    const prefix##_reflist_vtable_t *vt;                                                                               \
    prefix##_reflist_node_t *head;                                                                                     \
    prefix##_reflist_node_t *tail;                                                                                     \
    size_t size;                                                                                                       \
  } prefix##_reflist_t;


DECLARE_LIST_TYPE(args, gr_meta_args_t *)
DECLARE_LIST_TYPE(dynamic_args_array, dynamic_args_array_t *)
DECLARE_LIST_TYPE(string, char *)


/* ------------------------- generic set ---------------------------------------------------------------------------- */

#define DECLARE_SET_TYPE(prefix, entry_type)           \
  typedef entry_type prefix##_set_entry_t;             \
  typedef const entry_type prefix##_set_const_entry_t; \
                                                       \
  typedef struct                                       \
  {                                                    \
    prefix##_set_entry_t *set;                         \
    unsigned char *used;                               \
    size_t capacity;                                   \
    size_t size;                                       \
  } prefix##_set_t;

DECLARE_SET_TYPE(args, gr_meta_args_t *)


/* ------------------------- string-to-generic map ------------------------------------------------------------------ */

#define DECLARE_MAP_TYPE(prefix, value_type)                     \
  typedef value_type prefix##_map_value_t;                       \
  typedef const value_type prefix##_map_const_value_t;           \
                                                                 \
  typedef struct                                                 \
  {                                                              \
    const char *key;                                             \
    prefix##_map_value_t value;                                  \
  } prefix##_map_entry_t;                                        \
                                                                 \
  DECLARE_SET_TYPE(string_##prefix##_pair, prefix##_map_entry_t) \
  typedef string_##prefix##_pair_set_t prefix##_map_t;

DECLARE_MAP_TYPE(plot_func, plot_func_t)
DECLARE_MAP_TYPE(string, char *)
DECLARE_MAP_TYPE(uint, unsigned int)
DECLARE_MAP_TYPE(args_set, args_set_t *)

#undef DECLARE_MAP_TYPE
#undef DECLARE_SET_TYPE


/* ------------------------- event handling ------------------------------------------------------------------------- */

DECLARE_LIST_TYPE(event, gr_meta_event_t *)

typedef struct
{
  event_reflist_t *queue;
  gr_meta_event_callback_t *event_callbacks;
} event_queue_t;

#undef DECLARE_LIST_TYPE


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
static size_t argparse_calculate_needed_buffer_size(const char *format, int apply_padding);
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
static int args_check_format_compatibility(const arg_t *arg, const char *compatible_format);
static void args_decrease_arg_reference_count(args_node_t *args_node);


/* ------------------------- event handling ------------------------------------------------------------------------- */

static int process_events(void);


/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ general ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static error_t plot_init_static_variables(void);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot arguments ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static error_t plot_merge_args(gr_meta_args_t *args, const gr_meta_args_t *merge_args, const char **hierarchy_name_ptr,
                               uint_map_t *hierarchy_to_id);
static error_t plot_init_arg_structure(arg_t *arg, const char **hierarchy_name_ptr,
                                       unsigned int next_hierarchy_level_max_id);
static error_t plot_init_args_structure(gr_meta_args_t *args, const char **hierarchy_name_ptr,
                                        unsigned int next_hierarchy_level_max_id);
static void plot_set_flag_defaults(void);
static void plot_set_attribute_defaults(gr_meta_args_t *subplot_args);
static void plot_pre_plot(gr_meta_args_t *plot_args);
static void plot_process_wswindow_wsviewport(gr_meta_args_t *plot_args);
static void plot_pre_subplot(gr_meta_args_t *subplot_args);
static void plot_process_colormap(gr_meta_args_t *subplot_args);
static void plot_process_viewport(gr_meta_args_t *subplot_args);
static void plot_process_window(gr_meta_args_t *subplot_args);
static void plot_store_coordinate_ranges(gr_meta_args_t *subplot_args);
static void plot_post_plot(gr_meta_args_t *plot_args);
static void plot_post_subplot(gr_meta_args_t *subplot_args);
static error_t plot_get_args_in_hierarchy(gr_meta_args_t *args, const char **hierarchy_name_start_ptr, const char *key,
                                          uint_map_t *hierarchy_to_id, const gr_meta_args_t **found_args,
                                          const char ***found_hierarchy_ptr);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plotting ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static error_t plot_line(gr_meta_args_t *subplot_args);
static error_t plot_step(gr_meta_args_t *subplot_args);
static error_t plot_scatter(gr_meta_args_t *subplot_args);
static error_t plot_quiver(gr_meta_args_t *subplot_args);
static error_t plot_stem(gr_meta_args_t *subplot_args);
static error_t plot_hist(gr_meta_args_t *subplot_args);
static error_t plot_barplot(gr_meta_args_t *subplot_args);
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
static const char *next_fmt_key(const char *fmt) UNUSED;
static int get_id_from_args(const gr_meta_args_t *args, int *plot_id, int *subplot_id, int *series_id);
static int get_figure_size(const gr_meta_args_t *plot_args, int *pixel_width, int *pixel_height, double *metric_width,
                           double *metric_height);
static gr_meta_args_t *get_subplot_from_ndc_point(double x, double y);
static gr_meta_args_t *get_subplot_from_ndc_points(unsigned int n, const double *x, const double *y);


/* ------------------------- util ----------------------------------------------------------------------------------- */

static size_t djb2_hash(const char *str);
static int is_int_number(const char *str);
static int str_to_uint(const char *str, unsigned int *value_ptr);
static int int_equals_any(int number, unsigned int n, ...);
static int str_equals_any(const char *str, unsigned int n, ...);
static int str_equals_any_in_array(const char *str, const char **str_array);
static int uppercase_count(const char *str);
static unsigned long next_or_equal_power2(unsigned long num);
static int get_focus_and_factor(const int top, const int right, const int bottom, const int left,
                                const int keep_aspect_ratio, double *factor_x, double *factor_y, double *focus_x,
                                double *focus_y, gr_meta_args_t **subplot_args);


/* ========================= methods ================================================================================ */

/* ------------------------- argument ------------------------------------------------------------------------------- */

static args_value_iterator_t *arg_value_iter(const arg_t *arg);

static error_t arg_increase_array(arg_t *arg, size_t increment);

static int arg_first_value(const arg_t *arg, const char *first_value_format, void *first_value,
                           unsigned int *array_length);
#define arg_first_value(arg, first_value_format, first_value, array_length) \
  arg_first_value(arg, first_value_format, (void *)first_value, array_length)
static int arg_values(const arg_t *arg, const char *expected_format, ...);
static int arg_values_vl(const arg_t *arg, const char *expected_format, va_list *vl);

/* ------------------------- argument container --------------------------------------------------------------------- */

static void args_init(gr_meta_args_t *args);
static void args_finalize(gr_meta_args_t *args);

static gr_meta_args_t *args_flatcopy(const gr_meta_args_t *args) UNUSED;
static gr_meta_args_t *args_copy(const gr_meta_args_t *copy_args, const char **keys_copy_as_array,
                                 const char **ignore_keys);

static error_t args_push_common(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                                va_list *vl, int apply_padding);
static error_t args_push_vl(gr_meta_args_t *args, const char *key, const char *value_format, va_list *vl);
static error_t args_push_arg(gr_meta_args_t *args, arg_t *arg);
static error_t args_update_many(gr_meta_args_t *args, const gr_meta_args_t *update_args) UNUSED;
static error_t args_merge(gr_meta_args_t *args, const gr_meta_args_t *merge_args, const char *const *merge_keys);
static error_t args_setdefault_common(gr_meta_args_t *args, const char *key, const char *value_format,
                                      const void *buffer, va_list *vl, int apply_padding);
static error_t args_setdefault(gr_meta_args_t *args, const char *key, const char *value_format, ...);
static error_t args_setdefault_buf(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                                   int apply_padding) UNUSED;
static error_t args_setdefault_vl(gr_meta_args_t *args, const char *key, const char *value_format, va_list *vl);

static void args_clear(gr_meta_args_t *args, const char **exclude_keys);

static error_t args_increase_array(gr_meta_args_t *args, const char *key, size_t increment) UNUSED;

static unsigned int args_count(const gr_meta_args_t *args) UNUSED;

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
static error_t memwriter_insert(memwriter_t *memwriter, int index, const char *str) UNUSED;
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


/* ------------------------- generic list --------------------------------------------------------------------------- */

#define DECLARE_LIST_METHODS(prefix)                                                                                   \
  static prefix##_list_t *prefix##_list_new(void);                                                                     \
  static void prefix##_list_delete(prefix##_list_t *list);                                                             \
                                                                                                                       \
  static error_t prefix##_list_push_front(prefix##_list_t *list, prefix##_list_const_entry_t entry);                   \
  static error_t prefix##_list_push_back(prefix##_list_t *list, prefix##_list_const_entry_t entry);                    \
                                                                                                                       \
  static prefix##_list_entry_t prefix##_list_pop_front(prefix##_list_t *list);                                         \
  static prefix##_list_entry_t prefix##_list_pop_back(prefix##_list_t *list);                                          \
                                                                                                                       \
  static error_t prefix##_list_push(prefix##_list_t *list, prefix##_list_const_entry_t entry);                         \
  static prefix##_list_entry_t prefix##_list_pop(prefix##_list_t *list);                                               \
                                                                                                                       \
  static error_t prefix##_list_enqueue(prefix##_list_t *list, prefix##_list_const_entry_t entry);                      \
  static prefix##_list_entry_t prefix##_list_dequeue(prefix##_list_t *list);                                           \
                                                                                                                       \
  static int prefix##_list_empty(prefix##_list_t *list);                                                               \
                                                                                                                       \
  static error_t prefix##_list_entry_copy(prefix##_list_entry_t *copy, prefix##_list_const_entry_t entry);             \
  static error_t prefix##_list_entry_delete(prefix##_list_entry_t entry);                                              \
                                                                                                                       \
  static int prefix##_list_find_previous_node(const prefix##_list_t *list, const prefix##_list_node_t *node,           \
                                              prefix##_list_node_t **previous_node);                                   \
                                                                                                                       \
  /* ~~~~~~~~~~~~~~~~~~~~~~~ ref list ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */ \
                                                                                                                       \
  static prefix##_reflist_t *prefix##_reflist_new(void) MAYBE_UNUSED;                                                  \
  static void prefix##_reflist_delete(prefix##_reflist_t *list) MAYBE_UNUSED;                                          \
  static void prefix##_reflist_delete_with_entries(prefix##_reflist_t *list) MAYBE_UNUSED;                             \
                                                                                                                       \
  static error_t prefix##_reflist_push_front(prefix##_reflist_t *list, prefix##_reflist_entry_t entry) MAYBE_UNUSED;   \
  static error_t prefix##_reflist_push_back(prefix##_reflist_t *list, prefix##_reflist_entry_t entry) MAYBE_UNUSED;    \
                                                                                                                       \
  static prefix##_reflist_entry_t prefix##_reflist_pop_front(prefix##_reflist_t *list) MAYBE_UNUSED;                   \
  static prefix##_reflist_entry_t prefix##_reflist_pop_back(prefix##_reflist_t *list) MAYBE_UNUSED;                    \
                                                                                                                       \
  static error_t prefix##_reflist_push(prefix##_reflist_t *list, prefix##_reflist_entry_t entry) MAYBE_UNUSED;         \
  static prefix##_reflist_entry_t prefix##_reflist_pop(prefix##_reflist_t *list) MAYBE_UNUSED;                         \
                                                                                                                       \
  static error_t prefix##_reflist_enqueue(prefix##_reflist_t *list, prefix##_reflist_entry_t entry) MAYBE_UNUSED;      \
  static prefix##_reflist_entry_t prefix##_reflist_dequeue(prefix##_reflist_t *list) MAYBE_UNUSED;                     \
                                                                                                                       \
  static int prefix##_reflist_empty(prefix##_reflist_t *list) MAYBE_UNUSED;                                            \
                                                                                                                       \
  static error_t prefix##_reflist_entry_copy(prefix##_reflist_entry_t *copy, const prefix##_reflist_entry_t entry);    \
  static error_t prefix##_reflist_entry_delete(prefix##_reflist_entry_t entry);                                        \
                                                                                                                       \
  static int prefix##_reflist_find_previous_node(const prefix##_reflist_t *list, const prefix##_reflist_node_t *node,  \
                                                 prefix##_reflist_node_t **previous_node) MAYBE_UNUSED;

DECLARE_LIST_METHODS(args)
DECLARE_LIST_METHODS(dynamic_args_array)
DECLARE_LIST_METHODS(string)

/* ------------------------- generic set ---------------------------------------------------------------------------- */

#define DECLARE_SET_METHODS(prefix)                                                                            \
  static prefix##_set_t *prefix##_set_new(size_t capacity) MAYBE_UNUSED;                                       \
  static prefix##_set_t *prefix##_set_new_with_data(size_t count, prefix##_set_entry_t *entries) MAYBE_UNUSED; \
  static prefix##_set_t *prefix##_set_copy(const prefix##_set_t *set) MAYBE_UNUSED;                            \
  static void prefix##_set_delete(prefix##_set_t *set);                                                        \
  static int prefix##_set_add(prefix##_set_t *set, prefix##_set_const_entry_t entry) MAYBE_UNUSED;             \
  static int prefix##_set_find(const prefix##_set_t *set, prefix##_set_const_entry_t entry,                    \
                               prefix##_set_entry_t *saved_entry) MAYBE_UNUSED;                                \
  static int prefix##_set_contains(const prefix##_set_t *set, prefix##_set_const_entry_t entry) MAYBE_UNUSED;  \
  static ssize_t prefix##_set_index(const prefix##_set_t *set, prefix##_set_const_entry_t entry) MAYBE_UNUSED; \
                                                                                                               \
  static int prefix##_set_entry_copy(prefix##_set_entry_t *copy, prefix##_set_const_entry_t entry);            \
  static void prefix##_set_entry_delete(prefix##_set_entry_t entry);                                           \
  static size_t prefix##_set_entry_hash(prefix##_set_const_entry_t entry);                                     \
  static int prefix##_set_entry_equals(prefix##_set_const_entry_t entry1, prefix##_set_const_entry_t entry2);

DECLARE_SET_METHODS(args)


/* ------------------------- string-to-generic map ------------------------------------------------------------------ */

#define DECLARE_MAP_METHODS(prefix)                                                                               \
  DECLARE_SET_METHODS(string_##prefix##_pair)                                                                     \
                                                                                                                  \
  static prefix##_map_t *prefix##_map_new(size_t capacity) MAYBE_UNUSED;                                          \
  static prefix##_map_t *prefix##_map_new_with_data(size_t count, prefix##_map_entry_t *entries) MAYBE_UNUSED;    \
  static prefix##_map_t *prefix##_map_copy(const prefix##_map_t *map) MAYBE_UNUSED;                               \
  static void prefix##_map_delete(prefix##_map_t *prefix##_map) MAYBE_UNUSED;                                     \
  static int prefix##_map_insert(prefix##_map_t *prefix##_map, const char *key, prefix##_map_const_value_t value) \
      MAYBE_UNUSED;                                                                                               \
  static int prefix##_map_insert_default(prefix##_map_t *prefix##_map, const char *key,                           \
                                         prefix##_map_const_value_t value) MAYBE_UNUSED;                          \
  static int prefix##_map_at(const prefix##_map_t *prefix##_map, const char *key, prefix##_map_value_t *value)    \
      MAYBE_UNUSED;                                                                                               \
                                                                                                                  \
  static int prefix##_map_value_copy(prefix##_map_value_t *copy, prefix##_map_const_value_t value);               \
  static void prefix##_map_value_delete(prefix##_map_value_t value);

DECLARE_MAP_METHODS(plot_func)
DECLARE_MAP_METHODS(string)
DECLARE_MAP_METHODS(uint)
DECLARE_MAP_METHODS(args_set)

#undef DECLARE_MAP_METHODS
#undef DECLARE_SET_METHODS


/* ------------------------- event handling ------------------------------------------------------------------------- */

DECLARE_LIST_METHODS(event)

static event_queue_t *event_queue_new(void);
static void event_queue_delete(event_queue_t *queue);

static void event_queue_register(event_queue_t *queue, gr_meta_event_type_t type, gr_meta_event_callback_t callback);
static void event_queue_unregister(event_queue_t *queue, gr_meta_event_type_t type);

static int event_queue_process_next(event_queue_t *queue);
static int event_queue_process_all(event_queue_t *queue);

static error_t event_queue_enqueue_new_plot_event(event_queue_t *queue, int plot_id);
static error_t event_queue_enqueue_update_plot_event(event_queue_t *queue, int plot_id);
static error_t event_queue_enqueue_size_event(event_queue_t *queue, int plot_id, int width, int height);
static error_t event_queue_enqueue_merge_end_event(event_queue_t *queue, const char *identificator);


#undef DECLARE_LIST_METHODS


/* ========================= static variables ======================================================================= */

/* ------------------------- argument parsing ----------------------------------------------------------------------- */

static int argparse_valid_format[128];
static read_param_t argparse_format_to_read_callback[128];
static delete_value_t argparse_format_to_delete_callback[128];
static size_t argparse_format_to_size[128];
static int argparse_format_has_array_terminator[128];
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
const char *plot_hierarchy_names[] = {"root", "plots", "subplots", "series", NULL};
static int plot_scatter_markertypes[] = {
    GKS_K_MARKERTYPE_SOLID_CIRCLE,   GKS_K_MARKERTYPE_SOLID_TRI_UP, GKS_K_MARKERTYPE_SOLID_TRI_DOWN,
    GKS_K_MARKERTYPE_SOLID_SQUARE,   GKS_K_MARKERTYPE_SOLID_BOWTIE, GKS_K_MARKERTYPE_SOLID_HGLASS,
    GKS_K_MARKERTYPE_SOLID_DIAMOND,  GKS_K_MARKERTYPE_SOLID_STAR,   GKS_K_MARKERTYPE_SOLID_TRI_RIGHT,
    GKS_K_MARKERTYPE_SOLID_TRI_LEFT, GKS_K_MARKERTYPE_SOLID_PLUS,   GKS_K_MARKERTYPE_PENTAGON,
    GKS_K_MARKERTYPE_HEXAGON,        GKS_K_MARKERTYPE_HEPTAGON,     GKS_K_MARKERTYPE_OCTAGON,
    GKS_K_MARKERTYPE_STAR_4,         GKS_K_MARKERTYPE_STAR_5,       GKS_K_MARKERTYPE_STAR_6,
    GKS_K_MARKERTYPE_STAR_7,         GKS_K_MARKERTYPE_STAR_8,       GKS_K_MARKERTYPE_VLINE,
    GKS_K_MARKERTYPE_HLINE,          GKS_K_MARKERTYPE_OMARK,        INT_MAX};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ args ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static gr_meta_args_t *global_root_args = NULL;
static gr_meta_args_t *active_plot_args = NULL;
static unsigned int active_plot_index = 0;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ event handling ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static event_queue_t *event_queue = NULL;
static int processing_events = 0;


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind to fmt ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* TODO: Check format of: "hist", "isosurface", "imshow"  */
static string_map_entry_t kind_to_fmt[] = {{"line", "xys"},      {"hexbin", "xys"},
                                           {"polar", "xys"},     {"shade", "xys"},
                                           {"stem", "xys"},      {"step", "xys"},
                                           {"contour", "xyzc"},  {"contourf", "xyzc"},
                                           {"tricont", "xyzc"},  {"trisurf", "xyzc"},
                                           {"surface", "xyzc"},  {"wireframe", "xyzc"},
                                           {"plot3", "xyzc"},    {"scatter", "xyzc"},
                                           {"scatter3", "xyzc"}, {"quiver", "xyuv"},
                                           {"heatmap", "xyzc"},  {"hist", "x"},
                                           {"barplot", "xy"},    {"isosurface", "x"},
                                           {"imshow", ""},       {"nonuniformheatmap", "xyzc"}};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind to func ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static plot_func_map_entry_t kind_to_func[] = {
    {"line", plot_line},
    {"step", plot_step},
    {"scatter", plot_scatter},
    {"quiver", plot_quiver},
    {"stem", plot_stem},
    {"hist", plot_hist},
    {"barplot", plot_barplot},
    {"contour", plot_contour},
    {"contourf", plot_contourf},
    {"hexbin", plot_hexbin},
    {"heatmap", plot_heatmap},
    {"wireframe", plot_wireframe},
    {"surface", plot_surface},
    {"plot3", plot_plot3},
    {"scatter3", plot_scatter3},
    {"imshow", plot_imshow},
    {"isosurface", plot_isosurface},
    {"polar", plot_polar},
    {"trisurf", plot_trisurf},
    {"tricont", plot_tricont},
    {"shade", plot_shade},
    {"nonuniformheatmap", plot_heatmap},
};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ maps ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static string_map_t *fmt_map = NULL;
static plot_func_map_t *plot_func_map = NULL;
static string_map_t *plot_valid_keys_map = NULL;


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot clear ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const char *plot_clear_exclude_keys[] = {"array_index", "in_use", NULL};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot merge ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const char *plot_merge_ignore_keys[] = {"id", "series_id", "subplot_id", "plot_id", "array_index", "in_use", NULL};
const char *plot_merge_clear_keys[] = {"series", NULL};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ valid keys ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* IMPORTANT: Every key should only be part of ONE array -> keys can be assigned to the right object, if a user sends a
 * flat object that mixes keys of different hierarchies */

const char *valid_root_keys[] = {"plots", "append_plots", "hold_plots", NULL};
const char *valid_plot_keys[] = {"clear", "figsize", "size", "subplots", "update", NULL};
const char *valid_subplot_keys[] = {"adjust_xlim",  "adjust_ylim",
                                    "adjust_zlim",  "backgroundcolor",
                                    "colormap",     "keep_aspect_ratio",
                                    "kind",         "labels",
                                    "levels",       "location",
                                    "nbins",        "panzoom",
                                    "reset_ranges", "rotation",
                                    "series",       "subplot",
                                    "tilt",         "title",
                                    "xbins",        "xflip",
                                    "xform",        "xlabel",
                                    "xlim",         "xlog",
                                    "ybins",        "yflip",
                                    "ylabel",       "ylim",
                                    "ylog",         "zflip",
                                    "zlim",         "zlog",
                                    "clim",         NULL};
const char *valid_series_keys[] = {"a", "c", "markertype", "s", "spec", "step_where", "u", "v", "x", "y", "z", NULL};

/* ######################### public implementation ################################################################## */

/* ========================= methods ================================================================================ */

/* ------------------------- argument container --------------------------------------------------------------------- */

gr_meta_args_t *gr_newmeta()
{
  gr_meta_args_t *args = malloc(sizeof(gr_meta_args_t));
  if (args == NULL)
    {
      debug_print_malloc_error();
      return NULL;
    }
  args_init(args);
  return args;
}

void gr_deletemeta(gr_meta_args_t *args)
{
  args_finalize(args);
  free(args);
}

void gr_finalizemeta(void)
{
  if (plot_static_variables_initialized)
    {
      gr_deletemeta(global_root_args);
      global_root_args = NULL;
      active_plot_args = NULL;
      active_plot_index = 0;
      event_queue_delete(event_queue);
      event_queue = NULL;
      processing_events = 0;
      string_map_delete(fmt_map);
      fmt_map = NULL;
      plot_func_map_delete(plot_func_map);
      plot_func_map = NULL;
      string_map_delete(plot_valid_keys_map);
      plot_valid_keys_map = NULL;
      plot_static_variables_initialized = 0;
    }
}

int gr_meta_args_push(gr_meta_args_t *args, const char *key, const char *value_format, ...)
{
  va_list vl;
  error_t error;

  va_start(vl, value_format);

  error = args_push_vl(args, key, value_format, &vl);

  va_end(vl);

  return error == NO_ERROR;
}

int gr_meta_args_push_buf(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                          int apply_padding)
{
  error_t error;

  error = args_push_common(args, key, value_format, buffer, NULL, apply_padding);

  return error == NO_ERROR;
}

int gr_meta_args_contains(const gr_meta_args_t *args, const char *keyword)
{
  return args_at(args, keyword) != NULL;
}

void gr_meta_args_clear(gr_meta_args_t *args)
{
  args_clear(args, plot_clear_exclude_keys);
}

void gr_meta_args_remove(gr_meta_args_t *args, const char *key)
{
  args_node_t *tmp_node, *previous_node_by_keyword;

  if (args_find_previous_node(args, key, &previous_node_by_keyword))
    {
      if (previous_node_by_keyword == NULL)
        {
          tmp_node = args->kwargs_head->next;
          args_decrease_arg_reference_count(args->kwargs_head);
          free(args->kwargs_head);
          args->kwargs_head = tmp_node;
          if (tmp_node == NULL)
            {
              args->kwargs_tail = NULL;
            }
        }
      else
        {
          tmp_node = previous_node_by_keyword->next->next;
          args_decrease_arg_reference_count(previous_node_by_keyword->next);
          free(previous_node_by_keyword->next);
          previous_node_by_keyword->next = tmp_node;
          if (tmp_node == NULL)
            {
              args->kwargs_tail = previous_node_by_keyword;
            }
        }
      --(args->count);
    }
}


/* ------------------------- plot ----------------------------------------------------------------------------------- */

int gr_clearmeta(void)
{
  if (plot_init_static_variables() != NO_ERROR)
    {
      return 0;
    }
  gr_meta_args_clear(active_plot_args);
  if (plot_init_args_structure(active_plot_args, plot_hierarchy_names + 1, 1) != NO_ERROR)
    {
      return 0;
    }

  return 1;
}

int gr_mergemeta_named(const gr_meta_args_t *args, const char *identificator)
{
  if (plot_init_static_variables() != NO_ERROR)
    {
      return 0;
    }
  if (args != NULL)
    {
      if (plot_merge_args(global_root_args, args, NULL, NULL) != NO_ERROR)
        {
          return 0;
        }
    }

  process_events();
  event_queue_enqueue_merge_end_event(event_queue, identificator);
  process_events();

  return 1;
}

int gr_mergemeta(const gr_meta_args_t *args)
{
  return gr_mergemeta_named(args, "");
}

int gr_plotmeta(const gr_meta_args_t *args)
{
  gr_meta_args_t **current_subplot_args;
  plot_func_t plot_func;
  const char *kind = NULL;
  if (!gr_mergemeta(args))
    {
      return 0;
    }

  plot_set_attribute_defaults(active_plot_args);
  plot_pre_plot(active_plot_args);
  args_values(active_plot_args, "subplots", "A", &current_subplot_args);
  while (*current_subplot_args != NULL)
    {
      plot_pre_subplot(*current_subplot_args);
      args_values(*current_subplot_args, "kind", "s", &kind);
      logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));
      if (!plot_func_map_at(plot_func_map, kind, &plot_func))
        {
          return 0;
        }
      if (plot_func(*current_subplot_args) != NO_ERROR)
        {
          return 0;
        };
      plot_post_subplot(*current_subplot_args);
      ++current_subplot_args;
    }
  plot_post_plot(active_plot_args);

  process_events();

#ifndef NDEBUG
  logger((stderr, "root args after \"gr_plotmeta\" (active_plot_index: %d):\n", active_plot_index - 1));
  gr_dumpmeta(global_root_args, stderr);
#endif

  return 1;
}

int gr_switchmeta(unsigned int id)
{
  gr_meta_args_t **args_array = NULL;
  unsigned int args_array_length = 0;

  if (plot_init_static_variables() != NO_ERROR)
    {
      return 0;
    }

  if (plot_init_args_structure(global_root_args, plot_hierarchy_names, id + 1) != NO_ERROR)
    {
      return 0;
    }
  if (!args_first_value(global_root_args, "plots", "A", &args_array, &args_array_length))
    {
      return 0;
    }
  if (id + 1 > args_array_length)
    {
      return 0;
    }

  active_plot_index = id + 1;
  active_plot_args = args_array[id];

  return 1;
}

int gr_registermeta(gr_meta_event_type_t type, gr_meta_event_callback_t callback)
{
  if (plot_init_static_variables() != NO_ERROR)
    {
      return 0;
    }

  event_queue_register(event_queue, type, callback);

  return 1;
}

int gr_unregistermeta(gr_meta_event_type_t type)
{
  if (plot_init_static_variables() != NO_ERROR)
    {
      return 0;
    }

  event_queue_unregister(event_queue, type);

  return 1;
}

unsigned int gr_meta_max_plotid(void)
{
  unsigned int args_array_length = 0;

  if (args_first_value(global_root_args, "plots", "A", NULL, &args_array_length))
    {
      --args_array_length;
    }

  return args_array_length;
}


/* ------------------------- receiver / sender ---------------------------------------------------------------------- */

void *gr_openmeta(int is_receiver, const char *name, unsigned int id,
                  const char *(*custom_recv)(const char *, unsigned int),
                  int (*custom_send)(const char *, unsigned int, const char *))
{
  metahandle_t *handle;
  error_t error = NO_ERROR;

  handle = malloc(sizeof(metahandle_t));
  if (handle == NULL)
    {
      return NULL;
    }
  handle->is_receiver = is_receiver;
  handle->sender_receiver.receiver.comm.custom.recv = custom_recv;
  if (is_receiver)
    {
      if (custom_recv != NULL)
        {
          error = receiver_init_for_custom(handle, name, id, custom_recv);
        }
      else
        {
          error = receiver_init_for_socket(handle, name, id);
        }
    }
  else
    {
      if (custom_send != NULL)
        {
          error = sender_init_for_custom(handle, name, id, custom_send);
        }
      else
        {
          error = sender_init_for_socket(handle, name, id);
        }
    }

  if (error != NO_ERROR)
    {
      if (error != ERROR_NETWORK_WINSOCK_INIT)
        {
          handle->finalize(handle);
        }
      free(handle);
      handle = NULL;
    }

  return (void *)handle;
}

void gr_closemeta(const void *p)
{
  metahandle_t *handle = (metahandle_t *)p;

  handle->finalize(handle);
  free(handle);
}


/* ------------------------- receiver ------------------------------------------------------------------------------- */

gr_meta_args_t *gr_recvmeta(const void *p, gr_meta_args_t *args)
{
  metahandle_t *handle = (metahandle_t *)p;
  int created_args = 0;

  if (args == NULL)
    {
      args = gr_newmeta();
      if (args == NULL)
        {
          goto error_cleanup;
        }
      created_args = 1;
    }

  if (handle->sender_receiver.receiver.recv(handle) != NO_ERROR)
    {
      goto error_cleanup;
    }
  if (fromjson_read(args, memwriter_buf(handle->sender_receiver.receiver.memwriter)) != NO_ERROR)
    {
      goto error_cleanup;
    }

  if (memwriter_erase(handle->sender_receiver.receiver.memwriter, 0,
                      handle->sender_receiver.receiver.message_size + 1) != NO_ERROR)
    {
      goto error_cleanup;
    }

  return args;

error_cleanup:
  if (created_args)
    {
      gr_deletemeta(args);
    }

  return NULL;
}


/* ------------------------- sender --------------------------------------------------------------------------------- */

int gr_sendmeta(const void *p, const char *data_desc, ...)
{
  metahandle_t *handle = (metahandle_t *)p;
  va_list vl;
  error_t error;

  va_start(vl, data_desc);
  error = tojson_write_vl(handle->sender_receiver.sender.memwriter, data_desc, &vl);
  if (error == NO_ERROR && tojson_is_complete() && handle->sender_receiver.sender.send != NULL)
    {
      error = handle->sender_receiver.sender.send(handle);
    }
  va_end(vl);

  return error == NO_ERROR;
}

int gr_sendmeta_buf(const void *p, const char *data_desc, const void *buffer, int apply_padding)
{
  metahandle_t *handle = (metahandle_t *)p;
  error_t error;

  error = tojson_write_buf(handle->sender_receiver.sender.memwriter, data_desc, buffer, apply_padding);
  if (error == NO_ERROR && tojson_is_complete() && handle->sender_receiver.sender.send != NULL)
    {
      error = handle->sender_receiver.sender.send(handle);
    }

  return error == NO_ERROR;
}

int gr_sendmeta_ref(const void *p, const char *key, char format, const void *ref, int len)
{
  static const char VALID_OPENING_BRACKETS[] = "([{";
  static const char VALID_CLOSING_BRACKETS[] = ")]}";
  static const char VALID_SEPARATOR[] = ",";
  static gr_meta_args_t *current_args = NULL;
  static dynamic_args_array_t *current_args_array = NULL;
  char *_key = NULL;
  metahandle_t *handle = (metahandle_t *)p;
  char format_string[SENDMETA_REF_FORMAT_MAX_LENGTH];
  error_t error = NO_ERROR;

  if (tojson_struct_nested_level() == 0)
    {
      gr_sendmeta(handle, "o(");
    }
  if (strchr("soO", format) == NULL)
    {
      /* handle general cases (values and arrays) */
      if (islower(format))
        {
          if (current_args_array == NULL)
            {
              snprintf(format_string, SENDMETA_REF_FORMAT_MAX_LENGTH, "%s:%c,", key, format);
              error = gr_sendmeta_buf(handle, format_string, ref, 1);
            }
          else
            {
              snprintf(format_string, SENDMETA_REF_FORMAT_MAX_LENGTH, "%c", format);
              /* TODO: add error return value to `gr_meta_args_push_arg` (?) */
              gr_meta_args_push_buf(current_args, key, format_string, ref, 1);
            }
        }
      else
        {
          if (current_args_array == NULL)
            {
              snprintf(format_string, SENDMETA_REF_FORMAT_MAX_LENGTH, "%s:n%c,", key, format);
              error = gr_sendmeta(handle, format_string, len, ref);
            }
          else
            {
              snprintf(format_string, SENDMETA_REF_FORMAT_MAX_LENGTH, "n%c", format);
              gr_meta_args_push(current_args, key, format_string, len, ref);
            }
        }
    }
  else
    {
      static args_reflist_t *args_stack = NULL;
      static dynamic_args_array_reflist_t *args_array_stack = NULL;
      static string_list_t *key_stack = NULL;
      /* handle special cases (strings, objects and arrays of objects) */
      switch (format)
        {
        case 's':
          if (current_args_array == NULL)
            {
              snprintf(format_string, SENDMETA_REF_FORMAT_MAX_LENGTH, "%s:s,", key);
              error = gr_sendmeta(handle, format_string, ref);
            }
          else
            {
              gr_meta_args_push(current_args, key, "s", ref);
            }
          break;
        case 'o':
          if (strchr(VALID_OPENING_BRACKETS, *(const char *)ref))
            {
              if (current_args_array == NULL)
                {
                  snprintf(format_string, SENDMETA_REF_FORMAT_MAX_LENGTH, "%s:o(,", key);
                  gr_sendmeta(handle, format_string);
                }
              else
                {
                  if (args_stack == NULL)
                    {
                      args_stack = args_reflist_new();
                      if (args_stack == NULL)
                        {
                          error = ERROR_MALLOC;
                          break;
                        }
                    }
                  if (key_stack == NULL)
                    {
                      key_stack = string_list_new();
                      if (key_stack == NULL)
                        {
                          error = ERROR_MALLOC;
                          break;
                        }
                    }
                  if ((error = args_reflist_push(args_stack, current_args)) != NO_ERROR)
                    {
                      break;
                    }
                  if ((error = string_list_push(key_stack, key)) != NO_ERROR)
                    {
                      break;
                    }
                  current_args = gr_newmeta();
                  if (current_args == NULL)
                    {
                      error = ERROR_MALLOC;
                      break;
                    }
                }
            }
          else if (strchr(VALID_CLOSING_BRACKETS, *(const char *)ref))
            {
              if (current_args_array == NULL)
                {
                  gr_sendmeta(handle, ")");
                }
              else
                {
                  gr_meta_args_t *previous_args = args_reflist_pop(args_stack);
                  _key = string_list_pop(key_stack);
                  gr_meta_args_push(previous_args, _key, "a", current_args);
                  current_args = previous_args;
                  if (args_reflist_empty(args_stack))
                    {
                      args_reflist_delete_with_entries(args_stack);
                      args_stack = NULL;
                    }
                  if (string_list_empty(key_stack))
                    {
                      string_list_delete(key_stack);
                      key_stack = NULL;
                    }
                }
            }
          break;
        case 'O':
          if (strchr(VALID_OPENING_BRACKETS, *(const char *)ref))
            {
              if (current_args_array != NULL)
                {
                  if (args_array_stack == NULL)
                    {
                      args_array_stack = dynamic_args_array_reflist_new();
                      if (args_array_stack == NULL)
                        {
                          error = ERROR_MALLOC;
                          break;
                        }
                    }
                  if ((error = dynamic_args_array_reflist_push(args_array_stack, current_args_array)) != NO_ERROR)
                    {
                      break;
                    }
                }
              if (current_args != NULL)
                {
                  if (args_stack == NULL)
                    {
                      args_stack = args_reflist_new();
                      if (args_stack == NULL)
                        {
                          error = ERROR_MALLOC;
                          break;
                        }
                    }
                  if ((error = args_reflist_push(args_stack, current_args)) != NO_ERROR)
                    {
                      break;
                    }
                }
              if (key_stack == NULL)
                {
                  key_stack = string_list_new();
                  if (key_stack == NULL)
                    {
                      error = ERROR_MALLOC;
                      break;
                    }
                }
              if ((error = string_list_push(key_stack, key)) != NO_ERROR)
                {
                  break;
                }
              current_args_array = dynamic_args_array_new();
              if (current_args_array == NULL)
                {
                  error = ERROR_MALLOC;
                  break;
                }
              current_args = gr_newmeta();
              if (current_args == NULL)
                {
                  error = ERROR_MALLOC;
                  break;
                }
              if ((error = dynamic_args_array_push_back(current_args_array, current_args)) != NO_ERROR)
                {
                  break;
                }
            }
          else if (strchr(VALID_SEPARATOR, *(const char *)ref))
            {
              current_args = gr_newmeta();
              if (current_args == NULL)
                {
                  error = ERROR_MALLOC;
                  break;
                }
              assert(current_args_array != NULL);
              if ((error = dynamic_args_array_push_back(current_args_array, current_args)) != NO_ERROR)
                {
                  break;
                }
            }
          else if (strchr(VALID_CLOSING_BRACKETS, *(const char *)ref))
            {
              assert(key_stack != NULL);
              _key = string_list_pop(key_stack);
              if (args_array_stack != NULL)
                {
                  current_args = args_reflist_pop(args_stack);
                  gr_meta_args_push(current_args, _key, "nA", current_args_array->size, current_args_array->buf);
                  dynamic_args_array_delete(current_args_array);
                  current_args_array = dynamic_args_array_reflist_pop(args_array_stack);
                  if (dynamic_args_array_reflist_empty(args_array_stack))
                    {
                      dynamic_args_array_reflist_delete_with_entries(args_array_stack);
                      args_array_stack = NULL;
                    }
                }
              else
                {
                  snprintf(format_string, SENDMETA_REF_FORMAT_MAX_LENGTH, "%s:nA,", _key);
                  gr_sendmeta(handle, format_string, current_args_array->size, current_args_array->buf);
                  dynamic_args_array_delete_with_elements(current_args_array);
                  current_args_array = NULL;
                  current_args = NULL;
                }
              if (string_list_empty(key_stack))
                {
                  string_list_delete(key_stack);
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

int gr_sendmeta_args(const void *p, const gr_meta_args_t *args)
{
  metahandle_t *handle = (metahandle_t *)p;
  error_t error;

  error = tojson_write_args(handle->sender_receiver.sender.memwriter, args);
  if (error == NO_ERROR && tojson_is_complete() && handle->sender_receiver.sender.send != NULL)
    {
      error = handle->sender_receiver.sender.send(handle);
    }

  return error == NO_ERROR;
}


/* ------------------------- user interaction ----------------------------------------------------------------------- */

int gr_inputmeta(const gr_meta_args_t *input_args)
{
  /*
   * reset_ranges:
   * - `x`, `y`: mouse cursor position
   * - `key`: Pressed key (as string)
   * zoom:
   * - `x`, `y`: start point
   * - `angle_delta`: mouse wheel rotation angle in eighths of a degree, can be replaced by factor (double type)
   * - `factor`: zoom factor, can be replaced by angle_delta (double type)
   * box zoom:
   * - `x1`, `y1`, `x2`, `y2`: coordinates of a box selection, (x1, y1) is the fixed corner
   * - `keep_aspect_ratio`: if set to `1`, the aspect ratio of the gr window is preserved (defaults to `1`)
   * pan:
   * - `x`, `y`: start point
   * - `xshift`, `yshift`: shift in x and y direction
   *
   * All coordinates are expected to be given as workstation coordinates (integer type)
   */
  int width, height, max_width_height;
  int x, y, x1, y1, x2, y2;
  gr_meta_args_t *subplot_args;
  const double *viewport;
  double viewport_mid_x, viewport_mid_y;

  logger((stderr, "Processing input\n"));

  get_figure_size(NULL, &width, &height, NULL, NULL);
  max_width_height = max(width, height);
  logger((stderr, "Using size (%d, %d)\n", width, height));

  if (args_values(input_args, "x", "i", &x) && args_values(input_args, "y", "i", &y))
    {
      double ndc_x, ndc_y;
      char *key;

      ndc_x = (double)x / max_width_height;
      ndc_y = (double)(height - y) / max_width_height;
      logger((stderr, "x: %d, y: %d, ndc_x: %lf, ndc_y: %lf\n", x, y, ndc_x, ndc_y));

      subplot_args = get_subplot_from_ndc_point(ndc_x, ndc_y);

      if (args_values(input_args, "key", "s", &key))
        {
          logger((stderr, "Got key \"%s\"\n", key));

          if (strcmp(key, "r") == 0)
            {
              if (subplot_args != NULL)
                {
                  logger((stderr, "Reset single subplot coordinate ranges\n"));
                  gr_meta_args_push(subplot_args, "reset_ranges", "i", 1);
                }
              else
                {
                  gr_meta_args_t **subplot_args_ptr;
                  logger((stderr, "Reset all subplot coordinate ranges\n"));
                  args_values(active_plot_args, "subplots", "A", &subplot_args_ptr);
                  while (*subplot_args_ptr != NULL)
                    {
                      gr_meta_args_push(*subplot_args_ptr, "reset_ranges", "i", 1);
                      ++subplot_args_ptr;
                    }
                }
            }

          return 1;
        }

      if (subplot_args != NULL)
        {
          double angle_delta, factor;
          int xshift, yshift;
          args_values(subplot_args, "viewport", "D", &viewport);

          if (args_values(input_args, "angle_delta", "d", &angle_delta))
            {
              double focus_x, focus_y;

              viewport_mid_x = (viewport[0] + viewport[1]) / 2.0;
              viewport_mid_y = (viewport[2] + viewport[3]) / 2.0;
              focus_x = ndc_x - viewport_mid_x;
              focus_y = ndc_y - viewport_mid_y;
              logger((stderr, "Zoom to ndc focus point (%lf, %lf), angle_delta %lf\n", focus_x, focus_y, angle_delta));
              gr_meta_args_push(subplot_args, "panzoom", "ddd", focus_x, focus_y,
                                1.0 - INPUTMETA_ANGLE_DELTA_FACTOR * angle_delta);

              return 1;
            }
          else if (args_values(input_args, "factor", "d", &factor))
            {
              double focus_x, focus_y;

              viewport_mid_x = (viewport[0] + viewport[1]) / 2.0;
              viewport_mid_y = (viewport[2] + viewport[3]) / 2.0;
              focus_x = ndc_x - viewport_mid_x;
              focus_y = ndc_y - viewport_mid_y;
              logger((stderr, "Zoom to ndc focus point (%lf, %lf), factor %lf\n", focus_x, focus_y, factor));
              gr_meta_args_push(subplot_args, "panzoom", "ddd", focus_x, focus_y, factor);

              return 1;
            }

          if (args_values(input_args, "xshift", "i", &xshift) && args_values(input_args, "yshift", "i", &yshift))
            {
              double ndc_xshift, ndc_yshift;

              ndc_xshift = (double)-xshift / max_width_height;
              ndc_yshift = (double)yshift / max_width_height;
              logger((stderr, "Translate by ndc coordinates (%lf, %lf)\n", ndc_xshift, ndc_yshift));
              gr_meta_args_push(subplot_args, "panzoom", "ddd", ndc_xshift, ndc_yshift, 0.0);

              return 1;
            }
        }
    }

  if (args_values(input_args, "x1", "i", &x1) && args_values(input_args, "x2", "i", &x2) &&
      args_values(input_args, "y1", "i", &y1) && args_values(input_args, "y2", "i", &y2))
    {
      double focus_x, focus_y, factor_x, factor_y;
      int keep_aspect_ratio = INPUTMETA_DEFAULT_KEEP_ASPECT_RATIO;

      args_values(input_args, "keep_aspect_ratio", "i", &keep_aspect_ratio);

      if (!get_focus_and_factor(x1, y1, x2, y2, keep_aspect_ratio, &factor_x, &factor_y, &focus_x, &focus_y,
                                &subplot_args))
        {
          return 0;
        }

      logger((stderr, "Got widget size: (%d, %d)\n", width, height));
      logger((stderr, "Got box: (%d, %d, %d, %d)\n", x1, y1, x2, y2));
      logger((stderr, "zoom focus: (%lf, %lf)\n", focus_x, focus_y));
      logger((stderr, "zoom factors: (%lf, %lf)\n", factor_x, factor_y));

      gr_meta_args_push(subplot_args, "panzoom", "dddd", focus_x, focus_y, factor_x, factor_y);

      return 1;
    }

  return 0;
}

int gr_meta_get_box(const int x1, const int y1, const int x2, const int y2, const int keep_aspect_ratio, int *x, int *y,
                    int *w, int *h)
{
  int width, height, max_width_height;
  double focus_x, focus_y, factor_x, factor_y;
  double viewport_mid_x, viewport_mid_y;
  const double *viewport, *wswindow;
  gr_meta_args_t *subplot_args;
  get_figure_size(NULL, &width, &height, NULL, NULL);
  max_width_height = max(width, height);
  if (!get_focus_and_factor(x1, y1, x2, y2, keep_aspect_ratio, &factor_x, &factor_y, &focus_x, &focus_y, &subplot_args))
    {
      return 0;
    }
  args_values(active_plot_args, "wswindow", "D", &wswindow);
  args_values(subplot_args, "viewport", "D", &viewport);
  viewport_mid_x = (viewport[1] + viewport[0]) / 2.0;
  viewport_mid_y = (viewport[3] + viewport[2]) / 2.0;
  *w = (int)round(factor_x * width * (viewport[1] - viewport[0]) / (wswindow[1] - wswindow[0]));
  *h = (int)round(factor_y * height * (viewport[3] - viewport[2]) / (wswindow[3] - wswindow[2]));
  *x = (int)round(((viewport_mid_x + focus_x) - ((viewport_mid_x + focus_x) - viewport[0]) * factor_x) *
                  max_width_height);
  *y = (int)round(height - ((viewport_mid_y + focus_y) - ((viewport_mid_y + focus_y) - viewport[3]) * factor_y) *
                               max_width_height);
  return 1;
}

/* ######################### private implementation ################################################################# */

/* ========================= functions ============================================================================== */

/* ------------------------- argument parsing ----------------------------------------------------------------------- */

void *argparse_read_params(const char *format, const void *buffer, va_list *vl, int apply_padding, char **new_format)
{
  char *fmt, *current_format, first_format_char;
  size_t needed_buffer_size;
  void *save_buffer;
  argparse_state_t state;

  argparse_init_static_variables();

  /* copy format string since it is modified during the parsing process */
  fmt = gks_strdup(format);
  if (fmt == NULL)
    {
      debug_print_malloc_error();
      return NULL;
    }

  /* get needed save_buffer size to store all parameters and allocate memory */
  needed_buffer_size = argparse_calculate_needed_buffer_size(fmt, apply_padding);
  if (needed_buffer_size > 0)
    {
      save_buffer = malloc(needed_buffer_size);
      if (save_buffer == NULL)
        {
          debug_print_malloc_error();
          free(fmt);
          return NULL;
        }
    }
  else
    {
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
  while (*current_format)
    {
      state.current_format = tolower(*current_format);
      if (state.current_format != *current_format)
        {
          state.next_is_array = 1;
        }
      argparse_read_next_option(&state, &current_format);
      state.save_buffer =
          ((char *)state.save_buffer) + argparse_calculate_needed_padding(state.save_buffer, state.current_format);
      argparse_format_to_read_callback[(unsigned char)state.current_format](&state);
      state.next_is_array = 0;
      state.next_array_length = -1;
      if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, tolower(*current_format)) != NULL)
        {
          ++state.dataslot_count;
          if (state.dataslot_count == 1)
            {
              first_format_char = *current_format;
            }
        }
      ++current_format;
    }

  /* Reset the save buffer since it was shifted during the parsing process */
  state.save_buffer = save_buffer;
  if (state.dataslot_count > 1 && islower(first_format_char) && new_format != NULL)
    {
      *new_format = argparse_convert_to_array(&state);
    }

  /* cleanup */
  free(fmt);

  return state.save_buffer;
}

#define CHECK_PADDING(type)                                               \
  do                                                                      \
    {                                                                     \
      if (state->in_buffer != NULL && state->apply_padding)               \
        {                                                                 \
          ptrdiff_t needed_padding = state->data_offset % sizeof(type);   \
          state->in_buffer = ((char *)state->in_buffer) + needed_padding; \
          state->data_offset += needed_padding;                           \
        }                                                                 \
    }                                                                     \
  while (0)

#define READ_TYPE(type, terminate_array)                                                                       \
  void argparse_read_##type(argparse_state_t *state)                                                           \
  {                                                                                                            \
    size_t *size_t_typed_buffer;                                                                               \
    type *typed_buffer, **pointer_typed_buffer, *src_ptr;                                                      \
    size_t current_array_length;                                                                               \
                                                                                                               \
    if (state->next_is_array)                                                                                  \
      {                                                                                                        \
        current_array_length =                                                                                 \
            (state->next_array_length >= 0) ? state->next_array_length : state->default_array_length;          \
        size_t_typed_buffer = state->save_buffer;                                                              \
        *size_t_typed_buffer = current_array_length;                                                           \
        pointer_typed_buffer = (type **)++size_t_typed_buffer;                                                 \
        if (current_array_length + (terminate_array ? 1 : 0) > 0)                                              \
          {                                                                                                    \
            *pointer_typed_buffer = malloc((current_array_length + (terminate_array ? 1 : 0)) * sizeof(type)); \
          }                                                                                                    \
        else                                                                                                   \
          {                                                                                                    \
            *pointer_typed_buffer = NULL;                                                                      \
          }                                                                                                    \
        if (current_array_length > 0)                                                                          \
          {                                                                                                    \
            if (state->in_buffer != NULL)                                                                      \
              {                                                                                                \
                CHECK_PADDING(type **);                                                                        \
                src_ptr = *(type **)state->in_buffer;                                                          \
              }                                                                                                \
            else                                                                                               \
              {                                                                                                \
                src_ptr = va_arg(*state->vl, type *);                                                          \
              }                                                                                                \
            if (*pointer_typed_buffer != NULL)                                                                 \
              {                                                                                                \
                memcpy(*pointer_typed_buffer, src_ptr, current_array_length * sizeof(type));                   \
                if (terminate_array)                                                                           \
                  {                                                                                            \
                    /* cast to `type ***` instead of `type **` to ensure that `= NULL` is always a syntactical \
                     * valid statement (it wouldn't for primary types like `int` and `double`);                \
                     * for non-pointer types the following statement is never executed, so it should be fine   \
                     */                                                                                        \
                    (*(type ***)pointer_typed_buffer)[current_array_length] = NULL; /* array terminator */     \
                  }                                                                                            \
              }                                                                                                \
            else                                                                                               \
              {                                                                                                \
                debug_print_malloc_error();                                                                    \
              }                                                                                                \
            if (state->in_buffer != NULL)                                                                      \
              {                                                                                                \
                state->in_buffer = ((type **)state->in_buffer) + 1;                                            \
                state->data_offset += sizeof(type *);                                                          \
              }                                                                                                \
            state->save_buffer = ++pointer_typed_buffer;                                                       \
          }                                                                                                    \
      }                                                                                                        \
    else                                                                                                       \
      {                                                                                                        \
        typed_buffer = state->save_buffer;                                                                     \
        if (state->in_buffer != NULL)                                                                          \
          {                                                                                                    \
            CHECK_PADDING(type);                                                                               \
            *typed_buffer = *((type *)state->in_buffer);                                                       \
            state->in_buffer = ((type *)state->in_buffer) + 1;                                                 \
            state->data_offset += sizeof(type);                                                                \
          }                                                                                                    \
        else                                                                                                   \
          {                                                                                                    \
            *typed_buffer = va_arg(*state->vl, type);                                                          \
          }                                                                                                    \
        state->save_buffer = ++typed_buffer;                                                                   \
      }                                                                                                        \
  }

READ_TYPE(int, 0)
READ_TYPE(double, 0)
READ_TYPE(gr_meta_args_ptr_t, 1)

#undef READ_TYPE


void argparse_read_char(argparse_state_t *state)
{
  if (state->next_is_array)
    {
      argparse_read_char_array(state, 1);
    }
  else
    {
      char *typed_buffer = state->save_buffer;
      if (state->in_buffer != NULL)
        {
          *typed_buffer = *((char *)state->in_buffer);
          state->in_buffer = (char *)state->in_buffer + 1;
          state->data_offset += sizeof(char);
        }
      else
        {
          *typed_buffer = va_arg(*state->vl, int); /* char is promoted to int */
        }
      state->save_buffer = ++typed_buffer;
    }
}


void argparse_read_string(argparse_state_t *state)
{
  if (state->next_is_array)
    {
      size_t *size_t_typed_buffer;
      char ***pointer_typed_buffer;
      const char **src_ptr;
      size_t current_array_length;

      current_array_length = (state->next_array_length >= 0) ? state->next_array_length : state->default_array_length;
      if (state->in_buffer != NULL)
        {
          CHECK_PADDING(char **);
          src_ptr = *(const char ***)state->in_buffer;
        }
      else
        {
          src_ptr = va_arg(*state->vl, const char **);
        }
      size_t_typed_buffer = state->save_buffer;
      *size_t_typed_buffer = current_array_length;
      pointer_typed_buffer = (char ***)++size_t_typed_buffer;
      *pointer_typed_buffer = malloc((current_array_length + 1) * sizeof(char *));
      if (*pointer_typed_buffer != NULL)
        {
          int found_malloc_fail;
          unsigned int i;
          for (i = 0; i < current_array_length; i++)
            {
              (*pointer_typed_buffer)[i] = malloc(strlen(src_ptr[i]) + 1);
            }
          found_malloc_fail = 0;
          for (i = 0; i < current_array_length && !found_malloc_fail; i++)
            {
              if ((*pointer_typed_buffer)[i] == NULL)
                {
                  found_malloc_fail = 1;
                }
            }
          if (!found_malloc_fail)
            {
              for (i = 0; i < current_array_length; i++)
                {
                  size_t current_string_length;
                  current_string_length = strlen(src_ptr[i]);
                  memcpy((*pointer_typed_buffer)[i], src_ptr[i], current_string_length);
                  (*pointer_typed_buffer)[i][current_string_length] = '\0';
                }
              (*pointer_typed_buffer)[current_array_length] = NULL; /* array terminator */
            }
          else
            {
              for (i = 0; i < current_array_length; i++)
                {
                  free((*pointer_typed_buffer)[i]);
                }
              free(*pointer_typed_buffer);
              debug_print_malloc_error();
            }
        }
      else
        {
          debug_print_malloc_error();
        }
      if (state->in_buffer != NULL)
        {
          state->in_buffer = ((char ***)state->in_buffer) + 1;
          state->data_offset += sizeof(char **);
        }
      state->save_buffer = ++pointer_typed_buffer;
    }
  else
    {
      argparse_read_char_array(state, 0);
    }
}


void argparse_read_default_array_length(argparse_state_t *state)
{
  if (state->in_buffer != NULL)
    {
      const size_t *typed_buffer = state->in_buffer;
      CHECK_PADDING(size_t);
      state->default_array_length = *typed_buffer;
      state->in_buffer = typed_buffer + 1;
      state->data_offset += sizeof(size_t);
    }
  else
    {
      state->default_array_length = va_arg(*state->vl, int);
    }
}


/* helper function */
void argparse_read_char_array(argparse_state_t *state, int store_array_length)
{
  char **pointer_typed_buffer;
  const char *src_ptr;
  size_t current_array_length;

  if (state->in_buffer != NULL)
    {
      CHECK_PADDING(char *);
      src_ptr = *(char **)state->in_buffer;
    }
  else
    {
      src_ptr = va_arg(*state->vl, char *);
    }
  current_array_length = (state->next_array_length >= 0) ? state->next_array_length : (int)strlen(src_ptr);
  if (store_array_length)
    {
      size_t *size_t_typed_buffer = state->save_buffer;
      *size_t_typed_buffer = current_array_length;
      pointer_typed_buffer = (char **)++size_t_typed_buffer;
    }
  else
    {
      pointer_typed_buffer = (char **)state->save_buffer;
    }
  *pointer_typed_buffer = malloc(current_array_length + 1);
  if (*pointer_typed_buffer != NULL)
    {
      memcpy(*pointer_typed_buffer, src_ptr, current_array_length);
      (*pointer_typed_buffer)[current_array_length] = '\0';
    }
  else
    {
      debug_print_malloc_error();
    }
  if (state->in_buffer != NULL)
    {
      state->in_buffer = ((char **)state->in_buffer) + 1;
      state->data_offset += sizeof(char *);
    }
  state->save_buffer = ++pointer_typed_buffer;
}

#undef CHECK_PADDING

void argparse_init_static_variables()
{
  if (!argparse_static_variables_initialized)
    {
      argparse_valid_format['n'] = 1;
      argparse_valid_format['i'] = 1;
      argparse_valid_format['I'] = 1;
      argparse_valid_format['d'] = 1;
      argparse_valid_format['D'] = 1;
      argparse_valid_format['c'] = 1;
      argparse_valid_format['C'] = 1;
      argparse_valid_format['s'] = 1;
      argparse_valid_format['S'] = 1;
      argparse_valid_format['a'] = 1;
      argparse_valid_format['A'] = 1;

      argparse_format_to_read_callback['i'] = argparse_read_int;
      argparse_format_to_read_callback['d'] = argparse_read_double;
      argparse_format_to_read_callback['c'] = argparse_read_char;
      argparse_format_to_read_callback['s'] = argparse_read_string;
      argparse_format_to_read_callback['a'] = argparse_read_gr_meta_args_ptr_t;
      argparse_format_to_read_callback['n'] = argparse_read_default_array_length;

      argparse_format_to_delete_callback['s'] = free;
      argparse_format_to_delete_callback['a'] = (delete_value_t)gr_deletemeta;

      argparse_format_to_size['i'] = sizeof(int);
      argparse_format_to_size['I'] = sizeof(int *);
      argparse_format_to_size['d'] = sizeof(double);
      argparse_format_to_size['D'] = sizeof(double *);
      argparse_format_to_size['c'] = sizeof(char);
      argparse_format_to_size['C'] = sizeof(char *);
      argparse_format_to_size['s'] = sizeof(char *);
      argparse_format_to_size['S'] = sizeof(char **);
      argparse_format_to_size['a'] = sizeof(gr_meta_args_t *);
      argparse_format_to_size['A'] = sizeof(gr_meta_args_t **);
      argparse_format_to_size['n'] = 0;              /* size for array length is reserved by an array call itself */
      argparse_format_to_size['#'] = sizeof(size_t); /* only used internally */

      argparse_format_has_array_terminator['s'] = 1;
      argparse_format_has_array_terminator['a'] = 1;

      argparse_static_variables_initialized = 1;
    }
}

size_t argparse_calculate_needed_buffer_size(const char *format, int apply_padding)
{
  size_t needed_size;
  size_t size_for_current_specifier;
  int is_array;

  needed_size = 0;
  is_array = 0;
  if (strlen(format) > 1 && argparse_format_has_array_terminator[(unsigned char)*format])
    {
      /* Add size for a NULL pointer terminator in the buffer itself because it will be converted to an array buffer
       * later (-> see `argparse_convert_to_array`) */
      size_for_current_specifier = argparse_format_to_size[(unsigned char)*format];
      needed_size = size_for_current_specifier;
    }
  while (*format)
    {
      char current_format;
      if (*format == '(')
        {
          format = argparse_skip_option(format);
          if (!*format)
            {
              break;
            }
        }
      if (tolower(*format) != *format)
        {
          is_array = 1;
        }
      current_format = *format;
      while (current_format)
        {
          size_for_current_specifier = argparse_format_to_size[(unsigned char)current_format];
          if (apply_padding)
            {
              /* apply needed padding for memory alignment first */
              needed_size += argparse_calculate_needed_padding((void *)needed_size, current_format);
            }
          /* then add the actual needed memory size */
          needed_size += size_for_current_specifier;
          if (is_array)
            {
              current_format = '#';
              is_array = 0;
            }
          else
            {
              current_format = '\0';
            }
        }
      ++format;
    }

  return needed_size;
}

size_t argparse_calculate_needed_padding(void *buffer, char current_format)
{
  int size_for_current_specifier;
  int needed_padding;

  size_for_current_specifier = argparse_format_to_size[(unsigned char)current_format];
  if (size_for_current_specifier > 0)
    {
      needed_padding = size_for_current_specifier - ((char *)buffer - (char *)0) % size_for_current_specifier;
      if (needed_padding == size_for_current_specifier)
        {
          needed_padding = 0;
        }
    }
  else
    {
      needed_padding = 0;
    }

  return needed_padding;
}

void argparse_read_next_option(argparse_state_t *state, char **format)
{
  char *fmt = *format;
  unsigned int next_array_length;
  char *current_char;

  ++fmt;
  if (*fmt != '(')
    {
      return; /* there is no option that could be read */
    }

  current_char = ++fmt;
  while (*current_char && *current_char != ')')
    {
      ++current_char;
    }
  if (!*current_char)
    {
      debug_print_error(("Option \"%s\" in format string \"%s\" is not terminated -> ignore it.\n", fmt, *format));
      return;
    }
  *current_char = '\0';

  if (!str_to_uint(fmt, &next_array_length))
    {
      debug_print_error(
          ("Option \"%s\" in format string \"%s\" could not be converted to a number -> ignore it.\n", fmt, *format));
      return;
    }

  state->next_array_length = next_array_length;
  *format = current_char;
}

const char *argparse_skip_option(const char *format)
{
  if (*format != '(')
    {
      return format;
    }
  while (*format && *format != ')')
    {
      ++format;
    }
  if (*format)
    {
      ++format;
    }
  return format;
}

char *argparse_convert_to_array(argparse_state_t *state)
{
  void *new_save_buffer = NULL;
  size_t *size_t_typed_buffer;
  void ***general_typed_buffer;
  char *new_format = NULL;

  new_save_buffer = malloc(sizeof(size_t) + sizeof(void *));
  if (new_save_buffer == NULL)
    {
      goto cleanup;
    }
  size_t_typed_buffer = new_save_buffer;
  *size_t_typed_buffer = state->dataslot_count;
  general_typed_buffer = (void ***)(size_t_typed_buffer + 1);
  *general_typed_buffer = state->save_buffer;
  if (argparse_format_has_array_terminator[(unsigned char)state->current_format])
    {
      (*general_typed_buffer)[*size_t_typed_buffer] = NULL;
    }
  state->save_buffer = new_save_buffer;
  new_format = malloc(2 * sizeof(char));
  new_format[0] = toupper(state->current_format);
  new_format[1] = '\0';
  if (new_format == NULL)
    {
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

arg_t *args_create_args(const char *key, const char *value_format, const void *buffer, va_list *vl, int apply_padding)
{
  arg_t *arg;
  char *parsing_format;
  char *new_format = NULL;

  if (!args_validate_format_string(value_format))
    {
      return NULL;
    }

  arg = malloc(sizeof(arg_t));
  if (arg == NULL)
    {
      debug_print_malloc_error();
      return NULL;
    }
  if (key != NULL)
    {
      arg->key = gks_strdup(key);
      if (arg->key == NULL)
        {
          debug_print_malloc_error();
          free(arg);
          return NULL;
        }
    }
  else
    {
      arg->key = NULL;
    }
  arg->value_format = malloc(2 * strlen(value_format) + 1);
  if (arg->value_format == NULL)
    {
      debug_print_malloc_error();
      free((char *)arg->key);
      free(arg);
      return NULL;
    }
  parsing_format = malloc(strlen(value_format) + 1);
  if (parsing_format == NULL)
    {
      debug_print_malloc_error();
      free((char *)arg->key);
      free((char *)arg->value_format);
      free(arg);
      return NULL;
    }
  args_copy_format_string_for_parsing(parsing_format, value_format);
  arg->value_ptr = argparse_read_params(parsing_format, buffer, vl, apply_padding, &new_format);
  if (new_format == NULL)
    {
      args_copy_format_string_for_arg((char *)arg->value_format, value_format);
    }
  else
    {
      args_copy_format_string_for_arg((char *)arg->value_format, new_format);
      free(new_format);
    }
  free(parsing_format);
  arg->priv = malloc(sizeof(arg_private_t));
  if (arg->priv == NULL)
    {
      debug_print_malloc_error();
      free((char *)arg->key);
      free((char *)arg->value_format);
      free(arg);
      return NULL;
    }
  arg->priv->reference_count = 1;

  return arg;
}

int args_validate_format_string(const char *format)
{
  char *fmt;
  char *first_format_char;
  char *previous_char;
  char *current_char;
  char *option_start;
  int is_valid;

  if (format == NULL)
    {
      return 0;
    }
  fmt = gks_strdup(format);
  if (fmt == NULL)
    {
      debug_print_malloc_error();
      return 0;
    }

  first_format_char = NULL;
  previous_char = NULL;
  current_char = fmt;
  is_valid = 1;
  while (*current_char && is_valid)
    {
      if (*current_char == '(')
        {
          if (previous_char != NULL)
            {
              if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, tolower(*previous_char)) != NULL)
                {
                  previous_char = current_char;
                  option_start = ++current_char;
                  while (*current_char && *current_char != ')')
                    {
                      ++current_char;
                    }
                  if (*current_char)
                    {
                      *current_char = '\0';
                      is_valid = str_to_uint(option_start, NULL);
                      if (!is_valid)
                        {
                          debug_print_error(("The option \"%s\" in the format string \"%s\" in no valid number.\n",
                                             option_start, format));
                        }
                    }
                  else
                    {
                      is_valid = 0;
                      --current_char;
                      debug_print_error(
                          ("Option \"%s\" in the format string \"%s\" is not terminated.\n", option_start, format));
                    }
                }
              else
                {
                  is_valid = 0;
                  debug_print_error(("Specifier '%c' in the format string \"%s\" cannot have any options.\n",
                                     *previous_char, format));
                }
            }
          else
            {
              is_valid = 0;
              debug_print_error(
                  ("The format string \"%s\" is invalid: Format strings must not start with an option.\n", format));
            }
        }
      else
        {
          if (strchr(ARGS_VALID_FORMAT_SPECIFIERS, *current_char) == NULL)
            {
              is_valid = 0;
              debug_print_error(("Invalid specifier '%c' in the format string \"%s\".\n", *current_char, format));
            }
          else if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, *current_char) != NULL)
            {
              if (first_format_char != NULL && *current_char != *first_format_char)
                {
                  is_valid = 0;
                  debug_print_error(
                      ("The format string \"%s\" consists of different types which is not allowed.\n", format));
                }
              if (first_format_char == NULL)
                {
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

const char *args_skip_option(const char *format)
{
  if (*format != '(')
    {
      return format;
    }
  while (*format && *format != ')')
    {
      ++format;
    }
  if (*format)
    {
      ++format;
    }
  return format;
}

void args_copy_format_string_for_parsing(char *dst, const char *format)
{
  while (*format)
    {
      if (*format == 'C')
        {
          /* char arrays and strings are the same -> store them as strings for unified data handling */
          *dst++ = 's';
          /* skip an optional array length since strings have no array length */
          ++format;
          format = args_skip_option(format);
        }
      else
        {
          *dst++ = *format++;
        }
    }
  *dst = '\0';
}

void args_copy_format_string_for_arg(char *dst, const char *format)
{
  /* `dst` should have twice as much memory as `format` to ensure that no buffer overun can occur */
  while (*format)
    {
      if (*format == 'n')
        {
          /* Skip `n` since it is added for arrays anyway when needed */
          ++format;
          continue;
        }
      if (isupper(*format))
        {
          /* all array formats get an internal size value */
          *dst++ = 'n';
        }
      if (*format == 'C')
        {
          /* char arrays and strings are the same -> store them as strings for unified data handling */
          *dst++ = 's';
          ++format;
        }
      else
        {
          *dst++ = *format++;
        }
      /* Skip an optional array length since it already saved in the argument buffer itself (-> `n` format) */
      format = args_skip_option(format);
    }
  *dst = '\0';
}

int args_check_format_compatibility(const arg_t *arg, const char *compatible_format)
{
  char first_compatible_format_char, first_value_format_char;
  const char *current_format_ptr;
  char *compatible_format_for_arg;
  size_t dataslot_count, len_compatible_format;

  /* First, check if the compatible format itself is valid (-> known format, homogeneous, no options) */
  first_compatible_format_char = *compatible_format;
  if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, tolower(first_compatible_format_char)) == NULL)
    {
      return 0;
    }
  current_format_ptr = compatible_format;
  while (*current_format_ptr != '\0')
    {
      if (*current_format_ptr != first_compatible_format_char)
        {
          return 0;
        }
      ++current_format_ptr;
    }
  len_compatible_format = current_format_ptr - compatible_format;

  /* Second, check if original and compatible format are identical */
  /* within an argument, formats are stored **with** array length slots, so we need `nD` instead of `D` for example
   * -> convert the format before comparison */
  compatible_format_for_arg = malloc(2 * strlen(compatible_format) + 1);
  if (compatible_format_for_arg == NULL)
    {
      debug_print_malloc_error();
      return 0;
    }
  args_copy_format_string_for_arg(compatible_format_for_arg, compatible_format);
  if (strcmp(arg->value_format, compatible_format_for_arg) == 0)
    {
      free(compatible_format_for_arg);
      return 2;
    }
  free(compatible_format_for_arg);

  /* Otherwise, check if the format is compatible */
  /* Compatibility is only possible for single array types -> the original format string (ignoring `n`!) must not be
   * longer than 1 */
  dataslot_count = 0;
  current_format_ptr = arg->value_format;
  while (*current_format_ptr != '\0' && dataslot_count < 2)
    {
      if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, tolower(*current_format_ptr)) != NULL)
        {
          ++dataslot_count;
          if (dataslot_count == 1)
            {
              first_value_format_char = *current_format_ptr;
            }
        }
      ++current_format_ptr;
    }
  if (dataslot_count > 1)
    {
      return 0;
    }
  /* Check if the single format character is an uppercase version of the given compatible format */
  if (first_value_format_char != toupper(first_compatible_format_char))
    {
      return 0;
    }
  /* Check if the compatible format is not longer than the stored array */
  if (len_compatible_format > *(size_t *)arg->value_ptr)
    {
      return 0;
    }

  return 1;
}

void args_decrease_arg_reference_count(args_node_t *args_node)
{
  if (--(args_node->arg->priv->reference_count) == 0)
    {
      args_value_iterator_t *value_it = arg_value_iter(args_node->arg);
      while (value_it->next(value_it) != NULL)
        {
          /* use a char pointer since chars have no memory alignment restrictions */
          if (value_it->is_array)
            {
              if (argparse_format_to_delete_callback[(int)value_it->format] != NULL)
                {
                  char **current_value_ptr = *(char ***)value_it->value_ptr;
                  while (*current_value_ptr != NULL)
                    {
                      argparse_format_to_delete_callback[(int)value_it->format](*current_value_ptr);
                      /* cast to (char *) to allow pointer increment by bytes */
                      current_value_ptr =
                          (char **)((char *)current_value_ptr + argparse_format_to_size[(int)value_it->format]);
                    }
                }
              free(*(char ***)value_it->value_ptr);
            }
          else if (argparse_format_to_delete_callback[(int)value_it->format] != NULL)
            {
              argparse_format_to_delete_callback[(int)value_it->format](*(char **)value_it->value_ptr);
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


/* ------------------------- event handling ------------------------------------------------------------------------- */

int process_events(void)
{
  int processed_events = 0;
  /* Trigger event handling routines after plotting -> args container is fully processed (and modified consistently) at
   * this time */
  if (!processing_events)
    {
      processing_events = 1; /* Ensure that event processing won't trigger event processing again */
      processed_events = event_queue_process_all(event_queue);
      processing_events = 0;
    }

  return processed_events;
}


/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ general ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

error_t plot_init_static_variables(void)
{
  error_t error = NO_ERROR;

  if (!plot_static_variables_initialized)
    {
      logger((stderr, "Initializing static plot variables\n"));
      event_queue = event_queue_new();
      processing_events = 0;
      global_root_args = gr_newmeta();
      error_cleanup_and_set_error_if(global_root_args == NULL, ERROR_MALLOC);
      error = plot_init_args_structure(global_root_args, plot_hierarchy_names, 1);
      error_cleanup_if_error;
      plot_set_flag_defaults();
      error_cleanup_and_set_error_if(!args_values(global_root_args, "plots", "a", &active_plot_args), ERROR_INTERNAL);
      active_plot_index = 1;
      fmt_map = string_map_new_with_data(array_size(kind_to_fmt), kind_to_fmt);
      error_cleanup_and_set_error_if(fmt_map == NULL, ERROR_MALLOC);
      plot_func_map = plot_func_map_new_with_data(array_size(kind_to_func), kind_to_func);
      error_cleanup_and_set_error_if(plot_func_map == NULL, ERROR_MALLOC);
      {
        const char **hierarchy_keys[] = {valid_root_keys, valid_plot_keys, valid_subplot_keys, valid_series_keys, NULL};
        const char **hierarchy_names_ptr, ***hierarchy_keys_ptr, **current_key_ptr;
        plot_valid_keys_map = string_map_new(array_size(valid_root_keys) + array_size(valid_plot_keys) +
                                             array_size(valid_subplot_keys) + array_size(valid_series_keys));
        error_cleanup_and_set_error_if(plot_valid_keys_map == NULL, ERROR_MALLOC);
        hierarchy_keys_ptr = hierarchy_keys;
        hierarchy_names_ptr = plot_hierarchy_names;
        while (*hierarchy_names_ptr != NULL && *hierarchy_keys_ptr != NULL)
          {
            current_key_ptr = *hierarchy_keys_ptr;
            while (*current_key_ptr != NULL)
              {
                string_map_insert(plot_valid_keys_map, *current_key_ptr, *hierarchy_names_ptr);
                ++current_key_ptr;
              }
            ++hierarchy_names_ptr;
            ++hierarchy_keys_ptr;
          }
      }
      plot_static_variables_initialized = 1;
    }
  return NO_ERROR;

error_cleanup:
  if (global_root_args != NULL)
    {
      gr_deletemeta(global_root_args);
      global_root_args = NULL;
    }
  if (fmt_map != NULL)
    {
      string_map_delete(fmt_map);
      fmt_map = NULL;
    }
  if (plot_func_map != NULL)
    {
      plot_func_map_delete(plot_func_map);
      plot_func_map = NULL;
    }
  if (plot_valid_keys_map != NULL)
    {
      string_map_delete(plot_valid_keys_map);
      plot_valid_keys_map = NULL;
    }
  return error;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot arguments ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

error_t plot_merge_args(gr_meta_args_t *args, const gr_meta_args_t *merge_args, const char **hierarchy_name_ptr,
                        uint_map_t *hierarchy_to_id)
{
  static args_set_map_t *key_to_cleared_args = NULL;
  static int recursion_level = -1;
  int plot_id, subplot_id, series_id;
  int append_plots;
  args_iterator_t *merge_it = NULL;
  arg_t *arg, *merge_arg;
  args_value_iterator_t *value_it = NULL, *merge_value_it = NULL;
  const char **current_hierarchy_name_ptr;
  gr_meta_args_t **args_array, **merge_args_array, *current_args;
  unsigned int i;
  error_t error = NO_ERROR;

  ++recursion_level;
  if (hierarchy_name_ptr == NULL)
    {
      hierarchy_name_ptr = plot_hierarchy_names;
    }
  if (hierarchy_to_id == NULL)
    {
      hierarchy_to_id = uint_map_new(array_size(plot_hierarchy_names));
      cleanup_and_set_error_if(hierarchy_to_id == NULL, ERROR_MALLOC);
    }
  else
    {
      hierarchy_to_id = uint_map_copy(hierarchy_to_id);
      cleanup_and_set_error_if(hierarchy_to_id == NULL, ERROR_MALLOC);
    }
  if (key_to_cleared_args == NULL)
    {
      key_to_cleared_args = args_set_map_new(array_size(plot_merge_clear_keys));
      cleanup_and_set_error_if(hierarchy_to_id == NULL, ERROR_MALLOC);
    }
  args_values(global_root_args, "append_plots", "i", &append_plots);
  get_id_from_args(merge_args, &plot_id, &subplot_id, &series_id);
  if (plot_id > 0)
    {
      uint_map_insert(hierarchy_to_id, "plots", plot_id);
    }
  else
    {
      uint_map_insert_default(hierarchy_to_id, "plots", append_plots ? 0 : active_plot_index);
      uint_map_at(hierarchy_to_id, "plots", (unsigned int *)&plot_id);
      logger((stderr, "Using plot_id \"%u\"\n", plot_id));
    }
  if (subplot_id > 0)
    {
      uint_map_insert(hierarchy_to_id, "subplots", subplot_id);
    }
  else
    {
      uint_map_insert_default(hierarchy_to_id, "subplots", 1);
      uint_map_at(hierarchy_to_id, "subplots", (unsigned int *)&subplot_id);
    }
  if (series_id > 0)
    {
      uint_map_insert(hierarchy_to_id, "series", series_id);
    }
  else
    {
      uint_map_insert_default(hierarchy_to_id, "series", 1);
      uint_map_at(hierarchy_to_id, "series", (unsigned int *)&series_id);
    }
  /* special case: clear the plot container before usage if
   * - it is the first call of `plot_merge_args` AND
   *   - `plot_id` is `1` and `hold_plots` is not set OR
   *   - `hold_plots` is true and no plot will be appended (`plot_id` > 0)
   */
  if (strcmp(*hierarchy_name_ptr, "root") == 0 && plot_id > 0)
    {
      int hold_plots_key_available, hold_plots;
      hold_plots_key_available = args_values(args, "hold_plots", "i", &hold_plots);
      if (hold_plots_key_available)
        {
          logger((stderr, "Do%s hold plots\n", hold_plots ? "" : " not"));
        }
      if ((hold_plots_key_available && !hold_plots) || (!hold_plots_key_available && plot_id == 1))
        {
          cleanup_and_set_error_if(!args_values(args, "plots", "A", &args_array), ERROR_INTERNAL);
          current_args = args_array[plot_id - 1];
          gr_meta_args_clear(current_args);
          error = plot_init_args_structure(current_args, hierarchy_name_ptr + 1, 1);
          cleanup_if_error;
          logger((stderr, "Cleared current args\n"));
        }
      else
        {
          logger((stderr, "Held current args\n"));
        }
    }
  merge_it = args_iter(merge_args);
  cleanup_and_set_error_if(merge_it == NULL, ERROR_MALLOC);
  while ((merge_arg = merge_it->next(merge_it)) != NULL)
    {
      if (str_equals_any_in_array(merge_arg->key, plot_merge_ignore_keys))
        {
          continue;
        }
      /* First, find the correct hierarchy level where the current merge value belongs to. */
      error = plot_get_args_in_hierarchy(args, hierarchy_name_ptr, merge_arg->key, hierarchy_to_id,
                                         (const gr_meta_args_t **)&current_args, &current_hierarchy_name_ptr);
      if (error == ERROR_PLOT_UNKNOWN_KEY)
        {
          logger((stderr, "WARNING: The key \"%s\" is not assigned to any hierarchy level.\n", merge_arg->key));
        }
      cleanup_if_error;
      if (str_equals_any_in_array(*current_hierarchy_name_ptr, plot_merge_clear_keys))
        {
          int clear_args = 1;
          args_set_t *cleared_args = NULL;
          if (args_set_map_at(key_to_cleared_args, *current_hierarchy_name_ptr, &cleared_args))
            {
              clear_args = !args_set_contains(cleared_args, current_args);
            }
          if (clear_args)
            {
              logger((stderr, "Perform a clear on the current args container\n"));
              gr_meta_args_clear(current_args);
              if (cleared_args == NULL)
                {
                  cleared_args = args_set_new(10); /* FIXME: do not use a magic number, use a growbable set instead! */
                  cleanup_and_set_error_if(cleared_args == NULL, ERROR_MALLOC);
                  cleanup_and_set_error_if(
                      !args_set_map_insert(key_to_cleared_args, *current_hierarchy_name_ptr, cleared_args),
                      ERROR_INTERNAL);
                }
              logger((stderr, "Add args container \"%p\" to cleared args with key \"%s\"\n", (void *)current_args,
                      *current_hierarchy_name_ptr));
              cleanup_and_set_error_if(!args_set_add(cleared_args, current_args), ERROR_INTERNAL);
            }
        }
      /* If the current key is a hierarchy key, perform a merge. Otherwise (else branch) put the value in without a
       * merge.
       */
      if (current_hierarchy_name_ptr[1] != NULL && strcmp(merge_arg->key, current_hierarchy_name_ptr[1]) == 0)
        {
          /* `args_at` cannot fail in this case because the `args` object was initialized with an empty structure
           * before. If `arg` is NULL, an internal error occurred. */
          arg = args_at(current_args, merge_arg->key);
          cleanup_and_set_error_if(arg == NULL, ERROR_INTERNAL);
          value_it = arg_value_iter(arg);
          merge_value_it = arg_value_iter(merge_arg);
          cleanup_and_set_error_if(value_it == NULL, ERROR_MALLOC);
          cleanup_and_set_error_if(merge_value_it == NULL, ERROR_MALLOC);
          /* Do not support two dimensional argument arrays like `nAnA`) -> a loop would be needed with more memory
           * management */
          cleanup_and_set_error_if(value_it->next(value_it) == NULL, ERROR_MALLOC);
          cleanup_and_set_error_if(merge_value_it->next(merge_value_it) == NULL, ERROR_MALLOC);
          /* Increase the array size of the internal args array if necessary */
          if (merge_value_it->array_length > value_it->array_length)
            {
              error = plot_init_arg_structure(arg, current_hierarchy_name_ptr, merge_value_it->array_length);
              cleanup_if_error;
              args_value_iterator_delete(value_it);
              value_it = arg_value_iter(arg);
              cleanup_and_set_error_if(value_it == NULL, ERROR_MALLOC);
              cleanup_and_set_error_if(value_it->next(value_it) == NULL, ERROR_MALLOC);
              args_array = *(gr_meta_args_t ***)value_it->value_ptr;
            }
          else
            {
              /* The internal args container stores always an array for hierarchy levels */
              args_array = *(gr_meta_args_t ***)value_it->value_ptr;
            }
          if (merge_value_it->is_array)
            {
              merge_args_array = *(gr_meta_args_t ***)merge_value_it->value_ptr;
            }
          else
            {
              merge_args_array = (gr_meta_args_t **)merge_value_it->value_ptr;
            }
          /* Merge array entries pairwise */
          for (i = 0; i < merge_value_it->array_length; ++i)
            {
              logger((stderr, "Perform a recursive merge on key \"%s\", array index \"%d\"\n", merge_arg->key, i));
              error =
                  plot_merge_args(args_array[i], merge_args_array[i], current_hierarchy_name_ptr + 1, hierarchy_to_id);
              cleanup_if_error;
            }
        }
      else
        {
          logger((stderr, "Perform a replace on key \"%s\"\n", merge_arg->key));
          error = args_push_arg(current_args, merge_arg);
          cleanup_if_error;
        }
    }

cleanup:
  if (recursion_level == 0)
    {
      if (key_to_cleared_args != NULL)
        {
          args_set_t *cleared_args = NULL;
          const char **current_key_ptr = plot_merge_clear_keys;
          while (*current_key_ptr != NULL)
            {
              if (args_set_map_at(key_to_cleared_args, *current_key_ptr, &cleared_args))
                {
                  args_set_delete(cleared_args);
                }
              ++current_key_ptr;
            }
          args_set_map_delete(key_to_cleared_args);
          key_to_cleared_args = NULL;
        }
    }
  if (hierarchy_to_id != NULL)
    {
      uint_map_delete(hierarchy_to_id);
      hierarchy_to_id = NULL;
    }
  if (merge_it != NULL)
    {
      args_iterator_delete(merge_it);
    }
  if (value_it != NULL)
    {
      args_value_iterator_delete(value_it);
    }
  if (merge_value_it != NULL)
    {
      args_value_iterator_delete(merge_value_it);
    }

  --recursion_level;

  return error;
}

error_t plot_init_arg_structure(arg_t *arg, const char **hierarchy_name_ptr, unsigned int next_hierarchy_level_max_id)
{
  gr_meta_args_t **args_array = NULL;
  unsigned int args_old_array_length;
  unsigned int i;
  error_t error = NO_ERROR;

  logger((stderr, "Init plot args structure for hierarchy: \"%s\"\n", *hierarchy_name_ptr));

  ++hierarchy_name_ptr;
  if (*hierarchy_name_ptr == NULL)
    {
      return NO_ERROR;
    }
  arg_first_value(arg, "A", NULL, &args_old_array_length);
  if (next_hierarchy_level_max_id <= args_old_array_length)
    {
      return NO_ERROR;
    }
  logger((stderr, "Increase array for key \"%s\" from %d to %d\n", *hierarchy_name_ptr, args_old_array_length,
          next_hierarchy_level_max_id));
  error = arg_increase_array(arg, next_hierarchy_level_max_id - args_old_array_length);
  return_if_error;
  arg_values(arg, "A", &args_array);
  for (i = args_old_array_length; i < next_hierarchy_level_max_id; ++i)
    {
      args_array[i] = gr_newmeta();
      gr_meta_args_push(args_array[i], "array_index", "i", i);
      return_error_if(args_array[i] == NULL, ERROR_MALLOC);
      error = plot_init_args_structure(args_array[i], hierarchy_name_ptr, 1);
      return_if_error;
      if (strcmp(*hierarchy_name_ptr, "plots") == 0)
        {
          gr_meta_args_push(args_array[i], "in_use", "i", 0);
        }
    }

  return NO_ERROR;
}

error_t plot_init_args_structure(gr_meta_args_t *args, const char **hierarchy_name_ptr,
                                 unsigned int next_hierarchy_level_max_id)
{
  arg_t *arg = NULL;
  gr_meta_args_t **args_array = NULL;
  unsigned int i;
  error_t error = NO_ERROR;

  logger((stderr, "Init plot args structure for hierarchy: \"%s\"\n", *hierarchy_name_ptr));

  ++hierarchy_name_ptr;
  if (*hierarchy_name_ptr == NULL)
    {
      return NO_ERROR;
    }
  arg = args_at(args, *hierarchy_name_ptr);
  if (arg == NULL)
    {
      args_array = calloc(next_hierarchy_level_max_id, sizeof(gr_meta_args_t *));
      error_cleanup_and_set_error_if(args_array == NULL, ERROR_MALLOC);
      for (i = 0; i < next_hierarchy_level_max_id; ++i)
        {
          args_array[i] = gr_newmeta();
          gr_meta_args_push(args_array[i], "array_index", "i", i);
          error_cleanup_and_set_error_if(args_array[i] == NULL, ERROR_MALLOC);
          error = plot_init_args_structure(args_array[i], hierarchy_name_ptr, 1);
          error_cleanup_if_error;
          if (strcmp(*hierarchy_name_ptr, "plots") == 0)
            {
              gr_meta_args_push(args_array[i], "in_use", "i", 0);
            }
        }
      error_cleanup_if(!gr_meta_args_push(args, *hierarchy_name_ptr, "nA", next_hierarchy_level_max_id, args_array));
      free(args_array);
      args_array = NULL;
    }
  else
    {
      error = plot_init_arg_structure(arg, hierarchy_name_ptr - 1, next_hierarchy_level_max_id);
      error_cleanup_if_error;
    }

  return NO_ERROR;

error_cleanup:
  if (args_array != NULL)
    {
      for (i = 0; i < next_hierarchy_level_max_id; ++i)
        {
          if (args_array[i] != NULL)
            {
              gr_deletemeta(args_array[i]);
            }
        }
      free(args_array);
    }

  return error;
}

void plot_set_flag_defaults(void)
{
  /* Use a standalone function for initializing flags instead of `plot_set_attribute_defaults` to guarantee the flags
   * are already set before `gr_plotmeta` is called (important for `gr_mergemeta`) */

  logger((stderr, "Set global flag defaults\n"));

  args_setdefault(global_root_args, "append_plots", "i", ROOT_DEFAULT_APPEND_PLOTS);
}

void plot_set_attribute_defaults(gr_meta_args_t *plot_args)
{
  const char *kind;
  gr_meta_args_t **current_subplot, **current_series;
  double garbage0, garbage1;

  logger((stderr, "Set plot attribute defaults\n"));

  args_setdefault(plot_args, "clear", "i", PLOT_DEFAULT_CLEAR);
  args_setdefault(plot_args, "update", "i", PLOT_DEFAULT_UPDATE);
  if (!gr_meta_args_contains(plot_args, "figsize"))
    {
      args_setdefault(plot_args, "size", "dd", PLOT_DEFAULT_WIDTH, PLOT_DEFAULT_HEIGHT);
    }

  args_values(plot_args, "subplots", "A", &current_subplot);
  while (*current_subplot != NULL)
    {
      args_setdefault(*current_subplot, "kind", "s", PLOT_DEFAULT_KIND);
      args_values(*current_subplot, "kind", "s", &kind);
      if (gr_meta_args_contains(*current_subplot, "labels"))
        {
          args_setdefault(*current_subplot, "location", "i", PLOT_DEFAULT_LOCATION);
        }
      args_setdefault(*current_subplot, "subplot", "dddd", PLOT_DEFAULT_SUBPLOT_MIN_X, PLOT_DEFAULT_SUBPLOT_MAX_X,
                      PLOT_DEFAULT_SUBPLOT_MIN_Y, PLOT_DEFAULT_SUBPLOT_MAX_Y);
      args_setdefault(*current_subplot, "xlog", "i", PLOT_DEFAULT_XLOG);
      args_setdefault(*current_subplot, "ylog", "i", PLOT_DEFAULT_YLOG);
      args_setdefault(*current_subplot, "zlog", "i", PLOT_DEFAULT_ZLOG);
      args_setdefault(*current_subplot, "xflip", "i", PLOT_DEFAULT_XFLIP);
      args_setdefault(*current_subplot, "yflip", "i", PLOT_DEFAULT_YFLIP);
      args_setdefault(*current_subplot, "zflip", "i", PLOT_DEFAULT_ZFLIP);
      if (str_equals_any(kind, 1, "heatmap"))
        {
          args_setdefault(*current_subplot, "adjust_xlim", "i", 0);
          args_setdefault(*current_subplot, "adjust_ylim", "i", 0);
        }
      else
        {
          args_setdefault(
              *current_subplot, "adjust_xlim", "i",
              (args_values(*current_subplot, "xlim", "dd", &garbage0, &garbage1) ? 0 : PLOT_DEFAULT_ADJUST_XLIM));
          args_setdefault(
              *current_subplot, "adjust_ylim", "i",
              (args_values(*current_subplot, "ylim", "dd", &garbage0, &garbage1) ? 0 : PLOT_DEFAULT_ADJUST_YLIM));
          args_setdefault(
              *current_subplot, "adjust_zlim", "i",
              (args_values(*current_subplot, "zlim", "dd", &garbage0, &garbage1) ? 0 : PLOT_DEFAULT_ADJUST_ZLIM));
        }
      args_setdefault(*current_subplot, "colormap", "i", PLOT_DEFAULT_COLORMAP);
      args_setdefault(*current_subplot, "rotation", "i", PLOT_DEFAULT_ROTATION);
      args_setdefault(*current_subplot, "tilt", "i", PLOT_DEFAULT_TILT);
      args_setdefault(*current_subplot, "keep_aspect_ratio", "i", PLOT_DEFAULT_KEEP_ASPECT_RATIO);

      if (str_equals_any(kind, 2, "contour", "contourf"))
        {
          args_setdefault(*current_subplot, "levels", "i", PLOT_DEFAULT_CONTOUR_LEVELS);
        }
      else if (strcmp(kind, "hexbin") == 0)
        {
          args_setdefault(*current_subplot, "nbins", "i", PLOT_DEFAULT_HEXBIN_NBINS);
        }
      else if (strcmp(kind, "tricont") == 0)
        {
          args_setdefault(*current_subplot, "levels", "i", PLOT_DEFAULT_TRICONT_LEVELS);
        }

      args_values(*current_subplot, "series", "A", &current_series);
      while (*current_series != NULL)
        {
          args_setdefault(*current_series, "spec", "s", SERIES_DEFAULT_SPEC);
          if (strcmp(kind, "step") == 0)
            {
              args_setdefault(*current_series, "step_where", "s", PLOT_DEFAULT_STEP_WHERE);
            }
          ++current_series;
        }
      ++current_subplot;
    }
}

void plot_pre_plot(gr_meta_args_t *plot_args)
{
  int clear;

  logger((stderr, "Pre plot processing\n"));

  args_values(plot_args, "clear", "i", &clear);
  logger((stderr, "Got keyword \"clear\" with value %d\n", clear));
  if (clear)
    {
      gr_clearws();
    }
  plot_process_wswindow_wsviewport(plot_args);
}

void plot_process_wswindow_wsviewport(gr_meta_args_t *plot_args)
{
  int pixel_width, pixel_height;
  int previous_pixel_width, previous_pixel_height;
  double metric_width, metric_height;
  double aspect_ratio_ws;
  double wsviewport[4] = {0.0, 0.0, 0.0, 0.0};
  double wswindow[4] = {0.0, 0.0, 0.0, 0.0};

  get_figure_size(plot_args, &pixel_width, &pixel_height, &metric_width, &metric_height);

  if (!args_values(plot_args, "previous_pixel_size", "ii", &previous_pixel_width, &previous_pixel_height) ||
      (previous_pixel_width != pixel_width || previous_pixel_height != pixel_height))
    {
      /* TODO: handle error return value? */
      event_queue_enqueue_size_event(event_queue, active_plot_index - 1, pixel_width, pixel_height);
    }

  aspect_ratio_ws = metric_width / metric_height;
  if (aspect_ratio_ws > 1)
    {
      wsviewport[1] = metric_width;
      wsviewport[3] = metric_width / aspect_ratio_ws;
      wswindow[1] = 1.0;
      wswindow[3] = 1.0 / aspect_ratio_ws;
    }
  else
    {
      wsviewport[1] = metric_height * aspect_ratio_ws;
      wsviewport[3] = metric_height;
      wswindow[1] = aspect_ratio_ws;
      wswindow[3] = 1.0;
    }

  gr_setwsviewport(wsviewport[0], wsviewport[1], wsviewport[2], wsviewport[3]);
  gr_setwswindow(wswindow[0], wswindow[1], wswindow[2], wswindow[3]);

  gr_meta_args_push(plot_args, "wswindow", "dddd", wswindow[0], wswindow[1], wswindow[2], wswindow[3]);
  gr_meta_args_push(plot_args, "wsviewport", "dddd", wsviewport[0], wsviewport[1], wsviewport[2], wsviewport[3]);
  gr_meta_args_push(plot_args, "previous_pixel_size", "ii", pixel_width, pixel_height);

  logger((stderr, "Stored wswindow (%lf, %lf, %lf, %lf)\n", wswindow[0], wswindow[1], wswindow[2], wswindow[3]));
  logger(
      (stderr, "Stored wsviewport (%lf, %lf, %lf, %lf)\n", wsviewport[0], wsviewport[1], wsviewport[2], wsviewport[3]));
}

void plot_pre_subplot(gr_meta_args_t *subplot_args)
{
  const char *kind;
  double alpha;

  logger((stderr, "Pre subplot processing\n"));

  args_values(subplot_args, "kind", "s", &kind);
  logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));
  if (str_equals_any(kind, 2, "imshow", "isosurface"))
    {
      plot_process_viewport(subplot_args);
    }
  else
    {
      plot_process_viewport(subplot_args);
      plot_store_coordinate_ranges(subplot_args);
      plot_process_window(subplot_args);
      if (str_equals_any(kind, 1, "polar"))
        {
          plot_draw_polar_axes(subplot_args);
        }
      else
        {
          plot_draw_axes(subplot_args, 1);
        }
    }

  plot_process_colormap(subplot_args);
  gr_uselinespec(" ");

  gr_savestate();
  if (args_values(subplot_args, "alpha", "d", &alpha))
    {
      gr_settransparency(alpha);
    }
}

void plot_process_colormap(gr_meta_args_t *subplot_args)
{
  int colormap;

  if (args_values(subplot_args, "colormap", "i", &colormap))
    {
      gr_setcolormap(colormap);
    }
  /* TODO: Implement other datatypes for `colormap` */
}

void plot_process_viewport(gr_meta_args_t *subplot_args)
{
  const char *kind;
  const double *subplot;
  int keep_aspect_ratio;
  double metric_width, metric_height;
  double aspect_ratio_ws;
  double vp[4];
  double viewport[4] = {0.0, 0.0, 0.0, 0.0};
  int background_color_index;

  args_values(subplot_args, "kind", "s", &kind);
  args_values(subplot_args, "subplot", "D", &subplot);
  args_values(subplot_args, "keep_aspect_ratio", "i", &keep_aspect_ratio);
  logger((stderr, "Using subplot: %lf, %lf, %lf, %lf\n", subplot[0], subplot[1], subplot[2], subplot[3]));

  get_figure_size(NULL, NULL, NULL, &metric_width, &metric_height);

  aspect_ratio_ws = metric_width / metric_height;
  memcpy(vp, subplot, sizeof(vp));
  if (aspect_ratio_ws > 1)
    {
      vp[2] /= aspect_ratio_ws;
      vp[3] /= aspect_ratio_ws;
      if (keep_aspect_ratio)
        {
          double border = 0.5 * (vp[1] - vp[0]) * (1.0 - 1.0 / aspect_ratio_ws);
          vp[0] += border;
          vp[1] -= border;
        }
    }
  else
    {
      vp[0] *= aspect_ratio_ws;
      vp[1] *= aspect_ratio_ws;
      if (keep_aspect_ratio)
        {
          double border = 0.5 * (vp[3] - vp[2]) * (1.0 - aspect_ratio_ws);
          vp[2] += border;
          vp[3] -= border;
        }
    }

  if (str_equals_any(kind, 5, "wireframe", "surface", "plot3", "scatter3", "trisurf"))
    {
      double tmp_vp[4];
      double extent;

      if (str_equals_any(kind, 2, "surface", "trisurf"))
        {
          extent = min(vp[1] - vp[0] - 0.1, vp[3] - vp[2]);
        }
      else
        {
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

  if (aspect_ratio_ws > 1)
    {
      viewport[2] += (1 - (subplot[3] - subplot[2]) * (subplot[3] - subplot[2])) * 0.02;
    }
  if (str_equals_any(kind, 7, "tricont", "contour", "contourf", "heatmap", "nonuniformheatmap", "hexbin", "quiver"))
    {
      viewport[1] -= 0.1;
    }

  if (args_values(subplot_args, "backgroundcolor", "i", &background_color_index))
    {
      gr_savestate();
      gr_selntran(0);
      gr_setfillintstyle(GKS_K_INTSTYLE_SOLID);
      gr_setfillcolorind(background_color_index);
      if (aspect_ratio_ws > 1)
        {
          gr_fillrect(subplot[0], subplot[1], subplot[2] / aspect_ratio_ws, subplot[3] / aspect_ratio_ws);
        }
      else
        {
          gr_fillrect(subplot[0] * aspect_ratio_ws, subplot[1] * aspect_ratio_ws, subplot[2], subplot[3]);
        }
      gr_selntran(1);
      gr_restorestate();
    }

  if (strcmp(kind, "polar") == 0)
    {
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

  gr_meta_args_push(subplot_args, "vp", "dddd", vp[0], vp[1], vp[2], vp[3]);
  gr_meta_args_push(subplot_args, "viewport", "dddd", viewport[0], viewport[1], viewport[2], viewport[3]);

  logger((stderr, "Stored vp (%lf, %lf, %lf, %lf)\n", vp[0], vp[1], vp[2], vp[3]));
  logger((stderr, "Stored viewport (%lf, %lf, %lf, %lf)\n", viewport[0], viewport[1], viewport[2], viewport[3]));
}

void plot_process_window(gr_meta_args_t *subplot_args)
{
  int scale = 0;
  const char *kind;
  int xlog, ylog, zlog;
  int xflip, yflip, zflip;
  int major_count, x_major_count, y_major_count;
  const double *stored_window;
  double x_min, x_max, y_min, y_max, z_min, z_max;
  double x, y, xzoom, yzoom;
  int adjust_xlim, adjust_ylim, adjust_zlim;
  double x_tick, y_tick;
  double x_org_low, x_org_high, y_org_low, y_org_high;
  int reset_ranges = 0;

  args_values(subplot_args, "kind", "s", &kind);
  args_values(subplot_args, "xlog", "i", &xlog);
  args_values(subplot_args, "ylog", "i", &ylog);
  args_values(subplot_args, "zlog", "i", &zlog);
  args_values(subplot_args, "xflip", "i", &xflip);
  args_values(subplot_args, "yflip", "i", &yflip);
  args_values(subplot_args, "zflip", "i", &zflip);

  if (strcmp(kind, "polar") != 0)
    {
      scale |= xlog ? GR_OPTION_X_LOG : 0;
      scale |= ylog ? GR_OPTION_Y_LOG : 0;
      scale |= zlog ? GR_OPTION_Z_LOG : 0;
      scale |= xflip ? GR_OPTION_FLIP_X : 0;
      scale |= yflip ? GR_OPTION_FLIP_Y : 0;
      scale |= zflip ? GR_OPTION_FLIP_Z : 0;
    }

  args_values(subplot_args, "xrange", "dd", &x_min, &x_max);
  args_values(subplot_args, "yrange", "dd", &y_min, &y_max);
  if (args_values(subplot_args, "reset_ranges", "i", &reset_ranges) && reset_ranges)
    {
      if (args_values(subplot_args, "original_xrange", "dd", &x_min, &x_max) &&
          args_values(subplot_args, "original_yrange", "dd", &y_min, &y_max) &&
          args_values(subplot_args, "original_adjust_xlim", "i", &adjust_xlim) &&
          args_values(subplot_args, "original_adjust_ylim", "i", &adjust_ylim))
        {
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
  if (gr_meta_args_contains(subplot_args, "panzoom"))
    {
      if (!gr_meta_args_contains(subplot_args, "original_xrange"))
        {
          gr_meta_args_push(subplot_args, "original_xrange", "dd", x_min, x_max);
          args_values(subplot_args, "adjust_xlim", "i", &adjust_xlim);
          gr_meta_args_push(subplot_args, "original_adjust_xlim", "i", adjust_xlim);
          gr_meta_args_push(subplot_args, "adjust_xlim", "i", 0);
        }
      if (!gr_meta_args_contains(subplot_args, "original_yrange"))
        {
          gr_meta_args_push(subplot_args, "original_yrange", "dd", y_min, y_max);
          args_values(subplot_args, "adjust_ylim", "i", &adjust_ylim);
          gr_meta_args_push(subplot_args, "original_adjust_ylim", "i", adjust_ylim);
          gr_meta_args_push(subplot_args, "adjust_ylim", "i", 0);
        }
      if (!args_values(subplot_args, "panzoom", "dddd", &x, &y, &xzoom, &yzoom))
        {
          if (args_values(subplot_args, "panzoom", "ddd", &x, &y, &xzoom))
            {
              yzoom = xzoom;
            }
          else
            {
              /* TODO: Add error handling for type mismatch (-> next statement would fail) */
              args_values(subplot_args, "panzoom", "dd", &x, &y);
              yzoom = xzoom = 0.0;
            }
        }
      /* Ensure the correct window is set in GR */
      if (args_values(subplot_args, "window", "D", &stored_window))
        {
          gr_setwindow(stored_window[0], stored_window[1], stored_window[2], stored_window[3]);
          logger((stderr, "Window before `gr_panzoom` (%lf, %lf, %lf, %lf)\n", stored_window[0], stored_window[1],
                  stored_window[2], stored_window[3]));
        }
      gr_panzoom(x, y, xzoom, yzoom, &x_min, &x_max, &y_min, &y_max);
      logger((stderr, "Window after `gr_panzoom` (%lf, %lf, %lf, %lf)\n", x_min, x_max, y_min, y_max));
      gr_meta_args_push(subplot_args, "xrange", "dd", x_min, x_max);
      gr_meta_args_push(subplot_args, "yrange", "dd", y_min, y_max);
      gr_meta_args_remove(subplot_args, "panzoom");
    }

  if (str_equals_any(kind, 6, "wireframe", "surface", "plot3", "scatter3", "polar", "trisurf"))
    {
      major_count = 2;
    }
  else
    {
      major_count = 5;
    }

  if (!(scale & GR_OPTION_X_LOG))
    {
      args_values(subplot_args, "adjust_xlim", "i", &adjust_xlim);
      if (adjust_xlim)
        {
          logger((stderr, "xrange before \"gr_adjustlimits\": (%lf, %lf)\n", x_min, x_max));
          gr_adjustlimits(&x_min, &x_max);
          logger((stderr, "xrange after \"gr_adjustlimits\": (%lf, %lf)\n", x_min, x_max));
        }
      x_major_count = major_count;
      x_tick = gr_tick(x_min, x_max) / x_major_count;
    }
  else
    {
      x_tick = x_major_count = 1;
    }
  if (!(scale & GR_OPTION_FLIP_X))
    {
      x_org_low = x_min;
      x_org_high = x_max;
    }
  else
    {
      x_org_low = x_max;
      x_org_high = x_min;
    }
  gr_meta_args_push(subplot_args, "xtick", "d", x_tick);
  gr_meta_args_push(subplot_args, "xorg", "dd", x_org_low, x_org_high);
  gr_meta_args_push(subplot_args, "xmajor", "i", x_major_count);

  if (!(scale & GR_OPTION_Y_LOG))
    {
      args_values(subplot_args, "adjust_ylim", "i", &adjust_ylim);
      if (adjust_ylim)
        {
          logger((stderr, "yrange before \"gr_adjustlimits\": (%lf, %lf)\n", y_min, y_max));
          gr_adjustlimits(&y_min, &y_max);
          logger((stderr, "yrange after \"gr_adjustlimits\": (%lf, %lf)\n", y_min, y_max));
        }
      y_major_count = major_count;
      y_tick = gr_tick(y_min, y_max) / y_major_count;
    }
  else
    {
      y_tick = y_major_count = 1;
    }
  if (!(scale & GR_OPTION_FLIP_Y))
    {
      y_org_low = y_min;
      y_org_high = y_max;
    }
  else
    {
      y_org_low = y_max;
      y_org_high = y_min;
    }
  gr_meta_args_push(subplot_args, "ytick", "d", y_tick);
  gr_meta_args_push(subplot_args, "yorg", "dd", y_org_low, y_org_high);
  gr_meta_args_push(subplot_args, "ymajor", "i", y_major_count);

  logger((stderr, "Storing window (%lf, %lf, %lf, %lf)\n", x_min, x_max, y_min, y_max));
  gr_meta_args_push(subplot_args, "window", "dddd", x_min, x_max, y_min, y_max);
  if (strcmp(kind, "polar") != 0)
    {
      gr_setwindow(x_min, x_max, y_min, y_max);
    }
  else
    {
      gr_setwindow(-1.0, 1.0, -1.0, 1.0);
    }

  if (str_equals_any(kind, 5, "wireframe", "surface", "plot3", "scatter3", "trisurf"))
    {
      double z_major_count;
      double z_tick;
      double z_org_low, z_org_high;
      int rotation, tilt;

      args_values(subplot_args, "zrange", "dd", &z_min, &z_max);
      if (!(scale & GR_OPTION_Z_LOG))
        {
          args_values(subplot_args, "adjust_zlim", "i", &adjust_zlim);
          if (adjust_zlim)
            {
              logger((stderr, "zrange before \"gr_adjustlimits\": (%lf, %lf)\n", z_min, z_max));
              gr_adjustlimits(&z_min, &z_max);
              logger((stderr, "zrange after \"gr_adjustlimits\": (%lf, %lf)\n", z_min, z_max));
            }
          z_major_count = major_count;
          z_tick = gr_tick(z_min, z_max) / z_major_count;
        }
      else
        {
          z_tick = z_major_count = 1;
        }
      if (!(scale & GR_OPTION_FLIP_Z))
        {
          z_org_low = z_min;
          z_org_high = z_max;
        }
      else
        {
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

void plot_store_coordinate_ranges(gr_meta_args_t *subplot_args)
{
  const char *kind;
  const char *fmt;
  gr_meta_args_t **current_series;
  unsigned int series_count;
  const char *data_component_names[] = {"x", "y", "z", "c", NULL};
  const char **current_component_name;
  double *current_component = NULL;
  unsigned int point_count = 0;
  const char *range_keys[][2] = {{"xlim", "xrange"}, {"ylim", "yrange"}, {"zlim", "zrange"}, {"clim", "crange"}};
  const char *(*current_range_keys)[2];
  unsigned int i;

  logger((stderr, "Storing coordinate ranges\n"));
  /* TODO: support that single `lim` values are `null` / unset! */

  args_values(subplot_args, "kind", "s", &kind);
  string_map_at(fmt_map, kind, (char **)&fmt); /* TODO: check if the map access was successful */
  current_range_keys = range_keys;
  current_component_name = data_component_names;
  while (*current_component_name != NULL)
    {
      double min_component = DBL_MAX;
      double max_component = -DBL_MAX;
      double step = -DBL_MAX;
      if (strchr(fmt, **current_component_name) == NULL ||
          gr_meta_args_contains(subplot_args, (*current_range_keys)[1]))
        {
          ++current_range_keys;
          ++current_component_name;
          continue;
        }
      if (!gr_meta_args_contains(subplot_args, (*current_range_keys)[0]))
        {
          args_first_value(subplot_args, "series", "A", &current_series, &series_count);
          while (*current_series != NULL)
            {
              if (args_first_value(*current_series, *current_component_name, "D", &current_component, &point_count))
                {
                  for (i = 0; i < point_count; i++)
                    {
                      min_component = min(current_component[i], min_component);
                      max_component = max(current_component[i], max_component);
                    }
                }
              ++current_series;
            }
          if (strcmp(kind, "quiver") == 0)
            {
              step = max(find_max_step(point_count, current_component), step);
              if (step > 0.0)
                {
                  min_component -= step;
                  max_component += step;
                }
            }
          else if (strcmp(kind, "heatmap") == 0 && str_equals_any(*current_component_name, 2, "x", "y"))
            {
              min_component -= 0.5;
              max_component += 0.5;
            }
          else if ((strcmp(kind, "hist") == 0 || strcmp(kind, "barplot") == 0) &&
                   strcmp("y", *current_component_name) == 0)
            {
              min_component = 0;
            }
        }
      else
        {
          args_values(subplot_args, (*current_range_keys)[0], "dd", &min_component, &max_component);
        }
      /* TODO: This may be obsolete when all supported format-strings are added
       color is an optional part of the format strings */
      if (!(min_component == DBL_MAX && max_component == -DBL_MAX && strcmp(*current_component_name, "c") == 0))
        {
          gr_meta_args_push(subplot_args, (*current_range_keys)[1], "dd", min_component, max_component);
        }
      ++current_range_keys;
      ++current_component_name;
    }
  /* For quiver plots use u^2 + v^2 as z value */
  if (strcmp(kind, "quiver") == 0)
    {
      double min_component = DBL_MAX;
      double max_component = -DBL_MAX;
      if (!gr_meta_args_contains(subplot_args, "zlim"))
        {
          double *u, *v;
          /* TODO: Support more than one series? */
          /* TODO: `ERROR_PLOT_COMPONENT_LENGTH_MISMATCH` */
          args_values(subplot_args, "series", "A", &current_series);
          args_first_value(*current_series, "u", "D", &u, &point_count);
          args_first_value(*current_series, "v", "D", &v, NULL);
          for (i = 0; i < point_count; i++)
            {
              double z = u[i] * u[i] + v[i] * v[i];
              min_component = min(z, min_component);
              max_component = max(z, max_component);
            }
          min_component = sqrt(min_component);
          max_component = sqrt(max_component);
        }
      else
        {
          args_values(subplot_args, "zlim", "dd", &min_component, &max_component);
        }
      gr_meta_args_push(subplot_args, "zrange", "dd", min_component, max_component);
    }
}

void plot_post_plot(gr_meta_args_t *plot_args)
{
  int update;

  logger((stderr, "Post plot processing\n"));

  args_values(plot_args, "update", "i", &update);
  logger((stderr, "Got keyword \"update\" with value %d\n", update));
  if (update)
    {
      gr_updatews();
    }
}

void plot_post_subplot(gr_meta_args_t *subplot_args)
{
  const char *kind;

  logger((stderr, "Post subplot processing\n"));

  gr_restorestate();
  args_values(subplot_args, "kind", "s", &kind);
  logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));
  if (str_equals_any(kind, 4, "line", "step", "scatter", "stem") && gr_meta_args_contains(subplot_args, "labels"))
    {
      plot_draw_legend(subplot_args);
    }
}

error_t plot_get_args_in_hierarchy(gr_meta_args_t *args, const char **hierarchy_name_start_ptr, const char *key,
                                   uint_map_t *hierarchy_to_id, const gr_meta_args_t **found_args,
                                   const char ***found_hierarchy_name_ptr)
{
  const char *key_hierarchy_name, **current_hierarchy_name_ptr;
  gr_meta_args_t *current_args, **args_array;
  arg_t *current_arg;
  unsigned int args_array_length, current_id;

  logger((stderr, "Check hierarchy level for key \"%s\"...\n", key));
  return_error_if(!string_map_at(plot_valid_keys_map, key, (char **)&key_hierarchy_name), ERROR_PLOT_UNKNOWN_KEY);
  logger((stderr, "... got hierarchy \"%s\"\n", key_hierarchy_name));
  current_hierarchy_name_ptr = hierarchy_name_start_ptr;
  current_args = args;
  if (strcmp(*hierarchy_name_start_ptr, key_hierarchy_name) != 0)
    {
      while (*++current_hierarchy_name_ptr != NULL)
        {
          current_arg = args_at(current_args, *current_hierarchy_name_ptr);
          return_error_if(current_arg == NULL, ERROR_INTERNAL);
          arg_first_value(current_arg, "A", &args_array, &args_array_length);
          uint_map_at(hierarchy_to_id, *current_hierarchy_name_ptr, &current_id);
          /* Check for the invalid id 0 because id 0 is set for append mode */
          if (current_id == 0)
            {
              current_id = args_array_length + 1;
              if (strcmp(*current_hierarchy_name_ptr, "plots") == 0)
                {
                  int last_plot_in_use = 0;
                  if (args_values(args_array[args_array_length - 1], "in_use", "i", &last_plot_in_use) &&
                      !last_plot_in_use)
                    {
                      /* Use the last already existing plot if it is still empty */
                      --current_id;
                    }
                }
              logger((stderr, "Append mode, set id to \"%u\"\n", current_id));
              uint_map_insert(hierarchy_to_id, *current_hierarchy_name_ptr, current_id);
            }
          if (current_id > args_array_length)
            {
              plot_init_args_structure(current_args, current_hierarchy_name_ptr - 1, current_id);
              arg_first_value(current_arg, "A", &args_array, &args_array_length);
            }
          current_args = args_array[current_id - 1];
          if (strcmp(*current_hierarchy_name_ptr, "plots") == 0)
            {
              int in_use;
              error_t error = NO_ERROR;
              args_values(current_args, "in_use", "i", &in_use);
              if (in_use)
                {
                  error = event_queue_enqueue_update_plot_event(event_queue, current_id - 1);
                }
              else
                {
                  error = event_queue_enqueue_new_plot_event(event_queue, current_id - 1);
                }
              return_if_error;
              gr_meta_args_push(current_args, "in_use", "i", 1);
            }
          if (strcmp(*current_hierarchy_name_ptr, key_hierarchy_name) == 0)
            {
              break;
            }
        }
      return_error_if(*current_hierarchy_name_ptr == NULL, ERROR_INTERNAL);
    }
  if (found_args != NULL)
    {
      *found_args = current_args;
    }
  if (found_hierarchy_name_ptr != NULL)
    {
      *found_hierarchy_name_ptr = current_hierarchy_name_ptr;
    }

  return NO_ERROR;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plotting ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

error_t plot_line(gr_meta_args_t *subplot_args)
{
  gr_meta_args_t **current_series;

  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      double *x, *y;
      unsigned int x_length, y_length;
      char *spec;
      int mask;
      return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      args_values(*current_series, "spec", "s", &spec); /* `spec` is always set */
      mask = gr_uselinespec(spec);
      if (int_equals_any(mask, 5, 0, 1, 3, 4, 5))
        {
          gr_polyline(x_length, x, y);
        }
      if (mask & 2)
        {
          gr_polymarker(x_length, x, y);
        }
      ++current_series;
    }

  return NO_ERROR;
}

error_t plot_step(gr_meta_args_t *subplot_args)
{
  /*
   * Parameters:
   * `x` as double array
   * `y` as double array
   * optional step position `step_where` as string, modes: `pre`, `mid`, `post`, Default: `mid`
   * optional `spec`
   */
  gr_meta_args_t **current_series;

  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      double *x, *y, *x_step_boundaries = NULL, *y_step_values = NULL;
      unsigned int x_length, y_length, mask, i;
      char *spec;
      return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length) && x_length < 1,
                      ERROR_PLOT_MISSING_DATA);
      return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      args_values(*current_series, "spec", "s", &spec); /* `spec` is always set */
      mask = gr_uselinespec(spec);
      if (int_equals_any(mask, 5, 0, 1, 3, 4, 5))
        {
          const char *where;
          args_values(*current_series, "step_where", "s", &where); /* `spec` is always set */
          if (strcmp(where, "pre") == 0)
            {
              x_step_boundaries = calloc(2 * x_length - 1, sizeof(double));
              y_step_values = calloc(2 * x_length - 1, sizeof(double));
              x_step_boundaries[0] = x[0];
              for (i = 1; i < 2 * x_length - 2; i += 2)
                {
                  x_step_boundaries[i] = x[i / 2];
                  x_step_boundaries[i + 1] = x[i / 2 + 1];
                }
              y_step_values[0] = y[0];
              for (i = 1; i < 2 * x_length - 1; i += 2)
                {
                  y_step_values[i] = y_step_values[i + 1] = y[i / 2 + 1];
                }
              gr_polyline(2 * x_length - 1, x_step_boundaries, y_step_values);
            }
          else if (strcmp(where, "post") == 0)
            {
              x_step_boundaries = calloc(2 * x_length - 1, sizeof(double));
              y_step_values = calloc(2 * x_length - 1, sizeof(double));
              for (i = 0; i < 2 * x_length - 2; i += 2)
                {
                  x_step_boundaries[i] = x[i / 2];
                  x_step_boundaries[i + 1] = x[i / 2 + 1];
                }
              x_step_boundaries[2 * x_length - 2] = x[x_length - 1];
              for (i = 0; i < 2 * x_length - 2; i += 2)
                {
                  y_step_values[i] = y_step_values[i + 1] = y[i / 2];
                }
              y_step_values[2 * x_length - 2] = y[x_length - 1];
              gr_polyline(2 * x_length - 1, x_step_boundaries, y_step_values);
            }
          else if (strcmp(where, "mid") == 0)
            {
              x_step_boundaries = calloc(2 * x_length, sizeof(double));
              y_step_values = calloc(2 * x_length, sizeof(double));
              x_step_boundaries[0] = x[0];
              for (i = 1; i < 2 * x_length - 2; i += 2)
                {
                  x_step_boundaries[i] = x_step_boundaries[i + 1] = (x[i / 2] + x[i / 2 + 1]) / 2.0;
                }
              x_step_boundaries[2 * x_length - 1] = x[x_length - 1];
              for (i = 0; i < 2 * x_length - 1; i += 2)
                {
                  y_step_values[i] = y_step_values[i + 1] = y[i / 2];
                }
              gr_polyline(2 * x_length, x_step_boundaries, y_step_values);
            }
          free(x_step_boundaries);
          free(y_step_values);
        }
      if (mask & 2)
        {
          gr_polymarker(x_length, x, y);
        }
      ++current_series;
    }

  return NO_ERROR;
}

error_t plot_scatter(gr_meta_args_t *subplot_args)
{
  /*
   * Parameters:
   * `x` as double array
   * `y` as double array
   * optional marker size `z` as double array
   * optional marker color `c` as double array for each marker or as single integer for all markers
   * optional `markertype` as integer (see: [Marker types](https://gr-framework.org/markertypes.html?highlight=marker))
   */
  gr_meta_args_t **current_series;
  int *previous_marker_type = plot_scatter_markertypes;
  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      double *x = NULL, *y = NULL, *z = NULL, *c = NULL, c_min, c_max;
      unsigned int x_length, y_length, z_length, c_length;
      int i, c_index = -1, markertype;
      return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      if (args_first_value(*current_series, "z", "D", &z, &z_length))
        {
          return_error_if(x_length != z_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
        }
      if (args_values(*current_series, "markertype", "i", &markertype))
        {
          gr_setmarkertype(markertype);
        }
      else
        {
          gr_setmarkertype(*previous_marker_type++);
          if (*previous_marker_type == INT_MAX)
            {
              previous_marker_type = plot_scatter_markertypes;
            }
        }
      if (!args_first_value(*current_series, "c", "D", &c, &c_length) &&
          args_values(*current_series, "c", "i", &c_index))
        {
          if (c_index < 0)
            {
              logger((stderr, "Invalid scatter color %d, using 0 instead\n", c_index));
              c_index = 0;
            }
          else if (c_index > 255)
            {
              logger((stderr, "Invalid scatter color %d, using 255 instead\n", c_index));
              c_index = 255;
            }
        }
      if (z != NULL || c != NULL)
        {
          args_values(subplot_args, "crange", "dd", &c_min, &c_max);
          for (i = 0; i < x_length; i++)
            {
              if (z != NULL)
                {
                  if (i < z_length)
                    {
                      gr_setmarkersize(z[i] / 100.0);
                    }
                  else
                    {
                      gr_setmarkersize(2.0);
                    }
                }
              if (c != NULL)
                {
                  if (i < c_length)
                    {
                      c_index = 1000 + (int)(255 * (c[i] - c_min) / c_max);
                    }
                  else
                    {
                      c_index = 989;
                    }
                  gr_setmarkercolorind(c_index);
                }
              else if (c_index != -1)
                {
                  gr_setmarkercolorind(1000 + c_index);
                }
              gr_polymarker(1, &x[i], &y[i]);
            }
        }
      else
        {
          gr_polymarker(x_length, x, y);
        }
      ++current_series;
    }

  return NO_ERROR;
}

error_t plot_quiver(gr_meta_args_t *subplot_args)
{
  gr_meta_args_t **current_series;

  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
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

error_t plot_stem(gr_meta_args_t *subplot_args)
{
  const double *window;
  double base_line_y[2] = {0.0, 0.0};
  double stem_x[2], stem_y[2] = {0.0};
  gr_meta_args_t **current_series;

  args_values(subplot_args, "window", "D", &window);
  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      double *x, *y;
      unsigned int x_length, y_length;
      char *spec;
      unsigned int i;
      return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      gr_polyline(2, (double *)window, base_line_y);
      gr_setmarkertype(GKS_K_MARKERTYPE_SOLID_CIRCLE);
      args_values(*current_series, "spec", "s", &spec);
      gr_uselinespec(spec);
      for (i = 0; i < x_length; ++i)
        {
          stem_x[0] = stem_x[1] = x[i];
          stem_y[1] = y[i];
          gr_polyline(2, stem_x, stem_y);
        }
      gr_polymarker(x_length, x, y);
      ++current_series;
    }

  return NO_ERROR;
}

error_t plot_hist(gr_meta_args_t *subplot_args)
{
  const double *window;
  double y_min;
  gr_meta_args_t **current_series;

  args_values(subplot_args, "window", "D", &window);
  y_min = window[2];
  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      double *x, *y;
      unsigned int x_length, y_length;
      unsigned int i;
      return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      for (i = 0; i <= y_length; ++i)
        {
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


error_t plot_barplot(gr_meta_args_t *subplot_args)
{
  const double *window;
  gr_meta_args_t **current_series;

  args_values(subplot_args, "window", "D", &window);
  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      double *x, *y;
      unsigned int x_length, y_length;
      unsigned int i;
      return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length + 1, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      for (i = 1; i <= y_length; ++i)
        {
          gr_setfillcolorind(989);
          gr_setfillintstyle(GKS_K_INTSTYLE_SOLID);
          gr_fillrect(x[i - 1], x[i], 0, y[i - 1]);
          gr_setfillcolorind(1);
          gr_setfillintstyle(GKS_K_INTSTYLE_HOLLOW);
          gr_fillrect(x[i - 1], x[i], 0, y[i - 1]);
        }
      ++current_series;
    }

  return NO_ERROR;
}

error_t plot_contour(gr_meta_args_t *subplot_args)
{
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
  if (h == NULL)
    {
      debug_print_malloc_error();
      error = ERROR_MALLOC;
      goto cleanup;
    }
  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      args_first_value(*current_series, "x", "D", &x, &x_length);
      args_first_value(*current_series, "y", "D", &y, &y_length);
      args_first_value(*current_series, "z", "D", &z, &z_length);
      if (x_length == y_length && x_length == z_length)
        {
          if (gridit_x == NULL)
            {
              gridit_x = malloc(PLOT_CONTOUR_GRIDIT_N * sizeof(double));
              gridit_y = malloc(PLOT_CONTOUR_GRIDIT_N * sizeof(double));
              gridit_z = malloc(PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N * sizeof(double));
              if (gridit_x == NULL || gridit_y == NULL || gridit_z == NULL)
                {
                  debug_print_malloc_error();
                  error = ERROR_MALLOC;
                  goto cleanup;
                }
            }
          gr_gridit(x_length, x, y, z, PLOT_CONTOUR_GRIDIT_N, PLOT_CONTOUR_GRIDIT_N, gridit_x, gridit_y, gridit_z);
          for (i = 0; i < PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N; i++)
            {
              z_min = min(gridit_z[i], z_min);
              z_max = max(gridit_z[i], z_max);
            }
          for (i = 0; i < num_levels; ++i)
            {
              h[i] = z_min + (1.0 * i) / num_levels * (z_max - z_min);
            }
          gr_contour(PLOT_CONTOUR_GRIDIT_N, PLOT_CONTOUR_GRIDIT_N, num_levels, gridit_x, gridit_y, h, gridit_z, 1000);
        }
      else
        {
          if (x_length * y_length != z_length)
            {
              error = ERROR_PLOT_COMPONENT_LENGTH_MISMATCH;
              goto cleanup;
            }
          for (i = 0; i < num_levels; ++i)
            {
              h[i] = z_min + (1.0 * i) / num_levels * (z_max - z_min);
            }
          gr_contour(x_length, y_length, num_levels, x, y, h, z, 1000);
        }
      ++current_series;
    }
  if ((error = plot_draw_colorbar(subplot_args, 0.0, num_levels)) != NO_ERROR)
    {
      goto cleanup;
    }

cleanup:
  free(h);
  free(gridit_x);
  free(gridit_y);
  free(gridit_z);

  return error;
}

error_t plot_contourf(gr_meta_args_t *subplot_args)
{
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
  if (h == NULL)
    {
      debug_print_malloc_error();
      error = ERROR_MALLOC;
      goto cleanup;
    }
  args_values(subplot_args, "scale", "i", &scale);
  gr_setscale(scale);
  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      args_first_value(*current_series, "x", "D", &x, &x_length);
      args_first_value(*current_series, "y", "D", &y, &y_length);
      args_first_value(*current_series, "z", "D", &z, &z_length);
      if ((error = plot_draw_colorbar(subplot_args, 0.0, num_levels)) != NO_ERROR)
        {
          goto cleanup;
        }
      gr_setlinecolorind(1);
      if (x_length == y_length && x_length == z_length)
        {
          if (gridit_x == NULL)
            {
              gridit_x = malloc(PLOT_CONTOUR_GRIDIT_N * sizeof(double));
              gridit_y = malloc(PLOT_CONTOUR_GRIDIT_N * sizeof(double));
              gridit_z = malloc(PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N * sizeof(double));
              if (gridit_x == NULL || gridit_y == NULL || gridit_z == NULL)
                {
                  debug_print_malloc_error();
                  error = ERROR_MALLOC;
                  goto cleanup;
                }
            }
          gr_gridit(x_length, x, y, z, PLOT_CONTOUR_GRIDIT_N, PLOT_CONTOUR_GRIDIT_N, gridit_x, gridit_y, gridit_z);
          for (i = 0; i < PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N; i++)
            {
              z_min = min(gridit_z[i], z_min);
              z_max = max(gridit_z[i], z_max);
            }
          for (i = 0; i < num_levels; ++i)
            {
              h[i] = z_min + (1.0 * i) / num_levels * (z_max - z_min);
            }
          gr_contourf(PLOT_CONTOUR_GRIDIT_N, PLOT_CONTOUR_GRIDIT_N, num_levels, gridit_x, gridit_y, h, gridit_z, 1000);
        }
      else
        {
          if (x_length * y_length != z_length)
            {
              error = ERROR_PLOT_COMPONENT_LENGTH_MISMATCH;
              goto cleanup;
            }
          for (i = 0; i < num_levels; ++i)
            {
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

error_t plot_hexbin(gr_meta_args_t *subplot_args)
{
  int nbins;
  gr_meta_args_t **current_series;

  args_values(subplot_args, "nbins", "i", &nbins);
  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      double *x, *y;
      unsigned int x_length, y_length;
      int cntmax;
      return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      cntmax = gr_hexbin(x_length, x, y, nbins);
      /* TODO: return an error in the else case? */
      if (cntmax > 0)
        {
          gr_meta_args_push(subplot_args, "zrange", "dd", 0.0, 1.0 * cntmax);
          plot_draw_colorbar(subplot_args, 0.0, 256);
        }
      ++current_series;
    }

  return NO_ERROR;
}

error_t plot_heatmap(gr_meta_args_t *subplot_args)
{
  const char *kind = NULL;
  gr_meta_args_t **current_series;
  int i, zlim_set, icmap[256], *rgba, *data, zlog;
  unsigned int width, height, z_length;
  double *x, *y, *z, z_min, z_max, c_min, c_max, tmp, zv;

  args_values(subplot_args, "series", "A", &current_series);
  args_values(subplot_args, "kind", "s", &kind);
  return_error_if(!args_first_value(*current_series, "z", "D", &z, &z_length), ERROR_PLOT_MISSING_DATA);
  return_error_if(!args_first_value(*current_series, "x", "D", &x, &width), ERROR_PLOT_MISSING_DATA);
  return_error_if(!args_first_value(*current_series, "y", "D", &y, &height), ERROR_PLOT_MISSING_DATA);
  args_values(subplot_args, "zrange", "dd", &z_min, &z_max);
  if (!args_values(subplot_args, "zlog", "i", &zlog))
    {
      zlog = 0;
    }
  if (zlog)
    {
      z_min = log(z_min);
      z_max = log(z_max);
    }

  if (!args_values(subplot_args, "crange", "dd", &c_min, &c_max))
    {
      c_min = z_min;
      c_max = z_max;
    }
  if (zlog)
    {
      c_min = log(c_min);
      c_max = log(c_max);
    }

  if (str_equals_any(kind, 1, "nonuniformheatmap"))
    {
      --width;
      --height;
    }
  for (i = 0; i < 256; i++)
    {
      gr_inqcolor(1000 + i, icmap + i);
    }

  data = malloc(height * width * sizeof(int));
  if (z_max > z_min)
    {
      for (i = 0; i < width * height; i++)
        {
          if (zlog)
            {
              zv = log(z[i]);
            }
          else
            {
              zv = z[i];
            }

          if (zv > z_max || zv < z_min)
            {
              data[i] = -1;
            }
          else
            {
              data[i] = (int)((zv - c_min) / (c_max - c_min) * 255);
              if (data[i] >= 255)
                {
                  data[i] = 255;
                }
              else if (data[i] < 0)
                {
                  data[i] = 0;
                }
            }
        }
    }
  else
    {
      for (i = 0; i < width * height; i++)
        {
          data[i] = 0;
        }
    }
  rgba = malloc(height * width * sizeof(int));
  if (str_equals_any(kind, 1, "heatmap"))
    {
      for (i = 0; i < height * width; i++)
        {
          if (data[i] == -1)
            {
              rgba[i] = 0;
            }
          else
            {
              rgba[i] = (255 << 24) + icmap[data[i]];
            }
        }
      gr_drawimage(0.5, width + 0.5, height + 0.5, 0.5, width, height, rgba, 0);
    }
  else
    {
      for (i = 0; i < height * width; i++)
        {
          if (data[i] == -1)
            {
              rgba[i] = 1256 + 1; /* Invalid color index -> gr_nonuniformcellarray draws a transparent rectangle */
            }
          else
            {
              rgba[i] = data[i] + 1000;
            }
        }
      gr_nonuniformcellarray(x, y, width, height, 1, 1, width, height, rgba);
    }
  free(rgba);
  free(data);

  plot_draw_colorbar(subplot_args, 0.0, 256);
  return NO_ERROR;
}

error_t plot_wireframe(gr_meta_args_t *subplot_args)
{
  double *gridit_x = NULL, *gridit_y = NULL, *gridit_z = NULL;
  gr_meta_args_t **current_series;
  error_t error = NO_ERROR;

  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      args_first_value(*current_series, "x", "D", &x, &x_length);
      args_first_value(*current_series, "y", "D", &y, &y_length);
      args_first_value(*current_series, "z", "D", &z, &z_length);
      gr_setfillcolorind(0);
      if (x_length == y_length && x_length == z_length)
        {
          if (gridit_x == NULL)
            {
              gridit_x = malloc(PLOT_WIREFRAME_GRIDIT_N * sizeof(double));
              gridit_y = malloc(PLOT_WIREFRAME_GRIDIT_N * sizeof(double));
              gridit_z = malloc(PLOT_WIREFRAME_GRIDIT_N * PLOT_WIREFRAME_GRIDIT_N * sizeof(double));
              if (gridit_x == NULL || gridit_y == NULL || gridit_z == NULL)
                {
                  debug_print_malloc_error();
                  error = ERROR_MALLOC;
                  goto cleanup;
                }
            }
          gr_gridit(x_length, x, y, z, PLOT_WIREFRAME_GRIDIT_N, PLOT_WIREFRAME_GRIDIT_N, gridit_x, gridit_y, gridit_z);
          gr_surface(PLOT_WIREFRAME_GRIDIT_N, PLOT_WIREFRAME_GRIDIT_N, gridit_x, gridit_y, gridit_z,
                     GR_OPTION_FILLED_MESH);
        }
      else
        {
          if (x_length * y_length != z_length)
            {
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

error_t plot_surface(gr_meta_args_t *subplot_args)
{
  double *gridit_x = NULL, *gridit_y = NULL, *gridit_z = NULL;
  gr_meta_args_t **current_series;
  error_t error = NO_ERROR;

  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      args_first_value(*current_series, "x", "D", &x, &x_length);
      args_first_value(*current_series, "y", "D", &y, &y_length);
      args_first_value(*current_series, "z", "D", &z, &z_length);
      /* TODO: add support for GR3 */
      if (x_length == y_length && x_length == z_length)
        {
          if (gridit_x == NULL)
            {
              gridit_x = malloc(PLOT_SURFACE_GRIDIT_N * sizeof(double));
              gridit_y = malloc(PLOT_SURFACE_GRIDIT_N * sizeof(double));
              gridit_z = malloc(PLOT_SURFACE_GRIDIT_N * PLOT_SURFACE_GRIDIT_N * sizeof(double));
              if (gridit_x == NULL || gridit_y == NULL || gridit_z == NULL)
                {
                  debug_print_malloc_error();
                  error = ERROR_MALLOC;
                  goto cleanup;
                }
            }
          gr_gridit(x_length, x, y, z, PLOT_SURFACE_GRIDIT_N, PLOT_SURFACE_GRIDIT_N, gridit_x, gridit_y, gridit_z);
          gr_surface(PLOT_SURFACE_GRIDIT_N, PLOT_SURFACE_GRIDIT_N, gridit_x, gridit_y, gridit_z,
                     GR_OPTION_COLORED_MESH);
        }
      else
        {
          if (x_length * y_length != z_length)
            {
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

error_t plot_plot3(gr_meta_args_t *subplot_args)
{
  gr_meta_args_t **current_series;

  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
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

error_t plot_scatter3(gr_meta_args_t *subplot_args)
{
  gr_meta_args_t **current_series;
  double c_min, c_max;
  unsigned int x_length, y_length, z_length, c_length, i, c_index;
  double *x, *y, *z, *c;
  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      return_error_if(!args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!args_first_value(*current_series, "z", "D", &z, &z_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length || x_length != z_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      gr_setmarkertype(GKS_K_MARKERTYPE_SOLID_CIRCLE);
      if (args_first_value(*current_series, "c", "D", &c, &c_length))
        {
          args_values(subplot_args, "crange", "dd", &c_min, &c_max);
          for (i = 0; i < x_length; i++)
            {
              if (i < c_length)
                {
                  c_index = 1000 + (int)(255 * (c[i] - c_min) / c_max);
                }
              else
                {
                  c_index = 989;
                }
              gr_setmarkercolorind(c_index);
              gr_polymarker3d(1, x + i, y + i, z + i);
            }
        }
      else
        {
          if (args_values(*current_series, "c", "i", &c_index))
            {
              gr_setmarkercolorind(c_index);
            }
          gr_polymarker3d(x_length, x, y, z);
        }
      ++current_series;
    }
  plot_draw_axes(subplot_args, 2);

  return NO_ERROR;
}

error_t plot_imshow(gr_meta_args_t *subplot_args)
{
  gr_meta_args_t **current_series;

  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      /* TODO: Implement me! */
      ++current_series;
    }

  return ERROR_NOT_IMPLEMENTED;
}

error_t plot_isosurface(gr_meta_args_t *subplot_args)
{
  gr_meta_args_t **current_series;

  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      /* TODO: Implement me! */
      ++current_series;
    }

  return ERROR_NOT_IMPLEMENTED;
}

error_t plot_polar(gr_meta_args_t *subplot_args)
{
  const double *window;
  double r_min, r_max, tick;
  int n;
  gr_meta_args_t **current_series;

  args_values(subplot_args, "window", "D", &window);
  r_min = window[2];
  r_max = window[3];
  tick = 0.5 * gr_tick(r_min, r_max);
  n = (int)ceil((r_max - r_min) / tick);
  r_max = r_min + n * tick;
  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
      double *rho, *theta, *x, *y;
      unsigned int rho_length, theta_length;
      char *spec;
      unsigned int i;
      return_error_if(!args_first_value(*current_series, "x", "D", &theta, &theta_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!args_first_value(*current_series, "y", "D", &rho, &rho_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(rho_length != theta_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      x = malloc(rho_length * sizeof(double));
      y = malloc(rho_length * sizeof(double));
      if (x == NULL || y == NULL)
        {
          debug_print_malloc_error();
          free(x);
          free(y);
          return ERROR_MALLOC;
        }
      for (i = 0; i < rho_length; ++i)
        {
          double current_rho = (rho[i] - r_min) / (r_max - r_min);
          x[i] = current_rho * cos(theta[i]);
          y[i] = current_rho * sin(theta[i]);
        }
      args_values(*current_series, "spec", "s", &spec); /* `spec` is always set */
      gr_uselinespec(spec);
      gr_polyline(rho_length, x, y);
      free(x);
      free(y);
      ++current_series;
    }

  return NO_ERROR;
}

error_t plot_trisurf(gr_meta_args_t *subplot_args)
{
  gr_meta_args_t **current_series;

  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
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

error_t plot_tricont(gr_meta_args_t *subplot_args)
{
  double z_min, z_max;
  double *levels;
  int num_levels;
  gr_meta_args_t **current_series;
  int i;

  args_values(subplot_args, "zrange", "dd", &z_min, &z_max);
  args_values(subplot_args, "levels", "i", &num_levels);
  levels = malloc(num_levels * sizeof(double));
  if (levels == NULL)
    {
      debug_print_malloc_error();
      return ERROR_MALLOC;
    }
  for (i = 0; i < num_levels; ++i)
    {
      levels[i] = z_min + ((1.0 * i) / (num_levels - 1)) * (z_max - z_min);
    }
  args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != NULL)
    {
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
  plot_draw_colorbar(subplot_args, 0.0, num_levels);
  free(levels);

  return NO_ERROR;
}

error_t plot_shade(gr_meta_args_t *subplot_args)
{
  gr_meta_args_t **current_shader;
  const char *data_component_names[] = {"x", "y", NULL};
  double *components[2];
  /* char *spec = ""; TODO: read spec from data! */
  int xform, xbins, ybins;
  double **current_component = components;
  const char **current_component_name = data_component_names;
  unsigned int point_count;

  args_values(subplot_args, "series", "A", &current_shader);
  while (*current_component_name != NULL)
    {
      args_first_value(*current_shader, *current_component_name, "D", current_component, &point_count);
      ++current_component_name;
      ++current_component;
    }
  if (!args_values(subplot_args, "xform", "i", &xform))
    {
      xform = 1;
    }
  if (!args_values(subplot_args, "xbins", "i", &xbins))
    {
      xbins = 100;
    }
  if (!args_values(subplot_args, "ybins", "i", &ybins))
    {
      ybins = 100;
    }
  gr_shadepoints(point_count, components[0], components[1], xform, xbins, ybins);

  return NO_ERROR;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ auxiliary drawing functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

error_t plot_draw_axes(gr_meta_args_t *args, unsigned int pass)
{
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

  args_values(args, "kind", "s", &kind);
  args_values(args, "viewport", "D", &viewport);
  args_values(args, "vp", "D", &vp);
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
  if (str_equals_any(kind, 5, "wireframe", "surface", "plot3", "scatter3", "trisurf"))
    {
      args_values(args, "ztick", "d", &z_tick);
      args_values(args, "zorg", "dd", &z_org_low, &z_org_high);
      args_values(args, "zmajor", "i", &z_major_count);
      if (pass == 1)
        {
          gr_grid3d(x_tick, 0, z_tick, x_org_low, y_org_high, z_org_low, 2, 0, 2);
          gr_grid3d(0, y_tick, 0, x_org_low, y_org_high, z_org_low, 0, 2, 0);
        }
      else
        {
          gr_axes3d(x_tick, 0, z_tick, x_org_low, y_org_low, z_org_low, x_major_count, 0, z_major_count, -ticksize);
          gr_axes3d(0, y_tick, 0, x_org_high, y_org_low, z_org_low, 0, y_major_count, 0, ticksize);
        }
    }
  else
    {
      if (str_equals_any(kind, 3, "heatmap", "shade", "nonuniformheatmap"))
        {
          ticksize = -ticksize;
        }
      if (!str_equals_any(kind, 1, "shade"))
        {
          gr_grid(x_tick, y_tick, 0, 0, x_major_count, y_major_count);
        }
      gr_axes(x_tick, y_tick, x_org_low, y_org_low, x_major_count, y_major_count, ticksize);
      gr_axes(x_tick, y_tick, x_org_high, y_org_high, -x_major_count, -y_major_count, -ticksize);
    }

  if (args_values(args, "title", "s", &title))
    {
      gr_savestate();
      gr_settextalign(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);
      gr_textext(0.5 * (viewport[0] + viewport[1]), vp[3], title);
      gr_restorestate();
    }

  if (str_equals_any(kind, 5, "wireframe", "surface", "plot3", "scatter3", "trisurf"))
    {
      if (args_values(args, "xlabel", "s", &x_label) && args_values(args, "ylabel", "s", &y_label) &&
          args_values(args, "zlabel", "s", &z_label))
        {
          gr_titles3d(x_label, y_label, z_label);
        }
    }
  else
    {
      if (args_values(args, "xlabel", "s", &x_label))
        {
          gr_savestate();
          gr_settextalign(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_BOTTOM);
          gr_textext(0.5 * (viewport[0] + viewport[1]), vp[2] + 0.5 * charheight, x_label);
          gr_restorestate();
        }
      if (args_values(args, "ylabel", "s", &y_label))
        {
          gr_savestate();
          gr_settextalign(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);
          gr_setcharup(-1, 0);
          gr_textext(vp[0] + 0.5 * charheight, 0.5 * (viewport[2] + viewport[3]), y_label);
          gr_restorestate();
        }
    }

  return NO_ERROR;
}

error_t plot_draw_polar_axes(gr_meta_args_t *args)
{
  const double *window, *viewport;
  double diag;
  double charheight;
  double r_min, r_max;
  double tick;
  double x[2], y[2];
  int i, n, alpha;
  char text_buffer[PLOT_POLAR_AXES_TEXT_BUFFER];

  args_values(args, "viewport", "D", &viewport);
  diag = sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
              (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));
  charheight = max(0.018 * diag, 0.012);

  args_values(args, "window", "D", &window);
  r_min = window[2];
  r_max = window[3];

  gr_savestate();
  gr_setcharheight(charheight);
  gr_setlinetype(GKS_K_LINETYPE_SOLID);

  tick = 0.5 * gr_tick(r_min, r_max);
  n = (int)ceil((r_max - r_min) / tick);
  for (i = 0; i <= n; i++)
    {
      double r = (i * 1.0) / n;
      if (i % 2 == 0)
        {
          gr_setlinecolorind(88);
          if (i > 0)
            {
              gr_drawarc(-r, r, -r, r, 0, 180);
              gr_drawarc(-r, r, -r, r, 180, 360);
            }
          gr_settextalign(GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);
          x[0] = 0.05;
          y[0] = r;
          gr_wctondc(x, y);
          snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%g", r_min + i * tick);
          gr_text(x[0], y[0], text_buffer);
        }
      else
        {
          gr_setlinecolorind(90);
          gr_drawarc(-r, r, -r, r, 0, 180);
          gr_drawarc(-r, r, -r, r, 180, 360);
        }
    }
  for (alpha = 0; alpha < 360; alpha += 45)
    {
      x[0] = cos(alpha * M_PI / 180.0);
      y[0] = sin(alpha * M_PI / 180.0);
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

error_t plot_draw_legend(gr_meta_args_t *subplot_args)
{
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
  args_values(subplot_args, "viewport", "D", &viewport);
  args_values(subplot_args, "location", "i", &location);
  gr_savestate();
  gr_selntran(0);
  gr_setscale(0);
  w = 0;
  for (current_label = labels; *current_label != NULL; ++current_label)
    {
      gr_inqtextext(0, 0, *(char **)current_label, tbx, tby);
      w = max(w, tbx[2]);
    }

  h = (num_series + 1) * 0.03;
  if (int_equals_any(location, 3, 8, 9, 10))
    {
      px = 0.5 * (viewport[0] + viewport[1] - w);
    }
  else if (int_equals_any(location, 3, 2, 3, 6))
    {
      px = viewport[0] + 0.11;
    }
  else
    {
      px = viewport[1] - 0.05 - w;
    }
  if (int_equals_any(location, 4, 5, 6, 7, 10))
    {
      py = 0.5 * (viewport[2] + viewport[3] + h) - 0.03;
    }
  else if (int_equals_any(location, 3, 3, 4, 8))
    {
      py = viewport[2] + h;
    }
  else
    {
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
  while (*current_series != NULL)
    {
      char *spec;
      int mask;

      gr_savestate();
      args_values(*current_series, "spec", "s", &spec); /* `spec` is always set */
      mask = gr_uselinespec(spec);
      if (int_equals_any(mask, 5, 0, 1, 3, 4, 5))
        {
          legend_symbol_x[0] = px - 0.07;
          legend_symbol_x[1] = px - 0.01;
          legend_symbol_y[0] = py;
          legend_symbol_y[1] = py;
          gr_polyline(2, legend_symbol_x, legend_symbol_y);
        }
      if (mask & 2)
        {
          legend_symbol_x[0] = px - 0.06;
          legend_symbol_x[1] = px - 0.02;
          legend_symbol_y[0] = py;
          legend_symbol_y[1] = py;
          gr_polymarker(2, legend_symbol_x, legend_symbol_y);
        }
      gr_restorestate();
      gr_settextalign(GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);
      if (*current_label != NULL)
        {
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

error_t plot_draw_colorbar(gr_meta_args_t *args, double off, unsigned int colors)
{
  const double *viewport;
  double c_min, c_max;
  int *data;
  double diag, charheight;
  int scale, flip, options;
  unsigned int i;

  gr_savestate();
  args_values(args, "viewport", "D", &viewport);
  if (!args_values(args, "clim", "dd", &c_min, &c_max))
    {
      args_values(args, "zrange", "dd", &c_min, &c_max);
    }
  data = malloc(colors * sizeof(int));
  if (data == NULL)
    {
      debug_print_malloc_error();
      return ERROR_MALLOC;
    }
  for (i = 0; i < colors; ++i)
    {
      data[i] = 1000 + 255 * i / (colors - 1);
    }
  gr_inqscale(&options);
  if (args_values(args, "xflip", "i", &flip) && flip)
    {
      options = (options | GR_OPTION_FLIP_Y) & ~GR_OPTION_FLIP_X;
      gr_setscale(options);
    }
  else if (args_values(args, "yflip", "i", &flip) && flip)
    {
      options = options & ~GR_OPTION_FLIP_Y & ~GR_OPTION_FLIP_X;
      gr_setscale(options);
    }
  else
    {
      options = options & ~GR_OPTION_FLIP_X;
      gr_setscale(options);
    }
  gr_setwindow(0.0, 1.0, c_min, c_max);
  gr_setviewport(viewport[1] + 0.02 + off, viewport[1] + 0.05 + off, viewport[2], viewport[3]);
  gr_cellarray(0, 1, c_max, c_min, 1, colors, 1, 1, 1, colors, data);
  diag = sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
              (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));
  charheight = max(0.016 * diag, 0.012);
  gr_setcharheight(charheight);
  args_values(args, "scale", "i", &scale);
  if (scale & GR_OPTION_Z_LOG)
    {
      gr_setscale(GR_OPTION_Y_LOG);
      gr_axes(0, 2, 1, c_min, 0, 1, 0.005);
    }
  else
    {
      double c_tick = 0.5 * gr_tick(c_min, c_max);
      gr_axes(0, c_tick, 1, c_min, 0, 1, 0.005);
    }
  free(data);
  gr_restorestate();

  return NO_ERROR;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

double find_max_step(unsigned int n, const double *x)
{
  double max_step = 0.0;
  unsigned int i;

  if (n < 2)
    {
      return 0.0;
    }
  for (i = 1; i < n; ++i)
    {
      max_step = max(x[i] - x[i - 1], max_step);
    }

  return max_step;
}

const char *next_fmt_key(const char *kind)
{
  static const char *saved_fmt = NULL;
  static char fmt_key[2] = {0, 0};

  if (kind != NULL)
    {
      string_map_at(fmt_map, kind, (char **)&saved_fmt);
    }
  if (saved_fmt == NULL)
    {
      return NULL;
    }
  fmt_key[0] = *saved_fmt;
  if (*saved_fmt != '\0')
    {
      ++saved_fmt;
    }

  return fmt_key;
}

int get_id_from_args(const gr_meta_args_t *args, int *plot_id, int *subplot_id, int *series_id)
{
  const char *combined_id;
  int _plot_id = -1, _subplot_id = 0, _series_id = 0;

  if (args_values(args, "id", "s", &combined_id))
    {
      const char *valid_id_delims = ":.";
      int *id_ptrs[4], **current_id_ptr;
      char *copied_id_str, *current_id_str;
      size_t segment_length;
      int is_last_segment;

      id_ptrs[0] = &_plot_id;
      id_ptrs[1] = &_subplot_id;
      id_ptrs[2] = &_series_id;
      id_ptrs[3] = NULL;
      if ((copied_id_str = gks_strdup(combined_id)) == NULL)
        {
          debug_print_malloc_error();
          return 0;
        }

      current_id_ptr = id_ptrs;
      current_id_str = copied_id_str;
      is_last_segment = 0;
      while (*current_id_ptr != NULL && !is_last_segment)
        {
          segment_length = strcspn(current_id_str, valid_id_delims);
          if (current_id_str[segment_length] == '\0')
            {
              is_last_segment = 1;
            }
          else
            {
              current_id_str[segment_length] = '\0';
            }
          if (*current_id_str != '\0')
            {
              if (!str_to_uint(current_id_str, (unsigned int *)*current_id_ptr))
                {
                  logger((stderr, "Got an invalid id \"%s\"\n", current_id_str));
                }
              else
                {
                  logger((stderr, "Read id: %d\n", **current_id_ptr));
                }
            }
          ++current_id_ptr;
          ++current_id_str;
        }

      free(copied_id_str);
    }
  else
    {
      args_values(args, "plot_id", "i", &_plot_id);
      args_values(args, "subplot_id", "i", &_subplot_id);
      args_values(args, "series_id", "i", &_series_id);
    }
  /* plot id `0` references the first plot object (implicit object) -> handle it as the first plot and shift all ids by
   * one */
  *plot_id = _plot_id + 1;
  *subplot_id = _subplot_id;
  *series_id = _series_id;

  return _plot_id > 0 || _subplot_id > 0 || _series_id > 0;
}

int get_figure_size(const gr_meta_args_t *plot_args, int *pixel_width, int *pixel_height, double *metric_width,
                    double *metric_height)
{
  double display_metric_width, display_metric_height;
  int display_pixel_width, display_pixel_height;
  double dpm[2], dpi[2];
  int tmp_size_i[2], pixel_size[2];
  double tmp_size_d[2], metric_size[2];
  int i;

  if (plot_args == NULL)
    {
      plot_args = active_plot_args;
    }

#ifdef __EMSCRIPTEN__
  display_metric_width = 0.16384;
  display_metric_height = 0.12288;
  display_pixel_width = 640;
  display_pixel_height = 480;
#else
  gr_inqdspsize(&display_metric_width, &display_metric_height, &display_pixel_width, &display_pixel_height);
#endif
  dpm[0] = display_pixel_width / display_metric_width;
  dpm[1] = display_pixel_height / display_metric_height;
  dpi[0] = dpm[0] * 0.0254;
  dpi[1] = dpm[1] * 0.0254;

  /* TODO: Overwork this calculation */
  if (args_values(plot_args, "figsize", "dd", &tmp_size_d[0], &tmp_size_d[1]))
    {
      for (i = 0; i < 2; ++i)
        {
          pixel_size[i] = round(tmp_size_d[i] * dpi[i]);
          metric_size[i] = tmp_size_d[i] / 0.0254;
        }
    }
  else if (args_values(plot_args, "size", "dd", &tmp_size_d[0], &tmp_size_d[1]))
    {
      if (dpi[0] > 200 || dpi[1] > 200)
        {
          for (i = 0; i < 2; ++i)
            {
              pixel_size[i] = round(tmp_size_d[i] * dpi[i] / 100.0);
              metric_size[i] = tmp_size_d[i] / 0.000254;
            }
        }
      else
        {
          for (i = 0; i < 2; ++i)
            {
              pixel_size[i] = round(tmp_size_d[i]);
              metric_size[i] = tmp_size_d[i] / dpm[i];
            }
        }
    }
  else if (args_values(plot_args, "size", "ii", &tmp_size_i[0], &tmp_size_i[1]))
    {
      for (i = 0; i < 2; ++i)
        {
          pixel_size[i] = tmp_size_i[i];
          metric_size[i] = tmp_size_i[i] / dpm[i];
        }
    }
  else
    {
      /* If this branch is executed, there is an internal error (size has a default value if not set by the user) */
      return 0;
    }

  logger((stderr, "figure pixel size: (%d, %d)\n", pixel_size[0], pixel_size[1]));
  logger((stderr, "device dpi: (%lf, %lf)\n", dpi[0], dpi[1]));

  if (pixel_width != NULL)
    {
      *pixel_width = pixel_size[0];
    }
  if (pixel_height != NULL)
    {
      *pixel_height = pixel_size[1];
    }
  if (metric_width != NULL)
    {
      *metric_width = metric_size[0];
    }
  if (metric_height != NULL)
    {
      *metric_height = metric_size[1];
    }

  return 1;
}

gr_meta_args_t *get_subplot_from_ndc_point(double x, double y)
{
  gr_meta_args_t **subplot_args;
  const double *viewport;

  args_values(active_plot_args, "subplots", "A", &subplot_args);
  while (*subplot_args != NULL)
    {
      if (args_values(*subplot_args, "viewport", "D", &viewport))
        {
          if (viewport[0] <= x && x <= viewport[1] && viewport[2] <= y && y <= viewport[3])
            {
              unsigned int array_index;
              args_values(*subplot_args, "array_index", "i", &array_index);
              logger((stderr, "Found subplot id \"%u\" for ndc point (%lf, %lf)\n", array_index + 1, x, y));

              return *subplot_args;
            }
        }
      ++subplot_args;
    }

  return NULL;
}

gr_meta_args_t *get_subplot_from_ndc_points(unsigned int n, const double *x, const double *y)
{
  gr_meta_args_t *subplot_args;
  unsigned int i;

  for (i = 0, subplot_args = NULL; i < n && subplot_args == NULL; ++i)
    {
      subplot_args = get_subplot_from_ndc_point(x[i], y[i]);
    }

  return subplot_args;
}


/* ------------------------- util ----------------------------------------------------------------------------------- */

size_t djb2_hash(const char *str)
{
  /* String hash function by Dan Bernstein, see http://www.cse.yorku.ca/~oz/hash.html */
  size_t hash = 5381;
  char c;

  while ((c = *str++) != '\0')
    {
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

  return hash;
}

int is_int_number(const char *str)
{
  return strchr(FROMJSON_VALID_DELIMITERS, str[strspn(str, "0123456789-+")]) != NULL;
}

int str_to_uint(const char *str, unsigned int *value_ptr)
{
  char *conversion_end = NULL;
  unsigned long conversion_result;
  int success = 0;

  errno = 0;
  if (str != NULL && *str != '\0')
    {
      conversion_result = strtoul(str, &conversion_end, 10);
    }
  else
    {
      conversion_result = 0;
    }
  if (conversion_end == NULL || *conversion_end != '\0')
    {
      debug_print_error(("The parameter \"%s\" is not a valid number!\n", str));
    }
  else if (errno == ERANGE || conversion_result > UINT_MAX)
    {
      debug_print_error(("The parameter \"%s\" is too big, the number has been clamped to \"%u\"\n", str, UINT_MAX));
      conversion_result = UINT_MAX;
    }
  else
    {
      success = 1;
    }
  if (value_ptr != NULL)
    {
      *value_ptr = (unsigned int)conversion_result;
    }

  return success;
}

int int_equals_any(int number, unsigned int n, ...)
{
  va_list vl;
  int current_number;
  int any_is_equal;
  unsigned int i;

  va_start(vl, n);
  any_is_equal = 0;
  for (i = 0; i < n; i++)
    {
      current_number = va_arg(vl, int);
      if (number == current_number)
        {
          any_is_equal = 1;
          break;
        }
    }
  va_end(vl);

  return any_is_equal;
}

int str_equals_any(const char *str, unsigned int n, ...)
{
  va_list vl;
  char *current_str;
  int any_is_equal;
  unsigned int i;

  va_start(vl, n);
  any_is_equal = 0;
  for (i = 0; i < n; i++)
    {
      current_str = va_arg(vl, char *);
      if (strcmp(str, current_str) == 0)
        {
          any_is_equal = 1;
          break;
        }
    }
  va_end(vl);

  return any_is_equal;
}

int str_equals_any_in_array(const char *str, const char **str_array)
{
  /* `str_array` must be NULL terminated */
  const char **current_str_ptr;
  int any_is_equal;

  any_is_equal = 0;
  current_str_ptr = str_array;
  while (*current_str_ptr != NULL)
    {
      if (strcmp(str, *current_str_ptr) == 0)
        {
          any_is_equal = 1;
          break;
        }
      ++current_str_ptr;
    }

  return any_is_equal;
}

int uppercase_count(const char *str)
{
  int uppercase_count = 0;

  while (*str)
    {
      if (isupper(*str))
        {
          ++uppercase_count;
        }
      ++str;
    }
  return uppercase_count;
}

unsigned long next_or_equal_power2(unsigned long num)
{
#if defined(__GNUC__) || defined(__clang__)
  /* Subtract the count of leading bit zeros from the count of all bits to get `floor(log2(num)) + 1`. If `num` is a
   * power of 2 (only one bit is set), subtract 1 more. The result is `ceil(log2(num))`. Calculate
   * `1 << ceil(log2(num))` to get `exp2(ceil(log2(num)))` which is the power of 2 which is greater or equal than `num`.
   */
  return 1ul << ((CHAR_BIT * sizeof(unsigned long)) - __builtin_clzl(num) - (__builtin_popcountl(num) == 1 ? 1 : 0));
#elif defined(_MSC_VER)
  /* Calculate the index of the highest set bit (bit scan reverse) to get `floor(log2(num)) + 1`. If `num` is a power
   * of 2 (only one bit is set), subtract 1 more. The result is `ceil(log2(num))`. Calculate `1 << ceil(log2(num))`
   * to get `exp2(ceil(log2(num)))` which is the power of 2 which is greater or equal than `num`.
   */
  unsigned long index;
  _BitScanReverse(&index, num);
  return 1ul << (index + 1 - (__popcnt(num) == 1 ? 1 : 0));
#else
  /* Fallback algorithm in software: Shift one bit till the resulting number is greater or equal than `num` */
  unsigned long power = 1;
  while (power < num)
    {
      power <<= 1;
    }
  return power;
#endif
}

static int get_focus_and_factor(const int x1, const int y1, const int x2, const int y2, const int keep_aspect_ratio,
                                double *factor_x, double *factor_y, double *focus_x, double *focus_y,
                                gr_meta_args_t **subplot_args)
{
  double ndc_box_x[4], ndc_box_y[4];
  double ndc_left, ndc_top, ndc_right, ndc_bottom;
  const double *wswindow, *viewport;
  int width, height, max_width_height;

  get_figure_size(NULL, &width, &height, NULL, NULL);
  max_width_height = max(width, height);

  if (x1 <= x2)
    {
      ndc_left = (double)x1 / max_width_height;
      ndc_right = (double)x2 / max_width_height;
    }
  else
    {
      ndc_left = (double)x2 / max_width_height;
      ndc_right = (double)x1 / max_width_height;
    }
  if (y1 <= y2)
    {
      ndc_top = (double)(height - y1) / max_width_height;
      ndc_bottom = (double)(height - y2) / max_width_height;
    }
  else
    {
      ndc_top = (double)(height - y2) / max_width_height;
      ndc_bottom = (double)(height - y1) / max_width_height;
    }

  ndc_box_x[0] = ndc_left;
  ndc_box_y[0] = ndc_bottom;
  ndc_box_x[1] = ndc_right;
  ndc_box_y[1] = ndc_bottom;
  ndc_box_x[2] = ndc_left;
  ndc_box_y[2] = ndc_top;
  ndc_box_x[3] = ndc_right;
  ndc_box_y[3] = ndc_top;
  *subplot_args = get_subplot_from_ndc_points(array_size(ndc_box_x), ndc_box_x, ndc_box_y);
  if (*subplot_args == NULL)
    {
      return 0;
    }
  args_values(*subplot_args, "viewport", "D", &viewport);
  args_values(active_plot_args, "wswindow", "D", &wswindow);

  *factor_x = abs(x1 - x2) / (width * (viewport[1] - viewport[0]) / (wswindow[1] - wswindow[0]));
  *factor_y = abs(y1 - y2) / (height * (viewport[3] - viewport[2]) / (wswindow[3] - wswindow[2]));
  if (keep_aspect_ratio)
    {
      if (*factor_x <= *factor_y)
        {
          *factor_x = *factor_y;
          if (x1 > x2)
            {
              ndc_left = ndc_right - *factor_x * (viewport[1] - viewport[0]);
            }
        }
      else
        {
          *factor_y = *factor_x;
          if (y1 > y2)
            {
              ndc_top = ndc_bottom + *factor_y * (viewport[3] - viewport[2]);
            }
        }
    }
  *focus_x = (ndc_left - *factor_x * viewport[0]) / (1 - *factor_x) - (viewport[0] + viewport[1]) / 2.0;
  *focus_y = (ndc_top - *factor_y * viewport[3]) / (1 - *factor_y) - (viewport[2] + viewport[3]) / 2.0;
  return 1;
}


/* ========================= methods ================================================================================ */

/* ------------------------- argument ------------------------------------------------------------------------------- */

args_value_iterator_t *arg_value_iter(const arg_t *arg)
{
  return args_value_iterator_new(arg);
}

error_t arg_increase_array(arg_t *arg, size_t increment)
{
  size_t *current_size_ptr, new_size;
  void ***current_buffer_ptr, **new_buffer;
  int has_array_terminator;

  return_error_if(arg->value_format[0] != 'n', ERROR_ARGS_INCREASING_NON_ARRAY_VALUE);
  /* Currently, only one dimensional arrays can be increased */
  return_error_if(strlen(arg->value_format) != 2, ERROR_ARGS_INCREASING_MULTI_DIMENSIONAL_ARRAY);

  has_array_terminator = argparse_format_has_array_terminator[tolower(arg->value_format[1])];

  current_size_ptr = (size_t *)arg->value_ptr;
  current_buffer_ptr = (void ***)(current_size_ptr + 1);

  new_size = *current_size_ptr + increment;
  new_buffer = realloc(*current_buffer_ptr, sizeof(void *) * (new_size + (has_array_terminator ? 1 : 0)));
  return_error_if(new_buffer == NULL, ERROR_MALLOC);

  if (has_array_terminator)
    {
      unsigned int i;
      for (i = *current_size_ptr + 1; i < new_size + 1; ++i)
        {
          new_buffer[i] = NULL;
        }
    }

  *current_size_ptr = new_size;
  *current_buffer_ptr = new_buffer;

  return NO_ERROR;
}

int(arg_first_value)(const arg_t *arg, const char *first_value_format, void *first_value, unsigned int *array_length)
{
  char *transformed_first_value_format;
  char first_value_type;
  size_t *size_t_typed_value_ptr;
  void *value_ptr;

  transformed_first_value_format = malloc(2 * strlen(first_value_format) + 1);
  if (transformed_first_value_format == NULL)
    {
      debug_print_malloc_error();
      return 0;
    }
  args_copy_format_string_for_arg(transformed_first_value_format, first_value_format);
  /* check if value_format does not start with the transformed first_value_format */
  if (strncmp(arg->value_format, transformed_first_value_format, strlen(transformed_first_value_format)) != 0)
    {
      free(transformed_first_value_format);
      return 0;
    }
  free(transformed_first_value_format);
  first_value_type = (arg->value_format[0] != 'n') ? arg->value_format[0] : arg->value_format[1];
  if (islower(first_value_type))
    {
      value_ptr = arg->value_ptr;
    }
  else
    {
      size_t_typed_value_ptr = arg->value_ptr;
      if (array_length != NULL)
        {
          *array_length = *size_t_typed_value_ptr;
        }
      value_ptr = (size_t_typed_value_ptr + 1);
    }
  if (first_value != NULL)
    {
      if (isupper(first_value_type))
        {
          /* if the first value is an array simple store the pointer; the type the pointer is pointing to is unimportant
           * in this case so use a void pointer conversion to shorten the code */
          *(void **)first_value = *(void **)value_ptr;
        }
      else
        {
          switch (first_value_type)
            {
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

int arg_values(const arg_t *arg, const char *expected_format, ...)
{
  va_list vl;
  int was_successful;

  va_start(vl, expected_format);

  was_successful = arg_values_vl(arg, expected_format, &vl);

  va_end(vl);

  return was_successful;
}

int arg_values_vl(const arg_t *arg, const char *expected_format, va_list *vl)
{
  args_value_iterator_t *value_it = NULL;
  const char *current_va_format;
  int formats_are_equal = 0;
  int data_offset = 0;
  int was_successful = 0;

  if (!(formats_are_equal = args_check_format_compatibility(arg, expected_format)))
    {
      goto cleanup;
    }
  formats_are_equal = (formats_are_equal == 2);

  current_va_format = expected_format;
  value_it = arg_value_iter(arg);
  if (value_it->next(value_it) == NULL)
    {
      goto cleanup;
    }
  while (*current_va_format != '\0')
    {
      void *current_value_ptr;
      current_value_ptr = va_arg(*vl, void *);
      if (value_it->is_array && isupper(*current_va_format))
        {
          /* If an array is stored and an array format is given by the user, simply assign a pointer to the data. The
           * datatype itself is unimportant in this case. */
          *(void **)current_value_ptr = *(void **)value_it->value_ptr;
        }
      else
        {
          switch (value_it->format)
            {
            case 'i':
              if (value_it->is_array)
                {
                  /* The data is stored as an array but the user wants to assign the values to single variables (the
                   * array case was handled before by the void pointer). -> Assign value by value by incrementing a data
                   * offset in each step. */
                  *(int *)current_value_ptr = (*(int **)value_it->value_ptr)[data_offset++];
                }
              else
                {
                  /* The data is stored as a single value. Assign that value to the variable given by the user. */
                  *(int *)current_value_ptr = *(int *)value_it->value_ptr;
                }
              break;
            case 'd':
              if (value_it->is_array)
                {
                  *(double *)current_value_ptr = (*(double **)value_it->value_ptr)[data_offset++];
                }
              else
                {
                  *(double *)current_value_ptr = *(double *)value_it->value_ptr;
                }
              break;
            case 'c':
              if (value_it->is_array)
                {
                  *(char *)current_value_ptr = (*(char **)value_it->value_ptr)[data_offset++];
                }
              else
                {
                  *(char *)current_value_ptr = *(char *)value_it->value_ptr;
                }
              break;
            case 's':
              if (value_it->is_array)
                {
                  *(char **)current_value_ptr = (*(char ***)value_it->value_ptr)[data_offset++];
                }
              else
                {
                  *(char **)current_value_ptr = *(char **)value_it->value_ptr;
                }
              break;
            case 'a':
              if (value_it->is_array)
                {
                  *(gr_meta_args_t **)current_value_ptr = (*(gr_meta_args_t ***)value_it->value_ptr)[data_offset++];
                }
              else
                {
                  *(gr_meta_args_t **)current_value_ptr = *(gr_meta_args_t **)value_it->value_ptr;
                }
              break;
            default:
              goto cleanup;
            }
        }
      if (formats_are_equal)
        {
          /* Only iterate if the format given by the user is equal to the stored internal format. In the other case, the
           * user requests to store **one** array to single variables -> values are read from one pointer, no iteration
           * is needed. */
          value_it->next(value_it);
          data_offset = 0;
        }
      ++current_va_format;
    }
  was_successful = 1;

cleanup:
  if (value_it != NULL)
    {
      args_value_iterator_delete(value_it);
    }

  return was_successful;
}

/* ------------------------- argument container --------------------------------------------------------------------- */

void args_init(gr_meta_args_t *args)
{
  args->kwargs_head = NULL;
  args->kwargs_tail = NULL;
  args->count = 0;
}

void args_finalize(gr_meta_args_t *args)
{
  gr_meta_args_clear(args);
}

gr_meta_args_t *args_flatcopy(const gr_meta_args_t *copy_args)
{
  /* Clone the linked list but share the referenced values */
  gr_meta_args_t *args = NULL;
  args_iterator_t *it = NULL;
  args_node_t *args_node;
  arg_t *copy_arg;

  args = gr_newmeta();
  if (args == NULL)
    {
      debug_print_malloc_error();
      goto error_cleanup;
    }
  it = args_iter(copy_args);
  while ((copy_arg = it->next(it)) != NULL)
    {
      ++(copy_arg->priv->reference_count);
      args_node = malloc(sizeof(args_node_t));
      if (args_node == NULL)
        {
          debug_print_malloc_error();
          goto error_cleanup;
        }
      args_node->arg = copy_arg;
      args_node->next = NULL;

      if (args->kwargs_head == NULL)
        {
          args->kwargs_head = args_node;
        }
      else
        {
          args->kwargs_tail->next = args_node;
        }
      args->kwargs_tail = args_node;
      ++(args->count);
    }
  args_iterator_delete(it);

  return args;

error_cleanup:
  if (args != NULL)
    {
      gr_deletemeta(args);
    }
  if (it != NULL)
    {
      args_iterator_delete(it);
    }

  return NULL;
}

gr_meta_args_t *args_copy(const gr_meta_args_t *copy_args, const char **keys_copy_as_array, const char **ignore_keys)
{
  /* Clone the linked list and all values that are argument containers as well. Share all other values (-> **no deep
   * copy!**).
   * `keys_copy_as_array` can be used to always copy values of the specified keys as an array. It is only read for
   * values which are argument containers. The array must be terminated with a NULL pointer.
   * `ignore_keys` is an array of keys which will not be copied. The array must be terminated with a NULL pointer. */
  gr_meta_args_t *args = NULL, **args_array = NULL, *copied_args = NULL, **copied_args_array = NULL,
                 **current_args_copy = NULL;
  args_iterator_t *it = NULL;
  args_value_iterator_t *value_it = NULL;
  args_node_t *args_node;
  arg_t *copy_arg;

  args = gr_newmeta();
  if (args == NULL)
    {
      debug_print_malloc_error();
      goto error_cleanup;
    }
  it = args_iter(copy_args);
  error_cleanup_if(it == NULL);
  while ((copy_arg = it->next(it)) != NULL)
    {
      if (ignore_keys != NULL && str_equals_any_in_array(copy_arg->key, ignore_keys))
        {
          continue;
        }
      if (strncmp(copy_arg->value_format, "a", 1) == 0 || strncmp(copy_arg->value_format, "nA", 2) == 0)
        {
          value_it = arg_value_iter(copy_arg);
          error_cleanup_if(value_it == NULL);
          /* Do not support two dimensional argument arrays like `nAnA`) -> a loop would be needed with more memory
           * management */
          error_cleanup_if(value_it->next(value_it) == NULL);
          if (value_it->is_array)
            {
              args_array = *(gr_meta_args_t ***)value_it->value_ptr;
              copied_args_array = malloc(value_it->array_length * sizeof(gr_meta_args_t *));
              error_cleanup_if(copied_args_array == NULL);
              current_args_copy = copied_args_array;
              while (*args_array != NULL)
                {
                  *current_args_copy = args_copy(*args_array, keys_copy_as_array, ignore_keys);
                  error_cleanup_if(*current_args_copy == NULL);
                  ++args_array;
                  ++current_args_copy;
                }
              current_args_copy = NULL;
              gr_meta_args_push(args, it->arg->key, "nA", value_it->array_length, copied_args_array);
            }
          else
            {
              copied_args = args_copy(*(gr_meta_args_t **)value_it->value_ptr, keys_copy_as_array, ignore_keys);
              error_cleanup_if(copied_args == NULL);
              if (keys_copy_as_array != NULL && str_equals_any_in_array(it->arg->key, keys_copy_as_array))
                {
                  gr_meta_args_push(args, it->arg->key, "A(1)", &copied_args);
                }
              else
                {
                  gr_meta_args_push(args, it->arg->key, "a", copied_args);
                }
              copied_args = NULL;
            }
        }
      else
        {
          ++(copy_arg->priv->reference_count);
          args_node = malloc(sizeof(args_node_t));
          if (args_node == NULL)
            {
              debug_print_malloc_error();
              goto error_cleanup;
            }
          args_node->arg = copy_arg;
          args_node->next = NULL;

          if (args->kwargs_head == NULL)
            {
              args->kwargs_head = args_node;
            }
          else
            {
              args->kwargs_tail->next = args_node;
            }
          args->kwargs_tail = args_node;
          ++(args->count);
        }
    }
  goto cleanup;

error_cleanup:
  if (args != NULL)
    {
      gr_deletemeta(args);
      args = NULL;
    }
cleanup:
  if (current_args_copy != NULL)
    {
      while (current_args_copy != copied_args_array)
        {
          if (*current_args_copy != NULL)
            {
              gr_deletemeta(*current_args_copy);
            }
          --current_args_copy;
        }
    }
  if (copied_args_array != NULL)
    {
      free(copied_args_array);
    }
  if (it != NULL)
    {
      args_iterator_delete(it);
    }
  if (value_it != NULL)
    {
      args_value_iterator_delete(value_it);
    }

  return args;
}

error_t args_push_common(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                         va_list *vl, int apply_padding)
{
  arg_t *arg;
  args_node_t *args_node;

  if ((arg = args_create_args(key, value_format, buffer, vl, apply_padding)) == NULL)
    {
      return ERROR_MALLOC;
    }

  if ((args_node = args_find_node(args, key)) != NULL)
    {
      args_decrease_arg_reference_count(args_node);
      args_node->arg = arg;
    }
  else
    {
      args_node = malloc(sizeof(args_node_t));
      if (args_node == NULL)
        {
          debug_print_malloc_error();
          free((char *)arg->key);
          free((char *)arg->value_format);
          free(arg->priv);
          free(arg);
          return ERROR_MALLOC;
        }
      args_node->arg = arg;
      args_node->next = NULL;
      if (args->kwargs_head == NULL)
        {
          args->kwargs_head = args_node;
          args->kwargs_tail = args_node;
        }
      else
        {
          args->kwargs_tail->next = args_node;
          args->kwargs_tail = args_node;
        }
      ++(args->count);
    }

  return NO_ERROR;
}

error_t args_push_vl(gr_meta_args_t *args, const char *key, const char *value_format, va_list *vl)
{
  return args_push_common(args, key, value_format, NULL, vl, 0);
}

error_t args_push_arg(gr_meta_args_t *args, arg_t *arg)
{
  args_node_t *args_node = NULL, *previous_node_by_keyword = NULL;
  error_t error = NO_ERROR;

  ++(arg->priv->reference_count);
  args_node = malloc(sizeof(args_node_t));
  error_cleanup_and_set_error_if(args_node == NULL, ERROR_MALLOC);
  args_node->arg = arg;
  args_node->next = NULL;

  if (args->kwargs_head == NULL)
    {
      args->kwargs_head = args_node;
      args->kwargs_tail = args_node;
      ++(args->count);
    }
  else if (args_find_previous_node(args, arg->key, &previous_node_by_keyword))
    {
      if (previous_node_by_keyword == NULL)
        {
          args_node->next = args->kwargs_head->next;
          if (args->kwargs_head == args->kwargs_tail)
            {
              args->kwargs_tail = args_node;
            }
          args_decrease_arg_reference_count(args->kwargs_head);
          free(args->kwargs_head);
          args->kwargs_head = args_node;
        }
      else
        {
          args_node->next = previous_node_by_keyword->next->next;
          args_decrease_arg_reference_count(previous_node_by_keyword->next);
          free(previous_node_by_keyword->next);
          previous_node_by_keyword->next = args_node;
          if (args_node->next == NULL)
            {
              args->kwargs_tail = args_node;
            }
        }
    }
  else
    {
      args->kwargs_tail->next = args_node;
      args->kwargs_tail = args_node;
      ++(args->count);
    }

  return NO_ERROR;

error_cleanup:
  if (args_node != NULL)
    {
      free(args_node);
    }
  return error;
}

error_t args_update_many(gr_meta_args_t *args, const gr_meta_args_t *update_args)
{
  return args_merge(args, update_args, NULL);
}

error_t args_merge(gr_meta_args_t *args, const gr_meta_args_t *merge_args, const char *const *merge_keys)
{
  args_iterator_t *it = NULL;
  args_value_iterator_t *value_it = NULL, *merge_value_it = NULL;
  arg_t *update_arg, *current_arg;
  gr_meta_args_t **args_array, **merge_args_array;
  const char *const *current_key_ptr;
  int merge;
  unsigned int i;
  error_t error = NO_ERROR;

  it = args_iter(merge_args);
  cleanup_and_set_error_if(it == NULL, ERROR_MALLOC);
  while ((update_arg = it->next(it)) != NULL)
    {
      merge = 0;
      if (merge_keys != NULL)
        {
          current_key_ptr = merge_keys;
          while (*current_key_ptr != NULL)
            {
              if (strcmp(it->arg->key, *current_key_ptr) == 0)
                {
                  merge = 1;
                  break;
                }
              ++current_key_ptr;
            }
        }
      if (merge && (current_arg = args_at(args, update_arg->key)) != NULL)
        {
          value_it = arg_value_iter(current_arg);
          merge_value_it = arg_value_iter(update_arg);
          cleanup_and_set_error_if(value_it == NULL, ERROR_MALLOC);
          cleanup_and_set_error_if(merge_value_it == NULL, ERROR_MALLOC);
          /* Do not support two dimensional argument arrays like `nAnA`) -> a loop would be needed with more memory
           * management */
          cleanup_and_set_error_if(value_it->next(value_it) == NULL, ERROR_MALLOC);
          cleanup_and_set_error_if(merge_value_it->next(merge_value_it) == NULL, ERROR_MALLOC);
          if (value_it->is_array)
            {
              args_array = *(gr_meta_args_t ***)value_it->value_ptr;
            }
          else
            {
              args_array = (gr_meta_args_t **)value_it->value_ptr;
            }
          if (merge_value_it->is_array)
            {
              merge_args_array = *(gr_meta_args_t ***)merge_value_it->value_ptr;
            }
          else
            {
              merge_args_array = (gr_meta_args_t **)merge_value_it->value_ptr;
            }
          for (i = 0; i < value_it->array_length && i < merge_value_it->array_length; ++i)
            {
              error = args_merge(args_array[i], merge_args_array[i], merge_keys);
              cleanup_if_error;
            }
        }
      else
        {
          error = args_push_arg(args, update_arg);
          cleanup_if_error;
        }
    }

cleanup:
  if (it != NULL)
    {
      args_iterator_delete(it);
    }
  if (value_it != NULL)
    {
      args_value_iterator_delete(value_it);
    }
  if (merge_value_it != NULL)
    {
      args_value_iterator_delete(merge_value_it);
    }

  return error;
}

error_t args_setdefault_common(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                               va_list *vl, int apply_padding)
{
  if (!gr_meta_args_contains(args, key))
    {
      return args_push_common(args, key, value_format, buffer, vl, apply_padding);
    }
  return NO_ERROR;
}

error_t args_setdefault(gr_meta_args_t *args, const char *key, const char *value_format, ...)
{
  error_t error;
  va_list vl;
  va_start(vl, value_format);

  error = args_setdefault_vl(args, key, value_format, &vl);

  va_end(vl);

  return error;
}

error_t args_setdefault_buf(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                            int apply_padding)
{
  return args_setdefault_common(args, key, value_format, buffer, NULL, apply_padding);
}

error_t args_setdefault_vl(gr_meta_args_t *args, const char *key, const char *value_format, va_list *vl)
{
  return args_setdefault_common(args, key, value_format, NULL, vl, 0);
}

void args_clear(gr_meta_args_t *args, const char **exclude_keys)
{
  args_node_t *current_node, *next_node, *last_excluded_node;

  current_node = args->kwargs_head;
  last_excluded_node = NULL;
  while (current_node != NULL)
    {
      next_node = current_node->next;
      if (exclude_keys != NULL && str_equals_any_in_array(current_node->arg->key, exclude_keys))
        {
          if (last_excluded_node == NULL)
            {
              args->kwargs_head = current_node;
            }
          else
            {
              last_excluded_node->next = current_node;
            }
          last_excluded_node = current_node;
        }
      else
        {
          args_decrease_arg_reference_count(current_node);
          free(current_node);
          --(args->count);
        }
      current_node = next_node;
    }
  args->kwargs_tail = last_excluded_node;
  if (args->kwargs_tail == NULL)
    {
      args->kwargs_head = NULL;
    }
  else
    {
      args->kwargs_tail->next = NULL;
    }
}

error_t args_increase_array(gr_meta_args_t *args, const char *key, size_t increment)
{
  arg_t *arg;

  arg = args_at(args, key);
  return_error_if(arg == NULL, ERROR_ARGS_INVALID_KEY);
  return arg_increase_array(arg, increment);
}

unsigned int args_count(const gr_meta_args_t *args)
{
  return args->count;
}

arg_t *args_at(const gr_meta_args_t *args, const char *keyword)
{
  args_node_t *current_node;

  current_node = args_find_node(args, keyword);

  if (current_node != NULL)
    {
      return current_node->arg;
    }
  return NULL;
}

int(args_first_value)(const gr_meta_args_t *args, const char *keyword, const char *first_value_format,
                      void *first_value, unsigned int *array_length)
{
  arg_t *arg;

  arg = args_at(args, keyword);
  if (arg == NULL)
    {
      return 0;
    }

  return arg_first_value(arg, first_value_format, first_value, array_length);
}

int args_values(const gr_meta_args_t *args, const char *keyword, const char *expected_format, ...)
{
  va_list vl;
  arg_t *arg;
  int was_successful = 0;

  va_start(vl, expected_format);

  arg = args_at(args, keyword);
  cleanup_if(arg == NULL);

  was_successful = arg_values_vl(arg, expected_format, &vl);

cleanup:
  va_end(vl);

  return was_successful;
}

args_node_t *args_find_node(const gr_meta_args_t *args, const char *keyword)
{
  args_node_t *current_node;

  current_node = args->kwargs_head;
  while (current_node != NULL && strcmp(current_node->arg->key, keyword) != 0)
    {
      current_node = current_node->next;
    }

  return current_node;
}

int args_find_previous_node(const gr_meta_args_t *args, const char *keyword, args_node_t **previous_node)
{
  args_node_t *prev_node, *current_node;

  prev_node = NULL;
  current_node = args->kwargs_head;
  while (current_node != NULL && strcmp(current_node->arg->key, keyword) != 0)
    {
      prev_node = current_node;
      current_node = current_node->next;
    }

  if (current_node != NULL)
    {
      *previous_node = prev_node;
      return 1;
    }
  return 0;
}

args_iterator_t *args_iter(const gr_meta_args_t *args)
{
  return args_iterator_new(args->kwargs_head, NULL);
}


/* ------------------------- argument iterator ---------------------------------------------------------------------- */

args_iterator_t *args_iterator_new(const args_node_t *begin, const args_node_t *end)
{
  args_iterator_t *args_iterator;

  args_iterator = malloc(sizeof(args_iterator_t));
  if (args_iterator == NULL)
    {
      debug_print_malloc_error();
      return NULL;
    }
  args_iterator->priv = malloc(sizeof(args_iterator_private_t));
  if (args_iterator->priv == NULL)
    {
      debug_print_malloc_error();
      free(args_iterator);
      return NULL;
    }
  args_iterator_init(args_iterator, begin, end);
  return args_iterator;
}

void args_iterator_init(args_iterator_t *args_iterator, const args_node_t *begin, const args_node_t *end)
{
  args_iterator->next = args_iterator_next;
  args_iterator->arg = NULL;
  args_iterator->priv->next_node = begin;
  args_iterator->priv->end = end;
}

void args_iterator_delete(args_iterator_t *args_iterator)
{
  args_iterator_finalize(args_iterator);
  free(args_iterator->priv);
  free(args_iterator);
}

void args_iterator_finalize(args_iterator_t *args_iterator UNUSED) {}

arg_t *args_iterator_next(args_iterator_t *args_iterator)
{
  arg_t *next_arg;

  if ((args_iterator->priv->next_node != NULL) && (args_iterator->priv->next_node != args_iterator->priv->end))
    {
      next_arg = args_iterator->priv->next_node->arg;
      args_iterator->priv->next_node = args_iterator->priv->next_node->next;
    }
  else
    {
      next_arg = NULL;
    }
  args_iterator->arg = next_arg;
  return next_arg;
}


/* ------------------------- value iterator ------------------------------------------------------------------------- */

args_value_iterator_t *args_value_iterator_new(const arg_t *arg)
{
  args_value_iterator_t *args_value_iterator;

  args_value_iterator = malloc(sizeof(args_value_iterator_t));
  if (args_value_iterator == NULL)
    {
      debug_print_malloc_error();
      return NULL;
    }
  args_value_iterator->priv = malloc(sizeof(args_value_iterator_private_t));
  if (args_value_iterator->priv == NULL)
    {
      debug_print_malloc_error();
      free(args_value_iterator);
      return NULL;
    }
  args_value_iterator_init(args_value_iterator, arg);
  return args_value_iterator;
}

void args_value_iterator_init(args_value_iterator_t *args_value_iterator, const arg_t *arg)
{
  args_value_iterator->next = args_value_iterator_next;
  args_value_iterator->value_ptr = NULL;
  args_value_iterator->format = '\0';
  args_value_iterator->is_array = 0;
  args_value_iterator->array_length = 0;
  args_value_iterator->priv->value_buffer = arg->value_ptr;
  args_value_iterator->priv->value_format = arg->value_format;
}

void args_value_iterator_delete(args_value_iterator_t *args_value_iterator)
{
  args_value_iterator_finalize(args_value_iterator);
  free(args_value_iterator->priv);
  free(args_value_iterator);
}

void args_value_iterator_finalize(args_value_iterator_t *args_value_iterator UNUSED) {}

void *args_value_iterator_next(args_value_iterator_t *args_value_iterator)
{
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
  while (*format)
    {
      format = args_skip_option(format);
      if (!*format)
        {
          break;
        }
      current_format = tolower(*format);
      if (current_format != *format)
        {
          is_array = 1;
          array_length = *((size_t *)value_buffer);
          value_buffer = ((size_t *)value_buffer) + 1;
          value_ptr = value_buffer;
        }
#define STEP_VALUE_BUFFER_BY_TYPE(char, type)       \
  case char:                                        \
    if (is_array)                                   \
      {                                             \
        value_buffer = ((type **)value_buffer) + 1; \
      }                                             \
    else                                            \
      {                                             \
        value_buffer = ((type *)value_buffer) + 1;  \
      }                                             \
    break;

      switch (current_format)
        {
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

      if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, current_format) != NULL)
        {
          extracted_next_value = 1;
          break;
        }
      ++format;
    }

  if (extracted_next_value)
    {
      args_value_iterator->is_array = is_array;
      args_value_iterator->array_length = array_length;
      args_value_iterator->format = current_format;
      args_value_iterator->priv->value_format = ++format; /* FIXME: does not work for options! */
    }
  else
    {
      value_ptr = NULL;
      args_value_iterator->format = '\0';
    }

  args_value_iterator->priv->value_buffer = value_buffer;
  args_value_iterator->value_ptr = value_ptr;
  return value_ptr;
}


/* ------------------------- dynamic args array --------------------------------------------------------------------- */

dynamic_args_array_t *dynamic_args_array_new(void)
{
  dynamic_args_array_t *args_array;

  args_array = malloc(sizeof(dynamic_args_array_t));
  if (args_array == NULL)
    {
      return NULL;
    }
  args_array->buf = malloc(DYNAMIC_ARGS_ARRAY_INITIAL_SIZE * sizeof(gr_meta_args_t *));
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
      gr_deletemeta(args_array->buf[i]);
    }
  dynamic_args_array_delete(args_array);
}

error_t dynamic_args_array_push_back(dynamic_args_array_t *args_array, gr_meta_args_t *args)
{
  if (args_array->size == args_array->capacity)
    {
      gr_meta_args_t **enlarged_buf = realloc(
          args_array->buf, (args_array->capacity + DYNAMIC_ARGS_ARRAY_SIZE_INCREMENT) * sizeof(gr_meta_args_t *));
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


/* ------------------------- json deserializer ---------------------------------------------------------------------- */

int gr_readmeta(gr_meta_args_t *args, const char *json_string)
{
  return (fromjson_read(args, json_string) == NO_ERROR);
}

static error_t fromjson_read(gr_meta_args_t *args, const char *json_string)
{
  return fromjson_parse(args, json_string, NULL);
}

int gr_load_from_str(const char *json_string)
{
  return (fromjson_read(active_plot_args, json_string) == NO_ERROR);
}

error_t fromjson_parse(gr_meta_args_t *args, const char *json_string, fromjson_shared_state_t *shared_state)
{
  char *filtered_json_string = NULL;
  fromjson_state_t state;
  int allocated_shared_state_mem = 0;
  error_t error = NO_ERROR;

  state.datatype = JSON_DATATYPE_UNKNOWN;
  state.value_buffer = NULL;
  state.value_buffer_pointer_level = 0;
  state.next_value_memory = NULL;
  state.next_value_type = malloc(NEXT_VALUE_TYPE_SIZE);
  if (state.next_value_type == NULL)
    {
      debug_print_malloc_error();
      return ERROR_MALLOC;
    }
  state.args = args;
  if (shared_state == NULL)
    {
      shared_state = malloc(sizeof(fromjson_shared_state_t));
      if (shared_state == NULL)
        {
          free(state.next_value_type);
          debug_print_malloc_error();
          return ERROR_MALLOC;
        }
      if ((error = fromjson_copy_and_filter_json_string(&filtered_json_string, json_string)) != NO_ERROR)
        {
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
  if (state.parsing_object)
    {
      ++state.shared_state->json_ptr;
    }

  while (strchr("}", *state.shared_state->json_ptr) == NULL)
    {
      const char *current_key = NULL;

      if (state.parsing_object)
        {
          fromjson_parse_string(&state);
          current_key = *(const char **)state.value_buffer;
          free(state.value_buffer);
          state.value_buffer = NULL;
          ++(state.shared_state->json_ptr);
        }
      state.datatype = fromjson_check_type(&state);
      if (state.datatype)
        {
          if ((error = fromjson_datatype_to_func[state.datatype](&state)) != NO_ERROR)
            {
              break;
            }
          if (state.parsing_object)
            {
              gr_meta_args_push_buf(args, current_key, state.next_value_type, state.value_buffer, 0);
            }
          else
            {
              /* parsing values without an outer object (-> missing key) is not supported by the argument container */
              error = ERROR_PARSE_MISSING_OBJECT_CONTAINER;
              break;
            }
          if (strchr(FROMJSON_VALID_DELIMITERS, *state.shared_state->json_ptr) != NULL)
            {
              if (*state.shared_state->json_ptr == ',')
                {
                  ++state.shared_state->json_ptr;
                }
            }
          else
            {
              error = ERROR_PARSE_INVALID_DELIMITER;
              break;
            }
        }
      else
        {
          error = ERROR_PARSE_UNKNOWN_DATATYPE;
          break;
        }
      if (state.value_buffer_pointer_level > 1)
        {
          int i, outer_array_length = uppercase_count(state.next_value_type);
          for (i = 0; i < outer_array_length; ++i)
            {
              free(((char **)state.value_buffer)[i]);
            }
        }
      free(state.value_buffer);
      state.value_buffer = NULL;
      state.value_buffer_pointer_level = 0;
    }
  if (state.parsing_object && *state.shared_state->json_ptr == '\0')
    {
      error = ERROR_PARSE_INCOMPLETE_STRING;
    }
  if (*state.shared_state->json_ptr)
    {
      ++state.shared_state->json_ptr;
    }

  free(state.value_buffer);
  free(filtered_json_string);
  free(state.next_value_type);

  if (allocated_shared_state_mem)
    {
      free(shared_state);
    }

  return error;
}

#define CHECK_AND_ALLOCATE_MEMORY(type, length)                \
  do                                                           \
    {                                                          \
      if (state->value_buffer == NULL)                         \
        {                                                      \
          state->value_buffer = malloc(length * sizeof(type)); \
          if (state->value_buffer == NULL)                     \
            {                                                  \
              debug_print_malloc_error();                      \
              return 0;                                        \
            }                                                  \
          state->value_buffer_pointer_level = 1;               \
          state->next_value_memory = state->value_buffer;      \
        }                                                      \
    }                                                          \
  while (0)

error_t fromjson_parse_null(fromjson_state_t *state)
{
  if (strncmp(state->shared_state->json_ptr, "null", 4) != 0)
    {
      return ERROR_PARSE_NULL;
    }
  strcpy(state->next_value_type, "");
  state->shared_state->json_ptr += 4;
  return NO_ERROR;
}

error_t fromjson_parse_bool(fromjson_state_t *state)
{
  int bool_value;

  if (strncmp(state->shared_state->json_ptr, "true", 4) == 0)
    {
      bool_value = 1;
    }
  else if (strncmp(state->shared_state->json_ptr, "false", 5) == 0)
    {
      bool_value = 0;
    }
  else
    {
      return ERROR_PARSE_BOOL;
    }
  CHECK_AND_ALLOCATE_MEMORY(int, 1);
  *((int *)state->next_value_memory) = bool_value;
  strcpy(state->next_value_type, "i");
  state->shared_state->json_ptr += bool_value ? 4 : 5;
  return NO_ERROR;
}

error_t fromjson_parse_number(fromjson_state_t *state)
{
  error_t error;

  if (is_int_number(state->shared_state->json_ptr))
    {
      error = fromjson_parse_int(state);
    }
  else
    {
      error = fromjson_parse_double(state);
    }
  return error;
}

error_t fromjson_parse_int(fromjson_state_t *state)
{
  int was_successful;
  int int_value;

  int_value = fromjson_str_to_int((const char **)&state->shared_state->json_ptr, &was_successful);
  if (!was_successful)
    {
      return ERROR_PARSE_INT;
    }
  CHECK_AND_ALLOCATE_MEMORY(int, 1);
  *((int *)state->next_value_memory) = int_value;
  strcpy(state->next_value_type, "i");
  return NO_ERROR;
}

error_t fromjson_parse_double(fromjson_state_t *state)
{
  int was_successful;
  double double_value;

  double_value = fromjson_str_to_double((const char **)&state->shared_state->json_ptr, &was_successful);
  if (!was_successful)
    {
      return ERROR_PARSE_DOUBLE;
    }
  CHECK_AND_ALLOCATE_MEMORY(double, 1);
  *((double *)state->next_value_memory) = double_value;
  strcpy(state->next_value_type, "d");
  return NO_ERROR;
}

error_t fromjson_parse_string(fromjson_state_t *state)
{
  char *string_value;
  char *json_ptr;
  const char *src_ptr;
  char *dest_ptr;
  int string_is_complete;
  int skipped_char;

  CHECK_AND_ALLOCATE_MEMORY(char *, 1);
  json_ptr = state->shared_state->json_ptr;
  string_value = ++json_ptr;
  while (*json_ptr && !is_string_delimiter(json_ptr, string_value))
    {
      ++json_ptr;
    }
  string_is_complete = (*json_ptr != '\0');
  *json_ptr = '\0';
  /* Unescape '"' and '\' (since '\' is the escape character) */
  src_ptr = dest_ptr = string_value;
  skipped_char = 0;
  while (*src_ptr)
    {
      if (*src_ptr == '\\' && !skipped_char)
        {
          ++src_ptr;
          skipped_char = 1;
        }
      else
        {
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

error_t fromjson_parse_array(fromjson_state_t *state)
{
  fromjson_datatype_t json_datatype;
  const char *next_delim_ptr;

#define PARSE_VALUES(parse_suffix, c_type)                                                                         \
  do                                                                                                               \
    {                                                                                                              \
      c_type *values;                                                                                              \
      c_type *current_value_ptr;                                                                                   \
      CHECK_AND_ALLOCATE_MEMORY(c_type *, 1);                                                                      \
      values = malloc(array_length * sizeof(c_type));                                                              \
      if (values == NULL)                                                                                          \
        {                                                                                                          \
          debug_print_malloc_error();                                                                              \
          return ERROR_MALLOC;                                                                                     \
        }                                                                                                          \
      current_value_ptr = values;                                                                                  \
      *(c_type **)state->next_value_memory = values;                                                               \
      state->value_buffer_pointer_level = 2;                                                                       \
      state->next_value_memory = values;                                                                           \
      --state->shared_state->json_ptr;                                                                             \
      while (!error && strchr("]", *state->shared_state->json_ptr) == NULL)                                        \
        {                                                                                                          \
          ++state->shared_state->json_ptr;                                                                         \
          error = fromjson_parse_##parse_suffix(state);                                                            \
          ++current_value_ptr;                                                                                     \
          state->next_value_memory = current_value_ptr;                                                            \
        }                                                                                                          \
      snprintf(array_type + strlen(array_type), NEXT_VALUE_TYPE_SIZE, "%c(%lu)", toupper(*state->next_value_type), \
               array_length);                                                                                      \
    }                                                                                                              \
  while (0)

  if (strchr("]", *(state->shared_state->json_ptr + 1)) == NULL)
    {
      char array_type[NEXT_VALUE_TYPE_SIZE];
      size_t outer_array_length = 0;
      int is_nested_array = (*(state->shared_state->json_ptr + 1) == '[');
      unsigned int current_outer_array_index = 0;
      if (is_nested_array)
        {
          outer_array_length = fromjson_get_outer_array_length(state->shared_state->json_ptr);
          /* `char *` is only used as a generic type since all pointers to values have the same storage size */
          CHECK_AND_ALLOCATE_MEMORY(char *, outer_array_length);
        }
      array_type[0] = '\0';
      do
        {
          error_t error = NO_ERROR;
          size_t array_length = 0;
          state->shared_state->json_ptr += is_nested_array ? 2 : 1;
          next_delim_ptr = state->shared_state->json_ptr;
          while (*next_delim_ptr != ']' &&
                 fromjson_find_next_delimiter(&next_delim_ptr, next_delim_ptr, array_length == 0, 1))
            {
              ++array_length;
            }
          if (*next_delim_ptr != ']')
            {
              state->shared_state->json_ptr -= is_nested_array ? 2 : 1;
              return ERROR_PARSE_INCOMPLETE_STRING;
            }
          assert(array_length > 0);
          json_datatype = fromjson_check_type(state);
          switch (json_datatype)
            {
            case JSON_DATATYPE_NUMBER:
              if (is_int_number(state->shared_state->json_ptr))
                {
                  PARSE_VALUES(int, int);
                }
              else
                {
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
              debug_print_error(("The datatype \"%s\" is currently not supported in arrays!\n",
                                 fromjson_datatype_to_string[json_datatype]));
              state->shared_state->json_ptr -= is_nested_array ? 2 : 1;
              return ERROR_PARSE_ARRAY;
              break;
            }
          ++(state->shared_state->json_ptr);
          if (is_nested_array)
            {
              ++current_outer_array_index;
              /* `char **` is only used as a generic type since all pointers to values have the same storage size */
              state->next_value_memory = ((char **)state->value_buffer) + current_outer_array_index;
            }
        }
      while (is_nested_array && current_outer_array_index < outer_array_length &&
             strchr("]", *state->shared_state->json_ptr) == NULL);
      if (is_nested_array && *state->shared_state->json_ptr == ']')
        {
          ++state->shared_state->json_ptr;
        }
      strcpy(state->next_value_type, array_type);
    }
  else
    {
      strcpy(state->next_value_type, "I(0)");
    }

  return NO_ERROR;

#undef PARSE_VALUES
}

error_t fromjson_parse_object(fromjson_state_t *state)
{
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

fromjson_datatype_t fromjson_check_type(const fromjson_state_t *state)
{
  fromjson_datatype_t datatype;

  datatype = JSON_DATATYPE_UNKNOWN;
  switch (*state->shared_state->json_ptr)
    {
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
  if (datatype == JSON_DATATYPE_UNKNOWN)
    {
      if (*state->shared_state->json_ptr == 'n')
        {
          datatype = JSON_DATATYPE_NULL;
        }
      else if (strchr("ft", *state->shared_state->json_ptr) != NULL)
        {
          datatype = JSON_DATATYPE_BOOL;
        }
      else
        {
          datatype = JSON_DATATYPE_NUMBER;
        }
    }
  return datatype;
}

error_t fromjson_copy_and_filter_json_string(char **dest, const char *src)
{
  const char *src_ptr;
  char *dest_buffer, *dest_ptr;
  int in_string;

  src_ptr = src;
  dest_buffer = malloc(strlen(src) + 1);
  if (dest_buffer == NULL)
    {
      debug_print_malloc_error();
      return ERROR_MALLOC;
    }
  dest_ptr = dest_buffer;

  in_string = 0;
  while (*src_ptr)
    {
      if (is_string_delimiter(src_ptr, src))
        {
          in_string = !in_string;
        }
      if (in_string || !isspace(*src_ptr))
        {
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
                                 int exclude_nested_structures)
{
  if (*src == '\0')
    {
      return 0;
    }
  if (!include_start)
    {
      ++src;
    }
  if (exclude_nested_structures)
    {
      const char *src_ptr = src;
      int nested_level = 0;

      while (*src_ptr)
        {
          if (strchr("[{", *src_ptr))
            {
              ++nested_level;
            }
          else if (strchr("]}", *src_ptr))
            {
              if (nested_level > 0)
                {
                  --nested_level;
                }
              else
                {
                  break;
                }
            }
          else if (*src_ptr == ',' && nested_level == 0)
            {
              break;
            }
          ++src_ptr;
        }
      if (*src_ptr != '\0')
        {
          *delim_ptr = src_ptr;
          return 1;
        }
    }
  else
    {
      size_t segment_length = strcspn(src, FROMJSON_VALID_DELIMITERS);
      if (*(src + segment_length) != '\0')
        {
          *delim_ptr = src + segment_length;
          return 1;
        }
    }
  return 0;
}

size_t fromjson_get_outer_array_length(const char *str)
{
  size_t outer_array_length = 0;
  int current_array_level = 1;

  if (*str != '[')
    {
      return outer_array_length;
    }
  ++str;
  while (current_array_level > 0 && *str)
    {
      switch (*str)
        {
        case '[':
          ++current_array_level;
          break;
        case ']':
          --current_array_level;
          break;
        case ',':
          if (current_array_level == 1)
            {
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

double fromjson_str_to_double(const char **str, int *was_successful)
{
  char *conversion_end = NULL;
  double conversion_result;
  int success = 0;
  const char *next_delim_ptr = NULL;

  errno = 0;
  if (*str != NULL)
    {
      conversion_result = strtod(*str, &conversion_end);
    }
  else
    {
      conversion_result = 0.0;
    }
  if (conversion_end == NULL)
    {
      debug_print_error(("No number conversion was executed (the string is NULL)!\n"));
    }
  else if (*str == conversion_end || strchr(FROMJSON_VALID_DELIMITERS, *conversion_end) == NULL)
    {
      fromjson_find_next_delimiter(&next_delim_ptr, *str, 1, 0);
      debug_print_error(("The parameter \"%.*s\" is not a valid number!\n", next_delim_ptr - *str, *str));
    }
  else if (errno == ERANGE)
    {
      fromjson_find_next_delimiter(&next_delim_ptr, *str, 1, 0);
      if (conversion_result == HUGE_VAL || conversion_result == -HUGE_VAL)
        {
          debug_print_error(("The parameter \"%.*s\" caused an overflow, the number has been clamped to \"%lf\"\n",
                             next_delim_ptr - *str, *str, conversion_result));
        }
      else
        {
          debug_print_error(("The parameter \"%.*s\" caused an underflow, the number has been clamped to \"%lf\"\n",
                             next_delim_ptr - *str, *str, conversion_result));
        }
    }
  else
    {
      success = 1;
      *str = conversion_end;
    }
  if (was_successful != NULL)
    {
      *was_successful = success;
    }

  return conversion_result;
}

int fromjson_str_to_int(const char **str, int *was_successful)
{
  char *conversion_end = NULL;
  long conversion_result;
  int success = 0;
  const char *next_delim_ptr = NULL;

  errno = 0;
  if (*str != NULL)
    {
      conversion_result = strtol(*str, &conversion_end, 10);
    }
  else
    {
      conversion_result = 0;
    }
  if (conversion_end == NULL)
    {
      debug_print_error(("No number conversion was executed (the string is NULL)!\n"));
    }
  else if (*str == conversion_end || strchr(FROMJSON_VALID_DELIMITERS, *conversion_end) == NULL)
    {
      fromjson_find_next_delimiter(&next_delim_ptr, *str, 1, 0);
      debug_print_error(("The parameter \"%.*s\" is not a valid number!\n", next_delim_ptr - *str, *str));
    }
  else if (errno == ERANGE || conversion_result > INT_MAX || conversion_result < INT_MIN)
    {
      fromjson_find_next_delimiter(&next_delim_ptr, *str, 1, 0);
      if (conversion_result > INT_MAX)
        {
          debug_print_error(("The parameter \"%.*s\" is too big, the number has been clamped to \"%d\"\n",
                             next_delim_ptr - *str, *str, INT_MAX));
          conversion_result = INT_MAX;
        }
      else
        {
          debug_print_error(("The parameter \"%.*s\" is too small, the number has been clamped to \"%d\"\n",
                             next_delim_ptr - *str, *str, INT_MIN));
          conversion_result = INT_MIN;
        }
    }
  else
    {
      success = 1;
      *str = conversion_end;
    }
  if (was_successful != NULL)
    {
      *was_successful = success;
    }

  return (int)conversion_result;
}


/* ------------------------- json serializer ------------------------------------------------------------------------ */

#define CHECK_PADDING(type)                                                             \
  do                                                                                    \
    {                                                                                   \
      if (state->shared->data_ptr != NULL && state->shared->apply_padding)              \
        {                                                                               \
          ptrdiff_t needed_padding = state->shared->data_offset % sizeof(type);         \
          state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding; \
          state->shared->data_offset += needed_padding;                                 \
        }                                                                               \
    }                                                                                   \
  while (0)

#define RETRIEVE_SINGLE_VALUE(var, type, promoted_type)                    \
  do                                                                       \
    {                                                                      \
      CHECK_PADDING(type);                                                 \
      if (state->shared->data_ptr != NULL)                                 \
        {                                                                  \
          var = *((type *)state->shared->data_ptr);                        \
          state->shared->data_ptr = ((type *)state->shared->data_ptr) + 1; \
          state->shared->data_offset += sizeof(type);                      \
        }                                                                  \
      else                                                                 \
        {                                                                  \
          var = va_arg(*state->shared->vl, promoted_type);                 \
        }                                                                  \
    }                                                                      \
  while (0)

#define INIT_MULTI_VALUE(vars, type)                 \
  do                                                 \
    {                                                \
      if (state->shared->data_ptr != NULL)           \
        {                                            \
          CHECK_PADDING(type *);                     \
          vars = *(type **)state->shared->data_ptr;  \
        }                                            \
      else                                           \
        {                                            \
          vars = va_arg(*state->shared->vl, type *); \
        }                                            \
    }                                                \
  while (0)

#define FIN_MULTI_VALUE(type)                                               \
  do                                                                        \
    {                                                                       \
      if (state->shared->data_ptr != NULL)                                  \
        {                                                                   \
          state->shared->data_ptr = ((type **)state->shared->data_ptr) + 1; \
          state->shared->data_offset += sizeof(type *);                     \
        }                                                                   \
    }                                                                       \
  while (0)

#define DEFINE_STRINGIFY_VALUE(name, type, format_specifier)                  \
  error_t tojson_stringify_##name##_value(memwriter_t *memwriter, type value) \
  {                                                                           \
    return memwriter_printf(memwriter, format_specifier, value);              \
  }

#define DEFINE_STRINGIFY_SINGLE(name, type, promoted_type)                              \
  error_t tojson_stringify_##name(tojson_state_t *state)                                \
  {                                                                                     \
    type value;                                                                         \
    error_t error = NO_ERROR;                                                           \
    RETRIEVE_SINGLE_VALUE(value, type, promoted_type);                                  \
    if ((error = tojson_stringify_##name##_value(state->memwriter, value)) != NO_ERROR) \
      {                                                                                 \
        return error;                                                                   \
      }                                                                                 \
    state->shared->wrote_output = 1;                                                    \
    return error;                                                                       \
  }

#define DEFINE_STRINGIFY_MULTI(name, type)                                                                \
  error_t tojson_stringify_##name##_array(tojson_state_t *state)                                          \
  {                                                                                                       \
    type *values;                                                                                         \
    type current_value;                                                                                   \
    unsigned int length;                                                                                  \
    int remaining_elements;                                                                               \
    error_t error = NO_ERROR;                                                                             \
    INIT_MULTI_VALUE(values, type);                                                                       \
    if (state->additional_type_info != NULL)                                                              \
      {                                                                                                   \
        if (!str_to_uint(state->additional_type_info, &length))                                           \
          {                                                                                               \
            debug_print_error(                                                                            \
                ("The given array length \"%s\" is no valid number; the array contents will be ignored.", \
                 state->additional_type_info));                                                           \
            length = 0;                                                                                   \
          }                                                                                               \
      }                                                                                                   \
    else                                                                                                  \
      {                                                                                                   \
        length = state->shared->array_length;                                                             \
      }                                                                                                   \
    remaining_elements = length;                                                                          \
    /* write array start */                                                                               \
    if ((error = memwriter_putc(state->memwriter, '[')) != NO_ERROR)                                      \
      {                                                                                                   \
        return error;                                                                                     \
      }                                                                                                   \
    /* write array content */                                                                             \
    while (remaining_elements)                                                                            \
      {                                                                                                   \
        current_value = *values++;                                                                        \
        if ((error = tojson_stringify_##name##_value(state->memwriter, current_value)) != NO_ERROR)       \
          {                                                                                               \
            return error;                                                                                 \
          }                                                                                               \
        if (remaining_elements > 1)                                                                       \
          {                                                                                               \
            if ((error = memwriter_putc(state->memwriter, ',')) != NO_ERROR)                              \
              {                                                                                           \
                return error;                                                                             \
              }                                                                                           \
          }                                                                                               \
        --remaining_elements;                                                                             \
      }                                                                                                   \
    /* write array end */                                                                                 \
    if ((error = memwriter_putc(state->memwriter, ']')) != NO_ERROR)                                      \
      {                                                                                                   \
        return error;                                                                                     \
      }                                                                                                   \
    FIN_MULTI_VALUE(type);                                                                                \
    state->shared->wrote_output = 1;                                                                      \
    return error;                                                                                         \
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

error_t tojson_stringify_double_value(memwriter_t *memwriter, double value)
{
  error_t error;
  size_t string_start_index;
  const char *unprocessed_string;

  string_start_index = memwriter_size(memwriter);
  if ((error = memwriter_printf(memwriter, "%." XSTR(DBL_DECIMAL_DIG) "g", value)) != NO_ERROR)
    {
      return error;
    }
  unprocessed_string = memwriter_buf(memwriter) + string_start_index;
  if (strspn(unprocessed_string, "0123456789-") == memwriter_size(memwriter) - string_start_index)
    {
      if ((error = memwriter_putc(memwriter, '.')) != NO_ERROR)
        {
          return error;
        }
    }
  return NO_ERROR;
}

#undef XSTR
#undef STR

error_t tojson_stringify_char_array(tojson_state_t *state)
{
  char *chars;
  char *escaped_chars = NULL;
  unsigned int length;
  error_t error = NO_ERROR;

  INIT_MULTI_VALUE(chars, char);

  if (state->additional_type_info != NULL)
    {
      if (!str_to_uint(state->additional_type_info, &length))
        {
          debug_print_error(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                             state->additional_type_info));
          goto cleanup;
        }
    }
  else
    {
      if (state->shared->read_length_from_string)
        {
          length = 0;
        }
      else
        {
          length = state->shared->array_length;
        }
    }
  if ((error = tojson_escape_special_chars(&escaped_chars, chars, &length)) != NO_ERROR)
    {
      goto cleanup;
    }
  if ((error = memwriter_printf(state->memwriter, "\"%.*s\"", length, escaped_chars)) != NO_ERROR)
    {
      goto cleanup;
    }
  state->shared->wrote_output = 1;

  FIN_MULTI_VALUE(char);

cleanup:
  free(escaped_chars);
  return error;
}

error_t tojson_stringify_string_value(memwriter_t *memwriter, char *value)
{
  char *escaped_chars = NULL;
  unsigned int length = 0;
  error_t error = NO_ERROR;

  if ((error = tojson_escape_special_chars(&escaped_chars, value, &length)))
    {
      goto cleanup;
    }
  if ((error = memwriter_printf(memwriter, "\"%s\"", escaped_chars)) != NO_ERROR)
    {
      goto cleanup;
    }

cleanup:
  free(escaped_chars);
  return error;
}

error_t tojson_stringify_bool_value(memwriter_t *memwriter, int value)
{
  return memwriter_puts(memwriter, value ? "true" : "false");
}

error_t tojson_stringify_object(tojson_state_t *state)
{
  char **member_names = NULL;
  char **data_types = NULL;
  char **member_name_ptr;
  char **data_type_ptr;
  int has_members;
  error_t error = NO_ERROR;

  /* IMPORTANT: additional_type_info is altered after the unzip call! */
  if ((error = tojson_unzip_membernames_and_datatypes(state->additional_type_info, &member_names, &data_types)) !=
      NO_ERROR)
    {
      goto cleanup;
    }
  member_name_ptr = member_names;
  data_type_ptr = data_types;

  has_members =
      (member_name_ptr != NULL && *member_name_ptr != NULL && data_type_ptr != NULL && *data_type_ptr != NULL);
  /* write object start */
  if (!state->add_data_without_separator)
    {
      if (state->shared->add_data && has_members)
        {
          if ((error = memwriter_putc(state->memwriter, ',')) != NO_ERROR)
            {
              goto cleanup;
            }
        }
      else if (!state->shared->add_data)
        {
          if ((error = memwriter_putc(state->memwriter, '{')) != NO_ERROR)
            {
              goto cleanup;
            }
          ++(state->shared->struct_nested_level);
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
          if ((error = memwriter_printf(state->memwriter, "\"%s\":", *member_name_ptr)) != NO_ERROR)
            {
              goto cleanup;
            }
          if ((error = tojson_serialize(state->memwriter, *data_type_ptr, NULL, NULL, -1, -1, 0, NULL, NULL,
                                        state->shared)) != NO_ERROR)
            {
              goto cleanup;
            }
          ++member_name_ptr;
          ++data_type_ptr;
          if (*member_name_ptr != NULL && *data_type_ptr != NULL)
            {
              /* write JSON separator */
              if ((error = memwriter_putc(state->memwriter, ',')) != NO_ERROR)
                {
                  goto cleanup;
                }
            }
          else
            {
              serialized_all_members = 1;
            }
        }
    }
  /* write object end if the type info is complete */
  if (!state->is_type_info_incomplete)
    {
      --(state->shared->struct_nested_level);
      if ((error = memwriter_putc(state->memwriter, '}')) != NO_ERROR)
        {
          goto cleanup;
        }
    }
  /* Only set serial result if not set before */
  if (state->shared->serial_result == 0 && state->is_type_info_incomplete)
    {
      state->shared->serial_result = has_members ? incomplete : incomplete_at_struct_beginning;
    }

cleanup:
  free(member_names);
  free(data_types);
  if (error != NO_ERROR)
    {
      return error;
    }

  state->shared->wrote_output = 1;

  return NO_ERROR;
}

error_t tojson_stringify_args_value(memwriter_t *memwriter, gr_meta_args_t *args)
{
  error_t error = NO_ERROR;

  if ((error = memwriter_putc(memwriter, '{')) != NO_ERROR)
    {
      return error;
    }
  tojson_permanent_state.serial_result = incomplete_at_struct_beginning;
  if ((error = tojson_write_args(memwriter, args)) != NO_ERROR)
    {
      return error;
    }

  return NO_ERROR;
}

int tojson_get_member_count(const char *data_desc)
{
  int nested_level = 0;
  int member_count = 0;
  if (data_desc == NULL || *data_desc == '\0')
    {
      return 0;
    }
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

int tojson_is_json_array_needed(const char *data_desc)
{
  const char *relevant_data_types = "iIdDcCs";
  int nested_level = 0;
  int count_relevant_data_types = 0;

  while (*data_desc != 0 && count_relevant_data_types < 2)
    {
      if (*data_desc == '(')
        {
          ++nested_level;
        }
      else if (*data_desc == ')')
        {
          --nested_level;
        }
      else if (nested_level == 0 && strchr(relevant_data_types, *data_desc))
        {
          ++count_relevant_data_types;
        }
      ++data_desc;
    }
  return count_relevant_data_types >= 2;
}

void tojson_read_datatype(tojson_state_t *state)
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

error_t tojson_skip_bytes(tojson_state_t *state)
{
  unsigned int count;

  if (state->shared->data_ptr == NULL)
    {
      debug_print_error(("Skipping bytes is not supported when using the variable argument list and is ignored.\n"));
      return NO_ERROR;
    }

  if (state->additional_type_info != NULL)
    {
      if (!str_to_uint(state->additional_type_info, &count))
        {
          debug_print_error(("Byte skipping with an invalid number -> ignoring.\n"));
          return NO_ERROR;
        }
    }
  else
    {
      count = 1;
    }
  state->shared->data_ptr = ((char *)state->shared->data_ptr) + count;
  state->shared->data_offset += count;

  return NO_ERROR;
}

error_t tojson_close_object(tojson_state_t *state)
{
  error_t error;
  --(state->shared->struct_nested_level);
  if ((error = memwriter_putc(state->memwriter, '}')) != NO_ERROR)
    {
      return error;
    }
  return NO_ERROR;
}

error_t tojson_read_array_length(tojson_state_t *state)
{
  int value;

  RETRIEVE_SINGLE_VALUE(value, size_t, size_t);
  state->shared->array_length = value;

  return NO_ERROR;
}

error_t tojson_unzip_membernames_and_datatypes(char *mixed_ptr, char ***member_name_ptr, char ***data_type_ptr)
{
  int member_count;
  char **arrays[2];

  member_count = tojson_get_member_count(mixed_ptr);
  /* add 1 to member count for a terminatory NULL pointer */
  *member_name_ptr = malloc((member_count + 1) * sizeof(char *));
  *data_type_ptr = malloc((member_count + 1) * sizeof(char *));
  if (*member_name_ptr == NULL || *data_type_ptr == NULL)
    {
      free(*member_name_ptr);
      free(*data_type_ptr);
      *member_name_ptr = *data_type_ptr = NULL;
      debug_print_malloc_error();
      return ERROR_MALLOC;
    }
  arrays[member_name] = *member_name_ptr;
  arrays[data_type] = *data_type_ptr;
  if (member_count > 0)
    {
      char separators[2] = {':', ','};
      int current_array_index = member_name;
      int nested_type_level = 0;
      *arrays[current_array_index]++ = mixed_ptr;

      /* iterate over the whole type list */
      assert(mixed_ptr != NULL); /* otherwise there is an internal logical error since member_count > 0 */
      while (nested_type_level >= 0 && *mixed_ptr != 0)
        {
          /* extract one name or one type */
          while (*mixed_ptr != 0 && (nested_type_level > 0 || *mixed_ptr != separators[current_array_index]))
            {
              if (current_array_index == data_type)
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
              if (nested_type_level >= 0)
                {
                  ++mixed_ptr;
                }
            }
          if (*mixed_ptr != 0)
            {
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

error_t tojson_escape_special_chars(char **escaped_string, const char *unescaped_string, unsigned int *length)
{
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
  while (remaining_chars)
    {
      if (strchr(chars_to_escape, *src_ptr) != NULL)
        {
          ++needed_memory;
        }
      ++src_ptr;
      --remaining_chars;
    }
  dest_ptr = malloc(needed_memory);
  if (dest_ptr == NULL)
    {
      return ERROR_MALLOC;
    }
  *escaped_string = dest_ptr;
  src_ptr = unescaped_string;
  remaining_chars = len;
  while (remaining_chars)
    {
      if (strchr(chars_to_escape, *src_ptr) != NULL)
        {
          *dest_ptr++ = '\\';
        }
      *dest_ptr++ = *src_ptr++;
      --remaining_chars;
    }
  *dest_ptr = '\0';
  if (length != NULL)
    {
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
                         tojson_serialization_result_t *serial_result, tojson_shared_state_t *shared_state)
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
  if (shared_state == NULL)
    {
      shared_state = malloc(sizeof(tojson_shared_state_t));
      if (shared_state == NULL)
        {
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
    }
  else
    {
      if (data != NULL)
        {
          shared_state->data_ptr = data;
        }
      if (vl != NULL)
        {
          shared_state->vl = vl;
        }
      if (apply_padding >= 0)
        {
          shared_state->apply_padding = apply_padding;
        }
    }
  state.shared = shared_state;

  json_array_needed = tojson_is_json_array_needed(data_desc);
  /* write list head if needed */
  if (json_array_needed)
    {
      if ((error = memwriter_putc(memwriter, '[')) != NO_ERROR)
        {
          goto cleanup;
        }
    }
  while (*state.data_type_ptr != 0)
    {
      shared_state->wrote_output = 0;
      tojson_read_datatype(&state);
      if (tojson_datatype_to_func[(unsigned char)state.current_data_type])
        {
          error = tojson_datatype_to_func[(unsigned char)state.current_data_type](&state);
        }
      else
        {
          debug_print_error(("WARNING: '%c' (ASCII code %d) is not a valid type identifier\n", state.current_data_type,
                             state.current_data_type));
          error = ERROR_UNSUPPORTED_DATATYPE;
        }
      if (error != NO_ERROR)
        {
          goto cleanup;
        }
      if (*state.data_type_ptr != 0 && *state.data_type_ptr != ')' && shared_state->wrote_output)
        {
          /* write JSON separator, if data was written and the object is not closed in the next step */
          if ((error = memwriter_putc(memwriter, ',')) != NO_ERROR)
            {
              goto cleanup;
            }
        }
    }
  /* write list tail if needed */
  if (json_array_needed)
    {
      if ((error = memwriter_putc(memwriter, ']')) != NO_ERROR)
        {
          goto cleanup;
        }
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
          *serial_result = (shared_state->struct_nested_level > 0) ? incomplete : complete;
        }
    }
  if (struct_nested_level != NULL)
    {
      *struct_nested_level = shared_state->struct_nested_level;
    }

cleanup:
  if (allocated_shared_state_mem)
    {
      free(shared_state);
    }

  return error;
}

void tojson_init_static_variables(void)
{
  if (!tojson_static_variables_initialized)
    {
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

error_t tojson_init_variables(int *add_data, int *add_data_without_separator, char **_data_desc, const char *data_desc)
{
  tojson_init_static_variables();
  *add_data = (tojson_permanent_state.serial_result != complete);
  *add_data_without_separator = (tojson_permanent_state.serial_result == incomplete_at_struct_beginning);
  if (*add_data)
    {
      char *data_desc_ptr;
      int data_desc_len = strlen(data_desc);
      *_data_desc = malloc(data_desc_len + 3);
      if (*_data_desc == NULL)
        {
          debug_print_malloc_error();
          return ERROR_MALLOC;
        }
      data_desc_ptr = *_data_desc;
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
      *_data_desc = gks_strdup(data_desc);
      if (*_data_desc == NULL)
        {
          debug_print_malloc_error();
          return ERROR_MALLOC;
        }
    }

  return NO_ERROR;
}

error_t tojson_write_vl(memwriter_t *memwriter, const char *data_desc, va_list *vl)
{
  int add_data, add_data_without_separator;
  char *_data_desc;
  error_t error;

  error = tojson_init_variables(&add_data, &add_data_without_separator, &_data_desc, data_desc);
  if (!error)
    {
      error =
          tojson_serialize(memwriter, _data_desc, NULL, vl, 0, add_data, add_data_without_separator,
                           &tojson_permanent_state.struct_nested_level, &tojson_permanent_state.serial_result, NULL);
    }
  free(_data_desc);

  return error;
}

error_t tojson_write_buf(memwriter_t *memwriter, const char *data_desc, const void *buffer, int apply_padding)
{
  int add_data, add_data_without_separator;
  char *_data_desc;
  error_t error;

  error = tojson_init_variables(&add_data, &add_data_without_separator, &_data_desc, data_desc);
  if (!error)
    {
      error =
          tojson_serialize(memwriter, _data_desc, buffer, NULL, apply_padding, add_data, add_data_without_separator,
                           &tojson_permanent_state.struct_nested_level, &tojson_permanent_state.serial_result, NULL);
    }
  free(_data_desc);

  return error;
}

error_t tojson_write_arg(memwriter_t *memwriter, const arg_t *arg)
{
  error_t error = NO_ERROR;

  if (arg->key == NULL)
    {
      if ((error = tojson_write_buf(memwriter, arg->value_format, arg->value_ptr, 1)) != NO_ERROR)
        {
          return error;
        }
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
      if ((error = tojson_write_buf(memwriter, format, arg->value_ptr, 1)) != NO_ERROR)
        {
          free(format);
          return error;
        }
      free(format);
    }

  return error;
}

error_t tojson_write_args(memwriter_t *memwriter, const gr_meta_args_t *args)
{
  const char *key_hierarchy_name;
  args_iterator_t *it;
  arg_t *arg;

  it = args_iter(args);
  if ((arg = it->next(it)))
    {
      tojson_write_buf(memwriter, "o(", NULL, 1);
      do
        {
          tojson_write_arg(memwriter, arg);
        }
      while ((arg = it->next(it)));
      tojson_write_buf(memwriter, ")", NULL, 1);
    }
  args_iterator_delete(it);

  return 0;
}

int tojson_is_complete(void)
{
  return tojson_permanent_state.serial_result == complete;
}

int tojson_struct_nested_level(void)
{
  return tojson_permanent_state.struct_nested_level;
}


/* ------------------------- memwriter ------------------------------------------------------------------------------ */

memwriter_t *memwriter_new()
{
  memwriter_t *memwriter;

  memwriter = malloc(sizeof(memwriter_t));
  if (memwriter == NULL)
    {
      debug_print_malloc_error();
      return NULL;
    }
  memwriter->buf = malloc(MEMWRITER_INITIAL_SIZE);
  if (memwriter->buf == NULL)
    {
      free(memwriter);
      debug_print_malloc_error();
      return NULL;
    }
  memwriter->size = 0;
  memwriter->capacity = MEMWRITER_INITIAL_SIZE;

  return memwriter;
}

void memwriter_delete(memwriter_t *memwriter)
{
  if (memwriter != NULL)
    {
      free(memwriter->buf);
      free(memwriter);
    }
}

void memwriter_clear(memwriter_t *memwriter)
{
  memwriter->size = 0;
  *memwriter->buf = '\0';
}

error_t memwriter_replace(memwriter_t *memwriter, int index, int count, const char *replacement_str)
{
  int replacement_str_len = (replacement_str != NULL) ? strlen(replacement_str) : 0;
  error_t error = NO_ERROR;

  if ((replacement_str_len > count) &&
      (error = memwriter_ensure_buf(memwriter, replacement_str_len - count)) != NO_ERROR)
    {
      return error;
    }
  if (count != replacement_str_len)
    {
      memmove(memwriter->buf + index + replacement_str_len, memwriter->buf + index + count,
              memwriter->size - (index + count));
    }
  if (replacement_str != NULL)
    {
      memcpy(memwriter->buf + index, replacement_str, replacement_str_len);
    }
  memwriter->size += replacement_str_len - count;

  return error;
}

error_t memwriter_erase(memwriter_t *memwriter, int index, int count)
{
  return memwriter_replace(memwriter, index, count, NULL);
}

error_t memwriter_insert(memwriter_t *memwriter, int index, const char *str)
{
  return memwriter_replace(memwriter, index, 0, str);
}

error_t memwriter_enlarge_buf(memwriter_t *memwriter, size_t size_increment)
{
  void *new_buf;

  if (size_increment == 0)
    {
      if (memwriter->capacity >= MEMWRITER_EXPONENTIAL_INCREASE_UNTIL)
        {
          size_increment = MEMWRITER_LINEAR_INCREMENT_SIZE;
        }
      else
        {
          size_increment = memwriter->capacity;
        }
    }
  else
    {
      /* round up to the next `MEMWRITER_LINEAR_INCREMENT_SIZE` step */
      if (memwriter->capacity >= MEMWRITER_EXPONENTIAL_INCREASE_UNTIL)
        {
          size_increment =
              ((size_increment - 1) / MEMWRITER_LINEAR_INCREMENT_SIZE + 1) * MEMWRITER_LINEAR_INCREMENT_SIZE;
        }
      else
        {
          size_increment = next_or_equal_power2(memwriter->capacity + size_increment) - memwriter->capacity;
        }
    }
  new_buf = realloc(memwriter->buf, memwriter->capacity + size_increment);
  if (new_buf == NULL)
    {
      debug_print_malloc_error();
      return ERROR_MALLOC;
    }
  memwriter->buf = new_buf;
  memwriter->capacity += size_increment;

  return NO_ERROR;
}

error_t memwriter_ensure_buf(memwriter_t *memwriter, size_t needed_additional_size)
{
  if (memwriter->size + needed_additional_size > memwriter->capacity)
    {
      return memwriter_enlarge_buf(memwriter, memwriter->size + needed_additional_size - memwriter->capacity);
    }
  return NO_ERROR;
}

error_t memwriter_printf(memwriter_t *memwriter, const char *format, ...)
{
  va_list vl;
  error_t error = NO_ERROR;

  while (1)
    {
      int chars_needed;
      va_start(vl, format);
      chars_needed = vsnprintf(&memwriter->buf[memwriter->size], memwriter->capacity - memwriter->size, format, vl);
      va_end(vl);
      if (chars_needed < 0)
        {
          return ERROR_INTERNAL;
        }
      /* we need one more char because `vsnprintf` does exclude the trailing '\0' character in its calculations */
      if ((size_t)chars_needed < (memwriter->capacity - memwriter->size))
        {
          memwriter->size += chars_needed;
          break;
        }
      if ((error = memwriter_ensure_buf(memwriter, chars_needed + 1)) != NO_ERROR)
        {
          break;
        }
    }

  return error;
}

error_t memwriter_puts(memwriter_t *memwriter, const char *s)
{
  return memwriter_printf(memwriter, "%s", s);
}

error_t memwriter_putc(memwriter_t *memwriter, char c)
{
  return memwriter_printf(memwriter, "%c", c);
}

char *memwriter_buf(const memwriter_t *memwriter)
{
  return memwriter->buf;
}

size_t memwriter_size(const memwriter_t *memwriter)
{
  return memwriter->size;
}


/* ------------------------- receiver ------------------------------------------------------------------------------- */

error_t receiver_init_for_custom(metahandle_t *handle, const char *name, unsigned int id,
                                 const char *(*custom_recv)(const char *, unsigned int))
{
  handle->sender_receiver.receiver.comm.custom.recv = custom_recv;
  handle->sender_receiver.receiver.comm.custom.name = name;
  handle->sender_receiver.receiver.comm.custom.id = id;
  handle->sender_receiver.receiver.message_size = 0;
  handle->sender_receiver.receiver.recv = receiver_recv_for_custom;
  handle->finalize = receiver_finalize_for_custom;
  handle->sender_receiver.receiver.memwriter = memwriter_new();
  if (handle->sender_receiver.receiver.memwriter == NULL)
    {
      return ERROR_MALLOC;
    }

  return NO_ERROR;
}

error_t receiver_init_for_socket(metahandle_t *handle, const char *hostname, unsigned int port)
{
  char port_str[PORT_MAX_STRING_LENGTH];
  struct addrinfo *addr_result = NULL, addr_hints;
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
  if ((wsa_startup_error = WSAStartup(MAKEWORD(2, 2), &wsa_data)))
    {
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
  if ((error = getaddrinfo(hostname, port_str, &addr_hints, &addr_result)) != 0)
    {
#ifdef _WIN32
      psocketerror("getaddrinfo failed with error");
#else
      if (error == EAI_SYSTEM)
        {
          perror("getaddrinfo failed with error");
        }
      else
        {
          fprintf(stderr, "getaddrinfo failed with error: %s\n", gai_strerror(error));
        }
#endif
      return ERROR_NETWORK_HOSTNAME_RESOLUTION;
    }

  /* Create a socket for listening */
  if ((handle->sender_receiver.receiver.comm.socket.server_socket =
           socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol)) < 0)
    {
      psocketerror("socket creation failed");
      freeaddrinfo(addr_result);
      return ERROR_NETWORK_SOCKET_CREATION;
    }
    /* Set SO_REUSEADDR if available on this system */
#ifdef SO_REUSEADDR
  socket_opt = 1;
  if (setsockopt(handle->sender_receiver.receiver.comm.socket.server_socket, SOL_SOCKET, SO_REUSEADDR,
                 (char *)&socket_opt, sizeof(socket_opt)) < 0)
    {
      psocketerror("setting socket options failed");
      freeaddrinfo(addr_result);
      return ERROR_NETWORK_SOCKET_CREATION;
    }
#endif

  /* Bind the socket to given ip address and port */
  if (bind(handle->sender_receiver.receiver.comm.socket.server_socket, addr_result->ai_addr, addr_result->ai_addrlen))
    {
      psocketerror("bind failed");
      freeaddrinfo(addr_result);
      return ERROR_NETWORK_SOCKET_BIND;
    }
  freeaddrinfo(addr_result);

  /* Listen for incoming connections */
  if (listen(handle->sender_receiver.receiver.comm.socket.server_socket, 1))
    {
      psocketerror("listen failed");
      return ERROR_NETWORK_SOCKET_LISTEN;
    }

  /* Accecpt an incoming connection and get a new socket instance for communication */
  if ((handle->sender_receiver.receiver.comm.socket.client_socket =
           accept(handle->sender_receiver.receiver.comm.socket.server_socket, (struct sockaddr *)&client_addr,
                  &client_addrlen)) < 0)
    {
      psocketerror("accept failed");
      return ERROR_NETWORK_CONNECTION_ACCEPT;
    }

  handle->sender_receiver.receiver.memwriter = memwriter_new();
  if (handle->sender_receiver.receiver.memwriter == NULL)
    {
      return ERROR_MALLOC;
    }

  return NO_ERROR;
}

error_t receiver_finalize_for_custom(metahandle_t *handle)
{
  memwriter_delete(handle->sender_receiver.receiver.memwriter);

  return NO_ERROR;
}

error_t receiver_finalize_for_socket(metahandle_t *handle)
{
  error_t error = NO_ERROR;

  memwriter_delete(handle->sender_receiver.receiver.memwriter);
#ifdef _WIN32
  if (handle->sender_receiver.receiver.comm.socket.client_socket >= 0)
    {
      if (closesocket(handle->sender_receiver.receiver.comm.socket.client_socket))
        {
          psocketerror("client socket shutdown failed");
          error = ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
  if (handle->sender_receiver.receiver.comm.socket.server_socket >= 0)
    {
      if (closesocket(handle->sender_receiver.receiver.comm.socket.server_socket))
        {
          psocketerror("server socket shutdown failed");
          error = ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
  if (WSACleanup())
    {
      psocketerror("winsock shutdown failed");
      error = ERROR_NETWORK_WINSOCK_CLEANUP;
    }
#else
  if (handle->sender_receiver.receiver.comm.socket.client_socket >= 0)
    {
      if (close(handle->sender_receiver.receiver.comm.socket.client_socket))
        {
          psocketerror("client socket shutdown failed");
          error = ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
  if (handle->sender_receiver.receiver.comm.socket.server_socket >= 0)
    {
      if (close(handle->sender_receiver.receiver.comm.socket.server_socket))
        {
          psocketerror("server socket shutdown failed");
          error = ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
#endif

  return error;
}

error_t receiver_recv_for_socket(metahandle_t *handle)
{
  int search_start_index = 0;
  char *end_ptr;
  static char recv_buf[SOCKET_RECV_BUF_SIZE];
  error_t error = NO_ERROR;

  while ((end_ptr = memchr(memwriter_buf(handle->sender_receiver.receiver.memwriter) + search_start_index, ETB,
                           memwriter_size(handle->sender_receiver.receiver.memwriter) - search_start_index)) == NULL)
    {
      int bytes_received;
      search_start_index = memwriter_size(handle->sender_receiver.receiver.memwriter);
      bytes_received =
          recv(handle->sender_receiver.receiver.comm.socket.client_socket, recv_buf, SOCKET_RECV_BUF_SIZE, 0);
      if (bytes_received < 0)
        {
          psocketerror("error while receiving data");
          return ERROR_NETWORK_RECV;
        }
      else if (bytes_received == 0)
        {
          return ERROR_NETWORK_RECV_CONNECTION_SHUTDOWN;
        }
      if ((error = memwriter_printf(handle->sender_receiver.receiver.memwriter, "%.*s", bytes_received, recv_buf)) !=
          NO_ERROR)
        {
          return error;
        }
    }
  *end_ptr = '\0';
  handle->sender_receiver.receiver.message_size = end_ptr - memwriter_buf(handle->sender_receiver.receiver.memwriter);

  return error;
}

error_t receiver_recv_for_custom(metahandle_t *handle)
{
  /* TODO: is it really necessary to copy the memory? */
  const char *recv_buf;
  error_t error = NO_ERROR;

  recv_buf = handle->sender_receiver.receiver.comm.custom.recv(handle->sender_receiver.receiver.comm.custom.name,
                                                               handle->sender_receiver.receiver.comm.custom.id);
  if (recv_buf == NULL)
    {
      return ERROR_CUSTOM_RECV;
    }
  memwriter_clear(handle->sender_receiver.receiver.memwriter);
  if ((error = memwriter_puts(handle->sender_receiver.receiver.memwriter, recv_buf)) != NO_ERROR)
    {
      return error;
    }
  handle->sender_receiver.receiver.message_size = memwriter_size(handle->sender_receiver.receiver.memwriter);

  return error;
}


/* ------------------------- sender --------------------------------------------------------------------------------- */

error_t sender_init_for_custom(metahandle_t *handle, const char *name, unsigned int id,
                               int (*custom_send)(const char *, unsigned int, const char *))
{
  handle->sender_receiver.sender.comm.custom.send = custom_send;
  handle->sender_receiver.sender.comm.custom.name = name;
  handle->sender_receiver.sender.comm.custom.id = id;
  handle->sender_receiver.sender.send = sender_send_for_custom;
  handle->finalize = sender_finalize_for_custom;
  handle->sender_receiver.sender.memwriter = memwriter_new();
  if (handle->sender_receiver.sender.memwriter == NULL)
    {
      return ERROR_MALLOC;
    }

  return NO_ERROR;
}

error_t sender_init_for_socket(metahandle_t *handle, const char *hostname, unsigned int port)
{
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
  if ((wsa_startup_error = WSAStartup(MAKEWORD(2, 2), &wsa_data)))
    {
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
  if ((error = getaddrinfo(hostname, port_str, &addr_hints, &addr_result)) != 0)
    {
#ifdef _WIN32
      psocketerror("getaddrinfo failed with error");
#else
      if (error == EAI_SYSTEM)
        {
          perror("getaddrinfo failed with error");
        }
      else
        {
          fprintf(stderr, "getaddrinfo failed with error: %s\n", gai_strerror(error));
        }
#endif
      return ERROR_NETWORK_HOSTNAME_RESOLUTION;
    }

  /* Attempt to connect to an address until one succeeds */
  handle->sender_receiver.sender.comm.socket.client_socket = -1;
  for (addr_ptr = addr_result; addr_ptr != NULL && handle->sender_receiver.sender.comm.socket.client_socket < 0;
       addr_ptr = addr_ptr->ai_next)
    {
      /* Create a socket for connecting to server */
      handle->sender_receiver.sender.comm.socket.client_socket =
          socket(addr_ptr->ai_family, addr_ptr->ai_socktype, addr_ptr->ai_protocol);
      if (handle->sender_receiver.sender.comm.socket.client_socket < 0)
        {
          psocketerror("socket creation failed");
          return ERROR_NETWORK_SOCKET_CREATION;
        }
      /* Connect to server */
      if (connect(handle->sender_receiver.sender.comm.socket.client_socket, addr_ptr->ai_addr,
                  (int)addr_ptr->ai_addrlen))
        {
#ifdef _WIN32
          closesocket(handle->sender_receiver.sender.comm.socket.client_socket);
#else
          close(handle->sender_receiver.sender.comm.socket.client_socket);
#endif
          handle->sender_receiver.sender.comm.socket.client_socket = -1;
        }
    }
  freeaddrinfo(addr_result);

  if (handle->sender_receiver.sender.comm.socket.client_socket < 0)
    {
      fprintf(stderr, "cannot connect to host %s port %u: ", hostname, port);
      psocketerror("");
      return ERROR_NETWORK_CONNECT;
    }

  handle->sender_receiver.sender.memwriter = memwriter_new();
  if (handle->sender_receiver.sender.memwriter == NULL)
    {
      return ERROR_MALLOC;
    }

  return NO_ERROR;
}

error_t sender_finalize_for_custom(metahandle_t *handle)
{
  memwriter_delete(handle->sender_receiver.sender.memwriter);

  return NO_ERROR;
}

error_t sender_finalize_for_socket(metahandle_t *handle)
{
  error_t error = NO_ERROR;

  memwriter_delete(handle->sender_receiver.sender.memwriter);
#ifdef _WIN32
  if (handle->sender_receiver.sender.comm.socket.client_socket >= 0)
    {
      if (closesocket(handle->sender_receiver.sender.comm.socket.client_socket))
        {
          psocketerror("client socket shutdown failed");
          error = ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
  if (WSACleanup())
    {
      psocketerror("winsock shutdown failed");
      error = ERROR_NETWORK_WINSOCK_CLEANUP;
    }
#else
  if (handle->sender_receiver.sender.comm.socket.client_socket >= 0)
    {
      if (close(handle->sender_receiver.sender.comm.socket.client_socket))
        {
          psocketerror("client socket shutdown failed");
          error = ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
#endif

  return error;
}

error_t sender_send_for_socket(metahandle_t *handle)
{
  const char *buf, *send_ptr;
  size_t buf_size;
  int bytes_left;
  error_t error = NO_ERROR;

  if ((error = memwriter_putc(handle->sender_receiver.sender.memwriter, ETB)) != NO_ERROR)
    {
      return error;
    }

  buf = memwriter_buf(handle->sender_receiver.sender.memwriter);
  buf_size = memwriter_size(handle->sender_receiver.sender.memwriter);

  send_ptr = buf;
  bytes_left = buf_size;
  while (bytes_left)
    {
      int bytes_sent = send(handle->sender_receiver.sender.comm.socket.client_socket, buf, bytes_left, 0);
      if (bytes_sent < 0)
        {
          psocketerror("could not send any data");
          return ERROR_NETWORK_SEND;
        }
      send_ptr += bytes_sent;
      bytes_left -= bytes_sent;
    }

  memwriter_clear(handle->sender_receiver.sender.memwriter);

  return error;
}

error_t sender_send_for_custom(metahandle_t *handle)
{
  const char *buf;
  error_t error = NO_ERROR;

  buf = memwriter_buf(handle->sender_receiver.sender.memwriter);
  if (!handle->sender_receiver.sender.comm.custom.send(handle->sender_receiver.sender.comm.custom.name,
                                                       handle->sender_receiver.sender.comm.custom.id, buf))
    {
      error = ERROR_CUSTOM_SEND;
      return error;
    }
  memwriter_clear(handle->sender_receiver.sender.memwriter);

  return error;
}

#ifndef NDEBUG
void gr_dumpmeta(const gr_meta_args_t *args, FILE *f)
{
#define BUFFER_LEN 200
#define DEFAULT_ARRAY_PRINT_ELEMENTS_COUNT 10
#define DARK_BACKGROUND_ENV_KEY "GR_META_DARK_BACKGROUND"
#define ARRAY_PRINT_TRUNCATION_ENV_KEY "GR_META_ARRAY_PRINT_TRUNCATION"
  args_iterator_t *it;
  args_value_iterator_t *value_it;
  arg_t *arg;
  unsigned int i;
  char buffer[BUFFER_LEN];
  int count_characters;
  static int recursion_level = -1;
  int columns, cursor_xpos = 0;
  int use_color_codes;
  int has_dark_bg;
  struct gr_dumpmeta_color_codes_t
  {
    unsigned char k, i, d, c, s;
  } color_codes;
  unsigned int array_print_elements_count = DEFAULT_ARRAY_PRINT_ELEMENTS_COUNT;
#if defined(_WIN32) || defined(__EMSCRIPTEN__)
  columns = INT_MAX;
  use_color_codes = 0;
  has_dark_bg = 0;
#else
  struct winsize w;
  use_color_codes = isatty(fileno(f));
  ioctl(0, TIOCGWINSZ, &w);
  columns = w.ws_col;
  if (getenv(DARK_BACKGROUND_ENV_KEY) != NULL &&
      str_equals_any(getenv(DARK_BACKGROUND_ENV_KEY), 5, "1", "yes", "YES", "on", "ON"))
    {
      has_dark_bg = 1;
      color_codes.k = 122;
      color_codes.i = 81;
      color_codes.d = 215;
      color_codes.c = 228;
      color_codes.s = 155;
    }
  else
    {
      has_dark_bg = 0;
      color_codes.k = 18;
      color_codes.i = 25;
      color_codes.d = 88;
      color_codes.c = 55;
      color_codes.s = 22;
    }
#endif
  if (getenv(ARRAY_PRINT_TRUNCATION_ENV_KEY) != NULL)
    {
      if (str_equals_any(getenv(ARRAY_PRINT_TRUNCATION_ENV_KEY), 8, "", "0", "inf", "INF", "unlimited", "UNLIMITED",
                         "off", "OFF"))
        {
          array_print_elements_count = UINT_MAX;
        }
      else
        {
          str_to_uint(getenv(ARRAY_PRINT_TRUNCATION_ENV_KEY), &array_print_elements_count);
        }
    }

#define INDENT 2

#define print_indent                                                                                                   \
  do                                                                                                                   \
    {                                                                                                                  \
      if (use_color_codes)                                                                                             \
        {                                                                                                              \
          int i;                                                                                                       \
          for (i = 0; i < recursion_level; ++i)                                                                        \
            {                                                                                                          \
              fprintf(f, "\033[48;5;%dm%*s\033[0m", has_dark_bg ? (235 + (5 * i) % 25) : (255 - (5 * i) % 25), INDENT, \
                      "");                                                                                             \
            }                                                                                                          \
        }                                                                                                              \
      else                                                                                                             \
        {                                                                                                              \
          fprintf(f, "%*s", INDENT *recursion_level, "");                                                              \
        }                                                                                                              \
    }                                                                                                                  \
  while (0)

#define print_key                                                          \
  do                                                                       \
    {                                                                      \
      print_indent;                                                        \
      if (use_color_codes)                                                 \
        {                                                                  \
          fprintf(f, "\033[38;5;%dm%s\033[0m: ", color_codes.k, arg->key); \
        }                                                                  \
      else                                                                 \
        {                                                                  \
          fprintf(f, "%s: ", arg->key);                                    \
        }                                                                  \
    }                                                                      \
  while (0)

#define print_value(value_type, format_string, color_code)                                                       \
  do                                                                                                             \
    {                                                                                                            \
      if (use_color_codes)                                                                                       \
        {                                                                                                        \
          fprintf(f, "\033[38;5;%dm" format_string "\033[0m", color_code, *((value_type *)value_it->value_ptr)); \
        }                                                                                                        \
      else                                                                                                       \
        {                                                                                                        \
          fprintf(f, format_string, *((value_type *)value_it->value_ptr));                                       \
        }                                                                                                        \
    }                                                                                                            \
  while (0)

#define print_values(value_type, format_string, color_code)                                                        \
  do                                                                                                               \
    {                                                                                                              \
      int print_last_element = 0;                                                                                  \
      fputc('[', f);                                                                                               \
      cursor_xpos += strlen(arg->key) + 3;                                                                         \
      for (i = 0; i < min(value_it->array_length, array_print_elements_count); i++)                                \
        {                                                                                                          \
          if (print_last_element)                                                                                  \
            {                                                                                                      \
              i = value_it->array_length - 1;                                                                      \
            }                                                                                                      \
          if (array_print_elements_count >= value_it->array_length || i != array_print_elements_count - 2)         \
            {                                                                                                      \
              if (use_color_codes)                                                                                 \
                {                                                                                                  \
                  count_characters =                                                                               \
                      snprintf(buffer, BUFFER_LEN,                                                                 \
                               "\033[38;5;%dm" format_string "\033[0m"                                             \
                               "%s",                                                                               \
                               color_code, (*((value_type **)value_it->value_ptr))[i],                             \
                               (i < min(value_it->array_length, array_print_elements_count) - 1) ? ", " : "]");    \
                  count_characters -= (color_code >= 100) ? 15 : ((color_code >= 10) ? 14 : 13);                   \
                }                                                                                                  \
              else                                                                                                 \
                {                                                                                                  \
                  count_characters =                                                                               \
                      snprintf(buffer, BUFFER_LEN, format_string "%s", (*((value_type **)value_it->value_ptr))[i], \
                               (i < min(value_it->array_length, array_print_elements_count) - 1) ? ", " : "]");    \
                }                                                                                                  \
            }                                                                                                      \
          else                                                                                                     \
            {                                                                                                      \
              count_characters = snprintf(buffer, BUFFER_LEN, "..., ");                                            \
              print_last_element = 1;                                                                              \
            }                                                                                                      \
          if (cursor_xpos + count_characters > columns)                                                            \
            {                                                                                                      \
              fputc('\n', f);                                                                                      \
              print_indent;                                                                                        \
              cursor_xpos = INDENT * recursion_level + fprintf(f, "%*s", (int)strlen(arg->key) + 3, "") - 1;       \
            }                                                                                                      \
          fputs(buffer, f);                                                                                        \
          cursor_xpos += count_characters;                                                                         \
        }                                                                                                          \
      if (value_it->array_length == 0)                                                                             \
        {                                                                                                          \
          fputc(']', f);                                                                                           \
        }                                                                                                          \
    }                                                                                                              \
  while (0)

#define print_type(value_type, format_string, color_code)      \
  do                                                           \
    {                                                          \
      if (value_it->is_array)                                  \
        {                                                      \
          print_key;                                           \
          print_values(value_type, format_string, color_code); \
        }                                                      \
      else                                                     \
        {                                                      \
          print_key;                                           \
          print_value(value_type, format_string, color_code);  \
        }                                                      \
      fputc('\n', f);                                          \
    }                                                          \
  while (0)

  ++recursion_level;

  it = args_iter(args);
  while ((arg = it->next(it)) != NULL)
    {
      if (*arg->value_format)
        {
          value_it = arg_value_iter(arg);
          while (value_it->next(value_it) != NULL)
            {
              cursor_xpos = INDENT * recursion_level;
              switch (value_it->format)
                {
                case 'i':
                  print_type(int, "% d", color_codes.i);
                  break;
                case 'd':
                  print_type(double, "% lf", color_codes.d);
                  break;
                case 'c':
                  print_type(char, "'%c'", color_codes.c);
                  break;
                case 's':
                  print_type(char *, "\"%s\"", color_codes.s);
                  break;
                case 'a':
                  if (value_it->is_array)
                    {
                      print_key;
                      fprintf(f, "[\n");
                      for (i = 0; i < value_it->array_length; i++)
                        {
                          gr_dumpmeta((*((gr_meta_args_t ***)value_it->value_ptr))[i], f);
                          if (i < value_it->array_length - 1)
                            {
                              print_indent;
                              fprintf(f, ",\n");
                            }
                        }
                      print_indent;
                      fprintf(f, "]\n");
                    }
                  else
                    {
                      print_key;
                      fprintf(f, "\n");
                      gr_dumpmeta(*((gr_meta_args_t **)value_it->value_ptr), f);
                      fprintf(f, "\n");
                    }
                  break;
                default:
                  break;
                }
            }
          args_value_iterator_delete(value_it);
        }
      else
        {
          print_key;
          fprintf(f, "(none)\n");
        }
    }
  args_iterator_delete(it);

  --recursion_level;

#undef BUFFER_LEN
#undef DEFAULT_ARRAY_PRINT_ELEMENTS_COUNT
#undef DARK_BACKGROUND_ENV_KEY
#undef ARRAY_PRINT_TRUNCATION_ENV_KEY
#undef INDENT
#undef print_indent
#undef print_key
#undef print_type
}

void gr_dumpmeta_json(const gr_meta_args_t *args, FILE *f)
{
  static memwriter_t *memwriter = NULL;

  if (memwriter == NULL)
    {
      memwriter = memwriter_new();
    }
  tojson_write_args(memwriter, args);
  if (tojson_is_complete())
    {
      memwriter_putc(memwriter, '\0');
      fprintf(f, "%s\n", memwriter_buf(memwriter));
      memwriter_delete(memwriter);
      memwriter = NULL;
    }
}

char *gr_dumpmeta_json_str(void)
{
  static memwriter_t *memwriter = NULL;
  char *result;

  if (memwriter == NULL)
    {
      memwriter = memwriter_new();
    }
  /* tojson_write_args(memwriter, global_root_args); */
  tojson_write_args(memwriter, active_plot_args);
  if (tojson_is_complete())
    {
      memwriter_putc(memwriter, '\0');
      result = malloc(strlen(memwriter_buf(memwriter)) * sizeof(char));
      strcpy(result, memwriter_buf(memwriter));
      memwriter_delete(memwriter);
      memwriter = NULL;
      return result;
    }
  return "";
}

#ifdef EMSCRIPTEN
FILE *gr_get_stdout()
{
  return stdout;
}
#endif
#endif


/* ------------------------- generic list --------------------------------------------------------------------------- */

#define DEFINE_LIST_METHODS(prefix)                                                                                    \
  prefix##_list_t *prefix##_list_new(void)                                                                             \
  {                                                                                                                    \
    static const prefix##_list_vtable_t vt = {                                                                         \
        prefix##_list_entry_copy,                                                                                      \
        prefix##_list_entry_delete,                                                                                    \
    };                                                                                                                 \
    prefix##_list_t *list;                                                                                             \
                                                                                                                       \
    list = malloc(sizeof(prefix##_list_t));                                                                            \
    if (list == NULL)                                                                                                  \
      {                                                                                                                \
        return NULL;                                                                                                   \
      }                                                                                                                \
    list->vt = &vt;                                                                                                    \
    list->head = NULL;                                                                                                 \
    list->tail = NULL;                                                                                                 \
    list->size = 0;                                                                                                    \
                                                                                                                       \
    return list;                                                                                                       \
  }                                                                                                                    \
                                                                                                                       \
  void prefix##_list_delete(prefix##_list_t *list)                                                                     \
  {                                                                                                                    \
    prefix##_list_node_t *current_list_node;                                                                           \
    prefix##_list_node_t *next_list_node;                                                                              \
                                                                                                                       \
    current_list_node = list->head;                                                                                    \
    while (current_list_node != NULL)                                                                                  \
      {                                                                                                                \
        next_list_node = current_list_node->next;                                                                      \
        list->vt->entry_delete(current_list_node->entry);                                                              \
        free(current_list_node);                                                                                       \
        current_list_node = next_list_node;                                                                            \
      }                                                                                                                \
    free(list);                                                                                                        \
  }                                                                                                                    \
                                                                                                                       \
  error_t prefix##_list_push_front(prefix##_list_t *list, prefix##_list_const_entry_t entry)                           \
  {                                                                                                                    \
    prefix##_list_node_t *new_list_node;                                                                               \
    error_t error = NO_ERROR;                                                                                          \
                                                                                                                       \
    new_list_node = malloc(sizeof(prefix##_list_node_t));                                                              \
    error_cleanup_and_set_error_if(new_list_node == NULL, ERROR_MALLOC);                                               \
    error = list->vt->entry_copy(&new_list_node->entry, entry);                                                        \
    error_cleanup_if_error;                                                                                            \
    new_list_node->next = list->head;                                                                                  \
    list->head = new_list_node;                                                                                        \
    if (list->tail == NULL)                                                                                            \
      {                                                                                                                \
        list->tail = new_list_node;                                                                                    \
      }                                                                                                                \
    ++(list->size);                                                                                                    \
                                                                                                                       \
    return NO_ERROR;                                                                                                   \
                                                                                                                       \
  error_cleanup:                                                                                                       \
    free(new_list_node);                                                                                               \
    return error;                                                                                                      \
  }                                                                                                                    \
                                                                                                                       \
  error_t prefix##_list_push_back(prefix##_list_t *list, prefix##_list_const_entry_t entry)                            \
  {                                                                                                                    \
    prefix##_list_node_t *new_list_node;                                                                               \
    error_t error = NO_ERROR;                                                                                          \
                                                                                                                       \
    new_list_node = malloc(sizeof(prefix##_list_node_t));                                                              \
    error_cleanup_and_set_error_if(new_list_node == NULL, ERROR_MALLOC);                                               \
    error = list->vt->entry_copy(&new_list_node->entry, entry);                                                        \
    error_cleanup_if_error;                                                                                            \
    new_list_node->next = NULL;                                                                                        \
    if (list->head == NULL)                                                                                            \
      {                                                                                                                \
        list->head = new_list_node;                                                                                    \
      }                                                                                                                \
    else                                                                                                               \
      {                                                                                                                \
        list->tail->next = new_list_node;                                                                              \
      }                                                                                                                \
    list->tail = new_list_node;                                                                                        \
    ++(list->size);                                                                                                    \
                                                                                                                       \
    return NO_ERROR;                                                                                                   \
                                                                                                                       \
  error_cleanup:                                                                                                       \
    free(new_list_node);                                                                                               \
    return error;                                                                                                      \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_list_entry_t prefix##_list_pop_front(prefix##_list_t *list)                                                 \
  {                                                                                                                    \
    prefix##_list_node_t *front_node;                                                                                  \
    prefix##_list_entry_t front_entry;                                                                                 \
                                                                                                                       \
    assert(list->head != NULL);                                                                                        \
    front_node = list->head;                                                                                           \
    list->head = list->head->next;                                                                                     \
    if (list->tail == front_node)                                                                                      \
      {                                                                                                                \
        list->tail = NULL;                                                                                             \
      }                                                                                                                \
    front_entry = front_node->entry;                                                                                   \
    free(front_node);                                                                                                  \
    --(list->size);                                                                                                    \
                                                                                                                       \
    return front_entry;                                                                                                \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_list_entry_t prefix##_list_pop_back(prefix##_list_t *list)                                                  \
  {                                                                                                                    \
    prefix##_list_node_t *last_node;                                                                                   \
    prefix##_list_node_t *next_to_last_node = NULL;                                                                    \
    prefix##_list_entry_t last_entry;                                                                                  \
                                                                                                                       \
    assert(list->tail != NULL);                                                                                        \
    last_node = list->tail;                                                                                            \
    prefix##_list_find_previous_node(list, last_node, &next_to_last_node);                                             \
    if (next_to_last_node == NULL)                                                                                     \
      {                                                                                                                \
        list->head = list->tail = NULL;                                                                                \
      }                                                                                                                \
    else                                                                                                               \
      {                                                                                                                \
        list->tail = next_to_last_node;                                                                                \
        next_to_last_node->next = NULL;                                                                                \
      }                                                                                                                \
    last_entry = last_node->entry;                                                                                     \
    free(last_node);                                                                                                   \
    --(list->size);                                                                                                    \
                                                                                                                       \
    return last_entry;                                                                                                 \
  }                                                                                                                    \
                                                                                                                       \
  error_t prefix##_list_push(prefix##_list_t *list, prefix##_list_const_entry_t entry)                                 \
  {                                                                                                                    \
    return prefix##_list_push_front(list, entry);                                                                      \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_list_entry_t prefix##_list_pop(prefix##_list_t *list) { return prefix##_list_pop_front(list); }             \
                                                                                                                       \
  error_t prefix##_list_enqueue(prefix##_list_t *list, prefix##_list_const_entry_t entry)                              \
  {                                                                                                                    \
    return prefix##_list_push_back(list, entry);                                                                       \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_list_entry_t prefix##_list_dequeue(prefix##_list_t *list) { return prefix##_list_pop_front(list); }         \
                                                                                                                       \
  int prefix##_list_empty(prefix##_list_t *list) { return list->size == 0; }                                           \
                                                                                                                       \
  int prefix##_list_find_previous_node(const prefix##_list_t *list, const prefix##_list_node_t *node,                  \
                                       prefix##_list_node_t **previous_node)                                           \
  {                                                                                                                    \
    prefix##_list_node_t *prev_node;                                                                                   \
    prefix##_list_node_t *current_node;                                                                                \
                                                                                                                       \
    prev_node = NULL;                                                                                                  \
    current_node = list->head;                                                                                         \
    while (current_node != NULL)                                                                                       \
      {                                                                                                                \
        if (current_node == node)                                                                                      \
          {                                                                                                            \
            if (previous_node != NULL)                                                                                 \
              {                                                                                                        \
                *previous_node = prev_node;                                                                            \
              }                                                                                                        \
            return 1;                                                                                                  \
          }                                                                                                            \
        prev_node = current_node;                                                                                      \
        current_node = current_node->next;                                                                             \
      }                                                                                                                \
                                                                                                                       \
    return 0;                                                                                                          \
  }                                                                                                                    \
                                                                                                                       \
  /* ~~~~~~~~~~~~~~~~~~~~~~~ ref list ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */ \
                                                                                                                       \
  prefix##_reflist_t *prefix##_reflist_new(void)                                                                       \
  {                                                                                                                    \
    static const prefix##_reflist_vtable_t vt = {                                                                      \
        prefix##_reflist_entry_copy,                                                                                   \
        prefix##_reflist_entry_delete,                                                                                 \
    };                                                                                                                 \
    prefix##_reflist_t *list;                                                                                          \
                                                                                                                       \
    list = (prefix##_reflist_t *)prefix##_list_new();                                                                  \
    list->vt = &vt;                                                                                                    \
                                                                                                                       \
    return list;                                                                                                       \
  }                                                                                                                    \
                                                                                                                       \
  void prefix##_reflist_delete(prefix##_reflist_t *list) { prefix##_list_delete((prefix##_list_t *)list); }            \
                                                                                                                       \
  void prefix##_reflist_delete_with_entries(prefix##_reflist_t *list)                                                  \
  {                                                                                                                    \
    prefix##_reflist_node_t *current_reflist_node;                                                                     \
    prefix##_reflist_node_t *next_reflist_node;                                                                        \
                                                                                                                       \
    current_reflist_node = list->head;                                                                                 \
    while (current_reflist_node != NULL)                                                                               \
      {                                                                                                                \
        next_reflist_node = current_reflist_node->next;                                                                \
        prefix##_list_entry_delete((prefix##_list_entry_t)current_reflist_node->entry);                                \
        free(current_reflist_node);                                                                                    \
        current_reflist_node = next_reflist_node;                                                                      \
      }                                                                                                                \
    free(list);                                                                                                        \
  }                                                                                                                    \
                                                                                                                       \
  error_t prefix##_reflist_push_front(prefix##_reflist_t *list, prefix##_reflist_entry_t entry)                        \
  {                                                                                                                    \
    return prefix##_list_push_front((prefix##_list_t *)list, (prefix##_list_entry_t)entry);                            \
  }                                                                                                                    \
                                                                                                                       \
  error_t prefix##_reflist_push_back(prefix##_reflist_t *list, prefix##_reflist_entry_t entry)                         \
  {                                                                                                                    \
    return prefix##_list_push_back((prefix##_list_t *)list, (prefix##_list_entry_t)entry);                             \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_reflist_entry_t prefix##_reflist_pop_front(prefix##_reflist_t *list)                                        \
  {                                                                                                                    \
    return prefix##_list_pop_front((prefix##_list_t *)list);                                                           \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_reflist_entry_t prefix##_reflist_pop_back(prefix##_reflist_t *list)                                         \
  {                                                                                                                    \
    return prefix##_list_pop_back((prefix##_list_t *)list);                                                            \
  }                                                                                                                    \
                                                                                                                       \
  error_t prefix##_reflist_push(prefix##_reflist_t *list, prefix##_reflist_entry_t entry)                              \
  {                                                                                                                    \
    return prefix##_list_push((prefix##_list_t *)list, (prefix##_list_entry_t)entry);                                  \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_reflist_entry_t prefix##_reflist_pop(prefix##_reflist_t *list)                                              \
  {                                                                                                                    \
    return prefix##_list_pop((prefix##_list_t *)list);                                                                 \
  }                                                                                                                    \
                                                                                                                       \
  error_t prefix##_reflist_enqueue(prefix##_reflist_t *list, prefix##_reflist_entry_t entry)                           \
  {                                                                                                                    \
    return prefix##_list_enqueue((prefix##_list_t *)list, (prefix##_list_entry_t)entry);                               \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_reflist_entry_t prefix##_reflist_dequeue(prefix##_reflist_t *list)                                          \
  {                                                                                                                    \
    return prefix##_list_dequeue((prefix##_list_t *)list);                                                             \
  }                                                                                                                    \
                                                                                                                       \
  int prefix##_reflist_empty(prefix##_reflist_t *list) { return prefix##_list_empty((prefix##_list_t *)list); }        \
                                                                                                                       \
                                                                                                                       \
  int prefix##_reflist_find_previous_node(const prefix##_reflist_t *list, const prefix##_reflist_node_t *node,         \
                                          prefix##_reflist_node_t **previous_node)                                     \
  {                                                                                                                    \
    return prefix##_list_find_previous_node((prefix##_list_t *)list, (prefix##_list_node_t *)node,                     \
                                            (prefix##_list_node_t **)previous_node);                                   \
  }                                                                                                                    \
                                                                                                                       \
  error_t prefix##_reflist_entry_copy(prefix##_reflist_entry_t *copy, prefix##_reflist_entry_t entry)                  \
  {                                                                                                                    \
    *copy = entry;                                                                                                     \
    return NO_ERROR;                                                                                                   \
  }                                                                                                                    \
                                                                                                                       \
  error_t prefix##_reflist_entry_delete(prefix##_reflist_entry_t entry UNUSED) { return NO_ERROR; }

DEFINE_LIST_METHODS(args)

error_t args_list_entry_copy(args_list_entry_t *copy, args_list_const_entry_t entry)
{
  args_list_entry_t _copy;

  _copy = args_copy(entry, NULL, NULL);
  if (_copy == NULL)
    {
      return ERROR_MALLOC;
    }
  *copy = _copy;

  return NO_ERROR;
}

error_t args_list_entry_delete(args_list_entry_t entry)
{
  gr_deletemeta(entry);
  return NO_ERROR;
}

DEFINE_LIST_METHODS(dynamic_args_array)

error_t dynamic_args_array_list_entry_copy(dynamic_args_array_list_entry_t *copy,
                                           dynamic_args_array_list_const_entry_t entry)
{
  /* TODO: create a copy of the object! Otherwise code will segfault on list deletion for a non-ref list */
  *copy = (dynamic_args_array_list_entry_t)entry;
  return NO_ERROR;
}

error_t dynamic_args_array_list_entry_delete(dynamic_args_array_list_entry_t entry)
{
  dynamic_args_array_delete(entry);
  return NO_ERROR;
}

DEFINE_LIST_METHODS(string)

error_t string_list_entry_copy(string_list_entry_t *copy, const string_list_const_entry_t entry)
{
  string_list_entry_t _copy;

  _copy = strdup(entry);
  if (_copy == NULL)
    {
      return ERROR_MALLOC;
    }
  *copy = _copy;

  return NO_ERROR;
}

error_t string_list_entry_delete(string_list_entry_t entry)
{
  free(entry);
  return NO_ERROR;
}


/* ------------------------- generic set ---------------------------------------------------------------------------- */

#define DEFINE_SET_METHODS(prefix)                                                                                \
  prefix##_set_t *prefix##_set_new(size_t capacity)                                                               \
  {                                                                                                               \
    prefix##_set_t *set = NULL;                                                                                   \
    size_t power2_capacity = 1;                                                                                   \
                                                                                                                  \
    /* Use the power of 2 which is equal or greater than 2*capacity as the set capacity */                        \
    power2_capacity = next_or_equal_power2(2 * capacity);                                                         \
    set = malloc(sizeof(prefix##_set_t));                                                                         \
    if (set == NULL)                                                                                              \
      {                                                                                                           \
        debug_print_malloc_error();                                                                               \
        goto error_cleanup;                                                                                       \
      }                                                                                                           \
    set->set = NULL;                                                                                              \
    set->used = NULL;                                                                                             \
    set->set = malloc(power2_capacity * sizeof(prefix##_set_entry_t));                                            \
    if (set->set == NULL)                                                                                         \
      {                                                                                                           \
        debug_print_malloc_error();                                                                               \
        goto error_cleanup;                                                                                       \
      }                                                                                                           \
    set->used = calloc(power2_capacity, sizeof(unsigned char));                                                   \
    if (set->used == NULL)                                                                                        \
      {                                                                                                           \
        debug_print_malloc_error();                                                                               \
        goto error_cleanup;                                                                                       \
      }                                                                                                           \
    set->capacity = power2_capacity;                                                                              \
    set->size = 0;                                                                                                \
                                                                                                                  \
    logger((stderr, "Created a new set with capacity: %lu\n", set->capacity));                                    \
                                                                                                                  \
    return set;                                                                                                   \
                                                                                                                  \
  error_cleanup:                                                                                                  \
    if (set != NULL)                                                                                              \
      {                                                                                                           \
        if (set->set != NULL)                                                                                     \
          {                                                                                                       \
            free(set->set);                                                                                       \
          }                                                                                                       \
        if (set->used != NULL)                                                                                    \
          {                                                                                                       \
            free(set->used);                                                                                      \
          }                                                                                                       \
        free(set);                                                                                                \
      }                                                                                                           \
                                                                                                                  \
    return NULL;                                                                                                  \
  }                                                                                                               \
                                                                                                                  \
  prefix##_set_t *prefix##_set_new_with_data(size_t count, prefix##_set_entry_t *entries)                         \
  {                                                                                                               \
    prefix##_set_t *set;                                                                                          \
    size_t i;                                                                                                     \
                                                                                                                  \
    set = prefix##_set_new(count);                                                                                \
    if (set == NULL)                                                                                              \
      {                                                                                                           \
        return NULL;                                                                                              \
      }                                                                                                           \
    for (i = 0; i < count; ++i)                                                                                   \
      {                                                                                                           \
        if (!prefix##_set_add(set, entries[i]))                                                                   \
          {                                                                                                       \
            prefix##_set_delete(set);                                                                             \
            return NULL;                                                                                          \
          }                                                                                                       \
      }                                                                                                           \
                                                                                                                  \
    return set;                                                                                                   \
  }                                                                                                               \
                                                                                                                  \
  prefix##_set_t *prefix##_set_copy(const prefix##_set_t *set)                                                    \
  {                                                                                                               \
    prefix##_set_t *copy;                                                                                         \
    size_t i;                                                                                                     \
                                                                                                                  \
    copy = prefix##_set_new(set->size);                                                                           \
    if (copy == NULL)                                                                                             \
      {                                                                                                           \
        return NULL;                                                                                              \
      }                                                                                                           \
    for (i = 0; i < set->capacity; ++i)                                                                           \
      {                                                                                                           \
        if (set->used[i] && !prefix##_set_add(copy, set->set[i]))                                                 \
          {                                                                                                       \
            prefix##_set_delete(copy);                                                                            \
            return NULL;                                                                                          \
          }                                                                                                       \
      }                                                                                                           \
                                                                                                                  \
    return copy;                                                                                                  \
  }                                                                                                               \
                                                                                                                  \
  void prefix##_set_delete(prefix##_set_t *set)                                                                   \
  {                                                                                                               \
    size_t i;                                                                                                     \
    for (i = 0; i < set->capacity; ++i)                                                                           \
      {                                                                                                           \
        if (set->used[i])                                                                                         \
          {                                                                                                       \
            prefix##_set_entry_delete(set->set[i]);                                                               \
          }                                                                                                       \
      }                                                                                                           \
    free(set->set);                                                                                               \
    free(set->used);                                                                                              \
    free(set);                                                                                                    \
  }                                                                                                               \
                                                                                                                  \
  int prefix##_set_add(prefix##_set_t *set, prefix##_set_const_entry_t entry)                                     \
  {                                                                                                               \
    ssize_t index;                                                                                                \
                                                                                                                  \
    index = prefix##_set_index(set, entry);                                                                       \
    if (index < 0)                                                                                                \
      {                                                                                                           \
        return 0;                                                                                                 \
      }                                                                                                           \
    if (set->used[index])                                                                                         \
      {                                                                                                           \
        prefix##_set_entry_delete(set->set[index]);                                                               \
        --(set->size);                                                                                            \
        set->used[index] = 0;                                                                                     \
      }                                                                                                           \
    if (!prefix##_set_entry_copy(set->set + index, entry))                                                        \
      {                                                                                                           \
        return 0;                                                                                                 \
      }                                                                                                           \
    ++(set->size);                                                                                                \
    set->used[index] = 1;                                                                                         \
                                                                                                                  \
    return 1;                                                                                                     \
  }                                                                                                               \
                                                                                                                  \
  int prefix##_set_find(const prefix##_set_t *set, prefix##_set_const_entry_t entry,                              \
                        prefix##_set_entry_t *saved_entry)                                                        \
  {                                                                                                               \
    ssize_t index;                                                                                                \
                                                                                                                  \
    index = prefix##_set_index(set, entry);                                                                       \
    if (index < 0 || !set->used[index])                                                                           \
      {                                                                                                           \
        return 0;                                                                                                 \
      }                                                                                                           \
    *saved_entry = set->set[index];                                                                               \
    return 1;                                                                                                     \
  }                                                                                                               \
                                                                                                                  \
  int prefix##_set_contains(const prefix##_set_t *set, prefix##_set_const_entry_t entry)                          \
  {                                                                                                               \
    ssize_t index;                                                                                                \
                                                                                                                  \
    index = prefix##_set_index(set, entry);                                                                       \
    return index >= 0 && set->used[index];                                                                        \
  }                                                                                                               \
                                                                                                                  \
  ssize_t prefix##_set_index(const prefix##_set_t *set, prefix##_set_const_entry_t entry)                         \
  {                                                                                                               \
    size_t hash;                                                                                                  \
    size_t i;                                                                                                     \
                                                                                                                  \
    hash = prefix##_set_entry_hash(entry);                                                                        \
    for (i = 0; i < set->capacity; ++i)                                                                           \
      {                                                                                                           \
        /* Quadratic probing that will visit every slot in the hash table if the capacity is a power of 2, see: \ \
         * http://research.cs.vt.edu/AVresearch/hashing/quadratic.php */                                          \
        size_t next_index = (hash + (i * i + i) / 2) % set->capacity;                                             \
        if (!set->used[next_index] || prefix##_set_entry_equals(set->set[next_index], entry))                     \
          {                                                                                                       \
            return next_index;                                                                                    \
          }                                                                                                       \
      }                                                                                                           \
    return -1;                                                                                                    \
  }

DEFINE_SET_METHODS(args)

int args_set_entry_copy(args_set_entry_t *copy, args_set_const_entry_t entry)
{
  /* discard const because it is necessary to work on the object itself */
  /* TODO create two set types: copy and pointer version */
  *copy = (args_set_entry_t)entry;
  return 1;
}

void args_set_entry_delete(args_set_entry_t entry UNUSED) {}

size_t args_set_entry_hash(args_set_const_entry_t entry)
{
  return (size_t)entry;
}

int args_set_entry_equals(args_set_const_entry_t entry1, args_set_const_entry_t entry2)
{
  return entry1 == entry2;
}

/* ------------------------- string-to-generic map ------------------------------------------------------------------ */

#define DEFINE_MAP_METHODS(prefix)                                                                                 \
  DEFINE_SET_METHODS(string_##prefix##_pair)                                                                       \
                                                                                                                   \
  prefix##_map_t *prefix##_map_new(size_t capacity)                                                                \
  {                                                                                                                \
    string_##prefix##_pair_set_t *string_##prefix##_pair_set;                                                      \
                                                                                                                   \
    string_##prefix##_pair_set = string_##prefix##_pair_set_new(capacity);                                         \
    if (string_##prefix##_pair_set == NULL)                                                                        \
      {                                                                                                            \
        debug_print_malloc_error();                                                                                \
        return NULL;                                                                                               \
      }                                                                                                            \
                                                                                                                   \
    return (prefix##_map_t *)string_##prefix##_pair_set;                                                           \
  }                                                                                                                \
                                                                                                                   \
  prefix##_map_t *prefix##_map_new_with_data(size_t count, prefix##_map_entry_t *entries)                          \
  {                                                                                                                \
    return (prefix##_map_t *)string_##prefix##_pair_set_new_with_data(count, entries);                             \
  }                                                                                                                \
                                                                                                                   \
  prefix##_map_t *prefix##_map_copy(const prefix##_map_t *map)                                                     \
  {                                                                                                                \
    string_##prefix##_pair_set_t *string_##prefix##_pair_set;                                                      \
                                                                                                                   \
    string_##prefix##_pair_set = string_##prefix##_pair_set_copy((string_##prefix##_pair_set_t *)map);             \
    if (string_##prefix##_pair_set == NULL)                                                                        \
      {                                                                                                            \
        debug_print_malloc_error();                                                                                \
        return NULL;                                                                                               \
      }                                                                                                            \
                                                                                                                   \
    return (prefix##_map_t *)string_##prefix##_pair_set;                                                           \
  }                                                                                                                \
                                                                                                                   \
  void prefix##_map_delete(prefix##_map_t *prefix##_map)                                                           \
  {                                                                                                                \
    string_##prefix##_pair_set_delete((string_##prefix##_pair_set_t *)prefix##_map);                               \
  }                                                                                                                \
                                                                                                                   \
  int prefix##_map_insert(prefix##_map_t *prefix##_map, const char *key, prefix##_map_const_value_t value)         \
  {                                                                                                                \
    string_##prefix##_pair_set_entry_t entry;                                                                      \
                                                                                                                   \
    entry.key = key;                                                                                               \
    /* in this case, it is ok to remove the const attribute since a copy is created in the set implementation */   \
    entry.value = (prefix##_map_value_t)value;                                                                     \
    return string_##prefix##_pair_set_add((string_##prefix##_pair_set_t *)prefix##_map, entry);                    \
  }                                                                                                                \
                                                                                                                   \
  int prefix##_map_insert_default(prefix##_map_t *prefix##_map, const char *key, prefix##_map_const_value_t value) \
  {                                                                                                                \
    string_##prefix##_pair_set_entry_t entry;                                                                      \
                                                                                                                   \
    entry.key = key;                                                                                               \
    /* in this case, it is ok to remove the const attribute since a copy is created in the set implementation */   \
    entry.value = (prefix##_map_value_t)value;                                                                     \
    if (!string_##prefix##_pair_set_contains((string_##prefix##_pair_set_t *)prefix##_map, entry))                 \
      {                                                                                                            \
        return string_##prefix##_pair_set_add((string_##prefix##_pair_set_t *)prefix##_map, entry);                \
      }                                                                                                            \
    return 0;                                                                                                      \
  }                                                                                                                \
                                                                                                                   \
  int prefix##_map_at(const prefix##_map_t *prefix##_map, const char *key, prefix##_map_value_t *value)            \
  {                                                                                                                \
    string_##prefix##_pair_set_entry_t entry, saved_entry;                                                         \
                                                                                                                   \
    entry.key = key;                                                                                               \
    if (string_##prefix##_pair_set_find((string_##prefix##_pair_set_t *)prefix##_map, entry, &saved_entry))        \
      {                                                                                                            \
        if (value != NULL)                                                                                         \
          {                                                                                                        \
            *value = saved_entry.value;                                                                            \
          }                                                                                                        \
        return 1;                                                                                                  \
      }                                                                                                            \
    else                                                                                                           \
      {                                                                                                            \
        return 0;                                                                                                  \
      }                                                                                                            \
  }                                                                                                                \
                                                                                                                   \
  int string_##prefix##_pair_set_entry_copy(string_##prefix##_pair_set_entry_t *copy,                              \
                                            const string_##prefix##_pair_set_entry_t entry)                        \
  {                                                                                                                \
    const char *key_copy;                                                                                          \
    prefix##_map_value_t value_copy;                                                                               \
                                                                                                                   \
    key_copy = gks_strdup(entry.key);                                                                              \
    if (key_copy == NULL)                                                                                          \
      {                                                                                                            \
        return 0;                                                                                                  \
      }                                                                                                            \
    if (!prefix##_map_value_copy(&value_copy, entry.value))                                                        \
      {                                                                                                            \
        free((char *)key_copy);                                                                                    \
        return 0;                                                                                                  \
      }                                                                                                            \
    copy->key = key_copy;                                                                                          \
    copy->value = value_copy;                                                                                      \
                                                                                                                   \
    return 1;                                                                                                      \
  }                                                                                                                \
                                                                                                                   \
  void string_##prefix##_pair_set_entry_delete(string_##prefix##_pair_set_entry_t entry)                           \
  {                                                                                                                \
    free((char *)entry.key);                                                                                       \
    prefix##_map_value_delete(entry.value);                                                                        \
  }                                                                                                                \
                                                                                                                   \
  size_t string_##prefix##_pair_set_entry_hash(const string_##prefix##_pair_set_entry_t entry)                     \
  {                                                                                                                \
    return djb2_hash(entry.key);                                                                                   \
  }                                                                                                                \
                                                                                                                   \
  int string_##prefix##_pair_set_entry_equals(const string_##prefix##_pair_set_entry_t entry1,                     \
                                              const string_##prefix##_pair_set_entry_t entry2)                     \
  {                                                                                                                \
    return strcmp(entry1.key, entry2.key) == 0;                                                                    \
  }

DEFINE_MAP_METHODS(plot_func)

int plot_func_map_value_copy(plot_func_t *copy, const plot_func_t value)
{
  *copy = value;

  return 1;
}

void plot_func_map_value_delete(plot_func_t value UNUSED) {}


DEFINE_MAP_METHODS(string)

int string_map_value_copy(char **copy, const char *value)
{
  char *_copy;

  _copy = gks_strdup(value);
  if (_copy == NULL)
    {
      return 0;
    }
  *copy = _copy;

  return 1;
}

void string_map_value_delete(char *value)
{
  free(value);
}


DEFINE_MAP_METHODS(uint)

int uint_map_value_copy(unsigned int *copy, const unsigned int value)
{
  *copy = value;

  return 1;
}

void uint_map_value_delete(unsigned int value UNUSED) {}


DEFINE_MAP_METHODS(args_set)

int args_set_map_value_copy(args_set_t **copy, const args_set_t *value)
{
  /* discard const because it is necessary to work on the object itself */
  /* TODO create two map types: copy and pointer version */
  *copy = (args_set_t *)value;

  return 1;
}

void args_set_map_value_delete(args_set_t *value UNUSED) {}


#undef DEFINE_MAP_METHODS
#undef DEFINE_SET_METHODS


/* ------------------------- event handling ------------------------------------------------------------------------- */

DEFINE_LIST_METHODS(event)

error_t event_list_entry_copy(event_list_entry_t *copy, event_list_const_entry_t entry)
{
  event_list_entry_t _copy;

  _copy = malloc(sizeof(gr_meta_event_t));
  if (_copy == NULL)
    {
      return ERROR_MALLOC;
    }

  memcpy(_copy, entry, sizeof(gr_meta_event_t));
  *copy = _copy;

  return NO_ERROR;
}

error_t event_list_entry_delete(event_list_entry_t entry)
{
  free(entry);
  return NO_ERROR;
}

event_queue_t *event_queue_new(void)
{
  event_queue_t *queue = NULL;

  queue = malloc(sizeof(event_queue_t));
  error_cleanup_if(queue == NULL);
  queue->queue = NULL;
  queue->event_callbacks = NULL;
  queue->queue = event_reflist_new();
  error_cleanup_if(queue->queue == NULL);
  queue->event_callbacks = calloc(_GR_META_EVENT_TYPE_COUNT, sizeof(gr_meta_event_callback_t));
  error_cleanup_if(queue->event_callbacks == NULL);

  return queue;

error_cleanup:
  if (queue != NULL)
    {
      if (queue->queue != NULL)
        {
          event_reflist_delete(queue->queue);
        }
      if (queue->event_callbacks != NULL)
        {
          free(queue->event_callbacks);
        }
      free(queue);
    }

  return NULL;
}

void event_queue_delete(event_queue_t *queue)
{
  event_reflist_delete_with_entries(queue->queue);
  free(queue->event_callbacks);
  free(queue);
}

void event_queue_register(event_queue_t *queue, gr_meta_event_type_t type, gr_meta_event_callback_t callback)
{
  queue->event_callbacks[type] = callback;
}

void event_queue_unregister(event_queue_t *queue, gr_meta_event_type_t type)
{
  queue->event_callbacks[type] = NULL;
}

int event_queue_process_next(event_queue_t *queue)
{
  gr_meta_event_t *event;
  gr_meta_event_type_t type;

  if (event_reflist_empty(queue->queue))
    {
      return 0;
    }

  event = event_reflist_dequeue(queue->queue);
  type = *((int *)event);
  if (queue->event_callbacks[type] != NULL)
    {
      queue->event_callbacks[type](event);
    }

  return 1;
}

int event_queue_process_all(event_queue_t *queue)
{

  if (event_reflist_empty(queue->queue))
    {
      return 0;
    }

  while (event_queue_process_next(queue))
    ;

  return 1;
}

error_t event_queue_enqueue_new_plot_event(event_queue_t *queue, int plot_id)
{
  gr_meta_new_plot_event_t *new_plot_event = NULL;
  error_t error = NO_ERROR;

  new_plot_event = malloc(sizeof(gr_meta_new_plot_event_t));
  error_cleanup_and_set_error_if(new_plot_event == NULL, ERROR_MALLOC);
  new_plot_event->type = GR_META_EVENT_NEW_PLOT;
  new_plot_event->plot_id = plot_id;

  error = event_reflist_enqueue(queue->queue, (gr_meta_event_t *)new_plot_event);
  error_cleanup_if_error;

  return NO_ERROR;

error_cleanup:
  if (new_plot_event != NULL)
    {
      free(new_plot_event);
    }

  return error;
}

error_t event_queue_enqueue_update_plot_event(event_queue_t *queue, int plot_id)
{
  gr_meta_update_plot_event_t *update_plot_event = NULL;
  error_t error = NO_ERROR;

  update_plot_event = malloc(sizeof(gr_meta_update_plot_event_t));
  error_cleanup_and_set_error_if(update_plot_event == NULL, ERROR_MALLOC);
  update_plot_event->type = GR_META_EVENT_UPDATE_PLOT;
  update_plot_event->plot_id = plot_id;

  error = event_reflist_enqueue(queue->queue, (gr_meta_event_t *)update_plot_event);
  error_cleanup_if_error;

  return NO_ERROR;

error_cleanup:
  if (update_plot_event != NULL)
    {
      free(update_plot_event);
    }

  return error;
}

error_t event_queue_enqueue_size_event(event_queue_t *queue, int plot_id, int width, int height)
{
  gr_meta_size_event_t *size_event = NULL;
  error_t error = NO_ERROR;

  size_event = malloc(sizeof(gr_meta_size_event_t));
  error_cleanup_and_set_error_if(size_event == NULL, ERROR_MALLOC);
  size_event->type = GR_META_EVENT_SIZE;
  size_event->plot_id = plot_id;
  size_event->width = width;
  size_event->height = height;

  error = event_reflist_enqueue(queue->queue, (gr_meta_event_t *)size_event);
  error_cleanup_if_error;

  return NO_ERROR;

error_cleanup:
  if (size_event != NULL)
    {
      free(size_event);
    }

  return error;
}

error_t event_queue_enqueue_merge_end_event(event_queue_t *queue, const char *identificator)
{
  gr_meta_merge_end_event_t *merge_end_event = NULL;
  error_t error = NO_ERROR;

  merge_end_event = malloc(sizeof(gr_meta_merge_end_event_t));
  error_cleanup_and_set_error_if(merge_end_event == NULL, ERROR_MALLOC);
  merge_end_event->type = GR_META_EVENT_MERGE_END;
  merge_end_event->identificator = identificator;
  error = event_reflist_enqueue(queue->queue, (gr_meta_event_t *)merge_end_event);
  error_cleanup_if_error;

  return NO_ERROR;

error_cleanup:
  if (merge_end_event != NULL)
    {
      free(merge_end_event);
    }

  return error;
}


#undef DEFINE_LIST_METHODS
