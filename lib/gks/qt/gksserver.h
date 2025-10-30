#ifndef _GKSSERVER_H_
#define _GKSSERVER_H_

#include <list>
#include <QTcpServer>
#include <QTcpSocket>
#include <qstring.h>

#include "gkswidget.h"


struct SocketFunction
{
  enum Enum
  {
    unknown = 0,
    create_window = 1,
    draw = 2,
    is_alive = 3,
    close_window = 4,
    is_running = 5,
    inq_ws_state = 6,
    sample_locator = 7
  };
};


class GKSServer;


class GKSConnection : public QObject
{
  Q_OBJECT

public:
  GKSConnection(QTcpSocket *socket, GKSServer &server);
  virtual ~GKSConnection();
  void newWidget();

public slots:
  void readClient();
  void destroyedWidget();
  void disconnectedSocket();
  void updateWindowTitle(QString renderer = "");

signals:
  void data(char *);
  void close(GKSConnection &connection);

private:
  static unsigned int index;
  GKSServer &server;
  unsigned int widget_index;
  static const int window_shift;
  QTcpSocket *socket;
  GKSWidget *widget;
  char *dl;
  unsigned int dl_size;
  SocketFunction::Enum socket_function;
};


class GKSServer : public QTcpServer
{
  Q_OBJECT

public:
  GKSServer(QObject *parent = 0);
  virtual ~GKSServer();
  bool has_multiple_connections();

public slots:
  void connectSocket();
  void closeConnection(GKSConnection &connection);

private:
  static unsigned int port;
  std::list<const GKSConnection *> connections;
};

#endif
