#include "Receiver.hxx"

Receiver::Receiver(int listen_port)
{
  listen_port_ = listen_port;
  moveToThread(&thread_);
  connect(&thread_, &QThread::started, this, &Receiver::receiveData);
}

Receiver::~Receiver()
{
  if (grm_receiver_handle_ != nullptr)
    {
      grm_close(grm_receiver_handle_);
    }
  /*
   * TODO: Actually, we should call
   *   thread_.quit();
   *   thread_.wait();
   * but the thread cannot recognize this, since it will either be waiting in `grm_open` or `grm_recv`.
   * As a result, the thread lives until the application is closed.
   */
}

void Receiver::start()
{
  thread_.start();
}

void Receiver::dataProcessed()
{
  receiveData();
}

void Receiver::receiveData()
{
  ArgsWrapper args;
  bool received_data = false;

  /*
   * `gr_startlistener` opens and immediately closes a connection to test if `grplot` is running. Thus, we need to
   * repeat `grm_open` and `grm_recv` calls until a *real* data connection is established.
   */
  while (!received_data)
    {
      if (grm_receiver_handle_ == nullptr)
        {
          bool receiver_opened = false;
          while (!receiver_opened)
            {
              grm_receiver_handle_ = grm_open(GRM_RECEIVER, "127.0.0.1", listen_port_, nullptr, nullptr);
              if (grm_receiver_handle_ != nullptr)
                {
                  receiver_opened = true;
                }
              else
                {
                  qCritical() << "receiver could not be created";
                  qCritical() << "Retrying in 5 seconds";
                  QThread::sleep(5);
                }
            }
        }

      args.setWrapper(grm_recv(grm_receiver_handle_, nullptr));
      if (args.getWrapper() != nullptr)
        {
          received_data = true;
        }
      else
        {
          grm_close(grm_receiver_handle_);
          grm_receiver_handle_ = nullptr;
        }
    }

  emit resultReady(args);
}
