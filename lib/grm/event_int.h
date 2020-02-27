#ifndef GRM_EVENT_INT_H_INCLUDED
#define GRM_EVENT_INT_H_INCLUDED

/* ######################### includes ############################################################################### */

#include "datatype/template/list_int.h"
#include "error_int.h"
#include "event.h"


/* ######################### internal interface ##################################################################### */

/* ========================= datatypes ============================================================================== */

/* ------------------------- event handling ------------------------------------------------------------------------- */

DECLARE_LIST_TYPE(event, grm_event_t *)

typedef struct
{
  event_reflist_t *queue;
  grm_event_callback_t *event_callbacks;
} event_queue_t;

#undef DECLARE_LIST_TYPE

/* ========================= functions ============================================================================== */

/* ------------------------- event handling ------------------------------------------------------------------------- */

int process_events(void);


/* ========================= methods ================================================================================ */

/* ------------------------- event handling ------------------------------------------------------------------------- */

DECLARE_LIST_METHODS(event)

event_queue_t *event_queue_new(void);
void event_queue_delete(event_queue_t *queue);

void event_queue_register(event_queue_t *queue, grm_event_type_t type, grm_event_callback_t callback);
void event_queue_unregister(event_queue_t *queue, grm_event_type_t type);

int event_queue_process_next(event_queue_t *queue);
int event_queue_process_all(event_queue_t *queue);

error_t event_queue_enqueue_new_plot_event(event_queue_t *queue, int plot_id);
error_t event_queue_enqueue_update_plot_event(event_queue_t *queue, int plot_id);
error_t event_queue_enqueue_size_event(event_queue_t *queue, int plot_id, int width, int height);
error_t event_queue_enqueue_merge_end_event(event_queue_t *queue, const char *identificator);


#undef DECLARE_LIST_METHODS


#endif /* ifndef GRM_EVENT_INT_H_INCLUDED */
