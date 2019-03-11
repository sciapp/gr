#include <QPainter>
#include <QStatusBar>
#include <QMainWindow>
#include <QDebug>

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>

#include "gr.h"

#define COMPILING_DLL
#include "grwidget.h"

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#define DC_to_NDC(xd, yd, xn, yn) \
  xn = ((xd - transx) - b) / a;   \
  yn = ((yd - transy) - d) / c;

static double a, b, c, d;
static double cur_xl, cur_xr, cur_yb, cur_yt;
static double org_xl, org_xr, org_yb, org_yt;
static double vp_width, vp_height;
static int transx, transy;

static clock_t start;
static bool leftButton;


GRWidget::GRWidget(QWidget *parent) : QWidget(parent)
{
  init_gks();
}

void GRWidget::init_gks()
{
#ifdef _WIN32
  putenv("GKS_WSTYPE=381");
  putenv("GKS_DOUBLE_BUF=True");
#else
  setenv("GKS_WSTYPE", "381", 1);
  setenv("GKS_DOUBLE_BUF", "True", 1);
#endif
}

void GRWidget::paintEvent(QPaintEvent *event)
{
  QPainter painter;
  char buf[100];

#ifdef _WIN32
  sprintf(buf, "GKS_CONID=%p!%p", this, &painter);
  putenv(buf);
#else
  sprintf(buf, "%p!%p", this, &painter);
  setenv("GKS_CONID", buf, 1);
#endif

  painter.begin(this);

  clear_background(painter);
  gr_clearws();
  draw();
  gr_updatews();

  painter.end();
}

void GRWidget::clear_background(QPainter &painter)
{
  painter.fillRect(0, 0, width(), height(), QColor("white"));
}


InteractiveGRWidget::InteractiveGRWidget(QWidget *parent) : GRWidget(parent)
{
  rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
  drag_start = QPoint();
  zoom_rect = QRect();
  cur_xl = 0.0;
  cur_xr = 1.0;
  cur_yb = 0.0;
  cur_yt = 1.0;
  gr_inqwindow(&org_xl, &org_xr, &org_yb, &org_yt);
  setFocusPolicy(Qt::StrongFocus);
}

void InteractiveGRWidget::set_xform(void)
{
  a = (vp_width / 2.54 * physicalDpiX() * 100 - 1) / (cur_xr - cur_xl);
  b = -cur_xl * a;
  c = (vp_height / 2.54 * physicalDpiY() * 100 - 1) / (cur_yb - cur_yt);
  d = vp_height / 2.54 * physicalDpiY() * 100 - 1 - cur_yb * c;
}

void InteractiveGRWidget::paintEvent(QPaintEvent *event)
{
  QPainter painter;
  char buf[100];
  double mwidth, mheight;

#ifdef _WIN32
  sprintf(buf, "GKS_CONID=%p!%p", this, &painter);
  putenv(buf);
#else
  sprintf(buf, "%p!%p", this, &painter);
  setenv("GKS_CONID", buf, 1);
#endif

  painter.begin(this);

  clear_background(painter);
  gr_clearws();
  /* wsviewport is set here because the resize event is called before GR
     is initialized */
  mwidth = (double)width() / physicalDpiX() * 2.54 / 100;
  mheight = (double)height() / physicalDpiY() * 2.54 / 100;
  if (mwidth > mheight)
    {
      vp_width = mheight;
      vp_height = mheight;
      gr_setwsviewport((mwidth - mheight) / 2, mwidth - (mwidth - mheight) / 2, 0, mheight);
      transx = (mwidth - mheight) / 2 / 2.54 * physicalDpiX() * 100;
      transy = 0;
    }
  else
    {
      vp_width = mwidth;
      vp_height = mwidth;
      gr_setwsviewport(0, mwidth, (mheight - mwidth) / 2, mheight - (mheight - mwidth) / 2);
      transx = 0;
      transy = (mheight - mwidth) / 2 / 2.54 * physicalDpiY() * 100;
    }
  painter.translate(transx, transy);
  set_xform();
  draw();
  gr_updatews();

  painter.end();
}

void InteractiveGRWidget::mouseMoveEvent(QMouseEvent *event)
{
  double ex, ey, x, y;
  ex = event->pos().x();
  ey = event->pos().y();

  DC_to_NDC(ex, ey, x, y);
  gr_ndctowc(&x, &y);

  qobject_cast<QMainWindow *>(parent())->statusBar()->showMessage(QString(tr("(%1, %2)")).arg(x).arg(y));
  if (event->buttons() == Qt::LeftButton)
    {
      rubberBand->setGeometry(QRect(this->drag_start, event->pos()).normalized());
      update();
    }
}

void InteractiveGRWidget::mousePressEvent(QMouseEvent *event)
{
  if (event->buttons() == Qt::LeftButton)
    {
      start = clock();
      leftButton = true;
      this->drag_start = event->pos();
      rubberBand->setGeometry(QRect(this->drag_start, QSize()));
      rubberBand->show();
    }
  else
    {
      leftButton = false;
    }
}

void InteractiveGRWidget::mouseReleaseEvent(QMouseEvent *event)
{
  double duration;
  double xl, xr, yb, yt, width, height;

  duration = (clock() - start) / (double)CLOCKS_PER_SEC;
  if (leftButton && duration > 0.1)
    {
      zoom_rect = QRect(0, 0, this->width(), this->height()).intersected(rubberBand->geometry());

      if (!(zoom_rect.isNull()))
        {
          rubberBand->hide();
        }
      DC_to_NDC(zoom_rect.left(), zoom_rect.top(), xl, yt);
      gr_ndctowc(&xl, &yt);
      DC_to_NDC(zoom_rect.right(), zoom_rect.bottom(), xr, yb);
      gr_ndctowc(&xr, &yb);
      width = xr - xl;
      height = yt - yb;
      if (width > height)
        {
          yt = yb + width;
        }
      else
        {
          xr = xl + height;
        }
      gr_setwindow(xl, xr, yb, yt);
    }
  this->repaint();
}

void InteractiveGRWidget::wheelEvent(QWheelEvent *event)
{
  double fac, xmin, xmax, ymin, ymax;
  double x, y;

  fac = 1;
  gr_inqwindow(&xmin, &xmax, &ymin, &ymax);

#if QT_VERSION >= 0x050000
  QPoint numDegrees = event->angleDelta() / 8;

  if (!numDegrees.isNull())
    {
      if (numDegrees.y() < 0)
        fac = pow(1.01, -numDegrees.y());
      else
        fac = pow(1 / 1.01, numDegrees.y());
    }
#else
  int numDegrees = event->delta() / 8;

  if (event->orientation() == Qt::Vertical)
    {
      if (numDegrees < 0)
        fac = pow(1.01, -numDegrees);
      else
        fac = pow(1 / 1.01, numDegrees);
    }
#endif
  DC_to_NDC(event->x(), event->y(), x, y);
  gr_ndctowc(&x, &y);
  gr_setwindow(x - (x - xmin) * fac, x + (xmax - x) * fac, y - (y - ymin) * fac, y + (ymax - y) * fac);
  this->repaint();
}

void InteractiveGRWidget::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape)
    {
      gr_setwindow(org_xl, org_xr, org_yb, org_yt);
      repaint();
    }
}
