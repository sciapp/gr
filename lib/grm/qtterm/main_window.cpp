#include "main_window.h"

std::function<void(const grm_event_t *)> callback;
extern "C" void wrapper(const grm_event_t *cb)
{
  callback(cb);
}

void MainWindow::drawRubberband(bool keep_aspect_ratio, QPoint *pressed, QPoint *current, QPoint *global_pos)
{
  int x_diff = current->x() - pressed->x();
  int y_diff = current->y() - pressed->y();
  int box[4];
  if (grm_is3d(current->x(), current->y()) || (current->x() < 0 || current->y() > this->height() || current->y() < 0 ||
                                               current->x() > this->width() || (x_diff == 0 && y_diff == 0)))
    {
      mouse_release_point = QPoint(-1, -1);
      rubberBand->hide();
      QToolTip::hideText();
      return;
    }
  mouse_release_point = QPoint(pressed->x() + x_diff, pressed->y() + y_diff);
  if (grm_get_box(pressed->x(), pressed->y(), mouse_release_point.x(), mouse_release_point.y(), keep_aspect_ratio,
                  &box[0], &box[1], &box[2], &box[3]))
    {
      if ((mouse_press_point.x() - current->x()) * (mouse_press_point.y() - current->y()) <= 0)
        {
          QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
        }
      else
        {
          QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
        }
      rubberBand->setGeometry(QRect(box[0], box[1], box[2], box[3]));
      QToolTip::showText(*global_pos, QString("%1,%2").arg(rubberBand->size().width()).arg(rubberBand->size().height()),
                         this);
      rubberBand->show();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
  if (!to_draw)
    {
      return;
    }
  if (event->button() == Qt::LeftButton)
    {
      mouse_press_point = event->pos();
      mouse_release_point = QPoint(-1, -1);
      QApplication::setOverrideCursor(Qt::OpenHandCursor);
      mouse_down_to_boxzoom_timer->start(1000);
    }
  else if (event->button() == Qt::RightButton)
    {
      rubberBand->hide();
      mouse_press_point = event->pos();
      mouse_release_point = QPoint(-1, -1);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
  QApplication::restoreOverrideCursor();
  if (!to_draw)
    {
      return;
    }
  if (event->button() == Qt::LeftButton)
    {
      if (mouse_release_point.x() != -1 && mouse_release_point.y() != -1)
        { // Prevent click
        }
    }
  if (mouse_down_to_boxzoom_activate || event->button() == Qt::RightButton)
    {
      if (mouse_release_point.x() != -1 && mouse_release_point.y() != -1)
        { // Prevent click
          grm_args_push(input_args, "x1", "i", mouse_press_point.x());
          grm_args_push(input_args, "x2", "i", mouse_release_point.x());
          grm_args_push(input_args, "y1", "i", mouse_press_point.y());
          grm_args_push(input_args, "y2", "i", mouse_release_point.y());
          grm_args_push(input_args, "keep_aspect_ratio", "i", event->modifiers() & Qt::ShiftModifier);
          grm_input(input_args);

          tooltip->hide();
          reset_pixmap();
          rubberBand->hide();
        }
    }
  mouse_down_to_boxzoom_timer->stop();
  mouse_down_to_boxzoom_activate = false;
  QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *e)
{
  if (e->button() == Qt::LeftButton)
    {
      tooltip->hide();
      QPoint widget_cursor_pos = mapFromGlobal(QCursor::pos());
      reset_plot_at_position(widget_cursor_pos.x(), widget_cursor_pos.y());
    }
}

void MainWindow::reset_plot_at_position(int cursor_pos_x, int cursor_pos_y)
{
  grm_args_push(input_args, "x", "i", cursor_pos_x);
  grm_args_push(input_args, "y", "i", cursor_pos_y);
  grm_args_push(input_args, "key", "s", "r");
  grm_input(input_args);
  reset_pixmap();
}

void MainWindow::reset_pixmap()
{
  pixmap = nullptr;
  tooltip->hide();
  update();
}

void MainWindow::mouse_down_to_boxzoom_slot()
{
  mouse_down_to_boxzoom_activate = true;
  QApplication::setOverrideCursor(Qt::SizeAllCursor);
  mouse_down_to_boxzoom_timer->stop();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
  if (!to_draw)
    {
      return;
    }
  if (event->type() == QEvent::KeyPress)
    {
      QPoint widget_cursor_pos = mapFromGlobal(QCursor::pos());
      auto *key = event;
      switch (key->key())
        {
        case Qt::Key_R:
          reset_plot_at_position(widget_cursor_pos.x(), widget_cursor_pos.y());
          break;
        case Qt::Key_Z:
          grm_clear();
          to_draw = nullptr;
          tooltip->hide();
          reset_pixmap();
          break;
        default:
          // Do not do anything if key is unknown
          break;
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
  if (to_draw && event->oldSize() != event->size())
    {
      grm_args_t *size_container = grm_args_new();
      grm_args_push(size_container, "size", "dd", (double)event->size().width(), (double)event->size().height());
      grm_merge_hold(size_container);
    }
  pixmap = nullptr;
}

void MainWindow::size_callback(const grm_event_t *new_size_object)
{
  // TODO: Get Plot ID
  if (this->size() != QSize(new_size_object->size_event.width, new_size_object->size_event.height))
    {
      this->resize(new_size_object->size_event.width, new_size_object->size_event.height);
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
  if (!to_draw)
    {
      return;
    }
  if (!mouse_down_to_boxzoom_activate && event->buttons() == Qt::LeftButton)
    {
      /* panning */
      int x_diff = event->pos().x() - mouse_press_point.x();
      int y_diff = event->pos().y() - mouse_press_point.y();
      grm_args_push(input_args, "x", "i", mouse_press_point.x());
      grm_args_push(input_args, "y", "i", mouse_press_point.y());
      grm_args_push(input_args, "xshift", "i", x_diff);
      grm_args_push(input_args, "yshift", "i", y_diff);
      if (event->modifiers() & Qt::ShiftModifier)
        {
          grm_args_push(input_args, "shift_pressed", "i", 1);
        }
      if (event->modifiers() & Qt::CTRL)
        {
          grm_args_push(input_args, "ctrl_pressed", "i", 1);
        }
      if (event->modifiers() & Qt::ALT)
        {
          grm_args_push(input_args, "alt_pressed", "i", 1);
        }
      grm_input(input_args);
      mouse_press_point = event->pos();
      reset_pixmap();
      tooltip->updateData(grm_get_tooltip(event->pos().x(), event->pos().y()));
      QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    }
  else if (mouse_down_to_boxzoom_activate || event->buttons() == Qt::RightButton)
    {
      /* drag rubberband */
      if (!tooltip->isHidden())
        {
          tooltip->hide();
        }
      auto event_pos = QPoint(event->pos());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
      QPoint event_global_pos = QPoint(event->globalPosition().toPoint());
#else
      auto event_global_pos = QPoint(event->globalPos());
#endif

      if (event->modifiers() & Qt::ShiftModifier)
        {
          drawRubberband(true, &mouse_press_point, &event_pos, &event_global_pos);
        }
      else
        {
          drawRubberband(false, &mouse_press_point, &event_pos, &event_global_pos);
        }
    }
  else
    {
      tooltip->updateData(grm_get_tooltip(event->pos().x(), event->pos().y()));
    }
  mouse_down_to_boxzoom_timer->stop();
}


void MainWindow::wheelEvent(QWheelEvent *event)
{
  if (!to_draw)
    {
      return;
    }
  bool changed = false;
  QPoint numPixels = event->pixelDelta();
  QPoint numDegrees = event->angleDelta();
  int x, y;
#if QT_VERSION <= QT_VERSION_CHECK(5, 14, 2)
  x = event->pos().x();
  y = event->pos().y();
#else
  x = (int)event->position().x();
  y = (int)event->position().y();
#endif
  if (!numPixels.isNull() && numPixels.y() != 0)
    {
      // Scrolling with pixels (For high-res scrolling like on macOS)
      grm_args_push(input_args, "x", "i", x);
      grm_args_push(input_args, "y", "i", y);
      changed = true;
      grm_args_push(input_args, "angle_delta", "d", (double)numPixels.y() * 2);
    }
  else if (!numDegrees.isNull())
    {
      QPoint numSteps = numDegrees / 16;
      // Scrolling with degrees
      if (numSteps.y() != 0)
        {
          grm_args_push(input_args, "x", "i", x);
          grm_args_push(input_args, "y", "i", y);
          changed = true;
          grm_args_push(input_args, "angle_delta", "d", (double)numSteps.y() * 30);
        }
    }
  if (changed)
    {
      grm_input(input_args);
      update_timer->start();
      tooltip->hide();
    }
}

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
  init_env_vars();
  setMouseTracking(true);
  qRegisterMetaType<grm_args_t_wrapper>("grm_args_t_wrapper");

  mouse_down_to_boxzoom_activate = false;
  mouse_down_to_boxzoom_timer = new QTimer(this);
  mouse_down_to_boxzoom_timer->setSingleShot(true);
  connect(mouse_down_to_boxzoom_timer, SIGNAL(timeout()), this, SLOT(mouse_down_to_boxzoom_slot()));

  update_timer = new QTimer(this);
  update_timer->setSingleShot(true);
  connect(update_timer, SIGNAL(timeout()), this, SLOT(reset_pixmap()));

  input_args = grm_args_new();
  to_draw = nullptr;
  pixmap = nullptr;
  rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
  receiver_thread = new Receiver_Thread();
  QObject::connect(receiver_thread, SIGNAL(resultReady(grm_args_t_wrapper)), this, SLOT(received(grm_args_t_wrapper)));
  receiver_thread->start();
  // window resize callback
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  callback = [this](auto &&PH1) { size_callback(std::forward<decltype(PH1)>(PH1)); };
#else
  callback = std::bind(&MainWindow::size_callback, this, std::placeholders::_1);
#endif
  tooltip = new Tooltip(this);
  grm_register(GRM_EVENT_SIZE, wrapper);
  grm_args_t_wrapper configuration;
  configuration.set_wrapper(grm_args_new());
  grm_args_push(configuration.get_wrapper(), "hold_plots", "i", 0);
  grm_merge(configuration.get_wrapper());
  grm_args_delete(configuration.get_wrapper());
}

void MainWindow::init_env_vars()
{
  qputenv("GKS_WSTYPE", "381");
  qputenv("GKS_DOUBLE_BUF", "True");
}

void MainWindow::screenChanged()
{
  gr_configurews();
  reset_pixmap();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  event->ignore();
  hide();
  if (to_draw)
    {
      grm_args_delete(to_draw);
      to_draw = nullptr;
    }
}

void MainWindow::showEvent(QShowEvent *)
{
  QObject::connect(window()->windowHandle(), SIGNAL(screenChanged(QScreen *)), this, SLOT(screenChanged()));
}

void MainWindow::received(grm_args_t_wrapper args)
{
  if (!isVisible())
    {
      show();
    }
  if (to_draw)
    {
      grm_args_delete(to_draw);
    }
  grm_switch(1);
  to_draw = args.get_wrapper();
  grm_merge(to_draw);
  reset_pixmap();
}


void MainWindow::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event)
  QPainter painter;
  if (!pixmap)
    {
      pixmap = new QPixmap((int)(geometry().width() * this->devicePixelRatioF()),
                           (int)(geometry().height() * this->devicePixelRatioF()));
      std::stringstream addresses;
      pixmap->setDevicePixelRatio(this->devicePixelRatioF());
      addresses << static_cast<void *>(this) << "!" << static_cast<void *>(&painter);
      qputenv("GKS_CONID", addresses.str().c_str());
      painter.begin(pixmap);

      painter.fillRect(0, 0, width(), height(), QColor("white"));
      draw();

      painter.end();
    }
  if (pixmap)
    {
      painter.begin(this);
      painter.drawPixmap(0, 0, *pixmap);
      painter.end();
    }
}

void MainWindow::draw()
{
  if (to_draw)
    {
      int ret_code = grm_plot(nullptr);
      grm_args_clear(input_args);
    }
}

MainWindow::~MainWindow()
{
  delete tooltip;
  if (to_draw)
    {
      grm_args_delete(to_draw);
    }
  grm_args_delete(input_args);
  grm_finalize();
}

bool MainWindow::nativeGestureHandler(QNativeGestureEvent *pEvent)
{
  if (pEvent->gestureType() == Qt::ZoomNativeGesture)
    {
      zoomNativeGestureEvent(pEvent);
      return true;
    }
  else if (pEvent->gestureType() == Qt::SmartZoomNativeGesture)
    {
      smartZoomNativeGestureEvent(pEvent);
      return true;
    }
  return false;
}

void MainWindow::zoomNativeGestureEvent(QNativeGestureEvent *pEvent)
{
  if (!to_draw)
    {
      return;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  grm_args_push(input_args, "x", "i", pEvent->position().toPoint().x());
#else
  grm_args_push(input_args, "x", "i", pEvent->pos().x());
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  grm_args_push(input_args, "y", "i", pEvent->position().toPoint().y());
#else
  grm_args_push(input_args, "y", "i", pEvent->pos().y());
#endif
  grm_args_push(input_args, "factor", "d", 1 - (double)pEvent->value());

  grm_input(input_args);
  update();
}

void MainWindow::smartZoomNativeGestureEvent(QNativeGestureEvent *pEvent)
{
  if (!to_draw)
    {
      return;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  grm_args_push(input_args, "x", "i", pEvent->position().toPoint().x());
#else
  grm_args_push(input_args, "x", "i", pEvent->pos().x());
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  grm_args_push(input_args, "y", "i", pEvent->position().toPoint().y());
#else
  grm_args_push(input_args, "y", "i", pEvent->pos().y());
#endif
  grm_args_push(input_args, "factor", "d", 0.5);

  grm_input(input_args);
  update();
}
