/* ######################### includes ############################################################################### */

#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

#include <ctype.h>
#include <limits.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

#include "args_int.h"
#include "dynamic_args_array_int.h"
#include "json_int.h"
#include "net_int.h"
#include "datatype/string_list_int.h"
#include "datatype/template/list_int.h"


/* ######################### private interface ###################################################################### */

/* ========================= datatypes ============================================================================== */

/* ------------------------- args_list ------------------------------------------------------------------------------ */

DECLARE_LIST_TYPE(Args, grm_args_t *)


/* ------------------------- dynamic_args_array_list ---------------------------------------------------------------- */

DECLARE_LIST_TYPE(DynamicArgsArray, DynamicArgsArray *)

#undef DECLARE_LIST_TYPE


/* ========================= methods ================================================================================ */

/* ------------------------- args_list ------------------------------------------------------------------------------ */

DECLARE_LIST_METHODS(Args, args)


/* ------------------------- dynamic_args_array_list ---------------------------------------------------------------- */

DECLARE_LIST_METHODS(DynamicArgsArray, dynamicArgsArray)

#undef DECLARE_LIST_METHODS


/* ######################### private implementation ################################################################# */

/* ========================= methods ================================================================================ */

/* ------------------------- args_list ------------------------------------------------------------------------------ */

DEFINE_LIST_METHODS(Args, args)

grm_error_t argsListEntryCopy(ArgsListEntry *copy, ArgsListConstEntry entry)
{
  ArgsListEntry tmp_copy;

  tmp_copy = argsCopy(entry);
  if (tmp_copy == NULL) return GRM_ERROR_MALLOC;
  *copy = tmp_copy;

  return GRM_ERROR_NONE;
}

grm_error_t argsListEntryDelete(ArgsListEntry entry)
{
  grm_args_delete(entry);
  return GRM_ERROR_NONE;
}


/* ------------------------- dynamic_args_array_list ---------------------------------------------------------------- */

DEFINE_LIST_METHODS(DynamicArgsArray, dynamicArgsArray)

grm_error_t dynamicArgsArrayListEntryCopy(DynamicArgsArrayListEntry *copy, DynamicArgsArrayListConstEntry entry)
{
  /* TODO: create a copy of the object! Otherwise code will segfault on list deletion for a non-ref list */
  *copy = (DynamicArgsArrayListEntry)entry;
  return GRM_ERROR_NONE;
}

grm_error_t dynamicArgsArrayListEntryDelete(DynamicArgsArrayListEntry entry)
{
  dynamicArgsArrayDelete(entry);
  return GRM_ERROR_NONE;
}


#undef DEFINE_LIST_METHODS

/* ######################### internal implementation ################################################################ */

/* ========================= methods ================================================================================ */

/* ------------------------- receiver ------------------------------------------------------------------------------- */

grm_error_t receiverInitForCustom(NetHandle *handle, const char *name, unsigned int id,
                                  const char *(*custom_recv)(const char *, unsigned int))
{
  handle->sender_receiver.receiver.comm.custom.recv = custom_recv;
  handle->sender_receiver.receiver.comm.custom.name = name;
  handle->sender_receiver.receiver.comm.custom.id = id;
  handle->sender_receiver.receiver.message_size = 0;
  handle->sender_receiver.receiver.recv = receiverRecvForCustom;
  handle->sender_receiver.receiver.send = NULL;
  handle->finalize = receiverFinalizeForCustom;
  handle->sender_receiver.receiver.memwriter = memwriterNew();
  if (handle->sender_receiver.receiver.memwriter == NULL) return GRM_ERROR_MALLOC;

  return GRM_ERROR_NONE;
}

grm_error_t receiverInitForSocket(NetHandle *handle, const char *hostname, unsigned int port)
{
  char port_str[PORT_MAX_STRING_LENGTH];
  struct addrinfo *addr_info = NULL, addr_hints;
  struct sockaddr_in client_addr;
  socklen_t client_addrlen = sizeof(client_addr);
  int error;
#ifdef SO_REUSEADDR
  int socket_opt;
#endif
#ifdef _WIN32
  int wsa_startup_error = 0;
  WSADATA wsa_data;
#endif

  snprintf(port_str, PORT_MAX_STRING_LENGTH, "%u", port);

  handle->sender_receiver.receiver.memwriter = NULL;
  handle->sender_receiver.receiver.comm.socket.server_socket = -1;
  handle->sender_receiver.receiver.comm.socket.client_socket = -1;
  handle->sender_receiver.receiver.message_size = 0;
  handle->sender_receiver.receiver.recv = receiverRecvForSocket;
  handle->sender_receiver.receiver.send = senderSendForSocket;
  handle->finalize = receiverFinalizeForSocket;

#ifdef _WIN32
  /* Initialize winsock */
  if ((wsa_startup_error = WSAStartup(MAKEWORD(2, 2), &wsa_data)))
    {
#ifndef NDEBUG
      /* on WSAStartup failure `WSAGetLastError` should not be called (see MSDN), use the error code directly instead
       */
      wchar_t *message = NULL;
      FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                     wsa_startup_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&message, 0, NULL);
      debugPrintError(("winsock initialization failed: %S\n", message));
      LocalFree(message);
#endif
      return GRM_ERROR_NETWORK_WINSOCK_INIT;
    }
#endif

  memset(&addr_hints, 0, sizeof(addr_hints));
  addr_hints.ai_family = AF_UNSPEC;
  addr_hints.ai_socktype = SOCK_STREAM;
  addr_hints.ai_protocol = 0;
  addr_hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;

  /* Query a list of ip addresses for the given hostname */
  if ((error = getaddrinfo(hostname, port_str, &addr_hints, &addr_info)) != 0)
    {
#ifdef _WIN32
      pSocketError("getaddrinfo failed with error");
#else
      if (error == EAI_SYSTEM)
        {
          perror("getaddrinfo failed with error");
        }
      else
        {
          fprintf(stderr, "getaddrinfo failed with error: %s\n", gai_strerror(error));
        }
#endif
      return GRM_ERROR_NETWORK_HOSTNAME_RESOLUTION;
    }

  /* Create a socket for listening */
  if ((handle->sender_receiver.receiver.comm.socket.server_socket =
           socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol)) < 0)
    {
      pSocketError("socket creation failed");
      freeaddrinfo(addr_info);
      return GRM_ERROR_NETWORK_SOCKET_CREATION;
    }
    /* Set SO_REUSEADDR if available on this system */
#ifdef SO_REUSEADDR
  socket_opt = 1;
  if (setsockopt(handle->sender_receiver.receiver.comm.socket.server_socket, SOL_SOCKET, SO_REUSEADDR,
                 (char *)&socket_opt, sizeof(socket_opt)) < 0)
    {
      pSocketError("setting socket options failed");
      freeaddrinfo(addr_info);
      return GRM_ERROR_NETWORK_SOCKET_CREATION;
    }
#endif

  /* Bind the socket to given ip address and port */
  if (bind(handle->sender_receiver.receiver.comm.socket.server_socket, addr_info->ai_addr, addr_info->ai_addrlen))
    {
      pSocketError("bind failed");
      freeaddrinfo(addr_info);
      return GRM_ERROR_NETWORK_SOCKET_BIND;
    }
  freeaddrinfo(addr_info);

  /* Listen for incoming connections */
  if (listen(handle->sender_receiver.receiver.comm.socket.server_socket, 1))
    {
      pSocketError("listen failed");
      return GRM_ERROR_NETWORK_SOCKET_LISTEN;
    }

  /* Accecpt an incoming connection and get a new socket instance for communication */
  if ((handle->sender_receiver.receiver.comm.socket.client_socket =
           accept(handle->sender_receiver.receiver.comm.socket.server_socket, (struct sockaddr *)&client_addr,
                  &client_addrlen)) < 0)
    {
      pSocketError("accept failed");
      return GRM_ERROR_NETWORK_CONNECTION_ACCEPT;
    }

  handle->sender_receiver.receiver.memwriter = memwriterNew();
  if (handle->sender_receiver.receiver.memwriter == NULL) return GRM_ERROR_MALLOC;

  return GRM_ERROR_NONE;
}

grm_error_t receiverFinalizeForCustom(NetHandle *handle)
{
  memwriterDelete(handle->sender_receiver.receiver.memwriter);

  return GRM_ERROR_NONE;
}

grm_error_t receiverFinalizeForSocket(NetHandle *handle)
{
  grm_error_t error = GRM_ERROR_NONE;

  memwriterDelete(handle->sender_receiver.receiver.memwriter);
#ifdef _WIN32
  if (handle->sender_receiver.receiver.comm.socket.client_socket >= 0)
    {
      if (closesocket(handle->sender_receiver.receiver.comm.socket.client_socket))
        {
          pSocketError("client socket shutdown failed");
          error = GRM_ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
  if (handle->sender_receiver.receiver.comm.socket.server_socket >= 0)
    {
      if (closesocket(handle->sender_receiver.receiver.comm.socket.server_socket))
        {
          pSocketError("server socket shutdown failed");
          error = GRM_ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
  if (WSACleanup())
    {
      pSocketError("winsock shutdown failed");
      error = GRM_ERROR_NETWORK_WINSOCK_CLEANUP;
    }
#else
  if (handle->sender_receiver.receiver.comm.socket.client_socket >= 0)
    {
      if (close(handle->sender_receiver.receiver.comm.socket.client_socket))
        {
          pSocketError("client socket shutdown failed");
          error = GRM_ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
  if (handle->sender_receiver.receiver.comm.socket.server_socket >= 0)
    {
      if (close(handle->sender_receiver.receiver.comm.socket.server_socket))
        {
          pSocketError("server socket shutdown failed");
          error = GRM_ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
#endif

  return error;
}

grm_error_t receiverRecvForSocket(NetHandle *handle)
{
  int search_start_index = 0;
  char *end_ptr;
  static char recv_buf[SOCKET_RECV_BUF_SIZE];
  grm_error_t error = GRM_ERROR_NONE;

  while ((end_ptr = memchr(memwriterBuf(handle->sender_receiver.receiver.memwriter) + search_start_index, ETB,
                           memwriterSize(handle->sender_receiver.receiver.memwriter) - search_start_index)) == NULL)
    {
      int bytes_received;
      search_start_index = memwriterSize(handle->sender_receiver.receiver.memwriter);
      bytes_received =
          recv(handle->sender_receiver.receiver.comm.socket.client_socket, recv_buf, SOCKET_RECV_BUF_SIZE, 0);
      if (bytes_received < 0)
        {
          pSocketError("error while receiving data");
          return GRM_ERROR_NETWORK_RECV;
        }
      else if (bytes_received == 0)
        {
          return GRM_ERROR_NETWORK_RECV_CONNECTION_SHUTDOWN;
        }
      if ((error = memwriterPrintf(handle->sender_receiver.receiver.memwriter, "%.*s", bytes_received, recv_buf)) !=
          GRM_ERROR_NONE)
        {
          return error;
        }
    }
  *end_ptr = '\0';
  handle->sender_receiver.receiver.message_size = end_ptr - memwriterBuf(handle->sender_receiver.receiver.memwriter);

  return error;
}

grm_error_t receiverRecvForCustom(NetHandle *handle)
{
  /* TODO: is it really necessary to copy the memory? */
  const char *recv_buf;
  grm_error_t error = GRM_ERROR_NONE;

  recv_buf = handle->sender_receiver.receiver.comm.custom.recv(handle->sender_receiver.receiver.comm.custom.name,
                                                               handle->sender_receiver.receiver.comm.custom.id);
  if (recv_buf == NULL) return GRM_ERROR_CUSTOM_RECV;
  memwriterClear(handle->sender_receiver.receiver.memwriter);
  if ((error = memwriterPuts(handle->sender_receiver.receiver.memwriter, recv_buf)) != GRM_ERROR_NONE) return error;
  handle->sender_receiver.receiver.message_size = memwriterSize(handle->sender_receiver.receiver.memwriter);

  return error;
}


/* ------------------------- sender --------------------------------------------------------------------------------- */

grm_error_t senderInitForCustom(NetHandle *handle, const char *name, unsigned int id,
                                int (*custom_send)(const char *, unsigned int, const char *))
{
  handle->sender_receiver.sender.comm.custom.send = custom_send;
  handle->sender_receiver.sender.comm.custom.name = name;
  handle->sender_receiver.sender.comm.custom.id = id;
  handle->sender_receiver.sender.message_size = 0;
  handle->sender_receiver.sender.recv = NULL;
  handle->sender_receiver.sender.send = senderSendForCustom;
  handle->finalize = senderFinalizeForCustom;
  handle->sender_receiver.sender.memwriter = memwriterNew();
  if (handle->sender_receiver.sender.memwriter == NULL) return GRM_ERROR_MALLOC;

  return GRM_ERROR_NONE;
}

grm_error_t senderInitForSocket(NetHandle *handle, const char *hostname, unsigned int port)
{
  char port_str[PORT_MAX_STRING_LENGTH];
  struct addrinfo *addr_info = NULL, addr_hints;
  int socket_opt;
  int error;
#ifdef _WIN32
  int wsa_startup_error = 0;
  WSADATA wsa_data;
#endif
  size_t retry_count, max_retry_count = 50;
  int s;

  /* In order to not sleep an excessive amount start with a short sleep time and then ramp
     it up to `max_sleep_time` */
  int sleep_ms;
  int ms_to_ns = 1000000;
  int initial_sleep_time_ms[] = {5, 10, 25, 50, 100};
  int max_sleep_time_ms = 300;
  size_t n_initial_times = sizeof(initial_sleep_time_ms) / sizeof(initial_sleep_time_ms[0]);
  max_retry_count += n_initial_times;

  snprintf(port_str, PORT_MAX_STRING_LENGTH, "%u", port);

  handle->sender_receiver.sender.memwriter = NULL;
  handle->sender_receiver.sender.comm.socket.client_socket = -1;
  handle->sender_receiver.sender.message_size = 0;
  handle->sender_receiver.sender.recv = receiverRecvForSocket;
  handle->sender_receiver.sender.send = senderSendForSocket;
  handle->finalize = senderFinalizeForSocket;

#ifdef _WIN32
  /* Initialize winsock */
  if ((wsa_startup_error = WSAStartup(MAKEWORD(2, 2), &wsa_data)))
    {
#ifndef NDEBUG
      /* on WSAStartup failure `WSAGetLastError` should not be called (see MSDN), use the error code directly instead
       */
      wchar_t *message = NULL;
      FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                     wsa_startup_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&message, 0, NULL);
      debugPrintError(("winsock initialization failed: %S\n", message));
      LocalFree(message);
#endif
      return GRM_ERROR_NETWORK_WINSOCK_INIT;
    }
#endif

  memset(&addr_hints, 0, sizeof(addr_hints));
  addr_hints.ai_family = AF_INET;
  addr_hints.ai_socktype = SOCK_STREAM;

  /* Query a list of ip addresses for the given hostname */
  if ((error = getaddrinfo(hostname, port_str, &addr_hints, &addr_info)) != 0)
    {
      addr_hints.ai_family = AF_INET6;
      error = getaddrinfo(hostname, port_str, &addr_hints, &addr_info);
    }

  if (error != 0)
    {
#ifdef _WIN32
      pSocketError("getaddrinfo failed with error");
#else
      if (error == EAI_SYSTEM)
        {
          perror("getaddrinfo failed with error");
        }
      else
        {
          fprintf(stderr, "getaddrinfo failed with error: %s\n", gai_strerror(error));
        }
#endif
      return GRM_ERROR_NETWORK_HOSTNAME_RESOLUTION;
    }

  /* Create a socket for connecting to server */
  handle->sender_receiver.sender.comm.socket.client_socket =
      socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
  if (handle->sender_receiver.sender.comm.socket.client_socket < 0)
    {
      pSocketError("socket creation failed");

      return GRM_ERROR_NETWORK_SOCKET_CREATION;
    }

    /* Set `SO_REUSEADDR` to reuse address / port combinations, even if in `TIME_WAIT` state. This can happen
     * since the same address / port combination is used before to test connectivity to grplot */
#ifdef SO_REUSEADDR
  socket_opt = 1;
  if (setsockopt(handle->sender_receiver.sender.comm.socket.client_socket, SOL_SOCKET, SO_REUSEADDR,
                 (char *)&socket_opt, sizeof(socket_opt)) < 0)
    {
      pSocketError("setting SO_REUSEADDR socket option failed");
      freeaddrinfo(addr_info);
      return GRM_ERROR_NETWORK_SOCKET_CREATION;
    }
#endif
#ifdef SO_SNDBUF
  socket_opt = 128 * 128 * 16;
  if (setsockopt(handle->sender_receiver.sender.comm.socket.client_socket, SOL_SOCKET, SO_SNDBUF, (char *)&socket_opt,
                 sizeof(socket_opt)) < 0)
    {
      pSocketError("setting SO_SNDBUF socket option failed");
      freeaddrinfo(addr_info);
      return GRM_ERROR_NETWORK_SOCKET_CREATION;
    }
#endif
  /* Connect to server */
  for (retry_count = 1; retry_count <= max_retry_count; retry_count++)
    {
      if ((s = connect(handle->sender_receiver.sender.comm.socket.client_socket, addr_info->ai_addr,
                       (int)addr_info->ai_addrlen)) == -1)
        {
          sleep_ms = retry_count <= n_initial_times ? initial_sleep_time_ms[retry_count - 1] : max_sleep_time_ms;
#ifndef _WIN32
          struct timespec delay;
          delay.tv_sec = 0;
          delay.tv_nsec = sleep_ms * ms_to_ns;
          while (nanosleep(&delay, &delay) == -1)
            ;
#else
          Sleep(sleep_ms);
#endif
        }
      else
        break;
    }

  if (s == -1)
    {
#ifdef _WIN32
      closesocket(handle->sender_receiver.sender.comm.socket.client_socket);
#else
      close(handle->sender_receiver.sender.comm.socket.client_socket);
#endif
      handle->sender_receiver.sender.comm.socket.client_socket = -1;
    }
  freeaddrinfo(addr_info);

  if (handle->sender_receiver.sender.comm.socket.client_socket < 0)
    {
      fprintf(stderr, "cannot connect to host %s port %u: ", hostname, port);
      pSocketError("");
      return GRM_ERROR_NETWORK_CONNECT;
    }

  handle->sender_receiver.sender.memwriter = memwriterNew();
  if (handle->sender_receiver.sender.memwriter == NULL) return GRM_ERROR_MALLOC;

  return GRM_ERROR_NONE;
}

grm_error_t senderFinalizeForCustom(NetHandle *handle)
{
  memwriterDelete(handle->sender_receiver.sender.memwriter);

  return GRM_ERROR_NONE;
}

grm_error_t senderFinalizeForSocket(NetHandle *handle)
{
  grm_error_t error = GRM_ERROR_NONE;

  memwriterDelete(handle->sender_receiver.sender.memwriter);
#ifdef _WIN32
  if (handle->sender_receiver.sender.comm.socket.client_socket >= 0)
    {
      if (closesocket(handle->sender_receiver.sender.comm.socket.client_socket))
        {
          pSocketError("client socket shutdown failed");
          error = GRM_ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
  if (WSACleanup())
    {
      pSocketError("winsock shutdown failed");
      error = GRM_ERROR_NETWORK_WINSOCK_CLEANUP;
    }
#else
  if (handle->sender_receiver.sender.comm.socket.client_socket >= 0)
    {
      if (close(handle->sender_receiver.sender.comm.socket.client_socket))
        {
          pSocketError("client socket shutdown failed");
          error = GRM_ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
#endif

  return error;
}

grm_error_t senderSendForSocket(NetHandle *handle)
{
  const char *buf;
  size_t buf_size;
  int bytes_left;
  grm_error_t error = GRM_ERROR_NONE;

  if ((error = memwriterPutc(handle->sender_receiver.sender.memwriter, ETB)) != GRM_ERROR_NONE) return error;

  buf = memwriterBuf(handle->sender_receiver.sender.memwriter);
  buf_size = memwriterSize(handle->sender_receiver.sender.memwriter);

  bytes_left = buf_size;
  while (bytes_left)
    {
      int bytes_sent = send(handle->sender_receiver.sender.comm.socket.client_socket, buf, bytes_left, 0);
      if (bytes_sent < 0)
        {
          pSocketError("could not send any data");
          return GRM_ERROR_NETWORK_SEND;
        }
      bytes_left -= bytes_sent;
    }

  memwriterClear(handle->sender_receiver.sender.memwriter);

  return error;
}

grm_error_t senderSendForCustom(NetHandle *handle)
{
  const char *buf;
  grm_error_t error = GRM_ERROR_NONE;

  buf = memwriterBuf(handle->sender_receiver.sender.memwriter);
  if (!handle->sender_receiver.sender.comm.custom.send(handle->sender_receiver.sender.comm.custom.name,
                                                       handle->sender_receiver.sender.comm.custom.id, buf))
    {
      error = GRM_ERROR_CUSTOM_SEND;
      return error;
    }
  memwriterClear(handle->sender_receiver.sender.memwriter);

  return error;
}


/* ######################### public implementation ################################################################## */

/* ========================= methods ================================================================================ */

/* ------------------------- receiver / sender ---------------------------------------------------------------------- */

void *grm_open(int is_receiver, const char *name, unsigned int id,
               const char *(*custom_recv)(const char *, unsigned int),
               int (*custom_send)(const char *, unsigned int, const char *))
{
  NetHandle *handle;
  grm_error_t error = GRM_ERROR_NONE;

  handle = malloc(sizeof(NetHandle));
  if (handle == NULL) return NULL;
  handle->is_receiver = is_receiver;
  handle->sender_receiver.receiver.comm.custom.recv = custom_recv;
  if (is_receiver)
    {
      if (custom_recv != NULL)
        {
          error = receiverInitForCustom(handle, name, id, custom_recv);
        }
      else
        {
          error = receiverInitForSocket(handle, name, id);
        }
    }
  else
    {
      if (custom_send != NULL)
        {
          error = senderInitForCustom(handle, name, id, custom_send);
        }
      else
        {
          error = senderInitForSocket(handle, name, id);
        }
    }

  if (error != GRM_ERROR_NONE)
    {
      if (error != GRM_ERROR_NETWORK_WINSOCK_INIT) handle->finalize(handle);
      free(handle);
      handle = NULL;
    }

  return (void *)handle;
}

void grm_close(const void *p)
{
  NetHandle *handle = (NetHandle *)p;

  handle->finalize(handle);
  free(handle);
}


/* ------------------------- receiver ------------------------------------------------------------------------------- */

grm_args_t *grm_recv(const void *p, grm_args_t *args)
{
  NetHandle *handle = (NetHandle *)p;
  int created_args = 0;

  if (handle->sender_receiver.receiver.recv == NULL)
    {
      /* Return NULL if receiving is unsupported (for example on an unidirectional connection */
      return NULL;
    }

  if (args == NULL)
    {
      args = grm_args_new();
      if (args == NULL) goto error_cleanup;
      created_args = 1;
    }

  if (handle->sender_receiver.receiver.recv(handle) != GRM_ERROR_NONE) goto error_cleanup;
  if (fromJsonRead(args, memwriterBuf(handle->sender_receiver.receiver.memwriter)) != GRM_ERROR_NONE)
    goto error_cleanup;

  if (memwriterErase(handle->sender_receiver.receiver.memwriter, 0,
                     handle->sender_receiver.receiver.message_size + 1) != GRM_ERROR_NONE)
    {
      goto error_cleanup;
    }

  return args;

error_cleanup:
  if (created_args) grm_args_delete(args);

  return NULL;
}


/* ------------------------- sender --------------------------------------------------------------------------------- */

int grm_send(const void *p, const char *data_desc, ...)
{
  NetHandle *handle = (NetHandle *)p;
  va_list vl;
  grm_error_t error;

  va_start(vl, data_desc);
  if (handle->sender_receiver.sender.send != NULL)
    {
      error = toJsonWriteVl(handle->sender_receiver.sender.memwriter, data_desc, &vl);
      if (error == GRM_ERROR_NONE && toJsonIsComplete() && handle->sender_receiver.sender.send != NULL)
        {
          error = handle->sender_receiver.sender.send(handle);
        }
    }
  else
    {
      /* Send can be unsupported, if sending is requested on an unidirectional receiver */
      error = GRM_ERROR_NETWORK_SEND_UNSUPPORTED;
    }
  va_end(vl);

  return error == GRM_ERROR_NONE;
}

int grm_send_buf(const void *p, const char *data_desc, const void *buffer, int apply_padding)
{
  NetHandle *handle = (NetHandle *)p;
  grm_error_t error;

  error = toJsonWriteBuf(handle->sender_receiver.sender.memwriter, data_desc, buffer, apply_padding);
  if (error == GRM_ERROR_NONE && toJsonIsComplete() && handle->sender_receiver.sender.send != NULL)
    {
      error = handle->sender_receiver.sender.send(handle);
    }

  return error == GRM_ERROR_NONE;
}

int grm_send_ref(const void *p, const char *key, char format, const void *ref, int len)
{
  static const char valid_opening_brackets[] = "([{";
  static const char valid_closing_brackets[] = ")]}";
  static const char valid_separator[] = ",";
  static grm_args_t *current_args = NULL;
  static DynamicArgsArray *current_args_array = NULL;
  char *tmp_key = NULL;
  NetHandle *handle = (NetHandle *)p;
  char format_string[SEND_REF_FORMAT_MAX_LENGTH];
  grm_error_t error = GRM_ERROR_NONE;

  if (toJsonStructNestedLevel() == 0) grm_send(handle, "o(");
  if (strchr("soO", format) == NULL)
    {
      /* handle general cases (values and arrays) */
      if (islower(format))
        {
          if (current_args_array == NULL)
            {
              snprintf(format_string, SEND_REF_FORMAT_MAX_LENGTH, "%s:%c,", key, format);
              error = grm_send_buf(handle, format_string, ref, 1);
            }
          else
            {
              snprintf(format_string, SEND_REF_FORMAT_MAX_LENGTH, "%c", format);
              /* TODO: add error return value to `grm_args_push_arg` (?) */
              grm_args_push_buf(current_args, key, format_string, ref, 1);
            }
        }
      else
        {
          if (current_args_array == NULL)
            {
              snprintf(format_string, SEND_REF_FORMAT_MAX_LENGTH, "%s:n%c,", key, format);
              error = grm_send(handle, format_string, len, ref);
            }
          else
            {
              snprintf(format_string, SEND_REF_FORMAT_MAX_LENGTH, "n%c", format);
              grm_args_push(current_args, key, format_string, len, ref);
            }
        }
    }
  else
    {
      static ArgsReflist *args_stack = NULL;
      static DynamicArgsArrayReflist *args_array_stack = NULL;
      static StringList *key_stack = NULL;
      /* handle special cases (strings, objects and arrays of objects) */
      switch (format)
        {
        case 's':
          if (current_args_array == NULL)
            {
              snprintf(format_string, SEND_REF_FORMAT_MAX_LENGTH, "%s:s,", key);
              error = grm_send(handle, format_string, ref);
            }
          else
            {
              grm_args_push(current_args, key, "s", ref);
            }
          break;
        case 'o':
          if (strchr(valid_opening_brackets, *(const char *)ref))
            {
              if (current_args_array == NULL)
                {
                  snprintf(format_string, SEND_REF_FORMAT_MAX_LENGTH, "%s:o(,", key);
                  grm_send(handle, format_string);
                }
              else
                {
                  if (args_stack == NULL)
                    {
                      args_stack = argsReflistNew();
                      if (args_stack == NULL)
                        {
                          error = GRM_ERROR_MALLOC;
                          break;
                        }
                    }
                  if (key_stack == NULL)
                    {
                      key_stack = stringListNew();
                      if (key_stack == NULL)
                        {
                          error = GRM_ERROR_MALLOC;
                          break;
                        }
                    }
                  if ((error = argsReflistPush(args_stack, current_args)) != GRM_ERROR_NONE) break;
                  if ((error = stringListPush(key_stack, key)) != GRM_ERROR_NONE) break;
                  current_args = grm_args_new();
                  if (current_args == NULL)
                    {
                      error = GRM_ERROR_MALLOC;
                      break;
                    }
                }
            }
          else if (strchr(valid_closing_brackets, *(const char *)ref))
            {
              if (current_args_array == NULL)
                {
                  grm_send(handle, ")");
                }
              else
                {
                  grm_args_t *previous_args = argsReflistPop(args_stack);
                  tmp_key = stringListPop(key_stack);
                  grm_args_push(previous_args, tmp_key, "a", current_args);
                  current_args = previous_args;
                  if (argsReflistEmpty(args_stack))
                    {
                      argsReflistDeleteWithEntries(args_stack);
                      args_stack = NULL;
                    }
                  if (stringListEmpty(key_stack))
                    {
                      stringListDelete(key_stack);
                      key_stack = NULL;
                    }
                }
            }
          break;
        case 'O':
          if (strchr(valid_opening_brackets, *(const char *)ref))
            {
              if (current_args_array != NULL)
                {
                  if (args_array_stack == NULL)
                    {
                      args_array_stack = dynamicArgsArrayReflistNew();
                      if (args_array_stack == NULL)
                        {
                          error = GRM_ERROR_MALLOC;
                          break;
                        }
                    }
                  if ((error = dynamicArgsArrayReflistPush(args_array_stack, current_args_array)) != GRM_ERROR_NONE)
                    break;
                }
              if (current_args != NULL)
                {
                  if (args_stack == NULL)
                    {
                      args_stack = argsReflistNew();
                      if (args_stack == NULL)
                        {
                          error = GRM_ERROR_MALLOC;
                          break;
                        }
                    }
                  if ((error = argsReflistPush(args_stack, current_args)) != GRM_ERROR_NONE) break;
                }
              if (key_stack == NULL)
                {
                  key_stack = stringListNew();
                  if (key_stack == NULL)
                    {
                      error = GRM_ERROR_MALLOC;
                      break;
                    }
                }
              if ((error = stringListPush(key_stack, key)) != GRM_ERROR_NONE) break;
              current_args_array = dynamicArgsArrayNew();
              if (current_args_array == NULL)
                {
                  error = GRM_ERROR_MALLOC;
                  break;
                }
              current_args = grm_args_new();
              if (current_args == NULL)
                {
                  error = GRM_ERROR_MALLOC;
                  break;
                }
              if ((error = dynamicArgsArrayPushBack(current_args_array, current_args)) != GRM_ERROR_NONE) break;
            }
          else if (strchr(valid_separator, *(const char *)ref))
            {
              current_args = grm_args_new();
              if (current_args == NULL)
                {
                  error = GRM_ERROR_MALLOC;
                  break;
                }
              assert(current_args_array != NULL);
              if ((error = dynamicArgsArrayPushBack(current_args_array, current_args)) != GRM_ERROR_NONE) break;
            }
          else if (strchr(valid_closing_brackets, *(const char *)ref))
            {
              assert(key_stack != NULL);
              tmp_key = stringListPop(key_stack);
              if (args_array_stack != NULL)
                {
                  current_args = argsReflistPop(args_stack);
                  grm_args_push(current_args, tmp_key, "nA", current_args_array->size, current_args_array->buf);
                  dynamicArgsArrayDelete(current_args_array);
                  current_args_array = dynamicArgsArrayReflistPop(args_array_stack);
                  if (dynamicArgsArrayReflistEmpty(args_array_stack))
                    {
                      dynamicArgsArrayReflistDeleteWithEntries(args_array_stack);
                      args_array_stack = NULL;
                    }
                }
              else
                {
                  snprintf(format_string, SEND_REF_FORMAT_MAX_LENGTH, "%s:nA,", tmp_key);
                  grm_send(handle, format_string, current_args_array->size, current_args_array->buf);
                  dynamicArgsArrayDeleteWithElements(current_args_array);
                  current_args_array = NULL;
                  current_args = NULL;
                }
              if (stringListEmpty(key_stack))
                {
                  stringListDelete(key_stack);
                  key_stack = NULL;
                }
            }
          break;
        case '\0':
          grm_send(handle, ")");
          break;
        default:
          break;
        }
    }

  free((void *)tmp_key);

  return error == GRM_ERROR_NONE;
}

int grm_send_args(const void *p, const grm_args_t *args)
{
  NetHandle *handle = (NetHandle *)p;
  grm_error_t error;

  error = toJsonWriteArgs(handle->sender_receiver.sender.memwriter, args);
  if (error == GRM_ERROR_NONE && toJsonIsComplete() && handle->sender_receiver.sender.send != NULL)
    {
      error = handle->sender_receiver.sender.send(handle);
    }

  return error == GRM_ERROR_NONE;
}
