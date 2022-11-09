#ifndef GRM_NET_INT_H_INCLUDED
#define GRM_NET_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

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

struct _net_handle_t;
typedef struct _net_handle_t net_handle_t;

typedef err_t (*recv_callback_t)(net_handle_t *);
typedef err_t (*send_callback_t)(net_handle_t *);
typedef const char *(*custom_recv_callback_t)(const char *, unsigned int);
typedef int (*custom_send_callback_t)(const char *, unsigned int, const char *);
typedef err_t (*finalize_callback_t)(net_handle_t *);

struct _net_handle_t
{
  int is_receiver;
  union
  {
    struct
    {
      memwriter_t *memwriter;
      size_t message_size;
      recv_callback_t recv;
      union
      {
        struct
        {
          custom_recv_callback_t recv;
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
      memwriter_t *memwriter;
      send_callback_t send;
      union
      {
        struct
        {
          custom_send_callback_t send;
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
  finalize_callback_t finalize;
};

/* ========================= methods ================================================================================ */

/* ------------------------- receiver ------------------------------------------------------------------------------- */

static err_t receiver_init_for_socket(net_handle_t *handle, const char *hostname, unsigned int port);
static err_t receiver_init_for_custom(net_handle_t *handle, const char *name, unsigned int id,
                                      const char *(*custom_recv)(const char *, unsigned int));
static err_t receiver_finalize_for_socket(net_handle_t *handle);
static err_t receiver_finalize_for_custom(net_handle_t *handle);
static err_t receiver_recv_for_socket(net_handle_t *handle);
static err_t receiver_recv_for_custom(net_handle_t *handle);


/* ------------------------- sender --------------------------------------------------------------------------------- */

static err_t sender_init_for_socket(net_handle_t *handle, const char *hostname, unsigned int port);
static err_t sender_init_for_custom(net_handle_t *handle, const char *name, unsigned int id,
                                    int (*custom_send)(const char *, unsigned int, const char *));
static err_t sender_finalize_for_socket(net_handle_t *handle);
static err_t sender_finalize_for_custom(net_handle_t *handle);
static err_t sender_send_for_socket(net_handle_t *handle);
static err_t sender_send_for_custom(net_handle_t *handle);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_NET_INT_H_INCLUDED */
