#ifndef _GKSWIDGET_H_
#define _GKSWIDGET_H_

#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QWidget>
#else
#include <QtGui/QWidget>
#endif
#include <QtCore/QMutex>

class GKSWidget : public QWidget
{
  Q_OBJECT

public:
  GKSWidget(QWidget *parent = 0);
  virtual ~GKSWidget();

  static void inqdspsize(double *mwidth, double *mheight, int *width, int *height);
  static const QSize &frame_decoration_size();

public slots:
  void interpret(char *dl);

protected:
  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *event);
  void set_window_size_from_dl();

private:
  bool is_mapped;
  bool resize_requested_by_application;
  char *dl;
  static QSize frame_decoration_size_;
};

#endif
