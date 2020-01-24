#if defined(NO_QT5)
#define NO_QT
#endif

#ifndef NO_QT

#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QWidget>
#else
#include <QtGui/QWidget>
#endif
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QImage>
#include <QIcon>
#include <QProcessEnvironment>

#endif

#define QT_NAME_STRING "Qt5"
#define QT_PLUGIN_ENTRY_NAME gksqt

#include "qtplugin_impl.cxx"

#include "gkswidget.h"

static void create_pixmap(ws_state_list *p)
{
  p->pm = new QPixmap(p->width, p->height);
  p->pm->fill(Qt::white);

  p->pixmap = new QPainter(p->pm);
  p->pixmap->setClipRect(0, 0, p->width, p->height);

  get_pixmap();
}

static void resize_pixmap(int width, int height)
{
  if (p->width != width || p->height != height)
    {
      p->width = width;
      p->height = height;

      if (p->pm)
        {
          delete p->pixmap;
          delete p->pm;

          p->pm = new QPixmap(p->width, p->height);
          p->pm->fill(Qt::white);

          p->pixmap = new QPainter(p->pm);
          p->pixmap->setClipRect(0, 0, p->width, p->height);
        }
      p->has_been_resized = 1;
    }
}

GKSWidget::GKSWidget(QWidget *parent) : QWidget(parent)
{
  is_mapped = 0;
  dl = NULL;

  gkss->fontfile = gks_open_font();

  p->device_dpi_x = this->physicalDpiX();
  p->device_dpi_y = this->physicalDpiY();
  p->width = 500;
  p->height = 500;
  p->mwidth = (double)p->width / p->device_dpi_x * 0.0254;
  p->mheight = (double)p->height / p->device_dpi_y * 0.0254;

  initialize_data();

  setMinimumSize(2, 2);
  setWindowTitle(tr("GKS QtTerm"));
  setWindowIcon(QIcon(":/images/gksqt.png"));

  prevent_resize = !QProcessEnvironment::systemEnvironment().value("GKS_GKSQT_PREVENT_RESIZE").isEmpty();
}

void GKSWidget::paintEvent(QPaintEvent *)
{

  if (dl)
    {
      QPainter painter(this);
      p->pm->fill(Qt::white);
      interp(dl);

      if (!prevent_resize)
        {
          painter.drawPixmap(0, 0, *(p->pm));
        }
      else
        {
          int x = (width() - p->width) / 2;
          int y = (height() - p->height) / 2;
          painter.fillRect(0, 0, width(), height(), Qt::white);
          painter.drawPixmap(x, y, *(p->pm));
        }
    }
}

void GKSWidget::resizeEvent(QResizeEvent *event)
{
  (void)event;
  p->mwidth = (double)this->width() / p->device_dpi_x * 0.0254;
  p->mheight = (double)this->height() / p->device_dpi_y * 0.0254;
  resize_pixmap(this->width(), this->height());
  repaint();
}

static void set_window_size(char *s)
{
  int sp = 0, *len, *f;
  double *vp;
  len = (int *)(s + sp);
  while (*len)
    {
      f = (int *)(s + sp + sizeof(int));
      if (*f == 55)
        {
          vp = (double *)(s + sp + 3 * sizeof(int));
          p->mwidth = vp[1] - vp[0];
          p->width = nint(p->device_dpi_x * p->mwidth / 0.0254);
          if (p->width < 2)
            {
              p->width = 2;
              p->mwidth = (double)p->width / p->device_dpi_x * 0.0254;
            }

          p->mheight = vp[3] - vp[2];
          p->height = nint(p->device_dpi_y * p->mheight / 0.0254);
          if (p->height < 2)
            {
              p->height = 2;
              p->mheight = (double)p->height / p->device_dpi_y * 0.0254;
            }
        }
      sp += *len;
      len = (int *)(s + sp);
    }
}

void GKSWidget::interpret(char *dl)
{
  set_window_size(dl);
  if (!prevent_resize)
    {
      resize(p->width, p->height);
    }
  if (!is_mapped)
    {
      is_mapped = 1;
      create_pixmap(p);
      show();
    }

  this->dl = dl;
  repaint();
}
