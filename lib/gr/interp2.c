
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "gr.h"

#define INTERP2_NEAREST 0
#define INTERP2_LINEAR 1
#define INTERP2_CUBIC 3
#define INTERP2_SPLINE 2

static
char *xmalloc(int size)
{
  char *result = (char *) malloc(size);
  if (!result) {
    fprintf(stderr, "out of virtual memory\n");
    abort();
  }
  return result;
}

static
double cubic_interp(const double *x, const double *y, double xq)
{
  double a[4], result;

  a[0] = y[0];
  a[1] = (y[1] - a[0])/(x[1] - x[0]);
  a[2] = (y[2] - a[0] - a[1] * (x[2] - x[0])) / ((x[2] - x[0]) * (x[2] - x[1]));
  a[3] = (y[3] - a[0] - a[1] * (x[3] - x[0]) - a[2] * (x[3] - x[0]) *
         (x[3] - x[1])) / ((x[3] - x[0]) * (x[3] - x[1]) * (x[3] - x[2]));
  /* Horner's method */
  result = a[3];
  result = result * (xq - x[2]) + a[2];
  result = result * (xq - x[1]) + a[1];
  result = result * (xq - x[0]) + a[0];

  return result;
}

static
double bilinear_interp(const double *x, const double *y, const double *z,
                       int ix, int iy, int nx, double xq, double yq)
{
  double f1, f2;

  f1 = ((x[ix + 1] - xq) / (x[ix + 1] - x[ix])) * z[iy * nx + ix];
  f1 += ((xq - x[ix]) / (x[ix + 1] - x[ix])) * z[iy * nx + ix + 1];
  f2 = ((x[ix + 1] - xq) / (x[ix + 1] - x[ix])) * z[(iy + 1) * nx + ix];
  f2 += ((xq - x[ix]) / (x[ix + 1] - x[ix])) * z[(iy + 1) * nx + ix + 1];

  return ((y[iy + 1] - yq) / (y[iy + 1] - y[iy])) * f1 + ((yq - y[iy]) /
         (y[iy + 1] - y[iy])) * f2;
}

static
double linear_interp(const double *x, const double *y, double xq)
{
  return y[0] + ((y[1] - y[0]) / (x[1] - x[0])) * (xq - x[0]);
}

static
double bicubic_interp(const double *x, const double *y, const double *z,
                      int ix, int iy, int nx, int ny, double xq, double yq)
{
  double a[4];

  if (ix + 2 >= nx || ix - 1 < 0) {
    if (iy + 2 >= ny || iy - 1 < 0) {
      /* fallback: linear interpolation */
      return bilinear_interp(x, y, z, ix, iy, nx, xq, yq);
    } else {
      /* linear interpolation in X direction, cubic in Y direction */
      a[0] = linear_interp(x + ix, z + ((iy - 1) * nx + ix), xq);
      a[1] = linear_interp(x + ix, z + (iy * nx + ix), xq);
      a[2] = linear_interp(x + ix, z + ((iy + 1) * nx + ix), xq);
      a[3] = linear_interp(x + ix, z + ((iy + 2) * nx + ix), xq);
      return cubic_interp(y + iy - 1, a, yq);
    }
  } else {
    if (iy + 2 >= ny || iy - 1 < 0) {
      /* cubic interpolation in Y direction, linear in Y direction */
      a[0] = cubic_interp(x + ix - 1, z + (iy * nx + ix - 1), xq);
      a[1] = cubic_interp(x + ix - 1, z + ((iy + 1) * nx + ix - 1), xq);
      return linear_interp(y + iy, a, yq);
    } else {
      /* cubic interpolation in both directions */
      a[0] = cubic_interp(x + ix - 1, z + ((iy - 1) * nx + ix - 1), xq);
      a[1] = cubic_interp(x + ix - 1, z + (iy * nx + ix - 1), xq);
      a[2] = cubic_interp(x + ix - 1, z + ((iy + 1) * nx + ix - 1), xq);
      a[3] = cubic_interp(x + ix - 1, z + ((iy + 2) * nx + ix - 1), xq);
      return cubic_interp(y + iy - 1, a, yq);
    }
  }
}

static
void create_splines(const double *x, const double *y, int n, double **spline)
{
  int i;
  double *h, *l, *m, *z, *alpha;

  h = (double *) xmalloc((n - 1) * sizeof(double));
  l = (double *) xmalloc(n * sizeof(double));
  m = (double *) xmalloc((n - 1) * sizeof(double));
  z = (double *) xmalloc(n * sizeof(double));
  alpha = (double *) xmalloc((n - 1) * sizeof(double));

  for (i = 0; i < n - 1; i++) {
    h[i] = x[i + 1] - x[i];
    spline[i][0] = y[i];
  }
  spline[n-1][0] = y[n - 1];
  for (i = 1; i < n - 1; i++) {
    alpha[i] = (3./h[i]) * (y[i+1] - y[i]) - (3./(h[i-1])) * (y[i] - y[i - 1]);
  }
  l[0] = 1;
  m[0] = 0;
  z[0] = 0;
  for (i = 1; i < n - 1; i++) {
    l[i] = 2 * (x[i + 1] - x[i - 1] - h[i - 1] * m[i - 1]);
    m[i] = h[i] / l[i];
    z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
  }
  l[n - 1] = 1;
  z[n - 1] = 0;
  spline[n - 1][2] = 0;
  for (i = n - 2; i >= 0; i--) {
    spline[i][2] = z[i] - m[i] * spline[i + 1][2];
    spline[i][1] = (spline[i + 1][0] - spline[i][0]) / h[i] -
                   h[i] * ((spline[i + 1][2] + 2 * spline[i][2]) / 3);
    spline[i][3] = (spline[i + 1][2] - spline[i][2]) / (3 * h[i]);
  }
  free(h);
  free(l);
  free(m);
  free(z);
  free(alpha);
}

void gr_interp2(
  int nx, int ny, const double *x, const double *y, const double *z,
  int nxq, int nyq, const double *xq, const double *yq, double *zq,
  int method, double extrapval)
{
  int ixq, iyq, ix, iy, ind, i;
  double ***x_splines, **spline, *a, diff;

  if (method == INTERP2_SPLINE) {
    x_splines = (double ***) xmalloc(ny * sizeof(double **));
    spline = (double **) xmalloc(ny * sizeof(double *));
    for (ind = 0; ind < ny; ind++) {
      x_splines[ind] = (double **) xmalloc(ny * sizeof(double *));
      for (i = 0; i < nx; i++) {
        x_splines[ind][i] = (double *) xmalloc(4 * sizeof(double));
      }
      spline[ind] = (double *) xmalloc(4 * sizeof(double));
      create_splines(x, z + ind * nx, nx, x_splines[ind]);
    }
    a = (double *) xmalloc(ny * sizeof(double));
  }
  for (iyq = 0; iyq < nyq; iyq++) {
    for (ixq = 0; ixq < nxq; ixq++) {
      if ((xq[ixq] > x[nx - 1] || xq[ixq] < x[0]) ||
          (yq[iyq] > y[ny - 1] || yq[iyq] < y[0])) {
        /* location outside of grid */
        zq[iyq * nxq + ixq] = extrapval;
      } else {
        if (xq[ixq] > x[nx - 2]) {
          /* index of next X value less than xq[ixq] is the second last */
          ix = nx - 2;
        } else {
          ix = 0;
          while (ix + 1 < nx && x[ix + 1] < xq[ixq]) {
            /* searching for index of next X value less than xq[ixq] */
            ix++;
          }
        }

        if (yq[iyq] > y[ny - 2]) {
          /* index of next Y value less than yq[iyq] is the second last */
          iy = ny - 2;
        } else {
          iy = 0;
          while (iy + 1 < ny && y[iy + 1] < yq[iyq]) {
            /* searching for index of next Y value less than yq[iyq] */
            iy++;
          }
        }

        if (method == INTERP2_NEAREST) {
          if (ix + 1 < nx && xq[ixq] - x[ix] > x[ix + 1] - xq[ixq]) {
            ix++;
          }
          if (iy + 1 < ny && yq[iyq] - y[iy] > y[iy + 1] - yq[iyq]) {
            iy++;
          }
          zq[iyq * nxq + ixq] = z[iy * nx + ix];
        } else if (method == INTERP2_SPLINE) {
          /* interpolation in X direction: */
          diff = xq[ixq] - x[ix];
          for (ind = 0; ind < ny; ind++) {
            /* Horner's method */
            a[ind] = x_splines[ind][ix][3];
            a[ind] = a[ind] * diff + x_splines[ind][ix][2];
            a[ind] = a[ind] * diff + x_splines[ind][ix][1];
            a[ind] = a[ind] * diff + x_splines[ind][ix][0];
          }

          /* interpolation in Y direction: */
          create_splines(y, a, ny, spline);
          diff = yq[iyq] - y[iy];
          /* Horner's method */
          zq[iyq * nxq + ixq] = spline[iy][3];
          zq[iyq * nxq + ixq] = zq[iyq * nxq + ixq] * diff + spline[iy][2];
          zq[iyq * nxq + ixq] = zq[iyq * nxq + ixq] * diff + spline[iy][1];
          zq[iyq * nxq + ixq] = zq[iyq * nxq + ixq] * diff + spline[iy][0];
        } else if (method == INTERP2_CUBIC) {
          zq[iyq * nxq + ixq] = bicubic_interp(x, y, z, ix, iy,
                                               nx, ny, xq[ixq], yq[iyq]);
        } else if (method == INTERP2_LINEAR) {
          zq[iyq * nxq + ixq] = bilinear_interp(x, y, z, ix, iy,
                                                nx, xq[ixq], yq[iyq]);
        }
      }
    }
  }
  if (method == INTERP2_SPLINE) {
    /* free allocated memory */
    for (ind = 0; ind < ny; ind++) {
      for (i = 0; i < nx; i++) {
        free(x_splines[ind][i]);
      }
      free(x_splines[ind]);
      free(spline[ind]);
    }
    free(x_splines);
    free(spline);
    free(a);
  }
}
