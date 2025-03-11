#ifndef GRM_EVENT_INT_H_INCLUDED
#define GRM_EVENT_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include "datatype/template/list_int.h"
#include "error_int.h"
#include <grm/event.h>


/* ######################### internal interface ##################################################################### */

/* ========================= datatypes ============================================================================== */

/* ------------------------- event handling ------------------------------------------------------------------------- */

DECLARE_LIST_TYPE(Event, grm_event_t *)

typedef struct
{
  EventReflist *queue;
  grm_event_callback_t *event_callbacks;
} EventQueue;

#undef DECLARE_LIST_TYPE

/* ========================= functions ============================================================================== */

/* ------------------------- event handling ------------------------------------------------------------------------- */

int processEvents(void);


/* ========================= methods ================================================================================ */

/* ------------------------- event handling ------------------------------------------------------------------------- */

DECLARE_LIST_METHODS(Event, event)

EventQueue *eventQueueNew(void);
void eventQueueDelete(EventQueue *queue);

void eventQueueRegister(EventQueue *queue, grm_event_type_t type, grm_event_callback_t callback);
void eventQueueUnregister(EventQueue *queue, grm_event_type_t type);

int eventQueueProcessNext(EventQueue *queue);
int eventQueueProcessAll(EventQueue *queue);

grm_error_t eventQueueEnqueueNewPlotEvent(EventQueue *queue, int plot_id);
grm_error_t eventQueueEnqueueUpdatePlotEvent(EventQueue *queue, int plot_id);
grm_error_t eventQueueEnqueueSizeEvent(EventQueue *queue, int plot_id, int width, int height);
grm_error_t eventQueueEnqueueMergeEndEvent(EventQueue *queue, const char *identificator);
grm_error_t eventQueueEnqueueRequestEvent(EventQueue *queue, const char *request_string);
grm_error_t eventQueueEnqueueIntegralUpdateEvent(EventQueue *queue, double int_lim_low, double int_lim_high);

void eventQueueDiscardAllOfType(EventQueue *queue, grm_event_type_t type);

#undef DECLARE_LIST_METHODS


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_EVENT_INT_H_INCLUDED */
