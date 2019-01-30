#ifndef _GRWIDGET_H_
#define _GRWIDGET_H_

#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QRubberBand>
#include <QPoint>

#ifdef _WIN32
#ifdef COMPILING_DLL
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif
#else
#define DLL
#endif


// ##################### Base GRWidget #########################################

class DLL GRWidget : public QWidget
{
public:
  GRWidget(QWidget *parent = 0);

protected:
  void paintEvent(QPaintEvent *event);
  virtual void clear_background(QPainter &painter);
  virtual void draw() = 0;

private:
  void init_gks();
};


// ##################### Interactive GRWidget ##################################

class DLL InteractiveGRWidget : public GRWidget
{
public:
  InteractiveGRWidget(QWidget *parent = 0);

protected:
  void paintEvent(QPaintEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void wheelEvent(QWheelEvent *event);
  void keyPressEvent(QKeyEvent *event);

private:
  void set_xform(void);
  QRubberBand *rubberBand;
  QPoint drag_start;
  QRect zoom_rect;
};

#undef DLL
#endif /* ifndef GRWIDGET_H_INCLUDED */
