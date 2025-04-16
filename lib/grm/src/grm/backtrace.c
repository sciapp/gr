#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* FreeBSD excluded due to BinaryBuilder issues */
#if (defined(__unix__) || defined(__APPLE__)) && !defined(__EMSCRIPTEN__) && !defined(__FreeBSD__)
#define BACKTRACE_AVAILABLE 1
#endif

/* ######################### includes ############################################################################### */

#ifdef BACKTRACE_AVAILABLE
#include <execinfo.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <stdio.h>

#include "backtrace_int.h"
#include "util_int.h"


/* ######################### private interface ###################################################################### */

/* ========================= functions ============================================================================== */

#ifdef BACKTRACE_AVAILABLE
static void backtraceInit(void);
static void backtraceHandler(int sig);
#endif


/* ######################### private implementation ################################################################# */

/* ========================= macros ================================================================================= */

#define ENABLE_BACKTRACE_ENV_KEY "GRM_BACKTRACE"
#define MAX_CALLSTACK_DEPTH 128
#define MAX_FILEPATH_LENGTH 512


/* ========================= static variables ======================================================================= */

#ifdef BACKTRACE_AVAILABLE
static int is_backtrace_enabled = -1;
static int signals[] = {SIGABRT, SIGSEGV};
#endif

/* ========================= functions ============================================================================== */

#ifdef BACKTRACE_AVAILABLE
static const char *getTmpDirectoryNoMalloc(void)
{
  const char *tmp_dir;
  const char *env_vars[] = {
      "TMPDIR",
      "TMP",
      "TEMP",
      "TEMPDIR",
  };
  int i;

  for (i = 0; i < arraySize(env_vars); ++i)
    {
      if ((tmp_dir = getenv(env_vars[i])) != NULL) return tmp_dir;
    }

  return "/tmp";
}

void backtraceInit(void)
{
  if (is_backtrace_enabled < 0) is_backtrace_enabled = isEnvVariableEnabled(ENABLE_BACKTRACE_ENV_KEY);
}

void backtraceHandler(int sig)
{
  void *callstack[MAX_CALLSTACK_DEPTH];
  int frames;
  char backtrace_filepath[MAX_FILEPATH_LENGTH];
  int backtrace_fd;

  snprintf(backtrace_filepath, MAX_FILEPATH_LENGTH, "%s/grm_backtrace", getTmpDirectoryNoMalloc());
  frames = backtrace(callstack, MAX_CALLSTACK_DEPTH);
  backtrace_fd = open(backtrace_filepath, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IRGRP | S_IROTH);
  backtrace_symbols_fd(callstack, frames, backtrace_fd);
  close(backtrace_fd);
  switch (sig)
    {
    case SIGABRT:
      fprintf(stderr, "Caught \"Abort\" (signal %d)", sig);
      break;
    case SIGSEGV:
      fprintf(stderr, "Caught \"Segmentation fault\" (signal %d)", sig);
      break;
    default:
      fprintf(stderr, "Caught signal %d", sig);
    }
  fprintf(stderr, ", backtrace written to \"%s\"\n", backtrace_filepath);

  exit(1);
}
#endif


/* ######################### internal implementation ################################################################ */

/* ========================= functions ============================================================================== */

void installBacktraceHandler(void)
{
#ifdef BACKTRACE_AVAILABLE
  int i;

  for (i = 0; i < arraySize(signals); ++i)
    {
      signal(signals[i], backtraceHandler);
    }
#else
  fprintf(stderr, "Backtrace support not compiled in.\n");
#endif
}

void installBacktraceHandlerIfEnabled(void)
{
#ifdef BACKTRACE_AVAILABLE
  if (backtraceEnabled()) installBacktraceHandler();
#endif
}

void uninstallBacktraceHandler(void)
{
#ifdef BACKTRACE_AVAILABLE
  int i;

  for (i = 0; i < arraySize(signals); ++i)
    {
      signal(signals[i], SIG_DFL);
    }
#endif
}

void uninstallBacktraceHandlerIfEnabled(void)
{
#ifdef BACKTRACE_AVAILABLE
  if (backtraceEnabled()) uninstallBacktraceHandler();
#endif
}

int backtraceEnabled(void)
{
#ifdef BACKTRACE_AVAILABLE
  backtraceInit();
  return is_backtrace_enabled;
#else
  return 0;
#endif
}
