#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include <string.h>

#include "error_int.h"
#include "event_int.h"
#include "plot_int.h"


/* ######################### internal implementation ################################################################ */

/* ========================= static variables ======================================================================= */

/* ------------------------- event handling ------------------------------------------------------------------------- */

static int processing_events = 0;


/* ========================= functions ============================================================================== */

/* ------------------------- event handling ------------------------------------------------------------------------- */

int processEvents(void)
{
  int processed_events = 0;
  /* Trigger event handling routines after plotting -> args container is fully processed (and modified consistently) at
   * this time */
  if (!processing_events)
    {
      processing_events = 1; /* Ensure that event processing won't trigger event processing again */
      processed_events = eventQueueProcessAll(event_queue);
      processing_events = 0;
    }

  return processed_events;
}


/* ========================= methods ================================================================================ */

/* ------------------------- event handling ------------------------------------------------------------------------- */

DEFINE_LIST_METHODS(Event, event)

grm_error_t eventListEntryCopy(EventListEntry *copy, EventListConstEntry entry)
{
  EventListEntry tmp_copy;

  tmp_copy = malloc(sizeof(grm_event_t));
  if (tmp_copy == NULL) return GRM_ERROR_MALLOC;

  memcpy(tmp_copy, entry, sizeof(grm_event_t));
  *copy = tmp_copy;

  return GRM_ERROR_NONE;
}

grm_error_t eventListEntryDelete(EventListEntry entry)
{
  free(entry);
  return GRM_ERROR_NONE;
}

EventQueue *eventQueueNew(void)
{
  EventQueue *queue = NULL;

  queue = malloc(sizeof(EventQueue));
  errorCleanupIf(queue == NULL);
  queue->queue = NULL;
  queue->event_callbacks = NULL;
  queue->queue = eventReflistNew();
  errorCleanupIf(queue->queue == NULL);
  queue->event_callbacks = calloc(_GRM_EVENT_TYPE_COUNT, sizeof(grm_event_callback_t));
  errorCleanupIf(queue->event_callbacks == NULL);

  return queue;

error_cleanup:
  if (queue != NULL)
    {
      if (queue->queue != NULL) eventReflistDelete(queue->queue);
      if (queue->event_callbacks != NULL) free(queue->event_callbacks);
      free(queue);
    }

  return NULL;
}

void eventQueueDelete(EventQueue *queue)
{
  eventReflistDeleteWithEntries(queue->queue);
  free(queue->event_callbacks);
  free(queue);
}

void eventQueueRegister(EventQueue *queue, grm_event_type_t type, grm_event_callback_t callback)
{
  queue->event_callbacks[type] = callback;
}

void eventQueueUnregister(EventQueue *queue, grm_event_type_t type)
{
  queue->event_callbacks[type] = NULL;
}

int eventQueueProcessNext(EventQueue *queue)
{
  grm_event_t *event;
  grm_event_type_t type;

  if (eventReflistEmpty(queue->queue)) return 0;

  event = eventReflistDequeue(queue->queue);
  type = *((int *)event);
  if (queue->event_callbacks[type] != NULL) queue->event_callbacks[type](event);
  free(event);

  return 1;
}

int eventQueueProcessAll(EventQueue *queue)
{
  if (eventReflistEmpty(queue->queue)) return 0;

  while (eventQueueProcessNext(queue))
    ;

  return 1;
}

grm_error_t eventQueueEnqueueNewPlotEvent(EventQueue *queue, int plot_id)
{
  grm_new_plot_event_t *new_plot_event = NULL;
  grm_error_t error;

  new_plot_event = malloc(sizeof(grm_new_plot_event_t));
  errorCleanupAndSetErrorIf(new_plot_event == NULL, GRM_ERROR_MALLOC);
  new_plot_event->type = GRM_EVENT_NEW_PLOT;
  new_plot_event->plot_id = plot_id;

  error = eventReflistEnqueue(queue->queue, (grm_event_t *)new_plot_event);
  errorCleanupIfError;

  return GRM_ERROR_NONE;

error_cleanup:
  if (new_plot_event != NULL) free(new_plot_event);

  return error;
}

grm_error_t eventQueueEnqueueUpdatePlotEvent(EventQueue *queue, int plot_id)
{
  grm_update_plot_event_t *update_plot_event = NULL;
  grm_error_t error;

  update_plot_event = malloc(sizeof(grm_update_plot_event_t));
  errorCleanupAndSetErrorIf(update_plot_event == NULL, GRM_ERROR_MALLOC);
  update_plot_event->type = GRM_EVENT_UPDATE_PLOT;
  update_plot_event->plot_id = plot_id;

  error = eventReflistEnqueue(queue->queue, (grm_event_t *)update_plot_event);
  errorCleanupIfError;

  return GRM_ERROR_NONE;

error_cleanup:
  if (update_plot_event != NULL) free(update_plot_event);

  return error;
}

grm_error_t eventQueueEnqueueSizeEvent(EventQueue *queue, int plot_id, int width, int height)
{
  grm_size_event_t *size_event = NULL;
  grm_error_t error;

  eventQueueDiscardAllOfType(queue, GRM_EVENT_SIZE);

  size_event = malloc(sizeof(grm_size_event_t));
  errorCleanupAndSetErrorIf(size_event == NULL, GRM_ERROR_MALLOC);
  size_event->type = GRM_EVENT_SIZE;
  size_event->plot_id = plot_id;
  size_event->width = width;
  size_event->height = height;

  error = eventReflistEnqueue(queue->queue, (grm_event_t *)size_event);
  errorCleanupIfError;

  return GRM_ERROR_NONE;

error_cleanup:
  if (size_event != NULL) free(size_event);

  return error;
}

grm_error_t eventQueueEnqueueMergeEndEvent(EventQueue *queue, const char *identificator)
{
  grm_merge_end_event_t *merge_end_event = NULL;
  grm_error_t error;

  merge_end_event = malloc(sizeof(grm_merge_end_event_t));
  errorCleanupAndSetErrorIf(merge_end_event == NULL, GRM_ERROR_MALLOC);
  merge_end_event->type = GRM_EVENT_MERGE_END;
  merge_end_event->identificator = identificator;
  error = eventReflistEnqueue(queue->queue, (grm_event_t *)merge_end_event);
  errorCleanupIfError;

  return GRM_ERROR_NONE;

error_cleanup:
  if (merge_end_event != NULL) free(merge_end_event);

  return error;
}

grm_error_t eventQueueEnqueueRequestEvent(EventQueue *queue, const char *request_string)
{
  grm_request_event_t *request_event = NULL;
  grm_error_t error;

  request_event = malloc(sizeof(grm_request_event_t));
  errorCleanupAndSetErrorIf(request_event == NULL, GRM_ERROR_MALLOC);
  request_event->type = GRM_EVENT_REQUEST;
  request_event->request_string = request_string;
  error = eventReflistEnqueue(queue->queue, (grm_event_t *)request_event);
  errorCleanupIfError;

  return GRM_ERROR_NONE;

error_cleanup:
  if (request_event != NULL) free(request_event);

  return error;
}

grm_error_t eventQueueEnqueueIntegralUpdateEvent(EventQueue *queue, double int_lim_low, double int_lim_high)
{
  grm_integral_update_event_t *integral_update_event = NULL;
  grm_error_t error;

  integral_update_event = malloc(sizeof(grm_integral_update_event_t));
  errorCleanupAndSetErrorIf(integral_update_event == NULL, GRM_ERROR_MALLOC);
  integral_update_event->type = GRM_EVENT_INTEGRAL_UPDATE;
  integral_update_event->int_lim_low = int_lim_low;
  integral_update_event->int_lim_high = int_lim_high;
  error = eventReflistEnqueue(queue->queue, (grm_event_t *)integral_update_event);
  errorCleanupIfError;

  return GRM_ERROR_NONE;

error_cleanup:
  if (integral_update_event != NULL) free(integral_update_event);

  return error;
}

void eventQueueDiscardAllOfType(EventQueue *queue, grm_event_type_t type)
{
  EventReflistNode *previous_node = NULL, *current_node, *next_node = NULL;

  current_node = queue->queue->head;
  while (current_node != NULL)
    {
      next_node = current_node->next;
      if (current_node->entry->size_event.type == type)
        {
          logger((stderr, "Discarding event of type \"%d\"\n", type));
          eventReflistEntryDelete(current_node->entry);
          free(current_node);
          queue->queue->size--;
          if (current_node == queue->queue->head) queue->queue->head = next_node;
          if (current_node == queue->queue->tail) queue->queue->tail = previous_node;
        }
      else
        {
          previous_node = current_node;
        }
      current_node = next_node;
    }
}

#undef DEFINE_LIST_METHODS


/* ######################### public implementation ################################################################## */

/* ========================= methods ================================================================================ */

/* ------------------------- event handling ------------------------------------------------------------------------- */

int grm_register(grm_event_type_t type, grm_event_callback_t callback)
{
  if (plotInitStaticVariables() != GRM_ERROR_NONE) return 0;

  eventQueueRegister(event_queue, type, callback);

  return 1;
}

int grm_unregister(grm_event_type_t type)
{
  if (plotInitStaticVariables() != GRM_ERROR_NONE) return 0;

  eventQueueUnregister(event_queue, type);

  return 1;
}
