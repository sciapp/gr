#ifndef MAIN_WINDOW_H_INCLUDED
#define MAIN_WINDOW_H_INCLUDED

#include "gr.h"
#include "grm.h"
#include "receiver_thread.h"
#include "tooltip.h"
#include "grm_args_t_wrapper.h"
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

class MainWindow : public QWidget
{
  Q_OBJECT
public slots:
  void received(grm_args_t_wrapper args);

  void screenChanged();

  void reset_pixmap();

  void mouse_down_to_boxzoom_slot();

public:
  explicit MainWindow(QWidget *parent = nullptr);

  ~MainWindow() override;

protected:
  void mouseMoveEvent(QMouseEvent *) override;

  void mousePressEvent(QMouseEvent *) override;

  void mouseReleaseEvent(QMouseEvent *) override;

  void mouseDoubleClickEvent(QMouseEvent *e) override;

  void wheelEvent(QWheelEvent *) override;

  void keyPressEvent(QKeyEvent *event) override;

  void resizeEvent(QResizeEvent *event) override;

  void drawRubberband(bool, QPoint *, QPoint *, QPoint *);

  void draw();

  static void init_env_vars();

  void paintEvent(QPaintEvent *event) override;

  QPixmap *pixmap;
  QRubberBand *rubberBand;
  QPoint mouse_press_point;
  QPoint mouse_release_point;
  QTimer *update_timer, *mouse_down_to_boxzoom_timer;
  bool mouse_down_to_boxzoom_activate;

  bool event(QEvent *event) override
  {
    if (event->type() == QEvent::NativeGesture)
      {
        return nativeGestureHandler(dynamic_cast<QNativeGestureEvent *>(event));
      }
    return QWidget::event(event);
  }


private:
  Receiver_Thread *receiver_thread;
  grm_args_t *to_draw, *input_args;
  Tooltip *tooltip;

  void showEvent(QShowEvent *) override;

  bool nativeGestureHandler(QNativeGestureEvent *pEvent);

  void zoomNativeGestureEvent(QNativeGestureEvent *pEvent);

  void smartZoomNativeGestureEvent(QNativeGestureEvent *pEvent);

  void size_callback(const grm_event_t *);

  void closeEvent(QCloseEvent *event) override;

  void reset_plot_at_position(int cursor_pos_x, int cursor_pos_y);
};

#endif /* ifndef MAIN_WINDOW_H_INCLUDED */
