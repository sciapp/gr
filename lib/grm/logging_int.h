#ifndef GRM_LOGGING_INT_H_INCLUDED
#define GRM_LOGGING_INT_H_INCLUDED

#ifdef __unix__
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif
#endif

/* ######################### includes ############################################################################### */

#include <stdio.h>
#ifdef _WIN32
#include <io.h>
#endif


/* ######################### interface ############################################################################## */

/* ========================= macros ================================================================================= */

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


#endif /* ifndef GRM_LOGGING_INT_H_INCLUDED */
