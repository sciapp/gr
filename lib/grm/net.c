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
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
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

DECLARE_LIST_TYPE(args, grm_args_t *)


/* ------------------------- dynamic_args_array_list ---------------------------------------------------------------- */

DECLARE_LIST_TYPE(dynamic_args_array, dynamic_args_array_t *)

#undef DECLARE_LIST_TYPE


/* ========================= methods ================================================================================ */

/* ------------------------- args_list ------------------------------------------------------------------------------ */

DECLARE_LIST_METHODS(args)


/* ------------------------- dynamic_args_array_list ---------------------------------------------------------------- */

DECLARE_LIST_METHODS(dynamic_args_array)

#undef DECLARE_LIST_METHODS


/* ######################### private implementation ################################################################# */

/* ========================= methods ================================================================================ */

/* ------------------------- args_list ------------------------------------------------------------------------------ */

DEFINE_LIST_METHODS(args)

error_t args_list_entry_copy(args_list_entry_t *copy, args_list_const_entry_t entry)
{
  args_list_entry_t _copy;

  _copy = args_copy(entry);
  if (_copy == NULL)
    {
      return ERROR_MALLOC;
    }
  *copy = _copy;

  return NO_ERROR;
}

error_t args_list_entry_delete(args_list_entry_t entry)
{
  grm_args_delete(entry);
  return NO_ERROR;
}


/* ------------------------- dynamic_args_array_list ---------------------------------------------------------------- */

DEFINE_LIST_METHODS(dynamic_args_array)

error_t dynamic_args_array_list_entry_copy(dynamic_args_array_list_entry_t *copy,
                                           dynamic_args_array_list_const_entry_t entry)
{
  /* TODO: create a copy of the object! Otherwise code will segfault on list deletion for a non-ref list */
  *copy = (dynamic_args_array_list_entry_t)entry;
  return NO_ERROR;
}

error_t dynamic_args_array_list_entry_delete(dynamic_args_array_list_entry_t entry)
{
  dynamic_args_array_delete(entry);
  return NO_ERROR;
}


#undef DEFINE_LIST_METHODS

/* ######################### internal implementation ################################################################ */

/* ========================= methods ================================================================================ */

/* ------------------------- receiver ------------------------------------------------------------------------------- */

error_t receiver_init_for_custom(net_handle_t *handle, const char *name, unsigned int id,
                                 const char *(*custom_recv)(const char *, unsigned int))
{
  handle->sender_receiver.receiver.comm.custom.recv = custom_recv;
  handle->sender_receiver.receiver.comm.custom.name = name;
  handle->sender_receiver.receiver.comm.custom.id = id;
  handle->sender_receiver.receiver.message_size = 0;
  handle->sender_receiver.receiver.recv = receiver_recv_for_custom;
  handle->finalize = receiver_finalize_for_custom;
  handle->sender_receiver.receiver.memwriter = memwriter_new();
  if (handle->sender_receiver.receiver.memwriter == NULL)
    {
      return ERROR_MALLOC;
    }

  return NO_ERROR;
}

error_t receiver_init_for_socket(net_handle_t *handle, const char *hostname, unsigned int port)
{
  char port_str[PORT_MAX_STRING_LENGTH];
  struct addrinfo *addr_result = NULL, addr_hints;
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
  handle->sender_receiver.receiver.recv = receiver_recv_for_socket;
  handle->finalize = receiver_finalize_for_socket;

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
      debug_print_error(("winsock initialization failed: %S\n", message));
      LocalFree(message);
#endif
      return ERROR_NETWORK_WINSOCK_INIT;
    }
#endif

  memset(&addr_hints, 0, sizeof(addr_hints));
  addr_hints.ai_family = AF_UNSPEC;
  addr_hints.ai_socktype = SOCK_STREAM;
  addr_hints.ai_protocol = 0;
  addr_hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;

  /* Query a list of ip addresses for the given hostname */
  if ((error = getaddrinfo(hostname, port_str, &addr_hints, &addr_result)) != 0)
    {
#ifdef _WIN32
      psocketerror("getaddrinfo failed with error");
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
      return ERROR_NETWORK_HOSTNAME_RESOLUTION;
    }

  /* Create a socket for listening */
  if ((handle->sender_receiver.receiver.comm.socket.server_socket =
           socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol)) < 0)
    {
      psocketerror("socket creation failed");
      freeaddrinfo(addr_result);
      return ERROR_NETWORK_SOCKET_CREATION;
    }
    /* Set SO_REUSEADDR if available on this system */
#ifdef SO_REUSEADDR
  socket_opt = 1;
  if (setsockopt(handle->sender_receiver.receiver.comm.socket.server_socket, SOL_SOCKET, SO_REUSEADDR,
                 (char *)&socket_opt, sizeof(socket_opt)) < 0)
    {
      psocketerror("setting socket options failed");
      freeaddrinfo(addr_result);
      return ERROR_NETWORK_SOCKET_CREATION;
    }
#endif

  /* Bind the socket to given ip address and port */
  if (bind(handle->sender_receiver.receiver.comm.socket.server_socket, addr_result->ai_addr, addr_result->ai_addrlen))
    {
      psocketerror("bind failed");
      freeaddrinfo(addr_result);
      return ERROR_NETWORK_SOCKET_BIND;
    }
  freeaddrinfo(addr_result);

  /* Listen for incoming connections */
  if (listen(handle->sender_receiver.receiver.comm.socket.server_socket, 1))
    {
      psocketerror("listen failed");
      return ERROR_NETWORK_SOCKET_LISTEN;
    }

  /* Accecpt an incoming connection and get a new socket instance for communication */
  if ((handle->sender_receiver.receiver.comm.socket.client_socket =
           accept(handle->sender_receiver.receiver.comm.socket.server_socket, (struct sockaddr *)&client_addr,
                  &client_addrlen)) < 0)
    {
      psocketerror("accept failed");
      return ERROR_NETWORK_CONNECTION_ACCEPT;
    }

  handle->sender_receiver.receiver.memwriter = memwriter_new();
  if (handle->sender_receiver.receiver.memwriter == NULL)
    {
      return ERROR_MALLOC;
    }

  return NO_ERROR;
}

error_t receiver_finalize_for_custom(net_handle_t *handle)
{
  memwriter_delete(handle->sender_receiver.receiver.memwriter);

  return NO_ERROR;
}

error_t receiver_finalize_for_socket(net_handle_t *handle)
{
  error_t error = NO_ERROR;

  memwriter_delete(handle->sender_receiver.receiver.memwriter);
#ifdef _WIN32
  if (handle->sender_receiver.receiver.comm.socket.client_socket >= 0)
    {
      if (closesocket(handle->sender_receiver.receiver.comm.socket.client_socket))
        {
          psocketerror("client socket shutdown failed");
          error = ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
  if (handle->sender_receiver.receiver.comm.socket.server_socket >= 0)
    {
      if (closesocket(handle->sender_receiver.receiver.comm.socket.server_socket))
        {
          psocketerror("server socket shutdown failed");
          error = ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
  if (WSACleanup())
    {
      psocketerror("winsock shutdown failed");
      error = ERROR_NETWORK_WINSOCK_CLEANUP;
    }
#else
  if (handle->sender_receiver.receiver.comm.socket.client_socket >= 0)
    {
      if (close(handle->sender_receiver.receiver.comm.socket.client_socket))
        {
          psocketerror("client socket shutdown failed");
          error = ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
  if (handle->sender_receiver.receiver.comm.socket.server_socket >= 0)
    {
      if (close(handle->sender_receiver.receiver.comm.socket.server_socket))
        {
          psocketerror("server socket shutdown failed");
          error = ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
#endif

  return error;
}

error_t receiver_recv_for_socket(net_handle_t *handle)
{
  int search_start_index = 0;
  char *end_ptr;
  static char recv_buf[SOCKET_RECV_BUF_SIZE];
  error_t error = NO_ERROR;

  while ((end_ptr = memchr(memwriter_buf(handle->sender_receiver.receiver.memwriter) + search_start_index, ETB,
                           memwriter_size(handle->sender_receiver.receiver.memwriter) - search_start_index)) == NULL)
    {
      int bytes_received;
      search_start_index = memwriter_size(handle->sender_receiver.receiver.memwriter);
      bytes_received =
          recv(handle->sender_receiver.receiver.comm.socket.client_socket, recv_buf, SOCKET_RECV_BUF_SIZE, 0);
      if (bytes_received < 0)
        {
          psocketerror("error while receiving data");
          return ERROR_NETWORK_RECV;
        }
      else if (bytes_received == 0)
        {
          return ERROR_NETWORK_RECV_CONNECTION_SHUTDOWN;
        }
      if ((error = memwriter_printf(handle->sender_receiver.receiver.memwriter, "%.*s", bytes_received, recv_buf)) !=
          NO_ERROR)
        {
          return error;
        }
    }
  *end_ptr = '\0';
  handle->sender_receiver.receiver.message_size = end_ptr - memwriter_buf(handle->sender_receiver.receiver.memwriter);

  return error;
}

error_t receiver_recv_for_custom(net_handle_t *handle)
{
  /* TODO: is it really necessary to copy the memory? */
  const char *recv_buf;
  error_t error = NO_ERROR;

  recv_buf = handle->sender_receiver.receiver.comm.custom.recv(handle->sender_receiver.receiver.comm.custom.name,
                                                               handle->sender_receiver.receiver.comm.custom.id);
  if (recv_buf == NULL)
    {
      return ERROR_CUSTOM_RECV;
    }
  memwriter_clear(handle->sender_receiver.receiver.memwriter);
  if ((error = memwriter_puts(handle->sender_receiver.receiver.memwriter, recv_buf)) != NO_ERROR)
    {
      return error;
    }
  handle->sender_receiver.receiver.message_size = memwriter_size(handle->sender_receiver.receiver.memwriter);

  return error;
}


/* ------------------------- sender --------------------------------------------------------------------------------- */

error_t sender_init_for_custom(net_handle_t *handle, const char *name, unsigned int id,
                               int (*custom_send)(const char *, unsigned int, const char *))
{
  handle->sender_receiver.sender.comm.custom.send = custom_send;
  handle->sender_receiver.sender.comm.custom.name = name;
  handle->sender_receiver.sender.comm.custom.id = id;
  handle->sender_receiver.sender.send = sender_send_for_custom;
  handle->finalize = sender_finalize_for_custom;
  handle->sender_receiver.sender.memwriter = memwriter_new();
  if (handle->sender_receiver.sender.memwriter == NULL)
    {
      return ERROR_MALLOC;
    }

  return NO_ERROR;
}

error_t sender_init_for_socket(net_handle_t *handle, const char *hostname, unsigned int port)
{
  char port_str[PORT_MAX_STRING_LENGTH];
  struct addrinfo *addr_result = NULL, *addr_ptr = NULL, addr_hints;
  int error;
#ifdef _WIN32
  int wsa_startup_error = 0;
  WSADATA wsa_data;
#endif

  snprintf(port_str, PORT_MAX_STRING_LENGTH, "%u", port);

  handle->sender_receiver.sender.memwriter = NULL;
  handle->sender_receiver.sender.comm.socket.client_socket = -1;
  handle->sender_receiver.sender.send = sender_send_for_socket;
  handle->finalize = sender_finalize_for_socket;

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
      debug_print_error(("winsock initialization failed: %S\n", message));
      LocalFree(message);
#endif
      return ERROR_NETWORK_WINSOCK_INIT;
    }
#endif

  memset(&addr_hints, 0, sizeof(addr_hints));
  addr_hints.ai_family = AF_UNSPEC;
  addr_hints.ai_socktype = SOCK_STREAM;
  addr_hints.ai_protocol = IPPROTO_TCP;

  /* Query a list of ip addresses for the given hostname */
  if ((error = getaddrinfo(hostname, port_str, &addr_hints, &addr_result)) != 0)
    {
#ifdef _WIN32
      psocketerror("getaddrinfo failed with error");
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
      return ERROR_NETWORK_HOSTNAME_RESOLUTION;
    }

  /* Attempt to connect to an address until one succeeds */
  handle->sender_receiver.sender.comm.socket.client_socket = -1;
  for (addr_ptr = addr_result; addr_ptr != NULL && handle->sender_receiver.sender.comm.socket.client_socket < 0;
       addr_ptr = addr_ptr->ai_next)
    {
      /* Create a socket for connecting to server */
      handle->sender_receiver.sender.comm.socket.client_socket =
          socket(addr_ptr->ai_family, addr_ptr->ai_socktype, addr_ptr->ai_protocol);
      if (handle->sender_receiver.sender.comm.socket.client_socket < 0)
        {
          psocketerror("socket creation failed");
          return ERROR_NETWORK_SOCKET_CREATION;
        }
      /* Connect to server */
      if (connect(handle->sender_receiver.sender.comm.socket.client_socket, addr_ptr->ai_addr,
                  (int)addr_ptr->ai_addrlen))
        {
#ifdef _WIN32
          closesocket(handle->sender_receiver.sender.comm.socket.client_socket);
#else
          close(handle->sender_receiver.sender.comm.socket.client_socket);
#endif
          handle->sender_receiver.sender.comm.socket.client_socket = -1;
        }
    }
  freeaddrinfo(addr_result);

  if (handle->sender_receiver.sender.comm.socket.client_socket < 0)
    {
      fprintf(stderr, "cannot connect to host %s port %u: ", hostname, port);
      psocketerror("");
      return ERROR_NETWORK_CONNECT;
    }

  handle->sender_receiver.sender.memwriter = memwriter_new();
  if (handle->sender_receiver.sender.memwriter == NULL)
    {
      return ERROR_MALLOC;
    }

  return NO_ERROR;
}

error_t sender_finalize_for_custom(net_handle_t *handle)
{
  memwriter_delete(handle->sender_receiver.sender.memwriter);

  return NO_ERROR;
}

error_t sender_finalize_for_socket(net_handle_t *handle)
{
  error_t error = NO_ERROR;

  memwriter_delete(handle->sender_receiver.sender.memwriter);
#ifdef _WIN32
  if (handle->sender_receiver.sender.comm.socket.client_socket >= 0)
    {
      if (closesocket(handle->sender_receiver.sender.comm.socket.client_socket))
        {
          psocketerror("client socket shutdown failed");
          error = ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
  if (WSACleanup())
    {
      psocketerror("winsock shutdown failed");
      error = ERROR_NETWORK_WINSOCK_CLEANUP;
    }
#else
  if (handle->sender_receiver.sender.comm.socket.client_socket >= 0)
    {
      if (close(handle->sender_receiver.sender.comm.socket.client_socket))
        {
          psocketerror("client socket shutdown failed");
          error = ERROR_NETWORK_SOCKET_CLOSE;
        }
    }
#endif

  return error;
}

error_t sender_send_for_socket(net_handle_t *handle)
{
  const char *buf, *send_ptr;
  size_t buf_size;
  int bytes_left;
  error_t error = NO_ERROR;

  if ((error = memwriter_putc(handle->sender_receiver.sender.memwriter, ETB)) != NO_ERROR)
    {
      return error;
    }

  buf = memwriter_buf(handle->sender_receiver.sender.memwriter);
  buf_size = memwriter_size(handle->sender_receiver.sender.memwriter);

  send_ptr = buf;
  bytes_left = buf_size;
  while (bytes_left)
    {
      int bytes_sent = send(handle->sender_receiver.sender.comm.socket.client_socket, buf, bytes_left, 0);
      if (bytes_sent < 0)
        {
          psocketerror("could not send any data");
          return ERROR_NETWORK_SEND;
        }
      send_ptr += bytes_sent;
      bytes_left -= bytes_sent;
    }

  memwriter_clear(handle->sender_receiver.sender.memwriter);

  return error;
}

error_t sender_send_for_custom(net_handle_t *handle)
{
  const char *buf;
  error_t error = NO_ERROR;

  buf = memwriter_buf(handle->sender_receiver.sender.memwriter);
  if (!handle->sender_receiver.sender.comm.custom.send(handle->sender_receiver.sender.comm.custom.name,
                                                       handle->sender_receiver.sender.comm.custom.id, buf))
    {
      error = ERROR_CUSTOM_SEND;
      return error;
    }
  memwriter_clear(handle->sender_receiver.sender.memwriter);

  return error;
}


/* ######################### public implementation ################################################################## */

/* ========================= methods ================================================================================ */

/* ------------------------- receiver / sender ---------------------------------------------------------------------- */

void *grm_open(int is_receiver, const char *name, unsigned int id,
               const char *(*custom_recv)(const char *, unsigned int),
               int (*custom_send)(const char *, unsigned int, const char *))
{
  net_handle_t *handle;
  error_t error = NO_ERROR;

  handle = malloc(sizeof(net_handle_t));
  if (handle == NULL)
    {
      return NULL;
    }
  handle->is_receiver = is_receiver;
  handle->sender_receiver.receiver.comm.custom.recv = custom_recv;
  if (is_receiver)
    {
      if (custom_recv != NULL)
        {
          error = receiver_init_for_custom(handle, name, id, custom_recv);
        }
      else
        {
          error = receiver_init_for_socket(handle, name, id);
        }
    }
  else
    {
      if (custom_send != NULL)
        {
          error = sender_init_for_custom(handle, name, id, custom_send);
        }
      else
        {
          error = sender_init_for_socket(handle, name, id);
        }
    }

  if (error != NO_ERROR)
    {
      if (error != ERROR_NETWORK_WINSOCK_INIT)
        {
          handle->finalize(handle);
        }
      free(handle);
      handle = NULL;
    }

  return (void *)handle;
}

void grm_close(const void *p)
{
  net_handle_t *handle = (net_handle_t *)p;

  handle->finalize(handle);
  free(handle);
}


/* ------------------------- receiver ------------------------------------------------------------------------------- */

grm_args_t *grm_recv(const void *p, grm_args_t *args)
{
  net_handle_t *handle = (net_handle_t *)p;
  int created_args = 0;

  if (args == NULL)
    {
      args = grm_args_new();
      if (args == NULL)
        {
          goto error_cleanup;
        }
      created_args = 1;
    }

  if (handle->sender_receiver.receiver.recv(handle) != NO_ERROR)
    {
      goto error_cleanup;
    }
  if (fromjson_read(args, memwriter_buf(handle->sender_receiver.receiver.memwriter)) != NO_ERROR)
    {
      goto error_cleanup;
    }

  if (memwriter_erase(handle->sender_receiver.receiver.memwriter, 0,
                      handle->sender_receiver.receiver.message_size + 1) != NO_ERROR)
    {
      goto error_cleanup;
    }

  return args;

error_cleanup:
  if (created_args)
    {
      grm_args_delete(args);
    }

  return NULL;
}


/* ------------------------- sender --------------------------------------------------------------------------------- */

int grm_send(const void *p, const char *data_desc, ...)
{
  net_handle_t *handle = (net_handle_t *)p;
  va_list vl;
  error_t error;

  va_start(vl, data_desc);
  error = tojson_write_vl(handle->sender_receiver.sender.memwriter, data_desc, &vl);
  if (error == NO_ERROR && tojson_is_complete() && handle->sender_receiver.sender.send != NULL)
    {
      error = handle->sender_receiver.sender.send(handle);
    }
  va_end(vl);

  return error == NO_ERROR;
}

int grm_send_buf(const void *p, const char *data_desc, const void *buffer, int apply_padding)
{
  net_handle_t *handle = (net_handle_t *)p;
  error_t error;

  error = tojson_write_buf(handle->sender_receiver.sender.memwriter, data_desc, buffer, apply_padding);
  if (error == NO_ERROR && tojson_is_complete() && handle->sender_receiver.sender.send != NULL)
    {
      error = handle->sender_receiver.sender.send(handle);
    }

  return error == NO_ERROR;
}

int grm_send_ref(const void *p, const char *key, char format, const void *ref, int len)
{
  static const char VALID_OPENING_BRACKETS[] = "([{";
  static const char VALID_CLOSING_BRACKETS[] = ")]}";
  static const char VALID_SEPARATOR[] = ",";
  static grm_args_t *current_args = NULL;
  static dynamic_args_array_t *current_args_array = NULL;
  char *_key = NULL;
  net_handle_t *handle = (net_handle_t *)p;
  char format_string[SEND_REF_FORMAT_MAX_LENGTH];
  error_t error = NO_ERROR;

  if (tojson_struct_nested_level() == 0)
    {
      grm_send(handle, "o(");
    }
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
      static args_reflist_t *args_stack = NULL;
      static dynamic_args_array_reflist_t *args_array_stack = NULL;
      static string_list_t *key_stack = NULL;
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
          if (strchr(VALID_OPENING_BRACKETS, *(const char *)ref))
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
                      args_stack = args_reflist_new();
                      if (args_stack == NULL)
                        {
                          error = ERROR_MALLOC;
                          break;
                        }
                    }
                  if (key_stack == NULL)
                    {
                      key_stack = string_list_new();
                      if (key_stack == NULL)
                        {
                          error = ERROR_MALLOC;
                          break;
                        }
                    }
                  if ((error = args_reflist_push(args_stack, current_args)) != NO_ERROR)
                    {
                      break;
                    }
                  if ((error = string_list_push(key_stack, key)) != NO_ERROR)
                    {
                      break;
                    }
                  current_args = grm_args_new();
                  if (current_args == NULL)
                    {
                      error = ERROR_MALLOC;
                      break;
                    }
                }
            }
          else if (strchr(VALID_CLOSING_BRACKETS, *(const char *)ref))
            {
              if (current_args_array == NULL)
                {
                  grm_send(handle, ")");
                }
              else
                {
                  grm_args_t *previous_args = args_reflist_pop(args_stack);
                  _key = string_list_pop(key_stack);
                  grm_args_push(previous_args, _key, "a", current_args);
                  current_args = previous_args;
                  if (args_reflist_empty(args_stack))
                    {
                      args_reflist_delete_with_entries(args_stack);
                      args_stack = NULL;
                    }
                  if (string_list_empty(key_stack))
                    {
                      string_list_delete(key_stack);
                      key_stack = NULL;
                    }
                }
            }
          break;
        case 'O':
          if (strchr(VALID_OPENING_BRACKETS, *(const char *)ref))
            {
              if (current_args_array != NULL)
                {
                  if (args_array_stack == NULL)
                    {
                      args_array_stack = dynamic_args_array_reflist_new();
                      if (args_array_stack == NULL)
                        {
                          error = ERROR_MALLOC;
                          break;
                        }
                    }
                  if ((error = dynamic_args_array_reflist_push(args_array_stack, current_args_array)) != NO_ERROR)
                    {
                      break;
                    }
                }
              if (current_args != NULL)
                {
                  if (args_stack == NULL)
                    {
                      args_stack = args_reflist_new();
                      if (args_stack == NULL)
                        {
                          error = ERROR_MALLOC;
                          break;
                        }
                    }
                  if ((error = args_reflist_push(args_stack, current_args)) != NO_ERROR)
                    {
                      break;
                    }
                }
              if (key_stack == NULL)
                {
                  key_stack = string_list_new();
                  if (key_stack == NULL)
                    {
                      error = ERROR_MALLOC;
                      break;
                    }
                }
              if ((error = string_list_push(key_stack, key)) != NO_ERROR)
                {
                  break;
                }
              current_args_array = dynamic_args_array_new();
              if (current_args_array == NULL)
                {
                  error = ERROR_MALLOC;
                  break;
                }
              current_args = grm_args_new();
              if (current_args == NULL)
                {
                  error = ERROR_MALLOC;
                  break;
                }
              if ((error = dynamic_args_array_push_back(current_args_array, current_args)) != NO_ERROR)
                {
                  break;
                }
            }
          else if (strchr(VALID_SEPARATOR, *(const char *)ref))
            {
              current_args = grm_args_new();
              if (current_args == NULL)
                {
                  error = ERROR_MALLOC;
                  break;
                }
              assert(current_args_array != NULL);
              if ((error = dynamic_args_array_push_back(current_args_array, current_args)) != NO_ERROR)
                {
                  break;
                }
            }
          else if (strchr(VALID_CLOSING_BRACKETS, *(const char *)ref))
            {
              assert(key_stack != NULL);
              _key = string_list_pop(key_stack);
              if (args_array_stack != NULL)
                {
                  current_args = args_reflist_pop(args_stack);
                  grm_args_push(current_args, _key, "nA", current_args_array->size, current_args_array->buf);
                  dynamic_args_array_delete(current_args_array);
                  current_args_array = dynamic_args_array_reflist_pop(args_array_stack);
                  if (dynamic_args_array_reflist_empty(args_array_stack))
                    {
                      dynamic_args_array_reflist_delete_with_entries(args_array_stack);
                      args_array_stack = NULL;
                    }
                }
              else
                {
                  snprintf(format_string, SEND_REF_FORMAT_MAX_LENGTH, "%s:nA,", _key);
                  grm_send(handle, format_string, current_args_array->size, current_args_array->buf);
                  dynamic_args_array_delete_with_elements(current_args_array);
                  current_args_array = NULL;
                  current_args = NULL;
                }
              if (string_list_empty(key_stack))
                {
                  string_list_delete(key_stack);
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

  free((void *)_key);

  return error == NO_ERROR;
}

int grm_send_args(const void *p, const grm_args_t *args)
{
  net_handle_t *handle = (net_handle_t *)p;
  error_t error;

  error = tojson_write_args(handle->sender_receiver.sender.memwriter, args);
  if (error == NO_ERROR && tojson_is_complete() && handle->sender_receiver.sender.send != NULL)
    {
      error = handle->sender_receiver.sender.send(handle);
    }

  return error == NO_ERROR;
}
