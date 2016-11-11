
#include <QtNetwork>
#include <qapplication.h>

#include "gksserver.h"

#define PORT 8410
#define SIZE 262144

#define MAXCONN (int)10

GKSServer::GKSServer(QObject *parent)
  : QTcpServer(parent)
{
  setMaxPendingConnections(MAXCONN);
  connect(this, SIGNAL(newConnection()), this, SLOT(open()));
  connect(this, SIGNAL(newConnection()), this, SLOT(connectSocket()));
  s = NULL;
  bool ok = listen(QHostAddress::Any, PORT);
  if (!ok)
    {
      qWarning("GKSserver: Failed to listen to port %d", PORT);
      exit(1);
    }
  dl = (char *) malloc(SIZE);
  dl_size = SIZE;
  nbyte = 0;
  ba = (char *) malloc(SIZE);
  ba_size = SIZE;
  keepOnDisplay = false;
}

void GKSServer::open()
{
  emit(openWindow());
}

void GKSServer::connectSocket()
{
  if (s != NULL) {
    s->disconnectFromHost();
  }

  s = this->nextPendingConnection();

  connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
  connect(s, SIGNAL(disconnected()), this, SLOT(killSocket()));
}

void GKSServer::readClient()
{
  qint64 cc;
  int length;

  length = s->bytesAvailable();

  while (length > 0)
    {
      if (nbyte == 0)
        {
          if (s->bytesAvailable() < (int) sizeof(int))
            return;
          cc = s->read((char *) &nbyte, sizeof(int));

          if (nbyte > dl_size)
            {
              dl = (char *) realloc(dl, nbyte);
              dl_size = nbyte;
            }
        }
      if (s->bytesAvailable() < nbyte)
        return;

      cc = s->read(dl, nbyte);
      if (cc == nbyte)
        {
          if (nbyte + 4 > ba_size)
            {
              ba = (char *) realloc(ba, nbyte + 4);
              ba_size = nbyte + 4;
            }
          memmove(ba, dl, nbyte);
          memset(ba + nbyte, 0, 4);

          emit(data(ba));
          nbyte = 0;

          length = s->bytesAvailable();
        }
    }
}

void GKSServer::killSocket()
{
  if (!keepOnDisplay)
    exit(0);
}

void GKSServer::setKeepOnDisplay(const bool flag)
{
  keepOnDisplay = flag;
}
