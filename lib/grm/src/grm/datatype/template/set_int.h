/* Do not use an include guard since the following macros may be deleted with `undef` between multiple includes */

/* ######################### includes ############################################################################### */

#include <stdlib.h>
#include <sys/types.h>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include <grm/error_int.h>
#include <grm/logging_int.h>
#include <grm/util_int.h>


/* ######################### interface ############################################################################## */

/* ========================= datatypes ============================================================================== */

/* ------------------------- generic set ---------------------------------------------------------------------------- */

#undef DECLARE_SET_TYPE
#define DECLARE_SET_TYPE(type_prefix, entry_type)      \
  typedef entry_type type_prefix##SetEntry;            \
  typedef const entry_type type_prefix##SetConstEntry; \
                                                       \
  typedef struct                                       \
  {                                                    \
    type_prefix##SetEntry *set;                        \
    unsigned char *used;                               \
    size_t capacity;                                   \
    size_t size;                                       \
  } type_prefix##Set;


/* ========================= methods ================================================================================ */

#undef DECLARE_SET_METHODS
#define DECLARE_SET_METHODS(type_prefix, method_prefix)                                                            \
  type_prefix##Set *method_prefix##SetNew(size_t capacity) MAYBE_UNUSED;                                           \
  type_prefix##Set *method_prefix##SetNewWithData(size_t count, type_prefix##SetConstEntry *entries) MAYBE_UNUSED; \
  type_prefix##Set *method_prefix##SetCopy(const type_prefix##Set *set) MAYBE_UNUSED;                              \
  void method_prefix##SetDelete(type_prefix##Set *set);                                                            \
  int method_prefix##SetAdd(type_prefix##Set *set, type_prefix##SetConstEntry entry) MAYBE_UNUSED;                 \
  int method_prefix##SetFind(const type_prefix##Set *set, type_prefix##SetConstEntry entry,                        \
                             type_prefix##SetEntry *saved_entry) MAYBE_UNUSED;                                     \
  int method_prefix##SetContains(const type_prefix##Set *set, type_prefix##SetConstEntry entry) MAYBE_UNUSED;      \
  ssize_t method_prefix##SetIndex(const type_prefix##Set *set, type_prefix##SetConstEntry entry) MAYBE_UNUSED;     \
                                                                                                                   \
  int method_prefix##SetEntryCopy(type_prefix##SetEntry *copy, type_prefix##SetConstEntry entry);                  \
  void method_prefix##SetEntryDelete(type_prefix##SetEntry entry);                                                 \
  size_t method_prefix##SetEntryHash(type_prefix##SetConstEntry entry);                                            \
  int method_prefix##SetEntryEquals(type_prefix##SetConstEntry entry1, type_prefix##SetConstEntry entry2);


/* ######################### implementation ######################################################################### */

/* ========================= methods ================================================================================ */

/* ------------------------- generic set ---------------------------------------------------------------------------- */

#undef DEFINE_SET_METHODS
#define DEFINE_SET_METHODS(type_prefix, method_prefix)                                                               \
  type_prefix##Set *method_prefix##SetNew(size_t capacity)                                                           \
  {                                                                                                                  \
    type_prefix##Set *set = NULL;                                                                                    \
    size_t power2_capacity = 1;                                                                                      \
                                                                                                                     \
    /* Use the power of 2 which is equal or greater than 2*capacity as the set capacity */                           \
    power2_capacity = nextOrEqualPower2(2 * capacity);                                                               \
    set = (type_prefix##Set *)malloc(sizeof(type_prefix##Set));                                                      \
    if (set == NULL)                                                                                                 \
      {                                                                                                              \
        debugPrintMallocError();                                                                                     \
        goto error_cleanup;                                                                                          \
      }                                                                                                              \
    set->set = NULL;                                                                                                 \
    set->used = NULL;                                                                                                \
    set->set = (type_prefix##SetEntry *)malloc(power2_capacity * sizeof(type_prefix##SetEntry));                     \
    if (set->set == NULL)                                                                                            \
      {                                                                                                              \
        debugPrintMallocError();                                                                                     \
        goto error_cleanup;                                                                                          \
      }                                                                                                              \
    set->used = (unsigned char *)calloc(power2_capacity, sizeof(unsigned char));                                     \
    if (set->used == NULL)                                                                                           \
      {                                                                                                              \
        debugPrintMallocError();                                                                                     \
        goto error_cleanup;                                                                                          \
      }                                                                                                              \
    set->capacity = power2_capacity;                                                                                 \
    set->size = 0;                                                                                                   \
                                                                                                                     \
    logger((stderr, "Created a new set with capacity: %lu\n", set->capacity));                                       \
                                                                                                                     \
    return set;                                                                                                      \
                                                                                                                     \
  error_cleanup:                                                                                                     \
    if (set != NULL)                                                                                                 \
      {                                                                                                              \
        if (set->set != NULL) free(set->set);                                                                        \
        if (set->used != NULL) free(set->used);                                                                      \
        free(set);                                                                                                   \
      }                                                                                                              \
                                                                                                                     \
    return NULL;                                                                                                     \
  }                                                                                                                  \
                                                                                                                     \
  type_prefix##Set *method_prefix##SetNewWithData(size_t count, type_prefix##SetConstEntry *entries)                 \
  {                                                                                                                  \
    type_prefix##Set *set;                                                                                           \
    size_t i;                                                                                                        \
                                                                                                                     \
    set = method_prefix##SetNew(count);                                                                              \
    if (set == NULL) return NULL;                                                                                    \
    for (i = 0; i < count; ++i)                                                                                      \
      {                                                                                                              \
        if (!method_prefix##SetAdd(set, entries[i]))                                                                 \
          {                                                                                                          \
            method_prefix##SetDelete(set);                                                                           \
            return NULL;                                                                                             \
          }                                                                                                          \
      }                                                                                                              \
                                                                                                                     \
    return set;                                                                                                      \
  }                                                                                                                  \
                                                                                                                     \
  type_prefix##Set *method_prefix##SetCopy(const type_prefix##Set *set)                                              \
  {                                                                                                                  \
    type_prefix##Set *copy;                                                                                          \
    size_t i;                                                                                                        \
                                                                                                                     \
    copy = method_prefix##SetNew(set->size);                                                                         \
    if (copy == NULL) return NULL;                                                                                   \
    for (i = 0; i < set->capacity; ++i)                                                                              \
      {                                                                                                              \
        if (set->used[i] && !method_prefix##SetAdd(copy, set->set[i]))                                               \
          {                                                                                                          \
            method_prefix##SetDelete(copy);                                                                          \
            return NULL;                                                                                             \
          }                                                                                                          \
      }                                                                                                              \
                                                                                                                     \
    return copy;                                                                                                     \
  }                                                                                                                  \
                                                                                                                     \
  void method_prefix##SetDelete(type_prefix##Set *set)                                                               \
  {                                                                                                                  \
    size_t i;                                                                                                        \
    for (i = 0; i < set->capacity; ++i)                                                                              \
      {                                                                                                              \
        if (set->used[i]) method_prefix##SetEntryDelete(set->set[i]);                                                \
      }                                                                                                              \
    free(set->set);                                                                                                  \
    free(set->used);                                                                                                 \
    free(set);                                                                                                       \
  }                                                                                                                  \
                                                                                                                     \
  int method_prefix##SetAdd(type_prefix##Set *set, type_prefix##SetConstEntry entry)                                 \
  {                                                                                                                  \
    ssize_t index;                                                                                                   \
                                                                                                                     \
    index = method_prefix##SetIndex(set, entry);                                                                     \
    if (index < 0) return 0;                                                                                         \
    if (set->used[index])                                                                                            \
      {                                                                                                              \
        method_prefix##SetEntryDelete(set->set[index]);                                                              \
        --(set->size);                                                                                               \
        set->used[index] = 0;                                                                                        \
      }                                                                                                              \
    if (!method_prefix##SetEntryCopy(set->set + index, entry)) return 0;                                             \
    ++(set->size);                                                                                                   \
    set->used[index] = 1;                                                                                            \
                                                                                                                     \
    return 1;                                                                                                        \
  }                                                                                                                  \
                                                                                                                     \
  int method_prefix##SetFind(const type_prefix##Set *set, type_prefix##SetConstEntry entry,                          \
                             type_prefix##SetEntry *saved_entry)                                                     \
  {                                                                                                                  \
    ssize_t index;                                                                                                   \
                                                                                                                     \
    index = method_prefix##SetIndex(set, entry);                                                                     \
    if (index < 0 || !set->used[index]) return 0;                                                                    \
    *saved_entry = set->set[index];                                                                                  \
    return 1;                                                                                                        \
  }                                                                                                                  \
                                                                                                                     \
  int method_prefix##SetContains(const type_prefix##Set *set, type_prefix##SetConstEntry entry)                      \
  {                                                                                                                  \
    ssize_t index;                                                                                                   \
                                                                                                                     \
    index = method_prefix##SetIndex(set, entry);                                                                     \
    return index >= 0 && set->used[index];                                                                           \
  }                                                                                                                  \
                                                                                                                     \
  ssize_t method_prefix##SetIndex(const type_prefix##Set *set, type_prefix##SetConstEntry entry)                     \
  {                                                                                                                  \
    size_t hash;                                                                                                     \
    size_t i;                                                                                                        \
                                                                                                                     \
    hash = method_prefix##SetEntryHash(entry);                                                                       \
    for (i = 0; i < set->capacity; ++i)                                                                              \
      {                                                                                                              \
        /* Quadratic probing that will visit every slot in the hash table if the capacity is a power of 2, see: \    \
         * http://research.cs.vt.edu/AVresearch/hashing/quadratic.php */                                             \
        size_t next_index = (hash + (i * i + i) / 2) % set->capacity;                                                \
        if (!set->used[next_index] || method_prefix##SetEntryEquals(set->set[next_index], entry)) return next_index; \
      }                                                                                                              \
    return -1;                                                                                                       \
  }
