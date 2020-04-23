#include <stdio.h>
#include <sstream>

#include <iostream>

#include <QApplication>
#include <QDesktopWidget>
#include <QtNetwork>

#include "gksserver.h"


const int GKSConnection::window_shift = 30;
unsigned int GKSConnection::index = 0;
const unsigned int GKSServer::port = 8410;


GKSConnection::GKSConnection(QTcpSocket *socket) : socket(socket), widget(NULL), dl(NULL), dl_size(0)
{
  ++index;
  connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
  connect(socket, SIGNAL(disconnected()), this, SLOT(disconnectedSocket()));
  // send information about workstation back to client
  struct
  {
    int nbytes;
    double mwidth;
    double mheight;
    int width;
    int height;
    char name[6];
  } workstation_information = {sizeof(workstation_information), 0, 0, 0, 0, "gksqt"};
  GKSWidget::inqdspsize(&workstation_information.mwidth, &workstation_information.mheight,
                        &workstation_information.width, &workstation_information.height);
  socket->write(reinterpret_cast<const char *>(&workstation_information), workstation_information.nbytes);
}

GKSConnection::~GKSConnection()
{
  socket->close();
  delete socket;
  if (widget != NULL)
    {
      widget->close();
    }
}

void GKSConnection::readClient()
{
  while (socket->bytesAvailable() > 0)
    {
      if (dl_size == 0)
        {
          if (socket->bytesAvailable() < (long)sizeof(int)) return;
          socket->read((char *)&dl_size, sizeof(unsigned int));
        }
      if (socket->bytesAvailable() < dl_size) return;
      dl = new char[dl_size + sizeof(int)];
      socket->read(dl, dl_size);
      // The data buffer must be terminated by a zero integer -> `sizeof(int)` zero bytes
      memset(dl + dl_size, 0, sizeof(int));
      if (widget == NULL)
        {
          newWidget();
        }
      emit(data(dl));
      dl_size = 0;
    }
}

void GKSConnection::destroyedWidget()
{
  widget = NULL;
  emit(close(*this));
}

void GKSConnection::disconnectedSocket()
{
  if (widget != NULL)
    {
      widget->close();
      widget = NULL;
    }
}

void GKSConnection::newWidget()
{
  std::stringstream window_title_stream;
  window_title_stream << "GKS QtTerm";
  if (index > 1)
    {
      window_title_stream << " (" << index << ")";
    }
  widget = new GKSWidget();
  widget->setWindowTitle(window_title_stream.str().c_str());
  QPoint desktop_center = QApplication::desktop()->screenGeometry().center();
  widget->move((desktop_center.x() - widget->width() / 2 + index * window_shift),
               (desktop_center.y() - widget->height() / 2 + index * window_shift));
  connect(this, SIGNAL(data(char *)), widget, SLOT(interpret(char *)));

  widget->setAttribute(Qt::WA_QuitOnClose, false);
  widget->setAttribute(Qt::WA_DeleteOnClose);
  connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(destroyedWidget()));
}

GKSServer::GKSServer(QObject *parent) : QTcpServer(parent)
{
  connect(this, SIGNAL(newConnection()), this, SLOT(connectSocket()));
  if (!listen(QHostAddress::Any, port))
    {
      qWarning("GKSserver: Failed to listen to port %d", port);
      exit(1);
    }
}

GKSServer::~GKSServer()
{
  for (std::list<const GKSConnection *>::iterator it = connections.begin(); it != connections.end(); ++it)
    {
      delete *it;
    }
}

void GKSServer::connectSocket()
{
  QTcpSocket *socket = this->nextPendingConnection();
  GKSConnection *connection = new GKSConnection(socket);
  connect(connection, SIGNAL(close(GKSConnection &)), this, SLOT(closeConnection(GKSConnection &)));
  connections.push_back(connection);
}

void GKSServer::closeConnection(GKSConnection &connection)
{
  connections.remove(&connection);
  connection.deleteLater();
  if (connections.empty())
    {
      QApplication::quit();
    }
}
