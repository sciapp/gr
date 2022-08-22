#include "receiver_thread.h"

void Receiver_Thread::run()
{
  void *handle = nullptr;
  grm_args_t_wrapper args;
  while (running)
    {
      fflush(stdout);
      if (handle == nullptr)
        {
          handle = grm_open(GRM_RECEIVER, "0.0.0.0", 8002, nullptr, nullptr);
          if (handle == nullptr)
            {
              qCritical() << "receiver could not be created";
              qCritical() << "Retrying in 5 seconds";
              QThread::sleep(5);
              continue;
            }
        }
      args.set_wrapper(grm_recv(handle, nullptr));
      if (args.get_wrapper() == nullptr)
        {
          qCritical() << "data could not be received from stream";
          grm_close(handle);
          handle = nullptr;
        }
      else
        {
          emit resultReady(args);
        }
    }
  if (handle != nullptr)
    {
      grm_close(handle);
    }
}
