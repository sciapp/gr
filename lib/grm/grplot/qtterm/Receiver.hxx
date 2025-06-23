#ifndef RECEIVER_THREAD_H_INCLUDED
#define RECEIVER_THREAD_H_INCLUDED

#include "grm.h"
#include "ArgsWrapper.hxx"
#include <QThread>
#include <QDebug>


class Receiver : public QObject
{
  Q_OBJECT

public:
  Receiver(int listen_port);
  virtual ~Receiver();
  void start();

signals:
  void resultReady(ArgsWrapper args);

private slots:
  void receiveData();
  void dataProcessed();

private:
  QThread thread_;
  void *grm_receiver_handle_ = nullptr;
  int listen_port_;
};

#endif /* ifndef RECEIVER_THREAD_H_INCLUDED */
