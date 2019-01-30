#ifndef GLGRWIDGET_H_INCLUDED
#define GLGRWIDGET_H_INCLUDED

#include <QGLWidget>
#include <QDomDocument>
#include <QMessageBox>
#include <qpoint.h>

#include <vector>

#include "vec.h"
#include "rect.h"

#define PRINT_DPI 150
using std::vector;
using namespace std;

static const int ct_lower = 0;
static const int ct_upper = 1;
static const int ct_upway = 2;
static const int ct_downway = 4;
static const int ct_leftway = 8;
static const int ct_rightway = 16;

static const float LABEL_HEIGHT = .06f;

const int LOGX = 0x1;
const int LOGY = 0x2;
const int LOGZ = 0x4;
const int FLIPX = 0x8;
const int FLIPY = 0x10;
const int FLIPZ = 0x20;

struct Texture
{
  GLuint texName;
  int width, height;
  int textWidth, textHeight;
};

struct TickTexture
{
  Texture texture;
  float value;
  float height, width, xcut, ycut;
};

struct CustomizedLabel
{
  float value;
  QString text;
};

class GLGrWidget : public QGLWidget
{
  Q_OBJECT

signals:
  void rotationChanged(int);
  void tiltChanged(int);

public:
  //  GLGrWidget(QWidget *parent=NULL, const char *name=NULL);
  GLGrWidget(QWidget *parent = NULL);

protected:
  virtual void mousePressEvent(QMouseEvent *);
  virtual void mouseMoveEvent(QMouseEvent *);
  virtual void initializeGL();
  virtual void resizeGL(int w, int h);
  virtual void paintGL();

private:
  void setBoundaries(float xmin_, float xmax_, float ymin_, float ymax_, float zmin_, float zmax_, float cmin_ = -1,
                     float cmax_ = -1);

private slots:
  void get_plot3d_orders(QString &);

public slots:
  void setData(int xdim_, int ydim_, float *data_, float *dataCC_ = NULL);
  // set visual parameter
  void setFlipX(bool b);
  void setFlipY(bool b);
  void setFlipZ(bool b);
  void setRotation(int value);
  void setTilt(int value);

  void setPaintSurface(bool b);
  void setPaintContour(bool b);
  void setPaintContourLines(bool b);
  void setContourLineNumber(int value);
  void setPaintOrientation(bool b);
  void setPaintLegend(bool b);
  void setColormap(int c);
  void setXYType(int v);
  void setLogX(bool b);
  void setLogY(bool b);
  void setLogZ(bool b);
  void setUseCCdata(bool b);

  // set Axes parameter
  void setPaintXAxis(bool b);
  void setPaintYAxis(bool b);
  void setPaintZAxis(bool b);
  void setPaintAxes(bool b);
  void setXTick(float tick);
  void setYTick(float tick);
  void setZTick(float tick);
  void setMajorX(int major);
  void setMajorY(int major);
  void setMajorZ(int major);
  void setTickSize(float value);
  void setXMin(float value);
  void setXMin(int value) { setXMin((float)value); }
  void setXMax(float value);
  void setXMax(int value) { setXMax((float)value); }
  void setYMin(float value);
  void setYMin(int value) { setYMin((float)value); }
  void setYMax(float value);
  void setYMax(int value) { setYMax((float)value); }
  void setZMin(float value);
  void setZMin(int value) { setZMin((float)value); }
  void setZMax(float value);
  void setZMax(int value) { setZMax((float)value); }
  void setCMin(float value);
  void setCMin(int value) { setCMin((float)value); }
  void setCMax(float value);
  void setCMax(int value) { setCMax((float)value); }
  void setAutoScale(bool b);
  void setReductionRate(int rate);
  void addCustomizedLabel(int axis, float value, const QString &text);
  void resetCustomizedLabel(int axis);
  void setPaintCustomizedLabel(int axis, bool b);
  void setXTick(int value) { setXTick((float)value); }
  void setYTick(int value) { setYTick((float)value); }
  void setZTick(int value) { setZTick((float)value); }

  void setXLabel(const QString t);
  void setYLabel(const QString t);
  void setZLabel(const QString t);
  void setGridColor(int nr);


  void print();

private:
  const QGLContext *last_context;
  const float title_offset, subtitle_offset, legend_offset, proj_rad;

  // painting - parameter
  bool paint_surface, paint_contour, paint_contour_lines, paint_axes, paint_arrows, paint_legend;
  bool autoscale;
  bool flipx, flipy, flipz;
  int xytype, reduction_rate;
  int gridColor;
  bool logX, logY, logZ;
  bool useCCdata;

  // title - parameter
  QString title, subtitle, x_axis_label, y_axis_label, z_axis_label;

  // axes - parameter
  float x_tick, y_tick, z_tick;
  float tick_size;
  float xmin, xmax, ymin, ymax, zmin, zmax, cmin, cmax;
  int x_major_tick, y_major_tick, z_major_tick;
  bool paintXAxis, paintYAxis, paintZAxis;
  bool paintCustomizedXLabel, paintCustomizedYLabel, paintCustomizedZLabel;
  GLuint x_labelName, y_labelName, z_labelName;

  vector<TickTexture> x_textures, y_textures, z_textures;
  vector<CustomizedLabel> x_indiv_label, y_indiv_label, z_indiv_label;
  bool refreshXTextures, refreshYTextures, refreshZTextures;

  // camera - parameter
  int rotation, tilt;
  float upper_offset, left_offset;

  // data and boundaries
  float *data3D, *dataCC, *colorData_, *source_data3D, *source_dataCC;
  int xdim, ydim, source_xdim, source_ydim;

  // GL vectors
  void createSurfaceVertices();
  void createSurfaceIndices();
  void createSurfaceColors();
  void createGridIndices();
  void createColorTable(int ct);
  void createContourVertices();
  void createContourIndices();
  void createContourLineVertices();
  void createTextureArray(float min, float max, float tick, int major, vector<TickTexture> &textures,
                          vector<CustomizedLabel> *indiv, bool is_log);
  void reduceData(bool reduce3D = true);
  // flags
  bool refreshSurfaceVertices, refreshSurfaceIndices, refreshSurfaceColors, refreshGridIndices, refreshContourVertices,
      refreshContourIndices, refreshContourLineVertices;

  int surfaceIndicesDim, gridIndicesDim, contourIndicesDim;
  GLfloat *surfaceVertices;
  GLuint **surfaceIndices;
  GLfloat *surfaceColors;
  GLuint **gridIndices;
  QColor colorTable[72];
  GLfloat *contourVertices;
  GLuint **contourIndices;
  int **ct, ctDim, contourLineNumber;
  vector<vec> contourLineVertices;
  GLfloat camMat[4][4];

  // paint functions
  void paintSurface();
  void paintGrid();
  void paintContour();
  void paintContourLines();
  void paintAxes();
  void paintAxis(const vec &start, const vec &direction, const vec &tick_direction, float tick, int major,
                 float tick_size, float min, float max, const vector<TickTexture> &textures, bool flip, bool is_log);
  void paintTitle();
  void paintSubTitle();
  void paintArrow();
  void paintOrientation();
  void paintLegend();

  void drawTexturedRect(const vec &pos, vec xdir, vec ydir, float width, float height, float xcut, float ycut);


  // help methods
  void provideCorrectDirections(const vec &r1, const vec &r2, vec &nr1, vec &nr2, GLfloat camMat[4][4]);
  void find_way(int x, int y, int direction, float value);
  Texture load_texture(QString str, const QColor &color = Qt::black);
  void makeImage(QString str, GLubyte **texData, Texture *tex, const QColor &color);
  int make_pot(int x);
  void calc_autoscale();
  float data(int i, int j);
  float logXValue(float x);
  float logYValue(float y);
  float colorData(int i, int j);
  //  size_t getNextTag(const QString &txt,QString &attribute, int pos);


  // internal variables
  QPoint lastMousePos;
};

int round_f_to_i(float x);
void rebin(float *a1, int m, int n, float *a2, int a, int b);

#endif
