#if !defined(NO_AV) && !defined(NO_CAIRO)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#endif

#include "gks.h"
#include "gkscore.h"

#if !defined(NO_AV)
#include "vc.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32

#include <windows.h>
#ifndef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#endif

#endif

  DLLEXPORT void gks_videoplugin(int fctid, int dx, int dy, int dimx, int *i_arr, int len_f_arr_1, double *f_arr_1,
                                 int len_f_arr_2, double *f_arr_2, int len_c_arr, char *c_arr, void **ptr);

#ifdef __cplusplus
}
#endif

#if !defined(NO_AV) && !defined(NO_CAIRO)
static gks_state_list_t *gkss;

typedef struct ws_state_list_t
{
  char *path;
  char *mem_path;
  unsigned char *mem;
  long int width, height, framerate;
  int wtype;
  movie_t movie;
  void *cairo_ws_state_list;
  int video_plugin_initialized;
} ws_state_list;

static ws_state_list *p;

static void close_page(void)
{
  if ((p->wtype == 160 || p->wtype == 161 || p->wtype == 162) && p->movie)
    {
      vc_movie_finish(p->movie);
      free(p->movie);
      p->movie = NULL;
    }
}

static void write_page(void)
{
  int bg[3] = {255, 255, 255};
  int i, j, k;
  if (!p->movie)
    {
      char path[MAXPATHLEN];
      if (p->wtype == 160)
        {
          gks_filepath(path, p->path, "mp4", 0, 0);
        }
      else if (p->wtype == 161)
        {
          gks_filepath(path, p->path, "webm", 0, 0);
        }
      else if (p->wtype == 162)
        {
          gks_filepath(path, p->path, "ogg", 0, 0);
        }
      p->movie = vc_movie_create(path, p->framerate, 4000000);
    }
  frame_t frame = (frame_t)gks_malloc(sizeof(struct frame_t_));
  for (i = 0; i < p->height; i++)
    {
      for (j = 0; j < p->width; j++)
        {
          long ind = (i * p->width + j) * 4;
          double alpha = p->mem[ind + 3] / 255.0;
          for (k = 0; k < 3; k++)
            {
              double col = p->mem[ind + k] * alpha + bg[k] * (1 - alpha) + 0.5;
              if (col > 255)
                {
                  col = 255;
                }
              p->mem[ind + k] = (unsigned char)col;
            }
        }
    }
  frame->data = p->mem;
  frame->width = p->width;
  frame->height = p->height;
  if (p->movie)
    {
      vc_movie_append_frame(p->movie, frame);
    }
  gks_free(frame);
}

void gks_videoplugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                     char *chars, void **ptr)
{
  p = (ws_state_list *)*ptr;

  switch (fctid)
    {
    case 2:
      /* open workstation */

      gkss = (gks_state_list_t *)*ptr;

      p = (ws_state_list *)gks_malloc(sizeof(ws_state_list));
      p->cairo_ws_state_list = *ptr;
      p->video_plugin_initialized = 0;
      p->movie = NULL;
      p->mem = NULL;
      p->wtype = ia[2];
      p->path = chars;
      *ptr = p;

      long width, height, framerate, num_args;
      char *env;
      width = height = framerate = -1;
      env = (char *)gks_getenv("GKS_VIDEO_OPTS");
      if (env)
        {
          /* GKS_VIDEO_OPTS=<width>x<height>@<framerate> */
          num_args = sscanf(env, "%ldx%ld@%ld", &width, &height, &framerate);
          if (num_args == 0)
            {
              /* GKS_VIDEO_OPTS invalid */
              fprintf(stderr, "Failed to parse GKS_VIDEO_OPTS. Expected '<width>x<height>@<framerate>', "
                              "'<width>x<height>' or '<framerate>'\n");
              exit(1);
            }
          else if (num_args == 1)
            {
              /* GKS_VIDEO_OPTS=<framerate> */
              framerate = width;
              width = -1;
              height = -1;
            }
          else if (num_args == 2)
            {
              /* GKS_VIDEO_OPTS=<width>x<height> */
              framerate = -1;
            }
        }

      p->framerate = 24;
      p->width = 1024;
      p->height = 768;

      if (framerate > 0)
        {
          p->framerate = framerate;
        }
      if (width > 0)
        {
          p->width = width;
        }
      if (height > 0)
        {
          p->height = height;
        }

      p->mem_path = (char *)gks_malloc(MAXPATHLEN);
      p->mem = (unsigned char *)gks_malloc(p->width * p->height * 4 * sizeof(unsigned char));
      sprintf(p->mem_path, "!%ldx%ld@%p.mem", p->width, p->height, p->mem);
      chars = p->mem_path;
      /* set wstype for cairo png in memory */
      ia[2] = 143;

      p->video_plugin_initialized = 1;
      break;
    default:;
    }

  if (p && p->video_plugin_initialized)
    {
      gks_cairo_plugin(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, &(p->cairo_ws_state_list));
    }

  switch (fctid)
    {
    case 2:
      /* open workstation */
      if (p->mem_path)
        {
          gks_free(p->mem_path);
          p->mem_path = NULL;
        }
      break;
    case 3:
      /* close workstation */
      if (!p)
        {
          break;
        }
      p->video_plugin_initialized = 0;
      close_page();
      if (p->mem)
        {
          gks_free(p->mem);
        }
      gks_free(p);
      break;
    case 8:
      /* update workstation */
      if (ia[1] == GKS_K_PERFORM_FLAG)
        {
          write_page();
        }
      break;
    default:;
    }
}

#else

void gks_videoplugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                     char *chars, void **ptr)
{
  if (fctid == 2)
    {
      gks_perror("Video support not compiled in");
      ia[0] = 0;
    }
}

#endif
