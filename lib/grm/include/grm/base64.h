#ifndef GRM_BASE64_H_INCLUDED
#define GRM_BASE64_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <stdlib.h>

#include "util.h"


/* ######################### public interface ####################################################################### */

GRM_EXPORT char *grm_base64_decode(char *dst, const char *src, size_t *dst_len, int *was_successful);
GRM_EXPORT char *grm_base64_encode(char *dst, const char *src, size_t src_len, int *was_successful);

#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_BASE64_H_INCLUDED */
