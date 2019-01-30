#ifndef _BOUNDARY_H_
#define _BOUNDARY_H_

#ifdef __cplusplus
extern "C"
{
#endif

  int find_boundary(int n, double *x, double *y, double r, double (*r_function)(double x, double y), int n_contour,
                    int *contour);

#ifdef __cplusplus
}
#endif

#endif
