
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NO_ZMQ
#include <zmq.h>
#endif

#include "gks.h"
#include "gkscore.h"

#ifdef _WIN32

#include <windows.h>
#define DLLEXPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C"
{
#endif

#else

#ifdef __cplusplus
#define DLLEXPORT extern "C"
#else
#define DLLEXPORT
#endif

#endif

DLLEXPORT void gks_zmqplugin(
  int fctid, int dx, int dy, int dimx, int *ia,
  int lr1, double *r1, int lr2, double *r2,
  int lc, char *chars, void **ptr);

#ifdef _WIN32
#ifdef __cplusplus
}
#endif
#endif


typedef struct
  {
    void *context;
    void *publisher;
    gks_display_list_t dl;
  }
ws_state_list;

#ifndef NO_ZMQ

static
gks_state_list_t *gkss;

void gks_zmqplugin(
  int fctid, int dx, int dy, int dimx, int *ia,
  int lr1, double *r1, int lr2, double *r2,
  int lc, char *chars, void **ptr)
{
  ws_state_list *wss;
  
  wss = (ws_state_list *) *ptr;

  switch (fctid)
    {
    case 2:
      gkss = (gks_state_list_t *) *ptr;      
      wss = (ws_state_list *) gks_malloc(sizeof(ws_state_list));

      wss->context = zmq_ctx_new();
      wss->publisher = zmq_socket(wss->context, ZMQ_PUSH);
      zmq_bind(wss->publisher, "tcp://*:5556");

      gks_init_core(gkss);

      *ptr = wss;
      break;

    case 3:
      zmq_close(wss->publisher);
      zmq_ctx_destroy(wss->context);

      gks_free(wss);
      wss = NULL;
      break;

    case 8:
      if (ia[1] == GKS_K_PERFORM_FLAG)
        {
          zmq_send(wss->publisher, (char *) &wss->dl.nbytes, sizeof(int), 0);
          zmq_send(wss->publisher, wss->dl.buffer, wss->dl.nbytes, 0);
        }
      break;
  }

  if (wss != NULL)
    {
      gks_dl_write_item(
        &wss->dl, fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, gkss);
    }
}

#else

void gks_zmqplugin(
  int fctid, int dx, int dy, int dimx, int *ia,
  int lr1, double *r1, int lr2, double *r2,
  int lc, char *chars, void **ptr)
{
  if (fctid == 2)
  {
    gks_perror("0MQ support not compiled in");
    ia[0] = 0;
  }
}

#endif
