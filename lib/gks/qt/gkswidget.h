
#include <QtWidgets/QWidget>
#include <QtCore/QMutex>

#include "gksserver.h"

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
  char *dl;
};

