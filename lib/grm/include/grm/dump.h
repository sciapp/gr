#ifndef GRM_DUMP_H_INCLUDED
#define GRM_DUMP_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <stdio.h>

#include "args.h"
#include "util.h"


/* ######################### public interface ####################################################################### */

/* ========================= functions ============================================================================== */

/* ------------------------- dump ----------------------------------------------------------------------------------- */

#ifndef NDEBUG
EXPORT void grm_dump(const grm_args_t *args, FILE *f);
EXPORT void grm_dump_json(const grm_args_t *args, FILE *f);
EXPORT char *grm_dump_json_str(void);
EXPORT void grm_dump_bson(const grm_args_t *args, FILE *f);
#else
#define grm_dump(args, f) \
  do                      \
    {                     \
    }                     \
  while (0)
#define grm_dump_json(args, f) \
  do                           \
    {                          \
    }                          \
  while (0)
#define grm_dump_json_str \
  do                      \
    {                     \
    }                     \
  while (0)
#define grm_dump_bson(args, f) \
  do                           \
    {                          \
    }                          \
  while (0)
#endif

#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_DUMP_H_INCLUDED */
