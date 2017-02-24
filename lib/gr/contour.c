
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gks.h"
#include "gkscore.h"
#include "gr.h"
#include "contour.h"

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#define huge_value 1.7e+37F

#define contour_lines	    16		/* default number of contour lines */
#define contour_map_rate    0.5
#define contour_min_chh     0.005	/* minimum character height */
#define contour_max_length  1.2		/* maximum length of a labelled line */
#define contour_min_length  0.15	/* minimum length of a labelled line */
#define contour_max_pts     1000	/* maximum number of points */

enum contour_op
{
  CT_INIT, CT_ADD, CT_SUB, CT_EVAL, CT_END
};

typedef struct
{
  char lblfmt[10];
  int lblmjh;
  int txtflg;
  int use_color;
  int xdim, ydim;
  int wkid;
  int tnr, ndc;
  double *z;
  double scale_factor, aspect_ratio, vp[4], wn[4];
  double xmin, ymin, dx, dy;
  double zmin, zmax;
  int start_index;
  int end_index;
  double *gradient_mag;
  double *variance_list;
  int *label_map;
  int x_map_size, y_map_size;
  double x_map_factor, y_map_factor;
}
contour_vars_t;

static contour_vars_t contour_vars;

#define Z(i, j) (contour_vars.z[(i) + contour_vars.xdim*(j)])

typedef struct {
  int npoints;
  double *x;
  double *y;
} polyline_t;

static
int lookup_table[6][3][2] = {
  { {0, 1}, {0, 2} },
  { {0, 1}, {1, 2} },
  { {0, 2}, {1, 2} },
  { {0, 2}, {1, 2} },
  { {0, 1}, {1, 2} },
  { {0, 1}, {0, 2} }
};

static
char *xmalloc(int bytes)
{
  char *ptr;

  if ((ptr = (char *) malloc(bytes)) == NULL)
    {
      fprintf(stderr, "out of virtual memory\n");
      abort();
    }
  return ptr;
}

/*------------------------------------------------------------------------------
/ This gradient maintains a moving average of the magnitude of the gradient
/ vector at points along the contour line.  The idea behind this routine is that
/ at places where the magnitude is small, the contour lines will tend to be
/ farther apart thus allowing more room to write a label.
/-----------------------------------------------------------------------------*/

static
void gradient(int ind, int n, double *xpts, double *ypts, enum contour_op op)
{
  int i, j;
  static int count;
  double t;
  double xg1, yg1, xg2, yg2;
  double xgrad, ygrad;
  double txpt, typt;
  static double *magnitude = NULL;
  static double sum;
  static double max_mag;

  /* Since DRAW_CONTOURS is called with different values than     */
  /* xmin=ymin=0 and dx=dy=1, the computed contour-lines must be  */
  /* retransformed for gradient-calculations.                     */
  /* Otherwise, selected places for label could be wrong.         */

  txpt = (xpts[ind] - contour_vars.xmin) / contour_vars.dx;
  typt = (ypts[ind] - contour_vars.ymin) / contour_vars.dy;

  switch (op)
    {
    case CT_INIT:
      magnitude = (double *) xmalloc(n * sizeof(double));
      count = 0;
      sum = 0.0;
      max_mag = 0.0;
      break;

    case CT_ADD:
      i = (int) txpt;
      j = (int) typt;

      if (i == 0)
	xg1 = Z(i + 1, j) - Z(i, j);
      else if (i == contour_vars.xdim - 1)
	xg1 = Z(i, j) - Z(i - 1, j);
      else
	xg1 = (Z(i + 1, j) - Z(i - 1, j)) / 2.0;

      if (j == 0)
	yg1 = Z(i, j + 1) - Z(i, j);
      else if (j == contour_vars.ydim - 1)
	yg1 = Z(i, j) - Z(i, j - 1);
      else
	yg1 = (Z(i, j + 1) - Z(i, j - 1)) / 2.0;

      if (i == txpt)
	{
	  t = typt - j;
	  j++;
	}
      else
	{
	  t = txpt - i;
	  i++;
	}

      if (i == 0)
	xg2 = Z(i + 1, j) - Z(i, j);
      else if (i == contour_vars.xdim - 1)
	xg2 = Z(i, j) - Z(i - 1, j);
      else
	xg2 = (Z(i + 1, j) - Z(i - 1, j)) / 2.0;

      if (j == 0)
	yg2 = Z(i, j + 1) - Z(i, j);
      else if (j == contour_vars.ydim - 1)
	yg2 = Z(i, j) - Z(i, j - 1);
      else
	yg2 = (Z(i, j + 1) - Z(i, j - 1)) / 2.0;

      xgrad = xg1 + t * (xg2 - xg1);
      ygrad = yg1 + t * (yg2 - yg1);
      magnitude[ind] = xgrad * xgrad + ygrad * ygrad;
      sum += magnitude[ind];
      ++count;
      break;

    case CT_SUB:
      sum -= magnitude[ind];
      --count;
      break;

    case CT_EVAL:
      contour_vars.gradient_mag[ind] = sum / (double) count;
      if (contour_vars.gradient_mag[ind] > max_mag)
	max_mag = contour_vars.gradient_mag[ind];
      break;

    case CT_END:
      if (contour_vars.start_index != -1)
	{
	  ind = contour_vars.start_index - 1;
	  do
	    {
	      if (++ind >= n)
		ind = 1;
	      contour_vars.gradient_mag[ind] /= max_mag;
	    }
	  while (ind != contour_vars.end_index);
	}
      if (magnitude != NULL)
	free(magnitude);
      break;
    }
}

#undef Z

/*------------------------------------------------------------------------------
/ This routine calculates the variance of the points from a straight line.  The
/ idea behind this routine is that it is preferable to put a label on a straight
/ section than on a sharp bend.
/-----------------------------------------------------------------------------*/

static
void variance(int ind, int n, double *xpts, double *ypts, enum contour_op op)
{
  double y;
  double Sxx, Syy, Sxy;
  static double sigma_x, sigma_y, sigma_x2, sigma_y2, sigma_xy;
  static double max_var;
  static int count;

  switch (op)
    {
    case CT_INIT:
      sigma_x = sigma_y = sigma_x2 = sigma_y2 = sigma_xy = 0.0;
      max_var = 0.0;
      count = 0;
      break;

    case CT_ADD:
      y = ypts[ind];
      sigma_x += xpts[ind];
      sigma_y += y;
      sigma_x2 += xpts[ind] * xpts[ind];
      sigma_y2 += y * y;
      sigma_xy += xpts[ind] * y;
      ++count;
      break;

    case CT_SUB:
      y = ypts[ind];
      sigma_x -= xpts[ind];
      sigma_y -= y;
      sigma_x2 -= xpts[ind] * xpts[ind];
      sigma_y2 -= y * y;
      sigma_xy -= xpts[ind] * y;
      --count;
      break;

    case CT_EVAL:
      Sxx = sigma_x2 - sigma_x * sigma_x / count;
      Syy = sigma_y2 - sigma_y * sigma_y / count;
      Sxy = sigma_xy - sigma_x * sigma_y / count;
      if (Sxx >= Syy)
	contour_vars.variance_list[ind] = (Syy - Sxy * Sxy / Sxx) / count;
      else
	contour_vars.variance_list[ind] = (Sxx - Sxy * Sxy / Syy) / count;
      if (contour_vars.variance_list[ind] > max_var)
	max_var = contour_vars.variance_list[ind];
      break;

    case CT_END:
      if (contour_vars.start_index != -1)
	{
	  ind = contour_vars.start_index - 1;
	  do
	    {
	      if (++ind >= n)
		ind = 1;
	      contour_vars.variance_list[ind] /= max_var;
	    }
	  while (ind != contour_vars.end_index);
	}
      break;
    }
}

static
int find_good_place(int n, double *xpts, double *ypts, double r_sqr)
{
  int i, i_ind;
  int j, j_ind;
  int k;
  int closed;
  int map_xpos, map_ypos;
  double dx, dy, dx1, dy1;
  double dist, dist1;
  double min_t, t, min_var, var;
  unsigned short *ind;
  double r;

  contour_vars.gradient_mag = (double *) xmalloc(n * sizeof(double));
  contour_vars.variance_list = (double *) xmalloc(n * sizeof(double));
  ind = (unsigned short *) xmalloc(n * sizeof(short));

  contour_vars.start_index = -1;
  i = i_ind = 0;
  j = j_ind = 0;
  k = 0;
  ind[0] = 0xffff;
  closed = ((xpts[0] == xpts[n - 1]) && (ypts[0] == ypts[n - 1]));

  gradient(0, n, xpts, ypts, CT_INIT);
  variance(0, n, xpts, ypts, CT_INIT);
  gradient(0, n, xpts, ypts, CT_ADD);
  variance(0, n, xpts, ypts, CT_ADD);

  /*--------------------------------------------------------------------------
  / For each point on the line, find the variance and the average magnitude of
  / the gradient vector.
  /-------------------------------------------------------------------------*/
  while ((!closed && i < n - 1) ||
	 (closed && j_ind != contour_vars.start_index))
    {
      dx = xpts[i_ind] - xpts[j_ind];
      dy = ypts[i_ind] - ypts[j_ind];
      dist = dx * dx + dy * dy;
      while (dist < r_sqr)
	{
	  ++i;
	  if (++i_ind >= n)
	    {
	      if (!closed || k == 0)
		goto avg_done;	/* Not enough points for moving average */
	      i_ind = 1;
	    }
	  if (i_ind == k)
	    goto avg_done;
	  gradient(i_ind, n, xpts, ypts, CT_ADD);
	  variance(i_ind, n, xpts, ypts, CT_ADD);
	  dx = xpts[i_ind] - xpts[j_ind];
	  dy = ypts[i_ind] - ypts[j_ind];
	  dist = dx * dx + dy * dy;
	}

      ind[j_ind] = i;

      while (ind[k] < j)
	{
	  gradient(k, n, xpts, ypts, CT_SUB);
	  variance(k, n, xpts, ypts, CT_SUB);
	  if (++k >= n)
	    k = 1;
	}

      if (j >= ind[0])
	{
	  if (contour_vars.start_index == -1)
	    contour_vars.start_index = j_ind;
	  contour_vars.end_index = j_ind;
	  gradient(j_ind, n, xpts, ypts, CT_EVAL);
	  variance(j_ind, n, xpts, ypts, CT_EVAL);
	}

      ++j;
      if (++j_ind >= n)
	j_ind = 1;
    }

avg_done:
  gradient(0, n, xpts, ypts, CT_END);
  variance(0, n, xpts, ypts, CT_END);

  /*--------------------------------------------------------------------------
  / Find a point where to place the the label by minimizing variance
  / and gradient. (Variance is more important.)
  / The text must be neither outside the current viewport nor too close to
  / previous written labels.
  / Places in the middle of a line a preferred to those at a border.
  /-------------------------------------------------------------------------*/

  if ((k = contour_vars.start_index) != -1)
    {
      k = -1;
      r = sqrt(r_sqr);
      min_t = huge_value;
      min_var = huge_value;
      i = contour_vars.start_index - 1;
      do
	{
	  if (++i >= n)
	    i = 1;
	  t = contour_vars.gradient_mag[i] + contour_vars.variance_list[i] +
	    fabs(((double) i / (double) n) - 0.5);
	  var = contour_vars.variance_list[i];
	  map_xpos = (int) ((xpts[i] - contour_vars.xmin) *
	    contour_vars.x_map_factor) + 1;
	  map_ypos = (int) ((ypts[i] - contour_vars.ymin) *
	    contour_vars.y_map_factor) + 1;
	  dx = xpts[i] - xpts[0];
	  dy = (ypts[i] - ypts[0]) * contour_vars.aspect_ratio;
	  dist = dx * dx + dy * dy;
	  dx1 = xpts[i] - xpts[n - 1];
	  dy1 = (ypts[i] - ypts[n - 1]) * contour_vars.aspect_ratio;
	  dist1 = dx1 * dx1 + dy1 * dy1;
	  if ((t < 1.1 * min_t) &&
	      (var < 1.1 * min_var) &&
	      (xpts[i] - r > contour_vars.wn[0]) &&
	      (xpts[i] + r < contour_vars.wn[1]) &&
	      (ypts[i] - r > contour_vars.wn[2]) &&
	      (ypts[i] + r < contour_vars.wn[3]) &&
	      (dist > r_sqr) && (dist1 > r_sqr) &&
	      (contour_vars.label_map[map_xpos * contour_vars.x_map_size +
				      map_ypos] == 0))
	    {
	      min_t = t;
	      min_var = var;
	      k = i;
	    }
	}
      while (i != contour_vars.end_index);
    }

  if (k >= 0)
    {
      map_xpos = (int) ((xpts[k] - contour_vars.xmin) *
	contour_vars.x_map_factor) + 1;
      map_ypos = (int) ((ypts[k] - contour_vars.ymin) *
	contour_vars.y_map_factor) + 1;

      contour_vars.label_map
	[(map_xpos - 1) * contour_vars.x_map_size + map_ypos - 1] = 1;
      contour_vars.label_map
	[(map_xpos - 1) * contour_vars.x_map_size + map_ypos] = 1;
      contour_vars.label_map
	[(map_xpos - 1) * contour_vars.x_map_size + map_ypos + 1] = 1;
      contour_vars.label_map
	[map_xpos * contour_vars.x_map_size + map_ypos - 1] = 1;
      contour_vars.label_map
	[map_xpos * contour_vars.x_map_size + map_ypos] = 1;
      contour_vars.label_map
	[map_xpos * contour_vars.x_map_size + map_ypos + 1] = 1;
      contour_vars.label_map
	[(map_xpos + 1) * contour_vars.x_map_size + map_ypos - 1] = 1;
      contour_vars.label_map
	[(map_xpos + 1) * contour_vars.x_map_size + map_ypos] = 1;
      contour_vars.label_map
	[(map_xpos + 1) * contour_vars.x_map_size + map_ypos + 1] = 1;
    }

  free(ind);
  free(contour_vars.variance_list);
  free(contour_vars.gradient_mag);

  return (k);
}

static
void label_line(int n, double *xpts, double *ypts, double *zpts, char *label)
{
  int i, j, k;
  int error_ind;
  int n_pts;
  double dist;
  double r_sqr;
  double a, b, c;
  double ox, oy;
  double dx, dy, t;
  double xtpt1, ytpt1, xtpt2, ytpt2;
  double cpx, cpy, tx[4], ty[4];
  double x_up_val, y_up_val;
  double x_text_pos, y_text_pos;
  double d, e;

  /*--------------------------------------------------------------------------
  / Find out how large the label is so we will know how much room to leave
  / for it.
  /-------------------------------------------------------------------------*/

  x_up_val = 0.0;
  y_up_val = 1.0;
  gks_set_text_upvec(x_up_val, y_up_val);

  d = 0.0;
  e = 0.0;

  gks_select_xform(contour_vars.ndc);
  gks_inq_text_extent(contour_vars.wkid, d, e, label,
		      &error_ind, &cpx, &cpy, tx, ty);
  gks_select_xform(contour_vars.tnr);

  a = (tx[2] - tx[0]) / contour_vars.scale_factor;
  b = (ty[2] - ty[0]) / contour_vars.scale_factor;
  r_sqr = (a * a + b * b) / 4.0;	/* Gap in line reduced to half size */

  /*--------------------------------------------------------------------------
  / Try to find a good place to put the label on the contour line.
  /-------------------------------------------------------------------------*/

  k = find_good_place(n, xpts, ypts, r_sqr);

  if (k != -1)
    {
      /*----------------------------------------------------------------------
      / Find the first point outside of the circle centered at 'pts[k]'
      /---------------------------------------------------------------------*/

      i = k;
      do
	{
	  if (--i < 0)
	    {
	      i = n - 2;
	    }
	  dx = xpts[i] - xpts[k];
	  dy = (ypts[i] - ypts[k]) * contour_vars.aspect_ratio;
	  dist = dx * dx + dy * dy;
	}
      while (dist < r_sqr);

      /*----------------------------------------------------------------------
      / Find the intersection of line segment p[i]--p[i+1] with the circle
      /---------------------------------------------------------------------*/

      dx = xpts[i + 1] - xpts[i];
      dy = (ypts[i + 1] - ypts[i]) * contour_vars.aspect_ratio;
      ox = xpts[i] - xpts[k];
      oy = (ypts[i] - ypts[k]) * contour_vars.aspect_ratio;
      a = dx * dx + dy * dy;
      b = ox * dx + oy * dy;
      c = ox * ox + oy * oy - r_sqr;
      t = -(b + sqrt(b * b - a * c)) / a;
      xtpt1 = xpts[i] + t * dx;
      ytpt1 = ypts[i] + t * (ypts[i + 1] - ypts[i]);

      /*----------------------------------------------------------------------
      / Same as above but in the other direction
      /---------------------------------------------------------------------*/

      j = k;
      do
	{
	  if (++j >= n)
	    {
	      j = 1;
	    }
	  dx = xpts[j] - xpts[k];
	  dy = (ypts[j] - ypts[k]) * contour_vars.aspect_ratio;
	  dist = dx * dx + dy * dy;
	}
      while (dist < r_sqr);

      /*----------------------------------------------------------------------
      / Find the intersection of line segment p[j]--p[j-1] with the circle
      /---------------------------------------------------------------------*/

      dx = xpts[j - 1] - xpts[j];
      dy = (ypts[j - 1] - ypts[j]) * contour_vars.aspect_ratio;
      ox = xpts[j] - xpts[k];
      oy = (ypts[j] - ypts[k]) * contour_vars.aspect_ratio;
      a = dx * dx + dy * dy;
      b = ox * dx + oy * dy;
      c = ox * ox + oy * oy - r_sqr;
      t = -(b + sqrt(b * b - a * c)) / a;
      xtpt2 = xpts[j] + t * dx;
      ytpt2 = ypts[j] + t * (ypts[j - 1] - ypts[j]);

      /*----------------------------------------------------------------------
      / Calculate the character up vector.
      /---------------------------------------------------------------------*/

      x_up_val = (ytpt1 - ytpt2) * contour_vars.aspect_ratio;
      y_up_val = xtpt2 - xtpt1;
      if (y_up_val < 0.0)
	{
	  x_up_val = -x_up_val;
	  y_up_val = -y_up_val;
	}
      gks_set_text_upvec(x_up_val, y_up_val);

      x_text_pos = (xpts[k] - contour_vars.wn[0]) * contour_vars.scale_factor
	+ contour_vars.vp[0];
      y_text_pos = (ypts[k] - contour_vars.wn[2]) * contour_vars.scale_factor
	* contour_vars.aspect_ratio + contour_vars.vp[2];

      gks_select_xform(contour_vars.ndc);
      gks_text(x_text_pos, y_text_pos, label);
      gks_select_xform(contour_vars.tnr);

      /*---------------------------------------------------------------------/
      / Draw the contour line leaving a gap for the text.
      /---------------------------------------------------------------------*/

      if (i >= j)
	{
	  xpts[i + 1] = xtpt1;
	  ypts[i + 1] = ytpt1;
	  xpts[j - 1] = xtpt2;
	  ypts[j - 1] = ytpt2;
	  /* zpts remain the same */
	  n_pts = i - j + 3;
	  gr_polyline3d(n_pts, xpts + j - 1, ypts + j - 1, zpts + j - 1);
	}
      else
	{
	  xpts[i + 1] = xtpt1;
	  ypts[i + 1] = ytpt1;
	  n_pts = i + 2;
	  gr_polyline3d(n_pts, xpts, ypts, zpts);
	  xpts[j - 1] = xtpt2;
	  ypts[j - 1] = ytpt2;
	  n_pts = n - j + 1;
	  gr_polyline3d(n_pts, xpts + j - 1, ypts + j - 1, zpts + j - 1);
	}
    }
  else
    {
      n_pts = n;
      gr_polyline3d(n_pts, xpts, ypts, zpts);
    }
}

static
void draw(double x, double y, double z, int iflag)
{
  static int n = 0;
  static double xpts[contour_max_pts];
  static double ypts[contour_max_pts];
  static double zpts[contour_max_pts];
  static double line_length = 0;
  static int z_exept_flag = 0;
  double dx, dy;
  int linetype, colorind;
  char label[20];

  switch (iflag % 10)
    {
    case 1:			/* Continue polyline */
      if (z_exept_flag == 0)
	{
	  xpts[n] = x;
	  ypts[n] = y;
	  zpts[n] = z;
	  if ((contour_vars.txtflg == 1) &&
	      ((contour_vars.lblmjh == 1) ||
	       (((iflag / 10 - 1) % contour_vars.lblmjh) == 1)))
	    {
	      dx = xpts[n] - xpts[n - 1];
	      dy = (ypts[n] - ypts[n - 1]) * contour_vars.aspect_ratio;
	      line_length += contour_vars.scale_factor *
		sqrt(dx * dx + dy * dy);
	    }
	  n++;

	  if ((line_length >= contour_max_length) || (n >= contour_max_pts))
	    {
	      if ((contour_vars.txtflg == 1) &&
		  ((contour_vars.lblmjh == 1) ||
		   (((iflag / 10 - 1) % contour_vars.lblmjh) == 1)))
		{
		  linetype = GKS_K_LINETYPE_SOLID;
		  if (contour_vars.lblmjh > 1)
		    {
		      gks_set_pline_linetype(linetype);
		    }
		  sprintf(label, contour_vars.lblfmt, z);
		  label_line(n, xpts, ypts, zpts, label);
		}
	      else
		{
		  if ((contour_vars.lblmjh <= 1) ||
		      (((iflag / 10 - 1) % contour_vars.lblmjh) == 1))
		    {
		      linetype = GKS_K_LINETYPE_SOLID;
		    }
		  else
		    {
		      linetype = GKS_K_LINETYPE_DOTTED;
		    }
		  if (contour_vars.lblmjh > 1)
		    {
		      gks_set_pline_linetype(linetype);
		    }
                  if (contour_vars.use_color)
                    {
                      colorind = (int)(1000 + (z - contour_vars.zmin) /
                        (contour_vars.zmax - contour_vars.zmin) * 255);
                      gr_setlinecolorind(colorind);
                    }
		  gr_polyline3d(n, xpts, ypts, zpts);
		}
	      xpts[0] = x;
	      ypts[0] = y;
	      zpts[0] = z;
	      line_length = 0.0;
	      n = 1;
	    }
	}
      break;

    case 2:			/* New polyline */
    case 3:
      if ((z > contour_vars.zmin) && (z < contour_vars.zmax))
	{
	  z_exept_flag = 0;
	  xpts[0] = x;
	  ypts[0] = y;
	  zpts[0] = z;
	  line_length = 0.0;
	  n = 1;
	}
      else
	z_exept_flag = 1;
      break;

    case 4:			/* End polyline */
    case 5:
      if (z_exept_flag == 0)
	{
	  xpts[n] = x;
	  ypts[n] = y;
	  zpts[n] = z;
	  if ((contour_vars.txtflg == 1) &&
	      ((contour_vars.lblmjh == 1) ||
	       (((iflag / 10 - 1) % contour_vars.lblmjh) == 1)))
	    {
	      dx = xpts[n] - xpts[n - 1];
	      dy = (ypts[n] - ypts[n - 1]) * contour_vars.aspect_ratio;
	      line_length += contour_vars.scale_factor *
		sqrt(dx * dx + dy * dy);
	    }
	  n++;

	  if ((line_length >= contour_min_length) &&
	      (contour_vars.txtflg == 1) &&
	      ((contour_vars.lblmjh == 1) ||
	       (((iflag / 10 - 1) % contour_vars.lblmjh) == 1)))
	    {
	      linetype = GKS_K_LINETYPE_SOLID;
	      if (contour_vars.lblmjh > 1)
		{
		  gks_set_pline_linetype(linetype);
		}
	      sprintf(label, contour_vars.lblfmt, z);
	      label_line(n, xpts, ypts, zpts, label);
	    }
	  else
	    {
	      if ((contour_vars.lblmjh <= 1) ||
		  (((iflag / 10 - 1) % contour_vars.lblmjh) == 1))
		{
		  linetype = GKS_K_LINETYPE_SOLID;
		}
	      else
		{
		  linetype = GKS_K_LINETYPE_DOTTED;
		}
	      if (contour_vars.lblmjh > 1)
		{
		  gks_set_pline_linetype(linetype);
		}
              if (contour_vars.use_color)
                {
                  colorind = (int)(1000 + (z - contour_vars.zmin) /
                    (contour_vars.zmax - contour_vars.zmin) * 255);
                  gr_setlinecolorind(colorind);
                }
	      gr_polyline3d(n, xpts, ypts, zpts);
	    }
	}
      break;
    }
}

static
void calc_contours(
  double *z, int nrz, int nx, int ny, double *cv, int ncv, double zmax,
  int *bitmap, double xmin, double ymin, double dx, double dy)
{
/*
    This subroutine draws a contour through equal values of an array.
 
    *****     Formal arguments     ***********************************
 
    Z is the array for which contours are to be drawn.  The elements
    of Z are assumed to lie upon the nodes of a topologically
    rectangular coordinate system - e.g. cartesian, polar (except
    the origin), etc.

    The Z array is assumed to be organized in column-major order (as
    is in Fortran).
 
    NRZ is the number of rows declared for z in the calling program.

    NX is the limit for the first subscript of Z.
 
    NY is the limit for the second subscript of Z.
 
    CV are the values of the contours to be drawn.
 
    NCV is the number of contour values in CV.
 
    ZMAX is the maximum value of Z for consideration.  A value of
    Z[I,J] greater than ZMAX is a signal that that point and the
    grid line segments radiating from that point to it's neighbors
    are to be excluded from contouring.
 
    BITMAP is a work area large enough to hold NX*NY*NCV*2 bits.
 
    ******************************************************************
 
    DRAW is a subroutine used to draw contours.  The calling sequence
    for draw is:
 
       draw (x, y, z, iflag)

       Let NX = integer part of X, FX = fractional part of X.
       Then X should be interpreted such that increases in NX
       correspond to increases in the first subscript of Z, and
       FX is the fractional distance from the abscissa corresponding
       to NX to the abscissa corresponding to NX+1,
       and Y should be interpreted similarly for the second
       subscript of Z.

       The low-order digit of IFLAG will have one of the values:

           1 - continue a contour,
           2 - start a contour at a boundary,
           3 - start a contour not at a boundary,
           4 - finish a contour at a boundary,
           5 - finish a closed contour (not at a boundary).
               note that requests 1, 4 and 5 are for pen-down
               moves, and that requests 2 and 3 are for pen-up
               moves.
           6 - set X and Y to the approximate 'pen' position, using
               the notation discussed above.  this call may be
               ignored, the result being that the 'pen' position
               is taken to correspond to Z[1].

       IFLAG/10 is the contour number.
 
    L1 and L2 contain limits used during the spiral search for the
    beginning of a contour.
    IJ stores subcripts used during the spiral search.
 
    int i1[2], i2[2], i3[6]
 
    I1, I2 and I3 are used for subscript computations during the
    examination of lines from Z[I,J] to it's neighbors.
 
    double xint[4]
 
    XINT is used to mark intersections of the contour under
    consideration with the edges of the cell being examined.
  
    double xy[2]
 
    XY is used to compute coordinates for the draw subroutine.
 */

  static int l1[4] = { 0, 0, -1, -1 };
  static int i1[2] = { 1, 0 };
  static int i2[2] = { 1, -1 };
  static int i3[6] = { 1, 0, 0, 1, 1, 0 };

  double cval = 0, xint[4], z1, z2, zz;
  int idir, icur, jcur, jump, k, l, iedge, iflag = 0, ibkey;
  double xy[2];
  int ij[2], l2[4];
  int ii, jj, ni, ks = 0, ix, nxidir, icv = 0;

#define Z(i, j) z[(i) - 1 + ((j) - 1) * nrz]
#define BITMAP(i, j, k, l) \
  bitmap[(i) - 1 + ((j) - 1 + ((k) - 1 + ((l) - 1) * ncv) * ny) * nx]

  l1[0] = nx;
  l1[1] = ny;

/*  Set the current pen position.
    The default position corresponds to Z[1]. */

  xy[0] = 1.0;
  xy[1] = 1.0;
  icur = 1;
  jcur = 1;

/*  Clear the bitmap. */

  memset(bitmap, 0, nx * ny * ncv * 2 * sizeof(int));

/*  Search along a rectangular spiral path for a line segment having
    the following properties:

        1.  the end points are not excluded,
        2.  no mark has been recorded for the segment,
        3.  the values of z at the ends of the segment are such that 

            One Z is less than the current contour value, and the
            other is greater than or equal to the current contour
            value.

    Search all boundaries first, then search interior line segments.
    Note that the interior line segments near excluded points may be
    boundaries.
 */

  ibkey = 0;
L10:
  ij[0] = icur;
  ij[1] = jcur;
L20:
  l2[0] = ij[0];
  l2[1] = ij[1];
  l2[2] = -ij[0];
  l2[3] = -ij[1];
  idir = 0;

/*  Direction 0 is +I, 1 is +J, 2 is -I, 3 is -J. */

L30:
  nxidir = idir + 1;
  k = nxidir;
  if (nxidir > 3)
    {
      nxidir = 0;
    }
L40:
  ij[0] = abs(ij[0]);
  ij[1] = abs(ij[1]);
  if (Z(ij[0], ij[1]) > zmax)
    {
      goto L140;
    }
  l = 1;

/*  L=1 means horizontal line, L=2 means vertical line. */

L50:
  if (ij[l - 1] >= l1[l - 1])
    {
      goto L130;
    }
  ii = ij[0] + i1[l - 1];
  jj = ij[1] + i1[3 - l - 1];
  if (Z(ii, jj) > zmax)
    {
      goto L130;
    }
  jump = 100;

/*  The next 15 statements (or so) detect boundaries. */

L60:
  ix = 1;
  if (ij[3 - l - 1] == 1)
    {
      goto L80;
    }
  ii = ij[0] - i1[3 - l - 1];
  jj = ij[1] - i1[l - 1];
  if (Z(ii, jj) > zmax)
    {
      goto L70;
    }
  ii = ij[0] + i2[l - 1];
  jj = ij[1] + i2[3 - l - 1];
  if (Z(ii, jj) < zmax)
    {
      ix = 0;
    }
L70:
  if (ij[3 - l - 1] >= l1[3 - l - 1])
    {
      goto L90;
    }
L80:
  ii = ij[0] + i1[3 - l - 1];
  jj = ij[1] + i1[l - 1];
  if (Z(ii, jj) > zmax)
    {
      goto L90;
    }
  if (Z(ij[0] + 1, ij[1] + 1) < zmax)
    {
      goto L100;
    }
L90:
  ix += 2;
L100:
  if (jump == 280)
    {
      goto L280;
    }
  if (ix == 3)
    {
      goto L130;
    }
  if (ix + ibkey == 0)
    {
      goto L130;
    }

/*  Now determine whether the line segment is crossed by the contour. */

  ii = ij[0] + i1[l - 1];
  jj = ij[1] + i1[3 - l - 1];
  z1 = Z(ij[0], ij[1]);
  z2 = Z(ii, jj);
  for (icv = 1; icv <= ncv; ++icv)
    {
      if (BITMAP(ij[0], ij[1], icv, l) != 0)
	{
	  goto L120;
	}
      if (cv[icv - 1] <= min(z1, z2))
	{
	  goto L110;
	}
      if (cv[icv - 1] <= max(z1, z2))
	{
	  goto L190;
	}
    L110:
      BITMAP(ij[0], ij[1], icv, l) = 1;
    L120:
      ;
    }
L130:
  ++l;
  if (l <= 2)
    {
      goto L50;
    }
L140:
  l = idir % 2 + 1;
  ij[l - 1] = abs(ij[l - 1]) * (abs(l1[k - 1]) / l1[k - 1]);

/*  Lines from Z[I,J] to Z[I+1,J] and Z[I,J+1]
    are not satisfactory continue the spiral. */

L150:
  if (ij[l - 1] >= l1[k - 1])
    {
      goto L170;
    }
  ++ij[l - 1];
  if (ij[l - 1] > l2[k - 1])
    {
      goto L160;
    }
  goto L40;
L160:
  l2[k - 1] = ij[l - 1];
  idir = nxidir;
  goto L30;
L170:
  if (idir == nxidir)
    {
      goto L180;
    }
  ++nxidir;
  ij[l - 1] = l1[k - 1];
  k = nxidir;
  l = 3 - l;
  ij[l - 1] = l2[k - 1];
  if (nxidir > 3)
    {
      nxidir = 0;
    }
  goto L150;
L180:
  if (ibkey != 0)
    {
      return;
    }
  ibkey = 1;
  goto L10;

/*  An acceptable line segment has been found.
    Follow the contour until it either hits a boundary or closes.
 */

L190:
  iedge = l;
  cval = cv[icv - 1];
  if (ix != 1)
    {
      iedge += 2;
    }
  iflag = ibkey + 2;
  xint[iedge - 1] = (cval - z1) / (z2 - z1);
L200:
  xy[l - 1] = ij[l - 1] + xint[iedge - 1];
  xy[3 - l - 1] = ij[3 - l - 1];
  BITMAP(ij[0], ij[1], icv, l) = 1;
  draw(xmin + (xy[0] - 1.0) * dx, ymin + (xy[1] - 1.0) * dy, cval,
       iflag + 10 * icv);
  if (iflag < 4)
    {
      goto L210;
    }
  icur = ij[0];
  jcur = ij[1];
  goto L20;

/*  Continue a contour.  The edges are numbered clockwise with
    the bottom edge being edge number one.
 */

L210:
  ni = 1;
  if (iedge < 3)
    {
      goto L220;
    }
  ij[0] -= i3[iedge - 1];
  ij[1] -= i3[iedge + 1];
L220:
  for (k = 1; k <= 4; ++k)
    {
      if (k == iedge)
	{
	  goto L250;
	}
      ii = ij[0] + i3[k - 1];
      jj = ij[1] + i3[k];
      z1 = Z(ii, jj);
      ii = ij[0] + i3[k];
      jj = ij[1] + i3[k + 1];
      z2 = Z(ii, jj);
      if (cval <= min(z1, z2))
	{
	  goto L250;
	}
      if (cval > max(z1, z2))
	{
	  goto L250;
	}
      if (k == 1)
	{
	  goto L230;
	}
      if (k != 4)
	{
	  goto L240;
	}
    L230:
      zz = z1;
      z1 = z2;
      z2 = zz;
    L240:
      xint[k - 1] = (cval - z1) / (z2 - z1);
      ++ni;
      ks = k;
    L250:
      ;
    }
  if (ni == 2)
    {
      goto L260;
    }

/*  The contour crosses all four edges of the cell being examined.
    choose the lines top-to-left and bottom-to-right if the
    interpolation point on the top edge is less than the interpolation
    point on the bottom edge.  Otherwise, choose the other pair.  This
    method produces the same results if the axes are reversed.  The
    contour may close at any edge, but must not cross itself inside
    any cell.
 */

  ks = 5 - iedge;
  if (xint[2] < xint[0])
    {
      goto L260;
    }
  ks = 3 - iedge;
  if (ks <= 0)
    {
      ks += 4;
    }

/*  Determine whether the contour will close or run into a boundary
    at edge KS of the current cell.
 */

L260:
  l = ks;
  iflag = 1;
  jump = 280;
  if (ks < 3)
    {
      goto L270;
    }
  ij[0] += i3[ks - 1];
  ij[1] += i3[ks + 1];
  l = ks - 2;
L270:
  if (BITMAP(ij[0], ij[1], icv, l) == 0)
    {
      goto L60;
    }
  iflag = 5;
  goto L290;
L280:
  if (ix != 0)
    {
      iflag = 4;
    }
L290:
  iedge = ks + 2;
  if (iedge > 4)
    {
      iedge += -4;
    }
  xint[iedge - 1] = xint[ks - 1];
  goto L200;
}

#undef BITMAP
#undef Z

void gr_draw_contours(int nx, int ny, int nh, double *px, double *py, double *h,
		      double *z, int major_h)
{
  double mmin, mmax, *cv;
  int ncv, *bitmap;
  int i, j, k, n = 0;
  int precision, max_precision;
  char *s, buffer[80];
  int eflag, error_ind = 0;
  int rotation, tilt, scale_options;
  double char_height;

  gks_inq_open_ws(1, &error_ind, &n, &contour_vars.wkid);

  /* Inquire current transformation */
  contour_vars.ndc = 0;
  gks_inq_current_xformno(&error_ind, &contour_vars.tnr);
  gks_inq_xform(contour_vars.tnr, &error_ind, contour_vars.wn, contour_vars.vp);

  contour_vars.scale_factor =
    (contour_vars.vp[1] - contour_vars.vp[0]) /
    (contour_vars.wn[1] - contour_vars.wn[0]);
  contour_vars.aspect_ratio =
    (contour_vars.vp[3] - contour_vars.vp[2]) /
    (contour_vars.wn[3] - contour_vars.wn[2]) / contour_vars.scale_factor;

  contour_vars.xdim = nx;
  contour_vars.ydim = ny;
  contour_vars.lblmjh = major_h % 1000;
  contour_vars.label_map = NULL;
  contour_vars.use_color = major_h >= 1000;

  /* Don't label any lines if a 3D-transformation */
  /* or if any scale options are in effect.       */

  gr_inqscale(&scale_options);
  gr_inqspace(&contour_vars.zmin, &contour_vars.zmax, &rotation, &tilt);

  if ((rotation == 0) && (tilt == 90) &&
      (contour_vars.lblmjh > 0) && (scale_options == 0))
    {
      contour_vars.txtflg = 1;

      /* Compute size of 'label_map' with respect to      */
      /* viewport-size and character-height.              */
      /* 'label_map' is used to store information about   */
      /* previously drawn label positions to avoid        */
      /* overlapping                                      */

      gks_inq_text_height(&error_ind, &char_height);
      if (char_height < contour_min_chh)
	{
	  char_height = contour_min_chh;
	}

      contour_vars.x_map_size = (int) (contour_map_rate *
	(contour_vars.vp[1] - contour_vars.vp[0]) / char_height) + 2;
      contour_vars.y_map_size =	(int) (contour_map_rate *
	(contour_vars.vp[3] - contour_vars.vp[2]) / char_height) + 2;
      contour_vars.label_map = (int *) xmalloc(contour_vars.x_map_size *
	contour_vars.y_map_size * sizeof(int));

      k = 0;
      for (i = 0; i < contour_vars.x_map_size; i++)
	for (j = 0; j < contour_vars.y_map_size; j++)
	  {
	    contour_vars.label_map[k] = 0;
	    k++;
	  }
      contour_vars.x_map_factor =
	(double) (contour_vars.x_map_size - 3) /
	(contour_vars.wn[1] - contour_vars.wn[0]);
      contour_vars.y_map_factor =
	(double) (contour_vars.y_map_size - 3) /
	(contour_vars.wn[3] - contour_vars.wn[2]);
    }
  else
    {
      contour_vars.txtflg = 0;
    }

  mmin = huge_value;
  mmax = -huge_value;
  contour_vars.z = z;
  k = 0;
  for (j = 0; j < ny; j++)
    {
      for (i = 0; i < nx; i++)
	{
	  if (contour_vars.z[k] > mmax)
	    mmax = contour_vars.z[k];
	  else if (contour_vars.z[k] < mmin)
	    mmin = contour_vars.z[k];
	  k++;
	}
    }

  if (nh < 1)
    {
      ncv = contour_lines;
      cv = (double *) xmalloc(ncv * sizeof(double));
      for (i = 0; i < ncv; i++)
	cv[i] = mmin + (double) (i) / (ncv - 1) * (mmax - mmin);
    }
  else
    {
      ncv = nh;
      cv = h;
    }

  /*--------------------------------------------------------------------------
  / Find the maximum required precision for the labels and create the
  / appropriate format for 'sprintf'
  /-------------------------------------------------------------------------*/

  if (contour_vars.txtflg == 1)
    {
      max_precision = 0;
      eflag = 0;

      for (i = 0; i < ncv; i++)
	if ((contour_vars.txtflg == 1) &&
	    ((contour_vars.lblmjh == 1) || ((i % contour_vars.lblmjh) == 1)))
	  {
	    sprintf(buffer, "%g", cv[i]);
	    if ((s = (char *) strchr(buffer, '.')) != 0)
	      {
		precision = strspn(s + 1, "0123456789");
		if (*(s + 1 + precision) != '\0')
		  eflag = 1;
		if (precision > max_precision)
		  max_precision = precision;
	      }
	  }

      sprintf(contour_vars.lblfmt, "%%.%d%c", max_precision,
	      eflag ? 'e' : 'f');
    }

  bitmap = (int *) xmalloc(nx * ny * ncv * 2 * sizeof(int));
  contour_vars.xmin = px[0];
  contour_vars.ymin = py[0];
  contour_vars.dx = px[1] - contour_vars.xmin;
  contour_vars.dy = py[1] - contour_vars.ymin;

  calc_contours(contour_vars.z, nx, nx, ny, cv, ncv, mmax,
		bitmap, contour_vars.xmin, contour_vars.ymin,
		contour_vars.dx, contour_vars.dy);

  free(bitmap);

  if (contour_vars.label_map != NULL)
    free(contour_vars.label_map);
  if (cv != h)
    free(cv);
}

static
int get_lookup_table_index(int *triangle, double *z, double isolevel) {
  int index, i;

  index = 0;
  for (i = 0; i < 3; ++i) {
    index |= (z[triangle[i]] > isolevel ? 1 : 0) << i;
  }
  return index;
}

static
void interpolate_line_segment(double *x, double *y, double *z, int *triangle,
                              double isolevel, int *nlines, vertex_t *lines)
{
  int indices[2];
  int index, i, j;

  index = get_lookup_table_index(triangle, z, isolevel);
  if (index > 0 && index < 7) {
    for (i = 0; i < 2; ++i) {
      for (j = 0; j < 2; ++j) {
        indices[j] = triangle[lookup_table[index-1][i][j]];
      }
      lines[i].x = x[indices[0]] + (x[indices[1]] - x[indices[0]]) *
        ((isolevel - z[indices[0]]) / (z[indices[1]] - z[indices[0]]));
      lines[i].y = y[indices[0]] + (y[indices[1]] - y[indices[0]]) *
        ((isolevel - z[indices[0]]) / (z[indices[1]] - z[indices[0]]));
    }
    *nlines = 1;
  }
  else {
    *nlines = 0;
  }
}

static
void convert_segments_to_polylines(
  int nsegments, vertex_t *segments, int *nlines, polyline_t **lines) {
  polyline_t *lin;
  int nlin;
  polyline_t *cur_lin;
  vertex_t *points;
  static int start;
  static int end;
  static int capacity;
  static vertex_t *first_point;
  vertex_t **pointp;
  int *used_segments;
  int nfree_segments;
  int found_point;
  int line_segment_fits;
  int closed_segment;
  int i, j;

  lin = (polyline_t *) malloc(nsegments * sizeof(polyline_t));
  if (lin == NULL) {
    fprintf(stderr, "out of virtual memory\n");
    return;
  }
  nlin = 0;
  cur_lin = lin;
  points = (vertex_t *) malloc((nsegments + 2) * sizeof(vertex_t));
  if (points == NULL) {
    fprintf(stderr, "out of virtual memory\n");
    free(lin);
    return;
  }
  nlin = 0;
  start = -1;
  end = 0;
  capacity = nsegments + 2;
  first_point = NULL;
  used_segments = calloc(nsegments, sizeof(int));
  if (used_segments == NULL) {
    fprintf(stderr, "out of virtual memory\n");
    free(lin);
    free(points);
    return;
  }
  nfree_segments = nsegments;

  while (nfree_segments > 0) {
    /* each iteration generates a new polyline */
    found_point = 1;
    while (found_point) {
      /* each iteration adds a new point to the polyline */
      found_point = 0;
      for (i = 0; i < nsegments; i++) {
        /* each iteration tests if a line segment can be added to the current
           polyline */
        if (!used_segments[i]) {
          if (start >= 0) {
            /* the current line has already points */
            line_segment_fits = 0;
            for ((j = 0, pointp = &first_point); *pointp != NULL;
                 (j++, pointp++)) {
              if (fabs(segments[2*i].x - (*pointp)->x) < FEPS &&
                  fabs(segments[2*i].y - (*pointp)->y) < FEPS) {
                line_segment_fits = 1;
                closed_segment = (pointp == &first_point) ? 0 : 1;
                break;
              }
              else if (fabs(segments[2*i + 1].x - (*pointp)->x) < FEPS &&
                       fabs(segments[2*i + 1].y - (*pointp)->y) < FEPS) {
                line_segment_fits = 1;
                closed_segment = (pointp != &first_point) ? 0 : 1;
                break;
              }
            }
            if (line_segment_fits) {
              if (pointp == &first_point) {
                /* line segment fits at the beginning of the current polyline */
                points[start] = segments[2*i + (1-closed_segment)];
                first_point = &points[start];
                start = (start - 1 + capacity) % capacity;
              }
              else {
                /* line segment fits at the end of the current polyline */
                points[end] = segments[2*i + closed_segment];
                end = (end + 1 + capacity) % capacity;
              }
              used_segments[i] = 1;
              --nfree_segments;
              found_point = 1;
              break;
            }
          }
          else {
            /* empty polyline */
            start = (start + capacity) % capacity;
            points[start] = segments[2*i];
            points[end] = segments[2*i + 1];
            first_point = &points[start];
            start = (start - 1 + capacity) % capacity;
            end = (end + 1 + capacity) % capacity;
            used_segments[i] = 1;
            --nfree_segments;
            found_point = 1;
            break;
          }
        }
      }
    }
    cur_lin->npoints = 0;
    cur_lin->x = (double *) malloc((end-start + capacity-1) * sizeof(double));
    cur_lin->y = (double *) malloc((end-start + capacity-1) * sizeof(double));
    if (cur_lin->x == NULL || cur_lin->y == NULL) {
      fprintf(stderr, "out of virtual memory\n");
      free(cur_lin->x);
      free(cur_lin->y);
      return;
    }
    for (i = start+1; i != end; i = (i + 1) % capacity) {
      cur_lin->x[cur_lin->npoints] = points[i].x;
      cur_lin->y[cur_lin->npoints] = points[i].y;
      ++(cur_lin->npoints);
    }
    ++nlin;
    ++cur_lin;
    start = -1;
    end = 0;
  }

  lin = realloc(lin, nlin * sizeof(polyline_t));
  free(points);
  free(used_segments);

  *nlines = nlin;
  *lines = lin;
}

static
void march_triangles(
  double *x, double *y, double *z, int ntri, int *triangles,
  double isolevel, int *nlines, polyline_t **lines)
{
  int i, j;
  vertex_t cur_line[2];
  int cur_nlines;
  vertex_t *lin;
  double nlin;
  int cur_lin_index;

  lin = (vertex_t *) malloc(ntri * 2 * sizeof(vertex_t));
  if (lin == NULL) {
    fprintf(stderr, "out of virtual memory\n");
    return;
  }
  cur_lin_index = 0;
  for (i = 0; i < ntri; ++i) {
    interpolate_line_segment(x, y, z, &triangles[3*i],
                             isolevel, &cur_nlines, cur_line);
    for (j = 0; j < 2*cur_nlines; (j++, cur_lin_index++)) {
      lin[cur_lin_index].x = cur_line[j].x;
      lin[cur_lin_index].y = cur_line[j].y;
    }
  }
  lin = realloc(lin, cur_lin_index * sizeof(vertex_t));
  nlin = cur_lin_index / 2;

  convert_segments_to_polylines(nlin, lin, nlines, lines);
}

void gr_draw_tricont(int npoints, double *x, double *y, double *z,
                     int nlevels, double *levels, int *colors)
{
  int i, l;
  int ntri, *triangles;
  int nlines;
  polyline_t *lines;

  gr_delaunay(npoints, x, y, &ntri, &triangles);

  for (l = 0; l < nlevels; l++)
    {
      march_triangles(x, y, z, ntri, triangles, levels[l], &nlines, &lines);

      gr_setlinecolorind(colors[l]);
      for (i = 0; i < nlines; i++)
        gr_polyline(lines[i].npoints, lines[i].x, lines[i].y);

      free(lines);
    }
  free(triangles);
}

