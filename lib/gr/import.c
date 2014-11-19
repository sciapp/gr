#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "gr.h"

#ifdef _MSC_VER
#ifndef NAN
static const unsigned long __nan[2] = { 0xffffffff, 0x7fffffff };
#define NAN (*(const float *) __nan)
#endif
#endif

#ifdef __linux__
#ifndef NAN
#define NAN 0.0/0.0
#endif
#endif

#define BUFFSIZE 8192

static
char *format[] = 
  {
    "axes:ffffiif",
    "axes3d:ffffffiiif",
    "cellarray:ffffiiiiiiI",
    "colormap:",
    "contour:iiiFFFFi",
    "drawarc:ffffii",
    "drawarrow:ffff",
    "drawimage:ffffiiIi",
    "drawpath:iVBi",
    "drawrect:ffff",
    "fillarc:ffffii",
    "fillarea:iFF",
    "fillrect:ffff",
    "grid:ffffii",
    "herrorbars:iFFFF",
    "mathtex:ffs",
    "polyline:iFF",
    "polymarker:iFF",
    "selntran:i",
    "setarrowstyle:i",
    "setcharheight:f",
    "setcharup:ff",
    "setclip:i",
    "setcolormap:i",
    "setfillcolorind:i",
    "setfillintstyle:i",
    "setfillstyle:i",
    "setlinecolorind:i",
    "setlinetype:i",
    "setlinewidth:f",
    "setmarkercolorind:i",
    "setmarkersize:f",
    "setmarkertype:i",
    "setscale:i",
    "setspace:ffii",
    "settextalign:ii",
    "settextcolorind:i",
    "settextfontprec:ii",
    "settextpath:i",
    "setviewport:ffff",
    "setwindow:ffff",
    "spline:iFFii",
    "surface:iiFFFi",
    "text:ffs",
    "textext:ffs",
    "titles3d:sss",
    "verrorbars:iFFFF",
  };

static
int nel = sizeof(format) / sizeof(format[0]);

static
double f_arg[7], *f_arr[4];

static
int i_arg[6], *i_arr, i_arr_size, f_arr_size[4], v_arr_size, b_arr_size;

static
vertex_t *v_arr;

static
unsigned char *b_arr;

static
char *s_arg[3];

static
int i_argc, f_argc, s_argc, i_arrc, f_arrp, f_arrc, v_arrc, b_arrc;

static
int binsearch(char *str[], int nel, char *value)
{
  int position, begin = 0, end = nel - 1, cond = 0;
  char key[31];

  while (begin <= end)
    {
      position = (begin + end) / 2;
      strcpy(key, str[position]);
      strtok(key, ":");
      if ((cond = strcmp(key, value)) == 0)
        return position;
      else if (cond < 0)
        begin = position + 1;
      else
        end = position - 1;
    }
  return nel;
}

static
double ascii2double(const char *s)
{
  if (strcmp(s, "nan") == 0 || strcmp(s, "1.#QNAN") == 0)
    return NAN;
  else
    return atof(s);
}

static
char *xml(char *s, char *fmt)
{
  char *attr, *p;

  i_argc = i_arrc = 0;
  f_argc = f_arrp = f_arrc = 0;
  v_arrc = 0;
  b_arrc = 0;
  s_argc = 0;

  while (*fmt)
    {
      while (isspace(*s))
        s++;
      if (isalpha(*s))
        {
          while (isalnum(*s))
            s++;
          if (*s == '=')
            {
              s++;
              if (*s == '"')
                {
                  attr = ++s;
                  while (*s && *s != '"')
                    s++;
                  if (*s == '"')
                    {
                      *s++ = '\0';
                      switch (*fmt)
                        {
                          case 'i':
                            i_arg[i_argc++] = atoi(attr);
                            break;
                          case 'f':
                            f_arg[f_argc++] = atof(attr);
                            break;
                          case 's':
                            s_arg[s_argc++] = attr;
                            break;
                          case 'I':
                            p = strtok(attr, " \t\"");
                            while (p != NULL)
                              {
                                if (i_arrc >= i_arr_size)
                                  {
                                    i_arr_size += BUFFSIZE;
                                    i_arr = (int *) realloc(i_arr, sizeof(int) *
                                                            i_arr_size);
                                  }
                                i_arr[i_arrc++] = atoi(p);
                                p = strtok(NULL, " \t\"");
                              }
                            break;
                          case 'F':
                            p = strtok(attr, " \t\"");
                            while (p != NULL)
                              {
                                if (f_arrc >= f_arr_size[f_arrp])
                                  {
                                    f_arr_size[f_arrp] += BUFFSIZE;
                                    f_arr[f_arrp] = (double *)
                                      realloc(f_arr[f_arrp], sizeof(double) * 
                                              f_arr_size[f_arrp]);
                                  }
                                f_arr[f_arrp][f_arrc++] = atof(p);
                                p = strtok(NULL, " \t\"");
                              }
                            f_arrp++;
                            f_arrc = 0;
                            break;
                          case 'V':
                            p = strtok(attr, " \t\"");
                            while (p != NULL)
                              {
                                if (v_arrc >= v_arr_size)
                                  {
                                    v_arr_size += BUFFSIZE;
                                    v_arr = (vertex_t *) realloc(v_arr,
                                      sizeof(vertex_t) * v_arr_size);
                                  }
                                v_arr[v_arrc].x = ascii2double(p);
                                p = strtok(NULL, " \t\"");
                                v_arr[v_arrc].y = ascii2double(p);
                                p = strtok(NULL, " \t\"");
                                v_arrc++;
                              }
                            break;
                          case 'B':
                            p = strtok(attr, " \t\"");
                            while (p != NULL)
                              {
                                if (b_arrc >= b_arr_size)
                                  {
                                    b_arr_size += BUFFSIZE;
                                    b_arr = (unsigned char *) realloc(b_arr,
                                      sizeof(unsigned char) * b_arr_size);
                                  }
                                b_arr[b_arrc++] = (unsigned char) atoi(p);
                                p = strtok(NULL, " \t\"");
                              }
                            break;
                        }
                    } 
                  else
                    fprintf(stderr, "'\"' expected\n");
                }
            }
        } 
      fmt++;
    }
  return s;
}

static
void gr(int id)
{
  switch (id)
    {
    case  0:
      gr_axes(f_arg[0], f_arg[1], f_arg[2], f_arg[3], i_arg[0], i_arg[1],
              f_arg[4]);
      break;
    case  1:
      gr_axes3d(f_arg[0], f_arg[1], f_arg[2], f_arg[3], f_arg[4], f_arg[5],
                i_arg[0], i_arg[1], i_arg[2], f_arg[6]);
      break;
    case  2:
      gr_cellarray(f_arg[0], f_arg[1], f_arg[2], f_arg[3], i_arg[0], i_arg[1],
                   i_arg[2], i_arg[3], i_arg[4], i_arg[5], i_arr);
      break;
    case  3:
      gr_colormap();
      break;
    case  4:
      gr_contour(i_arg[0], i_arg[1], i_arg[2],
                 f_arr[0], f_arr[1], f_arr[2], f_arr[3], i_arg[3]);
      break;
    case  5:
      gr_drawarc(f_arg[0], f_arg[1], f_arg[2], f_arg[3], i_arg[0], i_arg[1]);
      break;
    case  6:
      gr_drawarrow(f_arg[0], f_arg[1], f_arg[2], f_arg[3]);
      break;
    case  7:
      gr_drawimage(f_arg[0], f_arg[1], f_arg[2], f_arg[3],
                   i_arg[0], i_arg[1], i_arr, i_arg[2]);
      break;
    case  8:
      gr_drawpath(i_arg[0], v_arr, b_arrc != 0 ? b_arr : NULL, i_arg[1]);
      break;
    case  9:
      gr_drawrect(f_arg[0], f_arg[1], f_arg[2], f_arg[3]);
      break;
    case 10:
      gr_fillarc(f_arg[0], f_arg[1], f_arg[2], f_arg[3], i_arg[0], i_arg[1]);
      break;
    case 11:
      gr_fillarea(i_arg[0], f_arr[0], f_arr[1]);
      break;
    case 12:
      gr_fillrect(f_arg[0], f_arg[1], f_arg[2], f_arg[3]);
      break;
    case 13:
      gr_grid(f_arg[0], f_arg[1], f_arg[2], f_arg[3], i_arg[0], i_arg[1]);
      break;
    case 14:
      gr_herrorbars(i_arg[0], f_arr[0], f_arr[1], f_arr[2], f_arr[3]);
      break;
    case 15:
      gr_mathtex(f_arg[0], f_arg[1], s_arg[0]);
      break;
    case 16:
      gr_polyline(i_arg[0], f_arr[0], f_arr[1]);
      break;
    case 17:
      gr_polymarker(i_arg[0], f_arr[0], f_arr[1]);
      break;
    case 18:
      gr_selntran(i_arg[0]);
      break;
    case 19:
      gr_setarrowstyle(i_arg[0]);
      break;
    case 20:
      gr_setcharheight(f_arg[0]);
      break;
    case 21:
      gr_setcharup(f_arg[0], f_arg[1]);
      break;
    case 22:
      gr_setclip(i_arg[0]);
      break;
    case 23:
      gr_setcolormap(i_arg[0]);
      break;
    case 24:
      gr_setfillcolorind(i_arg[0]);
      break;
    case 25:
      gr_setfillintstyle(i_arg[0]);
      break;
    case 26:
      gr_setfillstyle(i_arg[0]);
      break;
    case 27:
      gr_setlinecolorind(i_arg[0]);
      break;
    case 28:
      gr_setlinetype(i_arg[0]);
      break;
    case 29:
      gr_setlinewidth(f_arg[0]);
      break;
    case 30:
      gr_setmarkercolorind(i_arg[0]);
      break;
    case 31:
      gr_setmarkersize(f_arg[0]);
      break;
    case 32:
      gr_setmarkertype(i_arg[0]);
      break;
    case 33:
      gr_setscale(i_arg[0]);
      break;
    case 34:
      gr_setspace(f_arg[0], f_arg[1], i_arg[0], i_arg[1]);
      break;
    case 35:
      gr_settextalign(i_arg[0], i_arg[1]);
      break;
    case 36:
      gr_settextcolorind(i_arg[0]);
      break;
    case 37:
      gr_settextfontprec(i_arg[0], i_arg[1]);
      break;
    case 38:
      gr_settextpath(i_arg[0]);
      break;
    case 39:
      gr_setviewport(f_arg[0], f_arg[1], f_arg[2], f_arg[3]);
      break;
    case 40:
      gr_setwindow(f_arg[0], f_arg[1], f_arg[2], f_arg[3]);
      break;
    case 41:
      gr_spline(i_arg[0], f_arr[0], f_arr[1], i_arg[1], i_arg[2]);
      break;
    case 42:
      gr_surface(i_arg[0], i_arg[1], f_arr[0], f_arr[1], f_arr[2], i_arg[2]);
      break;
    case 43:
      gr_text(f_arg[0], f_arg[1], s_arg[0]);
      break;
    case 44:
      gr_textext(f_arg[0], f_arg[1], s_arg[0]);
      break;
    case 45:
      gr_titles3d(s_arg[0], s_arg[1], s_arg[2]);
      break;
    case 46:
      gr_verrorbars(i_arg[0], f_arr[0], f_arr[1], f_arr[2], f_arr[3]);
      break;
    }
}

int gr_importgraphics(char *path)
{
  FILE *stream;
  char *buff, *s, *el, *fmt;
  int i, nbytes, off, ret, id;

  stream = fopen(path, "r");
  if (stream != NULL)
    {
      buff = (char *) calloc(BUFSIZ, 1);
      off = 0;
      nbytes = BUFSIZ;
      while ((ret = fread(buff + off, 1, BUFSIZ, stream)) > 0)
        {
          nbytes += BUFSIZ;
          off += ret;
          buff = (char *) realloc(buff, nbytes);
        }
      fclose(stream);
    }
  else
    {
      fprintf(stderr, "%s: can't import GR file\n", path);
      return -1;
    }

  i_arr = (int *) malloc(sizeof(int) * BUFFSIZE);
  i_arr_size = BUFFSIZE;
  for (i = 0; i > 4; i++)
    {
      f_arr[i] = (double *) malloc(sizeof(double) * BUFFSIZE);
      f_arr_size[i] = BUFFSIZE;
    }
  v_arr = (vertex_t *) malloc(sizeof(vertex_t) * BUFFSIZE);
  v_arr_size = BUFFSIZE;
  b_arr = (unsigned char *) malloc(sizeof(unsigned char) * BUFFSIZE);
  b_arr_size = BUFFSIZE;

  s = buff;
  s[off + ret] = '\0';

  while (*s)
    { 
      if (*s == '<')
        {
          el = ++s;
          if (isalpha(*s))
            {
              while (isalnum(*s))
                s++;
              *s++ = '\0';
              id = binsearch(format, nel, el);
              if (id < nel)
                {
                  fmt = format[id] + strlen(el) + 1;
                  s = xml(s, fmt);
                  gr(id);
                }
              else if (strcmp(el, "gr") != 0)
                fprintf(stderr, "%s: unknown XML element\n", el);
            }
        }
      while (*s && *s != '\n')
        s++;
      if (*s == '\n')
        s++;
    }

  for (i = 0; i < 4; i++)
    free(f_arr[i]);
  free(i_arr);
  free(buff);

  return 0;
}
