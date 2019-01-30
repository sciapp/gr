#include <qpixmap.h>
#include <qpainter.h>
#include <qimage.h>
#include <qprinter.h>
#include <QMouseEvent>
#include <QPaintDevice>
#include <QPalette>
#include <QPrintDialog>
#include <qregexp.h>

#include <cmath>
#include <cfloat>
#include <iostream>

#include "glgr.h"
#include "glgrserver.h"

glgrServer *glgr_server = NULL;

GLGrWidget::GLGrWidget(QWidget *parent)
    : QGLWidget(parent), last_context(NULL), title_offset(.12f), subtitle_offset(.10f), legend_offset(.18f),
      proj_rad((float)sqrt(3 * 0.5 * 0.5)), paint_surface(false), paint_contour(false), paint_contour_lines(false),
      paint_axes(false), paint_arrows(false), paint_legend(false), autoscale(false), flipx(false), flipy(false),
      flipz(false), xytype(0), reduction_rate(100), gridColor(0), logX(false), logY(false), logZ(false),
      useCCdata(false), x_tick(0), y_tick(0), z_tick(0), tick_size(.02f), xmin(0), xmax(100), ymin(0), ymax(100),
      zmin(0), zmax(100), x_major_tick(0), y_major_tick(0), z_major_tick(0), paintXAxis(false), paintYAxis(false),
      paintZAxis(false), paintCustomizedXLabel(false), paintCustomizedYLabel(false), paintCustomizedZLabel(false),
      refreshXTextures(false), refreshYTextures(false), refreshZTextures(false), rotation(0), tilt(0), data3D(NULL),
      dataCC(NULL), source_data3D(NULL), source_dataCC(NULL), xdim(0), ydim(0), source_xdim(0), source_ydim(0),
      refreshSurfaceVertices(true), refreshSurfaceIndices(true), refreshSurfaceColors(true), refreshGridIndices(true),
      refreshContourVertices(true), refreshContourIndices(true), refreshContourLineVertices(true),
      surfaceVertices(NULL), surfaceIndices(NULL), surfaceColors(NULL), gridIndices(NULL), contourVertices(NULL),
      contourIndices(NULL), ct(NULL), contourLineNumber(5)
{
  QPalette palette;
  palette.setColor(backgroundRole(), QColor(Qt::white));
  setPalette(palette);
  setGeometry(10, 10, 600, 600);
  createColorTable(0);
  GLfloat *init_data = new GLfloat[4];
  for (int i = 0; i < 4; ++i) init_data[i] = 0;
  setData(2, 2, new GLfloat[4], NULL);
  if (glgr_server == NULL) glgr_server = new glgrServer();
  connect(glgr_server, SIGNAL(newData(QString &)), this, SLOT(get_plot3d_orders(QString &)));
}

void GLGrWidget::mousePressEvent(QMouseEvent *e)
{
  lastMousePos = e->pos();
}

void GLGrWidget::mouseMoveEvent(QMouseEvent *e)
{
  int diffx, diffy;
  QPoint next = e->pos();
  diffx = next.x() - lastMousePos.x();
  diffy = next.y() - lastMousePos.y();

  if (abs(diffx) >= width() / 360)
    {
      if (tilt >= 90 && tilt < 270)
        setRotation((rotation + int(diffx * 360. / width())) % 360);
      else
        setRotation((rotation - int(diffx * 360. / width())) % 360);
      if (rotation < 0) setRotation(360 - rotation);
      lastMousePos.setX(next.x());
      emit rotationChanged(rotation);
    }
  if (abs(diffy) >= height() / 360)
    {
      setTilt((tilt + int(diffy * 360. / height())) % 360);
      if (tilt < 0) setTilt(360 - tilt);
      lastMousePos.setY(next.y());
      emit tiltChanged(tilt);
    }
}

void GLGrWidget::setBoundaries(float xmin_, float xmax_, float ymin_, float ymax_, float zmin_, float zmax_,
                               float cmin_, float cmax_)
{
  if (xmax_ == xmin_) xmax_ += 1;
  if (ymax_ == ymin_) ymax_ += 1;
  if (zmax_ == zmin_) zmax_ += .1f;
  xmin = xmin_;
  xmax = xmax_;
  ymin = ymin_;
  ymax = ymax_;
  if (autoscale)
    calc_autoscale();
  else
    {
      zmin = zmin_;
      zmax = zmax_;
      if (cmin_ == -1 && cmax_ == -1)
        {
          cmin = zmin_;
          cmax = zmax_;
        }
      else
        {
          cmin = cmin_;
          cmax = cmax_;
        }
    }
  update();
}

void GLGrWidget::setData(int xdim_, int ydim_, float *data3D_, float *dataCC_)
{
  if (xdim_ == 1 || ydim_ == 1)
    {
      cerr << "Warning, size of one dimension is only 1\n";
      return;
    }
  if (data3D_ == NULL && dataCC_ && (xdim_ != source_xdim || ydim_ != source_ydim))
    {
      cerr << "Error: ColorCoding-field has wrong dimensions\n";
      return;
    }
  delete[] source_dataCC;
  if (dataCC == source_dataCC) // prevent reduceData() from deleting source_dataCC again
    dataCC = NULL;
  source_dataCC = NULL;

  if (data3D_)
    {
      source_xdim = xdim_;
      source_ydim = ydim_;
      if (source_data3D) delete[] source_data3D;
      if (data3D == source_data3D) // prevent reduceData() from deleting source_data3D again
        data3D = NULL;
      source_data3D = data3D_;
    }

  if (dataCC_)
    {
      source_dataCC = dataCC_;
      colorData_ = dataCC;
      useCCdata = true;
    }
  else
    {
      colorData_ = data3D;
      useCCdata = false;
    }
  reduceData(data3D_);

  if (autoscale) calc_autoscale();

  refreshSurfaceVertices = true;
  refreshSurfaceIndices = true;
  refreshGridIndices = true;
  refreshContourVertices = true;
  refreshContourIndices = true;
  refreshContourLineVertices = true;
  refreshSurfaceColors = true;
  update();
}

void GLGrWidget::setFlipX(bool b)
{
  if (b != flipx)
    {
      flipx = b;
      refreshSurfaceVertices = true;
      refreshContourVertices = true;
      refreshSurfaceColors = true;
      refreshContourLineVertices = true;
      update();
    }
}

void GLGrWidget::setFlipY(bool b)
{
  if (b != flipy)
    {
      flipy = b;
      refreshSurfaceVertices = true;
      refreshContourVertices = true;
      refreshSurfaceColors = true;
      refreshContourLineVertices = true;
      update();
    }
}

void GLGrWidget::setFlipZ(bool b)
{
  if (b != flipz)
    {
      flipz = b;
      refreshSurfaceVertices = true;
      refreshContourVertices = true;
      refreshSurfaceColors = true;
      refreshContourLineVertices = true;
      update();
    }
}

void GLGrWidget::setRotation(int value)
{
  if (rotation != value)
    {
      rotation = value;
      update();
    }
}

void GLGrWidget::setTilt(int value)
{
  if (tilt != value)
    {
      tilt = value;
      update();
    }
}

void GLGrWidget::setPaintSurface(bool b)
{
  if (paint_surface != b)
    {
      paint_surface = b;
      update();
    }
}

void GLGrWidget::setPaintContour(bool b)
{
  if (paint_contour != b)
    {
      paint_contour = b;
      update();
    }
}

void GLGrWidget::setPaintContourLines(bool b)
{
  if (paint_contour_lines != b)
    {
      paint_contour_lines = b;
      update();
    }
}

void GLGrWidget::setContourLineNumber(int b)
{
  if (contourLineNumber != b)
    {
      contourLineNumber = b;
      refreshContourLineVertices = b;
      update();
    }
}

void GLGrWidget::setPaintOrientation(bool b)
{
  if (paint_arrows != b)
    {
      paint_arrows = b;
      update();
    }
}

void GLGrWidget::setPaintLegend(bool b)
{
  if (paint_legend != b)
    {
      paint_legend = b;
      update();
    }
}

void GLGrWidget::setColormap(int c)
{
  createColorTable(c);
  refreshSurfaceColors = true;
  update();
}

void GLGrWidget::setXYType(int v)
{
  if (xytype != v)
    {
      if (v < 0 || v > 3)
        {
          cerr << "wrong xytype. set xytype to 0\n";
          v = 0;
        }
      xytype = v;
      refreshSurfaceVertices = true;
      refreshSurfaceIndices = true;
      refreshGridIndices = true;
      update();
    }
}

void GLGrWidget::setLogX(bool b)
{
  if (logX != b)
    {
      logX = b;
      refreshSurfaceVertices = true;
      refreshContourVertices = true;
      refreshContourLineVertices = true;
      refreshXTextures = true;
      update();
    }
}

void GLGrWidget::setLogY(bool b)
{
  if (logY != b)
    {
      logY = b;
      refreshSurfaceVertices = true;
      refreshContourVertices = true;
      refreshContourLineVertices = true;
      refreshYTextures = true;
      update();
    }
}

void GLGrWidget::setLogZ(bool b)
{
  if (logZ != b)
    {
      logZ = b;
      refreshSurfaceVertices = true;
      refreshContourVertices = true;
      refreshContourLineVertices = true;
      refreshSurfaceColors = true;
      refreshZTextures = true;
      update();
    }
}

void GLGrWidget::setUseCCdata(bool b)
{
  if (b == true && dataCC == NULL)
    {
      cerr << "no ColorCoding-data available.\n";
      return;
    }
  if (b != useCCdata)
    {
      useCCdata = b;
      colorData_ = b ? dataCC : data3D;
      refreshSurfaceColors = true;
      refreshContourLineVertices = true;
      update();
    }
}

// set Axes parameter
void GLGrWidget::setPaintXAxis(bool value)
{
  if (paintXAxis != value)
    {
      paintXAxis = value;
      update();
    }
}

void GLGrWidget::setPaintYAxis(bool value)
{
  if (paintYAxis != value)
    {
      paintYAxis = value;
      update();
    }
}

void GLGrWidget::setPaintZAxis(bool value)
{
  if (paintZAxis != value)
    {
      paintZAxis = value;
      update();
    }
}

void GLGrWidget::setPaintAxes(bool value)
{
  if (paint_axes != value)
    {
      paint_axes = value;
      if (value == false)
        {
          setPaintXAxis(false);
          setPaintYAxis(false);
          setPaintZAxis(false);
        }
      update();
    }
}

void GLGrWidget::setXTick(float tick)
{
  if (tick != 0 && tick != z_tick)
    {
      x_tick = tick;
      setPaintXAxis(true);
      setPaintAxes(true);
      refreshXTextures = true;
      update();
    }
}

void GLGrWidget::setYTick(float tick)
{
  if (tick != 0 && tick != y_tick)
    {
      y_tick = tick;
      setPaintYAxis(true);
      setPaintAxes(true);
      refreshYTextures = true;
      update();
    }
}

void GLGrWidget::setZTick(float tick)
{
  if (tick != 0 && tick != z_tick)
    {
      z_tick = tick;
      setPaintZAxis(true);
      setPaintAxes(true);
      refreshZTextures = true;
      update();
    }
}

void GLGrWidget::setMajorX(int major)
{
  if (major != x_major_tick && major != 0)
    {
      x_major_tick = major;
      refreshXTextures = true;
      update();
    }
}

void GLGrWidget::setMajorY(int major)
{
  if (major != y_major_tick && major != 0)
    {
      y_major_tick = major;
      refreshYTextures = true;
      update();
    }
}

void GLGrWidget::setMajorZ(int major)
{
  if (major != z_major_tick && major != 0)
    {
      z_major_tick = major;
      refreshZTextures = true;
      update();
    }
}


void GLGrWidget::setTickSize(float value)
{
  if (tick_size != value)
    {
      tick_size = value;
      update();
    }
}

void GLGrWidget::setXMin(float value)
{
  if (xmin != value)
    {
      xmin = value;
      refreshXTextures = true;
      update();
    }
}

void GLGrWidget::setXMax(float value)
{
  if (xmax != value)
    {
      xmax = value;
      refreshXTextures = true;
      update();
    }
}

void GLGrWidget::setYMin(float value)
{
  if (ymin != value)
    {
      ymin = value;
      refreshYTextures = true;
      update();
    }
}

void GLGrWidget::setYMax(float value)
{
  if (ymax != value)
    {
      ymax = value;
      refreshYTextures = true;
      update();
    }
}

void GLGrWidget::setZMin(float value)
{

  if (!autoscale && zmin != value)
    {
      zmin = value;
      refreshSurfaceVertices = true;
      refreshContourVertices = true;
      refreshContourLineVertices = true;
      refreshZTextures = true;
      if (!useCCdata) refreshSurfaceColors = true;
      update();
    }
}

void GLGrWidget::setZMax(float value)
{
  if (!autoscale && zmax != value)
    {
      zmax = value;
      refreshSurfaceVertices = true;
      refreshContourVertices = true;
      refreshContourLineVertices = true;
      refreshZTextures = true;
      if (!useCCdata) refreshSurfaceColors = true;
      update();
    }
}

void GLGrWidget::setCMin(float value)
{
  if (cmin != value)
    {
      cmin = value;
      if (useCCdata) refreshSurfaceColors = true;
      update();
    }
}

void GLGrWidget::setCMax(float value)
{
  if (cmax != value)
    {
      cmax = value;
      if (useCCdata) refreshSurfaceColors = true;
      update();
    }
}

void GLGrWidget::setAutoScale(bool b)
{
  if (autoscale != b)
    {
      autoscale = b;
      if (b) calc_autoscale();
    }
}

void GLGrWidget::setReductionRate(int rate)
{
  if (rate < 1)
    {
      cerr << "Reduction Rate is negative. Change to 1\n";
      rate = 1;
    }
  if (rate > 100)
    {
      cerr << "Reduction Rate is too high. Change to 100\n";
      rate = 100;
    }
  if (reduction_rate != rate)
    {
      reduction_rate = rate;
      reduceData();
      if (autoscale) calc_autoscale();
      refreshSurfaceVertices = true;
      refreshSurfaceIndices = true;
      refreshGridIndices = true;
      refreshContourVertices = true;
      refreshContourIndices = true;
      refreshContourLineVertices = true;
      refreshSurfaceColors = true;
      update();
    }
}

void GLGrWidget::addCustomizedLabel(int axis, float value, const QString &text)
{
  CustomizedLabel newLabel;
  newLabel.value = value;
  newLabel.text = text;
  switch (axis)
    {
    case 0:
      x_indiv_label.push_back(newLabel);
      break;
    case 1:
      y_indiv_label.push_back(newLabel);
      break;
    case 2:
      z_indiv_label.push_back(newLabel);
      break;
    default:
      cerr << "warning: wrong axis in addCustomizedLabel\n";
    }
}

void GLGrWidget::resetCustomizedLabel(int axis)
{
  switch (axis)
    {
    case 0:
      x_indiv_label.clear();
      break;
    case 1:
      y_indiv_label.clear();
      break;
    case 2:
      z_indiv_label.clear();
      break;
    default:
      cerr << "warning: wrong axis in resetCustomizedLabel\n";
    }
}

void GLGrWidget::setPaintCustomizedLabel(int axis, bool b)
{
  switch (axis)
    {
    case 0:
      paintCustomizedXLabel = b;
      break;
    case 1:
      paintCustomizedYLabel = b;
      break;
    case 2:
      paintCustomizedZLabel = b;
      break;
    default:
      cerr << "warning: wrong axis in setPaintCustomizedLabel\n";
    }
}


void GLGrWidget::setXLabel(const QString t)
{
  //  (t.toLatin1());

  if (t != x_axis_label)
    {
      x_axis_label = t;
      update();
    }
}


void GLGrWidget::setYLabel(const QString t)
{
  //  (t.toLatin1());
  if (t != y_axis_label)
    {
      y_axis_label = t;
      update();
    }
}


void GLGrWidget::setZLabel(const QString t)
{
  //  setZLabel(t.toLatin1());
  if (t != z_axis_label)
    {
      z_axis_label = t;
      update();
    }
}

void GLGrWidget::setGridColor(int nr)
{
  nr = nr % 8;
  if (nr != gridColor) gridColor = nr;
  update();
}


// GL-functions
void GLGrWidget::initializeGL()
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(1.0, 1.0, 1.0, 0.0);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void GLGrWidget::resizeGL(int w, int h)
{
  glViewport(0, 0, w, h);
}

void GLGrWidget::paintGL()
{
  // start modifying the projection matrix.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  upper_offset = 0;
  if (title != "") upper_offset += title_offset;
  if (subtitle != "") upper_offset += subtitle_offset;

  left_offset = 0;
  if (paint_legend || paint_arrows) left_offset = legend_offset;
  glOrtho(.5 - proj_rad - left_offset, .5 + proj_rad, .5 - proj_rad - .08, .5 + proj_rad + .08 + upper_offset, -100,
          100);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // start modifying camera matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.5, 0.5, -0.5);
  glRotatef(tilt, 1, 0, 0);
  glRotatef(-rotation, 0, 1, 0);
  glTranslatef(-0.5, -0.5, 0.5);

  // get actual camera-matrix
  glGetFloatv(GL_MODELVIEW_MATRIX, &camMat[0][0]);

  // create new Axes-Textures if necessary
  if ((refreshXTextures && paintXAxis && paint_axes) || last_context != context())
    {
      createTextureArray(xmin, xmax, x_tick, x_major_tick, x_textures, paintCustomizedXLabel ? &x_indiv_label : NULL,
                         logX);
      refreshXTextures = false;
    }
  if ((refreshYTextures && paintYAxis && paint_axes) || last_context != context())
    {
      createTextureArray(ymin, ymax, y_tick, y_major_tick, y_textures, paintCustomizedYLabel ? &y_indiv_label : NULL,
                         logY);
      refreshYTextures = false;
    }
  if ((refreshZTextures && paintZAxis && paint_axes) || last_context != context())
    {
      createTextureArray(zmin, zmax, z_tick, z_major_tick, z_textures, paintCustomizedZLabel ? &z_indiv_label : NULL,
                         logZ);
      refreshZTextures = false;
    }
  if (refreshSurfaceVertices && (paint_surface || xytype != 0))
    {
      createSurfaceVertices();
      refreshSurfaceVertices = false;
    }
  if (refreshSurfaceIndices && (paint_surface || xytype != 0))
    {
      createSurfaceIndices();
      refreshSurfaceIndices = false;
    }
  if (refreshSurfaceColors && ((paint_surface && (xytype == 0 || xytype == 3)) || paint_contour))
    {
      createSurfaceColors();
      refreshSurfaceColors = false;
    }
  if (refreshGridIndices && xytype != 0)
    {
      createGridIndices();
      refreshGridIndices = false;
    }
  if (refreshContourVertices && paint_contour)
    {
      createContourVertices();
      refreshContourVertices = false;
    }
  if (refreshContourIndices && paint_contour)
    {
      createContourIndices();
      refreshContourIndices = false;
    }
  if (refreshContourLineVertices && paint_contour_lines)
    {
      createContourLineVertices();
      refreshContourLineVertices = false;
    }

  last_context = context();

  if (paint_surface || xytype > 0) paintSurface();
  if ((xytype > 0 || paint_surface) && (xytype == 1 || xytype == 2)) paintGrid();
  if (paint_contour) paintContour();
  if (paint_contour_lines)
    {
      paintContourLines();
    }
  if (paint_axes) paintAxes();

  paintTitle();
  paintSubTitle();

  if (paint_arrows) paintOrientation();

  if (paint_legend) paintLegend();

  glFlush();
}

// GL vectors
void GLGrWidget::createSurfaceVertices()
{
  if (surfaceVertices) delete[] surfaceVertices;

  int index = 0;

  float numerator;

  if (xytype == 3 || xytype == 0)
    {
      surfaceVertices = new GLfloat[xdim * ydim * 3];

      for (int j = 0; j < ydim; ++j)
        for (int i = 0; i < xdim; ++i)
          {
            // calculate x - coordinate
            surfaceVertices[index++] = logXValue(i);
            // calculate z - coordinate
            if (!logZ)
              {
                surfaceVertices[index] = (data(i, j) - zmin) / (zmax - zmin);
              }
            else
              {
                numerator = (data(i, j) - zmin);
                if (numerator == 0) numerator = 1;
                surfaceVertices[index] = log(numerator) / log(zmax - zmin);
              }
            if (flipz) surfaceVertices[index] = surfaceVertices[index] * -1 + 1;
            index++;
            // calculate y - coordinate
            surfaceVertices[index++] = logYValue(j);
          }
    }
  else if (xytype == 1)
    {
      surfaceVertices = new GLfloat[xdim * ydim * 3 * 2];
      for (int j = 0; j < ydim; ++j)
        for (int i = 0; i < xdim; ++i)
          {
            // calculate x - coordinates
            surfaceVertices[index++] = logXValue(i);
            // calculate z - coordinate
            surfaceVertices[index++] = flipz ? 1 : 0;
            // calculate y - coordinate
            surfaceVertices[index++] = logYValue(j);

            surfaceVertices[index] = surfaceVertices[index - 3];
            index++;
            numerator = (data(i, j) - zmin);
            if (!logZ)
              surfaceVertices[index] = numerator / (zmax - zmin);
            else
              {
                if (numerator == 0) numerator = 1;
                surfaceVertices[index] = log(numerator) / log(zmax - zmin);
              }
            if (flipz) surfaceVertices[index] = surfaceVertices[index] * -1 + 1;
            index++;
            surfaceVertices[index] = surfaceVertices[index - 3];
            index++;
          }
    }
  else if (xytype == 2)
    {
      surfaceVertices = new GLfloat[xdim * ydim * 3 * 2];
      for (int i = 0; i < xdim; ++i)
        for (int j = 0; j < ydim; ++j)
          {
            // calculate x - coordinates
            surfaceVertices[index++] = logXValue(i);
            // calculate z - coordinate
            surfaceVertices[index++] = flipz ? 1 : 0;
            // calculate y - coordinate
            surfaceVertices[index++] = logYValue(j);

            surfaceVertices[index] = surfaceVertices[index - 3];
            index++;
            numerator = (data(i, j) - zmin);
            if (!logZ)
              surfaceVertices[index] = numerator / (zmax - zmin);
            else
              {
                if (numerator == 0) numerator = 1;
                surfaceVertices[index] = log(numerator) / log(zmax - zmin);
              }
            if (flipz) surfaceVertices[index] = surfaceVertices[index] * -1 + 1;
            index++;
            surfaceVertices[index] = surfaceVertices[index - 3];
            index++;
          }
    }
}

void GLGrWidget::createSurfaceIndices()
{
  if (surfaceIndices)
    {
      for (int i = 0; i < surfaceIndicesDim; ++i)
        if (surfaceIndices[i]) delete[] surfaceIndices[i];
      delete[] surfaceIndices;
    }

  if (xytype == 0 || xytype == 3)
    {
      surfaceIndicesDim = ydim - 1;
      surfaceIndices = new GLuint *[surfaceIndicesDim];
      for (int i = 0; i < surfaceIndicesDim; ++i)
        {
          surfaceIndices[i] = new GLuint[xdim * 2];
          for (int j = 0; j < xdim; ++j)
            {
              surfaceIndices[i][j * 2] = i * xdim + j;
              surfaceIndices[i][j * 2 + 1] = (i + 1) * xdim + j;
            }
        }
    }
  else
    {
      surfaceIndicesDim = ydim;
      surfaceIndices = new GLuint *[surfaceIndicesDim];
      for (int i = 0; i < surfaceIndicesDim; ++i)
        {
          surfaceIndices[i] = new GLuint[xdim * 2];
          for (int j = 0; j < xdim * 2; ++j) surfaceIndices[i][j] = 2 * i * xdim + j;
        }
    }
}

void GLGrWidget::createSurfaceColors()
{
  float max, min;
  bool maximum_too_low = false;
  QColor a;

  if (surfaceColors) delete[] surfaceColors;
  surfaceColors = new GLfloat[xdim * ydim * 3];

  int index = 0, ct_index;

  float numerator, denominator;
  if (useCCdata)
    {
      max = cmax;
      min = cmin;
    }
  else
    {
      max = zmax;
      min = zmin;
    }
  if (logZ)
    denominator = log(max - min);
  else
    denominator = max - min;
  for (int j = 0; j < ydim; ++j)
    for (int i = 0; i < xdim; ++i)
      {
        numerator = colorData(i, j) - min;
        if (logZ)
          {
            if (denominator == 0) denominator = 1;
            numerator = log(numerator);
            if (numerator > denominator) maximum_too_low = true;
            ct_index = (int)(numerator / denominator * 71);
          }
        else
          {
            if (numerator > denominator) maximum_too_low = true;
            ct_index = (int)(numerator / denominator * 71);
          }
        if (ct_index < 0) // negative index  at small values in logarithmic display-mode
          ct_index = 0;
        a = colorTable[ct_index % 72];

        surfaceColors[index++] = a.red() / 255.0;
        surfaceColors[index++] = a.green() / 255.0;
        surfaceColors[index++] = a.blue() / 255.0;
      }
  if (maximum_too_low) cerr << "warning, maximum for color_data too low\n";
}

void GLGrWidget::createGridIndices()
{
  int i, j;

  if (gridIndices)
    {
      for (i = 0; i < gridIndicesDim; ++i)
        if (gridIndices[i]) delete[] gridIndices[i];
      delete[] gridIndices;
    }

  if (xytype == 0 || xytype == 3)
    {
      gridIndicesDim = ydim - 1;
      gridIndices = new GLuint *[gridIndicesDim];
      for (i = 0; i < gridIndicesDim; ++i)
        {
          gridIndices[i] = new GLuint[xdim * 2];
          for (j = 0; j < xdim; ++j)
            {
              gridIndices[i][j * 2] = i * xdim + j;
              gridIndices[i][j * 2 + 1] = (i + 1) * xdim + j;
            }
        }
    }
  else
    {
      gridIndicesDim = ydim + 2;
      gridIndices = new GLuint *[gridIndicesDim];
      for (i = 0; i < gridIndicesDim - 2; ++i)
        {
          gridIndices[i] = new GLuint[xdim];
          for (j = 0; j < xdim; ++j)
            {
              gridIndices[i][j] = i * xdim * 2 + j * 2 + 1;
            }
        }
      gridIndices[gridIndicesDim - 1] = new GLuint[ydim];
      gridIndices[gridIndicesDim - 2] = new GLuint[ydim];
      for (i = 0; i < ydim; ++i)
        {
          gridIndices[gridIndicesDim - 2][i] = i * xdim * 2 + 1;
          gridIndices[gridIndicesDim - 1][i] = (i + 1) * xdim * 2 - 1;
        }
    }
}

void GLGrWidget::createColorTable(int ct)
{
  // 0: UNIFORM, 1: TEMPERATURE, 2: GRAYSCALE,
  // 3: GLOWING, 4: RAINBOW, 5: GEOLOGIC,
  // 6: GREENSCALE, 7: CYANSCALE, 8: BLUESCALE,
  // 9: MAGENTASCALE, 10: REDSCALE, 11: FLAME;
  double x;
  float h, l, s, r, g, b;
  QColor c;

  for (int i = 0; i < 72; i++)
    {
      x = (double)i / (double)(72);

      switch (ct)
        {
        case 0: // UNIFORM
        case 1: // TEMPERATURE
          if (ct == 0)
            h = i * 360.0 / 72 - 120;
          else
            h = 270 - i * 300.0 / 72;
          if (h < 0) h += 360;

          l = 255;
          s = 255;

          c = QColor::fromHsv((int)h, (int)s, (int)l);
          break;

        case 2: // GRAYSCALE
          r = x;
          g = x;
          b = x;
          c = QColor::fromRgb((int)(r * 255), (int)(g * 255), (int)(b * 255));
          break;

        case 3: // GLOWING
          r = pow(x, 1.0 / 4.0);
          g = x;
          b = pow(x, 4.0);
          c = QColor::fromRgb((int)(r * 255), (int)(g * 255), (int)(b * 255));
          break;

        case 4:  // RAINBOW
        case 11: // FLAME
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

          if (ct == 11)
            {
              r = 1.0 - r;
              g = 1.0 - g;
              b = 1.0 - b;
            }
          c = QColor::fromRgb((int)(r * 255), (int)(g * 255), (int)(b * 255));
          break;

        case 5: // GEOLOGIC
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
          c = QColor::fromRgb((int)(r * 255), (int)(g * 255), (int)(b * 255));
          break;

        case 6: // GREENSCALE
          r = x;
          g = pow(x, 1.0 / 4.0);
          b = pow(x, 4.0);
          c = QColor::fromRgb((int)(r * 255), (int)(g * 255), (int)(b * 255));
          break;

        case 7: // CYANSCALE
          r = pow(x, 4.0);
          g = pow(x, 1.0 / 4.0);
          b = x;
          c = QColor::fromRgb((int)(r * 255), (int)(g * 255), (int)(b * 255));
          break;

        case 8: // BLUESCALE
          r = pow(x, 4.0);
          g = x;
          b = pow(x, 1.0 / 4.0);
          c = QColor::fromRgb((int)(r * 255), (int)(g * 255), (int)(b * 255));
          break;

        case 9: // MAGENTASCALE
          r = x;
          g = pow(x, 4.0);
          b = pow(x, 1.0 / 4.0);
          c = QColor::fromRgb((int)(r * 255), (int)(g * 255), (int)(b * 255));
          break;

        case 10: // REDSCALE
          r = pow(x, 1.0 / 4.0);
          g = pow(x, 4.0);
          b = x;
          c = QColor::fromRgb((int)(r * 255), (int)(g * 255), (int)(b * 255));
          break;
        };

      colorTable[i] = c;
    }
}

void GLGrWidget::createContourVertices()
{
  if (contourVertices) delete[] contourVertices;

  contourVertices = new GLfloat[xdim * ydim * 3];

  int index = 0;

  for (int i = 0; i < ydim; ++i)
    for (int j = 0; j < xdim; ++j)
      {
        contourVertices[index++] = logXValue(j);
        contourVertices[index++] = 0;
        contourVertices[index++] = logYValue(i);
      }
}

void GLGrWidget::createContourIndices()
{
  if (contourIndices)
    {
      for (int i = 0; i < contourIndicesDim; ++i)
        if (contourIndices[i]) delete[] contourIndices[i];
      delete[] contourIndices;
    }

  contourIndicesDim = ydim - 1;
  contourIndices = new GLuint *[contourIndicesDim];
  for (int i = 0; i < contourIndicesDim; ++i)
    {
      contourIndices[i] = new GLuint[xdim * 2];
      for (int j = 0; j < xdim; ++j)
        {
          contourIndices[i][j * 2] = i * xdim + j;
          contourIndices[i][j * 2 + 1] = (i + 1) * xdim + j;
        }
    }
}


void GLGrWidget::createContourLineVertices()
{
  float value, min, max;
  int i, j, k;

  // delete old matrix
  if (ct)
    {
      for (j = 0; j < ctDim; ++j) delete[] ct[j];
      delete[] ct;
    }
  // create new matrix
  ct = new int *[xdim];
  for (j = 0; j < xdim; ++j) ct[j] = new int[ydim];
  ctDim = xdim;

  contourLineVertices.clear();

  min = useCCdata ? cmin : zmin;
  max = useCCdata ? cmax : zmax;

  for (k = 1; k < contourLineNumber + 1; ++k)
    {
      value = min + ((float)k / (contourLineNumber + 1)) * (max - min);
      for (j = ydim - 1; j >= 0; --j)
        for (i = 0; i < xdim; ++i) ct[i][j] = colorData(i, j) >= value ? ct_upper : ct_lower;

      for (j = 0; j < ydim; ++j)
        for (i = 0; i < xdim; ++i)
          {
            if (j - 1 >= 0 && ct[i][j - 1] == ct_lower && ct[i][j] & ct_upper && !(ct[i][j] & ct_downway))
              {
                find_way(i, j, ct_downway, value);
              }
            if (j + 1 < ydim && ct[i][j + 1] == ct_lower && ct[i][j] & ct_upper && !(ct[i][j] & ct_upway))
              {
                find_way(i, j, ct_upway, value);
              }
            if (i + 1 < xdim && ct[i + 1][j] == ct_lower && ct[i][j] & ct_upper && !(ct[i][j] & ct_rightway))
              {
                find_way(i, j, ct_rightway, value);
              }
            if (i - 1 >= 0 && ct[i - 1][j] == ct_lower && ct[i][j] & ct_upper && !(ct[i][j] & ct_leftway))
              {
                find_way(i, j, ct_leftway, value);
              }
          }
    }
}

void GLGrWidget::find_way(int x, int y, int direction, float value)
{
  int flag = 0;

  while (!flag)
    {
      if (direction & ct[x][y]) flag = 1;
      ct[x][y] |= direction;

      if (direction == ct_upway)
        if (y + 1 < ydim)
          {
            if (ct[x][y + 1] == ct_lower)
              {
                contourLineVertices.push_back(
                    vec(logXValue((float)x), 0,
                        logYValue((float)y + (colorData(x, y) - value) / (colorData(x, y) - colorData(x, y + 1)))));
                direction = ct_leftway;
              }
            else
              {
                y += 1;
                direction = ct_rightway;
              }
          }
        else
          break;

      else if (direction == ct_downway)
        if (y - 1 >= 0)
          {
            if (ct[x][y - 1] == ct_lower)
              {
                contourLineVertices.push_back(
                    vec(logXValue((float)x), 0,
                        logYValue((float)y - (colorData(x, y) - value) / (colorData(x, y) - colorData(x, y - 1)))));
                direction = ct_rightway;
              }
            else
              {
                y -= 1;
                direction = ct_leftway;
              }
          }
        else
          break;

      else if (direction == ct_leftway)
        if (x - 1 >= 0)
          {
            if (ct[x - 1][y] == ct_lower)
              {
                contourLineVertices.push_back(
                    vec(logXValue((float)x - (colorData(x, y) - value) / (colorData(x, y) - colorData(x - 1, y))), 0,
                        logYValue((float)y)));
                direction = ct_downway;
              }
            else
              {
                x -= 1;
                direction = ct_upway;
              }
          }
        else
          break;

      else if (direction == ct_rightway)
        {
          if (x + 1 < xdim)
            {
              if (ct[x + 1][y] == ct_lower)
                {
                  contourLineVertices.push_back(
                      vec(logXValue((float)x + (colorData(x, y) - value) / (colorData(x, y) - colorData(x + 1, y))), 0,
                          logYValue((float)y)));
                  direction = ct_upway;
                }
              else
                {
                  x += 1;
                  direction = ct_downway;
                }
            }
          else
            break;
        }
    }
  contourLineVertices.push_back(vec(0, 1000, 0));
}

void GLGrWidget::createTextureArray(float min, float max, float tick, int major, vector<TickTexture> &textures,
                                    vector<CustomizedLabel> *indiv, bool is_log)
{
  TickTexture new_texture;

  for (vector<TickTexture>::iterator it = textures.begin(); it != textures.end(); ++it)
    glDeleteTextures(1, &(it->texture.texName));
  textures.clear();

  if (indiv != NULL)
    {
      for (vector<CustomizedLabel>::iterator it = indiv->begin(); it != indiv->end(); ++it)
        {
          new_texture.texture = load_texture(it->text);
          new_texture.value = it->value;
          new_texture.height = LABEL_HEIGHT;
          new_texture.width = new_texture.height / new_texture.texture.textHeight * new_texture.texture.textWidth;
          new_texture.xcut = (float)new_texture.texture.textWidth / new_texture.texture.width;
          new_texture.ycut = (float)new_texture.texture.textHeight / new_texture.texture.height;
          textures.push_back(new_texture);
        }
    }
  else
    {
      if (!is_log)
        {
          float first_value;
          if (tick)
            first_value = (((int)(min / tick))) * tick;
          else
            first_value = max + 1;
          if (first_value < min) first_value += tick;

          for (int tick_counter = 0; first_value <= max; first_value += tick, tick_counter++)
            {
              if (major != 0 && tick_counter % major == 0)
                {
                  new_texture.texture = load_texture(QString("%1").arg(first_value, 0, 'g'));
                  new_texture.value = first_value;
                  new_texture.height = LABEL_HEIGHT;
                  new_texture.width =
                      new_texture.height / new_texture.texture.textHeight * new_texture.texture.textWidth;
                  new_texture.xcut = (float)new_texture.texture.textWidth / new_texture.texture.width;
                  new_texture.ycut = (float)new_texture.texture.textHeight / new_texture.texture.height;
                  textures.push_back(new_texture);
                }
            }
        }
      else
        {
          for (int i = 1; i <= max; i *= 10)
            {
              new_texture.texture = load_texture(QString("%1").arg(i));
              new_texture.value = i;
              new_texture.height = LABEL_HEIGHT;
              new_texture.width = new_texture.height / new_texture.texture.textHeight * new_texture.texture.textWidth;
              new_texture.xcut = (float)new_texture.texture.textWidth / new_texture.texture.width;
              new_texture.ycut = (float)new_texture.texture.textHeight / new_texture.texture.height;
              textures.push_back(new_texture);
            }
        }
    }
}

void GLGrWidget::reduceData(bool reduce3D)
{
  if (reduce3D && data3D && data3D != source_data3D) delete[] data3D;
  if (dataCC && dataCC != source_dataCC) delete[] dataCC;
  if (reduction_rate < 100 && reduction_rate > 0)
    {
      xdim = source_xdim * reduction_rate / 100;
      if (xdim < 2) xdim = 2;
      ydim = source_ydim * reduction_rate / 100;
      if (ydim < 2) ydim = 2;
      if (reduce3D)
        {
          data3D = new float[xdim * ydim];
          rebin(source_data3D, source_xdim, source_ydim, data3D, xdim, ydim);
        }
      if (source_dataCC)
        {
          dataCC = new float[xdim * ydim];
          rebin(source_dataCC, source_xdim, source_ydim, dataCC, xdim, ydim);
        }
      else
        dataCC = NULL;
    }
  else
    {
      xdim = source_xdim;
      ydim = source_ydim;
      data3D = source_data3D;
      dataCC = source_dataCC;
    }
  colorData_ = useCCdata ? dataCC : data3D;
}

// paint functions
void GLGrWidget::paintSurface()
{
  glColor3f(1.0, 1.0, 1.0);
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  glVertexPointer(3, GL_FLOAT, 0, surfaceVertices);
  glEnableClientState(GL_VERTEX_ARRAY);
  if (paint_surface && (xytype == 0 || xytype == 3))
    {
      glColorPointer(3, GL_FLOAT, 0, surfaceColors);
      glEnableClientState(GL_COLOR_ARRAY);
    }
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  for (int i = 0; i < ydim - 1; ++i) glDrawElements(GL_TRIANGLE_STRIP, xdim * 2, GL_UNSIGNED_INT, surfaceIndices[i]);
  glDisable(GL_POLYGON_OFFSET_FILL);
  glDisableClientState(GL_VERTEX_ARRAY);
  if (paint_surface && (xytype == 0 || xytype == 3)) glDisableClientState(GL_COLOR_ARRAY);
}

void GLGrWidget::paintGrid()
{
  int j;
  glVertexPointer(3, GL_FLOAT, 0, surfaceVertices);
  glEnableClientState(GL_VERTEX_ARRAY);
  switch (gridColor)
    {
    case 0:
      glColor3f(0, 0, 0);
      break;
    case 1:
      glColor3f(1, 0, 0);
      break;
    case 2:
      glColor3f(0, 1, 0);
      break;
    case 3:
      glColor3f(0, 0, 1);
      break;
    case 4:
      glColor3f(0, 1, 1);
      break;
    case 5:
      glColor3f(1, 1, 0);
      break;
    case 6:
      glColor3f(1, 0, 1);
      break;
    case 7:
      glColor3f(.5, .5, .5);
      break;
    }

  glPolygonMode(GL_BACK, GL_LINE);
  glPolygonMode(GL_FRONT, GL_LINE);
  if (xytype == 0 || xytype == 3)
    for (j = 0; j < ydim - 1; ++j) glDrawElements(GL_QUAD_STRIP, xdim * 2, GL_UNSIGNED_INT, gridIndices[j]);
  else
    {
      for (j = 0; j < ydim; ++j) glDrawElements(GL_LINE_STRIP, xdim, GL_UNSIGNED_INT, gridIndices[j]);
      for (j = ydim; j < ydim + 2; ++j) glDrawElements(GL_LINE_STRIP, ydim, GL_UNSIGNED_INT, gridIndices[j]);
    }
  //  glLineStipple(2,0xAAAA);
  //   glEnable(GL_LINE_STIPPLE);
  //   glPolygonMode(GL_FRONT,GL_LINE);
  //   glPolygonMode(GL_BACK,GL_POINT);
  //   for(int j=0;j<ydim-1;++j)
  //    glDrawElements(GL_QUAD_STRIP, xdim*2,GL_UNSIGNED_INT,surfaceIndices[j]);
  //   glDisable(GL_LINE_STIPPLE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDisableClientState(GL_VERTEX_ARRAY);
}

void GLGrWidget::paintContour()
{
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  glColorPointer(3, GL_FLOAT, 0, surfaceColors);
  glVertexPointer(3, GL_FLOAT, 0, contourVertices);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  for (int i = 0; i < contourIndicesDim; ++i)
    glDrawElements(GL_TRIANGLE_STRIP, xdim * 2, GL_UNSIGNED_INT, contourIndices[i]);
  glDisable(GL_POLYGON_OFFSET_FILL);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
}

void GLGrWidget::paintContourLines()
{
  glColor3f(0, 0, 0);

  glBegin(GL_LINE_STRIP);
  for (vector<vec>::iterator it = contourLineVertices.begin(); it != contourLineVertices.end(); ++it)
    {
      if (it->y() != 0)
        {
          glEnd();
          glBegin(GL_LINE_STRIP);
        }
      else
        glVertex3fv(it->getv());
    }
  glEnd();
}

void GLGrWidget::paintAxes()
{
  bool flap, upside_down;
  bool axis_at_bottom = true;

  vec tick_direction, start, start_z, test_pos;
  TickTexture label_texture;

  if (vec(0, 0, 1).transform2d(camMat).quadNorm() > .5 && vec(1, 0, 0).transform2d(camMat).quadNorm() > .5)
    flap = 0;
  else
    flap = 1;

  if (vec(0.5, 0, 0).transform2d(camMat).y() <= vec(0.5, 1, 0).transform2d(camMat).y())
    upside_down = 0;
  else
    upside_down = 1;

  // draw x-axis
  tick_direction = flap ? vec(0, 1, 0) : vec(0, 0, -1);
  if (!upside_down)
    {
      if (vec(0.5, 0, -1).transform2d(camMat).y() >= vec(0.5, 0, 0).transform2d(camMat).y())
        {
          start = vec(0, 0, 0);
        }
      else
        {
          start = vec(0, 0, -1);
          start_z += vec(0, 0, -1);
        }
    }
  else
    {
      if (vec(0.5, 0, -1).transform2d(camMat).y() >= vec(0.5, 0, 0).transform2d(camMat).y())
        {
          if (axis_at_bottom)
            start = vec(0, 0, -1);
          else
            start = vec(0, 1, 0);
        }
      else
        {
          if (axis_at_bottom)
            {
              start = vec(0, 0, 0);
              start_z += vec(0, 0, -1);
            }
          else
            {
              start = vec(0, 1, -1);
              start_z += vec(0, 0, -1);
            }
        }
    }
  test_pos = tick_direction / 2 + start;
  if (test_pos.y() == .5 || test_pos.z() == -.5) tick_direction *= -1;
  if (paintXAxis)
    {
      paintAxis(start, vec(1, 0, 0), tick_direction, x_tick, x_major_tick, tick_size, xmin, xmax, x_textures, flipx,
                logX);
      if (x_axis_label != "")
        {
          vec d1 = vec(1, 0, 0), d2 = tick_direction;
          provideCorrectDirections(d1, d2, d1, d2, camMat);
          vec pos = start + vec(0.5, 0, 0) + tick_direction * 0.2;
          label_texture.texture = load_texture(x_axis_label);
          label_texture.height = LABEL_HEIGHT;
          label_texture.width =
              label_texture.height / label_texture.texture.textHeight * label_texture.texture.textWidth;
          label_texture.xcut = (float)label_texture.texture.textWidth / label_texture.texture.width;
          label_texture.ycut = (float)label_texture.texture.textHeight / label_texture.texture.height;
          drawTexturedRect(pos, d1, d2, label_texture.width, label_texture.height, label_texture.xcut,
                           label_texture.ycut);
          glDeleteTextures(1, &label_texture.texture.texName);
        }
    }

  // draw y-axis
  tick_direction = flap ? vec(0, 1, 0) : vec(1, 0, 0);

  if (!upside_down)
    {
      if (vec(0, 0, -0.5).transform2d(camMat).y() > vec(1, 0, -0.5).transform2d(camMat).y())
        {
          start = vec(1, 0, 0);
        }
      else
        {
          start = vec(0, 0, 0);
          start_z += vec(1, 0, 0);
        }
    }
  else
    {
      if (vec(0, 0, -0.5).transform2d(camMat).y() >= vec(1, 0, -0.5).transform2d(camMat).y())
        {
          if (axis_at_bottom)
            start = vec(0, 0, 0);
          else
            start = vec(1, 1, 0);
        }
      else
        {
          if (axis_at_bottom)
            {
              start = vec(1, 0, 0);
              start_z += vec(1, 0, 0);
            }
          else
            {
              start = vec(0, 1, 0);
              start_z += vec(1, 0, 0);
            }
        }
    }

  test_pos = tick_direction / 2 + start;
  if (test_pos.x() == .5 || test_pos.y() == .5) tick_direction *= -1;

  if (paintYAxis)
    {
      paintAxis(start, vec(0, 0, -1), tick_direction, y_tick, y_major_tick, tick_size, ymin, ymax, y_textures, flipy,
                logY);
      if (y_axis_label != "")
        {
          vec d1 = vec(0, 0, -1), d2 = tick_direction;
          provideCorrectDirections(d1, d2, d1, d2, camMat);
          vec pos = start + vec(0, 0, -0.5) + tick_direction * 0.2;
          label_texture.texture = load_texture(y_axis_label);
          label_texture.height = LABEL_HEIGHT;
          label_texture.width =
              label_texture.height / label_texture.texture.textHeight * label_texture.texture.textWidth;
          label_texture.xcut = (float)label_texture.texture.textWidth / label_texture.texture.width;
          label_texture.ycut = (float)label_texture.texture.textHeight / label_texture.texture.height;
          drawTexturedRect(pos, d1, d2, label_texture.width, label_texture.height, label_texture.xcut,
                           label_texture.ycut);
          glDeleteTextures(1, &label_texture.texture.texName);
        }
    }
  // draw z-axis
  if (vec(1, 0, 0).transform2d(camMat).quadNorm() > vec(0, 0, 1).transform2d(camMat).quadNorm())
    tick_direction = vec(1, 0, 0);
  else
    tick_direction = vec(0, 0, 1);

  test_pos = tick_direction / 2 + start_z;
  if (test_pos.x() == .5 || test_pos.z() == -.5) tick_direction *= -1;

  if (paintZAxis)
    {
      paintAxis(start_z, vec(0, 1, 0), tick_direction, z_tick, z_major_tick, tick_size, zmin, zmax, z_textures, flipz,
                logZ);
      if (z_axis_label != "")
        {
          vec d2 = vec(0, 1, 0), d1 = tick_direction;
          provideCorrectDirections(d1, d2, d1, d2, camMat);
          vec pos = start_z + vec(0, 1 + LABEL_HEIGHT + .01, 0);
          label_texture.texture = load_texture(z_axis_label);
          label_texture.height = LABEL_HEIGHT;
          label_texture.width =
              label_texture.height / label_texture.texture.textHeight * label_texture.texture.textWidth;
          label_texture.xcut = (float)label_texture.texture.textWidth / label_texture.texture.width;
          label_texture.ycut = (float)label_texture.texture.textHeight / label_texture.texture.height;
          drawTexturedRect(pos, d1, d2, label_texture.width, label_texture.height, label_texture.xcut,
                           label_texture.ycut);
          glDeleteTextures(1, &label_texture.texture.texName);
        }
    }
}

void GLGrWidget::paintAxis(const vec &start, const vec &direction, const vec &tick_direction, float tick, int major,
                           float tick_size, float min, float max, const vector<TickTexture> &textures, bool flip,
                           bool is_log)
{
  float first_tick_value, tick_value;
  int major_tick = 0, tick_counter;
  vec r1, r2, t1, t2, tick_pos;

  float len;

  // draw axis
  glColor3f(0, 0, 0);
  glBegin(GL_LINES);
  glVertex3fv(start.getv());
  glVertex3fv((start + direction).getv());
  glEnd();

  // draw ticks
  if (!is_log)
    { // linear mode
      first_tick_value = (((int)(min / tick))) * tick;
      if (first_tick_value < min) first_tick_value += tick;
      len = max - min;
      tick_value = first_tick_value;
      if (tick > 0)
        {
          for (tick_counter = 0; tick_value <= max; tick_value += tick, tick_counter++)
            {
              // major tick or not
              if (major != 0 && tick_counter % major == 0)
                major_tick = 1;
              else
                major_tick = 0;

              if (!flip)
                tick_pos = start + (tick_value - min) / len * direction;
              else
                tick_pos = start + (1 - (tick_value - min) / len) * direction;

              glBegin(GL_LINES);
              glVertex3fv(tick_pos.getv());
              glVertex3fv((tick_pos + (major_tick ? 2 : 1) * tick_size * tick_direction).getv());
              glEnd();
            }
        }
      for (vector<TickTexture>::const_iterator it = textures.begin(); it != textures.end(); ++it)
        {
          if (!flip)
            tick_pos = start + (it->value - min) / len * direction;
          else
            tick_pos = start + (1 - (it->value - min) / len) * direction;
          glBegin(GL_LINES);
          glVertex3fv(tick_pos.getv());
          glVertex3fv((tick_pos + 2 * tick_size * tick_direction).getv());
          glEnd();
        }
    }
  else
    { // logarithmic mode
      len = log(max - min);
      for (int i = 1; i < max; i *= 10)
        for (int j = 1; j <= 10 && j * i < max; ++j)
          {
            tick_value = i * j;
            if (!flip)
              tick_pos = start + log(tick_value - min) / len * direction;
            else
              tick_pos = start + (1 - log(tick_value - min) / len) * direction;
            if (j == 10)
              major_tick = 1;
            else
              major_tick = 0;
            glBegin(GL_LINES);
            glVertex3fv(tick_pos.getv());
            glVertex3fv((tick_pos + (major_tick ? 2 : 1) * tick_size * tick_direction).getv());
            glEnd();
          }
      for (vector<TickTexture>::const_iterator it = textures.begin(); it != textures.end(); ++it)
        {
          if (!flip)
            tick_pos = start + log(it->value - min) / len * direction;
          else
            tick_pos = start + log((1 - (it->value - min)) / len) * direction;
          glBegin(GL_LINES);
          glVertex3fv(tick_pos.getv());
          glVertex3fv((tick_pos + 2 * tick_size * tick_direction).getv());
          glEnd();
        }
    }

  // estimate text-label directions
  r1 = tick_direction;
  if (direction.transform2d(camMat).quadNorm() >= .4)
    r2 = direction;
  else
    {
      if (vec(1, 0, 0) != direction.abs() && vec(1, 0, 0) != tick_direction.abs())
        r2 = vec(1, 0, 0);
      else if (vec(0, 0, 1) != direction.abs() && vec(0, 0, 1) != tick_direction.abs())
        r2 = vec(0, 0, 1);
      else
        r2 = vec(0, 1, 0);
    }

  if (fabs(r1.transform2d(camMat).x()) < fabs(r2.transform2d(camMat).x()))
    {
      vec temp = r1;
      r1 = r2;
      r2 = temp;
    }

  provideCorrectDirections(r1, r2, r1, r2, camMat);

  // check if textures intersect and raise distance if necessary
  vector<Rect> rect_vector;
  int step = 1;
  bool ok = false;
  float diff;
  vec position;

  for (vector<TickTexture>::const_iterator it = textures.begin(); it != textures.end(); ++it)
    {
      if (tick_direction == r1 || tick_direction == -r1)
        diff = it->width;
      else
        diff = it->height;
      if (!flip)
        {
          if (!is_log)
            position = start + (it->value - min) / len * direction;
          else
            position = start + log(it->value - min) / len * direction;
        }
      else
        {
          if (!is_log)
            position = start + (1 - (it->value - min) / len) * direction;
          else
            position = start + (1 - log(it->value - min) / len) * direction;
        }
      tick_pos = position + (2 * tick_size + .01 + diff / 2) * tick_direction;
      rect_vector.push_back(Rect(tick_pos, r1, r2, it->width, it->height, camMat));
    }

  while (!ok && step < (int)textures.size())
    {
      ok = true;
      for (vector<Rect>::iterator it = rect_vector.begin() + step; it != rect_vector.end(); ++it)
        if (it->intersects(*(it - step)))
          {
            step *= 2;
            ok = false;
            break;
          }
    }
  // draw Textures
  if (step <= (int)textures.size())
    for (int i = 0; i < (int)rect_vector.size(); i += step)
      {
        glBindTexture(GL_TEXTURE_2D, textures[i].texture.texName);
        drawTexturedRect(rect_vector[i].pos(), r1, r2, textures[i].width, textures[i].height, textures[i].xcut,
                         textures[i].ycut);
      }
}

void GLGrWidget::paintTitle()
{
  if (title != "")
    {
      glPushMatrix();
      glLoadIdentity();
      TickTexture title_texture;
      title_texture.texture = load_texture(title);
      // title_texture.height = (float) title_texture.texture.height/this->height();
      title_texture.height = title_offset;
      title_texture.width = title_texture.height / title_texture.texture.textHeight * title_texture.texture.textWidth;
      title_texture.xcut = (float)title_texture.texture.textWidth / title_texture.texture.width;
      title_texture.ycut = (float)title_texture.texture.textHeight / title_texture.texture.height;
      vec pos(.5, .5 + sqrt(3 * .25) + title_offset / 2 + (subtitle != "" ? subtitle_offset : 0) + .08, 100);
      drawTexturedRect(pos, vec(1, 0, 0), vec(0, 1, 0), title_texture.width, title_texture.height, title_texture.xcut,
                       title_texture.ycut);
      glDeleteTextures(1, &title_texture.texture.texName);
      glPopMatrix();
    }
}

void GLGrWidget::paintSubTitle()
{
  if (subtitle != "")
    {
      glPushMatrix();
      glLoadIdentity();
      TickTexture title_texture;
      title_texture.texture = load_texture(subtitle);
      // title_texture.height = (float) title_texture.texture.height/this->height();
      title_texture.height = subtitle_offset;
      title_texture.width = title_texture.height / title_texture.texture.textHeight * title_texture.texture.textWidth;
      title_texture.xcut = (float)title_texture.texture.textWidth / title_texture.texture.width;
      title_texture.ycut = (float)title_texture.texture.textHeight / title_texture.texture.height;
      vec pos(.5, .5 + sqrt(3 * .25) + subtitle_offset / 2 + .08, 100);
      drawTexturedRect(pos, vec(1, 0, 0), vec(0, 1, 0), title_texture.width, title_texture.height, title_texture.xcut,
                       title_texture.ycut);
      glPopMatrix();
    }
}

void GLGrWidget::paintArrow()
{
  float bog, PI = 3.1415927f;

  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(.1f, 0, 0);
  glEnd();

  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(.15f, 0, 0);
  for (int i = 0; i <= 360; i += 36)
    {
      bog = 2 * PI / 360 * i;
      glVertex3f(.1f, (float)(.015 * sin(bog)), (float)(.015 * cos(bog)));
    }
  glEnd();
}

void GLGrWidget::paintOrientation()
{
  glLoadIdentity();
  glTranslatef(-.4f, .6f - proj_rad, 0);
  glRotatef(tilt, 1, 0, 0);
  glRotatef(-rotation, 0, 1, 0);

  glColor3f(1, 0, 0);
  paintArrow();
  glRotatef(90, 0, 0, 1);
  glColor3f(0, 1, 0);
  paintArrow();
  glRotatef(90, 0, 1, 0);
  glColor3f(0, 0, 1);
  paintArrow();

  Texture v;
  float xcut, ycut, width, height;
  vec pos;

  glPushMatrix();
  glLoadIdentity();
  v = load_texture("x", Qt::red);
  height = .06f;
  width = height / v.textHeight * v.textWidth;
  xcut = (float)v.textWidth / v.width;
  ycut = (float)v.textHeight / v.height;
  pos = vec(-.45f, .75f - proj_rad + height / 2, 100);
  drawTexturedRect(pos, vec(1, 0, 0), vec(0, 1, 0), width, height, xcut, ycut);
  glDeleteTextures(1, &v.texName);
  pos.setX(pos.x() + width * 2);

  v = load_texture("y", Qt::blue);
  width = height / v.textHeight * v.textWidth;
  xcut = (float)v.textWidth / v.width;
  ycut = (float)v.textHeight / v.height;
  drawTexturedRect(pos, vec(1, 0, 0), vec(0, 1, 0), width, height, xcut, ycut);
  glDeleteTextures(1, &v.texName);
  pos.setX(pos.x() + width * 2);

  v = load_texture("z", Qt::green);
  width = height / v.textHeight * v.textWidth;
  xcut = (float)v.textWidth / v.width;
  ycut = (float)v.textHeight / v.height;
  drawTexturedRect(pos, vec(1, 0, 0), vec(0, 1, 0), width, height, xcut, ycut);
  glDeleteTextures(1, &v.texName);

  glPopMatrix();
}

void GLGrWidget::paintLegend()
{
  QColor a;
  float xcut, ycut, width, height;
  vec pos;

  glPushMatrix();
  glLoadIdentity();
  glBegin(GL_QUAD_STRIP);
  for (int i = 0; i < 72; ++i)
    {
      a = colorTable[i];
      glColor3f(a.red() / 255.0, a.green() / 255.0, a.blue() / 255.0);
      glVertex3f(-.5f, i / 71.0f, 100);
      glVertex3f(-.4f, i / 71.0f, 100);
    }
  glEnd();

  Texture value;
  value = load_texture(QString("%1").arg(useCCdata ? cmin : zmin, 0, 'g'));
  height = .06f;
  width = height / value.textHeight * value.textWidth;
  xcut = (float)value.textWidth / value.width;
  ycut = (float)value.textHeight / value.height;
  pos = vec(-.45f, -height / 2, 100);
  drawTexturedRect(pos, vec(1, 0, 0), vec(0, 1, 0), width, height, xcut, ycut);
  glDeleteTextures(1, &value.texName);

  value = load_texture(QString("%1").arg(useCCdata ? cmax : zmax, 0, 'g'));
  width = height / value.textHeight * value.textWidth;
  xcut = (float)value.textWidth / value.width;
  ycut = (float)value.textHeight / value.height;
  pos = vec(-.45f, 1.0f + height / 2, 100);
  drawTexturedRect(pos, vec(1, 0, 0), vec(0, 1, 0), width, height, xcut, ycut);
  glDeleteTextures(1, &value.texName);

  glPopMatrix();
}

void GLGrWidget::drawTexturedRect(const vec &pos, vec xdir, vec ydir, float width, float height, float xcut, float ycut)
{
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glVertex3fv((pos - xdir * width / 2 - ydir * height / 2).getv());
  glTexCoord2f(0.0, ycut);
  glVertex3fv((pos - xdir * width / 2 + ydir * height / 2).getv());
  glTexCoord2f(xcut, ycut);
  glVertex3fv((pos + xdir * width / 2 + ydir * height / 2).getv());
  glTexCoord2f(xcut, 0.0);
  glVertex3fv((pos + xdir * width / 2 - ydir * height / 2).getv());
  glEnd();
  glDisable(GL_TEXTURE_2D);
}

// help methods
void GLGrWidget::provideCorrectDirections(const vec &r1, const vec &r2, vec &nr1, vec &nr2, GLfloat camMat[4][4])
{
  vec t1, t2;
  nr1 = r1;
  nr2 = r2;
  t1 = nr1.transform2d(camMat);
  t2 = nr2.transform2d(camMat);
  if (t2.y() < 0)
    {
      nr2 *= -1;
      t2 = nr2.transform2d(camMat);
    }
  if (t1.x() * t2.y() - t1.y() * t2.x() < 0) nr1 *= -1;
}

Texture GLGrWidget::load_texture(QString str, const QColor &color)
{
  Texture tex;
  glGenTextures(1, &(tex.texName));
  glBindTexture(GL_TEXTURE_2D, tex.texName);

  GLubyte *texData;

  makeImage(str, &texData, &tex, color);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
  delete[] texData;
  return tex;
}

void GLGrWidget::makeImage(QString str, GLubyte **texData, Texture *tex, const QColor &color)
{
  QFont font("Helvetica", 36, QFont::Normal);
  font.setStyleStrategy(QFont::PreferAntialias);

  QFontMetrics fm(font);

  tex->textWidth = fm.width(str);          // textbreite
  tex->textHeight = fm.height();           // texthoehe
  tex->width = make_pot(tex->textWidth);   // pixmapbreite
  tex->height = make_pot(tex->textHeight); // pixmaphoehe

  QPixmap pix(tex->width, tex->height);
  pix.fill(Qt::white);

  QPainter p(&pix);

  p.setPen(color);
  p.setFont(font);
  p.drawText(0, tex->height - tex->textHeight, tex->textWidth, tex->textHeight, Qt::AlignLeft | Qt::AlignVCenter, str);
  p.end();

  QImage *texImage = new QImage();
  *texImage = QGLWidget::convertToGLFormat(pix.toImage());

  *texData = new GLubyte[texImage->width() * texImage->height() * 4];
  for (int i = 0; i < texImage->height() * texImage->width(); ++i)
    {
      (*texData)[i * 4] = (GLubyte)texImage->bits()[i * 4];
      (*texData)[i * 4 + 1] = (GLubyte)texImage->bits()[i * 4 + 1];
      (*texData)[i * 4 + 2] = (GLubyte)texImage->bits()[i * 4 + 2];
      if ((int)(*texData)[i * 4] == 255 && (int)(*texData)[i * 4 + 1] == 255 && (int)(*texData)[i * 4 + 2] == 255)
        {
#ifdef WIN32
          (*texData)[i * 4 + 3] = (GLubyte)255;
#else
          (*texData)[i * 4 + 3] = (GLubyte)0;
#endif
        }
      else
        (*texData)[i * 4 + 3] = (GLubyte)255;
    }
  delete texImage;
}

int GLGrWidget::make_pot(int x)
{
  unsigned short y = x, s = USHRT_MAX / 2 + 1;
  int r = -1;
  if (x > USHRT_MAX / 2) cerr << "too big texture format\n";
  for (int i = 0; i < 16; ++i)
    {
      if (s & y)
        {
          r = s * 2;
          break;
        }
      else
        s /= 2;
    }
  return r;
}

void GLGrWidget::calc_autoscale()
{
  int i;
  zmin = cmin = FLT_MAX;
  zmax = cmax = FLT_MIN;
  for (i = 0; i < xdim * ydim; ++i)
    {
      if (data3D[i] < zmin) zmin = data3D[i];
      if (data3D[i] > zmax) zmax = data3D[i];
      if (dataCC)
        {
          if (dataCC[i] < cmin) cmin = dataCC[i];
          if (dataCC[i] > cmax) cmax = dataCC[i];
        }
    }
  refreshZTextures = true;
}

float GLGrWidget::data(int i, int j)
{
  if (flipx) i = xdim - 1 - i;
  if (flipy) j = ydim - 1 - j;
  return data3D[j * xdim + i];
}

float GLGrWidget::logXValue(float x)
{
  float value;
  if (!logX)
    value = (float)x / (xdim - 1);
  else
    {
      if (x < 1) x = 1;
      if (!flipx)
        {
          if (x == 0) x = 1;
          value = log(x) / log((double)(xdim - 1));
        }
      else
        {
          x = xdim - 1 - x;
          if (x == 0) x = 1;
          value = 1 - (log(x) / log((double)(xdim - 1)));
        }
    }
  return value;
}

float GLGrWidget::logYValue(float y)
{
  float value;
  if (!logY)
    value = (float)-y / (ydim - 1);
  else
    {
      if (y < 1) y = 1;
      if (!flipy)
        {
          if (y == 0) y = 1;
          value = -log(y) / log((double)(ydim - 1));
        }
      else
        {
          y = ydim - 1 - y;
          if (y == 0) y = 1;
          value = (log(y) / log((double)(ydim - 1))) - 1;
        }
    }
  return value;
}

float GLGrWidget::colorData(int i, int j)
{
  if (flipx) i = xdim - 1 - i;
  if (flipy) j = ydim - 1 - j;
  return colorData_[j * xdim + i];
}

void GLGrWidget::print()
{
  int pix_number;
  vector<TickTexture>::iterator it;
  QPrinter *printer = new QPrinter();
  printer->setResolution(PRINT_DPI);
  printer->setColorMode(QPrinter::Color);
  QPrintDialog printdialog(printer);
  if (printdialog.exec() == QDialog::Accepted)
    {
      // delete old textures, because a new GLcontext is created for renderPixmap()
      // so the textures must be created new and sent to new GLContext
      for (it = x_textures.begin(); it != x_textures.end(); ++it) glDeleteTextures(1, &(it->texture.texName));
      for (it = y_textures.begin(); it != y_textures.end(); ++it) glDeleteTextures(1, &(it->texture.texName));
      for (it = z_textures.begin(); it != z_textures.end(); ++it) glDeleteTextures(1, &(it->texture.texName));
      x_textures.clear();
      y_textures.clear();
      z_textures.clear();

      QPainter *painter = new QPainter(printer);
      // L.Sch.
      //    pix_number=QPaintDeviceMetrics(printer).width();
      //    if (QPaintDeviceMetrics(printer).height() < pix_number)
      //     pix_number=QPaintDeviceMetrics(printer).height();
      pix_number = printer->width();
      if (printer->height() < pix_number) pix_number = printer->height();
      //   L.Sch.
      //    painter->drawPixmap((QPaintDeviceMetrics(printer).width()-pix_number)/2,(QPaintDeviceMetrics(printer).height()-pix_number)/2,
      //			renderPixmap(pix_number,pix_number));
      painter->drawPixmap((printer->width() - pix_number) / 2, (printer->height() - pix_number) / 2,
                          renderPixmap(pix_number, pix_number));
      painter->end();
      // The textures for the GLcontext of renderPixmap() are automatically erased, when the context is destroyed.
      // i have to clear the texture_lists in order to prevent eventually erasing wrong textures of the GLWidget
      // Context, when new textures are constructed and the old ones in the texture_lists are erased.
      x_textures.clear();
      y_textures.clear();
      z_textures.clear();
    }
}

void rebin(float *a1, int m, int n, float *a2, int a, int b)
{
  int i, j;
  float x, y, ip1, ip2;
  float v1, v2;

  for (i = 0; i < a; ++i)
    for (j = 0; j < b; ++j)
      {
        // calculate pixelposition in source-array
        x = (float)i / (a - 1) * (m - 1);
        y = (float)j / (b - 1) * (n - 1);
        // interpolate  first direction
        if (x < m - 1)
          {
            v1 = a1[(int)y * m + (int)x];
            v2 = a1[(int)y * m + (int)x + 1];
            ip1 = v1 + (v2 - v1) * (x - (int)x);
          }
        else
          ip1 = a1[(int)y * m + (int)x];

        if (y < n - 1)
          {
            if (x < m - 1)
              {
                v1 = a1[(int)(y + 1) * m + (int)x];
                v2 = a1[(int)(y + 1) * m + (int)x + 1];
                ip2 = v1 + (v2 - v1) * (x - (int)x);
              }
            else
              ip2 = a1[(int)(y + 1) * m + (int)x];
          }
        else
          ip2 = ip1;

        // second direction
        a2[a * j + i] = (ip1 + (ip2 - ip1) * (y - (int)y));
      }
}


void GLGrWidget::get_plot3d_orders(QString &data_string)
{
  QString attribute;

  bool error;

  float float_value;
  int int_value;

  data_string = data_string.simplified();

  QDomNodeList gr_rootChilds;
  int n_gr_rootChilds;
  QString attribute_val;


  QDomDocument domDoc = QDomDocument(QString("grDoc"));
  domDoc.setContent(data_string);
  QDomElement gr_root = domDoc.documentElement();

  if (gr_root.tagName() != "gr")
    {
      QMessageBox::information(NULL, QObject::tr("DOM gr"), QObject::tr("The file is not an XML gr file."));
      return;
    }
  else if (gr_root.hasAttribute("version") && gr_root.attribute("version") != "1.0")
    {
      QMessageBox::information(NULL, QObject::tr("DOM gr"),
                               QObject::tr("The file is not an XML version 1.0 "
                                           "file."));
      return;
    }

  bool hasChilds = gr_root.hasChildNodes();
  if (hasChilds)
    {
      gr_rootChilds = gr_root.childNodes();
      n_gr_rootChilds = gr_rootChilds.count();
    }
  else
    {
      QMessageBox::information(NULL, QObject::tr("DOM gr"), QObject::tr("The XML file has no child nodes."));
      return;
    }

  //============================================================================================//

  for (int i = 0; i < n_gr_rootChilds; i++)
    {

      error = false;
      QString node_name = gr_rootChilds.item(i).nodeName();
      QDomNamedNodeMap namedNodeMap = gr_rootChilds.item(i).attributes();
      int n_attr = namedNodeMap.count();

      if (node_name == QString("axes3d"))
        {

          for (int j = 0; j < n_attr; j++)
            {
              attribute = namedNodeMap.item(j).nodeName();
              attribute_val = namedNodeMap.item(j).nodeValue();
              if (attribute == QString("xtick"))
                {

                  float_value = attribute_val.toFloat();
                  setXTick(float_value);
                }
              else if (attribute == QString("ytick"))
                {

                  float_value = attribute_val.toFloat();
                  setYTick(float_value);
                }
              else if (attribute == QString("ztick"))
                {

                  float_value = attribute_val.toFloat();
                  setZTick(float_value);
                }
              else if (attribute == QString("ticksize"))
                {

                  float_value = attribute_val.toFloat();
                  setTickSize(float_value);
                }
              else if (attribute == QString("majorx"))
                {

                  int_value = attribute_val.toInt();
                  setMajorX(int_value);
                }
              else if (attribute == QString("majory"))
                {

                  int_value = attribute_val.toInt();
                  setMajorY(int_value);
                }
              else if (attribute == QString("majorz"))
                {

                  int_value = attribute_val.toInt();
                  setMajorZ(int_value);
                }
            }
        }
      else if (node_name == QString("setcolormap"))
        {

          for (int j = 0; j < n_attr; j++)
            {
              attribute = namedNodeMap.item(j).nodeName();
              attribute_val = namedNodeMap.item(j).nodeValue();
              if (attribute == QString("index"))
                {
                  int_value = attribute_val.toInt();
                  if (int_value > 12 || int_value < 0) int_value = 0;
                  setColormap(int_value);
                }
            }
        }
      else if (node_name == QString("contour"))
        {

          int nh;
          setPaintContourLines(1);
          if (namedNodeMap.contains("nh"))
            {
              nh = namedNodeMap.namedItem("nh").nodeValue().toInt();
              setContourLineNumber(nh);
            }
        }
      else if (node_name == QString("fillarea"))
        {

          for (int j = 0; j < n_attr; j++)
            {
              attribute = namedNodeMap.item(j).nodeName();
              attribute_val = namedNodeMap.item(j).nodeValue();
            }
        }
      else if (node_name == QString("setscale"))
        {

          for (int j = 0; j < n_attr; j++)
            {
              attribute = namedNodeMap.item(j).nodeName();
              attribute_val = namedNodeMap.item(j).nodeValue();
              if (attribute == QString("scale"))
                {
                  int_value = attribute_val.toInt();

                  if (int_value & LOGX)
                    {
                      setLogX(true);
                    }
                  else
                    {
                      setLogX(false);
                    }
                  if (int_value & LOGY)
                    {
                      setLogY(true);
                    }
                  else
                    {
                      setLogY(false);
                    }
                  if (int_value & LOGZ)
                    {
                      setLogZ(true);
                    }
                  else
                    {
                      setLogZ(false);
                    }
                  if (int_value & FLIPX)
                    {
                      setFlipX(true);
                    }
                  else
                    {
                      setFlipX(false);
                    }
                  if (int_value & FLIPY)
                    {
                      setFlipY(true);
                    }
                  else
                    {
                      setFlipY(false);
                    }
                  if (int_value & FLIPZ)
                    {
                      setFlipZ(true);
                    }
                  else
                    {
                      setFlipZ(false);
                    }
                }
            }
        }
      else if (node_name == QString("settextalign"))
        {

          for (int j = 0; j < n_attr; j++)
            {
              attribute = namedNodeMap.item(j).nodeName();
              attribute_val = namedNodeMap.item(j).nodeValue();
            }
        }
      else if (node_name == QString("setwindow"))
        {

          for (int j = 0; j < n_attr; j++)
            {
              attribute = namedNodeMap.item(j).nodeName();
              attribute_val = namedNodeMap.item(j).nodeValue();
              if (attribute == "xmin")
                {

                  float_value = attribute_val.toFloat();
                  setXMin(float_value);
                }
              else if (attribute == QString("xmax"))
                {

                  float_value = attribute_val.toFloat();
                  setXMax(float_value);
                }
              else if (attribute == QString("ymin"))
                {

                  float_value = attribute_val.toFloat();
                  setYMin(float_value);
                }
              else if (attribute == QString("ymax"))
                {

                  float_value = attribute_val.toFloat();
                  setYMax(float_value);
                }
            }
        }
      else if (node_name == QString("setspace"))
        {

          for (int j = 0; j < n_attr; j++)
            {
              attribute = namedNodeMap.item(j).nodeName();
              attribute_val = namedNodeMap.item(j).nodeValue();
              if (attribute == QString("zmin"))
                {

                  float_value = attribute_val.toFloat();
                  setZMin(float_value);
                }
              else if (attribute == QString("zmax"))
                {

                  float_value = attribute_val.toFloat();
                  setZMax(float_value);
                }
              else if (attribute == QString("rotation"))
                {

                  int_value = attribute_val.toInt();
                  setRotation(int_value);
                }
              else if (attribute == QString("tilt"))
                {

                  int_value = attribute_val.toInt();
                  setTilt(int_value);
                }
            }
        }
      else if (node_name == QString("surface"))
        {

          int nx, ny, option;
          float *d;

          for (int j = 0; j < n_attr; j++)
            {
              attribute = namedNodeMap.item(j).nodeName();
              attribute_val = namedNodeMap.item(j).nodeValue();
            }

          if (namedNodeMap.contains("nx"))
            {
              nx = namedNodeMap.namedItem("nx").nodeValue().toInt();
            }
          else
            {
              QMessageBox::information(NULL, QObject::tr("DOM gr"), QObject::tr("no attribute <nx>."));
              return;
            }

          if (namedNodeMap.contains("ny"))
            {
              ny = namedNodeMap.namedItem("ny").nodeValue().toInt();
            }
          else
            {
              QMessageBox::information(NULL, QObject::tr("DOM gr"), QObject::tr("no attribute <ny>."));
              return;
            }

          if (namedNodeMap.contains("option"))
            {
              option = namedNodeMap.namedItem("option").nodeValue().toInt();

              if (option >= 1) setPaintSurface(1);
              if (option >= 2) setPaintContour(1);
              if (option >= 3) setPaintContourLines(1);
            }
          else
            {
              QMessageBox::information(NULL, QObject::tr("DOM gr"), QObject::tr("no attribute <option>."));
              return;
            }

          if (namedNodeMap.contains("z"))
            {
              if ((nx * ny) <= 0)
                {
                  QMessageBox::warning(NULL, QObject::tr("DOM gr"),
                                       QObject::tr("nx *ny <= 0 --> cannot allocate memory for z-values"));
                  return;
                }

              d = new float[nx * ny];
              int k1, k2;
              bool ok = true;
              QString d_str = namedNodeMap.namedItem("z").nodeValue().simplified();
              k1 = 0;
              int np = 0;
              for (int i = 0; i < nx * ny; i++)
                {
                  k2 = d_str.indexOf(' ', k1);
                  *(d + i) = d_str.mid(k1, k2 - k1).toFloat(&ok);
                  if (!ok)
                    {
                      std::cerr << "error in data3d: not enough values" << std::endl;
                      error = true;
                      break;
                    }
                  else
                    {
                      np++;
                      if (k2 > 0) k1 = k2 + 1;
                    }
                }

              if (error)
                {
                  continue;
                }
              else
                {
                  setData(nx, ny, d, NULL);
                }
            }
          else
            {
              QMessageBox::information(NULL, QObject::tr("DOM gr"), QObject::tr("no attribute <z>."));
              return;
            }
        }
      else if (node_name == QString("titles3d"))
        {

          for (int j = 0; j < n_attr; j++)
            {
              attribute = namedNodeMap.item(j).nodeName();
              attribute_val = namedNodeMap.item(j).nodeValue();
              if (attribute == QString("xtitle"))
                {
                  setXLabel(attribute_val);
                }
              else if (attribute == QString("ytitle"))
                {
                  setYLabel(attribute_val);
                }
              else if (attribute == QString("ztitle"))
                {
                  setZLabel(attribute_val);
                }
            }
        }
      else if (node_name == QString(""))
        {
          ;
        }
    }
  update();
}
