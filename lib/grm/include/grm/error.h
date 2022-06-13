#ifndef GRM_ERROR_INT_H_INCLUDED
#define GRM_ERROR_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __unix__
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif
#endif

/* ######################### includes ############################################################################### */

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#ifdef _WIN32
#include <io.h>
#include <winerror.h>
#endif


/* ######################### internal interface ##################################################################### */

/* ========================= macros ================================================================================= */

/* ------------------------- error handling ------------------------------------------------------------------------- */

#ifndef NDEBUG
#define debug_print_error(error_message_arguments) \
  do                                               \
    {                                              \
      debug_printf error_message_arguments;        \
    }                                              \
  while (0)

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
#define debug_print_malloc_error()
#define psocketerror(prefix_message)
#endif


/* ========================= datatypes ============================================================================== */

/* ------------------------- error handling ------------------------------------------------------------------------- */

#define ENUM_ELEMENTS(X, Y)                           \
  X(ERROR_NONE, 0)                                    \
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
  X(ERROR_NETWORK_RECV_UNSUPPORTED, 28)               \
  X(ERROR_NETWORK_RECV_CONNECTION_SHUTDOWN, 29)       \
  X(ERROR_NETWORK_SEND, 30)                           \
  X(ERROR_NETWORK_SEND_UNSUPPORTED, 31)               \
  X(ERROR_NETWORK_SOCKET_CLOSE, 32)                   \
  X(ERROR_NETWORK_WINSOCK_CLEANUP, 33)                \
  X(ERROR_CUSTOM_RECV, 34)                            \
  X(ERROR_CUSTOM_SEND, 35)                            \
  X(ERROR_PLOT_COLORMAP, 36)                          \
  X(ERROR_PLOT_NORMALIZATION, 37)                     \
  X(ERROR_PLOT_UNKNOWN_KEY, 38)                       \
  X(ERROR_PLOT_UNKNOWN_ALGORITHM, 39)                 \
  X(ERROR_PLOT_MISSING_ALGORITHM, 40)                 \
  X(ERROR_PLOT_UNKNOWN_KIND, 41)                      \
  X(ERROR_PLOT_MISSING_DATA, 42)                      \
  X(ERROR_PLOT_COMPONENT_LENGTH_MISMATCH, 43)         \
  X(ERROR_PLOT_MISSING_DIMENSIONS, 44)                \
  X(ERROR_PLOT_MISSING_LABELS, 45)                    \
  X(ERROR_PLOT_INVALID_ID, 46)                        \
  X(ERROR_PLOT_OUT_OF_RANGE, 47)                      \
  X(ERROR_PLOT_INCOMPATIBLE_ARGUMENTS, 48)            \
  X(ERROR_PLOT_INVALID_REQUEST, 49)                   \
  X(ERROR_BASE64_BLOCK_TOO_SHORT, 50)                 \
  X(ERROR_BASE64_INVALID_CHARACTER, 51)               \
  X(ERROR_LAYOUT_INVALID_INDEX, 52)                   \
  X(ERROR_LAYOUT_CONTRADICTING_ATTRIBUTES, 53)        \
  X(ERROR_LAYOUT_INVALID_ARGUMENT_RANGE, 54)          \
  X(ERROR_LAYOUT_COMPONENT_LENGTH_MISMATCH, 55)       \
  Y(ERROR_NOT_IMPLEMENTED, 56)

#define ENUM_VALUE(name, value) name = value,
#define ENUM_LAST_VALUE(name, value) name = value
#define STRING_ARRAY_VALUE(name, value) #name,
#define STRING_ARRAY_LAST_VALUE(name, value) #name

typedef enum
{
  ENUM_ELEMENTS(ENUM_VALUE, ENUM_LAST_VALUE)
} err_t;

/* ######################### public implementatin ################################################################### */

/* ========================= global variables ======================================================================= */

/* ------------------------- error handling ------------------------------------------------------------------------- */

extern const char *error_names[];


/* ========================= functions ============================================================================== */

/* ------------------------- error handling ------------------------------------------------------------------------- */

#ifndef NDEBUG
void debug_printf(const char *format, ...);
#endif


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_ERROR_INT_H_INCLUDED */
