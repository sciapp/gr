#ifndef GRM_BASE64_INT_H_INCLUDED
#define GRM_BASE64_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <stdlib.h>

#include "grm/error.h"
#include <grm/base64.h>


/* ######################### internal interface ##################################################################### */

err_t block_decode(char dst[3], const char src[4], int block_len, int *decoded_block_len);
err_t block_encode(char dst[4], const char src[3], int block_len);

char *base64_decode(char *dst, const char *src, size_t *dst_len, err_t *error);
char *base64_encode(char *dst, const char *src, size_t src_len, err_t *error);

#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_BASE64_INT_H_INCLUDED */
