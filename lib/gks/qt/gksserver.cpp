
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

void GKSServer::connectSocket()
{
  if (s != NULL)
    s->disconnectFromHost();

  s = this->nextPendingConnection();

  connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
  connect(s, SIGNAL(disconnected()), this, SLOT(killSocket()));
}

void GKSServer::readClient()
{
  qint64 cc;

  if (nbyte == 0)
    {
      if (s->bytesAvailable() < (int) sizeof(int))
	return;
      cc = s->read((char *) &nbyte, sizeof(int));
      if (nbyte > dl_size)
	{
	  dl = (char *) realloc(dl, nbyte + 1);
          dl_size = nbyte;
	}
    }
  if (s->bytesAvailable() < nbyte)
    return;

  cc = s->read(dl, nbyte);
  if (cc == nbyte)
    {
      dl[nbyte] = '\0';
      if (nbyte > ba_size)
	{
	  ba = (char *) realloc(ba, nbyte + 1);
	  ba_size = nbyte;
	}
      memmove(ba, dl, nbyte + 1);
      if (!s->bytesAvailable())
	emit(data(ba));
      nbyte = 0;
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
