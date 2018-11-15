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
#include <QIcon>
#include <QProcessEnvironment>

#endif

#define QT_NAME_STRING "Qt5"
#define QT_PLUGIN_ENTRY_NAME gksqt

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
  is_mapped = 0;
  dl = NULL;

  gkss->fontfile = gks_open_font();

  p->width = this->width();
  p->height = this->height();
  p->mwidth = this->widthMM() * 0.001;
  p->mheight = this->heightMM() * 0.001;

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

      if (!prevent_resize) {
        painter.drawPixmap(0, 0, *(p->pm));
      } else {
        int x = (width() - p->width) / 2;
        int y = (height() - p->height) / 2;
        painter.fillRect(0, 0, width(), height(), Qt::white);
        painter.drawPixmap(x, y, *(p->pm));
      }
    }
}

void GKSWidget::resizeEvent(QResizeEvent *event)
{
  p->mwidth = this->widthMM() * 0.001;
  p->mheight = this->heightMM() * 0.001;
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
          if (p->mwidth > 0)
            {
              double mwidth = round((vp[1] - vp[0]) * 1000) * 0.001;
              p->width = nint(mwidth / p->mwidth * p->width);
            }
          else
            {
              p->width = 2;
            }

          if (p->mheight > 0)
            {
              double mheight = round((vp[3] - vp[2]) * 1000) * 0.001;
              p->height = nint(mheight / p->mheight * p->height);
            }
          else
            {
              p->height = 2;
            }
        }
      sp += *len;
      len = (int *) (s + sp);
    }
}

void GKSWidget::interpret(char *dl)
{
  set_window_size(dl);
  if (!prevent_resize) {
    resize(p->width, p->height);
  } else {
    p->mwidth = p->width * widthMM() * 0.001 / width();
    p->mheight = p->height * heightMM() * 0.001 / height();
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
