#ifndef MAIN_WINDOW_H_INCLUDED
#define MAIN_WINDOW_H_INCLUDED

#include "gr.h"
#include "grm.h"
#include "Bounding_object.h"
#include "Bounding_logic.h"
#include "TreeWidget.h"
#include <QtGlobal>
#include <QApplication>
#include <QWindow>
#include <QToolTip>
#include <QPainter>
#include <QTimer>
#include <QEvent>
#include <QDebug>
#include <sstream>
#include <QRubberBand>
#include <QMouseEvent>
#include <functional>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDockWidget>
#include <QMenuBar>


class MainWindow : public QWidget
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

  ~MainWindow() override;

protected:
  void draw();


  static void init_env_vars();

  void paintEvent(QPaintEvent *event) override;

  void keyPressEvent(QKeyEvent *event) override;

  void mousePressEvent(QMouseEvent *event) override;

  void mouseMoveEvent(QMouseEvent *event) override;

  void leaveEvent(QEvent *event) override;

  void wheelEvent(QWheelEvent *event) override;

  void mouseDoubleClickEvent(QMouseEvent *event) override;

  QPixmap *pixmap;

public slots:

  void show_container_slot();

  void show_bounding_boxes_slot();

  void save_file_slot();

  void open_file_slot();
  void print_slot();

private:
  grm_args_t *to_draw, *input_args;
  Bounding_logic *bounding_logic;
  std::vector<Bounding_object> clicked;
  Bounding_object *current_selection, *mouse_move_selection;
  bool highlightBoundingObjects;
  TreeWidget *treewidget;

  void init_quad_plot_data();

  void init_simple_plot_data();

  void reset_pixmap();

  void resizeEvent(QResizeEvent *event) override;

  void moveEvent(QMoveEvent *event) override;

  void highlight_current_selection(QPainter &painter);

  void extract_bounding_boxes_from_grm(QPainter &painter);

  int amount_scrolled;

  QMenuBar *menubar;
  QMenu *configuration_menu, *file_menu;
  QAction *show_container_action, *show_bounding_boxes_action, *save_file_action, *open_file_action, *print_action;
};

#endif /* ifndef MAIN_WINDOW_H_INCLUDED */
