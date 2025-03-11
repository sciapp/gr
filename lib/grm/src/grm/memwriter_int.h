#ifndef GRM_MEMWRITER_INT_H_INCLUDED
#define GRM_MEMWRITER_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <stdlib.h>

#include "error_int.h"
#include "util_int.h"

/* ######################### internal interface ##################################################################### */

/* ========================= macros ================================================================================= */

/* ------------------------- memwriter ------------------------------------------------------------------------------ */

#define MEMWRITER_INITIAL_SIZE 32768
#define MEMWRITER_EXPONENTIAL_INCREASE_UNTIL 268435456
#define MEMWRITER_LINEAR_INCREMENT_SIZE 67108864

#define ETB '\027'


/* ========================= datatypes ============================================================================== */

/* ------------------------- memwriter ------------------------------------------------------------------------------ */

struct Memwriter
{
  char *buf;
  size_t size;
  size_t capacity;
};
typedef struct Memwriter Memwriter;

/* ========================= methods ================================================================================ */

/* ------------------------- memwriter ------------------------------------------------------------------------------ */

Memwriter *memwriterNew(void);
void memwriterDelete(Memwriter *memwriter);
void memwriterClear(Memwriter *memwriter);
grm_error_t memwriterReplace(Memwriter *memwriter, int index, int count, const char *replacement_str);
grm_error_t memwriterErase(Memwriter *memwriter, int index, int count);
grm_error_t memwriterInsert(Memwriter *memwriter, int index, const char *str) UNUSED;
grm_error_t memwriterEnlargeBuf(Memwriter *memwriter, size_t size_increment);
grm_error_t memwriterEnsureBuf(Memwriter *memwriter, size_t needed_additional_size);
grm_error_t memwriterPrintf(Memwriter *memwriter, const char *format, ...);
grm_error_t memwriterPuts(Memwriter *memwriter, const char *s);
grm_error_t memwriterPutsWithLen(Memwriter *memwriter, char *s, size_t length);
grm_error_t memwriterMemcpy(Memwriter *memwriter, const void *source, size_t num);
grm_error_t memwriterMemcpyRevChunks(Memwriter *memwriter, const void *source, size_t num, int chunk_size);
grm_error_t memwriterPutc(Memwriter *memwriter, char c);
char *memwriterBuf(const Memwriter *memwriter);
size_t memwriterSize(const Memwriter *memwriter);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_MEMWRITER_INT_H_INCLUDED */
