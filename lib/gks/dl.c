
#include <string.h>
#include <stdlib.h>

#include "gks.h"
#include "gkscore.h"

#define SEGM_SIZE 262144 /* 256K */

#define COPY(s, n)                              \
  memmove(d->buffer + d->nbytes, (void *)s, n); \
  d->nbytes += n
#define PAD(n)                         \
  memset(d->buffer + d->nbytes, 0, n); \
  d->nbytes += n
#define RESOLVE(arg, type, nbytes) \
  arg = (type *)(s + sp);          \
  sp += nbytes

#define static_assert(cond, message) ((void)sizeof(char[(cond) ? 1 : -1]))

#ifndef GKS_UNUSED
#define GKS_UNUSED(x) (void)(x)
#endif

static void reallocate(gks_display_list_t *d, int len)
{
  while (d->nbytes + len > d->size) d->size += SEGM_SIZE;

  d->buffer = (char *)gks_realloc(d->buffer, d->size + 1);
}

static int purge(gks_display_list_t *d, char *t)
/*
   Clear display list preserving workstation specific functions.
   Return purged display list (t) and length (in bytes)
 */
{
  char *s;
  int i;
  int sp = 0, tp = 0, *len, fctid;
  static const char *attribute_buffer[MAX_ATTRIBUTE_FCTID + 1];
  static const char *color_buffer[MAX_COLOR];
  memset(attribute_buffer, 0, sizeof(char *) * (MAX_ATTRIBUTE_FCTID + 1));
  memset(color_buffer, 0, sizeof(char *) * MAX_COLOR);

  s = d->buffer;
  len = (int *)(s + sp);
  while (*len)
    {
      fctid = *(int *)(s + sp + sizeof(int));
      switch (fctid)
        {
        case 48: /* setcolorrep */
          {
            int colorind = *(int *)(s + sp + 2 * sizeof(int));
            if (colorind >= 0 && colorind < MAX_COLOR)
              {
                color_buffer[colorind] = s + sp;
              }
          }
          break;
        case 54: /* setwswindow */
        case 55: /* setwsviewport */
          attribute_buffer[fctid] = s + sp;
          break;
        default:
          break;
        }
      sp += *len;
      len = (int *)(s + sp);
    }
  for (i = 0; i < MAX_COLOR; i++)
    {
      if (color_buffer[i])
        {
          len = (int *)(color_buffer[i]);
          memmove(t + tp, color_buffer[i], *len);
          tp += *len;
        }
    }
  for (i = 0; i <= MAX_ATTRIBUTE_FCTID; i++)
    {
      if (attribute_buffer[i])
        {
          len = (int *)(attribute_buffer[i]);
          memmove(t + tp, attribute_buffer[i], *len);
          tp += *len;
        }
    }
  return tp;
}

/*
 * Debug function to print the displaylist
 */
void printdl(char *d, int fctid)
{
  char *cur_pos = d;
  int sublen = ((int *)cur_pos)[0];
  while (sublen != 0)
    {
      int cur_fctid = ((int *)cur_pos)[1];
      cur_pos += 2 * sizeof(int);
      if (cur_fctid == fctid)
        {
          switch (cur_fctid)
            {
            case GKS_SET_BBOX_CALLBACK:
              printf("BEGIN SELECTION %d\n", ((int *)cur_pos)[0]);
              break;
            case GKS_CANCEL_BBOX_CALLBACK:
              {
                double *bbox = (double *)((int *)cur_pos + 1);
                printf("END SELECTION %d with %f %f %f %f\n", ((int *)cur_pos)[0], bbox[0], bbox[1], bbox[2], bbox[3]);
              }
              break;
            default:
              break;
            }
        }
      cur_pos += sublen - 2 * sizeof(int);
      sublen = ((int *)cur_pos)[0];
    }
}

void gks_dl_write_item(gks_display_list_t *d, int fctid, int dx, int dy, int dimx, int *i_arr, int len_f_arr_1,
                       double *f_arr_1, int len_f_arr_2, double *f_arr_2, int len_c_arr, char *c_arr,
                       gks_state_list_t *gkss)
{
  char s[GKS_K_TEXT_MAX_SIZE], *t = NULL;
  int len = -1, slen, tp = 0;

  GKS_UNUSED(len_f_arr_1);
  GKS_UNUSED(len_f_arr_2);
  GKS_UNUSED(len_c_arr);

  switch (fctid)
    {
    case 2: /* open workstation */

      d->state = GKS_K_WS_INACTIVE;
      d->buffer = (char *)gks_malloc(SEGM_SIZE + 1);
      d->size = SEGM_SIZE;
      d->nbytes = d->position = 0;
      d->empty = 1;

      len = 2 * sizeof(int) + sizeof(gks_state_list_t) + 3 * sizeof(int);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(gkss, sizeof(gks_state_list_t));
      COPY(i_arr, 3 * sizeof(int));
      break;

    case 3: /* close workstation */

      free(d->buffer);
      d->buffer = NULL;
      break;

    case 4: /* activate workstation */

      d->state = GKS_K_WS_ACTIVE;
      break;

    case 5: /* deactivate workstation */

      d->state = GKS_K_WS_INACTIVE;
      break;

    case 6: /* clear workstation */

      t = gks_malloc(d->size);
      tp = purge(d, t);
      d->nbytes = d->position = 0;

      len = 2 * sizeof(int) + sizeof(gks_state_list_t) + 3 * sizeof(int);
      fctid = 2;

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(gkss, sizeof(gks_state_list_t));
      COPY(i_arr, 3 * sizeof(int));

      COPY(t, tp);
      free(t);
      break;

    case 12: /* polyline */
    case 13: /* polymarker */
    case 15: /* fill area */
      if (d->state == GKS_K_WS_ACTIVE)
        {
          len = 3 * sizeof(int) + 2 * i_arr[0] * sizeof(double);
          if (d->nbytes + len > d->size) reallocate(d, len);

          COPY(&len, sizeof(int));
          COPY(&fctid, sizeof(int));
          COPY(i_arr, sizeof(int));
          COPY(f_arr_1, i_arr[0] * sizeof(double));
          COPY(f_arr_2, i_arr[0] * sizeof(double));

          d->empty = 0;
        }
      break;

    case 14: /* text */

      if (d->state == GKS_K_WS_ACTIVE)
        {
          len = 3 * sizeof(int) + 2 * sizeof(double) + GKS_K_TEXT_MAX_SIZE;
          if (d->nbytes + len > d->size) reallocate(d, len);

          memset((void *)s, 0, GKS_K_TEXT_MAX_SIZE);
          slen = strlen(c_arr);
          memcpy(s, c_arr, slen < GKS_K_TEXT_MAX_SIZE ? slen : GKS_K_TEXT_MAX_SIZE - 1);

          COPY(&len, sizeof(int));
          COPY(&fctid, sizeof(int));
          COPY(f_arr_1, sizeof(double));
          COPY(f_arr_2, sizeof(double));
          COPY(&slen, sizeof(int));
          COPY(s, GKS_K_TEXT_MAX_SIZE);

          d->empty = 0;
        }
      break;

    case 16:  /* cell array */
    case 201: /* draw image */

      if (d->state == GKS_K_WS_ACTIVE)
        {
          len = (5 + dimx * dy) * sizeof(int) + 4 * sizeof(double);
          if (d->nbytes + len > d->size) reallocate(d, len);

          COPY(&len, sizeof(int));
          COPY(&fctid, sizeof(int));
          COPY(f_arr_1, 2 * sizeof(double));
          COPY(f_arr_2, 2 * sizeof(double));
          COPY(&dx, sizeof(int));
          COPY(&dy, sizeof(int));
          COPY(&dimx, sizeof(int));
          tp = dimx * (dy - 1) + dx;
          COPY(i_arr, tp * sizeof(int));
          PAD((dimx - dx) * sizeof(int)); /* (dimx * dy - tp) elements */

          d->empty = 0;
        }
      break;

    case 17: /* GDP */
      if (d->state == GKS_K_WS_ACTIVE)
        {
          len = (2 + 3 + i_arr[2]) * sizeof(int) + 2 * i_arr[0] * sizeof(double);
          if (d->nbytes + len > d->size) reallocate(d, len);

          COPY(&len, sizeof(int));
          COPY(&fctid, sizeof(int));
          COPY(i_arr, (3 + i_arr[2]) * sizeof(int));
          COPY(f_arr_1, i_arr[0] * sizeof(double));
          COPY(f_arr_2, i_arr[0] * sizeof(double));

          d->empty = 0;
        }
      break;

    case 19:  /* set linetype */
    case 21:  /* set polyline color index */
    case 23:  /* set markertype */
    case 25:  /* set polymarker color index */
    case 30:  /* set text color index */
    case 33:  /* set text path */
    case 36:  /* set fillarea interior style */
    case 37:  /* set fillarea style index */
    case 38:  /* set fillarea color index */
    case 52:  /* select normalization transformation */
    case 53:  /* set clipping indicator */
    case 108: /* set resample method */
    case 207: /* set border color index */
    case 208: /* select clipping transformation */
    case 211: /* set clip region */

      len = 3 * sizeof(int);
      if (d->nbytes + len > d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(i_arr, sizeof(int));
      break;

    case 27: /* set text font and precision */
    case 34: /* set text alignment */

      len = 4 * sizeof(int);
      if (d->nbytes + len > d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(i_arr, 2 * sizeof(int));
      break;

    case 20:  /* set linewidth scale factor */
    case 24:  /* set marker size scale factor */
    case 28:  /* set character expansion factor */
    case 29:  /* set character spacing */
    case 31:  /* set character height */
    case 109: /* set nominal size */
    case 200: /* set text slant */
    case 203: /* set transparency */
    case 206: /* set border width */

      len = 2 * sizeof(int) + sizeof(double);
      if (d->nbytes + len > d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(f_arr_1, sizeof(double));
      break;

    case 32:  /* set character up vector */
    case 212: /* set clip sector */

      len = 2 * sizeof(int) + 2 * sizeof(double);
      if (d->nbytes + len > d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(f_arr_1, sizeof(double));
      COPY(f_arr_2, sizeof(double));
      break;

    case 41: /* set aspect source flags */

      len = 2 * sizeof(int) + 13 * sizeof(int);
      if (d->nbytes + len > d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(i_arr, 13 * sizeof(int));
      break;

    case 48: /* set color representation */

      len = 3 * sizeof(int) + 3 * sizeof(double);
      if (d->nbytes + len > d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(&i_arr[1], sizeof(int));
      COPY(f_arr_1, 3 * sizeof(double));
      break;

    case 49: /* set window */
    case 50: /* set viewport */
    case 54: /* set workstation window */
    case 55: /* set workstation viewport */

      len = 3 * sizeof(int) + 4 * sizeof(double);
      if (d->nbytes + len > d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(i_arr, sizeof(int));
      COPY(f_arr_1, 2 * sizeof(double));
      COPY(f_arr_2, 2 * sizeof(double));
      break;

    case 202: /* set shadow */

      len = 2 * sizeof(int) + 3 * sizeof(double);
      if (d->nbytes + len >= d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(f_arr_1, 3 * sizeof(double));
      break;

    case 204: /* set coord xform */

      len = 2 * sizeof(int) + 6 * sizeof(double);
      if (d->nbytes + len >= d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(f_arr_1, 6 * sizeof(double));
      break;

    case 250: /* begin selection */

      len = 2 * sizeof(int) + 2 * sizeof(int);
      if (d->nbytes + len >= d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(i_arr, 2 * sizeof(int));
      break;

    case 251: /* end selection */

      len = 2 * sizeof(int);
      if (d->nbytes + len >= d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      break;

    case 252: /* move selection */

      len = 2 * sizeof(int) + 2 * sizeof(double);
      if (d->nbytes + len >= d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(f_arr_1, sizeof(double));
      COPY(f_arr_2, sizeof(double));
      break;

    case 260: /* set bbox callback */

      len = 3 * sizeof(int) + 2 * sizeof(void(*));
      if (d->nbytes + len >= d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(&i_arr[0], sizeof(int));
      static_assert(sizeof(double *) == sizeof(void (*)(int, double, double, double, double)),
                    "sizeof(double *) != sizeof(void(*)(int,double,double,double,double) on this architecture");
      static_assert(
          sizeof(double *) == sizeof(void (*)(unsigned int, unsigned int, unsigned int *)),
          "sizeof(double *) != sizeof(void (*)(unsigned int, unsigned int, unsigned int *) on this architecture");
      COPY(&f_arr_1, sizeof(void(*)));
      COPY(&f_arr_2, sizeof(void(*)));
      break;

    case 261: /* cancel bbox callback */
    case 262: /* set background */
    case 263: /* clear background */

      len = 2 * sizeof(int);
      if (d->nbytes + len >= d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      break;

    case 264: /* begin partial drawing */

      len = 3 * sizeof(int) + sizeof(void(*));
      if (d->nbytes + len >= d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(&i_arr[0], sizeof(int));
      static_assert(
          sizeof(double *) == sizeof(void (*)(int, double, double, double, double)),
          "sizeof(double *) != sizeof(void (*)(int, unsigned int, unsigned int, unsigned int *) on this architecture");
      COPY(&f_arr_1, sizeof(void(*)));
      break;

    case 265: /* end partial drawing */

      len = 3 * sizeof(int);
      if (d->nbytes + len >= d->size) reallocate(d, len);

      COPY(&len, sizeof(int));
      COPY(&fctid, sizeof(int));
      COPY(&i_arr[0], sizeof(int));
      break;
    }

  if (d->buffer != NULL)
    {
      if (d->nbytes + 4 > d->size) reallocate(d, 4);

      memset(d->buffer + d->nbytes, 0, 4);
    }
}


int gks_dl_read_item(char *dl, gks_state_list_t **gkss,
                     void (*fn)(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2,
                                int lc, char *chars, void **ptr))
{
  int sp = 0;
  int null_val = 0, i;
  char *s = dl;
  int *ia = NULL, *tmp;
  double *r1 = NULL, *r2 = NULL;
  char *chars = NULL;
  gks_state_list_t *sl;
  int *fctid, *dx = &null_val, *dy = &null_val, *dimx = &null_val, *lc = &null_val;

  RESOLVE(fctid, int, sizeof(int));
  switch (*fctid)
    {
    case 2: /* open workstation */
      RESOLVE(sl, gks_state_list_t, sizeof(gks_state_list_t));
      memcpy(*gkss, sl, sizeof(gks_state_list_t));
      RESOLVE(ia, int, 3 * sizeof(int));
      break;

    case 6: /* clear workstation */
      RESOLVE(sl, gks_state_list_t, sizeof(gks_state_list_t));
      memcpy(*gkss, sl, sizeof(gks_state_list_t));
      break;

    case 12: /* polyline */
    case 13: /* polymarker */
    case 15: /* fill area */
      RESOLVE(ia, int, sizeof(int));
      RESOLVE(r1, double, ia[0] * sizeof(double));
      RESOLVE(r2, double, ia[0] * sizeof(double));
      break;

    case 14: /* text */
      RESOLVE(r1, double, sizeof(double));
      RESOLVE(r2, double, sizeof(double));
      RESOLVE(lc, int, sizeof(int));
      RESOLVE(chars, char, GKS_K_TEXT_MAX_SIZE);
      break;

    case 16:  /* cell array */
    case 201: /* draw image */
      RESOLVE(r1, double, 2 * sizeof(double));
      RESOLVE(r2, double, 2 * sizeof(double));
      RESOLVE(dx, int, sizeof(int));
      RESOLVE(dy, int, sizeof(int));
      RESOLVE(dimx, int, sizeof(int));
      RESOLVE(ia, int, (*dimx * (*dy - 1) + *dx) * sizeof(int));
      sp += (*dimx - *dx) * sizeof(int); /* undo PAD */
      break;

    case 17: /* GDP */
      tmp = (int *)(s + sp);
      RESOLVE(ia, int, (3 + tmp[2]) * sizeof(int));
      RESOLVE(r1, double, tmp[0] * sizeof(double));
      RESOLVE(r2, double, tmp[0] * sizeof(double));
      break;

    case 19:  /* set linetype */
    case 21:  /* set polyline color index */
    case 23:  /* set markertype */
    case 25:  /* set polymarker color index */
    case 30:  /* set text color index */
    case 33:  /* set text path */
    case 36:  /* set fillarea interior style */
    case 37:  /* set fillarea style index */
    case 38:  /* set fillarea color index */
    case 52:  /* select normalization transformation */
    case 53:  /* set clipping indicator */
    case 108: /* set resample method */
    case 207: /* set border color index */
    case 208: /* select clipping transformation */
    case 211: /* set clip region */
    case 265: /* end partial drawing */
      RESOLVE(ia, int, sizeof(int));
      break;

    case 260:                                         /* set bbox callback */
      RESOLVE(ia, int, sizeof(int));                  /* id */
      r1 = *((double **)(s + sp));                    /* bbox callback function */
      r2 = *((double **)(s + sp + sizeof(double *))); /* mask callback function */
      sp += 2 * sizeof(double *);
      break;

    case 264:                        /* begin partial drawing */
      RESOLVE(ia, int, sizeof(int)); /* id */
      r1 = *((double **)(s + sp));   /* callback function to pass the finished partial drawing */
      sp += sizeof(double *);
      break;

    case 27:  /* set text font and precision */
    case 34:  /* set text alignment */
    case 250: /* begin selection */
      RESOLVE(ia, int, 2 * sizeof(int));
      break;

    case 20:  /* set linewidth scale factor */
    case 24:  /* set marker size scale factor */
    case 28:  /* set character expansion factor */
    case 29:  /* set character spacing */
    case 31:  /* set character height */
    case 109: /* set nominal size */
    case 200: /* set text slant */
    case 203: /* set transparency */
    case 206: /* set border width */
      RESOLVE(r1, double, sizeof(double));
      break;

    case 32:  /* set character up vector */
    case 212: /* set clip sector */
      RESOLVE(r1, double, sizeof(double));
      RESOLVE(r2, double, sizeof(double));
      break;

    case 41: /* set aspect source flags */
      RESOLVE(ia, int, 13 * sizeof(int));
      break;

    case 48: /* set color representation */
      sp -= sizeof(int);
      RESOLVE(ia, int, 2 * sizeof(int));
      RESOLVE(r1, double, 3 * sizeof(double));
      break;

    case 49: /* set window */
    case 50: /* set viewport */
    case 54: /* set workstation window */
    case 55: /* set workstation viewport */
      RESOLVE(ia, int, sizeof(int));
      RESOLVE(r1, double, 2 * sizeof(double));
      RESOLVE(r2, double, 2 * sizeof(double));
      break;

    case 202: /* set shadow */
      RESOLVE(r1, double, 3 * sizeof(double));
      break;

    case 204: /* set coord xform */
      RESOLVE(r1, double, 6 * sizeof(double));
      break;

    case 251: /* end selection */
    case 262: /* set background */
    case 263: /* clear background */
      break;

    case 252: /* move selection */
      RESOLVE(r1, double, sizeof(double));
      RESOLVE(r2, double, sizeof(double));
      break;
    }

  switch (*fctid)
    {
    case 19:
      (*gkss)->ltype = ia[0];
      break;
    case 20:
      (*gkss)->lwidth = r1[0];
      break;
    case 21:
      (*gkss)->plcoli = ia[0];
      break;
    case 23:
      (*gkss)->mtype = ia[0];
      break;
    case 24:
      (*gkss)->mszsc = r1[0];
      break;
    case 25:
      (*gkss)->pmcoli = ia[0];
      break;
    case 27:
      (*gkss)->txfont = ia[0];
      (*gkss)->txprec = ia[1];
      break;
    case 28:
      (*gkss)->chxp = r1[0];
      break;
    case 29:
      (*gkss)->chsp = r1[0];
      break;
    case 30:
      (*gkss)->txcoli = ia[0];
      break;
    case 31:
      (*gkss)->chh = r1[0];
      break;
    case 32:
      (*gkss)->chup[0] = r1[0];
      (*gkss)->chup[1] = r2[0];
      break;
    case 33:
      (*gkss)->txp = ia[0];
      break;
    case 34:
      (*gkss)->txal[0] = ia[0];
      (*gkss)->txal[1] = ia[1];
      break;
    case 36:
      (*gkss)->ints = ia[0];
      break;
    case 37:
      (*gkss)->styli = ia[0];
      break;
    case 38:
      (*gkss)->facoli = ia[0];
      break;
    case 41:
      for (i = 0; i < 13; i++) (*gkss)->asf[i] = ia[i];
      break;
    case 49:
      (*gkss)->window[ia[0]][0] = r1[0];
      (*gkss)->window[ia[0]][1] = r1[1];
      (*gkss)->window[ia[0]][2] = r2[0];
      (*gkss)->window[ia[0]][3] = r2[1];
      break;
    case 50:
      (*gkss)->viewport[ia[0]][0] = r1[0];
      (*gkss)->viewport[ia[0]][1] = r1[1];
      (*gkss)->viewport[ia[0]][2] = r2[0];
      (*gkss)->viewport[ia[0]][3] = r2[1];
      break;
    case 52:
      (*gkss)->cntnr = ia[0];
      break;
    case 53:
      (*gkss)->clip = ia[0];
      break;
    case 54:
      (*gkss)->aspect_ratio = (r1[1] - r1[0]) / (r2[1] - r2[0]);
      break;
    case 108:
      (*gkss)->resample_method = ia[0];
      break;
    case 109:
      (*gkss)->nominal_size = r1[0];
      break;
    case 200:
      (*gkss)->txslant = r1[0];
      break;
    case 202:
      (*gkss)->shoff[0] = r1[0];
      (*gkss)->shoff[1] = r1[1];
      (*gkss)->blur = r1[2];
      break;
    case 203:
      (*gkss)->alpha = r1[0];
      break;
    case 206:
      (*gkss)->bwidth = r1[0];
      break;
    case 207:
      (*gkss)->bcoli = ia[0];
      break;
    case 208:
      (*gkss)->clip_tnr = ia[0];
      break;
    case 211:
      (*gkss)->clip_region = ia[0];
      break;
    case 212:
      (*gkss)->clip_start_angle = r1[0];
      (*gkss)->clip_end_angle = r2[0];
      break;
    }

  fn(*fctid, *dx, *dy, *dimx, ia, 0, r1, 0, r2, *lc, chars, (void **)gkss);
  return sp;
}
