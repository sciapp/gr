#include "receiver_thread.h"

void Receiver_Thread::run()
{
  void *handle = nullptr;
  grm_args_t_wrapper args;
  bool ready = false;

  while (true)
    {
      fflush(stdout);
      if (handle == nullptr)
        {
          handle = grm_open(GRM_RECEIVER, "127.0.0.1", 8002, nullptr, nullptr);
          if (handle == nullptr)
            {
              qCritical() << "receiver could not be created";
              qCritical() << "Retrying in 5 seconds";
              QThread::sleep(5);
              ready = false;
              continue;
            }
        }
      args.set_wrapper(grm_recv(handle, nullptr));
      if (args.get_wrapper() == nullptr)
        {
          if (ready)
            {
              qCritical() << "data could not be received from stream";
            }
          grm_close(handle);
          handle = nullptr;
        }
      else
        {
          emit resultReady(args);
          ready = true;
        }
    }
  if (handle != nullptr)
    {
      grm_close(handle);
    }
}
