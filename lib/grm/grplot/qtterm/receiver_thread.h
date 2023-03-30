#ifndef RECEIVER_THREAD_H_INCLUDED
#define RECEIVER_THREAD_H_INCLUDED

#include "grm.h"
#include "grm_args_t_wrapper.h"
#include <QThread>
#include <QDebug>


class Receiver_Thread : public QThread
{
  Q_OBJECT
signals:
  void resultReady(grm_args_t_wrapper args);

private:
  void run() override;

  bool running = true;
};

#endif /* ifndef RECEIVER_THREAD_H_INCLUDED */
