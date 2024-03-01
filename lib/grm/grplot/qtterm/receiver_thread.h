#ifndef RECEIVER_THREAD_H_INCLUDED
#define RECEIVER_THREAD_H_INCLUDED

#include "grm.h"
#include "grm_args_t_wrapper.h"
#include <QThread>
#include <QDebug>


class Receiver : public QObject
{
  Q_OBJECT

public:
  Receiver();
  virtual ~Receiver();
  void start();

signals:
  void resultReady(grm_args_t_wrapper args);

private slots:
  void receiveData();
  void dataProcessed();

private:
  QThread thread_;
  void *grm_receiver_handle_ = nullptr;
};

#endif /* ifndef RECEIVER_THREAD_H_INCLUDED */
