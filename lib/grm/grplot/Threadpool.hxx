#ifndef THREADPOOL_HXX_INCLUDED
#define THREADPOOL_HXX_INCLUDED

#ifdef _MSC_VER
#define NO_THREADS 1
#endif
#ifndef NO_THREADS

#include <stddef.h>
#include <pthread.h>

typedef void (*WorkerFuncT)(void *arg);
typedef struct Threadpool
{
  WorkerFuncT worker_func;
  pthread_mutex_t work_mutex;
  pthread_cond_t wait_for_work_cond;
  pthread_cond_t work_fetched;
  pthread_cond_t all_work_done_cond;
  size_t working_cnt;
  size_t thread_cnt;
  pthread_t *threads;
  int stop;
} ThreadpoolT;

void threadpoolCreate(ThreadpoolT *tp, size_t num, WorkerFuncT func);
void threadpoolDestroy(ThreadpoolT *tp);
void threadpoolAddWork(ThreadpoolT *tp, void *arg);
void threadpoolWait(ThreadpoolT *tp);

#endif
#endif /* ifndef THREADPOOL_HXX_INCLUDED */
