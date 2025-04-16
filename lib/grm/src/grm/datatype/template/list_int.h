/* Do not use an include guard since the following macros may be deleted with `undef` between multiple includes */

/* ######################### includes ############################################################################### */

#include <assert.h>
#include <stdlib.h>

#include <grm/error_int.h>
#include <grm/util_int.h>


/* ######################### interface ############################################################################## */

/* ========================= datatypes ============================================================================== */

/* ------------------------- generic list --------------------------------------------------------------------------- */

#undef DECLARE_LIST_TYPE
#define DECLARE_LIST_TYPE(type_prefix, entry_type)                                                                     \
  typedef entry_type type_prefix##ListEntry;                                                                           \
  typedef const entry_type type_prefix##ListConstEntry;                                                                \
  typedef grm_error_t (*type_prefix##ListEntryCopyFunc)(type_prefix##ListEntry *, type_prefix##ListConstEntry);        \
  typedef grm_error_t (*type_prefix##ListEntryDeleteFunc)(type_prefix##ListEntry);                                     \
                                                                                                                       \
  typedef struct                                                                                                       \
  {                                                                                                                    \
    type_prefix##ListEntryCopyFunc entry_copy;                                                                         \
    type_prefix##ListEntryDeleteFunc entry_delete;                                                                     \
  } type_prefix##ListVtable;                                                                                           \
                                                                                                                       \
  typedef struct type_prefix##ListNode                                                                                 \
  {                                                                                                                    \
    type_prefix##ListEntry entry;                                                                                      \
    struct type_prefix##ListNode *next;                                                                                \
  } type_prefix##ListNode;                                                                                             \
                                                                                                                       \
  typedef struct                                                                                                       \
  {                                                                                                                    \
    const type_prefix##ListVtable *vt;                                                                                 \
    type_prefix##ListNode *head;                                                                                       \
    type_prefix##ListNode *tail;                                                                                       \
    size_t size;                                                                                                       \
  } type_prefix##List;                                                                                                 \
                                                                                                                       \
  /* ~~~~~~~~~~~~~~~~~~~~~~~ ref list ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */ \
                                                                                                                       \
  typedef entry_type type_prefix##ReflistEntry;                                                                        \
  typedef grm_error_t (*type_prefix##ReflistEntryCopyFunc)(type_prefix##ReflistEntry *, type_prefix##ReflistEntry);    \
  typedef grm_error_t (*type_prefix##ReflistEntryDeleteFunc)(type_prefix##ReflistEntry);                               \
                                                                                                                       \
  typedef struct                                                                                                       \
  {                                                                                                                    \
    type_prefix##ReflistEntryCopyFunc entry_copy;                                                                      \
    type_prefix##ReflistEntryDeleteFunc entry_delete;                                                                  \
  } type_prefix##ReflistVtable;                                                                                        \
                                                                                                                       \
  typedef struct type_prefix##ReflistNode                                                                              \
  {                                                                                                                    \
    type_prefix##ReflistEntry entry;                                                                                   \
    struct type_prefix##ReflistNode *next;                                                                             \
  } type_prefix##ReflistNode;                                                                                          \
                                                                                                                       \
  typedef struct                                                                                                       \
  {                                                                                                                    \
    const type_prefix##ReflistVtable *vt;                                                                              \
    type_prefix##ReflistNode *head;                                                                                    \
    type_prefix##ReflistNode *tail;                                                                                    \
    size_t size;                                                                                                       \
  } type_prefix##Reflist;


/* ========================= methods ================================================================================ */

/* ------------------------- generic list --------------------------------------------------------------------------- */

#undef DECLARE_LIST_METHODS
#define DECLARE_LIST_METHODS(type_prefix, method_prefix)                                                               \
  type_prefix##List *method_prefix##ListNew(void);                                                                     \
  void method_prefix##ListDelete(type_prefix##List *list);                                                             \
                                                                                                                       \
  grm_error_t method_prefix##ListPushFront(type_prefix##List *list, type_prefix##ListConstEntry entry);                \
  grm_error_t method_prefix##ListPushBack(type_prefix##List *list, type_prefix##ListConstEntry entry);                 \
                                                                                                                       \
  type_prefix##ListEntry method_prefix##ListPopFront(type_prefix##List *list);                                         \
  type_prefix##ListEntry method_prefix##ListPopBack(type_prefix##List *list);                                          \
                                                                                                                       \
  grm_error_t method_prefix##ListPush(type_prefix##List *list, type_prefix##ListConstEntry entry);                     \
  type_prefix##ListEntry method_prefix##ListPop(type_prefix##List *list);                                              \
                                                                                                                       \
  grm_error_t method_prefix##ListEnqueue(type_prefix##List *list, type_prefix##ListConstEntry entry);                  \
  type_prefix##ListEntry method_prefix##ListDequeue(type_prefix##List *list);                                          \
                                                                                                                       \
  int method_prefix##ListEmpty(type_prefix##List *list);                                                               \
                                                                                                                       \
  grm_error_t method_prefix##ListEntryCopy(type_prefix##ListEntry *copy, type_prefix##ListConstEntry entry);           \
  grm_error_t method_prefix##ListEntryDelete(type_prefix##ListEntry entry);                                            \
                                                                                                                       \
  int method_prefix##ListFindPreviousNode(const type_prefix##List *list, const type_prefix##ListNode *node,            \
                                          type_prefix##ListNode **previous_node);                                      \
                                                                                                                       \
  /* ~~~~~~~~~~~~~~~~~~~~~~~ ref list ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */ \
                                                                                                                       \
  type_prefix##Reflist *method_prefix##ReflistNew(void) MAYBE_UNUSED;                                                  \
  void method_prefix##ReflistDelete(type_prefix##Reflist *list) MAYBE_UNUSED;                                          \
  void method_prefix##ReflistDeleteWithEntries(type_prefix##Reflist *list) MAYBE_UNUSED;                               \
                                                                                                                       \
  grm_error_t method_prefix##ReflistPushFront(type_prefix##Reflist *list, type_prefix##ReflistEntry entry)             \
      MAYBE_UNUSED;                                                                                                    \
  grm_error_t method_prefix##ReflistPushBack(type_prefix##Reflist *list, type_prefix##ReflistEntry entry)              \
      MAYBE_UNUSED;                                                                                                    \
                                                                                                                       \
  type_prefix##ReflistEntry method_prefix##ReflistPopFront(type_prefix##Reflist *list) MAYBE_UNUSED;                   \
  type_prefix##ReflistEntry method_prefix##ReflistPopBack(type_prefix##Reflist *list) MAYBE_UNUSED;                    \
                                                                                                                       \
  grm_error_t method_prefix##ReflistPush(type_prefix##Reflist *list, type_prefix##ReflistEntry entry) MAYBE_UNUSED;    \
  type_prefix##ReflistEntry method_prefix##ReflistPop(type_prefix##Reflist *list) MAYBE_UNUSED;                        \
                                                                                                                       \
  grm_error_t method_prefix##ReflistEnqueue(type_prefix##Reflist *list, type_prefix##ReflistEntry entry) MAYBE_UNUSED; \
  type_prefix##ReflistEntry method_prefix##ReflistDequeue(type_prefix##Reflist *list) MAYBE_UNUSED;                    \
                                                                                                                       \
  int method_prefix##ReflistEmpty(type_prefix##Reflist *list) MAYBE_UNUSED;                                            \
                                                                                                                       \
  grm_error_t method_prefix##ReflistEntryCopy(type_prefix##ReflistEntry *copy, const type_prefix##ReflistEntry entry); \
  grm_error_t method_prefix##ReflistEntryDelete(type_prefix##ReflistEntry entry);                                      \
                                                                                                                       \
  int method_prefix##ReflistFindPreviousNode(const type_prefix##Reflist *list, const type_prefix##ReflistNode *node,   \
                                             type_prefix##ReflistNode **previous_node) MAYBE_UNUSED;


/* ######################### implementation ######################################################################### */

/* ========================= methods ================================================================================ */

/* ------------------------- generic list --------------------------------------------------------------------------- */

#undef DEFINE_LIST_METHODS
#define DEFINE_LIST_METHODS(type_prefix, method_prefix)                                                                \
  type_prefix##List *method_prefix##ListNew(void)                                                                      \
  {                                                                                                                    \
    static const type_prefix##ListVtable vt = {                                                                        \
        method_prefix##ListEntryCopy,                                                                                  \
        method_prefix##ListEntryDelete,                                                                                \
    };                                                                                                                 \
    type_prefix##List *list;                                                                                           \
                                                                                                                       \
    list = malloc(sizeof(type_prefix##List));                                                                          \
    if (list == NULL) return NULL;                                                                                     \
    list->vt = &vt;                                                                                                    \
    list->head = NULL;                                                                                                 \
    list->tail = NULL;                                                                                                 \
    list->size = 0;                                                                                                    \
                                                                                                                       \
    return list;                                                                                                       \
  }                                                                                                                    \
                                                                                                                       \
  void method_prefix##ListDelete(type_prefix##List *list)                                                              \
  {                                                                                                                    \
    type_prefix##ListNode *current_list_node;                                                                          \
    type_prefix##ListNode *next_list_node;                                                                             \
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
  grm_error_t method_prefix##ListPushFront(type_prefix##List *list, type_prefix##ListConstEntry entry)                 \
  {                                                                                                                    \
    type_prefix##ListNode *new_list_node;                                                                              \
    grm_error_t error = GRM_ERROR_NONE;                                                                                \
                                                                                                                       \
    new_list_node = malloc(sizeof(type_prefix##ListNode));                                                             \
    errorCleanupAndSetErrorIf(new_list_node == NULL, GRM_ERROR_MALLOC);                                                \
    error = list->vt->entry_copy(&new_list_node->entry, entry);                                                        \
    errorCleanupIfError;                                                                                               \
    new_list_node->next = list->head;                                                                                  \
    list->head = new_list_node;                                                                                        \
    if (list->tail == NULL) list->tail = new_list_node;                                                                \
    ++(list->size);                                                                                                    \
                                                                                                                       \
    return GRM_ERROR_NONE;                                                                                             \
                                                                                                                       \
  error_cleanup:                                                                                                       \
    free(new_list_node);                                                                                               \
    return error;                                                                                                      \
  }                                                                                                                    \
                                                                                                                       \
  grm_error_t method_prefix##ListPushBack(type_prefix##List *list, type_prefix##ListConstEntry entry)                  \
  {                                                                                                                    \
    type_prefix##ListNode *new_list_node;                                                                              \
    grm_error_t error = GRM_ERROR_NONE;                                                                                \
                                                                                                                       \
    new_list_node = malloc(sizeof(type_prefix##ListNode));                                                             \
    errorCleanupAndSetErrorIf(new_list_node == NULL, GRM_ERROR_MALLOC);                                                \
    error = list->vt->entry_copy(&new_list_node->entry, entry);                                                        \
    errorCleanupIfError;                                                                                               \
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
    return GRM_ERROR_NONE;                                                                                             \
                                                                                                                       \
  error_cleanup:                                                                                                       \
    free(new_list_node);                                                                                               \
    return error;                                                                                                      \
  }                                                                                                                    \
                                                                                                                       \
  type_prefix##ListEntry method_prefix##ListPopFront(type_prefix##List *list)                                          \
  {                                                                                                                    \
    type_prefix##ListNode *front_node;                                                                                 \
    type_prefix##ListEntry front_entry;                                                                                \
                                                                                                                       \
    assert(list->head != NULL);                                                                                        \
    front_node = list->head;                                                                                           \
    list->head = list->head->next;                                                                                     \
    if (list->tail == front_node) list->tail = NULL;                                                                   \
    front_entry = front_node->entry;                                                                                   \
    free(front_node);                                                                                                  \
    --(list->size);                                                                                                    \
                                                                                                                       \
    return front_entry;                                                                                                \
  }                                                                                                                    \
                                                                                                                       \
  type_prefix##ListEntry method_prefix##ListPopBack(type_prefix##List *list)                                           \
  {                                                                                                                    \
    type_prefix##ListNode *last_node;                                                                                  \
    type_prefix##ListNode *next_to_last_node = NULL;                                                                   \
    type_prefix##ListEntry last_entry;                                                                                 \
                                                                                                                       \
    assert(list->tail != NULL);                                                                                        \
    last_node = list->tail;                                                                                            \
    method_prefix##ListFindPreviousNode(list, last_node, &next_to_last_node);                                          \
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
  grm_error_t method_prefix##ListPush(type_prefix##List *list, type_prefix##ListConstEntry entry)                      \
  {                                                                                                                    \
    return method_prefix##ListPushFront(list, entry);                                                                  \
  }                                                                                                                    \
                                                                                                                       \
  type_prefix##ListEntry method_prefix##ListPop(type_prefix##List *list) { return method_prefix##ListPopFront(list); } \
                                                                                                                       \
  grm_error_t method_prefix##ListEnqueue(type_prefix##List *list, type_prefix##ListConstEntry entry)                   \
  {                                                                                                                    \
    return method_prefix##ListPushBack(list, entry);                                                                   \
  }                                                                                                                    \
                                                                                                                       \
  type_prefix##ListEntry method_prefix##ListDequeue(type_prefix##List *list)                                           \
  {                                                                                                                    \
    return method_prefix##ListPopFront(list);                                                                          \
  }                                                                                                                    \
                                                                                                                       \
  int method_prefix##ListEmpty(type_prefix##List *list) { return list->size == 0; }                                    \
                                                                                                                       \
  int method_prefix##ListFindPreviousNode(const type_prefix##List *list, const type_prefix##ListNode *node,            \
                                          type_prefix##ListNode **previous_node)                                       \
  {                                                                                                                    \
    type_prefix##ListNode *prev_node;                                                                                  \
    type_prefix##ListNode *current_node;                                                                               \
                                                                                                                       \
    prev_node = NULL;                                                                                                  \
    current_node = list->head;                                                                                         \
    while (current_node != NULL)                                                                                       \
      {                                                                                                                \
        if (current_node == node)                                                                                      \
          {                                                                                                            \
            if (previous_node != NULL) *previous_node = prev_node;                                                     \
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
  type_prefix##Reflist *method_prefix##ReflistNew(void)                                                                \
  {                                                                                                                    \
    static const type_prefix##ReflistVtable vt = {                                                                     \
        method_prefix##ReflistEntryCopy,                                                                               \
        method_prefix##ReflistEntryDelete,                                                                             \
    };                                                                                                                 \
    type_prefix##Reflist *list;                                                                                        \
                                                                                                                       \
    list = (type_prefix##Reflist *)method_prefix##ListNew();                                                           \
    list->vt = &vt;                                                                                                    \
                                                                                                                       \
    return list;                                                                                                       \
  }                                                                                                                    \
                                                                                                                       \
  void method_prefix##ReflistDelete(type_prefix##Reflist *list)                                                        \
  {                                                                                                                    \
    method_prefix##ListDelete((type_prefix##List *)list);                                                              \
  }                                                                                                                    \
                                                                                                                       \
  void method_prefix##ReflistDeleteWithEntries(type_prefix##Reflist *list)                                             \
  {                                                                                                                    \
    type_prefix##ReflistNode *current_reflist_node;                                                                    \
    type_prefix##ReflistNode *next_reflist_node;                                                                       \
                                                                                                                       \
    current_reflist_node = list->head;                                                                                 \
    while (current_reflist_node != NULL)                                                                               \
      {                                                                                                                \
        next_reflist_node = current_reflist_node->next;                                                                \
        method_prefix##ListEntryDelete((type_prefix##ListEntry)current_reflist_node->entry);                           \
        free(current_reflist_node);                                                                                    \
        current_reflist_node = next_reflist_node;                                                                      \
      }                                                                                                                \
    free(list);                                                                                                        \
  }                                                                                                                    \
                                                                                                                       \
  grm_error_t method_prefix##ReflistPushFront(type_prefix##Reflist *list, type_prefix##ReflistEntry entry)             \
  {                                                                                                                    \
    return method_prefix##ListPushFront((type_prefix##List *)list, (type_prefix##ListEntry)entry);                     \
  }                                                                                                                    \
                                                                                                                       \
  grm_error_t method_prefix##ReflistPushBack(type_prefix##Reflist *list, type_prefix##ReflistEntry entry)              \
  {                                                                                                                    \
    return method_prefix##ListPushBack((type_prefix##List *)list, (type_prefix##ListEntry)entry);                      \
  }                                                                                                                    \
                                                                                                                       \
  type_prefix##ReflistEntry method_prefix##ReflistPopFront(type_prefix##Reflist *list)                                 \
  {                                                                                                                    \
    return method_prefix##ListPopFront((type_prefix##List *)list);                                                     \
  }                                                                                                                    \
                                                                                                                       \
  type_prefix##ReflistEntry method_prefix##ReflistPopBack(type_prefix##Reflist *list)                                  \
  {                                                                                                                    \
    return method_prefix##ListPopBack((type_prefix##List *)list);                                                      \
  }                                                                                                                    \
                                                                                                                       \
  grm_error_t method_prefix##ReflistPush(type_prefix##Reflist *list, type_prefix##ReflistEntry entry)                  \
  {                                                                                                                    \
    return method_prefix##ListPush((type_prefix##List *)list, (type_prefix##ListEntry)entry);                          \
  }                                                                                                                    \
                                                                                                                       \
  type_prefix##ReflistEntry method_prefix##ReflistPop(type_prefix##Reflist *list)                                      \
  {                                                                                                                    \
    return method_prefix##ListPop((type_prefix##List *)list);                                                          \
  }                                                                                                                    \
                                                                                                                       \
  grm_error_t method_prefix##ReflistEnqueue(type_prefix##Reflist *list, type_prefix##ReflistEntry entry)               \
  {                                                                                                                    \
    return method_prefix##ListEnqueue((type_prefix##List *)list, (type_prefix##ListEntry)entry);                       \
  }                                                                                                                    \
                                                                                                                       \
  type_prefix##ReflistEntry method_prefix##ReflistDequeue(type_prefix##Reflist *list)                                  \
  {                                                                                                                    \
    return method_prefix##ListDequeue((type_prefix##List *)list);                                                      \
  }                                                                                                                    \
                                                                                                                       \
  int method_prefix##ReflistEmpty(type_prefix##Reflist *list)                                                          \
  {                                                                                                                    \
    return method_prefix##ListEmpty((type_prefix##List *)list);                                                        \
  }                                                                                                                    \
                                                                                                                       \
                                                                                                                       \
  int method_prefix##ReflistFindPreviousNode(const type_prefix##Reflist *list, const type_prefix##ReflistNode *node,   \
                                             type_prefix##ReflistNode **previous_node)                                 \
  {                                                                                                                    \
    return method_prefix##ListFindPreviousNode((type_prefix##List *)list, (type_prefix##ListNode *)node,               \
                                               (type_prefix##ListNode **)previous_node);                               \
  }                                                                                                                    \
                                                                                                                       \
  grm_error_t method_prefix##ReflistEntryCopy(type_prefix##ReflistEntry *copy, type_prefix##ReflistEntry entry)        \
  {                                                                                                                    \
    *copy = entry;                                                                                                     \
    return GRM_ERROR_NONE;                                                                                             \
  }                                                                                                                    \
                                                                                                                       \
  grm_error_t method_prefix##ReflistEntryDelete(type_prefix##ReflistEntry entry UNUSED) { return GRM_ERROR_NONE; }
