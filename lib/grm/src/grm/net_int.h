#ifndef GRM_NET_INT_H_INCLUDED
#define GRM_NET_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <netinet/in.h>
#endif

#include "error_int.h"
#include "memwriter_int.h"
#include <grm/net.h>


/* ######################### internal interface ##################################################################### */

/* ========================= macros ================================================================================= */

/* ------------------------- receiver / sender----------------------------------------------------------------------- */

#define SOCKET_RECV_BUF_SIZE (MEMWRITER_INITIAL_SIZE - 1)


/* ------------------------- sender --------------------------------------------------------------------------------- */

#define SEND_REF_FORMAT_MAX_LENGTH 100
#define PORT_MAX_STRING_LENGTH 80


/* ========================= datatypes ============================================================================== */

/* ------------------------- receiver / sender ---------------------------------------------------------------------- */

struct NetHandle;
typedef struct NetHandle NetHandle;

typedef grm_error_t (*RecvCallback)(NetHandle *);
typedef grm_error_t (*SendCallback)(NetHandle *);
typedef const char *(*CustomRecvCallback)(const char *, unsigned int);
typedef int (*CustomSendCallback)(const char *, unsigned int, const char *);
typedef grm_error_t (*FinalizeCallback)(NetHandle *);

struct NetHandle
{
  int is_receiver;
  union
  {
    struct
    {
      /*
       * **Important**:
       * - `memwriter`
       * - `message_size`
       * - `recv`
       * - `send`
       * must be definied in both union parts in the same order
       * -> these attributes can be accessed with both receiver and sender structs
       */
      Memwriter *memwriter;
      size_t message_size;
      RecvCallback recv;
      SendCallback send;
      union
      {
        struct
        {
          CustomRecvCallback recv;
          const char *name;
          unsigned int id;
        } custom;
        struct
        {
          int client_socket;
          int server_socket;
        } socket;
      } comm;
    } receiver;
    struct
    {
      Memwriter *memwriter;
      size_t message_size;
      RecvCallback recv;
      SendCallback send;
      union
      {
        struct
        {
          CustomSendCallback send;
          const char *name;
          unsigned int id;
        } custom;
        struct
        {
          int client_socket;
          struct sockaddr_in server_address;
        } socket;
      } comm;
    } sender;
  } sender_receiver;
  FinalizeCallback finalize;
};

/* ========================= methods ================================================================================ */

/* ------------------------- receiver ------------------------------------------------------------------------------- */

static grm_error_t receiverInitForSocket(NetHandle *handle, const char *hostname, unsigned int port);
static grm_error_t receiverInitForCustom(NetHandle *handle, const char *name, unsigned int id,
                                         const char *(*custom_recv)(const char *, unsigned int));
static grm_error_t receiverFinalizeForSocket(NetHandle *handle);
static grm_error_t receiverFinalizeForCustom(NetHandle *handle);
static grm_error_t receiverRecvForSocket(NetHandle *handle);
static grm_error_t receiverRecvForCustom(NetHandle *handle);


/* ------------------------- sender --------------------------------------------------------------------------------- */

static grm_error_t senderInitForSocket(NetHandle *handle, const char *hostname, unsigned int port);
static grm_error_t senderInitForCustom(NetHandle *handle, const char *name, unsigned int id,
                                       int (*custom_send)(const char *, unsigned int, const char *));
static grm_error_t senderFinalizeForSocket(NetHandle *handle);
static grm_error_t senderFinalizeForCustom(NetHandle *handle);
static grm_error_t senderSendForSocket(NetHandle *handle);
static grm_error_t senderSendForCustom(NetHandle *handle);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_NET_INT_H_INCLUDED */
