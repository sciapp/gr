#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef _MSC_VER
#include <unistd.h>
#else
#include <io.h>
#endif

#include "logging_int.h"
#include "util_int.h"


/* ######################### private interface ###################################################################### */

/* ========================= functions ============================================================================== */

static void loggerInit(void);


/* ######################### internal implementation ################################################################ */

/* ========================= macros ================================================================================= */

#define ENABLE_LOGGING_ENV_KEY "GRM_DEBUG"


/* ========================= static variables ======================================================================= */

static int logging_enabled = -1;


/* ========================= functions ============================================================================== */

/* ------------------------- logging -------------------------------------------------------------------------------- */

void loggerInit(void)
{
  if (logging_enabled < 0) logging_enabled = isEnvVariableEnabled(ENABLE_LOGGING_ENV_KEY);
}


/* ######################### public implementation ################################################################## */

/* ========================= functions ============================================================================== */

/* ------------------------- logging -------------------------------------------------------------------------------- */

int loggerEnabled(void)
{
  loggerInit();
  return logging_enabled;
}

void logger1(FILE *stream, const char *filename, int line_number, const char *current_function)
{
  loggerInit();

  if (logging_enabled)
    {
#if defined(_WIN32) || defined(__EMSCRIPTEN__)
      fprintf(stream, "[DEBUG:GRM] %s:%d(%s): ", filename, line_number, current_function);
#else
      if (isatty(fileno(stream)))
        {
          fprintf(stream,
                  "\033[32;1m[DEBUG:GRM]\033[0m \033[36m%s\033[0m:\033[33m%d\033[0m(\033[34m%s\033[0m): ", filename,
                  line_number, current_function);
        }
      else
        {
          fprintf(stream, "[DEBUG:GRM] %s:%d(%s): ", filename, line_number, current_function);
        }
#endif
    }
}

void logger2(FILE *stream, const char *format, ...)
{
  va_list vl;

  loggerInit();

  if (logging_enabled)
    {
      va_start(vl, format);
      vfprintf(stream, format, vl);
      va_end(vl);
    }
}
