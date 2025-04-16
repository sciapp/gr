#ifndef GRM_BSON_INT_H_INCLUDED
#define GRM_BSON_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <stdarg.h>

#include <grm/args.h>
#include "error_int.h"
#include "memwriter_int.h"
#include "json_int.h"

#include "datatype/size_t_list_int.h"


/* ######################### internal interface ##################################################################### */

/* ========================= global varibales ======================================================================= */

/* ========================= macros ================================================================================= */

/* ========================= datatypes ============================================================================== */

/* ------------------------- bson deserializer ---------------------------------------------------------------------- */

typedef struct
{
  int length;
  int num_bytes_read_before;
  int num_elements;
} FromBsonArrayInfos;

typedef struct
{
  int length;
  int num_bytes_read_before;
} FromBsonObjectInfos;

typedef struct
{
  grm_args_t *args;
  const char *cur_byte;
  int num_read_bytes;
  char cur_value_format;
  void *cur_value_buf;
  const char *cur_key;
  FromBsonArrayInfos *array_infos;
  FromBsonObjectInfos *object_infos;
} FromBsonState;


/* ------------------------- bson serializer ------------------------------------------------------------------------ */

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
} ToBsonSharedState;

typedef struct
{
  Memwriter *memwriter;
  char *data_type_ptr;
  char current_data_type;
  char *additional_type_info;
  int is_type_info_incomplete;
  int add_data_without_separator;
  ToBsonSharedState *shared;
} ToBsonState;

typedef struct
{
  ToJsonSerializationResult serial_result;
  unsigned int struct_nested_level;
  SizeTList *memwriter_object_start_offset_stack;
} ToBsonPermanentState;

/* ========================= small helper functions ================================================================= */

void revMemCpy(void *dest, const void *src, size_t len);

void memCpyRevChunks(void *dest, const void *src, size_t len, size_t chunk_size);

char byteToType(const char *byte);

void intToBytes(int i, char **bytes);

void bytesToInt(int *i, const char *bytes);

void doubleToBytes(double d, char **bytes);

void bytesToDouble(double *d, const char *bytes);

/* ------------------------- bson deserializer ---------------------------------------------------------------------- */

grm_error_t fromBsonRead(grm_args_t *args, const char *bson_bytes);
grm_error_t fromBsonReadValueFormat(FromBsonState *state, char *value_format);
grm_error_t fromBsonReadKey(FromBsonState *state, const char **key);
grm_error_t fromBsonSkipKey(FromBsonState *state);
grm_error_t fromBsonReadLength(FromBsonState *state, int *length);

grm_error_t fromBsonReadDoubleValue(FromBsonState *state, double *d);
grm_error_t fromBsonReadIntValue(FromBsonState *state, int *i);
grm_error_t fromBsonReadStringValue(FromBsonState *state, const char **s);
grm_error_t fromBsonReadBoolValue(FromBsonState *state, int *b);
grm_error_t fromBsonReadObject(FromBsonState *state);

grm_error_t fromBsonParseDouble(FromBsonState *state);
grm_error_t fromBsonParseInt(FromBsonState *state);
grm_error_t fromBsonParseBool(FromBsonState *state);
grm_error_t fromBsonParseArray(FromBsonState *state);
grm_error_t fromBsonParseObject(FromBsonState *state);

grm_error_t fromBsonReadIntArray(FromBsonState *state);
grm_error_t fromBsonReadDoubleArray(FromBsonState *state);
grm_error_t fromBsonReadStringArray(FromBsonState *state);
grm_error_t fromBsonReadBoolArray(FromBsonState *state);

void fromBsonInitStaticVariables(void);

/* ------------------------- bson serializer ------------------------------------------------------------------------ */

grm_error_t toBsonIntValue(Memwriter *memwriter, int value);
grm_error_t toBsonDoubleValue(Memwriter *memwriter, double value);
grm_error_t toBsonCharValue(Memwriter *memwriter, char value);
grm_error_t toBsonStringValue(Memwriter *memwriter, char *value);
grm_error_t toBsonBoolValue(Memwriter *memwriter, int value);
grm_error_t toBsonArgsValue(Memwriter *memwriter, grm_args_t *args);

grm_error_t toBsonInt(ToBsonState *state);
grm_error_t toBsonDouble(ToBsonState *state);
grm_error_t toBsonChar(ToBsonState *state);
grm_error_t toBsonString(ToBsonState *state);
grm_error_t toBsonBool(ToBsonState *state);
grm_error_t toBsonArgs(ToBsonState *state);

grm_error_t toBsonIntArray(ToBsonState *state);
grm_error_t toBsonDoubleArray(ToBsonState *state);
grm_error_t toBsonOptimizedArray(ToBsonState *state);
grm_error_t toBsonCharArray(ToBsonState *state);
grm_error_t toBsonStringArray(ToBsonState *state);
grm_error_t toBsonBoolArray(ToBsonState *state);
grm_error_t toBsonArgsArray(ToBsonState *state);

grm_error_t toBsonReadArrayLength(ToBsonState *state);
grm_error_t toBsonSkipBytes(ToBsonState *state);
grm_error_t toBsonObject(ToBsonState *state);
grm_error_t toBsonOpenObject(Memwriter *memwriter);
grm_error_t toBsonCloseObject(ToBsonState *state);

int toBsonGetMemberCount(const char *data_desc);
int toBsonIsBsonArrayNeeded(const char *data_desc);
void toBsonReadDatatype(ToBsonState *state);
grm_error_t toBsonUnzipMemberNamesAndDatatypes(char *mixed_ptr, char ***member_name_ptr, char ***data_type_ptr);
grm_error_t toBsonSerialize(Memwriter *memwriter, char *data_desc, const void *data, va_list *vl, int apply_padding,
                            int add_data, int add_data_without_separator, unsigned int *struct_nested_level,
                            ToJsonSerializationResult *serial_result, ToBsonSharedState *shared_state);
void toBsonInitStaticVariables(void);
grm_error_t toBsonInitVariables(int *add_data, int *add_data_without_separator, char **data_desc_priv,
                                const char *data_desc);
grm_error_t toBsonWrite(Memwriter *memwriter, const char *data_desc, ...);
grm_error_t toBsonWriteVl(Memwriter *memwriter, const char *data_desc, va_list *vl);
grm_error_t toBsonWriteBuf(Memwriter *memwriter, const char *data_desc, const void *buffer, int apply_padding);
grm_error_t toBsonWriteArg(Memwriter *memwriter, const grm_arg_t *arg);
grm_error_t toBsonWriteArgs(Memwriter *memwriter, const grm_args_t *args);
int toBsonIsComplete(void);
int toBsonStructNestedLevel(void);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_BSON_INT_H_INCLUDED */
