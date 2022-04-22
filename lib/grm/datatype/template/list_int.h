/* Do not use an include guard since the following macros may be deleted with `undef` between multiple includes */

/* ######################### includes ############################################################################### */

#include <assert.h>
#include <stdlib.h>

#include "error_int.h"
#include "util_int.h"


/* ######################### interface ############################################################################## */

/* ========================= datatypes ============================================================================== */

/* ------------------------- generic list --------------------------------------------------------------------------- */

#undef DECLARE_LIST_TYPE
#define DECLARE_LIST_TYPE(prefix, entry_type)                                                                          \
  typedef entry_type prefix##_list_entry_t;                                                                            \
  typedef const entry_type prefix##_list_const_entry_t;                                                                \
  typedef err_t (*prefix##_list_entry_copy_func_t)(prefix##_list_entry_t *, prefix##_list_const_entry_t);              \
  typedef err_t (*prefix##_list_entry_delete_func_t)(prefix##_list_entry_t);                                           \
                                                                                                                       \
  typedef struct                                                                                                       \
  {                                                                                                                    \
    prefix##_list_entry_copy_func_t entry_copy;                                                                        \
    prefix##_list_entry_delete_func_t entry_delete;                                                                    \
  } prefix##_list_vtable_t;                                                                                            \
                                                                                                                       \
  typedef struct _##prefix##_list_node_t                                                                               \
  {                                                                                                                    \
    prefix##_list_entry_t entry;                                                                                       \
    struct _##prefix##_list_node_t *next;                                                                              \
  } prefix##_list_node_t;                                                                                              \
                                                                                                                       \
  typedef struct                                                                                                       \
  {                                                                                                                    \
    const prefix##_list_vtable_t *vt;                                                                                  \
    prefix##_list_node_t *head;                                                                                        \
    prefix##_list_node_t *tail;                                                                                        \
    size_t size;                                                                                                       \
  } prefix##_list_t;                                                                                                   \
                                                                                                                       \
  /* ~~~~~~~~~~~~~~~~~~~~~~~ ref list ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */ \
                                                                                                                       \
  typedef entry_type prefix##_reflist_entry_t;                                                                         \
  typedef err_t (*prefix##_reflist_entry_copy_func_t)(prefix##_reflist_entry_t *, prefix##_reflist_entry_t);           \
  typedef err_t (*prefix##_reflist_entry_delete_func_t)(prefix##_reflist_entry_t);                                     \
                                                                                                                       \
  typedef struct                                                                                                       \
  {                                                                                                                    \
    prefix##_reflist_entry_copy_func_t entry_copy;                                                                     \
    prefix##_reflist_entry_delete_func_t entry_delete;                                                                 \
  } prefix##_reflist_vtable_t;                                                                                         \
                                                                                                                       \
  typedef struct _##prefix##_reflist_node_t                                                                            \
  {                                                                                                                    \
    prefix##_reflist_entry_t entry;                                                                                    \
    struct _##prefix##_reflist_node_t *next;                                                                           \
  } prefix##_reflist_node_t;                                                                                           \
                                                                                                                       \
  typedef struct                                                                                                       \
  {                                                                                                                    \
    const prefix##_reflist_vtable_t *vt;                                                                               \
    prefix##_reflist_node_t *head;                                                                                     \
    prefix##_reflist_node_t *tail;                                                                                     \
    size_t size;                                                                                                       \
  } prefix##_reflist_t;


/* ========================= methods ================================================================================ */

/* ------------------------- generic list --------------------------------------------------------------------------- */

#undef DECLARE_LIST_METHODS
#define DECLARE_LIST_METHODS(prefix)                                                                                   \
  prefix##_list_t *prefix##_list_new(void);                                                                            \
  void prefix##_list_delete(prefix##_list_t *list);                                                                    \
                                                                                                                       \
  err_t prefix##_list_push_front(prefix##_list_t *list, prefix##_list_const_entry_t entry);                            \
  err_t prefix##_list_push_back(prefix##_list_t *list, prefix##_list_const_entry_t entry);                             \
                                                                                                                       \
  prefix##_list_entry_t prefix##_list_pop_front(prefix##_list_t *list);                                                \
  prefix##_list_entry_t prefix##_list_pop_back(prefix##_list_t *list);                                                 \
                                                                                                                       \
  err_t prefix##_list_push(prefix##_list_t *list, prefix##_list_const_entry_t entry);                                  \
  prefix##_list_entry_t prefix##_list_pop(prefix##_list_t *list);                                                      \
                                                                                                                       \
  err_t prefix##_list_enqueue(prefix##_list_t *list, prefix##_list_const_entry_t entry);                               \
  prefix##_list_entry_t prefix##_list_dequeue(prefix##_list_t *list);                                                  \
                                                                                                                       \
  int prefix##_list_empty(prefix##_list_t *list);                                                                      \
                                                                                                                       \
  err_t prefix##_list_entry_copy(prefix##_list_entry_t *copy, prefix##_list_const_entry_t entry);                      \
  err_t prefix##_list_entry_delete(prefix##_list_entry_t entry);                                                       \
                                                                                                                       \
  int prefix##_list_find_previous_node(const prefix##_list_t *list, const prefix##_list_node_t *node,                  \
                                       prefix##_list_node_t **previous_node);                                          \
                                                                                                                       \
  /* ~~~~~~~~~~~~~~~~~~~~~~~ ref list ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */ \
                                                                                                                       \
  prefix##_reflist_t *prefix##_reflist_new(void) MAYBE_UNUSED;                                                         \
  void prefix##_reflist_delete(prefix##_reflist_t *list) MAYBE_UNUSED;                                                 \
  void prefix##_reflist_delete_with_entries(prefix##_reflist_t *list) MAYBE_UNUSED;                                    \
                                                                                                                       \
  err_t prefix##_reflist_push_front(prefix##_reflist_t *list, prefix##_reflist_entry_t entry) MAYBE_UNUSED;            \
  err_t prefix##_reflist_push_back(prefix##_reflist_t *list, prefix##_reflist_entry_t entry) MAYBE_UNUSED;             \
                                                                                                                       \
  prefix##_reflist_entry_t prefix##_reflist_pop_front(prefix##_reflist_t *list) MAYBE_UNUSED;                          \
  prefix##_reflist_entry_t prefix##_reflist_pop_back(prefix##_reflist_t *list) MAYBE_UNUSED;                           \
                                                                                                                       \
  err_t prefix##_reflist_push(prefix##_reflist_t *list, prefix##_reflist_entry_t entry) MAYBE_UNUSED;                  \
  prefix##_reflist_entry_t prefix##_reflist_pop(prefix##_reflist_t *list) MAYBE_UNUSED;                                \
                                                                                                                       \
  err_t prefix##_reflist_enqueue(prefix##_reflist_t *list, prefix##_reflist_entry_t entry) MAYBE_UNUSED;               \
  prefix##_reflist_entry_t prefix##_reflist_dequeue(prefix##_reflist_t *list) MAYBE_UNUSED;                            \
                                                                                                                       \
  int prefix##_reflist_empty(prefix##_reflist_t *list) MAYBE_UNUSED;                                                   \
                                                                                                                       \
  err_t prefix##_reflist_entry_copy(prefix##_reflist_entry_t *copy, const prefix##_reflist_entry_t entry);             \
  err_t prefix##_reflist_entry_delete(prefix##_reflist_entry_t entry);                                                 \
                                                                                                                       \
  int prefix##_reflist_find_previous_node(const prefix##_reflist_t *list, const prefix##_reflist_node_t *node,         \
                                          prefix##_reflist_node_t **previous_node) MAYBE_UNUSED;


/* ######################### implementation ######################################################################### */

/* ========================= methods ================================================================================ */

/* ------------------------- generic list --------------------------------------------------------------------------- */

#undef DEFINE_LIST_METHODS
#define DEFINE_LIST_METHODS(prefix)                                                                                    \
  prefix##_list_t *prefix##_list_new(void)                                                                             \
  {                                                                                                                    \
    static const prefix##_list_vtable_t vt = {                                                                         \
        prefix##_list_entry_copy,                                                                                      \
        prefix##_list_entry_delete,                                                                                    \
    };                                                                                                                 \
    prefix##_list_t *list;                                                                                             \
                                                                                                                       \
    list = malloc(sizeof(prefix##_list_t));                                                                            \
    if (list == NULL)                                                                                                  \
      {                                                                                                                \
        return NULL;                                                                                                   \
      }                                                                                                                \
    list->vt = &vt;                                                                                                    \
    list->head = NULL;                                                                                                 \
    list->tail = NULL;                                                                                                 \
    list->size = 0;                                                                                                    \
                                                                                                                       \
    return list;                                                                                                       \
  }                                                                                                                    \
                                                                                                                       \
  void prefix##_list_delete(prefix##_list_t *list)                                                                     \
  {                                                                                                                    \
    prefix##_list_node_t *current_list_node;                                                                           \
    prefix##_list_node_t *next_list_node;                                                                              \
                                                                                                                       \
    current_list_node = list->head;                                                                                    \
    while (current_list_node != NULL)                                                                                  \
      {                                                                                                                \
        next_list_node = current_list_node->next;                                                                      \
        list->vt->entry_delete(current_list_node->entry);                                                              \
        free(current_list_node);                                                                                       \
        current_list_node = next_list_node;                                                                            \
      }                                                                                                                \
    free(list);                                                                                                        \
  }                                                                                                                    \
                                                                                                                       \
  err_t prefix##_list_push_front(prefix##_list_t *list, prefix##_list_const_entry_t entry)                             \
  {                                                                                                                    \
    prefix##_list_node_t *new_list_node;                                                                               \
    err_t error = ERROR_NONE;                                                                                          \
                                                                                                                       \
    new_list_node = malloc(sizeof(prefix##_list_node_t));                                                              \
    error_cleanup_and_set_error_if(new_list_node == NULL, ERROR_MALLOC);                                               \
    error = list->vt->entry_copy(&new_list_node->entry, entry);                                                        \
    error_cleanup_if_error;                                                                                            \
    new_list_node->next = list->head;                                                                                  \
    list->head = new_list_node;                                                                                        \
    if (list->tail == NULL)                                                                                            \
      {                                                                                                                \
        list->tail = new_list_node;                                                                                    \
      }                                                                                                                \
    ++(list->size);                                                                                                    \
                                                                                                                       \
    return ERROR_NONE;                                                                                                 \
                                                                                                                       \
  error_cleanup:                                                                                                       \
    free(new_list_node);                                                                                               \
    return error;                                                                                                      \
  }                                                                                                                    \
                                                                                                                       \
  err_t prefix##_list_push_back(prefix##_list_t *list, prefix##_list_const_entry_t entry)                              \
  {                                                                                                                    \
    prefix##_list_node_t *new_list_node;                                                                               \
    err_t error = ERROR_NONE;                                                                                          \
                                                                                                                       \
    new_list_node = malloc(sizeof(prefix##_list_node_t));                                                              \
    error_cleanup_and_set_error_if(new_list_node == NULL, ERROR_MALLOC);                                               \
    error = list->vt->entry_copy(&new_list_node->entry, entry);                                                        \
    error_cleanup_if_error;                                                                                            \
    new_list_node->next = NULL;                                                                                        \
    if (list->head == NULL)                                                                                            \
      {                                                                                                                \
        list->head = new_list_node;                                                                                    \
      }                                                                                                                \
    else                                                                                                               \
      {                                                                                                                \
        list->tail->next = new_list_node;                                                                              \
      }                                                                                                                \
    list->tail = new_list_node;                                                                                        \
    ++(list->size);                                                                                                    \
                                                                                                                       \
    return ERROR_NONE;                                                                                                 \
                                                                                                                       \
  error_cleanup:                                                                                                       \
    free(new_list_node);                                                                                               \
    return error;                                                                                                      \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_list_entry_t prefix##_list_pop_front(prefix##_list_t *list)                                                 \
  {                                                                                                                    \
    prefix##_list_node_t *front_node;                                                                                  \
    prefix##_list_entry_t front_entry;                                                                                 \
                                                                                                                       \
    assert(list->head != NULL);                                                                                        \
    front_node = list->head;                                                                                           \
    list->head = list->head->next;                                                                                     \
    if (list->tail == front_node)                                                                                      \
      {                                                                                                                \
        list->tail = NULL;                                                                                             \
      }                                                                                                                \
    front_entry = front_node->entry;                                                                                   \
    free(front_node);                                                                                                  \
    --(list->size);                                                                                                    \
                                                                                                                       \
    return front_entry;                                                                                                \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_list_entry_t prefix##_list_pop_back(prefix##_list_t *list)                                                  \
  {                                                                                                                    \
    prefix##_list_node_t *last_node;                                                                                   \
    prefix##_list_node_t *next_to_last_node = NULL;                                                                    \
    prefix##_list_entry_t last_entry;                                                                                  \
                                                                                                                       \
    assert(list->tail != NULL);                                                                                        \
    last_node = list->tail;                                                                                            \
    prefix##_list_find_previous_node(list, last_node, &next_to_last_node);                                             \
    if (next_to_last_node == NULL)                                                                                     \
      {                                                                                                                \
        list->head = list->tail = NULL;                                                                                \
      }                                                                                                                \
    else                                                                                                               \
      {                                                                                                                \
        list->tail = next_to_last_node;                                                                                \
        next_to_last_node->next = NULL;                                                                                \
      }                                                                                                                \
    last_entry = last_node->entry;                                                                                     \
    free(last_node);                                                                                                   \
    --(list->size);                                                                                                    \
                                                                                                                       \
    return last_entry;                                                                                                 \
  }                                                                                                                    \
                                                                                                                       \
  err_t prefix##_list_push(prefix##_list_t *list, prefix##_list_const_entry_t entry)                                   \
  {                                                                                                                    \
    return prefix##_list_push_front(list, entry);                                                                      \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_list_entry_t prefix##_list_pop(prefix##_list_t *list) { return prefix##_list_pop_front(list); }             \
                                                                                                                       \
  err_t prefix##_list_enqueue(prefix##_list_t *list, prefix##_list_const_entry_t entry)                                \
  {                                                                                                                    \
    return prefix##_list_push_back(list, entry);                                                                       \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_list_entry_t prefix##_list_dequeue(prefix##_list_t *list) { return prefix##_list_pop_front(list); }         \
                                                                                                                       \
  int prefix##_list_empty(prefix##_list_t *list) { return list->size == 0; }                                           \
                                                                                                                       \
  int prefix##_list_find_previous_node(const prefix##_list_t *list, const prefix##_list_node_t *node,                  \
                                       prefix##_list_node_t **previous_node)                                           \
  {                                                                                                                    \
    prefix##_list_node_t *prev_node;                                                                                   \
    prefix##_list_node_t *current_node;                                                                                \
                                                                                                                       \
    prev_node = NULL;                                                                                                  \
    current_node = list->head;                                                                                         \
    while (current_node != NULL)                                                                                       \
      {                                                                                                                \
        if (current_node == node)                                                                                      \
          {                                                                                                            \
            if (previous_node != NULL)                                                                                 \
              {                                                                                                        \
                *previous_node = prev_node;                                                                            \
              }                                                                                                        \
            return 1;                                                                                                  \
          }                                                                                                            \
        prev_node = current_node;                                                                                      \
        current_node = current_node->next;                                                                             \
      }                                                                                                                \
                                                                                                                       \
    return 0;                                                                                                          \
  }                                                                                                                    \
                                                                                                                       \
  /* ~~~~~~~~~~~~~~~~~~~~~~~ ref list ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */ \
                                                                                                                       \
  prefix##_reflist_t *prefix##_reflist_new(void)                                                                       \
  {                                                                                                                    \
    static const prefix##_reflist_vtable_t vt = {                                                                      \
        prefix##_reflist_entry_copy,                                                                                   \
        prefix##_reflist_entry_delete,                                                                                 \
    };                                                                                                                 \
    prefix##_reflist_t *list;                                                                                          \
                                                                                                                       \
    list = (prefix##_reflist_t *)prefix##_list_new();                                                                  \
    list->vt = &vt;                                                                                                    \
                                                                                                                       \
    return list;                                                                                                       \
  }                                                                                                                    \
                                                                                                                       \
  void prefix##_reflist_delete(prefix##_reflist_t *list) { prefix##_list_delete((prefix##_list_t *)list); }            \
                                                                                                                       \
  void prefix##_reflist_delete_with_entries(prefix##_reflist_t *list)                                                  \
  {                                                                                                                    \
    prefix##_reflist_node_t *current_reflist_node;                                                                     \
    prefix##_reflist_node_t *next_reflist_node;                                                                        \
                                                                                                                       \
    current_reflist_node = list->head;                                                                                 \
    while (current_reflist_node != NULL)                                                                               \
      {                                                                                                                \
        next_reflist_node = current_reflist_node->next;                                                                \
        prefix##_list_entry_delete((prefix##_list_entry_t)current_reflist_node->entry);                                \
        free(current_reflist_node);                                                                                    \
        current_reflist_node = next_reflist_node;                                                                      \
      }                                                                                                                \
    free(list);                                                                                                        \
  }                                                                                                                    \
                                                                                                                       \
  err_t prefix##_reflist_push_front(prefix##_reflist_t *list, prefix##_reflist_entry_t entry)                          \
  {                                                                                                                    \
    return prefix##_list_push_front((prefix##_list_t *)list, (prefix##_list_entry_t)entry);                            \
  }                                                                                                                    \
                                                                                                                       \
  err_t prefix##_reflist_push_back(prefix##_reflist_t *list, prefix##_reflist_entry_t entry)                           \
  {                                                                                                                    \
    return prefix##_list_push_back((prefix##_list_t *)list, (prefix##_list_entry_t)entry);                             \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_reflist_entry_t prefix##_reflist_pop_front(prefix##_reflist_t *list)                                        \
  {                                                                                                                    \
    return prefix##_list_pop_front((prefix##_list_t *)list);                                                           \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_reflist_entry_t prefix##_reflist_pop_back(prefix##_reflist_t *list)                                         \
  {                                                                                                                    \
    return prefix##_list_pop_back((prefix##_list_t *)list);                                                            \
  }                                                                                                                    \
                                                                                                                       \
  err_t prefix##_reflist_push(prefix##_reflist_t *list, prefix##_reflist_entry_t entry)                                \
  {                                                                                                                    \
    return prefix##_list_push((prefix##_list_t *)list, (prefix##_list_entry_t)entry);                                  \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_reflist_entry_t prefix##_reflist_pop(prefix##_reflist_t *list)                                              \
  {                                                                                                                    \
    return prefix##_list_pop((prefix##_list_t *)list);                                                                 \
  }                                                                                                                    \
                                                                                                                       \
  err_t prefix##_reflist_enqueue(prefix##_reflist_t *list, prefix##_reflist_entry_t entry)                             \
  {                                                                                                                    \
    return prefix##_list_enqueue((prefix##_list_t *)list, (prefix##_list_entry_t)entry);                               \
  }                                                                                                                    \
                                                                                                                       \
  prefix##_reflist_entry_t prefix##_reflist_dequeue(prefix##_reflist_t *list)                                          \
  {                                                                                                                    \
    return prefix##_list_dequeue((prefix##_list_t *)list);                                                             \
  }                                                                                                                    \
                                                                                                                       \
  int prefix##_reflist_empty(prefix##_reflist_t *list) { return prefix##_list_empty((prefix##_list_t *)list); }        \
                                                                                                                       \
                                                                                                                       \
  int prefix##_reflist_find_previous_node(const prefix##_reflist_t *list, const prefix##_reflist_node_t *node,         \
                                          prefix##_reflist_node_t **previous_node)                                     \
  {                                                                                                                    \
    return prefix##_list_find_previous_node((prefix##_list_t *)list, (prefix##_list_node_t *)node,                     \
                                            (prefix##_list_node_t **)previous_node);                                   \
  }                                                                                                                    \
                                                                                                                       \
  err_t prefix##_reflist_entry_copy(prefix##_reflist_entry_t *copy, prefix##_reflist_entry_t entry)                    \
  {                                                                                                                    \
    *copy = entry;                                                                                                     \
    return ERROR_NONE;                                                                                                 \
  }                                                                                                                    \
                                                                                                                       \
  err_t prefix##_reflist_entry_delete(prefix##_reflist_entry_t entry UNUSED) { return ERROR_NONE; }
