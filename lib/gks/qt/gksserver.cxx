#include <stdio.h>

#include <QtNetwork>
#include <qapplication.h>

#include "gksserver.h"

#define PORT 8410
#define SIZE 262144

GKSServer::GKSServer(QObject *parent) : QTcpServer(parent)
{
  connect(this, SIGNAL(newConnection()), this, SLOT(connectSocket()));
  if (!listen(QHostAddress::Any, PORT))
    {
      qWarning("GKSserver: Failed to listen to port %d", PORT);
      exit(1);
    }

  dl = (char *)malloc(SIZE);
  dl_size = SIZE;
  nbyte = 0;
  ba = (char *)malloc(SIZE);
  ba_size = SIZE;
}

void GKSServer::newWidget()
{
  widget = new GKSWidget();
  connect(this, SIGNAL(data(char *)), widget, SLOT(interpret(char *)));

  widget->setAttribute(Qt::WA_QuitOnClose, false);
  widget->setAttribute(Qt::WA_DeleteOnClose);
  connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(killSocket()));
}

void GKSServer::connectSocket()
{
  socket = this->nextPendingConnection();
  connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
  connect(socket, SIGNAL(disconnected()), this, SLOT(disconnectSocket()));

  newWidget();
}

void GKSServer::readClient()
{
  qint64 cc;
  int length;

  length = socket->bytesAvailable();

  while (length > 0)
    {
      if (nbyte == 0)
        {
          if (socket->bytesAvailable() < (int)sizeof(int)) return;
          cc = socket->read((char *)&nbyte, sizeof(int));

          if (nbyte > dl_size)
            {
              dl = (char *)realloc(dl, nbyte);
              dl_size = nbyte;
            }
        }
      if (socket->bytesAvailable() < nbyte) return;

      cc = socket->read(dl, nbyte);
      if (cc == nbyte)
        {
          if (nbyte + 4 > ba_size)
            {
              ba = (char *)realloc(ba, nbyte + 4);
              ba_size = nbyte + 4;
            }
          memmove(ba, dl, nbyte);
          memset(ba + nbyte, 0, 4);

          emit(data(ba));
          nbyte = 0;

          length = socket->bytesAvailable();
        }
    }
}

void GKSServer::killSocket()
{
  widget->close();
  newWidget();
}

void GKSServer::disconnectSocket()
{
  widget->close();
  exit(0);
}
