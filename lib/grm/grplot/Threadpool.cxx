#ifdef _MSC_VER
#define NO_THREADS 1
#endif
#ifndef NO_THREADS
#include <assert.h>
#include <cmath>

#include "Threadpool.hxx"

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

typedef void *ThreadpoolWorkT;
ThreadpoolWorkT next_work_item;

static void *threadpoolWorker(void *arg)
{
  auto tp = static_cast<ThreadpoolT *>(arg);
  ThreadpoolWorkT work;

  pthread_mutex_lock(&(tp->work_mutex));
  while (1)
    {
      while (next_work_item == nullptr && !tp->stop) pthread_cond_wait(&(tp->wait_for_work_cond), &(tp->work_mutex));

      if (tp->stop) break;

      work = next_work_item;
      next_work_item = nullptr;
      pthread_cond_signal(&(tp->work_fetched));
      tp->working_cnt++;
      pthread_mutex_unlock(&(tp->work_mutex));

      if (work != nullptr) tp->worker_func(work);

      pthread_mutex_lock(&(tp->work_mutex));
      tp->working_cnt--;
      if (!tp->stop && tp->working_cnt == 0 && next_work_item == nullptr)
        pthread_cond_signal(&(tp->all_work_done_cond));
    }

  tp->thread_cnt--;
  pthread_cond_signal(&(tp->all_work_done_cond));
  pthread_mutex_unlock(&(tp->work_mutex));
  return nullptr;
}

void threadpoolCreate(ThreadpoolT *tp, size_t num, WorkerFuncT worker_func)
{
  size_t i;

  num = max(num, 1);

  tp->worker_func = worker_func;
  tp->thread_cnt = num;
  tp->threads = (pthread_t *)calloc(num, sizeof(pthread_t));
  pthread_mutex_init(&(tp->work_mutex), nullptr);
  pthread_cond_init(&(tp->wait_for_work_cond), nullptr);
  pthread_cond_init(&(tp->work_fetched), nullptr);
  pthread_cond_init(&(tp->all_work_done_cond), nullptr);
  tp->working_cnt = 0;
  tp->stop = 0;

  for (i = 0; i < num; i++)
    {
      pthread_create(&tp->threads[i], nullptr, threadpoolWorker, tp);
    }
}

void threadpoolDestroy(ThreadpoolT *tp)
{
  if (tp == nullptr) return;

  threadpoolWait(tp);

  pthread_mutex_lock(&(tp->work_mutex));
  int thread_cnt = tp->thread_cnt;
  tp->stop = 1;
  pthread_cond_broadcast(&(tp->wait_for_work_cond));
  pthread_mutex_unlock(&(tp->work_mutex));

  for (int i = 0; i < thread_cnt; ++i)
    {
      pthread_join(tp->threads[i], nullptr);
    }

  pthread_mutex_destroy(&(tp->work_mutex));
  pthread_cond_destroy(&(tp->wait_for_work_cond));
  pthread_cond_destroy(&(tp->work_fetched));
  pthread_cond_destroy(&(tp->all_work_done_cond));
  free(tp->threads);
  free(tp);
}

void threadpoolAddWork(ThreadpoolT *tp, void *arg)
{
  pthread_mutex_lock(&(tp->work_mutex));

  assert(next_work_item == nullptr);
  next_work_item = arg;

  pthread_cond_signal(&(tp->wait_for_work_cond));
  /* wait until `next_work_item` is picked up by a worker thread before returning */
  pthread_cond_wait(&(tp->work_fetched), &(tp->work_mutex));
  pthread_mutex_unlock(&(tp->work_mutex));
}

void threadpoolWait(ThreadpoolT *tp)
{
  if (tp == nullptr) return;

  pthread_mutex_lock(&(tp->work_mutex));
  while ((!tp->stop && tp->working_cnt != 0) || (tp->stop && tp->thread_cnt != 0))
    {
      pthread_cond_wait(&(tp->all_work_done_cond), &(tp->work_mutex));
    }
  pthread_mutex_unlock(&(tp->work_mutex));
}

#endif
