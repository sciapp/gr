#ifdef __unix__
#define _XOPEN_SOURCE 500
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include "gr.h"

#define MEMWRITER_INITIAL_SIZE 32768
#define MEMWRITER_SIZE_INCREMENT 32768

#define ETB '\027'

typedef struct {
  char *buf;
  size_t size;
  size_t capacity;
} memwriter_t;

typedef int (*post_serialize_callback_t)(void *handle);
typedef int (*jupyter_send_callback_t)(const char *);

typedef struct {
  int target;
  memwriter_t *memwriter;
  post_serialize_callback_t post_serialize;
  union {
    struct {
      /* callback to a function that handles jupyter communication */
      jupyter_send_callback_t send;
    } jupyter;
    struct {
      int client_socket_fd;
      struct sockaddr_in server_address;
    } socket;
  };
} metahandle_t;

enum {
  member_name, data_type
};

typedef enum {
  complete = 1, incomplete, incomplete_at_struct_beginning
  /* 0 is unknown / not set */
} serialization_result_t;

typedef struct {
  int apply_padding;
  int array_length;
  void *data_ptr;
  va_list *vl;
  int data_offset;
  int wrote_output;
  int add_data;
  int add_data_without_separator;
  serialization_result_t serial_result;
  unsigned int struct_nested_level;
} shared_state_t;

typedef struct {
  int next_is_ptr;
  char *data_type_ptr;
  char current_data_type;
  char *additional_type_info;
  int is_type_info_incomplete;
  shared_state_t *shared;
} state_t;

typedef struct {
  serialization_result_t serial_result;
  unsigned int struct_nested_level;
} permanent_state_t;

static int post_serialize_socket(void *handle);
static int post_serialize_jupyter(void *handle);

static permanent_state_t permanent_state = {complete, 0};

memwriter_t *memwriter_new() {
  memwriter_t *memwriter;

  memwriter = malloc(sizeof(memwriter_t));
  if (memwriter == NULL) {
    fprintf(stderr, "out of virtual memory\n");
    return NULL;
  }
  memwriter->buf = malloc(MEMWRITER_INITIAL_SIZE);
  if (memwriter->buf == NULL) {
    free(memwriter);
    fprintf(stderr, "out of virtual memory\n");
    return NULL;
  }
  memwriter->size = 0;
  memwriter->capacity = MEMWRITER_INITIAL_SIZE;

  return memwriter;
}

void memwriter_delete(memwriter_t *memwriter) {
  if (memwriter != NULL) {
    free(memwriter->buf);
    free(memwriter);
  }
}

void memwriter_clear(memwriter_t *memwriter) {
  memwriter->size = 0;
  *memwriter->buf = '\0';
}

int memwriter_enlarge_buf(memwriter_t *memwriter) {
  void *new_buf;

  new_buf = realloc(memwriter->buf, memwriter->capacity + MEMWRITER_SIZE_INCREMENT);
  if (new_buf == NULL) {
    fprintf(stderr, "out of virtual memory\n");
    return 0;
  }
  memwriter->buf = new_buf;
  memwriter->capacity += MEMWRITER_SIZE_INCREMENT;

  return 1;
}

int memwriter_printf(memwriter_t *memwriter, const char *format, ...) {
  va_list vl;
  int chars_needed;
  int was_successful;

  was_successful = 0;
  while (1) {
    va_start(vl, format);
    chars_needed = vsnprintf(&memwriter->buf[memwriter->size], memwriter->capacity - memwriter->size, format, vl);
    va_end(vl);
    if (chars_needed < (int) (memwriter->capacity - memwriter->size)) {
      memwriter->size += chars_needed;
      was_successful = 1;
      break;
    }
    if (!memwriter_enlarge_buf(memwriter)) {
      break;
    }
  }

  return was_successful;
}

int memwriter_puts(memwriter_t *memwriter, const char *s) {
  return memwriter_printf(memwriter, "%s", s);
}

int memwriter_putc(memwriter_t *memwriter, char c) {
  return memwriter_printf(memwriter, "%c", c);
}

char *memwriter_buf(const memwriter_t *memwriter) {
  return memwriter->buf;
}

size_t memwriter_size(const memwriter_t *memwriter) {
  return memwriter->size;
}

/* TODO: better error handling for the json routines */

#define CHECK_PADDING(type)						\
  do {									\
    if (state->shared->data_ptr != NULL && state->shared->apply_padding) { \
      int needed_padding = state->shared->data_offset % sizeof(type);	\
      state->shared->data_ptr = ((char *) state->shared->data_ptr) + needed_padding; \
      state->shared->data_offset += needed_padding;			\
    }									\
  } while (0)

#define RETRIEVE_SINGLE_VALUE(var, type, promoted_type)                 \
  do {                                                                  \
    CHECK_PADDING(type);                                                \
    if (state->shared->data_ptr != NULL) {                              \
      var = *((type *)state->shared->data_ptr);                         \
      state->shared->data_ptr = ((type *)state->shared->data_ptr) + 1;  \
      state->shared->data_offset += sizeof(type);                       \
    }                                                                   \
    else {                                                              \
      var = va_arg(*state->shared->vl, promoted_type);                  \
    }                                                                   \
  } while (0)

#define INIT_MULTI_VALUE(vars, type)                    \
  do {                                                  \
    if (state->shared->data_ptr != NULL) {              \
      if (state->next_is_ptr) {                         \
        CHECK_PADDING(type *);                          \
        vars = *(type **)state->shared->data_ptr;       \
      }                                                 \
      else {                                            \
        CHECK_PADDING(type);                            \
        vars = (type *)state->shared->data_ptr;         \
      }                                                 \
    }                                                   \
    else {                                              \
      if (state->next_is_ptr) {                         \
        vars = va_arg(*state->shared->vl, type *);      \
      }                                                 \
    }                                                   \
  } while (0)

#define RETRIEVE_NEXT_VALUE(var, vars, type)    \
  do {                                          \
    if (state->shared->data_ptr != NULL) {      \
      var = *vars++;                            \
    }                                           \
    else {                                      \
      if (state->next_is_ptr) {                 \
        var = *vars++;                          \
      }                                         \
      else {                                    \
        var = va_arg(*state->shared->vl, type); \
      }                                         \
    }                                           \
  } while (0)

#define FIN_MULTI_VALUE(type)						\
  do {									\
    if (state->shared->data_ptr != NULL) {				\
      if (state->next_is_ptr) {						\
        state->shared->data_ptr = ((type **)state->shared->data_ptr) + 1; \
        state->shared->data_offset += sizeof(type *);			\
      }									\
      else {								\
        state->shared->data_ptr = ((type *)state->shared->data_ptr) + length; \
        state->shared->data_offset += length * sizeof(type);		\
      }									\
    }									\
  } while (0)

static
int serialize(memwriter_t *memwriter, char *data_desc, void *data, va_list *vl,
              int apply_padding, int add_data, int add_data_without_separator,
              unsigned int *struct_nested_level, serialization_result_t *serial_result,
              shared_state_t *shared_state);

static
int get_member_count(const char *data_desc) {
  int nested_level = 0;
  int member_count = 0;
  if (*data_desc == 0) return member_count;
  while (*data_desc != 0) {
    switch (*data_desc) {
    case '(':
      ++nested_level;
      break;
    case ')':
      --nested_level;
      break;
    case ',':
      ++member_count;
      break;
    default:
      break;
    }
    ++data_desc;
  }
  ++member_count;     /* add last member (because it is not terminated by a ',') */
  return member_count;
}

static
int is_json_array_needed(const char *data_desc) {
  const char *relevant_data_types = "iIdDcCs";
  int nested_level = 0;
  int count_relevant_data_types = 0;

  while (*data_desc != 0 && count_relevant_data_types < 2) {
    switch (*data_desc) {
    case '(':
      ++nested_level;
      break;
    case ')':
      --nested_level;
      break;
    default:
      if (nested_level == 0) {
        if (strchr(relevant_data_types, *data_desc)) {
          ++count_relevant_data_types;
        }
      }
      break;
    }
    ++data_desc;
  }
  return count_relevant_data_types >= 2;
}

static
int read_data_type(state_t *state) {
  int nested_level = 0;
  char *additional_type_info = NULL;
  state->current_data_type = *state->data_type_ptr;
  ++(state->data_type_ptr);
  if (*state->data_type_ptr == '(') {
    ++nested_level;
    additional_type_info = ++(state->data_type_ptr);
    while (*state->data_type_ptr != 0 && nested_level > 0) {
      switch (*state->data_type_ptr) {
      case '(':
        ++nested_level;
        break;
      case ')':
        --nested_level;
        break;
      }
      if (nested_level > 0) {
        ++(state->data_type_ptr);
      }
    }
    if (*state->data_type_ptr != 0) {
      *(state->data_type_ptr)++ = 0;  /* termination character for additional_type_info */
      state->is_type_info_incomplete = 0;
    }
    else {
      state->is_type_info_incomplete = 1; /* character search hit '\0' and not ')' */
    }
  }
  state->additional_type_info = additional_type_info;
  return 0;
}

static
int read_next_is_ptr(state_t *state) {
  state->next_is_ptr = 1;
  return 0;
}

static
int skip_bytes(state_t *state) {
  int count;

  count = (state->additional_type_info != NULL) ? atoi(state->additional_type_info) : 1;
  if (state->shared->data_ptr != NULL) {
    state->shared->data_ptr = ((char *) state->shared->data_ptr) + count;
    state->shared->data_offset += count;
  }
  else {
    fprintf(stderr,
	    "Skipping bytes is not supported when using the variable argument list " \
            "and is ignored.\n");
  }
  return 0;
}

static
int close_object(memwriter_t *memwriter, state_t *state) {
  --(state->shared->struct_nested_level);
  memwriter_putc(memwriter, '}');
  return 0;
}

static
int read_array_length(state_t *state) {
  int value;

  RETRIEVE_SINGLE_VALUE(value, int, int);
  state->shared->array_length = value;
  return 0;
}

#define DEF_STRINGIFY_SINGLE(type, promoted_type, format_specifier)	\
  static int stringify_ ## type(memwriter_t *memwriter, state_t *state) { \
    type value;								\
    RETRIEVE_SINGLE_VALUE(value, type, promoted_type);			\
    memwriter_printf(memwriter, #format_specifier, value);		\
    state->shared->wrote_output = 1;					\
    return 0;								\
  }

#define DEF_STRINGIFY_MULTI(type, format_specifier)                     \
  static int stringify_ ## type ## _array(memwriter_t *memwriter, state_t *state) { \
    type *values;                                                       \
    type current_value;                                                 \
    int length;                                                         \
    int remaining_elements;                                             \
    INIT_MULTI_VALUE(values, type);                                     \
    length = (state->additional_type_info != NULL) ? atoi(state->additional_type_info) : state->shared->array_length; \
    remaining_elements = length;                                        \
    /* write array start */                                             \
    memwriter_putc(memwriter, '[');                                     \
    /* write array content */                                           \
    while (remaining_elements) {                                        \
      RETRIEVE_NEXT_VALUE(current_value, values, type);                 \
      memwriter_printf(memwriter, #format_specifier "%s", current_value, ((remaining_elements > 1) ? "," : "")); \
      --remaining_elements;                                             \
    }                                                                   \
    /* write array end */                                               \
    memwriter_putc(memwriter, ']');                                     \
    FIN_MULTI_VALUE(type);                                              \
    state->shared->wrote_output = 1;                                    \
    state->next_is_ptr = 0;                                             \
    return 0;                                                           \
  }

DEF_STRINGIFY_SINGLE(int, int, %d)
DEF_STRINGIFY_MULTI(int, %d)

DEF_STRINGIFY_SINGLE(double, double, %f)
DEF_STRINGIFY_MULTI(double, %f)

DEF_STRINGIFY_SINGLE(char, int, %c)

static
int stringify_char_array(memwriter_t *memwriter, state_t *state) {
  char *chars;
  int length;

  if (state->shared->data_ptr == NULL && !state->next_is_ptr) {
    fprintf(stderr, "Serializing strings from single chars is not supported"
            "when using variable argument lists.");
    return -1;
  }

  if (state->next_is_ptr) {
    if (state->shared->data_ptr != NULL) {
      CHECK_PADDING(char*);
      chars = *(char **)state->shared->data_ptr;
    }
    else {
      chars = va_arg(*state->shared->vl, char*);
    }
  }
  else {
    CHECK_PADDING(char);
    chars = (char *)state->shared->data_ptr;
  }

  length = (state->additional_type_info != NULL) ? atoi(state->additional_type_info) : state->shared->array_length;
  memwriter_printf(memwriter, "\"%s\"", chars);

  FIN_MULTI_VALUE(char);
  state->shared->wrote_output = 1;
  state->next_is_ptr = 0;
  return 0;
}

static
int stringify_bool(memwriter_t *memwriter, state_t *state) {
  int value;
  RETRIEVE_SINGLE_VALUE(value, int, int);
  memwriter_puts(memwriter, value ? "true" : "false");
  state->shared->wrote_output = 1;
  return 0;
}

static
int unzip_member_names_and_data_types(char *mixed_ptr, char ***member_name_ptr, char ***data_type_ptr) {
  int member_count;
  char **arrays[2];
  char separators[2];
  int current_array_index;
  int nested_type_level;

  member_count = get_member_count(mixed_ptr);
  /* add 1 to member count for a terminatory NULL pointer */
  /* TODO: check malloc calls */
  *member_name_ptr = malloc((member_count + 1) * sizeof(char **));
  *data_type_ptr = malloc((member_count + 1) * sizeof(char **));
  arrays[member_name] = *member_name_ptr;
  arrays[data_type] = *data_type_ptr;
  if (member_count > 0) {
    separators[0] = ':';
    separators[1] = ',';
    current_array_index = member_name;
    *arrays[current_array_index]++ = mixed_ptr;

    nested_type_level = 0;
    /* iterate over the whole type list */
    while (nested_type_level >= 0 && *mixed_ptr != 0) {
      /* extract one name or one type */
      while (*mixed_ptr != 0 && (nested_type_level > 0 || *mixed_ptr != separators[current_array_index])) {
        if (current_array_index == data_type) {
          switch (*mixed_ptr) {
          case '(':
            ++nested_type_level;
            break;
          case ')':
            --nested_type_level;
            break;
          }
        }
        if (nested_type_level >= 0) {
          ++mixed_ptr;
        }
      }
      if (*mixed_ptr != 0) {
        *mixed_ptr++ = 0;    /* terminate string in buffer */
        current_array_index = 1 - current_array_index;  /* alternate between member_name (0) and data_type (1) */
        *arrays[current_array_index]++ = mixed_ptr;
      }
    }
  }
  *arrays[member_name] = NULL;
  *arrays[data_type] = NULL;

  return 0;
}

static
int stringify_struct(memwriter_t *memwriter, state_t *state) {
  char **member_names;
  char **data_types;
  char **member_name_ptr;
  char **data_type_ptr;
  int has_members;
  int serialized_all_members;

  /* IMPORTANT: additional_type_info is altered after the unzip call! */
  unzip_member_names_and_data_types(state->additional_type_info, &member_names, &data_types);
  member_name_ptr = member_names;
  data_type_ptr = data_types;

  has_members = (*member_name_ptr != NULL && *data_type_ptr != NULL);
  /* write object start */
  if (!state->shared->add_data_without_separator) {
    if (state->shared->add_data && has_members) {
      memwriter_putc(memwriter, ',');
      state->shared->add_data = 0;
    }
    else  if (!state->shared->add_data) {
      memwriter_putc(memwriter, '{');
      ++(state->shared->struct_nested_level);
    }
  }
  if (has_members) {
    /* write object content */
    serialized_all_members = 0;
    while (!serialized_all_members) {
      memwriter_printf(memwriter, "\"%s\":", *member_name_ptr);
      serialize(memwriter, *data_type_ptr, NULL, NULL, -1, -1, -1, NULL, NULL, state->shared);
      ++member_name_ptr;
      ++data_type_ptr;
      if (*member_name_ptr != NULL && *data_type_ptr != NULL) {
        /* write JSON separator */
        memwriter_putc(memwriter, ',');
      }
      else {
        serialized_all_members = 1;
      }
    }
  }
  /* write object end if the type info is complete */
  if (!state->is_type_info_incomplete) {
    --(state->shared->struct_nested_level);
    memwriter_putc(memwriter, '}');
  }
  /* Only set serial result if not set before */
  if (state->shared->serial_result == 0) {
    if ((state->is_type_info_incomplete)) {
      state->shared->serial_result = has_members ? incomplete : incomplete_at_struct_beginning;
    }
  }

  /* cleanup */
  free(member_names);
  free(data_types);

  state->shared->wrote_output = 1;
  return 0;
}

static
int serialize(memwriter_t* memwriter, char *data_desc, void *data, va_list *vl,
              int apply_padding, int add_data, int add_data_without_separator,
              unsigned int *struct_nested_level, serialization_result_t *serial_result,
              shared_state_t *shared_state) {
  /**
   * memwriter: memwriter handle
   * data_desc: data description
   *      i: int
   *      I: int array -> I(count) or nI for variable length (see 'n' below); 'pI' indicates a pointer to an array
   *      d: double
   *      D: double array
   *      c: char
   *      C: string (char array)
   *      n: array length (for all following arrays)
   *      s: struct -> s(name:type, name:type, ...)
   *      e: empty byte (ignored memory) -> e(count) to specify multiple bytes
   * data: pointer to the buffer that shall be serialized
   * vl: if data is NULL the needed values are read from the va_list vl
   */

  state_t state;
  int json_array_needed = 0;
  int allocated_shared_state_mem = 0;

  state.next_is_ptr = 0;
  state.data_type_ptr = data_desc;
  state.current_data_type = 0;
  state.additional_type_info = NULL;
  state.is_type_info_incomplete = 0;
  if (shared_state == NULL) {
    /* TODO: check malloc call */
    shared_state = malloc(sizeof(state_t));
    shared_state->apply_padding = apply_padding;
    shared_state->array_length = 0;
    shared_state->data_ptr = data;
    shared_state->vl = vl;
    shared_state->data_offset = 0;
    shared_state->wrote_output = 0;
    shared_state->add_data = add_data;
    shared_state->add_data_without_separator = add_data_without_separator;
    shared_state->serial_result = 0;
    shared_state->struct_nested_level = *struct_nested_level;
    allocated_shared_state_mem = 1;
  }
  else {
    if (data != NULL) shared_state->data_ptr = data;
    if (vl != NULL) shared_state->vl = vl;
    if (apply_padding >= 0) shared_state->apply_padding = apply_padding;
  }
  state.shared = shared_state;

  json_array_needed = is_json_array_needed(data_desc);
  /* write list head if needed */
  if (json_array_needed) {
    memwriter_putc(memwriter, '[');
  }
  while (*state.data_type_ptr != 0) {
    shared_state->wrote_output = 0;
    read_data_type(&state);
    switch (state.current_data_type) {
    case 'n':
      read_array_length(&state);
      break;
    case 'p':
      read_next_is_ptr(&state);
      break;
    case 'e':
      skip_bytes(&state);
      break;
    case 'i':
      stringify_int(memwriter, &state);
      break;
    case 'I':
      stringify_int_array(memwriter, &state);
      break;
    case 'd':
      stringify_double(memwriter, &state);
      break;
    case 'D':
      stringify_double_array(memwriter, &state);
      break;
    case 'c':
      stringify_char(memwriter, &state);
      break;
    case 'C':
      stringify_char_array(memwriter, &state);
      break;
    case 'b':
      stringify_bool(memwriter, &state);
      break;
    case 's':
      stringify_struct(memwriter, &state);
      break;
    case ')':
      close_object(memwriter, &state);
      break;
    default:
      fprintf(stderr, "WARNING: %c (ASCII code %d) is not a valid type identifier\n",
              state.current_data_type, state.current_data_type);
      break;
    }
    if (*state.data_type_ptr != 0 && *state.data_type_ptr != ')' && shared_state->wrote_output) {
      /* write JSON separator, if data was written and the object is not closed in the next step */
      memwriter_putc(memwriter, ',');
    }
  }
  /* write list tail if needed */
  if (json_array_needed) {
    memwriter_putc(memwriter, ']');
  }

  if (serial_result != NULL) {
    /* check if shared_state->serial_result was set before */
    if (shared_state->serial_result) {
      *serial_result = shared_state->serial_result;
    }
    else {
      *serial_result = (shared_state->struct_nested_level > 0) ? incomplete : complete;
    }
  }
  if (struct_nested_level != NULL) {
    *struct_nested_level = shared_state->struct_nested_level;
  }

  /* cleanup */
  if (allocated_shared_state_mem) {
    free(shared_state);
  }

  return 0;
}

static
int init_variables(int *add_data, int *add_data_without_separator, char **_data_desc,
                   const char *data_desc) {
  *add_data = (permanent_state.serial_result != complete);
  *add_data_without_separator = (permanent_state.serial_result == incomplete_at_struct_beginning);
  if (*add_data) {
    /* TODO: check malloc call */
    *_data_desc = malloc(strlen(data_desc) + 3);
    strcpy(*_data_desc, "s(");
    strcpy((*_data_desc) + 2, data_desc);
  }
  else {
    /* TODO: check malloc call */
    *_data_desc = malloc(strlen(data_desc) + 1);
    strcpy(*_data_desc, data_desc);
  }

  return 0;
}

static
int tojson_write_vl(memwriter_t *memwriter, const char *data_desc, va_list *vl) {
  int add_data, add_data_without_separator;
  char *_data_desc;
  init_variables(&add_data, &add_data_without_separator, &_data_desc, data_desc);
  serialize(memwriter, _data_desc, NULL, vl, 0, add_data, add_data_without_separator,
            &permanent_state.struct_nested_level, &permanent_state.serial_result, NULL);
  free(_data_desc);

  return 1;
}

static
int tojson_is_complete() {
  return permanent_state.serial_result == complete;
}

static
int init_for_socket(metahandle_t *handle, va_list vl) {
  const char *hostname;
  unsigned int port;
  struct hostent *he;
#if defined(_WIN32) && !defined(__GNUC__)
  WSADATA wsa_data;
#endif

  hostname = va_arg(vl, const char *);
  port = va_arg(vl, unsigned int);

#if defined(_WIN32) && !defined(__GNUC__)
  /* Initialize Winsock */
  if (WSAStartup(MAKEWORD(2,2), &wsa_data)) {
    fprintf(stderr, "Winsock initialization failed");
    return -1;
  }
#endif

  he = gethostbyname(hostname);
  if (he == NULL || he->h_addr_list == NULL) {
    perror("gethostbyname");
    return -1;
  }
  handle->socket.client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  memcpy(&handle->socket.server_address.sin_addr, he->h_addr_list[0], sizeof(struct in_addr));
  handle->socket.server_address.sin_family = AF_INET;
  handle->socket.server_address.sin_port = htons(port);
  if (connect(handle->socket.client_socket_fd,
              (struct sockaddr *) &handle->socket.server_address,
              sizeof(handle->socket.server_address)) < 0) {
    perror("connect");
    return -1;
  }
  handle->memwriter = memwriter_new();
  if (handle->memwriter == NULL) {
    return -1;
  }
  handle->post_serialize = post_serialize_socket;

  return 0;
}

static
int init_for_jupyter(metahandle_t *handle, va_list vl) {
  jupyter_send_callback_t jupyter_send_callback;

  jupyter_send_callback = va_arg(vl, jupyter_send_callback_t);

  handle->jupyter.send = jupyter_send_callback;
  handle->memwriter = memwriter_new();
  if (handle->memwriter == NULL) {
    return -1;
  }
  handle->post_serialize = post_serialize_jupyter;
  return 0;
}

static
int finalize_for_jupyter(metahandle_t *handle) {
  /* TODO: implement me! */
  return 0;
}

static
int finalize_for_socket(metahandle_t *handle) {
  int result;
  int error = 0;

#if defined(_WIN32)
  result = closesocket(handle->socket.client_socket_fd);
#else
  result = close(handle->socket.client_socket_fd);
#endif
#if defined(_WIN32) && !defined(__GNUC__)
  result |= WSACleanup();
#endif
  if (result != 0) {
    fprintf(stderr, "Winsocket shutdown failed");
    error = -1;
  }
  return error;
}

static
int post_serialize_socket(void *p) {
  metahandle_t *handle = (metahandle_t *) p;
  char *buf;
  size_t buf_size;

  memwriter_putc(handle->memwriter, ETB);

  buf = memwriter_buf(handle->memwriter);
  buf_size = memwriter_size(handle->memwriter);

  send(handle->socket.client_socket_fd, buf, buf_size, 0);

  memwriter_clear(handle->memwriter);

  return 0;
}

static
int post_serialize_jupyter(void *p) {
  metahandle_t *handle = (metahandle_t *) p;
  char *buf;
  size_t buf_size;

  buf = memwriter_buf(handle->memwriter);
  buf_size = memwriter_size(handle->memwriter);

  handle->jupyter.send(buf);

  memwriter_clear(handle->memwriter);

  return 0;
}

void *gr_openmeta(int target, ...) {
  va_list vl;
  metahandle_t *handle;
  int error;

  handle = malloc(sizeof(metahandle_t));
  handle->target = target;
  va_start(vl, target);
  switch (target) {
  case GR_TARGET_JUPYTER:
    error = init_for_jupyter(handle, vl);
    break;
  case GR_TARGET_SOCKET:
    error = init_for_socket(handle, vl);
    break;
  }
  va_end(vl);

  if (error != 0) {
    free(handle);
    handle = NULL;
  }

  return (void *) handle;
}

void gr_closemeta(const void *p) {
  metahandle_t *handle = (metahandle_t *) p;

  switch (handle->target) {
  case GR_TARGET_JUPYTER:
    finalize_for_jupyter(handle);
    break;
  case GR_TARGET_SOCKET:
    finalize_for_socket(handle);
    break;
  }
  memwriter_delete(handle->memwriter);
  free(handle);
}

int gr_sendmeta(const void *p, const char *data_desc, ...) {
  metahandle_t *handle = (metahandle_t *) p;
  va_list vl;
  int error;
  int was_successful;

  va_start(vl, data_desc);
  was_successful = tojson_write_vl(handle->memwriter, data_desc, &vl);
  if (tojson_is_complete() && handle->post_serialize != NULL) {
    error = handle->post_serialize(handle);
    was_successful = was_successful && (error == 0);
  }
  va_end(vl);

  return was_successful;
}
