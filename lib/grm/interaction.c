#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include <math.h>

#include "args_int.h"
#include "interaction_int.h"
#include "plot_int.h"


/* ######################### public implementation ################################################################## */

/* ========================= methods ================================================================================ */

/* ------------------------- user interaction ----------------------------------------------------------------------- */

int grm_input(const grm_args_t *input_args)
{
  /*
   * reset_ranges:
   * - `x`, `y`: mouse cursor position
   * - `key`: Pressed key (as string)
   * zoom:
   * - `x`, `y`: start point
   * - `angle_delta`: mouse wheel rotation angle in eighths of a degree, can be replaced by factor (double type)
   * - `factor`: zoom factor, can be replaced by angle_delta (double type)
   * box zoom:
   * - `x1`, `y1`, `x2`, `y2`: coordinates of a box selection, (x1, y1) is the fixed corner
   * - `keep_aspect_ratio`: if set to `1`, the aspect ratio of the gr window is preserved (defaults to `1`)
   * pan:
   * - `x`, `y`: start point
   * - `xshift`, `yshift`: shift in x and y direction
   *
   * All coordinates are expected to be given as workstation coordinates (integer type)
   */
  int width, height, max_width_height;
  int x, y, x1, y1, x2, y2;
  grm_args_t *subplot_args;
  const double *viewport;
  double viewport_mid_x, viewport_mid_y;

  logger((stderr, "Processing input\n"));

  get_figure_size(NULL, &width, &height, NULL, NULL);
  max_width_height = max(width, height);
  logger((stderr, "Using size (%d, %d)\n", width, height));

  if (args_values(input_args, "x", "i", &x) && args_values(input_args, "y", "i", &y))
    {
      double ndc_x, ndc_y;
      char *key;

      ndc_x = (double)x / max_width_height;
      ndc_y = (double)(height - y) / max_width_height;
      logger((stderr, "x: %d, y: %d, ndc_x: %lf, ndc_y: %lf\n", x, y, ndc_x, ndc_y));

      subplot_args = get_subplot_from_ndc_point(ndc_x, ndc_y);

      if (args_values(input_args, "key", "s", &key))
        {
          logger((stderr, "Got key \"%s\"\n", key));

          if (strcmp(key, "r") == 0)
            {
              if (subplot_args != NULL)
                {
                  logger((stderr, "Reset single subplot coordinate ranges\n"));
                  grm_args_push(subplot_args, "reset_ranges", "i", 1);
                }
              else
                {
                  grm_args_t **subplot_args_ptr;
                  logger((stderr, "Reset all subplot coordinate ranges\n"));
                  args_values(active_plot_args, "subplots", "A", &subplot_args_ptr);
                  while (*subplot_args_ptr != NULL)
                    {
                      grm_args_push(*subplot_args_ptr, "reset_ranges", "i", 1);
                      ++subplot_args_ptr;
                    }
                }
            }

          return 1;
        }

      if (subplot_args != NULL)
        {
          double angle_delta, factor;
          int xshift, yshift;
          args_values(subplot_args, "viewport", "D", &viewport);

          if (args_values(input_args, "angle_delta", "d", &angle_delta))
            {
              double focus_x, focus_y;

              viewport_mid_x = (viewport[0] + viewport[1]) / 2.0;
              viewport_mid_y = (viewport[2] + viewport[3]) / 2.0;
              focus_x = ndc_x - viewport_mid_x;
              focus_y = ndc_y - viewport_mid_y;
              logger((stderr, "Zoom to ndc focus point (%lf, %lf), angle_delta %lf\n", focus_x, focus_y, angle_delta));
              grm_args_push(subplot_args, "panzoom", "ddd", focus_x, focus_y,
                            1.0 - INPUT_ANGLE_DELTA_FACTOR * angle_delta);

              return 1;
            }
          else if (args_values(input_args, "factor", "d", &factor))
            {
              double focus_x, focus_y;

              viewport_mid_x = (viewport[0] + viewport[1]) / 2.0;
              viewport_mid_y = (viewport[2] + viewport[3]) / 2.0;
              focus_x = ndc_x - viewport_mid_x;
              focus_y = ndc_y - viewport_mid_y;
              logger((stderr, "Zoom to ndc focus point (%lf, %lf), factor %lf\n", focus_x, focus_y, factor));
              grm_args_push(subplot_args, "panzoom", "ddd", focus_x, focus_y, factor);

              return 1;
            }

          if (args_values(input_args, "xshift", "i", &xshift) && args_values(input_args, "yshift", "i", &yshift))
            {
              double ndc_xshift, ndc_yshift;

              ndc_xshift = (double)-xshift / max_width_height;
              ndc_yshift = (double)yshift / max_width_height;
              logger((stderr, "Translate by ndc coordinates (%lf, %lf)\n", ndc_xshift, ndc_yshift));
              grm_args_push(subplot_args, "panzoom", "ddd", ndc_xshift, ndc_yshift, 0.0);

              return 1;
            }
        }
    }

  if (args_values(input_args, "x1", "i", &x1) && args_values(input_args, "x2", "i", &x2) &&
      args_values(input_args, "y1", "i", &y1) && args_values(input_args, "y2", "i", &y2))
    {
      double focus_x, focus_y, factor_x, factor_y;
      int keep_aspect_ratio = INPUT_DEFAULT_KEEP_ASPECT_RATIO;

      args_values(input_args, "keep_aspect_ratio", "i", &keep_aspect_ratio);

      if (!get_focus_and_factor(x1, y1, x2, y2, keep_aspect_ratio, &factor_x, &factor_y, &focus_x, &focus_y,
                                &subplot_args))
        {
          return 0;
        }

      logger((stderr, "Got widget size: (%d, %d)\n", width, height));
      logger((stderr, "Got box: (%d, %d, %d, %d)\n", x1, y1, x2, y2));
      logger((stderr, "zoom focus: (%lf, %lf)\n", focus_x, focus_y));
      logger((stderr, "zoom factors: (%lf, %lf)\n", factor_x, factor_y));

      grm_args_push(subplot_args, "panzoom", "dddd", focus_x, focus_y, factor_x, factor_y);

      return 1;
    }

  return 0;
}

int grm_get_box(const int x1, const int y1, const int x2, const int y2, const int keep_aspect_ratio, int *x, int *y,
                int *w, int *h)
{
  int width, height, max_width_height;
  double focus_x, focus_y, factor_x, factor_y;
  double viewport_mid_x, viewport_mid_y;
  const double *viewport, *wswindow;
  grm_args_t *subplot_args;
  get_figure_size(NULL, &width, &height, NULL, NULL);
  max_width_height = max(width, height);
  if (!get_focus_and_factor(x1, y1, x2, y2, keep_aspect_ratio, &factor_x, &factor_y, &focus_x, &focus_y, &subplot_args))
    {
      return 0;
    }
  args_values(active_plot_args, "wswindow", "D", &wswindow);
  args_values(subplot_args, "viewport", "D", &viewport);
  viewport_mid_x = (viewport[1] + viewport[0]) / 2.0;
  viewport_mid_y = (viewport[3] + viewport[2]) / 2.0;
  *w = (int)round(factor_x * width * (viewport[1] - viewport[0]) / (wswindow[1] - wswindow[0]));
  *h = (int)round(factor_y * height * (viewport[3] - viewport[2]) / (wswindow[3] - wswindow[2]));
  *x = (int)round(((viewport_mid_x + focus_x) - ((viewport_mid_x + focus_x) - viewport[0]) * factor_x) *
                  max_width_height);
  *y = (int)round(height - ((viewport_mid_y + focus_y) - ((viewport_mid_y + focus_y) - viewport[3]) * factor_y) *
                               max_width_height);
  return 1;
}
