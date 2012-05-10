
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#if !defined(VMS) && !defined(_WIN32)
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#define TMPDIR "C:\\TEMP"
#define DIRDELIM "\\"
#else
#define TMPDIR "/tmp"
#define DIRDELIM "/"
#endif

#include "gks.h"
#include "gkscore.h"
#include "gr.h"
#include "text.h"
#include "spline.h"
#include "contour.h"
#include "strlib.h"
#include "property.h"
#include "md5.h"

typedef struct
{
  int index;
  float red, green, blue;
}
color_t;

typedef struct
{
  float xmin, xmax, ymin, ymax;
}
rect_t;

typedef struct
{
  float a, b, c, d;
}
norm_xform;

typedef struct
{
  int scale_options;
  float xmin, xmax, ymin, ymax, zmin, zmax, a, b, c, d, e, f;
}
linear_xform;

typedef struct
{
  float zmin, zmax;
  int phi, delta;
  float a1, a2, b, c1, c2, c3, d;
}
world_xform;

typedef struct
{
  int sign;
  float x0, x1, y0, y1, z0, z1;
  float xmin, xmax;
  int initialize;
  float *buf, *ymin, *ymax;
}
hlr_t;

static
norm_xform nx = { 1, 0, 1, 0 };

static
linear_xform lx = { 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0 };

static
world_xform wx = { 0, 1, 60, 60, 0, 0, 0, 0, 0, 0, 0 };

static
hlr_t hlr = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, NULL, NULL, NULL };

static
int autoinit = 1, flag_abort = 0, double_buf = 0, accel = 0;

static
float cxl, cxr, cyf, cyb, czb, czt;

static
int arrow_style = 0;

static
int flag_printing = 0, flag_graphics = 0;

static
FILE *stream;

static
float xfac[4] = { 0, 0, -0.5, -1 };

static
float yfac[6] = { 0, -1.2, -1, -0.5, 0, 0.2 };

#define check_autoinit if (autoinit) initgks()

#define NDC 0
#define WC  1

#define POINT_INC 2048

#define RESOLUTION_X 4096
#define BACKGROUND 0
#define FIRST_COLOR 8
#define LAST_COLOR 79
#define MAX_COLOR 980
#define MISSING_VALUE FLT_MAX

#ifndef FLT_MAX
#define FLT_MAX 1.701411735e+38
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FEPS 1.0e-6

#define OPTION_X_LOG (1 << 0)
#define OPTION_Y_LOG (1 << 1)
#define OPTION_Z_LOG (1 << 2)

#define OPTION_FLIP_X (1 << 3)
#define OPTION_FLIP_Y (1 << 4)
#define OPTION_FLIP_Z (1 << 5)

#define LEFT   (1<<0)
#define RIGHT  (1<<1)
#define FRONT  (1<<2)
#define BACK   (1<<3)
#define BOTTOM (1<<4)
#define TOP    (1<<5)

#define GR_HEADER  "<?xml version='1.0' encoding='ISO-8859-1'?>\n<gr>\n"
#define GR_TRAILER "</gr>\n"

#define nominalWindowHeight 500
#define qualityFactor 4

typedef enum
{
  OPTION_LINES, OPTION_MESH, OPTION_FILLED_MESH, OPTION_Z_SHADED_MESH,
  OPTION_COLORED_MESH, OPTION_CELL_ARRAY, OPTION_SHADED_MESH
}
surface_option_t;

typedef enum {
  COLORMAP_UNIFORM, COLORMAP_TEMPERATURE, COLORMAP_GRAYSCALE,
  COLORMAP_GLOWING, COLORMAP_RAINBOW, COLORMAP_GEOLOGIC,
  COLORMAP_GREENSCALE, COLORMAP_CYANSCALE, COLORMAP_BLUESCALE,
  COLORMAP_MAGENTASCALE, COLORMAP_REDSCALE, COLORMAP_FLAME,
  COLORMAP_BROWNSCALE, COLORMAP_USER_DEFINED
}
colormap_t;

typedef struct
{
  char *format;
  float width, height;
}
format_t;

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#define arc(angle) (M_PI * (angle) / 180.0)
#define deg(rad) ((rad) * 180.0 / M_PI)

static
float *xpoint = NULL, *ypoint = NULL, *zpoint = NULL;

static
int npoints = 0, maxpoints = 0;

/*  0 - 20    21 - 45    46 - 70    71 - 90           rot/  */
static
int rep_table[16][3] =
{                    /*   tilt  */
  {2, 2, 2}, {2, 2, 2}, {1, 3, 2}, {1, 3, 3},    /*  0 - 20 */
  {2, 0, 0}, {2, 0, 0}, {1, 3, 2}, {1, 3, 1},    /* 21 - 45 */
  {2, 0, 0}, {0, 0, 0}, {1, 3, 0}, {1, 3, 1},    /* 46 - 70 */
  {0, 0, 0}, {0, 0, 0}, {1, 3, 0}, {1, 3, 1}     /* 71 - 90 */
};

static
int axes_rep[4][3] =
{
  {1, 1, 1}, {1, 1, 2}, {0, 0, 0}, {0, 0, 3}
};

static
int angle[4] = {20, 45, 70, 90};

static
format_t formats[] = {
  { "A4",        0.210, 0.297 },
  { "B5",        0.176, 0.250 },
  { "Letter",    0.216, 0.279 }, 
  { "Legal",     0.216, 0.356 }, 
  { "Executive", 0.191, 0.254 }, 
  { "A0",        0.841, 1.189 }, 
  { "A1",        0.594, 0.841 }, 
  { "A2",        0.420, 0.594 }, 
  { "A3",        0.297, 0.420 }, 
  { "A5",        0.148, 0.210 }, 
  { "A6",        0.105, 0.148 }, 
  { "A7",        0.074, 0.105 }, 
  { "A8",        0.052, 0.074 }, 
  { "A9",        0.037, 0.052 }, 
  { "B0",        1.000, 1.414 }, 
  { "B1",        0.500, 0.707 }, 
  { "B10",       0.031, 0.044 }, 
  { "B2",        0.500, 0.707 }, 
  { "B3",        0.353, 0.500 }, 
  { "B4",        0.250, 0.353 }, 
  { "B6",        0.125, 0.176 }, 
  { "B7",        0.088, 0.125 }, 
  { "B8",        0.062, 0.088 }, 
  { "B9",        0.044, 0.062 }, 
  { "C5E",       0.163, 0.229 }, 
  { "Comm10E",   0.105, 0.241 }, 
  { "DLE",       0.110, 0.220 }, 
  { "Folio",     0.210, 0.330 }, 
  { "Ledger",    0.432, 0.279 }, 
  { "Tabloid",   0.279, 0.432 }, 
  { NULL,        0.000, 0.000 }
};

static
int vertex_list[18][25] = {
  { 3, -10,80, 0,100, 10,80, 2, 0,100, 0,-100, 0 },
  { 3, -7,80, 0,100, 7,80, 2, 0,100, 0,-100, 0 },
  { 6, 0,85, -10,80, 0,100, 10,80, 0,85, 0,-100, 0 },
  { -4, 0,85, -10,80, 0,100, 10,80, 2, 0,85, 0,-100, 0 },
  { 6, 0,80, -10,80, 0,100, 10,80, 0,80, 0,-100, 0 },
  { -4, 0,80, -10,80, 0,100, 10,80, 2, 0,80, 0,-100, 0 },
  { 6, 0,75, -10,80, 0,100, 10,80, 0,75, 0,-100, 0 },
  { -4, 0,75, -10,80, 0,100, 10,80, 2, 0,75, 0,-100, 0 },
  { 3, -10,80, 0,100, 10,80, 2, 0,100, 0,-100, 3, -10,-80, 0,-100, 10,-80, 0 },
  { 3, -7,80, 0,100, 7,80, 2, 0,100, 0,-100, 3, -7,-80, 0,-100, 7,-80, 0 },
  { 10, 0,85, -10,80, 0,100, 10,80, 0,85, 0,-85, -10,-80, 0,-100, 10,-80,
    0,-85, 0 },
  { -4, 0,85, -10,80, 0,100, 10,80, 2, 0,85, 0,-85, -4, -10,-80, 0,-100,
    10,-80, 0,-85, 0 },
  { 10, 0,80, -10,80, 0,100, 10,80, 0,80, 0,-80, -10,-80, 0,-100, 10,-80,
    0,-80, 0 },
  { -4, 0,80, -10,80, 0,100, 10,80, 2, 0,80, 0,-80, -4, -10,-80, 0,-100,
    10,-80, 0,-80, 0 },
  { 10, 0,75, -10,80, 0,100, 10,80, 0,75, 0,-75, -10,-80, 0,-100, 10,-80,
    0,-75, 0 },
  { -4, 0,75, -10,80, 0,100, 10,80, 2, 0,75, 0,-75, -4, -10,-80, 0,-100,
    10,-80, 0,-75, 0 },
  { 3, -10,80, 0,100, 10,80, 2, -1,98, -1,-100, 2, 1,98, 1,-100, 0 },
  { 3, -10,80, 0,100, 10,80, 2, -1,98, -1,-98, 3, -10,-80, 0,-100, 10,-80,
    2, 1,98, 1,-98, 0 }
};

static
int cmap[72][3] = {
  { 27, 27, 29 },
  { 9, 9, 19 },
  { 17, 17, 34 },
  { 24, 24, 49 },
  { 31, 31, 62 },
  { 39, 39, 77 },
  { 45, 45, 90 },
  { 53, 53, 106 },
  { 60, 60, 120 },
  { 67, 67, 133 },
  { 74, 74, 149 },
  { 81, 81, 161 },
  { 89, 89, 177 },
  { 95, 95, 190 },
  { 103, 103, 205 },
  { 110, 110, 219 },
  { 116, 116, 233 },
  { 125, 124, 250 },
  { 123, 131, 244 },
  { 106, 139, 211 },
  { 93, 145, 185 },
  { 78, 152, 156 },
  { 62, 160, 124 },
  { 49, 166, 99 },
  { 33, 174, 67 },
  { 21, 181, 42 },
  { 5, 189, 11 },
  { 11, 192, 0 },
  { 32, 192, 0 },
  { 56, 192, 0 },
  { 74, 192, 0 },
  { 99, 192, 0 },
  { 120, 192, 0 },
  { 141, 192, 0 },
  { 163, 192, 0 },
  { 183, 192, 0 },
  { 190, 187, 5 },
  { 186, 181, 11 },
  { 182, 173, 19 },
  { 179, 166, 26 },
  { 175, 159, 33 },
  { 171, 150, 41 },
  { 168, 144, 47 },
  { 164, 136, 55 },
  { 161, 130, 61 },
  { 154, 122, 61 },
  { 147, 115, 57 },
  { 140, 108, 54 },
  { 133, 101, 50 },
  { 126, 94, 47 },
  { 118, 86, 43 },
  { 112, 80, 40 },
  { 104, 72, 36 },
  { 97, 65, 32 },
  { 97, 68, 39 },
  { 102, 76, 51 },
  { 105, 83, 61 },
  { 109, 90, 72 },
  { 112, 97, 82 },
  { 116, 104, 93 },
  { 120, 112, 105 },
  { 122, 118, 114 },
  { 127, 127, 126 },
  { 138, 138, 138 },
  { 153, 153, 153 },
  { 168, 168, 168 },
  { 181, 181, 181 },
  { 197, 197, 197 },
  { 209, 209, 209 },
  { 225, 225, 225 },
  { 239, 239, 239 },
  { 251, 251, 251 }
};

#ifdef _WIN32

LPSTR FAR PASCAL DLLGetEnv(LPSTR lpszVariableName)
{
  LPSTR lpEnvSearch;
  LPSTR lpszVarSearch;

  if (!*lpszVariableName)
    return NULL;

  lpEnvSearch = GetEnvironmentStrings();

  while (*lpEnvSearch)
    {
      /*
       *  Make a copy of the pointer to the name of the
       *  environment variable to search for.
       */
      lpszVarSearch = lpszVariableName;

      /*  Check to see if the variable names match */
      while (*lpEnvSearch && *lpszVarSearch && *lpEnvSearch == *lpszVarSearch)
	{
	  lpEnvSearch++;
	  lpszVarSearch++;
	}
      /*
       *  If the names match, the lpEnvSearch pointer is on the "="
       *  character and lpszVarSearch is on a null terminator.
       *  Increment and return lpszEnvSearch, which will point to the
       *  environment variable's contents.
       *
       *  If the names do not match, increment lpEnvSearch until it
       *  reaches the end of the current variable string.
       */
      if (*lpEnvSearch == '=' && *lpszVarSearch == '\0')
	return (lpEnvSearch + 1);
      else
	while (*lpEnvSearch)
	  lpEnvSearch++;

      /*
       *  At this point the end of the environment variable's string
       *  has been reached. Increment lpEnvSearch to move to the
       *  next variable in the environment block. If it is NULL,
       *  the end of the environment block has been reached.
       */
      lpEnvSearch++;
    }

  return NULL;		/*
			 *  If this section of code is reached, the variable
			 *  was not found.
			 */
}

#endif

static
char *xmalloc(int size)
{
  char *result = (char *) malloc(size);
  if (!result)
    {
      fprintf(stderr, "out of virtual memory\n");
      abort();
    }
  return (result);
}

static
char *xrealloc(void *ptr, int size)
{
  char *result = (char *) realloc(ptr, size);
  if (!result)
    {
      fprintf(stderr, "out of virtual memory\n");
      abort();
    }
  return (result);
}

static
void reallocate(int npoints)
{
  while (npoints >= maxpoints)
    maxpoints += POINT_INC;

  xpoint = (float *) xrealloc(xpoint, maxpoints * sizeof(float));
  ypoint = (float *) xrealloc(ypoint, maxpoints * sizeof(float));
  zpoint = (float *) xrealloc(zpoint, maxpoints * sizeof(float));
}

static
float x_lin(float x)
{
  float result;

  if (OPTION_X_LOG & lx.scale_options)
    {
      if (x > 0)
	result = lx.a * log10(x) + lx.b;
      else
	result = -FLT_MAX;
    }
  else
    result = x;

  if (OPTION_FLIP_X & lx.scale_options)
    result = lx.xmax - result + lx.xmin;

  return (result);
}

static
float y_lin(float y)
{
  float result;

  if (OPTION_Y_LOG & lx.scale_options)
    {
      if (y > 0)
	result = lx.c * log10(y) + lx.d;
      else
	result = -FLT_MAX;
    }
  else
    result = y;

  if (OPTION_FLIP_Y & lx.scale_options)
    result = lx.ymax - result + lx.ymin;

  return (result);
}

static
float z_lin(float z)
{
  float result;

  if (OPTION_Z_LOG & lx.scale_options)
    {
      if (z > 0)
	result = lx.e * log10(z) + lx.f;
      else
	result = -FLT_MAX;
    }
  else
    result = z;

  if (OPTION_FLIP_Z & lx.scale_options)
    result = lx.zmax - result + lx.zmin;

  return (result);
}

static
float x_log(float x)
{
  if (OPTION_FLIP_X & lx.scale_options)
    x = lx.xmax - x + lx.xmin;

  if (OPTION_X_LOG & lx.scale_options)
    return (pow(10.0, (double) ((x - lx.b) / lx.a)));
  else
    return (x);
}

static
float y_log(float y)
{
  if (OPTION_FLIP_Y & lx.scale_options)
    y = lx.ymax - y + lx.ymin;

  if (OPTION_Y_LOG & lx.scale_options)
    return (pow(10.0, (double) ((y - lx.d) / lx.c)));
  else
    return (y);
}

static
float z_log(float z)
{
  if (OPTION_FLIP_Z & lx.scale_options)
    z = lx.zmax - z + lx.zmin;

  if (OPTION_Z_LOG & lx.scale_options)
    return (pow(10.0, (double) ((z - lx.f) / lx.e)));
  else
    return (z);
}

static
float atan_2(float x, float y)
{
  float a;

  if (y == 0)
    if (x < 0)
      a = -M_PI;
    else
      a = M_PI;
  else
    a = atan(x / y);

  return (a);
}

static
void apply_world_xform (float *x, float *y, float *z)
{
  float xw, yw;

  xw = wx.a1 * *x + wx.a2 * *y + wx.b;
  yw = wx.c1 * *x + wx.c2 * *y + wx.c3 * *z + wx.d;
  *x = xw;
  *y = yw;
}

static
void foreach_openws(void (*routine) (int, void *), void *arg)
{
  int state, count, n = 1, errind, ol, wkid;

  gks_inq_operating_state(&state);
  if (state >= GKS_K_WSOP)
    {
      gks_inq_open_ws(n, &errind, &ol, &wkid);
      for (count = ol; count >= 1; count--)
	{
	  n = count;
	  gks_inq_open_ws(n, &errind, &ol, &wkid);

	  routine(wkid, arg);
	}
    }
}

static
void foreach_activews(void (*routine) (int, void *), void *arg)
{
  int state, count, n = 1, errind, ol, wkid;

  gks_inq_operating_state(&state);
  if (state >= GKS_K_WSAC)
    {
      gks_inq_active_ws(n, &errind, &ol, &wkid);
      for (count = ol; count >= 1; count--)
	{
	  n = count;
	  gks_inq_active_ws(n, &errind, &ol, &wkid);

	  routine(wkid, arg);
	}
    }
}

static
void initialize(int state)
{
  int tnr = WC, font = 3, options = 0;
  float xmin = 0.2, xmax = 0.9, ymin = 0.2, ymax = 0.9;
  int asf[13] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
  float size = 2, height = 0.027;
  char *env = NULL, *display = NULL;

  if (state == GKS_K_GKCL)
    {
      gks_select_xform(tnr);
      gks_set_viewport(tnr, xmin, xmax, ymin, ymax);

      gks_set_asf(asf);
      gks_set_pmark_size(size);
      gks_set_pmark_type(GKS_K_MARKERTYPE_ASTERISK);
      gks_set_text_fontprec(font, GKS_K_TEXT_PRECISION_STRING);
      gks_set_text_height(height);
      gks_set_text_align(GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_BASE);
    }

  autoinit = 0;
#ifdef _WIN32
  double_buf = DLLGetEnv("GKS_DOUBLE_BUF") != NULL;
  env = DLLGetEnv("GR_ACCEL");
  display = DLLGetEnv("GR_DISPLAY");
#else
  double_buf = getenv("GKS_DOUBLE_BUF") != NULL;
  env = getenv("GR_ACCEL");
  display = getenv("GR_DISPLAY");
#endif
  if (env != NULL)
    accel = atoi(env);
  else if (display != NULL)
    accel = 1;
  else
    accel = 0;

  gr_setscale(options);
}

static
void initgks(void)
{
  int state, errfil = 0, wkid = 1, errind, conid, wtype;

  gks_inq_operating_state(&state);
  if (state == GKS_K_GKCL)
    gks_open_gks(errfil);

  initialize(state);

  if (state == GKS_K_GKCL || state == GKS_K_GKOP)
    {
      gks_open_ws(wkid, GKS_K_CONID_DEFAULT, GKS_K_WSTYPE_DEFAULT);
      gks_activate_ws(wkid);
    }

  gks_inq_ws_conntype(wkid, &errind, &conid, &wtype);
  if (!double_buf)
    double_buf = wtype == 381 || wtype == 400 || wtype == 410;
}

void gr_opengks(void)
{
  int errfil = 0;

  gks_open_gks(errfil);

  initialize(GKS_K_GKCL);
}

void gr_closegks(void)
{
  gks_close_gks();
  autoinit = 1;
}

void gr_inqdspsize(float *mwidth, float *mheight, int *width, int *height)
{
  int n = 1, errind, wkid, ol, conid, wtype, dcunit;

  check_autoinit;

  gks_inq_open_ws(n, &errind, &ol, &wkid);
  gks_inq_ws_conntype(wkid, &errind, &conid, &wtype);
  gks_inq_max_ds_size(wtype, &errind, &dcunit, mwidth, mheight, width, height);
}

void gr_openws(int workstation_id, char *connection, int type)
{
  if (connection)
    {
      if (!*connection)
	connection = NULL;
    }
  gks_open_ws(workstation_id, connection, type);
}

void gr_closews(int workstation_id)
{
  gks_close_ws(workstation_id);
}

void gr_activatews(int workstation_id)
{
  gks_activate_ws(workstation_id);
}

void gr_deactivatews(int workstation_id)
{
  gks_deactivate_ws(workstation_id);
}

static
void clear(int workstation_id, int *clearflag)
{
  int wkid = workstation_id, state, errind, conid, wtype, wkcat;

  gks_inq_operating_state(&state);
  if (state == GKS_K_SGOP)
    gks_close_seg();

  gks_inq_ws_conntype(wkid, &errind, &conid, &wtype);
  gks_inq_ws_category(wtype, &errind, &wkcat);

  if (wkcat == GKS_K_WSCAT_OUTPUT || wkcat == GKS_K_WSCAT_OUTIN ||
      wkcat == GKS_K_WSCAT_MO)
    {
      gks_clear_ws(wkid, *clearflag);
      gks_update_ws(wkid, GKS_K_POSTPONE_FLAG);
    }
}

void gr_clearws(void)
{
  int clearflag = double_buf ? GKS_K_CLEAR_CONDITIONALLY : GKS_K_CLEAR_ALWAYS;

  check_autoinit;

  foreach_activews((void (*)(int, void *)) clear, (void *) &clearflag);

  if (flag_graphics)
    fseek(stream, 0L, SEEK_SET);
}

static
void update(int workstation_id, int *regenflag)
{
  int wkid = workstation_id, errind, conid, wtype, wkcat;

  gks_inq_ws_conntype(wkid, &errind, &conid, &wtype);
  gks_inq_ws_category(wtype, &errind, &wkcat);

  if (wkcat == GKS_K_WSCAT_OUTPUT || wkcat == GKS_K_WSCAT_OUTIN)
    gks_update_ws(wkid, *regenflag);
}

void gr_updatews(void)
{
  int regenflag = double_buf ? GKS_K_PERFORM_FLAG : GKS_K_POSTPONE_FLAG;

  check_autoinit;

  foreach_openws((void (*)(int, void *)) update, (void *) &regenflag);
}

#define gks(primitive) \
  int npoints = n; \
  float *px = x, *py = y; \
  register int i; \
\
  check_autoinit; \
\
  if (lx.scale_options) \
    { \
      if (npoints >= maxpoints) \
	reallocate(npoints); \
\
      px = xpoint; \
      py = ypoint; \
      for (i = 0; i < npoints; i++) \
	{ \
	  px[i] = x_lin(x[i]); \
	  py[i] = y_lin(y[i]); \
	} \
    } \
\
  primitive(npoints, px, py)

static
void print_int_array(char *name, int n, int *data)
{
  register int i;

  fprintf(stream, " %s='", name);
  for (i = 0; i < n; i++)
    {
      if (i > 0)
	fprintf(stream, " ");
      fprintf(stream, "%d", data[i]);
    }
  fprintf(stream, "'");
}

static
void print_float_array(char *name, int n, float *data)
{
  register int i;

  fprintf(stream, " %s='", name);
  for (i = 0; i < n; i++)
    {
      if (i > 0)
	fprintf(stream, " ");
      fprintf(stream, "%g", data[i]);
    }
  fprintf(stream, "'");
}

static
void primitive(char *name, int n, float *x, float *y)
{
  fprintf(stream, "<%s len='%d'", name, n);
  print_float_array("x", n, x);
  print_float_array("y", n, y);
  fprintf(stream, "/>\n");
}

void gr_polyline(int n, float *x, float *y)
{
  gks(gks_polyline);

  if (flag_graphics)
    primitive("polyline", n, x, y);
}

void gr_polymarker(int n, float *x, float *y)
{
  gks(gks_polymarker);

  if (flag_graphics)
    primitive("polymarker", n, x, y);
}

void gr_text(float x, float y, char *string)
{
  int errind, tnr;

  check_autoinit;

  gks_inq_current_xformno(&errind, &tnr);
  if (tnr != NDC)
    gks_select_xform(NDC);

  gks_text(x, y, string);

  if (tnr != NDC)
    gks_select_xform(tnr);

  if (flag_graphics)
    fprintf(stream, "<text x='%g' y='%g' text='%s'/>\n", x, y, string);
}

void gr_fillarea(int n, float *x, float *y)
{
  gks(gks_fillarea);

  if (flag_graphics)
    primitive("fillarea", n, x, y);
}

void gr_cellarray(
  float xmin, float xmax, float ymin, float ymax, int dimx, int dimy,
  int scol, int srow, int ncol, int nrow, int *color)
{
  int wkid = 1, errind;
  int rgb[MAX_COLOR], *data;
  float r, g, b;

  check_autoinit;

  gks_cellarray(
    x_lin(xmin), y_lin(ymax), x_lin(xmax), y_lin(ymin),
    dimx, dimy, scol, srow, ncol, nrow, color);

  if (flag_graphics)
    {
      register int i, n;

      for (i = 0; i < MAX_COLOR; i++)
	{
	  gks_inq_color_rep(wkid, i, GKS_K_VALUE_SET, &errind, &r, &g, &b);
	  rgb[i] =    ((int) (r * 255) & 0xff)
		   + (((int) (g * 255) & 0xff) << 8)
		   + (((int) (b * 255) & 0xff) << 16);
	}

      n = dimx * dimy;
      data = (int *) xmalloc(n * sizeof(int));
      for (i = 0; i < n; i++)
	data[i] = color[i] >= 0 && color[i] < MAX_COLOR ? rgb[color[i]] : 0;

      fprintf(stream, "<image xmin='%g' xmax='%g' ymin='%g' ymax='%g' "
	      "width='%d' height='%d'",
	      xmin, xmax, ymin, ymax, dimx, dimy);
      print_int_array("data", n, data);
      fprintf(stream, " path=''/>\n");

      free(data);
    }
}

void gr_spline(int n, float *px, float *py, int m, int method)
{
  int err = 0, i, j;
  float *t, *s;
  double *sx, *sy, *x, *f, *df, *y, *c, *wk, *se, var, d;
  int ic, job, ier;

  if (n <= 2)
    {
      fprintf(stderr, "invalid number of points\n");
      return;
    }
  else if (n >= m)
    {
      fprintf(stderr, "invalid number of domain values\n");
      return;
    }

  check_autoinit;

  t = (float *) xmalloc(sizeof(float) * m);
  s = (float *) xmalloc(sizeof(float) * m);
  sx = (double *) xmalloc(sizeof(double) * m);
  sy = (double *) xmalloc(sizeof(double) * m);
  x = (double *) xmalloc(sizeof(double) * n);
  f = (double *) xmalloc(sizeof(double) * n);
  df = (double *) xmalloc(sizeof(double) * n);
  y = (double *) xmalloc(sizeof(double) * n);
  c = (double *) xmalloc(sizeof(double) * 3 * (n - 1));
  se = (double *) xmalloc(sizeof(double) * n);
  wk = (double *) xmalloc(sizeof(double) * 7 * (n + 2));

  for (i = 0; i < n; i++)
    {
      x[i] = (double) ((x_lin(px[i]) - lx.xmin) / (lx.xmax - lx.xmin));
      f[i] = (double) ((y_lin(py[i]) - lx.ymin) / (lx.ymax - lx.ymin));
      df[i] = 1;
    }

  if (method >= -1)
    {
      for (i = 1; i < n; i++)
	if (px[i - 1] >= px[i])
	  {
	    fprintf(stderr, "points not sorted in ascending order\n");
	    err = 1;
	  }

      if (!err)
	{
	  sx[0] = x[0];
	  for (j = 1; j < m - 1; j++)
	    sx[j] = x[0] + j * (x[n - 1] - x[0]) / (m - 1);
	  sx[m - 1] = x[n - 1];

	  job = 0;
	  ic = n - 1;
	  var = (double) method;

	  cubgcv(x, f, df, &n, y, c, &ic, &var, &job, se, wk, &ier);

	  if (ier == 0)
	    {
	      for (j = 0; j < m; j++)
		{
		  i = 0;
		  while ((i < ic) && (x[i] <= sx[j]))
		    i++;
		  if (x[i] > sx[j])
		    i--;
		  if (i < 0)
		    i = 0;
		  else if (i >= ic)
		    i = ic - 1;
		  d = sx[j] - x[i];

		  s[j] = (float) (((c[i + 2 * ic] * d + c[i + ic]) * d +
				   c[i]) * d + y[i]);
		}
	    }
	  else
	    {
	      fprintf(stderr, "invalid argument to math library\n");
	      err = 1;
	    }
	}
    }
  else
    {
      b_spline(n, x, f, m, sx, sy);

      for (j = 0; j < m; j++)
	s[j] = (float) sy[j];
    }

  if (!err)
    {
      for (j = 0; j < m; j++)
	{
	  t[j] = x_log((float) (lx.xmin + sx[j] * (lx.xmax - lx.xmin)));
	  s[j] = y_log((float) (lx.ymin + s[j] * (lx.ymax - lx.ymin)));
	}
      gr_polyline(m, t, s);
    }

  free(wk);
  free(se);
  free(c);
  free(y);
  free(df);
  free(f);
  free(x);
  free(sy);
  free(sx);
  free(s);
  free(t);

  if (flag_graphics)
    {
      fprintf(stream, "<spline len='%d'", n);
      print_float_array("x", n, px);
      print_float_array("y", n, py);
      fprintf(stream, " method='%d'/>\n", method);
    }
}

void gr_setasf(int *asfs)
{
  check_autoinit;

  gks_set_asf(asfs);
}

void gr_setlineind(int index)
{
  check_autoinit;

  gks_set_pline_index(index);

  if (flag_graphics)
    fprintf(stream, "<setlineind index='%d'/>\n", index);
}

void gr_setlinetype(int type)
{
  check_autoinit;

  gks_set_pline_linetype(type);

  if (flag_graphics)
    fprintf(stream, "<setlinetype type='%d'/>\n", type);
}

void gr_setlinewidth(float width)
{
  check_autoinit;

  gks_set_pline_linewidth(width);

  if (flag_graphics)
    fprintf(stream, "<setlinewidth width='%g'/>\n", width);
}

void gr_setlinecolorind(int color)
{
  check_autoinit;

  gks_set_pline_color_index(color);

  if (flag_graphics)
    fprintf(stream, "<setlinecolorind color='%d'/>\n", color);
}

void gr_setmarkerind(int index)
{
  check_autoinit;

  gks_set_pmark_index(index);

  if (flag_graphics)
    fprintf(stream, "<setmarkerind index='%d'/>\n", index);
}

void gr_setmarkertype(int type)
{
  check_autoinit;

  gks_set_pmark_type(type);

  if (flag_graphics)
    fprintf(stream, "<setmarkertype type='%d'/>\n", type);
}

void gr_setmarkersize(float size)
{
  check_autoinit;

  gks_set_pmark_size(size);

  if (flag_graphics)
    fprintf(stream, "<setmarkersize size='%g'/>\n", size);
}

void gr_setmarkercolorind(int color)
{
  check_autoinit;

  gks_set_pmark_color_index(color);

  if (flag_graphics)
    fprintf(stream, "<setmarkercolorind color='%d'/>\n", color);
}

void gr_settextind(int index)
{
  check_autoinit;

  gks_set_text_index(index);

  if (flag_graphics)
    fprintf(stream, "<settextind index='%d'/>\n", index);
}

void gr_settextfontprec(int font, int precision)
{
  check_autoinit;

  gks_set_text_fontprec(font, precision);

  if (flag_graphics)
    fprintf(stream, "<settextfontprec font='%d' precision='%d'/>\n",
	    font, precision);
}

void gr_setcharexpan(float factor)
{
  check_autoinit;

  gks_set_text_expfac(factor);
}

void gr_setcharspace(float spacing)
{
  check_autoinit;

  gks_set_text_spacing(spacing);
}

void gr_settextcolorind(int color)
{
  check_autoinit;

  gks_set_text_color_index(color);

  if (flag_graphics)
    fprintf(stream, "<settextcolorind color='%d'/>\n", color);
}

void gr_setcharheight(float height)
{
  check_autoinit;

  gks_set_text_height(height);

  if (flag_graphics)
    fprintf(stream, "<setcharheight height='%g'/>\n", height);
}

void gr_setcharup(float ux, float uy)
{
  check_autoinit;

  gks_set_text_upvec(ux, uy);

  if (flag_graphics)
    fprintf(stream, "<setcharup x='%g' y='%g'/>\n", ux, uy);
}

void gr_settextpath(int path)
{
  check_autoinit;

  gks_set_text_path(path);

  if (flag_graphics)
    fprintf(stream, "<settextpath path='%d'/>\n", path);
}

void gr_settextalign(int horizontal, int vertical)
{
  check_autoinit;

  gks_set_text_align(horizontal, vertical);

  if (flag_graphics)
    fprintf(stream, "<settextalign halign='%d' valign='%d'/>\n",
	    horizontal, vertical);
}

void gr_setfillind(int index)
{
  check_autoinit;

  gks_set_fill_index(index);

  if (flag_graphics)
    fprintf(stream, "<setfillind index='%d'/>\n", index);
}

void gr_setfillintstyle(int style)
{
  check_autoinit;

  gks_set_fill_int_style(style);

  if (flag_graphics)
    fprintf(stream, "<setfillintstyle intstyle='%d'/>\n", style);
}

void gr_setfillstyle(int index)
{
  check_autoinit;

  gks_set_fill_style_index(index);

  if (flag_graphics)
    fprintf(stream, "<setfillstyle style='%d'/>\n", index);
}

void gr_setfillcolorind(int color)
{
  check_autoinit;

  gks_set_fill_color_index(color);

  if (flag_graphics)
    fprintf(stream, "<setfillcolorind color='%d'/>\n", color);
}

static
void setcolor(int workstation_id, color_t *color)
{
  int wkid = workstation_id;

  gks_set_color_rep(wkid, color->index, color->red, color->green,
		    color->blue);
}

void gr_setcolorrep(int index, float red, float green, float blue)
{
  color_t color;

  color.index = index;
  color.red = red;
  color.green = green;
  color.blue = blue;

  check_autoinit;

  foreach_activews((void (*)(int, void *)) setcolor, (void *) &color);
}

int gr_setscale(int options)
{
  int errind, tnr;
  float wn[4], vp[4];
  int result = 0, scale_options;

  check_autoinit;

  gks_inq_current_xformno(&errind, &tnr);
  gks_inq_xform(tnr, &errind, wn, vp);

  nx.a = (vp[1] - vp[0]) / (wn[1] - wn[0]);
  nx.b = vp[0] - wn[0] * nx.a;
  nx.c = (vp[3] - vp[2]) / (wn[3] - wn[2]);
  nx.d = vp[2] - wn[2] * nx.c;

  scale_options = lx.scale_options;
  lx.scale_options = 0;

  lx.xmin = wn[0];
  lx.xmax = wn[1];

  if (OPTION_X_LOG & options)
    {
      if (wn[0] > 0)
	{
	  lx.a = (wn[1] - wn[0]) / log10(wn[1] / wn[0]);
	  lx.b = wn[0] - lx.a * log10(wn[0]);
	  lx.scale_options |= OPTION_X_LOG;
	}
      else
	result = -1;
    }

  lx.ymin = wn[2];
  lx.ymax = wn[3];

  if (OPTION_Y_LOG & options)
    {
      if (wn[2] > 0)
	{
	  lx.c = (wn[3] - wn[2]) / log10(wn[3] / wn[2]);
	  lx.d = wn[2] - lx.c * log10(wn[2]);
	  lx.scale_options |= OPTION_Y_LOG;
	}
      else
	result = -1;
    }

  gr_setspace(wx.zmin, wx.zmax, wx.phi, wx.delta);

  lx.zmin = wx.zmin;
  lx.zmax = wx.zmax;

  if (OPTION_Z_LOG & options)
    {
      if (lx.zmin > 0)
	{
	  lx.e = (lx.zmax - lx.zmin) / log10(lx.zmax / lx.zmin);
	  lx.f = lx.zmin - lx.e * log10(lx.zmin);
	  lx.scale_options |= OPTION_Z_LOG;
	}
      else
	result = -1;
    }

  if (OPTION_FLIP_X & options)
    lx.scale_options |= OPTION_FLIP_X;

  if (OPTION_FLIP_Y & options)
    lx.scale_options |= OPTION_FLIP_Y;

  if (OPTION_FLIP_Z & options)
    lx.scale_options |= OPTION_FLIP_Z;

  if (accel)
    if (options != scale_options)
      gr_setproperty(
	"logx: %d; logy: %d; logz: %d; flipx: %d; flipy: %d; flipz: %d;",
	OPTION_X_LOG & lx.scale_options ? 1 : 0,
	OPTION_Y_LOG & lx.scale_options ? 1 : 0,
	OPTION_Z_LOG & lx.scale_options ? 1 : 0,
	OPTION_FLIP_X & lx.scale_options ? 1 : 0,
	OPTION_FLIP_Y & lx.scale_options ? 1 : 0,
	OPTION_FLIP_Z & lx.scale_options ? 1 : 0);

  if (flag_graphics)
    fprintf(stream, "<setscale scale='%d'/>\n", options);

  return result;
}

void gr_inqscale(int *options)
{
  *options = lx.scale_options;
}

void gr_setwindow(float xmin, float xmax, float ymin, float ymax)
{
  int tnr = WC;

  check_autoinit;

  gks_set_window(tnr, xmin, xmax, ymin, ymax);
  gr_setscale(lx.scale_options);

  if (accel)
    gr_setproperty("xmin: %g; xmax: %g; ymin: %g; ymax: %g;",
		   lx.xmin, lx.xmax, lx.ymin, lx.ymax);

  if (flag_graphics)
    fprintf(stream, "<setwindow xmin='%g' xmax='%g' ymin='%g' ymax='%g'/>\n",
	    xmin, xmax, ymin, ymax);
}

void gr_inqwindow(float *xmin, float *xmax, float *ymin, float *ymax)
{
  *xmin = lx.xmin;
  *xmax = lx.xmax;
  *ymin = lx.ymin;
  *ymax = lx.ymax;
}

void gr_setviewport(float xmin, float xmax, float ymin, float ymax)
{
  int tnr = WC;

  check_autoinit;

  gks_set_viewport(tnr, xmin, xmax, ymin, ymax);
  gr_setscale(lx.scale_options);

  if (flag_graphics)
    fprintf(stream, "<setviewport xmin='%g' xmax='%g' ymin='%g' ymax='%g'/>\n",
	    xmin, xmax, ymin, ymax);
}

void gr_selntran(int transform)
{
  check_autoinit;

  gks_select_xform(transform);
}

void gr_setclip(int indicator)
{
  check_autoinit;

  gks_set_clipping(indicator);
}

static
void wswindow(int workstation_id, rect_t *rect)
{
  int wkid = workstation_id;

  gks_set_ws_window(wkid, rect->xmin, rect->xmax, rect->ymin, rect->ymax);
}

void gr_setwswindow(float xmin, float xmax, float ymin, float ymax)
{
  rect_t rect;

  rect.xmin = xmin;
  rect.xmax = xmax;
  rect.ymin = ymin;
  rect.ymax = ymax;

  check_autoinit;

  foreach_activews((void (*)(int, void *)) wswindow, (void *) &rect);
}

static
void wsviewport(int workstation_id, rect_t *rect)
{
  int wkid = workstation_id;

  gks_set_ws_viewport(wkid, rect->xmin, rect->xmax, rect->ymin, rect->ymax);
}

void gr_setwsviewport(float xmin, float xmax, float ymin, float ymax)
{
  rect_t rect;

  rect.xmin = xmin;
  rect.xmax = xmax;
  rect.ymin = ymin;
  rect.ymax = ymax;

  check_autoinit;

  foreach_activews((void (*)(int, void *)) wsviewport, (void *) &rect);
}

void gr_createseg(int segment)
{
  check_autoinit;

  gks_create_seg(segment);
}

static
void copyseg(int workstation_id, int *segment)
{
  int wkid = workstation_id, errind, conid, wtype, wkcat;

  gks_inq_ws_conntype(wkid, &errind, &conid, &wtype);
  gks_inq_ws_category(wtype, &errind, &wkcat);

  if (wkcat == GKS_K_WSCAT_OUTPUT || wkcat == GKS_K_WSCAT_OUTIN)
    {
      gks_copy_seg_to_ws(wkid, *segment);
      gks_update_ws(wkid, GKS_K_POSTPONE_FLAG);
    }
}

void gr_copysegws(int segment)
{
  int segn = segment;

  check_autoinit;

  foreach_activews((void (*)(int, void *)) copyseg, (void *) &segn);
}

static
void redrawseg(int workstation_id, void *foo)
{
  int wkid = workstation_id;

  gks_redraw_seg_on_ws(wkid);
}

void gr_redrawsegws(void)
{
  check_autoinit;

  foreach_activews((void (*)(int, void *)) redrawseg, (void *) NULL);
}

void gr_setsegtran(
  int segment, float fx, float fy, float transx, float transy, float phi,
  float scalex, float scaley)
{
  int segn = segment;
  float mat[3][2];

  check_autoinit;

  gks_eval_xform_matrix(fx, fy, transx, transy, phi, scalex, scaley,
			GKS_K_COORDINATES_NDC, mat);
  gks_set_seg_xform(segn, mat);
}

void gr_closeseg(void)
{
  check_autoinit;

  gks_close_seg();
}

void gr_emergencyclosegks(void)
{
  gks_emergency_close();
  autoinit = 1;
}

void gr_updategks(void)
{
  int state, count, n, errind, ol;
  int wkid, conid, wtype, wkcat;

  gks_inq_operating_state(&state);
  if (state >= GKS_K_WSOP)
    {
      n = 1;
      gks_inq_open_ws(n, &errind, &ol, &wkid);

      for (count = 1; count <= ol; count++)
	{
	  n = count;
	  gks_inq_open_ws(n, &errind, &ol, &wkid);

	  gks_inq_ws_conntype(wkid, &errind, &conid, &wtype);
	  gks_inq_ws_category(wtype, &errind, &wkcat);

	  if (wkcat == GKS_K_WSCAT_OUTPUT || wkcat == GKS_K_WSCAT_OUTIN)
	    gks_update_ws(wkid, GKS_K_POSTPONE_FLAG);
	}
    }
}

int gr_setspace(float zmin, float zmax, int rotation, int tilt)
{
  int errind, tnr;
  float wn[4], vp[4];
  float xmin, xmax, ymin, ymax, r, t, a, c;

  if (zmin < zmax)
    {
      if (rotation < 0 || rotation > 90 || tilt < 0 || tilt > 90)
	return -1;
    }
  else
    return -1;

  check_autoinit;

  if (accel)
    if (wx.zmin != zmin || wx.zmax != zmax ||
	wx.phi != rotation || wx.delta != tilt)
      gr_setproperty("zmin: %g; zmax: %g; rotation: %d; tilt: %d;",
		     zmin, zmax, rotation, tilt);

  gks_inq_current_xformno(&errind, &tnr);
  gks_inq_xform(tnr, &errind, wn, vp);

  xmin = wn[0];
  xmax = wn[1];
  ymin = wn[2];
  ymax = wn[3];

  wx.zmin = zmin;
  wx.zmax = zmax;
  wx.phi = rotation;
  wx.delta = tilt;

  r = arc(rotation);
  wx.a1 = cos(r);
  wx.a2 = sin(r);

  a = (xmax - xmin) / (wx.a1 + wx.a2);
  wx.b = xmin;

  wx.a1 = a * wx.a1 / (xmax - xmin);
  wx.a2 = a * wx.a2 / (ymax - ymin);
  wx.b = wx.b - wx.a1 * xmin - wx.a2 * ymin;

  t = arc(tilt);
  wx.c1 = (pow(cos(r), 2.) - 1.) * tan(t / 2.);
  wx.c2 = -(pow(sin(r), 2.) - 1.) * tan(t / 2.);
  wx.c3 = cos(t);

  c = (ymax - ymin) / (wx.c2 + wx.c3 - wx.c1);
  wx.d = ymin - c * wx.c1;

  wx.c1 = c * wx.c1 / (xmax - xmin);
  wx.c2 = c * wx.c2 / (ymax - ymin);
  wx.c3 = c * wx.c3 / (zmax - zmin);
  wx.d = wx.d - wx.c1 * xmin - wx.c2 * ymin - wx.c3 * zmin;

  return 0;
}

void gr_inqspace(float *zmin, float *zmax, int *rotation, int *tilt)
{
  *zmin = wx.zmin;
  *zmax = wx.zmax;
  *rotation = wx.phi;
  *tilt = wx.delta;
}

static
int iround(float x)
{
  if (x < 0)
    return ((int) (x - 0.5));
  else
    return ((int) (x + 0.5));
}

static
int gauss(float x)
{
  if (x >= 0 || x == (int) x)
    return ((int) x);
  else
    return ((int) x - 1);
}

static
int ipred(float x)
{
  if (x == (int) x)
    return ((int) x - 1);
  else
    return (gauss(x));
}

static
int isucc(float x)
{
  if (x == (int) x)
    return ((int) x);
  else
    return (gauss(x) + 1);
}

static
void end_pline(void)
{
  if (npoints >= 2)
    {
      gks_polyline(npoints, xpoint, ypoint);
      npoints = 0;
    }
}

static
void pline(float x, float y)
{
  if (npoints >= maxpoints)
    reallocate(npoints);

  xpoint[npoints] = x_lin(x);
  ypoint[npoints] = y_lin(y);
  npoints++;
}

static
void start_pline(float x, float y)
{
  end_pline();

  npoints = 0;
  pline(x, y);
}

static
void pline3d(float x, float y, float z)
{
  if (npoints >= maxpoints)
    reallocate(npoints);

  xpoint[npoints] = x_lin(x);
  ypoint[npoints] = y_lin(y);
  zpoint[npoints] = z_lin(z);

  apply_world_xform(xpoint + npoints, ypoint + npoints, zpoint + npoints);

  npoints++;
}

static
void start_pline3d(float x, float y, float z)
{
  end_pline();

  npoints = 0;
  pline3d(x, y, z);
}

int gr_textext(float x, float y, char *string)
{
  int errind, tnr, result;

  check_autoinit;

  gks_inq_current_xformno(&errind, &tnr);
  if (tnr != NDC)
    gks_select_xform(NDC);

  result = gr_textex(x, y, string, 0, NULL, NULL);

  if (tnr != NDC)
    gks_select_xform(tnr);

  if (flag_graphics)
    fprintf(stream, "<textex x='%g' y='%g' text='%s'/>\n", x, y, string);

  return result;
}

void gr_inqtextext(float x, float y, char *string, float *tbx, float *tby)
{
  int errind, tnr;
  register int i;

  check_autoinit;

  gks_inq_current_xformno(&errind, &tnr);
  if (tnr != NDC)
    gks_select_xform(NDC);

  gr_textex(x, y, string, 1, tbx, tby);

  if (tnr != NDC)
    {
      gks_select_xform(tnr);

      for (i = 0; i < 4; i++)
	{
	  tbx[i] = (tbx[i] - nx.b) / nx.a;
	  tby[i] = (tby[i] - nx.d) / nx.c;
	  if (lx.scale_options)
	    {
	      tbx[i] = x_log(tbx[i]);
	      tby[i] = y_log(tby[i]);
	    }
	}
    }
}

static
void text2d(float x, float y, char *chars)
{
  int errind, tnr;

  if (lx.scale_options)
    {
      x = x_lin(x);
      y = y_lin(y);
    }

  gks_inq_current_xformno(&errind, &tnr);
  if (tnr != NDC)
    {
      x = nx.a * x + nx.b;
      y = nx.c * y + nx.d;
      gks_select_xform(NDC);
    }

  gr_textex(x, y, chars, 0, NULL, NULL);

  if (tnr != NDC)
    gks_select_xform(tnr);
}

void gr_axes(float x_tick, float y_tick, float x_org, float y_org,
	     int major_x, int major_y, float tick_size)
{
  int errind, tnr;
  int ltype, halign, valign, clsw;
  float chux, chuy;

  float clrt[4], wn[4], vp[4];
  float x_min, x_max, y_min, y_max;

  float tick, minor_tick, major_tick, x_label, y_label, x0, y0, xi, yi;
  int decade, exponent, i;
  char string[256];

  if (x_tick < 0 || y_tick < 0)
    {
      fprintf(stderr, "invalid interval length for major tick-marks\n");
      return;
    }
  else if (tick_size == 0)
    {
      fprintf(stderr, "invalid tick-size\n");
      return;
    }

  check_autoinit;

  /* inquire current normalization transformation */

  gks_inq_current_xformno(&errind, &tnr);
  gks_inq_xform(tnr, &errind, wn, vp);

  x_min = wn[0];
  x_max = wn[1];
  y_min = wn[2];
  y_max = wn[3];

  if (x_min > x_org || x_org > x_max || y_min > y_org || y_org > y_max)
    {
      fprintf(stderr, "origin outside current window\n");
      return;
    }

  /* save linetype, text alignment, character-up vector and
     clipping indicator */

  gks_inq_pline_linetype(&errind, &ltype);
  gks_inq_text_align(&errind, &halign, &valign);
  gks_inq_text_upvec(&errind, &chux, &chuy);
  gks_inq_clip(&errind, &clsw, clrt);

  gks_set_pline_linetype(GKS_K_LINETYPE_SOLID);
  gks_set_text_upvec(0, 1);
  gks_set_clipping(GKS_K_NOCLIP);

  if (y_tick != 0)
    {
      tick = tick_size * (x_max - x_min) / (vp[1] - vp[0]);

      minor_tick = x_log(x_lin(x_org) + tick);
      major_tick = x_log(x_lin(x_org) + 2. * tick);
      x_label = x_log(x_lin(x_org) + 3. * tick);

      /* set text alignment */

      if (x_lin(x_org) <= (x_lin(x_min) + x_lin(x_max)) / 2.)
	{
	  gks_set_text_align(GKS_K_TEXT_HALIGN_RIGHT, GKS_K_TEXT_VALIGN_HALF);

	  if (tick > 0)
	    x_label = x_log(x_lin(x_org) - tick);
	}
      else
	{
	  gks_set_text_align(GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);

	  if (tick < 0)
	    x_label = x_log(x_lin(x_org) - tick);
	}

      if (OPTION_Y_LOG & lx.scale_options)
	{
	  y0 = pow(10.0, (double) gauss(log10(y_min)));

	  i = ipred((float) (y_min / y0));
	  yi = y0 + i * y0;
	  decade = 0;

	  /* draw Y-axis */

	  start_pline(x_org, y_min);

	  while (yi <= y_max * (1.0 + FEPS))
	    {
	      pline(x_org, yi);

	      if (i == 0)
		{
		  xi = major_tick;

		  if (major_y > 0)
		    if (decade % major_y == 0)
		      if (yi != y_org || y_org == y_min || y_org == y_max)
			{
			  if (y_tick > 1)
			    {
			      exponent = iround(log10(yi));
			      sprintf(string, "10^{%d}", exponent);
			      text2d(x_label, yi, string);
			    }
			  else
			    text2d(x_label, yi, str_ftoa(string, yi, 0.));
			}
		}
	      else
		xi = minor_tick;

	      if (i == 0 || abs(major_y) == 1)
		{
		  pline(xi, yi);
		  pline(x_org, yi);
		}

	      if (i == 9)
		{
		  y0 = y0 * 10.;
		  i = 0;
		  decade++;
		}
	      else
		i++;

	      yi = y0 + i * y0;
	    }

	  if (yi > y_max * (1.0 + FEPS))
	    pline(x_org, y_max);

	  end_pline();
	}
      else
	{
	  i = isucc((float) (y_min / y_tick));
	  yi = i * y_tick;

	  /* draw Y-axis */

	  start_pline(x_org, y_min);

	  while (yi <= y_max * (1.0 + FEPS))
	    {
	      pline(x_org, yi);

	      if (major_y != 0)
		{
		  if (i % major_y == 0)
		    {
		      xi = major_tick;
		      if (yi != y_org || y_org == y_min || y_org == y_max)
			if (major_y > 0)
			  text2d(x_label, yi,
			         str_ftoa(string, yi, y_tick * major_y));
		    }
		  else
		    xi = minor_tick;
		}
	      else
		xi = major_tick;

	      pline(xi, yi);
	      pline(x_org, yi);

	      i++;
	      yi = i * y_tick;
	    }

	  if (yi > y_max * (1.0 + FEPS))
	    pline(x_org, y_max);

	  end_pline();
	}
    }

  if (x_tick != 0)
    {
      tick = tick_size * (y_max - y_min) / (vp[3] - vp[2]);

      minor_tick = y_log(y_lin(y_org) + tick);
      major_tick = y_log(y_lin(y_org) + 2. * tick);
      y_label = y_log(y_lin(y_org) + 3. * tick);

      /* set text alignment */

      if (y_lin(y_org) <= (y_lin(y_min) + y_lin(y_max)) / 2.)
	{
	  gks_set_text_align(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);

	  if (tick > 0)
	    y_label = y_log(y_lin(y_org) - tick);
	}
      else
	{
	  gks_set_text_align(GKS_K_TEXT_HALIGN_CENTER,
			     GKS_K_TEXT_VALIGN_BOTTOM);

	  if (tick < 0)
	    y_label = y_log(y_lin(y_org) - tick);
	}

      if (OPTION_X_LOG & lx.scale_options)
	{
	  x0 = pow(10.0, (double) gauss(log10(x_min)));

	  i = ipred((float) (x_min / x0));
	  xi = x0 + i * x0;
	  decade = 0;

	  /* draw X-axis */

	  start_pline(x_min, y_org);

	  while (xi <= x_max * (1.0 + FEPS))
	    {
	      pline(xi, y_org);

	      if (i == 0)
		{
		  yi = major_tick;

		  if (major_x > 0)
		    if (decade % major_x == 0)
		      if (xi != x_org || x_org == x_min || x_org == x_max)
			{
			  if (x_tick > 1)
			    {
			      exponent = iround(log10(xi));
			      sprintf(string, "10^{%d}", exponent);
			      text2d(xi, y_label, string);
			    }
			  else
			    text2d(xi, y_label, str_ftoa(string, xi, 0.));
			}
		}
	      else
		yi = minor_tick;

	      if (i == 0 || abs(major_x) == 1)
		{
		  pline(xi, yi);
		  pline(xi, y_org);
		}

	      if (i == 9)
		{
		  x0 = x0 * 10.;
		  i = 0;
		  decade++;
		}
	      else
		i++;

	      xi = x0 + i * x0;
	    }

	  if (xi > x_max * (1.0 + FEPS))
	    pline(x_max, y_org);

	  end_pline();
	}
      else
	{
	  i = isucc((float) (x_min / x_tick));
	  xi = i * x_tick;

	  /* draw X-axis */

	  start_pline(x_min, y_org);

	  while (xi <= x_max * (1.0 + FEPS))
	    {
	      pline(xi, y_org);

	      if (major_x != 0)
		{
		  if (i % major_x == 0)
		    {
		      yi = major_tick;
		      if (xi != x_org || x_org == x_min || x_org == x_max)
			if (major_x > 0)
			  text2d(xi, y_label,
			         str_ftoa(string, xi, x_tick * major_x));
		    }
		  else
		    yi = minor_tick;
		}
	      else
		yi = major_tick;

	      pline(xi, yi);
	      pline(xi, y_org);

	      i++;
	      xi = i * x_tick;
	    }

	  if (xi > x_max * (1.0 + FEPS))
	    pline(x_max, y_org);

	  end_pline();
	}
    }

  /* restore linetype, text alignment, character-up vector
     and clipping indicator */

  gks_set_pline_linetype(ltype);
  gks_set_text_align(halign, valign);
  gks_set_text_upvec(chux, chuy);
  gks_set_clipping(clsw);

  if (flag_graphics)
    fprintf(stream, "<axes x_tick='%g' y_tick='%g' x_org='%g' y_org='%g' "
	    "major_x='%d major_y='%d' ticksize='%g'/>\n",
	    x_tick, y_tick, x_org, y_org, major_x, major_y, tick_size);
}

static
void grid_line(float x0, float y0, float x1, float y1, float tick)
{
  int ltype = tick < 0 ? GKS_K_LINETYPE_SOLID : GKS_K_LINETYPE_DOTTED;

  gks_set_pline_linetype(ltype);

  start_pline(x0, y0);
  pline(x1, y1);
  end_pline();
}

void gr_grid(float x_tick, float y_tick, float x_org, float y_org,
	     int major_x, int major_y)
{
  int errind, tnr;
  int ltype, clsw;

  float clrt[4], wn[4], vp[4];
  float x_min, x_max, y_min, y_max;

  float x0, y0, xi, yi, tick;

  int i;

  if (x_tick < 0 || y_tick < 0)
    {
      fprintf(stderr, "invalid interval length for major tick-marks\n");
      return;
    }

  check_autoinit;

  /* inquire current normalization transformation */

  gks_inq_current_xformno(&errind, &tnr);
  gks_inq_xform(tnr, &errind, wn, vp);

  x_min = wn[0];
  x_max = wn[1];
  y_min = wn[2];
  y_max = wn[3];

  if (x_min > x_org || x_org > x_max || y_min > y_org || y_org > y_max)
    {
      fprintf(stderr, "origin outside current window\n");
      return;
    }

  /* save linetype and clipping indicator */

  gks_inq_pline_linetype(&errind, &ltype);
  gks_inq_clip(&errind, &clsw, clrt);

  gks_set_pline_linetype(GKS_K_LINETYPE_SOLID);
  gks_set_clipping(GKS_K_NOCLIP);

  if (y_tick != 0)
    {
      if (OPTION_Y_LOG & lx.scale_options)
	{
	  y0 = pow(10.0, (double) gauss(log10(y_min)));

	  i = ipred((float) (y_min / y0));
	  yi = y0 + i * y0;

	  /* draw horizontal grid lines */

	  while (yi <= y_max)
	    {
	      if (i == 0 || major_y == 1)
		{
		  if (i == 0)
		    tick = -1.;
		  else
		    tick = x_tick;

		  grid_line(x_min, yi, x_max, yi, tick);
		}

	      if (i == 9)
		{
		  y0 = y0 * 10.;
		  i = 0;
		}
	      else
		i++;

	      yi = y0 + i * y0;
	    }
	}
      else
	{
	  i = isucc((float) (y_min / y_tick));
	  yi = i * y_tick;

	  /* draw horizontal grid lines */

	  while (yi <= y_max)
	    {
	      if (major_y > 0)
		{
		  if (i % major_y == 0)
		    tick = -1.;
		  else
		    tick = y_tick;
		}
	      else
		tick = -1.;

	      grid_line(x_min, yi, x_max, yi, tick);

	      i++;
	      yi = i * y_tick;
	    }
	}
    }

  if (x_tick != 0)
    {
      if (OPTION_X_LOG & lx.scale_options)
	{
	  x0 = pow(10.0, (double) gauss(log10(x_min)));

	  i = ipred((float) (x_min / x0));
	  xi = x0 + i * x0;

	  /* draw vertical grid lines */

	  while (xi <= x_max)
	    {
	      if (i == 0 || major_x == 1)
		{
		  if (i == 0)
		    tick = -1.;
		  else
		    tick = x_tick;

		  grid_line(xi, y_min, xi, y_max, tick);
		}

	      if (i == 9)
		{
		  x0 = x0 * 10.;
		  i = 0;
		}
	      else
		i++;

	      xi = x0 + i * x0;
	    }
	}
      else
	{
	  i = isucc((float) (x_min / x_tick));
	  xi = i * x_tick;

	  /* draw vertical grid lines */

	  while (xi <= x_max)
	    {
	      if (major_x > 0)
		{
		  if (i % major_x == 0)
		    tick = -1.;
		  else
		    tick = x_tick;
		}
	      else
		tick = -1.;

	      grid_line(xi, y_min, xi, y_max, tick);

	      i++;
	      xi = i * x_tick;
	    }
	}
    }

  /* restore linetype and clipping indicator */

  gks_set_pline_linetype(ltype);
  gks_set_clipping(clsw);

  if (flag_graphics)
    fprintf(stream, "<grid x_tick='%g' y_tick='%g' x_org='%g' y_org='%g' "
	    "major_x='%d major_y='%d'/>\n",
	    x_tick, y_tick, x_org, y_org, major_x, major_y);
}

void gr_verrorbars(int n, float *px, float *py, float *e1, float *e2)
{
  int errind, i;
  float tick, x, x1, x2, y1, y2, marker_size;

  if (n < 1)
    {
      fprintf(stderr, "invalid number of points\n");
      return;
    }

  check_autoinit;

  gks_inq_pmark_size(&errind, &marker_size);

  for (i = 0; i < n; i++)
    {
      tick = marker_size * 0.0075 * (lx.xmax - lx.xmin);

      x = px[i];
      x1 = x_log(x_lin(x) - tick);
      x2 = x_log(x_lin(x) + tick);
      y1 = e1[i];
      y2 = e2[i];

      start_pline(x1, y1);
      pline(x2, y1);
      end_pline ();

      start_pline(x, y1);
      pline (x, y2);
      end_pline ();

      start_pline(x1, y2);
      pline (x2, y2);
      end_pline();
    }

  gr_polymarker(n, px, py);

  if (flag_graphics)
    {
      fprintf(stream, "<verrorbars len='%d'", n);
      print_float_array("x", n, px);
      print_float_array("y", n, py);
      print_float_array("e1", n, e1);
      print_float_array("e2", n, e2);
      fprintf(stream, "/>\n");
    }
}

void gr_herrorbars(int n, float *px, float *py, float *e1, float *e2)
{
  int errind, i;
  float tick, y, x1, x2, y1, y2, marker_size;

  if (n < 1)
    {
      fprintf(stderr, "invalid number of points\n");
      return;
    }

  check_autoinit;

  gks_inq_pmark_size(&errind, &marker_size);

  for (i = 0; i < n; i++)
    {
      tick = marker_size * 0.0075 * (lx.ymax - lx.ymin);

      y = py[i];
      y1 = y_log(y_lin(y) - tick);
      y2 = y_log(y_lin(y) + tick);
      x1 = e1[i];
      x2 = e2[i];

      start_pline(x1, y1);
      pline(x1, y2);
      end_pline();

      start_pline(x1, y);
      pline(x2, y);
      end_pline();

      start_pline(x2, y1);
      pline(x2, y2);
      end_pline();
    }

  gr_polymarker(n, px, py);

  if (flag_graphics)
    {
      fprintf(stream, "<herrorbars len='%d'", n);
      print_float_array("x", n, px);
      print_float_array("y", n, py);
      print_float_array("e1", n, e1);
      print_float_array("e2", n, e2);
      fprintf(stream, "/>\n");
    }
}

static
void clip_code(float x, float y, float z, int *c)
{
  *c = 0;
  if (x < cxl)
    *c = LEFT;
  else if (x > cxr)
    *c = RIGHT;
  if (y < cyf)
    *c = *c | FRONT;
  else if (y > cyb)
    *c = *c | BACK;
  if (z < czb)
    *c = *c | BOTTOM;
  else if (z > czt)
    *c = *c | TOP;
}

static
void clip3d(float *x0, float *x1, float *y0, float *y1, float *z0,
	    float *z1, int *visible)
{
  int c, c0, c1;
  float x = 0, y = 0, z = 0;

  clip_code(*x0, *y0, *z0, &c0);
  clip_code(*x1, *y1, *z1, &c1);

  *visible = 0;

  while (c0 | c1)
    {
      if (c0 & c1)
	return;
      c = c0 ? c0 : c1;

      if (c & LEFT)
	{
	  x = cxl;
	  y = *y0 + (*y1 - *y0) * (cxl - *x0) / (*x1 - *x0);
	  z = *z0 + (*z1 - *z0) * (cxl - *x0) / (*x1 - *x0);
	}
      else if (c & RIGHT)
	{
	  x = cxr;
	  y = *y0 + (*y1 - *y0) * (cxr - *x0) / (*x1 - *x0);
	  z = *z0 + (*z1 - *z0) * (cxr - *x0) / (*x1 - *x0);
	}
      else if (c & FRONT)
	{
	  x = *x0 + (*x1 - *x0) * (cyf - *y0) / (*y1 - *y0);
	  y = cyf;
	  z = *z0 + (*z1 - *z0) * (cyf - *y0) / (*y1 - *y0);
	}
      else if (c & BACK)
	{
	  x = *x0 + (*x1 - *x0) * (cyb - *y0) / (*y1 - *y0);
	  y = cyb;
	  z = *z0 + (*z1 - *z0) * (cyb - *y0) / (*y1 - *y0);
	}
      else if (c & BOTTOM)
	{
	  x = *x0 + (*x1 - *x0) * (czb - *z0) / (*z1 - *z0);
	  y = *y0 + (*y1 - *y0) * (czb - *z0) / (*z1 - *z0);
	  z = czb;
	}
      else if (c & TOP)
	{
	  x = *x0 + (*x1 - *x0) * (czt - *z0) / (*z1 - *z0);
	  y = *y0 + (*y1 - *y0) * (czt - *z0) / (*z1 - *z0);
	  z = czt;
	}
      if (c == c0)
	{
	  *x0 = x;
	  *y0 = y;
	  *z0 = z;
	  clip_code(x, y, z, &c0);
	}
      else
	{
	  *x1 = x;
	  *y1 = y;
	  *z1 = z;
	  clip_code(x, y, z, &c1);
	}
    }

  /* if we reach here, the line from (x0, y0, z0) to (x1, y1, z1) is visible */

  *visible = 1;
}

void gr_polyline3d(int n, float *px, float *py, float *pz)
{
  int errind, clsw, i;
  float clrt[4];

  float x, y, z, x0, y0, z0, x1, y1, z1;
  int clip = 1, visible = 1;

  check_autoinit;

  gr_setscale(lx.scale_options);

  gks_inq_clip(&errind, &clsw, clrt);

  if (clsw == GKS_K_CLIP)
    {
      cxl = lx.xmin;
      cxr = lx.xmax;
      cyf = lx.ymin;
      cyb = lx.ymax;
      czb = lx.zmin;
      czt = lx.zmax;
    }
  else
    {
      visible = 1;
    }

  x0 = px[0];
  y0 = py[0];
  z0 = pz[0];

  for (i = 1; i < n; i++)
    {
      x1 = px[i];
      y1 = py[i];
      z1 = pz[i];

      x = x1;
      y = y1;
      z = z1;
      if (clsw == GKS_K_CLIP)
	clip3d(&x0, &x1, &y0, &y1, &z0, &z1, &visible);

      if (visible)
	{
	  if (clip)
	    {
	      start_pline3d(x0, y0, z0);
	      clip = 0;
	    }
	  pline3d(x1, y1, z1);
	}

      clip = !visible || x != x1 || y != y1 || z != z1;
      x0 = x;
      y0 = y;
      z0 = z;
    }

  end_pline();
}

static
void text3d(float x, float y, float z, char *chars)
{
  int errind, tnr;

  x = x_lin(x);
  y = y_lin(y);
  z = z_lin(z);
  apply_world_xform(&x, &y, &z);

  gks_inq_current_xformno(&errind, &tnr);
  if (tnr != NDC)
    {
      x = nx.a * x + nx.b;
      y = nx.c * y + nx.d;
      gks_select_xform(NDC);
    }

  gr_textex(x, y, chars, 0, NULL, NULL);

  if (tnr != NDC)
    gks_select_xform(tnr);
}

void gr_axes3d(float x_tick, float y_tick, float z_tick,
	       float x_org, float y_org, float z_org,
	       int major_x, int major_y, int major_z, float tick_size)
{
  int errind, tnr;
  int ltype, halign, valign, font, prec, clsw;
  float chux, chuy, slant;

  float clrt[4], wn[4], vp[4];
  float x_min, x_max, y_min, y_max, z_min, z_max;

  float r, alpha, beta;
  float a[2], c[2], text_slant[4];
  int *anglep, which_rep, rep;

  float tick, minor_tick, major_tick, x_label, y_label;
  float x0, y0, z0, xi, yi, zi;
  int i, decade, exponent;
  char string[256];

  if (x_tick < 0 || y_tick < 0 || z_tick < 0)
    {
      fprintf(stderr, "invalid interval length for major tick-marks\n");
      return;
    }
  else if (tick_size == 0)
    {
      fprintf(stderr, "invalid tick-size\n");
      return;
    }

  check_autoinit;

  gr_setscale(lx.scale_options);

  /* inquire current normalization transformation */

  gks_inq_current_xformno(&errind, &tnr);
  gks_inq_xform(tnr, &errind, wn, vp);

  x_min = wn[0];
  x_max = wn[1];
  y_min = wn[2];
  y_max = wn[3];

  z_min = wx.zmin;
  z_max = wx.zmax;

  if (x_min > x_org || x_org > x_max || y_min > y_org || y_org > y_max ||
      z_min > z_org || z_org > z_max)
    {
      fprintf(stderr, "origin outside current window\n");
      return;
    }

  if (accel)
    {
      gr_setproperty("axes: 1; xtick: %g; ytick: %g; ztick: %g;",
		     x_tick, y_tick, z_tick);
      gr_setproperty("majorx: %d; majory: %d; majorz: %d; ticksize: %g;",
		     major_x, major_y, major_z, tick_size);
      if (accel > 1)
	return;
    }

  r = (x_max - x_min) / (y_max - y_min) * (vp[3] - vp[2]) / (vp[1] - vp[0]);

  alpha = atan_2(r * wx.c1, wx.a1);
  a[0] = -sin(alpha);
  c[0] = cos(alpha);
  alpha = deg(alpha);

  beta = atan_2(r * wx.c2, wx.a2);
  a[1] = -sin(beta);
  c[1] = cos(beta);
  beta = deg(beta);

  text_slant[0] = alpha;
  text_slant[1] = beta;
  text_slant[2] = - (90.0 + alpha - beta);
  text_slant[3] = 90.0 + alpha - beta;

  /* save linetype, text alignment, text font, text slant,
     character-up vector and clipping indicator */

  gks_inq_pline_linetype(&errind, &ltype);
  gks_inq_text_align(&errind, &halign, &valign);
  gks_inq_text_fontprec(&errind, &font, &prec);
  gks_inq_text_slant(&errind, &slant);
  gks_inq_text_upvec(&errind, &chux, &chuy);
  gks_inq_clip(&errind, &clsw, clrt);

  gks_set_pline_linetype(GKS_K_LINETYPE_SOLID);
  gks_set_text_fontprec(font, GKS_K_TEXT_PRECISION_STROKE);
  gks_set_clipping(GKS_K_NOCLIP);

  which_rep = 0;
  anglep = angle;
  while (wx.delta > *anglep++)
    which_rep++;
  anglep = angle;
  while (wx.phi > *anglep++)
    which_rep += 4;

  if (z_tick != 0)
    {
      tick = tick_size * (y_max - y_min) / (vp[3] - vp[2]);

      minor_tick = y_log(y_lin(y_org) + tick);
      major_tick = y_log(y_lin(y_org) + 2. * tick);
      y_label = y_log(y_lin(y_org) + 3. * tick);

      /* set text alignment */

      if (y_lin(y_org) <= (y_lin(y_min) + y_lin(y_max)) / 2.)
	{
	  gks_set_text_align(GKS_K_TEXT_HALIGN_RIGHT, GKS_K_TEXT_VALIGN_HALF);

	  if (tick > 0)
	    y_label = y_log(y_lin(y_org) - tick);
	}
      else
	{
	  gks_set_text_align(GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);

	  if (tick < 0)
	    y_label = y_log(y_lin(y_org) - tick);
	}

      rep = rep_table[which_rep][2];

      gks_set_text_upvec(a[axes_rep[rep][0]], c[axes_rep[rep][1]]);
      gks_set_text_slant(text_slant[axes_rep[rep][2]]);

      if (OPTION_Z_LOG & lx.scale_options)
	{
	  z0 = pow(10.0, (double) gauss(log10(z_min)));

	  i = ipred((float) (z_min / z0));
	  zi = z0 + i * z0;
	  decade = 0;

	  /* draw Z-axis */

	  start_pline3d(x_org, y_org, z_min);

	  while (zi <= z_max && !flag_abort)
	    {
	      pline3d(x_org, y_org, zi);

	      if (i == 0)
		{
		  yi = major_tick;
		  if (major_z > 0 && zi != z_org)
		    if (decade % major_z == 0)
		      {
			if (z_tick > 1)
			  {
			    exponent = iround(log10(zi));
			    sprintf(string, "10^{%d}", exponent);
			    text3d(x_org, y_label, zi, string);
			  }
			else
			  text3d(x_org, y_label, zi, str_ftoa(string, zi, 0.));
		      }
		}
	      else
		yi = minor_tick;

	      if (i == 0 || abs(major_z) == 1)
		{
		  pline3d(x_org, yi, zi);
		  pline3d(x_org, y_org, zi);
		}

	      if (i == 9)
		{
		  z0 = z0 * 10.;
		  i = 0;
		  decade++;
		}
	      else
		i++;

	      zi = z0 + i * z0;
	    }

	  pline3d(x_org, y_org, z_max);

	  end_pline();
	}
      else
	{
	  i = isucc((float) (z_min / z_tick));
	  zi = i * z_tick;

	  /* draw Z-axis */

	  start_pline3d(x_org, y_org, z_min);

	  while (zi <= z_max && !flag_abort)
	    {
	      pline3d(x_org, y_org, zi);

	      if (major_z != 0)
		{
		  if (i % major_z == 0)
		    {
		      yi = major_tick;
		      if ((zi != z_org) && (major_z > 0))
			text3d(x_org, y_label, zi,
			       str_ftoa(string, zi, z_tick * major_z));
		    }
		  else
		    yi = minor_tick;
		}
	      else
		yi = major_tick;

	      pline3d(x_org, yi, zi);
	      pline3d(x_org, y_org, zi);

	      i++;
	      zi = i * z_tick;
	    }

	  if (zi > z_max)
	    pline3d(x_org, y_org, z_max);

	  end_pline();
	}
    }

  if (y_tick != 0)
    {
      tick = tick_size * (x_max - x_min) / (vp[1] - vp[0]);

      minor_tick = x_log(x_lin(x_org) + tick);
      major_tick = x_log(x_lin(x_org) + 2. * tick);
      x_label = x_log(x_lin(x_org) + 3. * tick);

      /* set text alignment */

      if (x_lin(x_org) <= (x_lin(x_min) + x_lin(x_max)) / 2.)
	{
	  gks_set_text_align(GKS_K_TEXT_HALIGN_RIGHT, GKS_K_TEXT_VALIGN_HALF);

	  if (tick > 0)
	    x_label = x_log(x_lin(x_org) - tick);
	}
      else
	{
	  gks_set_text_align(GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);

	  if (tick < 0)
	    x_label = x_log(x_lin(x_org) - tick);
	}

      rep = rep_table[which_rep][1];
      if (rep == 0)
	gks_set_text_align(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);

      gks_set_text_upvec(a[axes_rep[rep][0]], c[axes_rep[rep][1]]);
      gks_set_text_slant(text_slant[axes_rep[rep][2]]);

      if (OPTION_Y_LOG & lx.scale_options)
	{
	  y0 = pow(10.0, (double) gauss(log10(y_min)));

	  i = ipred((float) (y_min / y0));
	  yi = y0 + i * y0;
	  decade = 0;

	  /* draw Y-axis */

	  start_pline3d(x_org, y_min, z_org);

	  while (yi <= y_max && !flag_abort)
	    {
	      pline3d(x_org, yi, z_org);

	      if (i == 0)
		{
		  xi = major_tick;
		  if (major_y > 0 && yi != y_org)
		    if (decade % major_y == 0)
		      {
			if (y_tick > 1)
			  {
			    exponent = iround(log10(yi));
			    sprintf(string, "10^{%d}", exponent);
			    text3d(x_label, yi, z_org, string);
			  }
			else
			  text3d(x_label, yi, z_org, str_ftoa(string, yi, 0.));
		      }
		}
	      else
		xi = minor_tick;

	      if (i == 0 || abs(major_y) == 1)
		{
		  pline3d(xi, yi, z_org);
		  pline3d(x_org, yi, z_org);
		}

	      if (i == 9)
		{
		  y0 = y0 * 10.;
		  i = 0;
		  decade++;
		}
	      else
		i++;

	      yi = y0 + i * y0;
	    }

	  pline3d(x_org, y_max, z_org);

	  end_pline();
	}
      else
	{
	  i = isucc((float) (y_min / y_tick));
	  yi = i * y_tick;

	  /* draw Y-axis */

	  start_pline3d(x_org, y_min, z_org);

	  while (yi <= y_max && !flag_abort)
	    {
	      pline3d(x_org, yi, z_org);

	      if (major_y != 0)
		{
		  if (i % major_y == 0)
		    {
		      xi = major_tick;
		      if ((yi != y_org) && (major_y > 0))
			text3d(x_label, yi, z_org,
			       str_ftoa(string, yi, y_tick * major_y));
		    }
		  else
		    xi = minor_tick;
		}
	      else
		xi = major_tick;

	      pline3d(xi, yi, z_org);
	      pline3d(x_org, yi, z_org);

	      i++;
	      yi = i * y_tick;
	    }

	  if (yi > y_max)
	    pline3d(x_org, y_max, z_org);

	  end_pline();
	}
    }

  if (x_tick != 0)
    {
      tick = tick_size * (y_max - y_min) / (vp[3] - vp[2]);

      minor_tick = y_log(y_lin(y_org) + tick);
      major_tick = y_log(y_lin(y_org) + 2. * tick);
      y_label = y_log(y_lin(y_org) + 3. * tick);

      /* set text alignment */

      if (y_lin(y_org) <= (y_lin(y_min) + y_lin(y_max)) / 2.)
	{
	  gks_set_text_align(GKS_K_TEXT_HALIGN_RIGHT, GKS_K_TEXT_VALIGN_HALF);

	  if (tick > 0)
	    y_label = y_log(y_lin(y_org) - tick);
	}
      else
	{
	  gks_set_text_align(GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);

	  if (tick < 0)
	    y_label = y_log(y_lin(y_org) - tick);
	}

      rep = rep_table[which_rep][0];
      if (rep == 2)
	gks_set_text_align(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);

      gks_set_text_upvec(a[axes_rep[rep][0]], c[axes_rep[rep][1]]);
      gks_set_text_slant(text_slant[axes_rep[rep][2]]);

      if (OPTION_X_LOG & lx.scale_options)
	{
	  x0 = pow(10.0, (double) gauss(log10(x_min)));

	  i = ipred((float) (x_min / x0));
	  xi = x0 + i * x0;
	  decade = 0;

	  /* draw X-axis */

	  start_pline3d(x_min, y_org, z_org);

	  while (xi <= x_max && !flag_abort)
	    {
	      pline3d(xi, y_org, z_org);

	      if (i == 0)
		{
		  yi = major_tick;
		  if (major_x > 0 && xi != x_org)
		    if (decade % major_x == 0)
		      {
			if (x_tick > 1)
			  {
			    exponent = iround(log10(xi));
			    sprintf(string, "10^{%d}", exponent);
			    text3d(xi, y_label, z_org, string);
			  }
			else
			  text3d(xi, y_label, z_org, str_ftoa(string, xi, 0.));
		      }
		}
	      else
		yi = minor_tick;

	      if (i == 0 || abs(major_x) == 1)
		{
		  pline3d(xi, yi, z_org);
		  pline3d(xi, y_org, z_org);
		}

	      if (i == 9)
		{
		  x0 = x0 * 10.;
		  i = 0;
		  decade++;
		}
	      else
		i++;

	      xi = x0 + i * x0;
	    }

	  pline3d(x_max, y_org, z_org);

	  end_pline();
	}
      else
	{
	  i = isucc((float) (x_min / x_tick));
	  xi = i * x_tick;

	  /* draw X-axis */

	  start_pline3d(x_min, y_org, z_org);

	  while (xi <= x_max && !flag_abort)
	    {
	      pline3d(xi, y_org, z_org);

	      if (major_x != 0)
		{
		  if (i % major_x == 0)
		    {
		      yi = major_tick;
		      if ((xi != x_org) && (major_x > 0))
			text3d(xi, y_label, z_org,
			       str_ftoa(string, xi, x_tick * major_x));
		    }
		  else
		    yi = minor_tick;
		}
	      else
		yi = major_tick;

	      pline3d(xi, yi, z_org);
	      pline3d(xi, y_org, z_org);

	      i++;
	      xi = i * x_tick;
	    }

	  if (xi > x_max)
	    pline3d(x_max, y_org, z_org);

	  end_pline();
	}
    }

  /* restore linetype, text alignment, text font, text slant,
     character-up vector and clipping indicator */

  gks_set_pline_linetype(ltype);
  gks_set_text_align(halign, valign);
  gks_set_text_fontprec(font, prec);
  gks_set_text_slant(slant);
  gks_set_text_upvec(chux, chuy);
  gks_set_clipping(clsw);
}

void gr_titles3d(char *x_title, char *y_title, char *z_title)
{
  int errind, tnr;
  int halign, valign, clsw, font, prec;
  float chux, chuy;

  float clrt[4], wn[4], vp[4];
  float x_min, x_max, y_min, y_max, z_min, z_max;
  float x_rel, y_rel, z_rel, x, y, z;

  float r, t, alpha, beta;
  float a[2], c[2];

  float slant, text_slant[4];
  int *anglep, which_rep, rep;

  float x_2d, y_2d, x_2d_max, y_2d_max;
  float x_angle, y_angle;
  float x_mid_x, x_mid_y, y_mid_x, y_mid_y;
  float a1, a2, c1, c2, c3, aa, cc;
  float xr, yr, zr;

  int flip_x, flip_y, flip_z;

  check_autoinit;

  gr_setscale(lx.scale_options);

  if (accel)
    {
      gr_setproperty("xlabel: \"%s\"; ylabel: \"%s\"; zlabel: \"%s\";",
		      x_title, y_title, z_title);
      if (accel > 1)
	return;
    }

  /* inquire current normalization transformation */

  gks_inq_current_xformno(&errind, &tnr);
  gks_inq_xform(tnr, &errind, wn, vp);

  x_min = wn[0];
  x_max = wn[1];
  y_min = wn[2];
  y_max = wn[3];

  z_min = wx.zmin;
  z_max = wx.zmax;

  r = (x_max - x_min) / (y_max - y_min) * (vp[3] - vp[2]) / (vp[1] - vp[0]);

  alpha = atan_2(r * wx.c1, wx.a1);
  a[0] = -sin(alpha);
  c[0] = cos(alpha);
  alpha = deg(alpha);

  beta = atan_2(r * wx.c2, wx.a2);
  a[1] = -sin(beta);
  c[1] = cos(beta);
  beta = deg(beta);

  text_slant[0] = alpha;
  text_slant[1] = beta;
  text_slant[2] = -(90.0 + alpha - beta);
  text_slant[3] = 90.0 + alpha - beta;

  /* save text alignment, text font, text slant,
     character-up vector and clipping indicator */

  gks_inq_text_align(&errind, &halign, &valign);
  gks_inq_text_fontprec(&errind, &font, &prec);
  gks_inq_text_slant(&errind, &slant);
  gks_inq_text_upvec(&errind, &chux, &chuy);
  gks_inq_clip(&errind, &clsw, clrt);

  gks_set_pline_linetype(GKS_K_LINETYPE_SOLID);
  gks_set_text_fontprec(font, GKS_K_TEXT_PRECISION_STROKE);
  gks_set_clipping(GKS_K_NOCLIP);

  which_rep = 0;
  anglep = angle;
  while (wx.delta > *anglep++)
    which_rep++;
  anglep = angle;
  while (wx.phi > *anglep++)
    which_rep += 4;

  r = arc(wx.phi);
  a1 = cos(r);
  a2 = sin(r);
  aa = a1 + a2;
  a1 = a1 / aa;
  a2 = a2 / aa;

  t = arc(wx.delta);
  c1 = (pow(cos(r), 2.) - 1) * tan(t / 2.);
  c2 = -(pow(sin(r), 2.) - 1) * tan(t / 2.);
  c3 = cos(t);
  cc = (c2 + c3 - c1);
  c1 = c1 / cc;
  c2 = c2 / cc;
  c3 = c3 / cc;

  a1 = fabs(a1) * (vp[1] - vp[0]);
  a2 = fabs(a2) * (vp[1] - vp[0]);
  c1 = fabs(c1) * (vp[3] - vp[2]);
  c2 = fabs(c2) * (vp[3] - vp[2]);
  c3 = fabs(c3) * (vp[3] - vp[2]);

  x_mid_x = vp[0] + a1 / 2.;
  x_mid_y = vp[2] + c1 / 2.;
  y_mid_x = 1 - vp[1] + a2 / 2.;
  y_mid_y = vp[2] + c2 / 2.;

  x_2d_max = sqrt(y_mid_x * y_mid_x + y_mid_y * y_mid_y);
  y_2d_max = sqrt(x_mid_x * x_mid_x + x_mid_y * x_mid_y);

  x_angle = atan_2(a1, c1);
  x_2d = y_mid_y / cos(x_angle);

  if (x_2d > x_2d_max)
    {
      x_angle = M_PI / 2. - x_angle;
      x_2d = y_mid_x / cos(x_angle);
    }

  xr = x_2d / (2 * sqrt(pow(c1, 2.) + pow(a1, 2.)));

  y_angle = atan_2(c2, a2);
  y_2d = x_mid_x / cos(y_angle);

  if (y_2d > y_2d_max)
    {
      y_angle = M_PI / 2. - y_angle;
      y_2d = x_mid_y / cos(y_angle);
    }

  if (wx.phi + wx.delta != 0)
    yr = y_2d / (2 * sqrt(pow(c2, 2.) + pow(a2, 2.)));
  else
    yr = 0;

  x_rel = xr * (x_lin(x_max) - x_lin(x_min));
  y_rel = yr * (y_lin(y_max) - y_lin(y_min));

  flip_x = OPTION_FLIP_X & lx.scale_options;
  flip_y = OPTION_FLIP_Y & lx.scale_options;
  flip_z = OPTION_FLIP_Z & lx.scale_options;

  if (*x_title)
    {
      rep = rep_table[which_rep][1];

      x = x_log(0.5 * (x_lin(x_min) + x_lin(x_max)));
      if (rep == 0)
	{
	  gks_set_text_align(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);

	  rep = rep_table[which_rep][0];

	  if (flip_y)
	    y = y_max;
	  else
	    y = y_min;

	  zr = x_mid_y / (2 * c3);
	  z_rel = zr * (z_lin(z_max) - z_lin(z_min));

	  if (flip_z)
	    z = z_log(z_lin(z_max) + z_rel);
	  else
	    z = z_log(z_lin(z_min) - z_rel);
	}
      else
	{
	  if (flip_y)
	    y = y_log(y_lin(y_max) + y_rel);
	  else
	    y = y_log(y_lin(y_min) - y_rel);

	  if (flip_z)
	    z = z_max;
	  else
	    z = z_min;

	  gks_set_text_align(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_HALF);
	}

      gks_set_text_upvec(a[axes_rep[rep][0]], c[axes_rep[rep][1]]);
      gks_set_text_slant(text_slant[axes_rep[rep][2]]);

      text3d(x, y, z, x_title);
    }

  if (*y_title)
    {
      rep = rep_table[which_rep][0];

      y = y_log(0.5 * (y_lin(y_min) + y_lin(y_max)));
      if (rep == 2)
	{
	  gks_set_text_align(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);

	  rep = rep_table[which_rep][1];

	  if (flip_x)
	    x = x_min;
	  else
	    x = x_max;

	  zr = y_mid_y / (2 * c3);
	  z_rel = zr * (z_lin(z_max) - z_lin(z_min));

	  if (flip_z)
	    z = z_log(z_lin(z_max) + z_rel);
	  else
	    z = z_log(z_lin(z_min) - z_rel);
	}
      else
	{
	  if (flip_x)
	    x = x_log(x_lin(x_min) - x_rel);
	  else
	    x = x_log(x_lin(x_max) + x_rel);

	  if (flip_z)
	    z = z_max;
	  else
	    z = z_min;

	  gks_set_text_align(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_HALF);
	}

      gks_set_text_upvec(a[axes_rep[rep][0]], c[axes_rep[rep][1]]);
      gks_set_text_slant(text_slant[axes_rep[rep][2]]);

      text3d(x, y, z, y_title);
    }

  if (*z_title)
    {
      rep = rep_table[which_rep][2];

      gks_set_text_upvec(a[axes_rep[rep][0]], c[axes_rep[rep][1]]);
      gks_set_text_slant(text_slant[axes_rep[rep][2]]);

      if (flip_x)
	x = x_max;
      else
	x = x_min;

      if (flip_y)
	y = y_max;
      else
	y = y_min;

      zr = (1 - (c1 + c3 + vp[2])) / (2 * c3);
      z_rel = zr * (z_lin(z_max) - z_lin(z_min));

      if (flip_z)
	z = z_log(z_lin(z_min) - 0.5 * z_rel);
      else
	z = z_log(z_lin(z_max) + 0.5 * z_rel);

      gks_set_text_align(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_BASE);

      text3d(x, y, z, z_title);
    }

  /* restore text alignment, text font, text slant,
     character-up vector and clipping indicator */

  gks_set_text_align(halign, valign);
  gks_set_text_fontprec(font, prec);
  gks_set_text_slant(slant);
  gks_set_text_upvec(chux, chuy);
  gks_set_clipping(clsw);
}

#define nint(x) (int)((x) + 0.5)

static
void init_hlr(void)
{
  register int sign, i, j, x1, x2;
  register float *hide, a, b, m = 0;
  float x[3], y[3], z[3], yj;
  float eps;

  eps = (lx.ymax - lx.ymin) * 1E-5;

  for (i = 0; i <= RESOLUTION_X; i++)
    {
      hlr.ymin[i] = -FLT_MAX;
      hlr.ymax[i] = FLT_MAX;
    }

  for (sign = -1; sign <= 1; sign += 2)
    {
      if (sign == 1)
	{
	  hide = hlr.ymin;
	  x[0] = hlr.x0; y[0] = hlr.y0; z[0] = hlr.z0;
	  x[1] = hlr.x1; y[1] = hlr.y0; z[1] = hlr.z0;
	  x[2] = hlr.x1; y[2] = hlr.y1; z[2] = hlr.z0;
	}
      else
	{
	  hide = hlr.ymax;
	  x[0] = hlr.x0; y[0] = hlr.y0; z[0] = hlr.z1;
	  x[1] = hlr.x0; y[1] = hlr.y1; z[1] = hlr.z1;
	  x[2] = hlr.x1; y[2] = hlr.y1; z[2] = hlr.z1;
	}

      for (i = 0; i < 3; i++)
	apply_world_xform(x + i, y + i, z + i);

      if (hlr.xmax > hlr.xmin)
	{
	  a = RESOLUTION_X / (hlr.xmax - hlr.xmin);
	  b = -(hlr.xmin * a);
	}
      else
	{
	  a = 1;
	  b = 0;
	}

      x1 = nint(a * x[0] + b);
      if (x1 < 0)
	x1 = 0;
      x2 = x1;

      for (i = 1; i < 3; i++)
	{
	  x1 = x2;
	  x2 = nint(a * x[i] + b);

	  if (x1 <= x2)
	    {
	      if (x1 != x2)
		m = (y[i] - y[i - 1]) / (x2 - x1);

	      for (j = x1; j <= x2; j++)
		{
		  if (x1 != x2)
		    yj = y[i - 1] + m * (j - x1);
		  else
		    yj = y[i];

		  hide[j] = yj - sign * eps;
		}
	    }
	}
    }
}

static
void pline_hlr(int n, float *x, float *y, float *z)
{
  register int i, j, x1, x2;
  register int visible, draw;
  register float *hide, a, b, c, m = 0;

  int saved_scale_options;
  float xj, yj;

  if (hlr.buf == NULL)
    {
      hlr.buf = (float *) xmalloc(sizeof(float) * (RESOLUTION_X + 1) * 2);
      hlr.ymin = hlr.buf;
      hlr.ymax = hlr.buf + RESOLUTION_X + 1;
    }

  if (hlr.sign == 1)
    hide = hlr.ymin;
  else
    hide = hlr.ymax;

  for (i = 0; i < n; i++)
    apply_world_xform(x + i, y + i, z + i);

  draw = !hlr.initialize || hlr.sign > 0;
  visible = 0;

  saved_scale_options = lx.scale_options;
  lx.scale_options = 0;

  if (hlr.xmax > hlr.xmin)
    {
      a = RESOLUTION_X / (hlr.xmax - hlr.xmin);
      b = -(hlr.xmin * a);
    }
  else
    {
      a = 1;
      b = 0;
    }
  c = 1.0 / a;

  x1 = nint(a * x[0] + b);
  if (x1 < 0)
    x1 = 0;
  x2 = x1;

  if (hlr.initialize)
    {
      init_hlr();

      if (hlr.ymin[x1] <= y[0] && y[0] <= hlr.ymax[x1])
	{
	  hide[x1] = y[0];

	  if (draw)
	    start_pline(x[0], y[0]);

	  visible = 1;
	}
    }

  for (i = 1; i < n; i++)
    {
      x1 = x2;
      x2 = nint(a * x[i] + b);

      if (x1 < x2)
	{
	  if (x1 != x2)
	    m = (y[i] - y[i - 1]) / (x2 - x1);

	  for (j = x1; j <= x2; j++)
	    {
	      if (x1 != x2)
		yj = y[i - 1] + m * (j - x1);
	      else
		yj = y[i];

	      if (hlr.ymin[j] <= yj && yj <= hlr.ymax[j])
		{
		  if (!visible)
		    {
		      if (draw)
			{
			  xj = c * j + hlr.xmin;

			  start_pline(xj, yj);
			}

		      visible = 1;
		    }
		}
	      else
		{
		  if (visible)
		    {
		      if (draw)
			{
			  xj = c * j + hlr.xmin;

			  pline(xj, yj);
			  end_pline();
			}

		      visible = 0;
		    }
		}

	      if ((yj - hide[j]) * hlr.sign > 0)
		hide[j] = yj;
	    }

	  if (visible && draw)
	    pline(x[i], y[i]);
	}

      else if (x1 == x2)
	{
	  if (draw)
	    {
              j = x1;
	      xj = c * j + hlr.xmin;
	      yj = y[i];

	      if ((yj - hide[j]) * hlr.sign > 0)
		{
		  start_pline(xj, hide[j]);
		  pline(xj, yj);
		  end_pline();

		  visible = 1;
		  hide[j] = yj;
		}
	      else
		visible = 0;
            }
	}
    }

  if (visible && draw)
    end_pline();

  lx.scale_options = saved_scale_options;
}

static
void glint(int dinp, int *inp, int doutp, int *outp)
{
  int i, j, k, n;
  float ratio, delta;

  n = (doutp + 1) / dinp;
  ratio = 1.0 / n;

  j = (n + 1) / 2;
  for (k = 0; k < j; k++)
    outp[k] = inp[0];

  for (i = 0; i < dinp - 1; i++)
    {
      delta = ratio * (inp[i + 1] - inp[i]);
      for (k = 1; k <= n; k++)
	outp[j++] = inp[i] + (int)(k * delta + 0.5);
    }

  for (k = j; k < doutp; k++)
    outp[k] = inp[dinp - 1];
}

static
void pixel(
  float xmin, float xmax, float ymin, float ymax,
  int dx, int dy, int *colia, int w, int h, int *pixmap,
  int dwk, int *wk1, int *wk2)
{
  int i, j, ix, nx;
  int sx = 1, sy = 1;

  if ((w + 1) % dx != 0 || (h + 1) % dy != 0)
    {
      fprintf(stderr, "improper input parameter values\n");
      return;
    }

  ix = 0;
  nx = (w + 1) / dx;

  for (i = 0; i < dx; i++)
    {
      for (j = 0; j < dy; j++)
	wk1[j] = colia[i + j * dx];

      glint(dy, wk1, h, wk2);
      for (j = 0; j < h; j++)
	pixmap[ix + j * w] = wk2[j];

      ix += nx;
    }

  for (j = 0; j < h; j++)
    {
      ix = 0;
      for (i = 0; i < dx; i++)
	{
	  wk1[i] = pixmap[ix + j * w];
	  ix += nx;
	}

      glint(dx, wk1, w, wk2);
      for (i = 0; i < w; i++)
	pixmap[i + j * w] = wk2[i];
    }

  gks_cellarray(xmin, ymin, xmax, ymax, w, h, sx, sy, w, h, pixmap);
}

#undef nint

static
void get_intensity(
  float *fx, float *fy, float *fz, float *light_source, float *intensity)
{
  int k;
  float max_x, max_y, max_z, min_x, min_y, min_z, norm_1, norm_2;
  float center[4], normal[4], negated[4], oddnormal[4], negated_norm[4];

  min_x = max_x = fx[0];
  min_y = max_y = fy[0];
  min_z = max_z = fz[0];

  for (k = 1; k < 4; k++)
    {
      if (fx[k] > max_x)
	max_x = fx[k];
      else if (fx[k] < min_x)
	min_x = fx[k];
      if (fy[k] > max_y)
	max_y = fy[k];
      else if (fy[k] < min_y)
	min_y = fy[k];
      if (fz[k] > max_z)
	max_z = fz[k];
      else if (fz[k] < min_z)
	min_z = fz[k];
    }

  center[0] = (max_x + min_x) / 2;
  center[1] = (max_y + min_y) / 2;
  center[2] = (max_z + min_z) / 2;

  for (k = 0; k < 3; k++)
    negated[k] = light_source[k] - center[k];

  norm_1 = (float) sqrt(negated[0] * negated[0] + negated[1] * negated[1] +
			negated[2] * negated[2]);

  for (k = 0; k < 3; k++)
    negated_norm[k] = negated[k] / norm_1;

  normal[0] =
    ((fy[1] - fy[0]) * (fz[2] - fz[0]) - (fz[1] - fz[0]) * (fy[2] - fy[0])) +
    ((fy[2] - fy[1]) * (fz[3] - fz[1]) - (fz[2] - fz[1]) * (fy[3] - fy[1]));
  normal[1] =
    ((fz[1] - fz[0]) * (fx[2] - fx[0]) - (fx[1] - fx[0]) * (fz[2] - fz[0])) +
    ((fz[2] - fz[1]) * (fx[3] - fx[1]) - (fx[2] - fx[1]) * (fz[3] - fz[1]));
  normal[2] =
    ((fx[1] - fx[0]) * (fy[2] - fy[0]) - (fy[1] - fy[0]) * (fx[2] - fx[0])) +
    ((fx[2] - fx[1]) * (fy[3] - fy[1]) - (fy[2] - fy[1]) * (fx[3] - fx[1]));
  normal[3] = 1;

  norm_2 = (float) sqrt(normal[0] * normal[0] + normal[1] * normal[1] +
			normal[2] * normal[2]);

  for (k = 0; k < 3; k++)
    oddnormal[k] = normal[k] / norm_2;

  *intensity =
    (oddnormal[0] * negated_norm[0] + oddnormal[1] * negated_norm[1] +
     oddnormal[2] * negated_norm[2]) * 0.8 + 0.2;
}

void gr_surface(int nx, int ny, float *px, float *py, float *pz, int option)
{
  int errind, ltype, coli, int_style;

  int i, ii, j, jj, k;
  int color;

  float *xn, *yn, *zn, *x, *y, *z;
  float facex[4], facey[4], facez[4], intensity = 0, meanz;
  float a, b, c, d, e, f;

  float ymin, ymax, zmin, zmax;

  int flip_x, flip_y, flip_z;
  int np;

  int *colia, w, h, *ca, dwk, *wk1, *wk2;

  static float light_source[3] = { 0.5, -1, 2 };

  if ((nx <= 0) || (ny <= 0))
    {
      fprintf(stderr, "invalid number of points\n");
      return;
    }

  /* be sure that points ordinates are sorted in ascending order */

  for (i = 1; i < nx; i++)
    if (px[i - 1] >= px[i])
      {
	fprintf(stderr, "points not sorted in ascending order\n");
	return;
      }

  for (j = 1; j < ny; j++)
    if (py[j - 1] >= py[j])
      {
	fprintf(stderr, "points not sorted in ascending order\n");
	return;
      }

  check_autoinit;

  gr_setscale(lx.scale_options);

#define Z(x, y) pz[(x) + nx * (y)]

  if (accel)
    {
      gr_beginproperty("data3d: %i; %i;", nx, ny);
      for (j = 0; j < ny; j++)
	for (i = 0; i < nx; i++)
	  gr_addproperty("%g;", Z(i, j));
      gr_endproperty();
      gr_setproperty("surface: 1; flush;");
      if (accel > 1)
	return;
    }

  a = 1.0 / (lx.xmax - lx.xmin);
  b = -(lx.xmin * a);
  c = 1.0 / (lx.ymax - lx.ymin);
  d = -(lx.ymin * c);
  e = 1.0 / (wx.zmax - wx.zmin);
  f = -(wx.zmin * e);

  /* save linetype, fill area interior style and color 
     index */

  gks_inq_pline_linetype(&errind, &ltype);
  gks_inq_fill_int_style(&errind, &int_style);
  gks_inq_fill_color_index(&errind, &coli);

  k = sizeof(float) * (nx + ny) * 3;
  xn = (float *) xmalloc(k);
  yn = (float *) xmalloc(k);
  zn = (float *) xmalloc(k);
  x = (float *) xmalloc(nx * sizeof(float));
  y = (float *) xmalloc(ny * sizeof(float));
  z = (float *) xmalloc(nx * ny * sizeof(float));

  flip_x = OPTION_FLIP_X & lx.scale_options;
  for (i = 0; i < nx; i++)
    x[i] = x_lin(px[flip_x ? nx - 1 - i : i]);

  flip_y = OPTION_FLIP_Y & lx.scale_options;
  for (j = 0; j < ny; j++)
    y[j] = y_lin(py[flip_y ? ny - 1 - j : j]);

  k = 0;
  for (j = 0; j < ny; j++)
    {
      jj = flip_y ? ny - j - 1 : j;
      for (i = 0; i < nx; i++)
	{
	  ii = flip_x ? nx - i - 1 : i;
	  z[k++] = z_lin(Z(ii, jj));
	}
    }

#undef Z

  hlr.x0 = x[0];
  hlr.x1 = x[nx - 1];
  hlr.y0 = y[0];
  hlr.y1 = y[ny - 1];
  hlr.z0 = wx.zmin;
  hlr.z1 = wx.zmax;

  hlr.xmin = x[0];
  hlr.xmax = x[nx - 1];
  ymin = y[0];
  ymax = y[ny - 1];
  zmin = wx.zmin;
  zmax = wx.zmax;

  apply_world_xform(&hlr.xmin, &ymin, &zmin);
  apply_world_xform(&hlr.xmax, &ymax, &zmax);

  flip_z = OPTION_FLIP_Z & lx.scale_options;
  gks_set_pline_linetype(flip_z ? GKS_K_LINETYPE_DOTTED : GKS_K_LINETYPE_SOLID);

#define Z(x, y) z[(x) + nx * (y)]

  hlr.sign = -1;

  do
    {
      hlr.sign = -hlr.sign;

      switch (option)
	{

	case OPTION_LINES:
	  {
	    j = 0;
	    hlr.initialize = 1;

	    while (j < ny && !flag_abort)
	      {
		k = 0;

		if (j > 0)
		  {
		    xn[k] = x[0];
		    yn[k] = y[j - 1];
		    zn[k] = Z(0, j - 1);
		    k++;
		  }

		for (i = 0; i < nx; i++)
		  {
		    xn[k] = x[i];
		    yn[k] = y[j];
		    zn[k] = Z(i, j);
		    k++;
		  }

		if (j == 0)

		  for (i = 1; i < ny; i++)
		    {
		      xn[k] = x[nx - 1];
		      yn[k] = y[i];
		      zn[k] = Z(nx - 1, i);
		      k++;
		    }

		pline_hlr(k, xn, yn, zn);

		hlr.initialize = 0;
		j++;
	      }

	    break;
	  }

	case OPTION_MESH:
	  {
	    k = 0;

	    for (i = 0; i < nx; i++)
	      {
		xn[k] = x[i];
		yn[k] = y[0];
		zn[k] = Z(i, 0);
		k++;
	      }

	    for (j = 1; j < ny; j++)
	      {
		xn[k] = x[nx - 1];
		yn[k] = y[j];
		zn[k] = Z(nx - 1, j);
		k++;
	      }

	    hlr.initialize = 1;

	    pline_hlr(k, xn, yn, zn);

	    i = nx - 1;

	    while (i > 0 && !flag_abort)
	      {
		hlr.initialize = 0;

		for (j = 1; j < ny; j++)
		  {
		    xn[0] = x[i - 1];
		    yn[0] = y[j - 1];
		    zn[0] = Z(i - 1, j - 1);

		    xn[1] = x[i - 1];
		    yn[1] = y[j];
		    zn[1] = Z(i - 1, j);

		    xn[2] = x[i];
		    yn[2] = y[j];
		    zn[2] = Z(i, j);

		    pline_hlr(3, xn, yn, zn);
		  }

		i--;
	      }

	    break;
	  }

	case OPTION_FILLED_MESH:
	case OPTION_Z_SHADED_MESH:
	case OPTION_COLORED_MESH:
	case OPTION_SHADED_MESH:
	  {
	    j = ny - 1;

	    gks_set_fill_int_style(GKS_K_INTSTYLE_SOLID);

	    while (j > 0 && !flag_abort)
	      {
		for (i = 1; i < nx; i++)
		  {
		    xn[0] = x[i - 1];
		    yn[0] = y[j];
		    zn[0] = Z(i - 1, j);

		    xn[1] = x[i - 1];
		    yn[1] = y[j - 1];
		    zn[1] = Z(i - 1, j - 1);

		    xn[2] = x[i];
		    yn[2] = y[j - 1];
		    zn[2] = Z(i, j - 1);

		    xn[3] = x[i];
		    yn[3] = y[j];
		    zn[3] = Z(i, j);

		    xn[4] = xn[0];
		    yn[4] = yn[0];
		    zn[4] = zn[0];

		    if (option == OPTION_SHADED_MESH)
		      {
			for (k = 0; k < 4; k++)
			  {
			    facex[k] = a * xn[k] + b;
			    facey[k] = c * yn[k] + d;
			    facez[k] = e * zn[k] + f;
			  }
			get_intensity(facex, facey, facez,
				      light_source, &intensity);
		      }

		    for (k = 0; k <= 4; k++)
		      apply_world_xform(xn + k, yn + k, zn + k);

		    meanz = 0.25 * (Z(i - 1, j - 1) + Z(i, j - 1) +
				    Z(i, j) + Z(i - 1, j));

		    if (option == OPTION_Z_SHADED_MESH)
		      {
			color = iround(meanz) + FIRST_COLOR;

			if (color < FIRST_COLOR)
			  color = FIRST_COLOR;
			else if (color > LAST_COLOR)
			  color = LAST_COLOR;

			gks_set_fill_color_index(color);
		      }

		    else if (option == OPTION_COLORED_MESH)
		      {
			color = iround((meanz - wx.zmin) / (wx.zmax - wx.zmin) *
			  (LAST_COLOR - FIRST_COLOR)) + FIRST_COLOR;

			if (color < FIRST_COLOR)
			  color = FIRST_COLOR;
			else if (color > LAST_COLOR)
			  color = LAST_COLOR;

			gks_set_fill_color_index(color);
		      }

		    else if (option == OPTION_SHADED_MESH)
		      {
			color = iround(intensity * (LAST_COLOR - FIRST_COLOR)) +
			  FIRST_COLOR;

			if (color < FIRST_COLOR)
			  color = FIRST_COLOR;
			else if (color > LAST_COLOR)
			  color = LAST_COLOR;

			gks_set_fill_color_index(color);
		      }

		    np = 4;
		    gks_fillarea(np, xn, yn);

		    if (option == OPTION_FILLED_MESH)
		      {
			np = 5;
			gks_polyline(np, xn, yn);
		      }
		  }

		j--;
	      }

	    break;
	  }

	case OPTION_CELL_ARRAY:

	  colia = (int *) xmalloc(nx * ny * sizeof(int));
	  k = 0;
	  for (j = 0; j < ny; j++)
	    for (i = 0; i < nx; i++)
	      {
		if (Z(i, j) != MISSING_VALUE)
		  {
		    color = FIRST_COLOR + (int) (
		      (Z(i, j) - wx.zmin) / (wx.zmax - wx.zmin) *
		      (LAST_COLOR - FIRST_COLOR));

		    if (color < FIRST_COLOR)
		      color = FIRST_COLOR;
		    else if (color > LAST_COLOR)
		      color = LAST_COLOR;
		  }
		else
		  color = BACKGROUND;

		colia[k++] = color;
	      }

	  w = (nx < 256) ? nx * (255 / nx + 1) - 1 : nx - 1;
	  h = (ny < 256) ? ny * (255 / ny + 1) - 1 : ny - 1;
	  ca = (int *) xmalloc(w * h * sizeof(int));

	  dwk = w;
	  if (h > dwk)
	    dwk = h;
	  if (nx > dwk)
	    dwk = nx;
	  if (ny > dwk)
	    dwk = ny;
	  wk1 = (int *) xmalloc(dwk * sizeof(int));
	  wk2 = (int *) xmalloc(dwk * sizeof(int));

	  pixel(hlr.xmin, hlr.xmax, ymin, ymax,
		nx, ny, colia, w, h, ca, dwk, wk1, wk2);

	  free(wk2);
	  free(wk1);
	  free(ca);
	  free(colia);

	  break;
	}

      gks_set_pline_linetype(flip_z ? GKS_K_LINETYPE_SOLID :
			     GKS_K_LINETYPE_DOTTED);
    }
  while ((hlr.sign >= 0) && !flag_abort &&
	 ((int) option <= (int) OPTION_MESH));

#undef Z

  free(z);
  free(y);
  free(x);
  free(zn);
  free(yn);
  free(xn);

  /* restore linetype, fill area interior style and color index */

  gks_set_pline_linetype(ltype);
  gks_set_fill_int_style(int_style);
  gks_set_fill_color_index(coli);
}

void gr_contour(
  int nx, int ny, int nh, float *px, float *py, float *h, float *pz,
  int major_h)
{
  int i, j;
  int errind, ltype, halign, valign;
  float chux, chuy;

  if ((nx <= 0) || (ny <= 0))
    {
      fprintf(stderr, "invalid number of points\n");
      return;
    }

  /* be sure that points ordinates are sorted in ascending order */

  for (i = 1; i < nx; i++)
    if (px[i - 1] >= px[i])
      {
	fprintf(stderr, "points not sorted in ascending order\n");
	return;
      }

  for (j = 1; j < ny; j++)
    if (py[j - 1] >= py[j])
      {
	fprintf(stderr, "points not sorted in ascending order\n");
	return;
      }

  check_autoinit;

  gr_setscale(lx.scale_options);

  /* save linetype, text alignment and character-up vector */

  gks_inq_pline_linetype(&errind, &ltype);
  gks_inq_text_align(&errind, &halign, &valign);
  gks_inq_text_upvec(&errind, &chux, &chuy);

  gks_set_text_align(GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_HALF);

  gr_draw_contours(nx, ny, nh, px, py, h, pz, major_h);

  /* restore linetype, character-up vector and text alignment */

  gks_set_pline_linetype(ltype);
  gks_set_text_align(halign, valign);
  gks_set_text_upvec(chux, chuy);
}

static
float value(float n1, float n2, float hue)
{
  float val;

  if (hue > 360)
    hue -= 360;
  if (hue < 0)
    hue += 360;

  if (hue < 60)
    val = n1 + (n2 - n1) * hue / 60;
  else if (hue < 180)
    val = n2;
  else if (hue < 240)
    val = n1 + (n2 - n1) * (240 - hue) / 60;
  else
    val = n1;

  return (val);
}

static
void hls_to_rgb(float h, float l, float s, float *r, float *g, float *b)
{
  float m1, m2;

  m2 = (l < 0.5) ? l * (1 + s) : l + s - l * s;
  m1 = 2 * l - m2;

  if (s == 0)
    {
      *r = *g = *b = l;
    }
  else
    {
      *r = value(m1, m2, h + 120);
      *g = value(m1, m2, h);
      *b = value(m1, m2, h - 120);
    }
}

void gr_setcolormap(int index)
{
  int i, ci;
  float r, g, b, h, l, s;
  double x;
  int inverted, j;

  i = 0;
  inverted = 0;
  r = g = b = 0;
  if (index < 0)
    {
      index = -index;
      i = LAST_COLOR - FIRST_COLOR + 1;
      inverted = 1;
    }  

  for (ci = FIRST_COLOR; ci <= LAST_COLOR; ci++)
    {
      x = (double) i / (double) (LAST_COLOR - FIRST_COLOR + 1);

      switch (index)
	{
	case COLORMAP_UNIFORM:
	case COLORMAP_TEMPERATURE:
	  if (index == COLORMAP_UNIFORM)
	    h = i * 360.0 / (LAST_COLOR - FIRST_COLOR + 1) - 120;
	  else
	    h = 270 - i * 300.0 / (LAST_COLOR - FIRST_COLOR + 1);

	  l = 0.5;
	  s = 0.75;

	  hls_to_rgb(h, l, s, &r, &g, &b);
	  break;

	case COLORMAP_GRAYSCALE:
	  r = x;
	  g = x;
	  b = x;
	  break;

	case COLORMAP_GLOWING:
	  r = pow(x, 1.0 / 4.0);
	  g = x;
	  b = pow(x, 4.0);
	  break;

	case COLORMAP_RAINBOW:
	case COLORMAP_FLAME:
	  if (x < 0.125)
	    r = 4.0 * (x + 0.125);
	  else if (x < 0.375)
	    r = 1.0;
	  else if (x < 0.625)
	    r = 4.0 * (0.625 - x);
	  else
	    r = 0;

	  if (x < 0.125)
	    g = 0;
	  else if (x < 0.375)
	    g = 4.0 * (x - 0.125);
	  else if (x < 0.625)
	    g = 1.0;
	  else if (x < 0.875)
	    g = 4.0 * (0.875 - x);
	  else
	    g = 0;

	  if (x < 0.375)
	    b = 0;
	  else if (x < 0.625)
	    b = 4.0 * (x - 0.375);
	  else if (x < 0.875)
	    b = 1.0;
	  else
	    b = 4.0 * (1.125 - x);

	  if (index == COLORMAP_FLAME)
	    {
	      r = 1.0 - r;
	      g = 1.0 - g;
	      b = 1.0 - b;
	    }
	  break;

	case COLORMAP_GEOLOGIC:
	  if (x < 0.333333)
	    r = 0.333333 - x;
	  else if (x < 0.666666)
	    r = 3.0 * (x - 0.333333);
	  else
	    r = 1.0 - (x - 0.666666);

	  if (x < 0.666666)
	    g = 0.75 * x + 0.333333;
	  else
	    g = 0.833333 - 1.5 * (x - 0.666666);

	  if (x < 0.333333)
	    b = 1.0 - 2.0 * x;
	  else if (x < 0.666666)
	    b = x;
	  else
	    b = 0.666666 - 2.0 * (x - 0.666666);
	  break;

	case COLORMAP_GREENSCALE:
	  r = x;
	  g = pow(x, 1.0 / 4.0);
	  b = pow(x, 4.0);
	  break;

	case COLORMAP_CYANSCALE:
	  r = pow(x, 4.0);
	  g = pow(x, 1.0 / 4.0);
	  b = x;
	  break;

	case COLORMAP_BLUESCALE:
	  r = pow(x, 4.0);
	  g = x;
	  b = pow(x, 1.0 / 4.0);
	  break;

	case COLORMAP_MAGENTASCALE:
	  r = x;
	  g = pow(x, 4.0);
	  b = pow(x, 1.0 / 4.0);
	  break;

	case COLORMAP_REDSCALE:
	  r = pow(x, 1.0 / 4.0);
	  g = pow(x, 4.0);
	  b = x;
	  break;

	case COLORMAP_BROWNSCALE:
	  r = 0.55 + x * 0.45;
	  g = 0.15 + x * 0.85;
	  b = 0;
	  break;

	case COLORMAP_USER_DEFINED:
	  j = inverted ? i - 1 : i;
	  r = cmap[j][0] / 255.0;
	  g = cmap[j][1] / 255.0;
	  b = cmap[j][2] / 255.0;
	  break;
	};

      gr_setcolorrep(ci, r, g, b);

      if (inverted)
	i--;
      else
	i++;
    }

  if (accel)
    gr_setproperty("colormap: %d;", index);
}

void gr_colormap(void)
{
  int errind, halign, valign, clsw, tnr;
  float clrt[4], wn[4], vp[4];
  float xmin, xmax, ymin, ymax, zmin, zmax;
  int w, h, sx, sy, nx, ny, colia[LAST_COLOR - FIRST_COLOR + 1];
  int i, nz, ci, cells;
  float x, y, z, dy, dz;
  char text[256];

  check_autoinit;

  gr_setscale(lx.scale_options);

  /* save text alignment and clipping indicator */

  gks_inq_text_align(&errind, &halign, &valign);
  gks_inq_clip(&errind, &clsw, clrt);

  /* inquire current normalization transformation */

  gks_inq_current_xformno(&errind, &tnr);
  gks_inq_xform(tnr, &errind, wn, vp);

  cells = LAST_COLOR - FIRST_COLOR + 1;
  for (ci = FIRST_COLOR; ci <= LAST_COLOR; ci++)
    colia[ci - FIRST_COLOR] = ci;

  xmin = lx.xmin;
  xmax = lx.xmax;
  ymin = lx.ymin;
  ymax = lx.ymax;
  zmin = wx.zmin;
  zmax = wx.zmax;

  w = nx = 1;
  h = ny = cells;
  sx = sy = 1;

  gks_cellarray(xmin, ymin, xmax, ymax, w, h, sx, sy, nx, ny, colia);

  dz = 0.5 * gr_tick(zmin, zmax);
  nz = (int) ((zmax - zmin) / dz + 0.5);
  dy = (ymax - ymin) / nz;

  gks_set_text_align(GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);
  gks_set_clipping(GKS_K_NOCLIP);

  x = xmax + 0.01 * (xmax - xmin) / (vp[1] - vp[0]);
  for (i = 0; i <= nz; i++)
    {
      y = ymin + i * dy;
      z = zmin + i * dz;
      text2d(x, y, str_ftoa(text, z, dz));
    }

  /* restore text alignment and clipping indicator */

  gks_set_text_align(halign, valign);
  gks_set_clipping(clsw);
}

float gr_tick(float amin, float amax)
{
  float tick_unit, exponent, factor;
  int n;

  if (amax > amin)
    {
      exponent = log10(amax - amin);
      n = gauss(exponent);

      factor = pow(10.0, (double) (exponent - n));

      if (factor > 5)
	tick_unit = 2;
      else
       if (factor > 2.5)
	tick_unit = 1;
      else
       if (factor > 1)
	tick_unit = 0.5;
      else
	tick_unit = 0.2;

      tick_unit = tick_unit * pow(10.0, (double) n);
    }
  else
    tick_unit = 0;

  return (tick_unit); /* return a tick unit that evenly divides into the
			 difference between the minimum and maximum value */
}

static
float fract(float x)
{
  return (x - (int) x);
}

void gr_adjustrange(float *amin, float *amax)
{
  float tick;

  if (*amin == *amax)
    {
      if (*amin != 0)
	tick = pow(10.0, (double) fract(log10(fabs(*amin))));
      else
	tick = 0.1;

      *amin = *amin - tick;
      *amax = *amax + tick;
    }

  tick = gr_tick(*amin, *amax);

  if (fract(*amin / tick) != 0)
    *amin = tick * gauss(*amin / tick);

  if (fract(*amax / tick) != 0)
    *amax = tick * (gauss(*amax / tick) + 1);
}

static
int gks_wstype(char *type)
{
  int wstype;

  if (!str_casecmp(type, "ps") || !str_casecmp(type, "eps"))
    wstype = 62;
  else if (!str_casecmp(type, "pdf"))
    wstype = 102;
  else if (!str_casecmp(type, "bmp"))
    wstype = 320;
  else if (!str_casecmp(type, "jpeg") || !str_casecmp(type, "jpg"))
    wstype = 321;
  else if (!str_casecmp(type, "png"))
    wstype = 322;
  else if (!str_casecmp(type, "tiff") || !str_casecmp(type, "tif"))
    wstype = 323;
  else if (!str_casecmp(type, "fig"))
    wstype = 370;
  else if (!str_casecmp(type, "svg"))
    wstype = 382;
  else if (!str_casecmp(type, "wmf"))
    wstype = 390;
  else
    {
      fprintf(stderr, "%s: unrecognized file type\nAvailable formats: \
bmp, eps, fig, jpeg, pdf, png, ps, svg, tiff or wmf\n", type);
      wstype = -1;
    }

  return wstype;
}

void gr_beginprint(char *pathname)
{
  int wkid = 6, wstype = 62;
  char *type;

  check_autoinit;

  if (!flag_printing)
    {
      if ((type = strrchr(pathname, '.')) != NULL)
	wstype = gks_wstype(type + 1);

      if (wstype >= 0)
	{
	  gks_open_ws(wkid, pathname, wstype);
	  gks_activate_ws(wkid);
	  flag_printing = 1;
	}
    }
  else
    fprintf(stderr, "print device already activated\n");
}

void gr_beginprintext(
  char *pathname, char *mode, char *format, char *orientation)
{
  int wkid = 6, wstype = 62;
  char *type;
  float width = 0.210, height = 0.297;
  format_t *p = formats;
  int color = 0, landscape = 0;

  check_autoinit;

  if (!flag_printing)
    {
      if ((type = strrchr(pathname, '.')) != NULL)
	wstype = gks_wstype(type + 1);

      if (wstype >= 0)
	{
	  if (str_casecmp(mode, "Color") == 0)
	    color = 1;
	  else if (str_casecmp(mode, "GrayScale") != 0)
	    fprintf(stderr, "%s: invalid color mode\n", mode);

	  while (p->format != NULL)
	    {
	      if (str_casecmp(p->format, format) == 0)
		{
		  width = p->width * 0.9;
		  height = p->height * 0.9;
		  break;
		}
	      p++;
	    }
	  if (p->format == NULL)
	    fprintf(stderr, "%s: invalid page size\n", format);

	  if (str_casecmp(orientation, "Landscape") == 0)
	    landscape = 1;
	  else if (str_casecmp(orientation, "Portrait") != 0)
	    fprintf(stderr, "%s: invalid page orientation\n", orientation);

	  if (wstype == 62)
	    {
	      if (!color)
		wstype -= 1;
	      if (landscape)
		wstype += 2;
	    }

	  gks_open_ws(wkid, pathname, wstype);
	  gks_activate_ws(wkid);

	  if (!landscape)
	    {
	      gks_set_ws_viewport(wkid, 0.0, width, 0.0, height);
	      if (width < height)
		gks_set_ws_window(wkid, 0.0, width / height, 0.0, 1.0);
	      else
		gks_set_ws_window(wkid, 0.0, 1.0, 0.0, height / width);
	    }
	  else
	    {
	      gks_set_ws_viewport(wkid, 0.0, height, 0.0, width);
	      if (width < height)
		gks_set_ws_window(wkid, 0.0, 1.0, 0.0, width / height);
	      else
		gks_set_ws_window(wkid, 0.0, height / width, 0.0, 1.0);
	    }

	  flag_printing = 1;
	}
    }
  else
    fprintf(stderr, "print device already activated\n");
}

void gr_endprint(void)
{
  int wkid = 6;

  if (flag_printing)
    {
      gks_deactivate_ws(wkid);
      gks_close_ws(wkid);
      flag_printing = 0;
    }
  else
    fprintf(stderr, "no print device activated\n");
}

void gr_ndctowc(float *x, float *y)
{
  check_autoinit;

  *x = x_log((*x - nx.b) / nx.a);
  *y = y_log((*y - nx.d) / nx.c);
}

void gr_wctondc(float *x, float *y)
{
  check_autoinit;

  *x = nx.a * x_lin(*x) + nx.b;
  *y = nx.c * y_lin(*y) + nx.d;
}

void gr_drawrect(float xmin, float xmax, float ymin, float ymax)
{
  float x[5], y[5];

  check_autoinit;

  x[0] = x[3] = min(xmin, xmax);
  x[1] = x[2] = max(xmin, xmax);
  x[4] = x[0];
  y[0] = y[1] = min(ymin, ymax);
  y[2] = y[3] = max(ymin, ymax);
  y[4] = y[0];
  
  gr_polyline(5, x, y);

  if (flag_graphics)
    fprintf(stream, "<drawrect xmin='%g' xmax='%g' ymin='%g' ymax='%g'/>\n",
	    xmin, xmax, ymin, ymax);
}

void gr_fillrect(float xmin, float xmax, float ymin, float ymax)
{
  float x[4], y[4];

  check_autoinit;

  x[0] = x[3] = min(xmin, xmax);
  x[1] = x[2] = max(xmin, xmax);
  y[0] = y[1] = min(ymin, ymax);
  y[2] = y[3] = max(ymin, ymax);
  
  gr_fillarea(4, x, y);

  if (flag_graphics)
    fprintf(stream, "<fillrect xmin='%g' xmax='%g' ymin='%g' ymax='%g'/>\n",
	    xmin, xmax, ymin, ymax);
}

void gr_drawarc(
  float xmin, float xmax, float ymin, float ymax, int a1, int a2)
{
  float xcenter, ycenter, width, height;
  int a, n;
  float x[361], y[361];

  check_autoinit;

  xcenter = (x_lin(xmin) + x_lin(xmax)) / 2.0;
  ycenter = (y_lin(ymin) + y_lin(ymax)) / 2.0;
  width   = fabs(x_lin(xmax) - x_lin(xmin)) / 2.0;
  height  = fabs(y_lin(ymax) - y_lin(ymin)) / 2.0;

  a1 %= 360;
  a2 %= 360;
  if (a2 <= a1)
    a2 += 360;

  n = 0;
  for (a = a1; a <= a2; a++)
    {
      x[n] = x_log(xcenter + width  * cos(a * M_PI / 180));
      y[n] = y_log(ycenter + height * sin(a * M_PI / 180));
      n++;
    }

  if (n > 1)
    gr_polyline(n, x, y);

  if (flag_graphics)
    fprintf(stream,
      "<drawarc xmin='%g' xmax='%g' ymin='%g' ymax='%g' a1='%d' a2='%d'/>\n",
      xmin, xmax, ymin, ymax, a1, a2);
}

void gr_fillarc(
  float xmin, float xmax, float ymin, float ymax, int a1, int a2)
{
  float xcenter, ycenter, width, height;
  int a, n;
  float x[361], y[361];

  check_autoinit;

  xcenter = (x_lin(xmin) + x_lin(xmax)) / 2.0;
  ycenter = (y_lin(ymin) + y_lin(ymax)) / 2.0;
  width   = fabs(x_lin(xmax) - x_lin(xmin)) / 2.0;
  height  = fabs(y_lin(ymax) - y_lin(ymin)) / 2.0;

  a1 %= 360;
  a2 %= 360;
  if (a2 <= a1)
    a2 += 360;

  n = 0;
  for (a = a1; a <= a2; a++)
    {
      x[n] = x_log(xcenter + width  * cos(a * M_PI / 180));
      y[n] = y_log(ycenter + height * sin(a * M_PI / 180));
      n++;
    }

  if (n > 2)
    gr_fillarea(n, x, y);

  if (flag_graphics)
    fprintf(stream,
      "<fillarc xmin='%g' xmax='%g' ymin='%g' ymax='%g' a1='%d' a2='%d'/>\n",
      xmin, xmax, ymin, ymax, a1, a2);
}

void gr_setarrowstyle(int style)
{
  check_autoinit;

  if (style >= 1 && style <= 18)
    arrow_style = style - 1;

  if (flag_graphics)
    fprintf(stream, "<setarrowstyle style='%d'/>\n", style);
}

void gr_drawarrow(float x1, float y1, float x2, float y2)
{
  float xs, ys, xe, ye;
  int errind, ltype, intstyle, tnr;
  float a, c, xc, yc, f;
  int fill, i, j, n;
  float xi, yi, x[10], y[10];

  check_autoinit;

  xs = nx.a * x_lin(x1) + nx.b;
  ys = nx.c * y_lin(y1) + nx.d;
  xe = nx.a * x_lin(x2) + nx.b;
  ye = nx.c * y_lin(y2) + nx.d;

  gks_inq_pline_linetype(&errind, &ltype);
  gks_inq_fill_int_style(&errind, &intstyle);
  gks_inq_current_xformno(&errind, &tnr);

  gks_set_fill_int_style(GKS_K_INTSTYLE_SOLID);
  gks_select_xform(NDC);

  c = sqrt((xe - xs) * (xe - xs) + (ye - ys) * (ye - ys));
  if (ys != ye)
    a = acos(fabs(xe - xs) / c);
  else
    a = 0;
  if (ye < ys)
    a = 2 * M_PI - a;
  if (xe < xs)
    a = M_PI - a;
  a -= M_PI / 2;

  xc = (xs + xe) / 2;
  yc = (ys + ye) / 2;
  f = 0.01 * c / 2;

  j = 0;
  while ((n = vertex_list[arrow_style][j++]) != 0)
    {
      fill = n < 0;
      n = abs(n);
      gks_set_pline_linetype(n > 2 ? GKS_K_LINETYPE_SOLID : ltype);
      for (i = 0; i < n; i++)
	{
	  xi = f * vertex_list[arrow_style][j++];
	  yi = f * vertex_list[arrow_style][j++];
	  x[i] = xc + cos(a) * xi - sin(a) * yi;
	  y[i] = yc + sin(a) * xi + cos(a) * yi;
	}
      if (fill)
	gks_fillarea(n, x, y);
      else
	gks_polyline(n, x, y);
    }

  gks_select_xform(tnr);
  gks_set_fill_int_style(intstyle);
  gks_set_pline_linetype(ltype);

  if (flag_graphics)
    fprintf(stream, "<arrow x1='%g' y1='%g' x2='%g' y2='%g'/>\n",
	    x1, y1, x2, y2);
}

void gr_drawimage(
  float xmin, float xmax, float ymin, float ymax,
  int width, int height, int *data)
{
  register int n;

  check_autoinit;

  gks_draw_image(
    x_lin(xmin), y_lin(ymax), x_lin(xmax), y_lin(ymin), width, height, data);

  if (flag_graphics)
    {
      n = width * height;
      fprintf(stream, "<image xmin='%g' xmax='%g' ymin='%g' ymax='%g' "
	      "width='%d' height='%d'",
	      xmin, xmax, ymin, ymax, width, height);
      print_int_array("data", n, data);
      fprintf(stream, " path=''/>\n");
    }
}

void gr_setshadow(float offsetx, float offsety, float blur)
{
  check_autoinit;

  gks_set_shadow(offsetx, offsety, blur);
}

void gr_settransparency(float alpha)
{
  check_autoinit;

  gks_set_transparency(alpha);
}

void gr_setcoordxform(float mat[3][2])
{
  check_autoinit;

  gks_set_coord_xform(mat);
}

void gr_begingraphics(char *path)
{
  if (!flag_graphics)
    {
      if (!strcmp(path, "-"))
	stream = stdout;
      else
	stream = fopen(path, "w");

      if (stream)
	{
	  fprintf(stream, GR_HEADER);
	  flag_graphics = 1;
	}
      else
	fprintf(stderr, "%s: open failed\n", path);
    }
}

void gr_endgraphics(void)
{
  if (flag_graphics)
    {
      fprintf(stream, GR_TRAILER);
      if (stream != stdout)
	fclose(stream);
      flag_graphics = 0;
    }
}

static
void latex2image(char *string, int pointSize, float *rgb,
                 int *width, int *height, int **data)
{
  int color;
  char s[FILENAME_MAX], path[FILENAME_MAX], cache[33];
  char *tmp, *null, cmd[1024];
  char tex[FILENAME_MAX], dvi[FILENAME_MAX], png[FILENAME_MAX];
  FILE *stream;

  color = ((int)(rgb[0] / 255)      ) + 
          ((int)(rgb[1] / 255) <<  8) +
          ((int)(rgb[2] / 255) << 16);
  sprintf(s, "%d%x%s", pointSize, color, string);
  md5(s, cache);
  sprintf(path, "%s%sgr-cache-%s.png", TMPDIR, DIRDELIM, cache);

  if (access(path, R_OK) != 0)
    {
      tmp = tempnam(TMPDIR, NULL);
      sprintf(tex, "%s.tex", tmp);
      sprintf(dvi, "%s.dvi", tmp);
      sprintf(png, "%s.png", tmp);
#ifdef _WN32
      null = "NUL";
#else
      null = "/dev/null";
#endif
      stream = fopen(tex, "w");
      fprintf(stream, "\
\\documentclass{article}\n\
\\pagestyle{empty}\n\
\\usepackage[dvips]{color}\n\
\\color[rgb]{%.3f,%.3f,%.3f}\n\
\\begin{document}\n\
\\[\n", rgb[0], rgb[1], rgb[2]);
      fwrite(string, strlen(string), 1, stream);
      fprintf(stream, "\n\
\\]\n\
\\end{document}");
      fclose(stream);

      sprintf(cmd, "latex -interaction=batchmode -halt-on-error -output-directory=%s %s >%s",
              TMPDIR, tex, null);
      system(cmd);

      if (access(dvi, R_OK) == 0)
        {
          sprintf(cmd, "dvipng -q -T tight -x %d %s -o %s >%s",
                  pointSize * 100, dvi, png, null);
          system(cmd);
          rename(png, path);

          sprintf(cmd, "rm -f %s.*", tmp);
          system(cmd);
        }
    }

  if (access(path, R_OK) == 0)
    gr_readimage(path, width, height, data);
}

void gr_mathtex(float x, float y, char *string)
{
  int errind, pointSize, color;
  float chh, rgb[3];
  int width, height, *data = NULL;
  float w, h, xmin, xmax, ymin, ymax;
  int halign, valign, tnr;

  check_autoinit;

  gks_inq_text_height(&errind, &chh);
  pointSize = chh * qualityFactor * nominalWindowHeight;

  gks_inq_text_color_index(&errind, &color);
  gks_inq_rgb(color, &rgb[0], &rgb[1], &rgb[2]);

  latex2image(string, pointSize, rgb, &width, &height, &data);

  if (data != NULL)
    {
      w =  width / (float) (qualityFactor * nominalWindowHeight);
      h = height / (float) (qualityFactor * nominalWindowHeight);

      gks_inq_text_align(&errind, &halign, &valign);

      xmin = x + xfac[halign] * w;
      xmax = xmin + w;
      ymin = y + yfac[valign] * h;
      ymax = ymin + h;

      gks_inq_current_xformno(&errind, &tnr);
      if (tnr != NDC)
        gks_select_xform(NDC);

      gr_drawimage(xmin, xmax, ymin, ymax, width, height, data);

      if (tnr != NDC)
        gks_select_xform(tnr);

      free(data);
    }

  if (flag_graphics)
    fprintf(stream, "<mathtex x='%g' y='%g' text='%s'/>\n", x, y, string);
}

void gr_beginselection(int index, int type)
{
  check_autoinit;

  gks_begin_selection(index, type);
}

void gr_endselection(void)
{
  check_autoinit;

  gks_end_selection();
}

void gr_moveselection(float x, float y)
{
  check_autoinit;

  gks_move_selection(x, y);
}

void gr_resizeselection(int type, float x, float y)
{
  check_autoinit;

  gks_resize_selection(type, x, y);
}

void gr_inqbbox(float *xmin, float *xmax, float *ymin, float *ymax)
{
  int errind;

  check_autoinit;

  gks_inq_bbox(&errind, xmin, xmax, ymin, ymax);
}
