/*
 *    Algorithm 642 collected algorithms from ACM.
 *    Algorithm appeared in ACM-Trans. Math. Software, Vol.12, No. 2,
 *    Jun., 1986, p. 150.
 *
 *  Subroutine Name     - cubgcv
 *
 *  Author              - M.F.Hutchinson
 *                        CSIRO Division of Mathematics and Statistics
 *                        P.O. Box 1965
 *                        Canberra, Act 2601
 *                        Australia
 *
 *  Latest Revision     - 15 August 1985
 *
 *  Purpose             - Cubic spline data smoother
 *
 *  Usage               - cubgcv (x,f,df,n,y,c,ic,var,job,se,wk,ier)
 *
 *  Arguments    x      - Vector of length n containing the
 *                          abscissae of the n data points
 *                          (x(i),f(i)) i=0..n-1. (input) x
 *                          must be ordered so that
 *                          x(i) < x(i+1).
 *               f      - Vector of length n containing the
 *                          ordinates (or function values)
 *                          of the n data points (input).
 *               df     - Vector of length n. (input/output)
 *                          df(i) is the relative standard deviation
 *                          of the error associated with data point i.
 *                          Each df(i) must be positive.  The values in
 *                          df are scaled by the subroutine so that
 *                          their mean square value is 1, and unscaled
 *                          again on normal exit.
 *                          The mean square value of the df(i) is returned
 *                          in wk(6) on normal exit.
 *                          If the absolute standard deviations are known,
 *                          these should be provided in df and the error
 *                          variance parameter var (see below) should then
 *                          be set to 1.
 *                          If the relative standard deviations are unknown,
 *                          set each df(i)=1.
 *               n      - Number of data points (input).
 *                          n must be >= 3.
 *               y,c    - Spline coefficients. (output) y
 *                          is a vector of length n. c is
 *                          an n-1 by 3 matrix. The value
 *                          of the spline approximation at t is
 *                          s(t)=((c(i,2)*d+c(i,1))*d+c(i,0))*d+y(i)
 *                          where x(i)<=t<x(i+1) and
 *                          d = t-x(i).
 *               ic     - Row dimension of matrix c exactly
 *                          as specified in the dimension
 *                          statement in the calling program. (input)
 *               var    - Error variance. (input/output)
 *                          If var is negative (i.e. unknown) then
 *                          the smoothing parameter is determined
 *                          by minimizing the generalized cross validation
 *                          and an estimate of the error variance is
 *                          returned in var.
 *                          If var is non-negative (i.e. known) then the
 *                          smoothing parameter is determined to minimize
 *                          an estimate, which depends on var, of the true
 *                          mean square error, and var is unchanged.
 *                          In particular, if var is zero, then an
 *                          interpolating natural cubic spline is calculated.
 *                          var should be set to 1 if absolute standard
 *                          deviations have been provided in df (see above).
 *               job    - Job selection parameter. (input)
 *                        job = 0 should be selected if point standard error
 *                          estimates are not required in se.
 *                        job = 1 should be selected if point standard error
 *                          estimates are required in se.
 *               se     - Vector of length n containing bayesian standard
 *                          error estimates of the fitted spline values in y.
 *                          se is not referenced if job=0. (output)
 *               wk     - Work vector of length 7*(n + 2). on normal exit the
 *                          first 7 values of wk are assigned as follows:
 *
 *                          wk(0) = smoothing parameter (= rho/(rho + 1))
 *                          wk(1) = estimate of the number of degrees of
 *                                  freedom of the residual sum of squares
 *                          wk(2) = generalized cross validation
 *                          wk(3) = mean square residual
 *                          wk(4) = estimate of the true mean square error
 *                                  at the data points
 *                          wk(5) = estimate of the error variance
 *                          wk(6) = mean square value of the df(i)
 *
 *                          if wk(0)=0 (rho=0) an interpolating natural cubic
 *                          spline has been calculated.
 *                          If wk(0)=1 (rho=infinite) a least squares
 *                          regression line has been calculated.
 *                          wk(1) is an estimate of the number of degrees of
 *                          freedom of the residual which reduces to the
 *                          usual value of n-2 when a least squares regression
 *                          line is calculated.
 *                          wk(2),wk(3),wk(4) are calculated with the df(i)
 *                          scaled to have mean square value 1.  The
 *                          unscaled values of wk(2),wk(3),wk(4) may be
 *                          calculated by dividing by wk(7).
 *                          wk(5) coincides with the output value of var if
 *                          var is negative on input.  It is calculated with
 *                          the unscaled values of the df(i) to facilitate
 *                          comparisons with a priori variance estimates.
 *
 *               ier    - Error parameter. (output)
 *                        Terminal error
 *                          ier = 129, ic is less than n-1.
 *                          ier = 130, n is less than 3.
 *                          ier = 131, input abscissae are not
 *                            ordered so that x(i)<x(i+1).
 *                          ier = 132, df(i) is not positive for some i.
 *                          ier = 133, job is not 0 or 1.
 *
 *  Required Routines   - spint1,spfit1,spcof1,sperr1
 *
 *  Remarks      The number of arithmetic operations required by the
 *               subroutine is proportional to n.  The subroutine
 *               uses an algorithm developed by M.F. Hutchinson and
 *               F.R. De Hoog, 'Smoothing noisy data with spline
 *               functions', Numer. Math. (in press)
 */

#include <stdlib.h>
#include <math.h>

#include "spline.h"

#define DEPS 1E-16

#define C(row,col)  c[*ic*(col)+row]
#define R(row,col)  r[(*n+2)*(col)+row]
#define T(row,col)  t[(*n+2)*(col)+row]
#define WK(row,col) wk[(*n+2)*(col)+row]

static
void spcof1(double *x, double *avh, double *y, double *dy, int *n, double *p,
	    double *q, double *a, double *c, int *ic, double *u, double *v)
/*
 * Calculates coefficients of a cubic smoothing spline from
 * parameters calculated by subroutine spfit1.
 */
{
  int i;
  double h, qh;

/* Calculate a */
  qh = *q / ((*avh) * (*avh));
  for (i = 0; i < *n; i++)
    {
      a[i] = y[i] - *p * dy[i] * v[i + 1];
      u[i + 1] *= qh;
    }

/* Calculate c */
  for (i = 1; i < *n; i++)
    {
      h = x[i] - x[i - 1];
      C(i - 1, 2) = (u[i + 1] - u[i]) / (3 * h);
      C(i - 1, 0) = (a[i] - a[i - 1]) / h - (h * C(i - 1, 2) + u[i]) * h;
      C(i - 1, 1) = u[i];
    }
}

static
void sperr1(double *x, double *avh, double *dy, int *n, double *r, double *p,
	    double *var, double *se)
/*
 * Calculates Bayesian estimates of the standard errors of the fitted
 * values of a cubic smoothing spline by calculating the diagonal elements
 * of the influence matrix.
 */
{
  int i;
  double f, g, h, f1, g1, h1;

/* Initialize */
  h = *avh / (x[1] - x[0]);
  se[0] = 1 - (*p) * dy[0] * dy[0] * h * h * R(2, 0);
  R(1, 0) = R(1, 1) = R(1, 2) = 0;

/* Calculate diagonal elements */
  for (i = 2; i < *n; i++)
    {
      f = h;
      h = *avh / (x[i] - x[i - 1]);
      g = -f - h;
      f1 = f * R(i - 1, 0) + g * R(i - 1, 1) + h * R(i - 1, 2);
      g1 = f * R(i - 1, 1) + g * R(i, 0) + h * R(i, 1);
      h1 = f * R(i - 1, 2) + g * R(i, 1) + h * R(i + 1, 0);
      se[i - 1] = 1 - (*p) * dy[i - 1] * dy[i - 1] *
	(f * f1 + g * g1 + h * h1);
    }
  se[*n - 1] = 1 - (*p) * dy[*n - 1] * dy[*n - 1] * h * h * R(*n - 1, 0);

/* Calculate standard error estimates */
  for (i = 0; i < *n; i++)
    se[i] = (se[i] * (*var) >= 0) ? sqrt(se[i] * (*var)) * dy[i] : 0;
}

static
void spint1(double *x, double *avh, double *y, double *dy, double *avdy, int *n,
	    double *a, double *c, int *ic, double *r, double *t, int *ier)
/*
 * Initializes the arrays c, r and t for one dimensional cubic
 * smoothing spline fitting by subroutine spfit1.  The values
 * df(i) are scaled so that the sum of their squares is n
 * and the average of the differences x(i+1) - x(i) is calculated
 * in avh in order to avoid underflow and overflow problems in
 * spfit1.
 *
 * Subroutine sets ier if elements of x are non-increasing,
 * If n is less than 3, if ic is less than n-1 or if dy(i) is
 * not positive for some i.
 */
{
  int i, done = 0;
  double e, f, g, h;

/* Initialization and input checking */
  *ier = 0;
  if (*n < 3)
    *ier = 130;
  else if (*ic < *n - 1)
    *ier = 129;
  else
    {

/* Get average x spacing in avh */
      g = 0;
      for (i = 1; i < *n; i++)
	{
	  h = x[i] - x[i - 1];
	  if (h <= 0)
	    {
	      done = 1;
	      break;
	    }
	  else
	    g += h;
	}
      if (!done)
	{
	  *avh = g / (*n - 1);

/* Scale relative weights */
	  g = 0;
	  for (i = 0; i < *n; i++)
	    if (dy[i] <= 0)
	      {
		done = 2;
		break;
	      }
	    else
	      g += dy[i] * dy[i];
	}
      if (!done)
	{
	  *avdy = sqrt(g / (*n));
	  for (i = 0; i < *n; i++)
	    dy[i] /= (*avdy);

/* Initialize h, f */
	  h = (x[1] - x[0]) / (*avh);
	  f = (y[1] - y[0]) / h;

/* Calculate a, t, r */
	  for (i = 1; i < *n - 1; i++)
	    {
	      g = h;
	      h = (x[i + 1] - x[i]) / (*avh);
	      e = f;
	      f = (y[i + 1] - y[i]) / h;
	      a[i] = f - e;
	      T(i + 1, 0) = 2 * (g + h) / 3;
	      T(i + 1, 1) = h / 3;
	      R(i + 1, 2) = dy[i - 1] / g;
	      R(i + 1, 0) = dy[i + 1] / h;
	      R(i + 1, 1) = -dy[i] / g - dy[i] / h;
	    }

/* Calculate c = r'*r */
	  R(*n, 1) = 0;
	  R(*n, 2) = 0;
	  R(*n + 1, 2) = 0;
	  for (i = 1; i < *n - 1; i++)
	    {
	      C(i, 0) = R(i + 1, 0) * R(i + 1, 0) +
		R(i + 1, 1) * R(i + 1, 1) +
		R(i + 1, 2) * R(i + 1, 2);
	      C(i, 1) = R(i + 1, 0) * R(i + 2, 1) +
		R(i + 1, 1) * R(i + 2, 2);
	      C(i, 2) = R(i + 1, 0) * R(i + 3, 2);
	    }
	  return;
	}
      if (done == 1)
	*ier = 131;
      else
	*ier = 132;
    }
}

static
void spfit1(double *x, double *avh, double *dy, int *n, double *rho, double *p,
	    double *q, double *fun, double *var, double *stat, double *a,
	    double *c, int *ic, double *r, double *t, double *u, double *v)
/*
 * Fits a cubic smoothing spline to data with relative
 * weighting dy for a given value of the smoothing parameter
 * rho using an algorithm based on that of C.H. Reinsch (1967),
 * Numer. Math. 10, 177-183.
 *
 * The trace of the influence matrix is calculated using an
 * algorithm developed by M.F.Hutchinson and F.R.De Hoog (Numer.
 * Math., in press), enabling the generalized cross validation
 * and related statistics to be calculated in order n operations.
 *
 * The arrays a, c, r and t are assumed to have been initialized
 * by the subroutine spint1.  overflow and underflow problems are
 * avoided by using p=rho/(1 + rho) and q=1/(1 + rho) instead of
 * rho and by scaling the differences x(i+1) - x(i) by avh.
 *
 * The values in df are assumed to have been scaled so that the
 * sum of their squared values is n.  The value in var, when it is
 * non-negative, is assumed to have been scaled to compensate for
 * the scaling of the values in df.
 *
 * The value returned in fun is an estimate of the true mean square
 * when var is non-negative, and is the generalized cross validation
 * when var is negative.
 */
{
  double e, f, g, h, rho1;
  int i;

/* Use p and q instead of rho to prevent overflow or underflow */
  rho1 = *rho + 1;
  *p = *rho / rho1;
  *q = 1 / rho1;
  if (fabs(rho1 - 1) < DEPS)
    *p = 0;
  if (fabs(rho1 - *rho) < DEPS)
    *q = 0;

/* Rational cholesky decomposition of p*c + q*t */
  f = g = h = 0;
  for (i = 0; i < 2; i++)
    R(i, 0) = 0;
  for (i = 2; i < *n; i++)
    {
      R(i - 2, 2) = g * R(i - 2, 0);
      R(i - 1, 1) = f * R(i - 1, 0);
      R(i, 0) = 1 / (*p * C(i - 1, 0) + *q * T(i, 0) - f * R(i - 1, 1) + g *
	R(i - 2, 2));
      f = (*p) * C(i - 1, 1) + (*q) * T(i, 1) - h * R(i - 1, 1);
      g = h;
      h = *p * C(i - 1, 2);
    }

/* Solve for u */
  u[0] = 0;
  u[1] = 0;
  for (i = 2; i < *n; i++)
    u[i] = a[i - 1] - R(i - 1, 1) * u[i - 1] - R(i - 2, 2) * u[i - 2];
  u[*n] = 0;
  u[*n + 1] = 0;
  for (i = *n - 1; i > 1; i--)
    u[i] = R(i, 0) * u[i] - R(i, 1) * u[i + 1] - R(i, 2) * u[i + 2];

/* Calculate residual vector v */
  e = 0;
  h = 0;
  for (i = 1; i < *n; i++)
    {
      g = h;
      h = (u[i + 1] - u[i]) / ((x[i] - x[i - 1]) / (*avh));
      v[i] = dy[i - 1] * (h - g);
      e += v[i] * v[i];
    }
  v[*n] = dy[*n - 1] * (-h);
  e += e * v[*n] * v[*n];

/* Calculate upper three bands of inverse matrix */
  R(*n, 0) = 0;
  R(*n, 1) = 0;
  R(*n + 1, 0) = 0;
  for (i = *n - 1; i > 1; i--)
    {
      g = R(i, 1);
      h = R(i, 2);
      R(i, 1) = -g * R(i + 1, 0) - h * R(i + 1, 1);
      R(i, 2) = -g * R(i + 1, 1) - h * R(i + 2, 0);
      R(i, 0) -= (g * R(i, 1) + h * R(i, 2));
    }

/* Calculate trace */
  f = g = h = 0;
  for (i = 2; i < *n; i++)
    {
      f += (R(i, 0) * C(i - 1, 0));
      g += (R(i, 1) * C(i - 1, 1));
      h += (R(i, 2) * C(i - 1, 2));
    }
  f += 2 * (g + h);

/* Calculate statistics */
  stat[0] = *p;
  stat[1] = f * (*p);
  stat[2] = *n * e / (f * f);
  stat[3] = e * (*p) * (*p) / (*n);
  stat[5] = *p * e / f;
  if (*var >= 0)
    {
      stat[4] = stat[3] - 2 * (*var) * stat[1] / (*n) + (*var);
      if (stat[4] < 0)
	stat[4] = 0;
      *fun = stat[4];
    }
  else
    {
      stat[4] = stat[5] - stat[3];
      *fun = stat[2];
    }
}

void cubgcv(
  double *x, double *f, double *df, int *n, double *y, double *c, int *ic,
  double *var, int *job, double *se, double *wk, int *ier)
{
  double delta, err, gf1, gf2, gf3, gf4, r1, r2, r3, r4, tau = 1.618033989;
  double ratio = 2.0, avh, avdf = 0.0, avar, stat[6], p, q;

  int done = 0, i;

/* Initialize */
  *ier = 133;
  if (*job >= 0 && *job <= 1)
    {
      spint1(x, &avh, f, df, &avdf, n, y, c, ic, wk, &WK(0, 3), ier);
      if (*ier == 0)
	{
	  avar = *var;
	  if (*var > 0)
	    avar = *var * avdf * avdf;

/* Check for zero variance */
	  if (fabs(*var) > DEPS)
	    {

/* Find local minimum of gcv or the expected mean square error */
	      r1 = 1;
	      r2 = ratio * r1;
	      spfit1(x, &avh, df, n, &r2, &p, &q, &gf2, &avar, stat, y, c, ic,
		     wk, &WK(0, 3), &WK(0, 5), &WK(0, 6));
	      do
		{
		  spfit1(x, &avh, df, n, &r1, &p, &q, &gf1, &avar,
			 stat, y, c, ic, wk, &WK(0, 3), &WK(0, 5), &WK(0, 6));
		  if (gf2 > gf1)
		    {
/* Exit if p zero */
		      if (p > 0)
			{
			  r2 = r1;
			  gf2 = gf1;
			  r1 /= ratio;
			}
		      else
			done = 1;
		    }
		}
	      while (gf2 > gf1 && p > 0);

	      if (!done)
		{
		  r3 = ratio * r2;
		  do
		    {
		      spfit1(x, &avh, df, n, &r3, &p, &q, &gf3, &avar, stat,
			     y, c, ic, wk, &WK(0, 3), &WK(0, 5), &WK(0, 6));
		      if (gf2 > gf3)
			{
/* Exit if q zero */
			  if (q > 0)
			    {
			      r2 = r3;
			      gf2 = gf3;
			      r3 *= ratio;
			    }
			  else
			    done = 1;
			}
		    }
		  while (gf2 > gf3 && q > 0);
		}

	      if (!done)
		{
		  r2 = r3;
		  gf2 = gf3;
		  delta = (r2 - r1) / tau;
		  r4 = r1 + delta;
		  r3 = r2 - delta;
		  spfit1(x, &avh, df, n, &r3, &p, &q, &gf3, &avar, stat,
			 y, c, ic, wk, &WK(0, 3), &WK(0, 5), &WK(0, 6));
		  spfit1(x, &avh, df, n, &r4, &p, &q, &gf4, &avar, stat,
			 y, c, ic, wk, &WK(0, 3), &WK(0, 5), &WK(0, 6));
		  do
		    {

/* Golden section search for local minimum */
		      if (gf3 > gf4)
			{
			  r1 = r3;
			  gf1 = gf3;
			  r3 = r4;
			  gf3 = gf4;
			  delta /= tau;
			  r4 = r1 + delta;
			  spfit1(x, &avh, df, n, &r4, &p, &q, &gf4, &avar, stat,
				 y, c, ic, wk, &WK(0, 3), &WK(0, 5), &WK(0, 6));
			}
		      else
			{
			  r2 = r4;
			  gf2 = gf4;
			  r4 = r3;
			  gf4 = gf3;
			  delta /= tau;
			  r3 = r2 - delta;
			  spfit1(x, &avh, df, n, &r3, &p, &q, &gf3, &avar, stat,
				 y, c, ic, wk, &WK(0, 3), &WK(0, 5), &WK(0, 6));
			}
		      err = (r2 - r1) / (r1 + r2);
		    }
		  while (err * err + 1 > 1 && err > 1E-6);
		  r1 = (r1 + r2) * 0.5;
		}
	    }
	  else
	    r1 = 0;

/* Calculate spline coefficients */
	  spfit1(x, &avh, df, n, &r1, &p, &q, &gf1, &avar, stat, y, c, ic,
		 wk, &WK(0, 3), &WK(0, 5), &WK(0, 6));
	  spcof1(x, &avh, f, df, n, &p, &q, y, c, ic, &WK(0, 5), &WK(0, 6));

/* Optionally calculate standard error estimates */
	  if (*var < 0)
	    {
	      avar = stat[5];
	      *var = avar / (avdf * avdf);
	    }
	  if (*job == 1)
	    sperr1(x, &avh, df, n, wk, &p, &avar, se);

/* Unscale df */
	  for (i = 0; i < *n; i++)
	    df[i] = df[i] * avdf;

/* Put statistics in wk */
	  for (i = 0; i < 6; i++)
	    WK(i, 0) = stat[i];
	  WK(5, 0) = stat[5] / (avdf * avdf);
	  WK(7, 0) = avdf * avdf;
	}
    }
}

void b_spline(int n, double *x, double *y, int m, double *sx, double *sy)
{
  double t, bl1, bl2, bl3, bl4;
  int i, j;
  double interval, xi_3, yi_3, xi, yi;

  interval = (double) (n - 1) / (double) (m);

  for (i = 2, j = 0; i <= n; i++)
    {
      if (i == 2)
	{
	  xi_3 = x[0] - (x[1] - x[0]);
	  yi_3 =
	    (y[1] * (xi_3 - x[0]) - y[0] * (xi_3 - x[1])) / (x[1] - x[0]);
	}
      else
	{
	  xi_3 = x[i - 3];
	  yi_3 = y[i - 3];
	}
      if (i == n)
	{
	  xi = x[n - 1] + (x[n - 1] - x[n - 2]);
	  yi =
	    (y[n - 1] * (xi - x[n - 2]) -
	     y[n - 2] * (xi - x[n - 1])) / (x[n - 1] - x[n - 2]);
	}
      else
	{
	  xi = x[i];
	  yi = y[i];
	}

      t = fmod(j * interval, 1.0);

      while (t < 1.0 && j < m)
	{
	  bl1 = (1.0 - t) * (1.0 - t) * (1.0 - t) / 6.0;
	  bl2 = (3.0 * t * t * t - 6.0 * t * t + 4.0) / 6.0;
	  bl3 = (-3.0 * t * t * t + 3.0 * t * t + 3.0 * t + 1.0) / 6.0;
	  bl4 = t * t * t / 6.0;

	  sx[j] = bl1 * xi_3 + bl2 * x[i - 2] + bl3 * x[i - 1] + bl4 * xi;
	  sy[j] = bl1 * yi_3 + bl2 * y[i - 2] + bl3 * y[i - 1] + bl4 * yi;

	  t += interval;
	  j++;
	}
    }
}
