#ifndef GRPLOT_WIDGET_H_INCLUDED
#define GRPLOT_WIDGET_H_INCLUDED

#include <QMenu>
#include <QMenuBar>
#include <QRubberBand>
#include <QTextDocument>
#include <QWidget>
#include <QMainWindow>

#include "gredit/Bounding_object.h"
#include "gredit/Bounding_logic.h"
#include "gredit/TreeWidget.h"

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
  void leaveEvent(QEvent *event) override;

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
  void show_container_slot();
  void show_bounding_boxes_slot();
  void save_file_slot();
  void open_file_slot();
  void enable_editor_functions();

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
  Bounding_logic *bounding_logic;
  std::vector<Bounding_object> clicked;
  Bounding_object *current_selection, *mouse_move_selection;
  bool highlightBoundingObjects;
  TreeWidget *treewidget;
  int amount_scrolled;
  bool enable_editor;

  QMenuBar *menu;
  QMenu *type, *algo, *export_menu, *editor_menu;
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
  QAction *show_container_action, *show_bounding_boxes_action, *save_file_action, *open_file_action, *editor_action;

  void reset_pixmap();
  void moveEvent(QMoveEvent *event) override;
  void highlight_current_selection(QPainter &painter);
  void extract_bounding_boxes_from_grm(QPainter &painter);
};

#endif /* ifndef GRPLOT_WIDGET_H_INCLUDED */
