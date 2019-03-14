/*

This code is based on Jonas Clever's bachelor thesis "Entwicklung eines
Verfahrens zur effizienten Visualisierung grosser Datenmengen im GR-Framework".

Link:
  https://pgi-jcns.fz-juelich.de/pub/doc/Bachelor/Bachelorarbeit_JonasClever.pdf

 */

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "gr.h"

#ifdef _WIN32
#define is_nan(a) _isnan(a)
#else
#define is_nan(a) isnan(a)
#endif

#define XFORM_BOOLEAN 0
#define XFORM_LINEAR 1
#define XFORM_LOG 2
#define XFORM_LOGLOG 3
#define XFORM_CUBIC 4
#define XFORM_EQUALIZED 5

static char *xcalloc(int count, int size)
{
  char *result = (char *)calloc(count, size);
  if (!result)
    {
      fprintf(stderr, "out of virtual memory\n");
      abort();
    }
  return (result);
}

static void rasterize(int n, double *x, double *y, double *roi, int w, int h, int *bins)
{
  double xl, xr, yb, yt;
  int i, ix, iy, num_bins = w * h;

  xl = roi[0];
  xr = roi[1];
  yb = roi[2];
  yt = roi[3];

  for (i = 0; i < num_bins; i++) bins[i] = 0;

  for (i = 0; i < n; i++)
    {
      if (x[i] >= roi[0] && x[i] <= roi[1] && y[i] >= roi[2] && y[i] <= roi[3])
        {
          ix = (int)((x[i] - xl) / (xr - xl) * (w - 1) + 0.5);
          iy = (int)((y[i] - yb) / (yt - yb) * (h - 1) + 0.5);
          bins[(h - iy - 1) * w + ix] += 1;
        }
    }
}

static void equalize(int w, int h, int *bins, int bmin, int bmax)
{
  int *hist, num_bins = w * h, i, *lut;
  double sum = 0, scale;

  hist = (int *)xcalloc(bmax + 1, sizeof(int));
  for (i = 0; i < num_bins; i++) hist[bins[i]] += 1;

  i = 0;
  while (hist[i] == 0 && i < bmax) i++;

  lut = (int *)xcalloc(bmax + 1, sizeof(int));
  scale = 255.0 / (num_bins - hist[i]);
  while (i < bmax)
    {
      i++;
      sum += hist[i];
      lut[i] = (int)(sum * scale);
    }

  for (i = 0; i < num_bins; i++) bins[i] = lut[bins[i]];

  free(lut);
  free(hist);
}

static void shade(int w, int h, int *bins, int xform)
{
  int num_bins = w * h, bmin, bmax, i;

  bmin = INT32_MAX;
  bmax = -INT32_MAX;

  for (i = 0; i < num_bins; i++)
    {
      if (bins[i] > bmax)
        bmax = bins[i];
      else if (bins[i] < bmin)
        bmin = bins[i];
    }

  if (xform == XFORM_EQUALIZED) /* equalize */
    {
      equalize(w, h, bins, bmin, bmax);
    }
  else
    {
      for (i = 0; i < num_bins; i++)
        {
          if (xform == XFORM_BOOLEAN) /* boolean */
            bins[i] = bins[i] > 0 ? 255 : 0;
          else if (xform == XFORM_LINEAR) /* linear */
            bins[i] = (int)((double)(bins[i] - bmin) / (bmax - bmin) * 255);
          else if (xform == XFORM_LOG) /* log */
            bins[i] = (int)(log1p(bins[i] - bmin) / log1p(bmax - bmin) * 255);
          else if (xform == XFORM_LOGLOG) /* loglog */
            bins[i] = (int)(log1p(log1p(bins[i] - bmin)) / log1p(log1p(bmax - bmin)) * 255);
          else if (xform == XFORM_CUBIC) /* cubic */
            bins[i] = (int)(pow(bins[i], 0.3) / pow(bmax - bmin, 0.3) * 255);
        }
    }

  for (i = 0; i < num_bins; i++) bins[i] += 1000;
}

static void lineLow(int x0, int y0, int x1, int y1, int w, int h, int *bins)
{
  int dx = x1 - x0;
  int dy = y1 - y0;
  int yi = 1;
  int y = y0;
  int D, x;

  if (dy < 0)
    {
      yi = -1;
      dy = -dy;
    }
  D = 2 * dy - dx;

  for (x = x0; x <= x1; x++)
    {
      bins[(h - y - 1) * w + x] += 1;
      if (D > 0)
        {
          y += yi;
          D -= 2 * dx;
        }
      D += 2 * dy;
    }
}

static void lineHigh(int x0, int y0, int x1, int y1, int w, int h, int *bins)
{
  int dx = x1 - x0;
  int dy = y1 - y0;
  int xi = 1;
  int x = x0;
  int D, y;

  if (dx < 0)
    {
      xi = -1;
      dx = -dx;
    }
  D = 2 * dx - dy;

  for (y = y0; y <= y1; y++)
    {
      bins[(h - y - 1) * w + x] += 1;
      if (D > 0)
        {
          x += xi;
          D -= 2 * dy;
        }
      D += 2 * dx;
    }
}

static void line(int x0, int y0, int x1, int y1, int w, int h, int *bins)
{
  if (abs(y1 - y0) < abs(x1 - x0))
    {
      if (x0 > x1)
        lineLow(x1, y1, x0, y0, w, h, bins);
      else
        lineLow(x0, y0, x1, y1, w, h, bins);
    }
  else
    {
      if (y0 > y1)
        lineHigh(x1, y1, x0, y0, w, h, bins);
      else
        lineHigh(x0, y0, x1, y1, w, h, bins);
    }
}

void gr_shade(int n, double *x, double *y, int lines, int xform, double *roi, int w, int h, int *bins)
{
  double xl, xr, yb, yt;
  int i, j, x0, y0, x1, y1, num_bins = w * h;

  if (lines == 1)
    {
      xl = roi[0];
      xr = roi[1];
      yb = roi[2];
      yt = roi[3];

      for (i = 0; i < num_bins; i++) bins[i] = 0;

      i = 0;
      while (i < n)
        {
          j = i + 1;
          while (j < n && !is_nan(x[j]) && !is_nan(y[j])) j++;
          j--;

          while (i < j)
            {
              if (x[i] >= xl && x[i] <= xr && y[i] >= yb && y[i] <= yt && x[i + 1] >= xl && x[i + 1] <= xr &&
                  y[i + 1] >= yb && y[i + 1] <= yt)
                {
                  x0 = (int)((x[i] - xl) / (xr - xl) * (w - 1) + 0.5);
                  y0 = (int)((y[i] - yb) / (yt - yb) * (h - 1) + 0.5);
                  x1 = (int)((x[i + 1] - xl) / (xr - xl) * (w - 1) + 0.5);
                  y1 = (int)((y[i + 1] - yb) / (yt - yb) * (h - 1) + 0.5);
                  line(x0, y0, x1, y1, w, h, bins);
                }
              i++;
            }

          i += 2;
        }
    }
  else
    {
      rasterize(n, x, y, roi, w, h, bins);
    }

  shade(w, h, bins, xform);
}
