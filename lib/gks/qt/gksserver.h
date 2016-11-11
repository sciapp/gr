
#include <QTcpServer>
#include <QTcpSocket>
#include <qstring.h>

class GKSServer : public QTcpServer
{
  Q_OBJECT
    
public:
  GKSServer(QObject *parent = 0);
  
public slots:
  void readClient();
  void killSocket();
  void connectSocket();
  void setKeepOnDisplay(const bool flag);
  void open();
 
private:
  QTcpSocket *s;
  char *dl, *ba;
  int nbyte, dl_size, ba_size;
  bool keepOnDisplay;
  bool isopen;

signals:
  void data(char *);
  void openWindow();
};
