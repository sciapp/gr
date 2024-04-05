#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include <execinfo.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#include "backtrace_int.h"
#include "util_int.h"


/* ######################### private interface ###################################################################### */

/* ========================= functions ============================================================================== */

static void backtrace_init(void);
static void backtrace_handler(int sig);


/* ######################### private implementation ################################################################# */

/* ========================= macros ================================================================================= */

#define ENABLE_BACKTRACE_ENV_KEY "GRM_BACKTRACE"
#define MAX_CALLSTACK_DEPTH 128
#define MAX_FILEPATH_LENGTH 512


/* ========================= static variables ======================================================================= */

static int is_backtrace_enabled = -1;
static int signals[] = {SIGABRT, SIGSEGV};

/* ========================= functions ============================================================================== */

void backtrace_init(void)
{
  if (is_backtrace_enabled < 0)
    {
      is_backtrace_enabled = is_env_variable_enabled(ENABLE_BACKTRACE_ENV_KEY);
    }
}

void backtrace_handler(int sig)
{
  void *callstack[MAX_CALLSTACK_DEPTH];
  int frames;
  char backtrace_filepath[MAX_FILEPATH_LENGTH];
  int backtrace_fd;

  snprintf(backtrace_filepath, MAX_FILEPATH_LENGTH, "%s/grm_backtrace", get_tmp_directory());
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


/* ######################### internal implementation ################################################################ */

/* ========================= functions ============================================================================== */

void install_backtrace_handler(void)
{
  int i;

  for (i = 0; i < array_size(signals); ++i)
    {
      signal(signals[i], backtrace_handler);
    }
}

void install_backtrace_handler_if_enabled(void)
{
  if (backtrace_enabled())
    {
      install_backtrace_handler();
    }
}

void uninstall_backtrace_handler(void)
{
  int i;

  for (i = 0; i < array_size(signals); ++i)
    {
      signal(signals[i], SIG_DFL);
    }
}

void uninstall_backtrace_handler_if_enabled(void)
{
  if (backtrace_enabled())
    {
      uninstall_backtrace_handler();
    }
}

int backtrace_enabled(void)
{
  backtrace_init();
  return is_backtrace_enabled;
}
