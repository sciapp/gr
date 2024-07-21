#ifndef GRM_JSON_INT_H_INCLUDED
#define GRM_JSON_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <stdarg.h>

#include <grm/args.h>
#include "grm/error.h"
#include "memwriter_int.h"


/* ######################### internal interface ##################################################################### */

/* ========================= global varibales ======================================================================= */

/* ------------------------- json deserializer ---------------------------------------------------------------------- */

extern const char *FROMJSON_VALID_DELIMITERS;


/* ========================= macros ================================================================================= */

/* ------------------------- json deserializer ---------------------------------------------------------------------- */

#define NEXT_VALUE_TYPE_SIZE 80


/* ========================= datatypes ============================================================================== */

/* ------------------------- json deserializer ---------------------------------------------------------------------- */

typedef enum
{
  JSON_DATATYPE_UNKNOWN,
  JSON_DATATYPE_NULL,
  JSON_DATATYPE_BOOL,
  JSON_DATATYPE_NUMBER,
  JSON_DATATYPE_STRING,
  JSON_DATATYPE_ARRAY,
  JSON_DATATYPE_OBJECT
} fromjson_datatype_t;

typedef struct
{
  char *json_ptr;
  int parsed_any_value_before;
} fromjson_shared_state_t;

typedef struct
{
  fromjson_datatype_t datatype;
  int parsing_object;
  void *value_buffer;
  int value_buffer_pointer_level;
  void *next_value_memory;
  char *next_value_type;
  grm_args_t *args;
  fromjson_shared_state_t *shared_state;
} fromjson_state_t;


/* ------------------------- json serializer ------------------------------------------------------------------------ */

typedef err_t (*tojson_post_processing_callback_t)(memwriter_t *, unsigned int, const char *);

enum
{
  member_name,
  data_type
};

typedef enum
{
  /* 0 is unknown / not set */
  complete = 1,
  incomplete,
  incomplete_at_struct_beginning
} tojson_serialization_result_t;

typedef struct
{
  int apply_padding;
  size_t array_length;
  int read_length_from_string;
  const void *data_ptr;
  va_list *vl;
  int data_offset;
  int wrote_output;
  int add_data;
  tojson_serialization_result_t serial_result;
  unsigned int struct_nested_level;
} tojson_shared_state_t;

typedef struct
{
  memwriter_t *memwriter;
  char *data_type_ptr;
  char current_data_type;
  char *additional_type_info;
  int is_type_info_incomplete;
  int add_data_without_separator;
  tojson_shared_state_t *shared;
} tojson_state_t;

typedef struct
{
  tojson_serialization_result_t serial_result;
  unsigned int struct_nested_level;
} tojson_permanent_state_t;

/* ========================= methods ================================================================================ */

/* ------------------------- json deserializer ---------------------------------------------------------------------- */

int grm_read(grm_args_t *args, const char *json_string);
err_t fromjson_read(grm_args_t *args, const char *json_string);

err_t fromjson_parse(grm_args_t *args, const char *json_string, fromjson_shared_state_t *shared_state);
err_t fromjson_parse_null(fromjson_state_t *state);
err_t fromjson_parse_bool(fromjson_state_t *state);
err_t fromjson_parse_number(fromjson_state_t *state);
err_t fromjson_parse_int(fromjson_state_t *state);
err_t fromjson_parse_double(fromjson_state_t *state);
err_t fromjson_parse_string(fromjson_state_t *state);
err_t fromjson_parse_array(fromjson_state_t *state);
err_t fromjson_parse_object(fromjson_state_t *state);

fromjson_datatype_t fromjson_check_type(const fromjson_state_t *state);
err_t fromjson_copy_and_filter_json_string(char **dest, const char *src);
int fromjson_is_escaped_delimiter(const char *delim_ptr, const char *str);
int fromjson_find_next_delimiter(const char **delim_ptr, const char *src, int include_start,
                                 int exclude_nested_structures);
size_t fromjson_get_outer_array_length(const char *str);
double fromjson_str_to_double(const char **str, int *was_successful);
int fromjson_str_to_int(const char **str, int *was_successful);


/* ------------------------- json serializer ------------------------------------------------------------------------ */

#define DECLARE_STRINGIFY(name, type)                           \
  err_t tojson_stringify_##name(tojson_state_t *state);         \
  err_t tojson_stringify_##name##_array(tojson_state_t *state); \
  err_t tojson_stringify_##name##_value(memwriter_t *memwriter, type value);

err_t tojson_read_array_length(tojson_state_t *state);
err_t tojson_skip_bytes(tojson_state_t *state);
DECLARE_STRINGIFY(int, int)
DECLARE_STRINGIFY(double, double)
DECLARE_STRINGIFY(char, char)
DECLARE_STRINGIFY(string, char *)
DECLARE_STRINGIFY(bool, int)
err_t tojson_stringify_object(tojson_state_t *state);
DECLARE_STRINGIFY(args, grm_args_t *)
err_t tojson_close_object(tojson_state_t *state);

#undef DECLARE_STRINGIFY_SINGLE
#undef DECLARE_STRINGIFY_MULTI

int tojson_get_member_count(const char *data_desc);
int tojson_is_json_array_needed(const char *data_desc);
void tojson_read_datatype(tojson_state_t *state);
err_t tojson_unzip_membernames_and_datatypes(char *mixed_ptr, char ***member_name_ptr, char ***data_type_ptr);
err_t tojson_escape_special_chars(char **escaped_string, const char *unescaped_string, unsigned int *length);
err_t tojson_serialize(memwriter_t *memwriter, char *data_desc, const void *data, va_list *vl, int apply_padding,
                       int add_data, int add_data_without_separator, unsigned int *struct_nested_level,
                       tojson_serialization_result_t *serial_result, tojson_shared_state_t *shared_state);
void tojson_init_static_variables(void);
err_t tojson_init_variables(int *add_data, int *add_data_without_separator, char **_data_desc, const char *data_desc);
err_t tojson_write(memwriter_t *memwriter, const char *data_desc, ...);
err_t tojson_write_vl(memwriter_t *memwriter, const char *data_desc, va_list *vl);
err_t tojson_write_buf(memwriter_t *memwriter, const char *data_desc, const void *buffer, int apply_padding);
err_t tojson_write_arg(memwriter_t *memwriter, const arg_t *arg);
err_t tojson_write_args(memwriter_t *memwriter, const grm_args_t *args);
int tojson_is_complete(void);
int tojson_struct_nested_level(void);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_JSON_INT_H_INCLUDED */
