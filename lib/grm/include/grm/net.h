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

EXPORT void *grm_open(int is_receiver, const char *name, unsigned int id,
                      const char *(*custom_recv)(const char *, unsigned int),
                      int (*custom_send)(const char *, unsigned int, const char *));
EXPORT grm_args_t *grm_recv(const void *p, grm_args_t *args);
EXPORT int grm_send(const void *p, const char *data_desc, ...);
EXPORT int grm_send_buf(const void *p, const char *data_desc, const void *buffer, int apply_padding);
EXPORT int grm_send_ref(const void *p, const char *key, char format, const void *ref, int len);
EXPORT int grm_send_args(const void *p, const grm_args_t *args);
EXPORT void grm_close(const void *p);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_NET_H_INCLUDED */
