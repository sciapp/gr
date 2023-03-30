#ifndef GRPLOT_WIDGET_H_INCLUDED
#define GRPLOT_WIDGET_H_INCLUDED

#include <QMenu>
#include <QMenuBar>
#include <QRubberBand>
#include <QTextDocument>
#include <QWidget>
#include <QMainWindow>

#include "qtterm/receiver_thread.h"
#include "qtterm/grm_args_t_wrapper.h"
#include <grm.h>

class GRPlotWidget : public QWidget
{
  Q_OBJECT

public:
  explicit GRPlotWidget(QMainWindow *parent, int argc, char **argv);
  ~GRPlotWidget() override;

protected:
  virtual void draw();
  void redraw();
  void keyPressEvent(QKeyEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
  void heatmap();
  void marginalheatmapall();
  void marginalheatmapline();
  void line();
  void sumalgorithm();
  void maxalgorithm();
  void volume();
  void isosurface();
  void surface();
  void wireframe();
  void contour();
  void imshow();
  void plot3();
  void contourf();
  void trisurf();
  void tricont();
  void scatter3();
  void scatter();
  void hist();
  void barplot();
  void stairs();
  void stem();
  void shade();
  void hexbin();
  void pdf();
  void png();
  void jpeg();
  void svg();
  void received(grm_args_t_wrapper args);
  void screenChanged();

private:
  struct MouseState
  {
    enum class Mode
    {
      normal,
      pan,
      boxzoom
    };
    Mode mode;
    QPoint pressed;
    QPoint anchor;
  };
  QPixmap *pixmap;
  grm_args_t *args_;
  MouseState mouseState;
  QRubberBand *rubberBand;
  grm_tooltip_info_t *tooltip;
  QTextDocument label;
  Receiver_Thread *receiver_thread;

  QMenuBar *menu;
  QMenu *type;
  QMenu *algo;
  QMenu *export_menu;
  QAction *heatmapAct;
  QAction *marginalheatmapAllAct;
  QAction *marginalheatmapLineAct;
  QAction *lineAct;
  QAction *sumAct;
  QAction *maxAct;
  QAction *volumeAct;
  QAction *isosurfaceAct;
  QAction *surfaceAct;
  QAction *wireframeAct;
  QAction *contourAct;
  QAction *imshowAct;
  QAction *plot3Act;
  QAction *contourfAct;
  QAction *trisurfAct;
  QAction *tricontAct;
  QAction *scatter3Act;
  QAction *scatterAct;
  QAction *histAct;
  QAction *barplotAct;
  QAction *stairsAct;
  QAction *stemAct;
  QAction *shadeAct;
  QAction *hexbinAct;
  QAction *PdfAct;
  QAction *PngAct;
  QAction *JpegAct;
  QAction *SvgAct;

  void reset_pixmap();
  void showEvent(QShowEvent *) override;
  void closeEvent(QCloseEvent *event) override;
  void size_callback(const grm_event_t *);
};

#endif /* ifndef GRPLOT_WIDGET_H_INCLUDED */
