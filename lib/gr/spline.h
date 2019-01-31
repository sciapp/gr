#ifndef _SPLINE_H_
#define _SPLINE_H_

#ifdef __cplusplus
extern "C"
{
#endif

  void cubgcv(double *x, double *f, double *df, int *n, double *y, double *c, int *ic, double *var, int *job,
              double *se, double *wk, int *ier);
  void b_spline(int n, double *x, double *y, int m, double *sx, double *sy);

#ifdef __cplusplus
}
#endif

#endif
