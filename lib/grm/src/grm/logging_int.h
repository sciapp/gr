#ifndef GRM_LOGGING_INT_H_INCLUDED
#define GRM_LOGGING_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <stdio.h>


/* ######################### interface ############################################################################## */

/* ========================= macros ================================================================================= */

/* ------------------------- logging -------------------------------------------------------------------------------- */

#define logger(logger_arguments)                             \
  do                                                         \
    {                                                        \
      logger1(stderr, __FILE__, __LINE__, CURRENT_FUNCTION); \
      logger2 logger_arguments;                              \
    }                                                        \
  while (0)


/* ========================= functions ============================================================================== */

/* ------------------------- logging -------------------------------------------------------------------------------- */

int loggerEnabled(void);
void logger1(FILE *stream, const char *filename, int line_number, const char *current_function);
void logger2(FILE *stream, const char *format, ...);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_LOGGING_INT_H_INCLUDED */
