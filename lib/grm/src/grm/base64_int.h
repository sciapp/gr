#ifndef GRM_BASE64_INT_H_INCLUDED
#define GRM_BASE64_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <stdlib.h>

#include "error_int.h"
#include <grm/base64.h>


/* ######################### internal interface ##################################################################### */

grm_error_t blockDecode(char dst[3], const char src[4], int block_len, int *decoded_block_len);
grm_error_t blockEncode(char dst[4], const char src[3], int block_len);

char *base64Decode(char *dst, const char *src, size_t *dst_len, grm_error_t *error);
char *base64Encode(char *dst, const char *src, size_t src_len, grm_error_t *error);

#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_BASE64_INT_H_INCLUDED */
