#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <strsafe.h>
#else
#include <dlfcn.h>
#include <sys/param.h>
#endif

#include "gkscore.h"

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#ifdef _WIN32
#define EXTENSION "dll"
#else
#ifndef aix
#define EXTENSION "so"
#else
#define EXTENSION "a"
#endif
#endif

#define NAME "plugin"
#define ENTRY_ARGS int, int, int, int, int *, int, double *, int, double *, int, char *, void **

static void *load_library(const char *name)
{
  char pathname[MAXPATHLEN];
#ifdef _WIN32
  wchar_t grdir[MAX_PATH], w_pathname[MAX_PATH];
  HINSTANCE handle;
#else
  const char *grdir, *error;
  void *handle;
#endif
  char symbol[255];
  void *entry = NULL;

  snprintf(pathname, MAXPATHLEN, "%s.%s", name, EXTENSION);
#ifdef _WIN32
  handle = LoadLibrary(pathname);
  if (handle == NULL)
    {
      GetEnvironmentVariableW(L"GRDIR", grdir, MAX_PATH);
      StringCbPrintfW(w_pathname, MAX_PATH, L"%ws\\bin\\%S.%S", grdir, name, EXTENSION);
      handle = LoadLibraryExW(w_pathname, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      if (handle == NULL)
        {
          /* Try loading with default search path if altered search path fails
             This value is a combination of LOAD_LIBRARY_SEARCH_APPLICATION_DIR,
             LOAD_LIBRARY_SEARCH_USER_DIRS, and LOAD_LIBRARY_SEARCH_SYSTEM32.
             They are searched in that order.
             Use AddDllDirectory to add additional user directories to the search.
           */
          handle = LoadLibraryExW(w_pathname, NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        }
    }
#else
  handle = dlopen(pathname, RTLD_LAZY);
  if (handle == NULL)
    {
      grdir = gks_getenv("GRDIR");
      if (grdir == NULL) grdir = GRDIR;
      snprintf(pathname, MAXPATHLEN, "%s/lib/%s.%s", grdir, name, EXTENSION);
      handle = dlopen(pathname, RTLD_LAZY);
    }
#endif

  if (handle != NULL)
    {
      snprintf(symbol, 255, "gks_%s", name);
#ifdef _WIN32
      entry = (void *)GetProcAddress(handle, symbol);
#else
      entry = dlsym(handle, symbol);
#endif
      if (entry == NULL)
        {
#ifdef _WIN32
          gks_perror("%s: unresolved symbol", symbol);
#else
          if ((error = dlerror()) != NULL) gks_perror((char *)error);
#endif
        }
    }
  else
    {
#ifdef _WIN32
      DWORD ec = 0;
      ec = GetLastError();
      gks_perror("%s: can't load library, error %d (0x%x)", pathname, ec, ec);
#else
      if ((error = dlerror()) != NULL) gks_perror((char *)error);
#endif
    }

  return entry;
}

static const char *get_qt_version_string()
{
  typedef const char *qversion_t();
  qversion_t *qVersion = NULL;

#ifdef _WIN32
  HMODULE handle = GetModuleHandle("Qt6Core.dll");
  if (handle == NULL)
    {
      handle = GetModuleHandle("Qt5Core.dll");
    }
  if (handle != NULL) qVersion = (qversion_t *)GetProcAddress(handle, "qVersion");
#else
  *(void **)(&qVersion) = dlsym(dlopen(NULL, RTLD_LAZY), "qVersion");
#endif
  if (qVersion != NULL) return qVersion();

  return NULL;
}

void gks_drv_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                    char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;
  const char *env;

  if (name == NULL)
    {
      name = NAME;
      if ((env = gks_getenv("GKS_PLUGIN")) != NULL) name = env;

      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}

void gks_x11_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                    char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;

  if (name == NULL)
    {
      name = "x11plugin";
      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}

void gks_gs_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                   char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;

  if (name == NULL)
    {
      name = "gsplugin";
      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}

void gks_gtk_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                    char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;

  if (name == NULL)
    {
      name = "gtkplugin";
      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}

void gks_wx_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                   char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;

  if (name == NULL)
    {
      name = "wxplugin";
      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}

void gks_qt_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                   char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;
  const char *qt_version_string;
  int qt_major_version;

  if (name == NULL)
    {
      qt_version_string = getenv("GKS_QT_VERSION");
      if (!qt_version_string)
        {
          qt_version_string = get_qt_version_string();
        }
      if (qt_version_string != NULL)
        {
          qt_major_version = atoi(qt_version_string);
          switch (qt_major_version)
            {
            case 6:
              name = "qt6plugin";
              break;
            case 5:
              name = "qt5plugin";
              break;
            default:
              name = "qtplugin";
              break;
            }
        }
      if (name == NULL) name = "qtplugin";
      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}

void gks_svg_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                    char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;

  if (name == NULL)
    {
      name = "svgplugin";
      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}

void gks_wmf_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                    char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;

  if (name == NULL)
    {
      name = "wmfplugin";
      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}

void gks_quartz_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                       char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;

  if (name == NULL)
    {
      name = "quartzplugin";
      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}

void gks_gl_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                   char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;

  if (name == NULL)
    {
      name = "glplugin";
      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}

void gks_cairo_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                      char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;

  if (name == NULL)
    {
      name = "cairoplugin";
      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}

void gks_zmq_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                    char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;

  if (name == NULL)
    {
      name = "zmqplugin";
      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}

void gks_pgf_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                    char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;

  if (name == NULL)
    {
      name = "pgfplugin";
      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}

void gks_video_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                      char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;

  if (name == NULL)
    {
      name = "videoplugin";
      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}

void gks_agg_plugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                    char *chars, void **ptr)
{
  static const char *name = NULL;
  static void (*entry)(ENTRY_ARGS) = NULL;

  if (name == NULL)
    {
      name = "aggplugin";
      *(void **)(&entry) = load_library(name);
    }

  if (entry != NULL) (*entry)(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
}
