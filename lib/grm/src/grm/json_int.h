#ifndef GRM_JSON_INT_H_INCLUDED
#define GRM_JSON_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <stdarg.h>

#include <grm/args.h>
#include "error_int.h"
#include "memwriter_int.h"


/* ######################### internal interface ##################################################################### */

/* ========================= global varibales ======================================================================= */

/* ------------------------- json deserializer ---------------------------------------------------------------------- */

extern const char *const FROM_JSON_VALID_DELIMITERS;


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
} FromJsonDatatype;

typedef struct
{
  char *json_ptr;
  int parsed_any_value_before;
} FromJsonSharedState;

typedef struct
{
  FromJsonDatatype datatype;
  int parsing_object;
  void *value_buffer;
  int value_buffer_pointer_level;
  void *next_value_memory;
  char *next_value_type;
  grm_args_t *args;
  FromJsonSharedState *shared_state;
} FromJsonState;


/* ------------------------- json serializer ------------------------------------------------------------------------ */

enum
{
  MEMBER_NAME,
  DATA_TYPE
};

typedef enum
{
  /* 0 is unknown / not set */
  COMPLETE = 1,
  INCOMPLETE,
  INCOMPLETE_AT_STRUCT_BEGINNING
} ToJsonSerializationResult;

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
  ToJsonSerializationResult serial_result;
  unsigned int struct_nested_level;
} ToJsonSharedState;

typedef struct
{
  Memwriter *memwriter;
  char *data_type_ptr;
  char current_data_type;
  char *additional_type_info;
  int is_type_info_incomplete;
  int add_data_without_separator;
  ToJsonSharedState *shared;
} ToJsonState;

typedef struct
{
  ToJsonSerializationResult serial_result;
  unsigned int struct_nested_level;
} ToJsonPermanentState;

/* ========================= methods ================================================================================ */

/* ------------------------- json deserializer ---------------------------------------------------------------------- */

int grm_read(grm_args_t *args, const char *json_string);
grm_error_t fromJsonRead(grm_args_t *args, const char *json_string);

grm_error_t fromJsonParse(grm_args_t *args, const char *json_string, FromJsonSharedState *shared_state);
grm_error_t fromJsonParseNull(FromJsonState *state);
grm_error_t fromJsonParseBool(FromJsonState *state);
grm_error_t fromJsonParseNumber(FromJsonState *state);
grm_error_t fromJsonParseInt(FromJsonState *state);
grm_error_t fromJsonParseDouble(FromJsonState *state);
grm_error_t fromJsonParseString(FromJsonState *state);
grm_error_t fromJsonParseArray(FromJsonState *state);
grm_error_t fromJsonParseObject(FromJsonState *state);

FromJsonDatatype fromJsonCheckType(const FromJsonState *state);
grm_error_t fromJsonCopyAndFilterJsonString(char **dest, const char *src);
int fromJsonIsEscapedDelimiter(const char *delim_ptr, const char *str);
int fromJsonFindNextDelimiter(const char **delim_ptr, const char *src, int include_start,
                              int exclude_nested_structures);
size_t fromJsonGetOuterArrayLength(const char *str);
double fromJsonStrToDouble(const char **str, int *was_successful);
int fromJsonStrToInt(const char **str, int *was_successful);


/* ------------------------- json serializer ------------------------------------------------------------------------ */

#define declareStringify(name, type)                            \
  grm_error_t toJsonStringify##name(ToJsonState *state);        \
  grm_error_t toJsonStringify##name##Array(ToJsonState *state); \
  grm_error_t toJsonStringify##name##Value(Memwriter *memwriter, type value);

grm_error_t toJsonReadArrayLength(ToJsonState *state);
grm_error_t toJsonSkipBytes(ToJsonState *state);
declareStringify(Int, int) declareStringify(Double, double) declareStringify(Char, char)
    declareStringify(String, char *) declareStringify(Bool, int) grm_error_t toJsonStringifyObject(ToJsonState *state);
declareStringify(Args, grm_args_t *) grm_error_t toJsonCloseObject(ToJsonState *state);

#undef declareStringify

int toJsonGetMemberCount(const char *data_desc);
int toJsonIsJsonArrayNeeded(const char *data_desc);
void toJsonReadDatatype(ToJsonState *state);
grm_error_t toJsonUnzipMemberNamesAndDatatypes(char *mixed_ptr, char ***member_name_ptr, char ***data_type_ptr);
grm_error_t toJsonEscapeSpecialChars(char **escaped_string, const char *unescaped_string, unsigned int *length);
grm_error_t toJsonSerialize(Memwriter *memwriter, char *data_desc, const void *data, va_list *vl, int apply_padding,
                            int add_data, int add_data_without_separator, unsigned int *struct_nested_level,
                            ToJsonSerializationResult *serial_result, ToJsonSharedState *shared_state);
void toJsonInitStaticVariables(void);
grm_error_t toJsonInitVariables(int *add_data, int *add_data_without_separator, char **data_desc_priv,
                                const char *data_desc);
grm_error_t toJsonWrite(Memwriter *memwriter, const char *data_desc, ...);
grm_error_t toJsonWriteVl(Memwriter *memwriter, const char *data_desc, va_list *vl);
grm_error_t toJsonWriteBuf(Memwriter *memwriter, const char *data_desc, const void *buffer, int apply_padding);
grm_error_t toJsonWriteArg(Memwriter *memwriter, const grm_arg_t *arg);
grm_error_t toJsonWriteArgs(Memwriter *memwriter, const grm_args_t *args);
int toJsonIsComplete(void);
int toJsonStructNestedLevel(void);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_JSON_INT_H_INCLUDED */
