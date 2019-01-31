
#include <QtNetwork>
#include <qapplication.h>

#include "glgrserver.h"

#define PORT (qint16)0x1234
#define MAXCONN (int)10
#define MY_BUFSIZ ((qint64)1 << 18)

glgrServer::glgrServer(QObject *parent) : QTcpServer(parent)
{
  setMaxPendingConnections(MAXCONN);
  connect(this, SIGNAL(newConnection()), this, SLOT(NewConnection()));
  s = NULL;
  bool ok = listen(QHostAddress::Any, PORT);
  if (!ok)
    {
      qWarning("glgrserver: Failed to listen to port %d", PORT);
      exit(1);
    }
}

void glgrServer::NewConnection()
{
  if (s != NULL) s->disconnectFromHost();

  s = this->nextPendingConnection();
  s->setReadBufferSize(MY_BUFSIZ);

  connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
  /* used to re-emit the readyRead() signal in case we already missed it
     at this point; see
       http://doc.trolltech.com/4.3/qabstractsocket.html#waitForReadyRead
         or
       http://doc.trolltech.com/4.3/qiodevice.html#readyRead
   */
  s->waitForReadyRead();
  connect(s, SIGNAL(disconnected()), this, SLOT(killSocket()));
}

void glgrServer::readClient()
{
  qint64 cc;
  char buf[MY_BUFSIZ + 1];

  while (s->bytesAvailable())
    {
      cc = s->read(buf, MY_BUFSIZ);
      if (cc <= 0 && !s->waitForReadyRead()) break;

      buf[cc] = '\0';
      data_string += QString(buf);
      if (data_string.indexOf("</gr>") != -1)
        {
          emit(newData(data_string));
          data_string = QString("");
        }
    }
}

void glgrServer::killSocket() {}
