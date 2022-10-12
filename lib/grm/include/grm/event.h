#ifndef GRM_EVENT_H_INCLUDED
#define GRM_EVENT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include "util.h"


/* ######################### public interface ####################################################################### */

/* ========================= datatypes ============================================================================== */

/* ------------------------- event handling ------------------------------------------------------------------------- */

typedef enum
{
  GRM_EVENT_NEW_PLOT,
  GRM_EVENT_UPDATE_PLOT,
  GRM_EVENT_SIZE,
  GRM_EVENT_MERGE_END,
  _GRM_EVENT_TYPE_COUNT /* helper entry to store how many different event types exist */
} grm_event_type_t;

typedef struct
{
  grm_event_type_t type;
  int plot_id;
} grm_new_plot_event_t;

typedef struct
{
  grm_event_type_t type;
  int plot_id;
} grm_update_plot_event_t;

typedef struct
{
  grm_event_type_t type;
  int plot_id;
  int width;
  int height;
} grm_size_event_t;

typedef struct
{
  grm_event_type_t type;
  const char *identificator;
} grm_merge_end_event_t;

typedef union
{
  grm_new_plot_event_t new_plot_event;
  grm_size_event_t size_event;
  grm_update_plot_event_t update_plot_event;
  grm_merge_end_event_t merge_end_event;
} grm_event_t;

typedef void (*grm_event_callback_t)(const grm_event_t *);


/* ========================= methods ================================================================================ */

/* ------------------------- event handling ------------------------------------------------------------------------- */

EXPORT int grm_register(grm_event_type_t type, grm_event_callback_t callback);
EXPORT int grm_unregister(grm_event_type_t type);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_EVENT_H_INCLUDED */
