#ifndef _WIN32
extern "C" void gks_cairo_plugin(
    int fctid,
    int dx, int dy, int dimx, int *i_arr,
    int len_f_arr_1, double *f_arr_1, int len_f_arr_2, double *f_arr_2,
    int len_c_arr, char *c_arr, void **ptr);
#endif

#include "../plugin.c"
