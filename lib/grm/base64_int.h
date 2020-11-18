#ifndef GRM_BASE64_INT_H_INCLUDED
#define GRM_BASE64_INT_H_INCLUDED

#include <stdlib.h>

#include "error_int.h"


error_t block_decode(char dst[3], const char src[4], int block_len, int *decoded_block_len);
error_t block_encode(char dst[4], const char src[3], int block_len);

char *base64_decode(char *dst, const char *src, size_t *dst_len, error_t *error);
char *base64_encode(char *dst, const char *src, size_t src_len, error_t *error);

#endif /* ifndef GRM_BASE64_INT_H_INCLUDED */
