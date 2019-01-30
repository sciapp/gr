#ifndef _CONTOUR_H_
#define _CONTOUR_H_

#ifdef __cplusplus
extern "C"
{
#endif

  void gr_draw_contours(int, int, int, double *, double *, double *, double *, int);
  void gr_draw_tricont(int, double *, double *, double *, int, double *, int *);

#ifdef __cplusplus
}
#endif

#endif
