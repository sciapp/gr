#ifndef _GRIDIT_H_
#define _GRIDIT_H_

#ifdef __cplusplus
extern "C"
{
#endif

  void idsfft(int *md, int *ncp, int *ndp, double *xd, double *yd, double *zd, int *nxi, int *nyi, double *xi,
              double *yi, double *zi, int *iwk, double *wk);

#ifdef __cplusplus
}
#endif

#endif
