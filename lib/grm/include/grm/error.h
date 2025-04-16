#ifndef GRM_ERROR_INT_H_INCLUDED
#define GRM_ERROR_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#else
#ifdef __unix__
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif
#endif
#endif


/* ######################### internal interface ##################################################################### */

/* ######################### public interface ####################################################################### */

/* ========================= datatypes ============================================================================== */

/* ------------------------- error handling ------------------------------------------------------------------------- */

#define GRM_ERROR_ENUM_ELEMENTS(X, Y)                     \
  X(GRM_ERROR_NONE, 0)                                    \
  X(GRM_ERROR_UNSPECIFIED, 1)                             \
  X(GRM_ERROR_INTERNAL, 2)                                \
  X(GRM_ERROR_MALLOC, 3)                                  \
  X(GRM_ERROR_UNSUPPORTED_OPERATION, 4)                   \
  X(GRM_ERROR_UNSUPPORTED_DATATYPE, 5)                    \
  X(GRM_ERROR_INVALID_ARGUMENT, 6)                        \
  X(GRM_ERROR_ARGS_INVALID_KEY, 7)                        \
  X(GRM_ERROR_ARGS_INCREASING_NON_ARRAY_VALUE, 8)         \
  X(GRM_ERROR_ARGS_INCREASING_MULTI_DIMENSIONAL_ARRAY, 9) \
  X(GRM_ERROR_PARSE_NULL, 10)                             \
  X(GRM_ERROR_PARSE_BOOL, 11)                             \
  X(GRM_ERROR_PARSE_INT, 12)                              \
  X(GRM_ERROR_PARSE_DOUBLE, 13)                           \
  X(GRM_ERROR_PARSE_STRING, 14)                           \
  X(GRM_ERROR_PARSE_ARRAY, 15)                            \
  X(GRM_ERROR_PARSE_OBJECT, 16)                           \
  X(GRM_ERROR_PARSE_UNKNOWN_DATATYPE, 17)                 \
  X(GRM_ERROR_PARSE_INVALID_DELIMITER, 18)                \
  X(GRM_ERROR_PARSE_INCOMPLETE_STRING, 19)                \
  X(GRM_ERROR_PARSE_MISSING_OBJECT_CONTAINER, 20)         \
  X(GRM_ERROR_PARSE_XML_NO_SCHEMA_FILE, 21)               \
  X(GRM_ERROR_PARSE_XML_INVALID_SCHEMA, 22)               \
  X(GRM_ERROR_PARSE_XML_FAILED_SCHEMA_VALIDATION, 23)     \
  X(GRM_ERROR_PARSE_XML_PARSING, 24)                      \
  X(GRM_ERROR_NETWORK_WINSOCK_INIT, 25)                   \
  X(GRM_ERROR_NETWORK_SOCKET_CREATION, 26)                \
  X(GRM_ERROR_NETWORK_SOCKET_BIND, 27)                    \
  X(GRM_ERROR_NETWORK_SOCKET_LISTEN, 28)                  \
  X(GRM_ERROR_NETWORK_CONNECTION_ACCEPT, 29)              \
  X(GRM_ERROR_NETWORK_HOSTNAME_RESOLUTION, 30)            \
  X(GRM_ERROR_NETWORK_CONNECT, 31)                        \
  X(GRM_ERROR_NETWORK_RECV, 32)                           \
  X(GRM_ERROR_NETWORK_RECV_UNSUPPORTED, 33)               \
  X(GRM_ERROR_NETWORK_RECV_CONNECTION_SHUTDOWN, 34)       \
  X(GRM_ERROR_NETWORK_SEND, 35)                           \
  X(GRM_ERROR_NETWORK_SEND_UNSUPPORTED, 36)               \
  X(GRM_ERROR_NETWORK_SOCKET_CLOSE, 37)                   \
  X(GRM_ERROR_NETWORK_WINSOCK_CLEANUP, 38)                \
  X(GRM_ERROR_CUSTOM_RECV, 39)                            \
  X(GRM_ERROR_CUSTOM_SEND, 40)                            \
  X(GRM_ERROR_PLOT_COLORMAP, 41)                          \
  X(GRM_ERROR_PLOT_NORMALIZATION, 42)                     \
  X(GRM_ERROR_PLOT_UNKNOWN_KEY, 43)                       \
  X(GRM_ERROR_PLOT_UNKNOWN_ALGORITHM, 44)                 \
  X(GRM_ERROR_PLOT_MISSING_ALGORITHM, 45)                 \
  X(GRM_ERROR_PLOT_UNKNOWN_KIND, 46)                      \
  X(GRM_ERROR_PLOT_MISSING_DATA, 47)                      \
  X(GRM_ERROR_PLOT_COMPONENT_LENGTH_MISMATCH, 48)         \
  X(GRM_ERROR_PLOT_MISSING_DIMENSIONS, 49)                \
  X(GRM_ERROR_PLOT_MISSING_LABELS, 50)                    \
  X(GRM_ERROR_PLOT_INVALID_ID, 51)                        \
  X(GRM_ERROR_PLOT_OUT_OF_RANGE, 52)                      \
  X(GRM_ERROR_PLOT_INCOMPATIBLE_ARGUMENTS, 53)            \
  X(GRM_ERROR_PLOT_INVALID_REQUEST, 54)                   \
  X(GRM_ERROR_BASE64_BLOCK_TOO_SHORT, 55)                 \
  X(GRM_ERROR_BASE64_INVALID_CHARACTER, 56)               \
  X(GRM_ERROR_LAYOUT_INVALID_INDEX, 57)                   \
  X(GRM_ERROR_LAYOUT_CONTRADICTING_ATTRIBUTES, 58)        \
  X(GRM_ERROR_LAYOUT_INVALID_ARGUMENT_RANGE, 59)          \
  X(GRM_ERROR_LAYOUT_COMPONENT_LENGTH_MISMATCH, 60)       \
  X(GRM_ERROR_TMP_DIR_CREATION, 61)                       \
  Y(GRM_ERROR_NOT_IMPLEMENTED, 62)

#define GRM_ENUM_VALUE(name, value) name = value,
#define GRM_ENUM_LAST_VALUE(name, value) name = value
#define GRM_STRING_ARRAY_VALUE(name, value) #name,

typedef enum
{
  GRM_ERROR_ENUM_ELEMENTS(GRM_ENUM_VALUE, GRM_ENUM_LAST_VALUE)
} grm_error_t;

/* ######################### public implementatin ################################################################### */

/* ========================= global variables ======================================================================= */

/* ------------------------- error handling ------------------------------------------------------------------------- */

extern const char *grm_error_names[];


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_ERROR_INT_H_INCLUDED */
