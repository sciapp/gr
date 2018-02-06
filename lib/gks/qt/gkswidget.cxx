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
#include <QtGui/QImage>

#endif

#define QT_NAME_STRING "Qt5"
#define QT_PLUGIN_ENTRY_NAME gksterm

#include "qtplugin_impl.cxx"

#include "gkswidget.h"

static
void create_pixmap(ws_state_list *p)
{
  p->pm = new QPixmap(p->width, p->height);
  p->pm->fill(Qt::white);

  p->pixmap = new QPainter(p->pm);
  p->pixmap->setClipRect(0, 0, p->width, p->height);

  get_pixmap();
}

static
void resize_pixmap(int width, int height)
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

GKSWidget::GKSWidget(QWidget *parent)
  : QWidget(parent)
{
  dl = NULL;
  is_mapped = 0;

  GKSServer *server = new GKSServer();
  connect(server, SIGNAL(data(char *)), this, SLOT(interpret(char *)));

  p->width = 500; p->height = 500;
  p->dpiX = p->dpiY = 100;

  initialize_data();

  setWindowTitle(tr("GKS QtTerm"));
}

void GKSWidget::paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  if (dl)
    {
      p->pm->fill(Qt::white);
      interp(dl);
      painter.drawPixmap(0, 0, *(p->pm));
    }
}

void GKSWidget::resizeEvent(QResizeEvent *event)
{
  resize_pixmap(this->size().width(), this->size().height());
  repaint();
}

static
void set_window_size(char *s)
{
  int sp = 0, *len, *f;
  double *vp;

  len = (int *) (s + sp);
  while (*len)
    {
      f = (int *) (s + sp + sizeof(int));
      if (*f == 55)
        {
          vp = (double *) (s + sp + 3 * sizeof(int));
          p->width  = nint((vp[1] - vp[0]) / 2.54 * p->dpiX * 100);
          p->height = nint((vp[3] - vp[2]) / 2.54 * p->dpiY * 100);
        }
      sp += *len;
      len = (int *) (s + sp);
    }
}

void GKSWidget::interpret(char *dl)
{
  set_window_size(dl);

  resize(p->width, p->height);
  if (!is_mapped)
    {
      is_mapped = 1;
      create_pixmap(p);
      show();
    }

  this->dl = dl;
  repaint();
}
