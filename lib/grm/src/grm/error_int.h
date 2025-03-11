#ifndef ERROR_INT_H_INCLUDED
#define ERROR_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <grm/error.h>

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
#define debugPrintError(error_message_arguments) \
  do                                             \
    {                                            \
      debugPrintf error_message_arguments;       \
    }                                            \
  while (0)

#define debugPrintMallocError()                                                                                       \
  do                                                                                                                  \
    {                                                                                                                 \
      if (isatty(fileno(stderr)))                                                                                     \
        {                                                                                                             \
          debugPrintError(                                                                                            \
              ("\033[96m%s\033[0m:\033[93m%d\033[0m: Memory allocation failed -> out of virtual memory.\n", __FILE__, \
               __LINE__));                                                                                            \
        }                                                                                                             \
      else                                                                                                            \
        {                                                                                                             \
          debugPrintError(("%s:%d: Memory allocation failed -> out of virtual memory.\n", __FILE__, __LINE__));       \
        }                                                                                                             \
    }                                                                                                                 \
  while (0)

#ifdef _WIN32
#define pSocketError(prefix_message)                                                                                   \
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
#define pSocketError(prefix_message) perror(prefix_message)
#endif
#else
#define debugPrintError(error_message_arguments)
#define debugPrintMallocError()
#define pSocketError(prefix_message)
#endif


/* ========================= functions ============================================================================== */

/* ------------------------- error handling ------------------------------------------------------------------------- */

#ifndef NDEBUG
void debugPrintf(const char *format, ...);
#endif


#ifdef __cplusplus
}
#endif
#endif /* ifndef ERROR_INT_H_INCLUDED */
