#ifndef _GKSSERVER_H_
#define _GKSSERVER_H_

#include <QTcpServer>
#include <QTcpSocket>
#include <qstring.h>

#include "gkswidget.h"

class GKSServer : public QTcpServer
{
  Q_OBJECT

public:
  GKSServer(QObject *parent = 0);

public slots:
  void readClient();
  void killSocket();
  void connectSocket();
  void disconnectSocket();
  void newWidget();

private:
  QTcpSocket *socket;
  char *dl, *ba;
  int nbyte, dl_size, ba_size;
  GKSWidget *widget;

signals:
  void data(char *);
};

#endif
