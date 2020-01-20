#ifndef GRM_NET_H_INCLUDED
#define GRM_NET_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include "args.h"
#include "util.h"


/* ######################### public interface ####################################################################### */

/* ========================= macros ================================================================================= */

/* ------------------------- receiver / sender----------------------------------------------------------------------- */

#define GRM_SENDER 0
#define GRM_RECEIVER 1


/* ========================= functions ============================================================================== */

/* ------------------------- receiver / sender----------------------------------------------------------------------- */

EXPORT void *grm_open(int, const char *, unsigned int, const char *(*)(const char *, unsigned int),
                      int (*)(const char *, unsigned int, const char *));
EXPORT grm_args_t *grm_recv(const void *p, grm_args_t *);
EXPORT int grm_send(const void *, const char *, ...);
EXPORT int grm_send_buf(const void *, const char *, const void *, int);
EXPORT int grm_send_ref(const void *, const char *, char, const void *, int);
EXPORT int grm_send_args(const void *p, const grm_args_t *);
EXPORT void grm_close(const void *);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_NET_H_INCLUDED */
