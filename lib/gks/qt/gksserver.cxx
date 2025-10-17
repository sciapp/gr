#include <stdio.h>
#include <sstream>

#include <iostream>

#include <QApplication>
#include <QtNetwork>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QScreen>
#else
#include <QDesktopWidget>
#endif

#include "gkscore.h"

#include "gksserver.h"

#define DEFAULT_PORT 8410

const int GKSConnection::window_shift = 30;
unsigned int GKSConnection::index = 0;
unsigned int GKSServer::port = DEFAULT_PORT;


#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
inline QRect operator-(const QRect &lhs, const QMargins &rhs)
{
  return QRect(QPoint(lhs.left() + rhs.left(), lhs.top() + rhs.top()),
               QPoint(lhs.right() - rhs.right(), lhs.bottom() - rhs.bottom()));
}

inline QRect &operator-=(QRect &rect, const QMargins &margins)
{
  rect = rect - margins;
  return rect;
}
#endif


GKSConnection::GKSConnection(QTcpSocket *socket, GKSServer &server)
    : server(server), socket(socket), widget(NULL), dl(NULL), dl_size(0), socket_function(SocketFunction::unknown)
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
  while (socket->bytesAvailable() > 0 || socket_function != SocketFunction::unknown)
    {
      switch (socket_function)
        {
        case SocketFunction::unknown:
          char socket_function_byte;
          socket->read(&socket_function_byte, 1);
          socket_function = static_cast<SocketFunction::Enum>(socket_function_byte);
          break;
        case SocketFunction::create_window:
          if (widget == NULL)
            {
              newWidget();
            }
          socket_function = SocketFunction::unknown;
          break;
        case SocketFunction::draw:
          if (dl_size == 0)
            {
              if (socket->bytesAvailable() < (long)sizeof(int)) return;
              socket->read((char *)&dl_size, sizeof(int));
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
          socket_function = SocketFunction::unknown;
          break;
        case SocketFunction::is_alive:
          {
            char reply[1]{static_cast<char>(SocketFunction::is_alive)};
            socket->write(reply, sizeof(reply));
            socket->flush();
            socket_function = SocketFunction::unknown;
          }
          break;
        case SocketFunction::close_window:
          {
            bool is_last_connection = !server.has_multiple_connections();
            if (is_last_connection)
              {
                // Prevent new connections from being accepted
                server.close();
              }
            if (widget != NULL)
              {
                widget->close();
              }
            char reply[1]{static_cast<char>(is_last_connection ? 0 : SocketFunction::is_alive)};
            socket->write(reply, sizeof(reply));
            socket->flush();
            socket_function = SocketFunction::unknown;
          }
          break;
        case SocketFunction::inq_ws_state:
          {
            char reply[1 + sizeof(gks_ws_state_t)];
            if (widget != NULL)
              {
                reply[0] = static_cast<char>(SocketFunction::inq_ws_state);
                double device_pixel_ratio = (
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
                    widget->devicePixelRatioF()
#elif QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                    widget->devicePixelRatio()
#else
                    1.0
#endif
                );
                *reinterpret_cast<gks_ws_state_t *>(&reply[1]) =
                    gks_ws_state_t{widget->width(), widget->height(), device_pixel_ratio};
              }
            else
              {
                /* If no widget exists, send back default size and device pixel ratio of primary screen */
                double device_pixel_ratio = (
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                    QGuiApplication::primaryScreen()->devicePixelRatio()
#else
                    1.0
#endif
                );
                reply[0] = static_cast<char>(SocketFunction::inq_ws_state);
                *reinterpret_cast<gks_ws_state_t *>(&reply[1]) = gks_ws_state_t{0, 0, device_pixel_ratio};
              }
            socket->write(reply, sizeof(reply));
            socket_function = SocketFunction::unknown;
          }
          break;
        case SocketFunction::sample_locator:
          {
            char reply[1 + sizeof(gks_locator_t)];
            double x, y;
            int state;
            reply[0] = static_cast<char>(SocketFunction::sample_locator);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            if (widget != NULL)
              {
                QPoint mouse_pos = widget->mapFromGlobal(QCursor::pos());
                x = (double)mouse_pos.x() / widget->width();
                y = 1.0 - (double)mouse_pos.y() / widget->height();
                state = QGuiApplication::mouseButtons();
              }
            else
#endif
              {
                x = y = state = 0;
              }
            *reinterpret_cast<gks_locator_t *>(&reply[1]) = gks_locator_t{x, y, state};
            socket->write(reply, sizeof(reply));
            socket_function = SocketFunction::unknown;
          }
          break;
        default:
          break;
        }
    }
}

void GKSConnection::destroyedWidget()
{
  widget = NULL;
  socket->close();
}

void GKSConnection::disconnectedSocket()
{
  if (widget != NULL)
    {
      widget->close();
      widget = NULL;
    }
  emit(close(*this));
}

void GKSConnection::updateWindowTitle(QString renderer)
{
  std::stringstream window_title_stream;
  window_title_stream << "GKS QtTerm";
  if (widget_index > 1 && !renderer.isEmpty())
    {
      window_title_stream << " (" << widget_index << ", " << renderer.toStdString() << ")";
    }
  else if (widget_index > 1)
    {
      window_title_stream << " (" << widget_index << ")";
    }
  else if (!renderer.isEmpty())
    {
      window_title_stream << " (" << renderer.toStdString() << ")";
    }
  widget->setWindowTitle(window_title_stream.str().c_str());
}

void GKSConnection::newWidget()
{
  widget = new GKSWidget();
  widget_index = index;
  updateWindowTitle();
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  QRect screen_geometry = QGuiApplication::primaryScreen()->availableGeometry();
#else
  QDesktopWidget *desktop = QApplication::desktop();
  QRect screen_geometry = desktop->screenGeometry(desktop->primaryScreen());
#endif
  QPoint screen_center = screen_geometry.center();
  QRect valid_position_area = screen_geometry - QMargins(0, 0, widget->width(), widget->height());
  if (GKSWidget::frame_decoration_size().isValid())
    {
      valid_position_area -=
          QMargins(0, 0, GKSWidget::frame_decoration_size().width(), GKSWidget::frame_decoration_size().height());
    }
  QPoint widget_position =
      QPoint((screen_center.x() - widget->width() / 2 - valid_position_area.left() + index * window_shift) %
                     valid_position_area.width() +
                 valid_position_area.left(),
             (screen_center.y() - widget->height() / 2 - valid_position_area.top() + index * window_shift) %
                     valid_position_area.height() +
                 valid_position_area.top());
  widget->move(widget_position);
  connect(this, SIGNAL(data(char *)), widget, SLOT(interpret(char *)));

  widget->setAttribute(Qt::WA_QuitOnClose, false);
  widget->setAttribute(Qt::WA_DeleteOnClose);
  connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(destroyedWidget()));
  connect(widget, SIGNAL(rendererChanged(QString)), this, SLOT(updateWindowTitle(QString)));
}

GKSServer::GKSServer(QObject *parent) : QTcpServer(parent)
{
  QHostAddress host_address = QHostAddress::LocalHost;
  port = DEFAULT_PORT;
  QString gks_display = QProcessEnvironment::systemEnvironment().value("GKS_DISPLAY");
  if (!gks_display.isEmpty())
    {
      QStringList str = gks_display.split(':');
      if (str.size() > 1)
        {
          host_address.setAddress(str[0].size() > 0 ? str[0] : "127.0.0.1");
          port = str[1].size() > 0 ? str[1].toInt() : DEFAULT_PORT;
        }
    }
  connect(this, SIGNAL(newConnection()), this, SLOT(connectSocket()));
  if (!listen(host_address, port))
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

bool GKSServer::has_multiple_connections()
{
  return connections.size() > 1;
}

void GKSServer::connectSocket()
{
  QTcpSocket *socket = this->nextPendingConnection();
  GKSConnection *connection = new GKSConnection(socket, *this);
  connect(connection, SIGNAL(close(GKSConnection &)), this, SLOT(closeConnection(GKSConnection &)));
  connect(connection, SIGNAL(requestApplicationShutdown(GKSConnection &)), this,
          SLOT(closeConnection(GKSConnection &)));
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
