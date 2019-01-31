#ifndef GLGRSERVER_H_INCLUDED
#define GLGRSERVER_H_INCLUDED


#include <QTcpServer>
#include <QTcpSocket>
#include <qstring.h>

class glgrServer : public QTcpServer
{
  Q_OBJECT

public:
  glgrServer(QObject *parent = 0);

public slots:
  void readClient();
  void killSocket();
  void NewConnection();

private:
  QString data_string;
  QTcpSocket *s;

signals:
  void newData(QString &);
};
#endif
