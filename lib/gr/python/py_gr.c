#include <Python.h>

#include "gr.h"

#define TRY(E) if (!(E)) return NULL

typedef struct 
  { 
    char *name;
    int value;
  } 
symbol_table_def;

PyObject *PyInit_gr(void);

static
int *intarray(PyObject *arg, int n, char *name)
{
  char message[50];
  PyObject *item;
  int *a, i;

  PyErr_Clear();
  if (!arg || !PyList_Check(arg) || PyList_Size(arg) < n)
    {
      sprintf(message, "need element with at least %d items for '%s'", n, name);
      PyErr_SetString(PyExc_TypeError, message);
      return NULL;
    }

  a = (int *) malloc(n * sizeof(int));
  if (a == NULL)
    {
      sprintf(message, "can't allocate memory for '%s'", name);
      PyErr_SetString(PyExc_TypeError, message);
      return NULL;
    }
  for (i = 0; i < n; i++)
    {
      item = PyList_GetItem(arg, i);
      if (!PyArg_Parse(item, "i", a + i))
	{
	  free(a);
	  sprintf(message, "type error in argument '%s'", name);
	  PyErr_SetString(PyExc_TypeError, message);
	  return NULL;
	}
    }
  return a;
}

static
float *floatarray(PyObject *arg, int n, char *name)
{
  char message[50];
  PyObject *item;
  float *a;
  int i;

  PyErr_Clear();
  if (!arg || !PyList_Check(arg) || PyList_Size(arg) < n)
    {
      sprintf(message, "need element with at least %d items for '%s'", n, name);
      PyErr_SetString(PyExc_TypeError, message);
      return NULL;
    }

  a = (float *) malloc(n * sizeof(float));
  if (a == NULL)
    {
      sprintf(message, "can't allocate memory for '%s'", name);
      PyErr_SetString(PyExc_TypeError, message);
      return NULL;
    }
  for (i = 0; i < n; i++)
    {
      item = PyList_GetItem(arg, i);
      if (!PyArg_Parse(item, "f", a + i))
	{
	  free(a);
	  sprintf(message, "type error in argument '%s'", name);
	  PyErr_SetString(PyExc_TypeError, message);
	  return NULL;
	}
    }
  return a;
}

static
PyObject *_gr_opengks(PyObject *self, PyObject *args)
{
  TRY(PyArg_ParseTuple(args, ":opengks"));

  gr_opengks();

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_closegks(PyObject *self, PyObject *args)
{
  TRY(PyArg_ParseTuple(args, ":closegks"));

  gr_closegks();

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_inqdspsize(PyObject *self, PyObject *args)
{
  float mwidth, mheight;
  int width, height;

  TRY(PyArg_ParseTuple(args, ":inqdspsize"));

  gr_inqdspsize(&mwidth, &mheight, &width, &height);

  return Py_BuildValue("ffii", mwidth, mheight, width, height);
}

static
PyObject *_gr_openws(PyObject *self, PyObject *args)
{
  int workstation_id, type;
  char *connection;

  TRY(PyArg_ParseTuple(args, "isi:openws",
		       &workstation_id, &connection, &type));

  gr_openws(workstation_id, *connection ? connection : NULL, type);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_closews(PyObject *self, PyObject *args)
{
  int workstation_id;

  TRY(PyArg_ParseTuple(args, "i:closews", &workstation_id));

  gr_closews(workstation_id);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_activatews(PyObject *self, PyObject *args)
{
  int workstation_id;

  TRY(PyArg_ParseTuple(args, "i:activatews", &workstation_id));

  gr_activatews(workstation_id);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_deactivatews(PyObject *self, PyObject *args)
{
  int workstation_id;

  TRY(PyArg_ParseTuple(args, "i:deactivatews", &workstation_id));

  gr_deactivatews(workstation_id);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_clearws(PyObject *self, PyObject *args)
{
  TRY(PyArg_ParseTuple(args, ":clearws"));

  gr_clearws();

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_updatews(PyObject *self, PyObject *args)
{
  TRY(PyArg_ParseTuple(args, ":updatews"));

  gr_updatews();

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_polyline(PyObject *self, PyObject *args)
{
  int n;
  float *x, *y;
  PyObject *opx, *opy;

  TRY(PyArg_ParseTuple(args, "iOO:polyline", &n, &opx, &opy));
  TRY(x = floatarray(opx, n, "x"));
  TRY(y = floatarray(opy, n, "y"));

  gr_polyline(n, x, y);

  free(y);
  free(x);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_polymarker(PyObject *self, PyObject *args)
{
  int n;
  float *x, *y;
  PyObject *opx, *opy;

  TRY(PyArg_ParseTuple(args, "iOO:polymarker", &n, &opx, &opy));
  TRY(x = floatarray(opx, n, "x"));
  TRY(y = floatarray(opy, n, "y"));

  gr_polymarker(n, x, y);

  free(y);
  free(x);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_text(PyObject *self, PyObject *args)
{
  float x, y;
  char *string;

  TRY(PyArg_ParseTuple(args, "ffs:text", &x, &y, &string));

  gr_text(x, y, string);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_fillarea(PyObject *self, PyObject *args)
{
  int n;
  float *x, *y;
  PyObject *opx, *opy;

  TRY(PyArg_ParseTuple(args, "iOO:fillarea", &n, &opx, &opy));
  TRY(x = floatarray(opx, n, "x"));
  TRY(y = floatarray(opy, n, "y"));

  gr_fillarea(n, x, y);

  free(y);
  free(x);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_cellarray(PyObject *self, PyObject *args)
{
  float xmin, xmax, ymin, ymax;
  int dimx, dimy, *color, scol = 1, srow = 1, ncol = -1, nrow = -1;
  PyObject *opcolor;

  TRY(PyArg_ParseTuple(args, "ffffiiO|iiii:cellarray",
		       &xmin, &xmax, &ymin, &ymax, &dimx, &dimy, &opcolor,
		       &scol, &srow, &ncol, &nrow));
  TRY(color = intarray(opcolor, dimx * dimy, "color"));

  if (ncol == -1)
    ncol = dimx - scol + 1;
  if (nrow == -1)
    nrow = dimy - srow + 1;

  gr_cellarray(xmin, xmax, ymin, ymax, dimx, dimy, scol, srow, ncol, nrow,
	       color);

  free(color);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_spline(PyObject *self, PyObject *args)
{
  int n, m, smoothing;
  float *x, *y;
  PyObject *opx, *opy;

  TRY(PyArg_ParseTuple(args, "iOOii:fillarea", &n, &opx, &opy, &m, &smoothing));
  TRY(x = floatarray(opx, n, "x"));
  TRY(y = floatarray(opy, n, "y"));

  gr_spline(n, x, y, m, smoothing);

  free(y);
  free(x);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setasf(PyObject *self, PyObject *args)
{
  int *asfs;
  PyObject *opasfs;

  TRY(PyArg_ParseTuple(args, "O:setasf", &opasfs));
  TRY(asfs = intarray(opasfs, 13, "asfs"));

  gr_setasf(asfs);

  free(asfs);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setlineind(PyObject *self, PyObject *args)
{
  int index;

  TRY(PyArg_ParseTuple(args, "i:setlineind", &index));

  gr_setlineind(index);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *_gr_setlinetype(PyObject *self, PyObject *args)
{
  int type;

  TRY(PyArg_ParseTuple(args, "i:setlinetype", &type));

  gr_setlinetype(type);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *_gr_setlinewidth(PyObject *self, PyObject *args)
{
  float width;

  TRY(PyArg_ParseTuple(args, "f:setlinewidth", &width));

  gr_setlinewidth(width);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *_gr_setlinecolorind(PyObject *self, PyObject *args)
{
  int color;

  TRY(PyArg_ParseTuple(args, "i:setlinecolorind", &color));

  gr_setlinecolorind(color);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setmarkerind(PyObject *self, PyObject *args)
{
  int index;

  TRY(PyArg_ParseTuple(args, "i:setmarkerind", &index));

  gr_setmarkerind(index);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setmarkertype(PyObject *self, PyObject *args)
{
  int type;

  TRY(PyArg_ParseTuple(args, "i:setmarkertype", &type));

  gr_setmarkertype(type);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setmarkersize(PyObject *self, PyObject *args)
{
  float size;

  TRY(PyArg_ParseTuple(args, "f:setmarkerize", &size));

  gr_setmarkersize(size);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setmarkercolorind(PyObject *self, PyObject *args)
{
  int color;

  TRY(PyArg_ParseTuple(args, "i:setmarkercolorind", &color));

  gr_setmarkercolorind(color);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_settextind(PyObject *self, PyObject *args)
{
  int index;

  TRY(PyArg_ParseTuple(args, "i:settextind", &index));

  gr_settextind(index);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_settextfontprec(PyObject *self, PyObject *args)
{
  int font, precision;

  TRY(PyArg_ParseTuple(args, "ii:settextfontprec", &font, &precision));

  gr_settextfontprec(font, precision);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setcharexpan(PyObject *self, PyObject *args)
{
  float factor;

  TRY(PyArg_ParseTuple(args, "f:setcharexpan", &factor));

  gr_setcharexpan(factor);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setcharspace(PyObject *self, PyObject *args)
{
  float spacing;

  TRY(PyArg_ParseTuple(args, "f:setcharspace", &spacing));

  gr_setcharspace(spacing);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_settextcolorind(PyObject *self, PyObject *args)
{
  int color;

  TRY(PyArg_ParseTuple(args, "i:settextcolorind", &color));

  gr_settextcolorind(color);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setcharheight(PyObject *self, PyObject *args)
{
  float height;

  TRY(PyArg_ParseTuple(args, "f:setcharheight", &height));

  gr_setcharheight(height);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setcharup(PyObject *self, PyObject *args)
{
  float ux, uy;

  TRY(PyArg_ParseTuple(args, "ff:setcharup", &ux, &uy));

  gr_setcharup(ux, uy);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_settextpath(PyObject *self, PyObject *args)
{
  int path;

  TRY(PyArg_ParseTuple(args, "i:settextpath", &path));

  gr_settextpath(path);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_settextalign(PyObject *self, PyObject *args)
{
  int horizontal, vertical;

  TRY(PyArg_ParseTuple(args, "ii:settextalign", &horizontal, &vertical));

  gr_settextalign(horizontal, vertical);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setfillind(PyObject *self, PyObject *args)
{
  int index;

  TRY(PyArg_ParseTuple(args, "i:setfillind", &index));

  gr_setfillind(index);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setfillintstyle(PyObject *self, PyObject *args)
{
  int style;

  TRY(PyArg_ParseTuple(args, "i:setfillintstyle", &style));

  gr_setfillintstyle(style);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setfillstyle(PyObject *self, PyObject *args)
{
  int index;

  TRY(PyArg_ParseTuple(args, "i:setfillstyle", &index));

  gr_setfillstyle(index);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setfillcolorind(PyObject *self, PyObject *args)
{
  int color;

  TRY(PyArg_ParseTuple(args, "i:setfillcolorind", &color));

  gr_setfillcolorind(color);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setcolorrep(PyObject *self, PyObject *args)
{
  int index;
  float red, green, blue;

  TRY(PyArg_ParseTuple(args, "ifff:setcolorrep", &index, &red, &green, &blue));

  gr_setcolorrep(index, red, green, blue);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setwindow(PyObject *self, PyObject *args)
{
  float xmin, xmax, ymin, ymax;

  TRY(PyArg_ParseTuple(args, "ffff:setwindow", &xmin, &xmax, &ymin, &ymax));

  gr_setwindow(xmin, xmax, ymin, ymax);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_inqwindow(PyObject *self, PyObject *args)
{
  float xmin, xmax, ymin, ymax;

  TRY(PyArg_ParseTuple(args, ":inqwindow"));

  gr_inqwindow(&xmin, &xmax, &ymin, &ymax);

  return Py_BuildValue("ffff", xmin, xmax, ymin, ymax);
}

static
PyObject *_gr_setviewport(PyObject *self, PyObject *args)
{
  float xmin, xmax, ymin, ymax;

  TRY(PyArg_ParseTuple(args, "ffff:setviewport", &xmin, &xmax, &ymin, &ymax));

  gr_setviewport(xmin, xmax, ymin, ymax);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_selntran(PyObject *self, PyObject *args)
{
  int transform;

  TRY(PyArg_ParseTuple(args, "i:selntran", &transform));

  gr_selntran(transform);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setclip(PyObject *self, PyObject *args)
{
  int indicator;

  TRY(PyArg_ParseTuple(args, "i:setclip", &indicator));

  gr_setclip(indicator);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setwswindow(PyObject *self, PyObject *args)
{
  float xmin, xmax, ymin, ymax;

  TRY(PyArg_ParseTuple(args, "ffff:setwswindow", &xmin, &xmax, &ymin, &ymax));

  gr_setwswindow(xmin, xmax, ymin, ymax);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setwsviewport(PyObject *self, PyObject *args)
{
  float xmin, xmax, ymin, ymax;

  TRY(PyArg_ParseTuple(args, "ffff:setwsviewport", &xmin, &xmax, &ymin, &ymax));

  gr_setwsviewport(xmin, xmax, ymin, ymax);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_createseg(PyObject *self, PyObject *args)
{
  int segment;

  TRY(PyArg_ParseTuple(args, "i:createseg", &segment));

  gr_createseg(segment);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_copysegws(PyObject *self, PyObject *args)
{
  int segment;

  TRY(PyArg_ParseTuple(args, "i:copysegws", &segment));

  gr_copysegws(segment);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_redrawsegws(PyObject *self, PyObject *args)
{
  TRY(PyArg_ParseTuple(args, ":redrawsegws"));

  gr_redrawsegws();

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setsegtran(PyObject *self, PyObject *args)
{
  int segment;
  float fx, fy, transx, transy, phi, scalex, scaley;

  TRY(PyArg_ParseTuple(args, "ifffffff:setsegtran",
		       &segment, &fx, &fy, &transx, &transy, &phi,
		       &scalex, &scaley));

  gr_setsegtran(segment, fx, fy, transx, transy, phi, scalex, scaley);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_closeseg(PyObject *self, PyObject *args)
{
  TRY(PyArg_ParseTuple(args, ":closeseg"));

  gr_closeseg();

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_emergencyclosegks(PyObject *self, PyObject *args)
{
  TRY(PyArg_ParseTuple(args, ":emergencyclosegks"));

  gr_emergencyclosegks();

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *_gr_updategks(PyObject *self, PyObject *args)
{
  TRY(PyArg_ParseTuple(args, ":updategks"));

  gr_updategks();

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *_gr_setspace(PyObject *self, PyObject *args)
{
  float zmin, zmax;
  int rotation, tilt, result;

  TRY(PyArg_ParseTuple
      (args, "ffii:setspace", &zmin, &zmax, &rotation, &tilt));
  result = gr_setspace(zmin, zmax, rotation, tilt);

  return Py_BuildValue("i", result);
}

PyObject *_gr_inqspace(PyObject *self, PyObject *args)
{
  float zmin, zmax;
  int rotation, tilt;

  TRY(PyArg_ParseTuple(args, ":inqspace"));

  gr_inqspace(&zmin, &zmax, &rotation, &tilt);

  return Py_BuildValue("ffii", zmin, zmax, rotation, tilt);
}

PyObject *_gr_setscale(PyObject *self, PyObject *args)
{
  int options, result;

  TRY(PyArg_ParseTuple(args, "i:setscale", &options));
  result = gr_setscale(options);

  return Py_BuildValue("i", result);
}

PyObject *_gr_inqscale(PyObject *self, PyObject *args)
{
  int options;

  TRY(PyArg_ParseTuple(args, ":inqscale"));

  gr_inqscale(&options);

  return Py_BuildValue("i", options);
}

PyObject *_gr_textext(PyObject *self, PyObject *args)
{
  float x, y;
  char *string;
  int result;

  TRY(PyArg_ParseTuple(args, "ffs:textext", &x, &y, &string));
  result = gr_textext(x, y, string);

  return Py_BuildValue("i", result);
}

PyObject *_gr_inqtextext(PyObject *self, PyObject *args)
{
  float x, y;
  char *string;
  float tbx[4], tby[4];

  TRY(PyArg_ParseTuple(args, "ffs:inqtextext", &x, &y, &string));

  gr_inqtextext(x, y, string, tbx, tby);

  return Py_BuildValue("[ffff] [ffff]",
		       tbx[0], tbx[1], tbx[2], tbx[3],
		       tby[0], tby[1], tby[2], tby[3]);
}

PyObject *_gr_axes(PyObject *self, PyObject *args)
{
  float x_tick, y_tick, x_org, y_org;
  int major_x, major_y;
  float tick_size;

  TRY(PyArg_ParseTuple(args, "ffffiif:axes", &x_tick, &y_tick,
		       &x_org, &y_org, &major_x, &major_y, &tick_size));

  gr_axes(x_tick, y_tick, x_org, y_org, major_x, major_y, tick_size);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *_gr_grid(PyObject *self, PyObject *args)
{
  float x_tick, y_tick, x_org, y_org;
  int major_x, major_y;

  TRY(PyArg_ParseTuple(args, "ffffii:grid", &x_tick, &y_tick,
		       &x_org, &y_org, &major_x, &major_y));

  gr_grid(x_tick, y_tick, x_org, y_org, major_x, major_y);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *_gr_verrorbars(PyObject *self, PyObject *args)
{
  int n;
  float *px, *py, *e1, *e2;
  PyObject *oppx, *oppy, *ope1, *ope2;

  TRY(PyArg_ParseTuple(args, "iOOOO:verrorbars", &n, &oppx, &oppy,
		       &ope1, &ope2));
  TRY(px = floatarray(oppx, n, "px"));
  TRY(py = floatarray(oppy, n, "py"));
  TRY(e1 = floatarray(ope1, n, "e1"));
  TRY(e2 = floatarray(ope2, n, "e2"));

  gr_verrorbars(n, px, py, e1, e2);

  free(e2);
  free(e1);
  free(py);
  free(px);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *_gr_herrorbars(PyObject *self, PyObject *args)
{
  int n;
  float *px, *py, *e1, *e2;
  PyObject *oppx, *oppy, *ope1, *ope2;

  TRY(PyArg_ParseTuple(args, "iOOOO:herrorbars", &n, &oppx, &oppy,
		       &ope1, &ope2));
  TRY(px = floatarray(oppx, n, "px"));
  TRY(py = floatarray(oppy, n, "py"));
  TRY(e1 = floatarray(ope1, n, "e1"));
  TRY(e2 = floatarray(ope2, n, "e2"));

  gr_herrorbars(n, px, py, e1, e2);

  free(e2);
  free(e1);
  free(py);
  free(px);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *_gr_polyline3d(PyObject *self, PyObject *args)
{
  int n;
  float *px, *py, *pz;
  PyObject *oppx, *oppy, *oppz;

  TRY(PyArg_ParseTuple(args, "iOOO:polyline3d", &n, &oppx, &oppy, &oppz));
  TRY(px = floatarray(oppx, n, "px"));
  TRY(py = floatarray(oppy, n, "py"));
  TRY(pz = floatarray(oppz, n, "pz"));

  gr_polyline3d(n, px, py, pz);

  free(pz);
  free(py);
  free(px);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *_gr_axes3d(PyObject *self, PyObject *args)
{
  float x_tick, y_tick, z_tick, x_org, y_org, z_org;
  int major_x, major_y, major_z;
  float tick_size;

  TRY(PyArg_ParseTuple(args, "ffffffiiif:axes3d",
		       &x_tick, &y_tick, &z_tick, &x_org, &y_org, &z_org,
		       &major_x, &major_y, &major_z, &tick_size));

  gr_axes3d(x_tick, y_tick, z_tick, x_org, y_org, z_org,
	    major_x, major_y, major_z, tick_size);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *_gr_titles3d(PyObject *self, PyObject *args)
{
  char *x_title, *y_title, *z_title;

  TRY(PyArg_ParseTuple(args, "sss:titles3d", &x_title, &y_title, &z_title));

  gr_titles3d(x_title, y_title, z_title);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *_gr_surface(PyObject *self, PyObject *args)
{
  int nx, ny;
  float *px, *py, *pz;
  int option;
  PyObject *oppx, *oppy, *oppz;

  TRY(PyArg_ParseTuple(args, "iiOOOi:surface",
		       &nx, &ny, &oppx, &oppy, &oppz, &option));
  TRY(px = floatarray(oppx, nx, "px"));
  TRY(py = floatarray(oppy, ny, "py"));
  TRY(pz = floatarray(oppz, nx * ny, "pz"));

  gr_surface(nx, ny, px, py, pz, option);

  free(pz);
  free(py);
  free(px);

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *_gr_contour(PyObject *self, PyObject *args)
{
  int nx, ny, nh;
  float *px, *py, *h, *pz;
  int major_h;
  PyObject *oppx, *oppy, *oph, *oppz;

  TRY(PyArg_ParseTuple(args, "iiiOOOOi:contour",
		       &nx, &ny, &nh, &oppx, &oppy, &oph, &oppz, &major_h));
  TRY(px = floatarray(oppx, nx, "px"));
  TRY(py = floatarray(oppy, ny, "py"));
  TRY(h = floatarray(oph, nh, "h"));
  TRY(pz = floatarray(oppz, nx * ny, "pz"));

  gr_contour(nx, ny, nh, px, py, h, pz, major_h);

  free(pz);
  free(h);
  free(py);
  free(px);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setcolormap(PyObject *self, PyObject *args)
{
  int index;

  TRY(PyArg_ParseTuple(args, "i:setcolormap", &index));

  gr_setcolormap(index);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_colormap(PyObject *self, PyObject *args)
{
  TRY(PyArg_ParseTuple(args, ":colormap"));

  gr_colormap();

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_tick(PyObject *self, PyObject *args)
{
  float amin, amax, tick;

  TRY(PyArg_ParseTuple(args, "ff:tick", &amin, &amax));

  tick = gr_tick(amin, amax);

  return Py_BuildValue("f", tick);
}

static
PyObject *_gr_adjustrange(PyObject *self, PyObject *args)
{
  float amin, amax;

  TRY(PyArg_ParseTuple(args, "ff:adjustrange", &amin, &amax));

  gr_adjustrange(&amin, &amax);

  return Py_BuildValue("ff", amin, amax);
}

static
PyObject *_gr_beginprint(PyObject *self, PyObject *args)
{
  char *pathname;

  TRY(PyArg_ParseTuple(args, "s:beginprint", &pathname));

  gr_beginprint(pathname);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_beginprintext(PyObject *self, PyObject *args)
{
  char *pathname, *mode, *format, *orientation;

  TRY(PyArg_ParseTuple(args, "ssss:beginprintext",
		       &pathname, &mode, &format, &orientation));

  gr_beginprintext(pathname, mode, format, orientation);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_endprint(PyObject *self, PyObject *args)
{
  TRY(PyArg_ParseTuple(args, ":endprint"));

  gr_endprint();

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_ndctowc(PyObject *self, PyObject *args)
{
  float x, y;

  TRY(PyArg_ParseTuple(args, "ff:ndctowc", &x, &y));

  gr_ndctowc(&x, &y);

  return Py_BuildValue("ff", x, y);
}

static
PyObject *_gr_wctondc(PyObject *self, PyObject *args)
{
  float x, y;

  TRY(PyArg_ParseTuple(args, "ff:wctondc", &x, &y));

  gr_wctondc(&x, &y);

  return Py_BuildValue("ff", x, y);
}

static
PyObject *_gr_drawrect(PyObject *self, PyObject *args)
{
  float xmin, xmax, ymin, ymax;

  TRY(PyArg_ParseTuple(args, "ffff:drawrect", &xmin, &xmax, &ymin, &ymax));

  gr_drawrect(xmin, xmax, ymin, ymax);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_fillrect(PyObject *self, PyObject *args)
{
  float xmin, xmax, ymin, ymax;

  TRY(PyArg_ParseTuple(args, "ffff:fillrect", &xmin, &xmax, &ymin, &ymax));

  gr_fillrect(xmin, xmax, ymin, ymax);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_drawarc(PyObject *self, PyObject *args)
{
  float xmin, xmax, ymin, ymax;
  int a1, a2;

  TRY(PyArg_ParseTuple(args, "ffffii:drawarc",
		       &xmin, &xmax, &ymin, &ymax, &a1, &a2));

  gr_drawarc(xmin, xmax, ymin, ymax, a1, a2);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_fillarc(PyObject *self, PyObject *args)
{
  float xmin, xmax, ymin, ymax;
  int a1, a2;

  TRY(PyArg_ParseTuple(args, "ffffii:fillarc",
		       &xmin, &xmax, &ymin, &ymax, &a1, &a2));

  gr_fillarc(xmin, xmax, ymin, ymax, a1, a2);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setarrowstyle(PyObject *self, PyObject *args)
{
  int style;

  TRY(PyArg_ParseTuple(args, "i:setarrowstyle", &style));

  gr_setarrowstyle(style);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_drawarrow(PyObject *self, PyObject *args)
{
  float x1, y1, x2, y2;

  TRY(PyArg_ParseTuple(args, "ffff:drawarrow", &x1, &y1, &x2, &y2));

  gr_drawarrow(x1, y1, x2, y2);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_readimage(PyObject *self, PyObject *args)
{
  char *path;
  int width = 0, height = 0, *data, i;
  PyObject *opdata, *result;

  TRY(PyArg_ParseTuple(args, "s:readimage", &path));

  if (!gr_readimage(path, &width, &height, &data))
    {
      opdata = PyList_New(width * height);
      for (i = 0; i < width * height; i++)
	PyList_SetItem(opdata, i, Py_BuildValue("i", data[i]));

      free(data);
    }
  else
    opdata = PyList_New(0);

  result = Py_BuildValue("iiO", width, height, opdata);
  Py_DECREF(opdata);

  return result;
}

static
PyObject *_gr_drawimage(PyObject *self, PyObject *args)
{
  float xmin, xmax, ymin, ymax;
  int width, height, *data;
  PyObject *opdata;

  TRY(PyArg_ParseTuple(args, "ffffiiO:drawimage",
		       &xmin, &xmax, &ymin, &ymax, &width, &height, &opdata));
  TRY(data = intarray(opdata, width * height, "data"));

  gr_drawimage(xmin, xmax, ymin, ymax, width, height, data);

  free(data);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setshadow(PyObject *self, PyObject *args)
{
  float offsetx, offsety, blur;

  TRY(PyArg_ParseTuple(args, "fff:setshadow", &offsetx, &offsety, &blur));

  gr_setshadow(offsetx, offsety, blur);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_settransparency(PyObject *self, PyObject *args)
{
  float alpha;

  TRY(PyArg_ParseTuple(args, "f:settransparency", &alpha));

  gr_settransparency(alpha);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_setcoordxform(PyObject *self, PyObject *args)
{
  PyObject *optran;
  float *tran, mat[3][2];

  TRY(PyArg_ParseTuple(args, "O:setcoordxform", &optran));
  TRY(tran = floatarray(optran, 6, "tran"));
  memcpy(mat, tran, 6 * sizeof(float));

  gr_setcoordxform(mat);

  free(tran);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_begingraphics(PyObject *self, PyObject *args)
{
  char *pathname;

  TRY(PyArg_ParseTuple(args, "s:begingraphics", &pathname));

  gr_begingraphics(pathname);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_endgraphics(PyObject *self, PyObject *args)
{
  TRY(PyArg_ParseTuple(args, ":endgraphics"));

  gr_endgraphics();

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_mathtex(PyObject *self, PyObject *args)
{
  float x, y;
  char *string;

  TRY(PyArg_ParseTuple(args, "ffs:mathtex", &x, &y, &string));
  gr_mathtex(x, y, string);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_beginselection(PyObject *self, PyObject *args)
{
  int index, type;

  TRY(PyArg_ParseTuple(args, "ii:beginselection", &index, &type));

  gr_beginselection(index, type);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_endselection(PyObject *self, PyObject *args)
{
  TRY(PyArg_ParseTuple(args, ":endselection"));

  gr_endselection();

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_moveselection(PyObject *self, PyObject *args)
{
  float x, y;

  TRY(PyArg_ParseTuple(args, "ff:moveselection", &x, &y));

  gr_moveselection(x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_resizeselection(PyObject *self, PyObject *args)
{
  int type;
  float x, y;

  TRY(PyArg_ParseTuple(args, "iff:resizeselection", &type, &x, &y));

  gr_resizeselection(type, x, y);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *_gr_inqbbox(PyObject *self, PyObject *args)
{
  float xmin, xmax, ymin, ymax;

  gr_inqbbox(&xmin, &xmax, &ymin, &ymax);

  return Py_BuildValue("ffff", xmin, xmax, ymin, ymax);
}

static
PyMethodDef gr_methods[] = {
  {"opengks", _gr_opengks, METH_VARARGS},
  {"closegks", _gr_closegks, METH_VARARGS},
  {"inqdspsize", _gr_inqdspsize, METH_VARARGS},
  {"openws", _gr_openws, METH_VARARGS},
  {"closews", _gr_closews, METH_VARARGS},
  {"activatews", _gr_activatews, METH_VARARGS},
  {"deactivatews", _gr_deactivatews, METH_VARARGS},
  {"clearws", _gr_clearws, METH_VARARGS},
  {"updatews", _gr_updatews, METH_VARARGS},
  {"polyline", _gr_polyline, METH_VARARGS},
  {"polymarker", _gr_polymarker, METH_VARARGS},
  {"text", _gr_text, METH_VARARGS},
  {"fillarea", _gr_fillarea, METH_VARARGS},
  {"cellarray", _gr_cellarray, METH_VARARGS},
  {"spline", _gr_spline, METH_VARARGS},
  {"setasf", _gr_setasf, METH_VARARGS},
  {"setlineind", _gr_setlineind, METH_VARARGS},
  {"setlinetype", _gr_setlinetype, METH_VARARGS},
  {"setlinewidth", _gr_setlinewidth, METH_VARARGS},
  {"setlinecolorind", _gr_setlinecolorind, METH_VARARGS},
  {"setmarkerind", _gr_setmarkerind, METH_VARARGS},
  {"setmarkertype", _gr_setmarkertype, METH_VARARGS},
  {"setmarkersize", _gr_setmarkersize, METH_VARARGS},
  {"setmarkercolorind", _gr_setmarkercolorind, METH_VARARGS},
  {"settextind", _gr_settextind, METH_VARARGS},
  {"settextfontprec", _gr_settextfontprec, METH_VARARGS},
  {"setcharexpan", _gr_setcharexpan, METH_VARARGS},
  {"setcharspace", _gr_setcharspace, METH_VARARGS},
  {"settextcolorind", _gr_settextcolorind, METH_VARARGS},
  {"setcharheight", _gr_setcharheight, METH_VARARGS},
  {"setcharup", _gr_setcharup, METH_VARARGS},
  {"settextpath", _gr_settextpath, METH_VARARGS},
  {"settextalign", _gr_settextalign, METH_VARARGS},
  {"setfillind", _gr_setfillind, METH_VARARGS},
  {"setfillintstyle", _gr_setfillintstyle, METH_VARARGS},
  {"setfillstyle", _gr_setfillstyle, METH_VARARGS},
  {"setfillcolorind", _gr_setfillcolorind, METH_VARARGS},
  {"setcolorrep", _gr_setcolorrep, METH_VARARGS},
  {"setwindow", _gr_setwindow, METH_VARARGS},
  {"inqwindow", _gr_inqwindow, METH_VARARGS},
  {"setviewport", _gr_setviewport, METH_VARARGS},
  {"selntran", _gr_selntran, METH_VARARGS},
  {"setclip", _gr_setclip, METH_VARARGS},
  {"setwswindow", _gr_setwswindow, METH_VARARGS},
  {"setwsviewport", _gr_setwsviewport, METH_VARARGS},
  {"createseg", _gr_createseg, METH_VARARGS},
  {"copysegws", _gr_copysegws, METH_VARARGS},
  {"redrawsegws", _gr_redrawsegws, METH_VARARGS},
  {"setsegtran", _gr_setsegtran, METH_VARARGS},
  {"closeseg", _gr_closeseg, METH_VARARGS},
  {"emergencyclosegks", _gr_emergencyclosegks, METH_VARARGS},
  {"updategks", _gr_updategks, METH_VARARGS},
  {"setspace", _gr_setspace, METH_VARARGS},
  {"inqspace", _gr_inqspace, METH_VARARGS},
  {"setscale", _gr_setscale, METH_VARARGS},
  {"inqscale", _gr_inqscale, METH_VARARGS},
  {"textext", _gr_textext, METH_VARARGS},
  {"inqtextext", _gr_inqtextext, METH_VARARGS},
  {"axes", _gr_axes, METH_VARARGS},
  {"grid", _gr_grid, METH_VARARGS},
  {"verrorbars", _gr_verrorbars, METH_VARARGS},
  {"herrorbars", _gr_herrorbars, METH_VARARGS},
  {"polyline3d", _gr_polyline3d, METH_VARARGS},
  {"axes3d", _gr_axes3d, METH_VARARGS},
  {"titles3d", _gr_titles3d, METH_VARARGS},
  {"surface", _gr_surface, METH_VARARGS},
  {"contour", _gr_contour, METH_VARARGS},
  {"setcolormap", _gr_setcolormap, METH_VARARGS},
  {"colormap", _gr_colormap, METH_VARARGS},
  {"tick", _gr_tick, METH_VARARGS},
  {"adjustrange", _gr_adjustrange, METH_VARARGS},
  {"beginprint", _gr_beginprint, METH_VARARGS},
  {"beginprintext", _gr_beginprintext, METH_VARARGS},
  {"endprint", _gr_endprint, METH_VARARGS},
  {"ndctowc", _gr_ndctowc, METH_VARARGS},
  {"wctondc", _gr_wctondc, METH_VARARGS},
  {"drawrect", _gr_drawrect, METH_VARARGS},
  {"fillrect", _gr_fillrect, METH_VARARGS},
  {"drawarc", _gr_drawarc, METH_VARARGS},
  {"fillarc", _gr_fillarc, METH_VARARGS},
  {"setarrowstyle", _gr_setarrowstyle, METH_VARARGS},
  {"drawarrow", _gr_drawarrow, METH_VARARGS},
  {"readimage", _gr_readimage, METH_VARARGS},
  {"drawimage", _gr_drawimage, METH_VARARGS},
  {"setshadow", _gr_setshadow, METH_VARARGS},
  {"settransparency", _gr_settransparency, METH_VARARGS},
  {"setcoordxform", _gr_setcoordxform, METH_VARARGS},
  {"begingraphics", _gr_begingraphics, METH_VARARGS},
  {"endgraphics", _gr_endgraphics, METH_VARARGS},
  {"mathtex", _gr_mathtex, METH_VARARGS},
  {"beginselection", _gr_beginselection, METH_VARARGS},
  {"endselection", _gr_endselection, METH_VARARGS},
  {"moveselection", _gr_moveselection, METH_VARARGS},
  {"resizeselection", _gr_resizeselection, METH_VARARGS},
  {"inqbbox", _gr_inqbbox, METH_VARARGS},
  {NULL, NULL}
};

static
symbol_table_def symbol[] =
{
  {"ASF_BUNDLED", 0},
  {"ASF_INDIVIDUAL", 1},
  {"NOCLIP", 0},
  {"CLIP", 1},
  {"INTSTYLE_HOLLOW", 0},
  {"INTSTYLE_SOLID", 1},
  {"INTSTYLE_PATTERN", 2},
  {"INTSTYLE_HATCH", 3},
  {"TEXT_HALIGN_NORMAL", 0},
  {"TEXT_HALIGN_LEFT", 1},
  {"TEXT_HALIGN_CENTER", 2},
  {"TEXT_HALIGN_RIGHT", 3},
  {"TEXT_VALIGN_NORMAL", 0},
  {"TEXT_VALIGN_TOP", 1},
  {"TEXT_VALIGN_CAP", 2},
  {"TEXT_VALIGN_HALF", 3},
  {"TEXT_VALIGN_BASE", 4},
  {"TEXT_VALIGN_BOTTOM", 5},
  {"TEXT_PATH_RIGHT", 0},
  {"TEXT_PATH_LEFT", 1},
  {"TEXT_PATH_UP", 2},
  {"TEXT_PATH_DOWN", 3},
  {"TEXT_PRECISION_STRING", 0},
  {"TEXT_PRECISION_CHAR", 1},
  {"TEXT_PRECISION_STROKE", 2},
  {"LINETYPE_SOLID", 1},
  {"LINETYPE_DASHED", 2},
  {"LINETYPE_DOTTED", 3},
  {"LINETYPE_DASHED_DOTTED", 4},
  {"LINETYPE_DASH_2_DOT", -1},
  {"LINETYPE_DASH_3_DOT", -2},
  {"LINETYPE_LONG_DASH", -3},
  {"LINETYPE_LONG_SHORT_DASH", -4},
  {"LINETYPE_SPACED_DASH", -5},
  {"LINETYPE_SPACED_DOT", -6},
  {"LINETYPE_DOUBLE_DOT", -7},
  {"LINETYPE_TRIPLE_DOT", -8},
  {"MARKERTYPE_DOT", 1},
  {"MARKERTYPE_PLUS", 2},
  {"MARKERTYPE_ASTERISK", 3},
  {"MARKERTYPE_CIRCLE", 4},
  {"MARKERTYPE_DIAGONAL_CROSS", 5},
  {"MARKERTYPE_SOLID_CIRCLE", -1},
  {"MARKERTYPE_TRIANGLE_UP", -2},
  {"MARKERTYPE_SOLID_TRI_UP", -3},
  {"MARKERTYPE_TRIANGLE_DOWN", -4},
  {"MARKERTYPE_SOLID_TRI_DOWN", -5},
  {"MARKERTYPE_SQUARE", -6},
  {"MARKERTYPE_SOLID_SQUARE", -7},
  {"MARKERTYPE_BOWTIE", -8},
  {"MARKERTYPE_SOLID_BOWTIE", -9},
  {"MARKERTYPE_HOURGLASS", -10},
  {"MARKERTYPE_SOLID_HGLASS", -11},
  {"MARKERTYPE_DIAMOND", -12},
  {"MARKERTYPE_SOLID_DIAMOND", -13},
  {"MARKERTYPE_STAR", -14},
  {"MARKERTYPE_SOLID_STAR", -15},
  {"MARKERTYPE_TRI_UP_DOWN", -16},
  {"MARKERTYPE_SOLID_TRI_RIGHT", -17},
  {"MARKERTYPE_SOLID_TRI_LEFT", -18},
  {"MARKERTYPE_HOLLOW_PLUS", -19},
  {"MARKERTYPE_OMARK", -20},
  {NULL, 0}
};

#if PY_MAJOR_VERSION >= 3

static
struct PyModuleDef gr_module = {
  {},
  "gr",
  0, 
  0,
  gr_methods,
  0,
  0,
  0,
  0,
};

PyObject *PyInit_gr(void)
{
  PyObject *m, *d;
  symbol_table_def *s = symbol;

  m = PyModule_Create(&gr_module);
  if (PyErr_Occurred())
    Py_FatalError("can't initialize module gr");

  d = PyModule_GetDict(m);
  while (s->name != NULL)
    {
      PyDict_SetItemString(d, s->name, PyLong_FromLong(s->value));
      s++;
    }

  return m;
}

#else

void initgr()
{
  PyObject *m, *d;
  symbol_table_def *s = symbol;

  m = Py_InitModule("gr", gr_methods);
  if (PyErr_Occurred())
    Py_FatalError("can't initialize module gr");

  d = PyModule_GetDict(m);
  while (s->name != NULL)
    {
      PyDict_SetItemString(d, s->name, PyInt_FromLong(s->value));
      s++;
    }
}

#endif

#ifdef __MAIN__

int main(int argc, char **argv)
{
#if PY_MAJOR_VERSION >= 3
  wchar_t **wargv = (wchar_t **) PyMem_Malloc(sizeof(wchar_t*) * argc);
  int i, argsize;

  PyImport_AppendInittab("gr", PyInit_gr);
#endif

  Py_Initialize();

#if PY_MAJOR_VERSION >= 3
  for (i = 0; i < argc; i++)
    {
      argsize = strlen(argv[i]);
      wargv[i] = (wchar_t *) PyMem_Malloc((argsize + 1) * sizeof(wchar_t));
      mbstowcs(wargv[i], argv[i], argsize + 1);
    }
  Py_Main(argc, wargv);
#else
  initgr();
  Py_Main(argc, argv);
#endif

  Py_Finalize();

  return 0;
}

#endif

