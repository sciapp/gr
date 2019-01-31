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

public slots:
  void interpret(char *dl);

protected:
  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *event);

private:
  int is_mapped;
  bool prevent_resize;
  char *dl;
};

#endif
