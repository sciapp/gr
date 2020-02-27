#ifdef __unix__
#define _POSIX_C_SOURCE 1
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

int process_events(void)
{
  int processed_events = 0;
  /* Trigger event handling routines after plotting -> args container is fully processed (and modified consistently) at
   * this time */
  if (!processing_events)
    {
      processing_events = 1; /* Ensure that event processing won't trigger event processing again */
      processed_events = event_queue_process_all(event_queue);
      processing_events = 0;
    }

  return processed_events;
}


/* ========================= methods ================================================================================ */

/* ------------------------- event handling ------------------------------------------------------------------------- */

DEFINE_LIST_METHODS(event)

error_t event_list_entry_copy(event_list_entry_t *copy, event_list_const_entry_t entry)
{
  event_list_entry_t _copy;

  _copy = malloc(sizeof(grm_event_t));
  if (_copy == NULL)
    {
      return ERROR_MALLOC;
    }

  memcpy(_copy, entry, sizeof(grm_event_t));
  *copy = _copy;

  return NO_ERROR;
}

error_t event_list_entry_delete(event_list_entry_t entry)
{
  free(entry);
  return NO_ERROR;
}

event_queue_t *event_queue_new(void)
{
  event_queue_t *queue = NULL;

  queue = malloc(sizeof(event_queue_t));
  error_cleanup_if(queue == NULL);
  queue->queue = NULL;
  queue->event_callbacks = NULL;
  queue->queue = event_reflist_new();
  error_cleanup_if(queue->queue == NULL);
  queue->event_callbacks = calloc(_GRM_EVENT_TYPE_COUNT, sizeof(grm_event_callback_t));
  error_cleanup_if(queue->event_callbacks == NULL);

  return queue;

error_cleanup:
  if (queue != NULL)
    {
      if (queue->queue != NULL)
        {
          event_reflist_delete(queue->queue);
        }
      if (queue->event_callbacks != NULL)
        {
          free(queue->event_callbacks);
        }
      free(queue);
    }

  return NULL;
}

void event_queue_delete(event_queue_t *queue)
{
  event_reflist_delete_with_entries(queue->queue);
  free(queue->event_callbacks);
  free(queue);
}

void event_queue_register(event_queue_t *queue, grm_event_type_t type, grm_event_callback_t callback)
{
  queue->event_callbacks[type] = callback;
}

void event_queue_unregister(event_queue_t *queue, grm_event_type_t type)
{
  queue->event_callbacks[type] = NULL;
}

int event_queue_process_next(event_queue_t *queue)
{
  grm_event_t *event;
  grm_event_type_t type;

  if (event_reflist_empty(queue->queue))
    {
      return 0;
    }

  event = event_reflist_dequeue(queue->queue);
  type = *((int *)event);
  if (queue->event_callbacks[type] != NULL)
    {
      queue->event_callbacks[type](event);
    }

  return 1;
}

int event_queue_process_all(event_queue_t *queue)
{

  if (event_reflist_empty(queue->queue))
    {
      return 0;
    }

  while (event_queue_process_next(queue))
    ;

  return 1;
}

error_t event_queue_enqueue_new_plot_event(event_queue_t *queue, int plot_id)
{
  grm_new_plot_event_t *new_plot_event = NULL;
  error_t error = NO_ERROR;

  new_plot_event = malloc(sizeof(grm_new_plot_event_t));
  error_cleanup_and_set_error_if(new_plot_event == NULL, ERROR_MALLOC);
  new_plot_event->type = GRM_EVENT_NEW_PLOT;
  new_plot_event->plot_id = plot_id;

  error = event_reflist_enqueue(queue->queue, (grm_event_t *)new_plot_event);
  error_cleanup_if_error;

  return NO_ERROR;

error_cleanup:
  if (new_plot_event != NULL)
    {
      free(new_plot_event);
    }

  return error;
}

error_t event_queue_enqueue_update_plot_event(event_queue_t *queue, int plot_id)
{
  grm_update_plot_event_t *update_plot_event = NULL;
  error_t error = NO_ERROR;

  update_plot_event = malloc(sizeof(grm_update_plot_event_t));
  error_cleanup_and_set_error_if(update_plot_event == NULL, ERROR_MALLOC);
  update_plot_event->type = GRM_EVENT_UPDATE_PLOT;
  update_plot_event->plot_id = plot_id;

  error = event_reflist_enqueue(queue->queue, (grm_event_t *)update_plot_event);
  error_cleanup_if_error;

  return NO_ERROR;

error_cleanup:
  if (update_plot_event != NULL)
    {
      free(update_plot_event);
    }

  return error;
}

error_t event_queue_enqueue_size_event(event_queue_t *queue, int plot_id, int width, int height)
{
  grm_size_event_t *size_event = NULL;
  error_t error = NO_ERROR;

  size_event = malloc(sizeof(grm_size_event_t));
  error_cleanup_and_set_error_if(size_event == NULL, ERROR_MALLOC);
  size_event->type = GRM_EVENT_SIZE;
  size_event->plot_id = plot_id;
  size_event->width = width;
  size_event->height = height;

  error = event_reflist_enqueue(queue->queue, (grm_event_t *)size_event);
  error_cleanup_if_error;

  return NO_ERROR;

error_cleanup:
  if (size_event != NULL)
    {
      free(size_event);
    }

  return error;
}

error_t event_queue_enqueue_merge_end_event(event_queue_t *queue, const char *identificator)
{
  grm_merge_end_event_t *merge_end_event = NULL;
  error_t error = NO_ERROR;

  merge_end_event = malloc(sizeof(grm_merge_end_event_t));
  error_cleanup_and_set_error_if(merge_end_event == NULL, ERROR_MALLOC);
  merge_end_event->type = GRM_EVENT_MERGE_END;
  merge_end_event->identificator = identificator;
  error = event_reflist_enqueue(queue->queue, (grm_event_t *)merge_end_event);
  error_cleanup_if_error;

  return NO_ERROR;

error_cleanup:
  if (merge_end_event != NULL)
    {
      free(merge_end_event);
    }

  return error;
}


#undef DEFINE_LIST_METHODS


/* ######################### public implementation ################################################################## */

/* ========================= methods ================================================================================ */

/* ------------------------- event handling ------------------------------------------------------------------------- */

int grm_register(grm_event_type_t type, grm_event_callback_t callback)
{
  if (plot_init_static_variables() != NO_ERROR)
    {
      return 0;
    }

  event_queue_register(event_queue, type, callback);

  return 1;
}

int grm_unregister(grm_event_type_t type)
{
  if (plot_init_static_variables() != NO_ERROR)
    {
      return 0;
    }

  event_queue_unregister(event_queue, type);

  return 1;
}
