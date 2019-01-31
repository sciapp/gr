
#ifndef NO_GTK

#include <stdio.h>

#endif

#include "gks.h"
#include "gkscore.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32

#include <windows.h>
#ifndef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#endif
#endif

  DLLEXPORT void gks_gtkplugin(int fctid, int dx, int dy, int dimx, int *i_arr, int len_f_arr_1, double *f_arr_1,
                               int len_f_arr_2, double *f_arr_2, int len_c_arr, char *c_arr, void **ptr);

#ifdef __cplusplus
}
#endif

#ifndef NO_GTK

void gks_gtkplugin(int fctid, int dx, int dy, int dimx, int *i_arr, int len_f_arr_1, double *f_arr_1, int len_f_arr_2,
                   double *f_arr_2, int len_c_arr, char *c_arr, void **ptr)
{
  if (fctid == 2)
    {
      gks_perror("GTK+ support not yet implemented");
      i_arr[0] = 0;
    }
}

#else

void gks_gtkplugin(int fctid, int dx, int dy, int dimx, int *i_arr, int len_f_arr_1, double *f_arr_1, int len_f_arr_2,
                   double *f_arr_2, int len_c_arr, char *c_arr, void **ptr)
{
  if (fctid == 2)
    {
      gks_perror("GTK+ support not compiled in");
      i_arr[0] = 0;
    }
}

#endif
