/* Do not use an include guard since the following macros may be deleted with `undef` between multiple includes */

#ifdef __unix__
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif
#endif

/* ######################### includes ############################################################################### */

#include <string.h>

#include "gkscore.h"
#include "set_int.h"
#include <grm/util_int.h>


/* ######################### interface ############################################################################## */

/* ========================= datatypes ============================================================================== */

/* ------------------------- string-to-generic map ------------------------------------------------------------------ */

#undef DECLARE_MAP_TYPE
#define DECLARE_MAP_TYPE(type_prefix, value_type)                    \
  typedef value_type type_prefix##MapValue;                          \
  typedef const value_type type_prefix##MapConstValue;               \
                                                                     \
  typedef struct                                                     \
  {                                                                  \
    const char *key;                                                 \
    type_prefix##MapValue value;                                     \
  } type_prefix##MapEntry;                                           \
  typedef const type_prefix##MapEntry type_prefix##MapConstEntry;    \
                                                                     \
  DECLARE_SET_TYPE(String##type_prefix##Pair, type_prefix##MapEntry) \
  typedef String##type_prefix##PairSet type_prefix##Map;


/* ========================= methods ================================================================================ */

/* ------------------------- string-to-generic map ------------------------------------------------------------------ */

#undef DECLARE_MAP_METHODS
#define DECLARE_MAP_METHODS(type_prefix, method_prefix)                                                                \
  DECLARE_SET_METHODS(String##type_prefix##Pair, string##type_prefix##Pair)                                            \
                                                                                                                       \
  type_prefix##Map *method_prefix##MapNew(size_t capacity) MAYBE_UNUSED;                                               \
  type_prefix##Map *method_prefix##MapNewWithData(size_t count, type_prefix##MapConstEntry *entries) MAYBE_UNUSED;     \
  type_prefix##Map *method_prefix##MapCopy(const type_prefix##Map *map) MAYBE_UNUSED;                                  \
  void method_prefix##MapDelete(type_prefix##Map *map) MAYBE_UNUSED;                                                   \
  int method_prefix##MapInsert(type_prefix##Map *map, const char *key, type_prefix##MapConstValue value) MAYBE_UNUSED; \
  int method_prefix##MapInsertDefault(type_prefix##Map *map, const char *key, type_prefix##MapConstValue value)        \
      MAYBE_UNUSED;                                                                                                    \
  int method_prefix##MapAt(const type_prefix##Map *map, const char *key, type_prefix##MapValue *value) MAYBE_UNUSED;   \
                                                                                                                       \
  int method_prefix##MapValueCopy(type_prefix##MapValue *copy, type_prefix##MapConstValue value);                      \
  void method_prefix##MapValueDelete(type_prefix##MapValue value);


/* ######################### implementation ######################################################################### */

/* ========================= methods ================================================================================ */

/* ------------------------- string-to-generic map ------------------------------------------------------------------ */

#undef DEFINE_MAP_METHODS
#define DEFINE_MAP_METHODS(type_prefix, method_prefix)                                                           \
  DEFINE_SET_METHODS(String##type_prefix##Pair, string##type_prefix##Pair)                                       \
                                                                                                                 \
  type_prefix##Map *method_prefix##MapNew(size_t capacity)                                                       \
  {                                                                                                              \
    String##type_prefix##PairSet *set;                                                                           \
                                                                                                                 \
    set = string##type_prefix##PairSetNew(capacity);                                                             \
    if (set == NULL)                                                                                             \
      {                                                                                                          \
        debugPrintMallocError();                                                                                 \
        return NULL;                                                                                             \
      }                                                                                                          \
                                                                                                                 \
    return (type_prefix##Map *)set;                                                                              \
  }                                                                                                              \
                                                                                                                 \
  type_prefix##Map *method_prefix##MapNewWithData(size_t count, type_prefix##MapConstEntry *entries)             \
  {                                                                                                              \
    return (type_prefix##Map *)string##type_prefix##PairSetNewWithData(count, entries);                          \
  }                                                                                                              \
                                                                                                                 \
  type_prefix##Map *method_prefix##MapCopy(const type_prefix##Map *map)                                          \
  {                                                                                                              \
    String##type_prefix##PairSet *set;                                                                           \
                                                                                                                 \
    set = string##type_prefix##PairSetCopy((String##type_prefix##PairSet *)map);                                 \
    if (set == NULL)                                                                                             \
      {                                                                                                          \
        debugPrintMallocError();                                                                                 \
        return NULL;                                                                                             \
      }                                                                                                          \
                                                                                                                 \
    return (type_prefix##Map *)set;                                                                              \
  }                                                                                                              \
                                                                                                                 \
  void method_prefix##MapDelete(type_prefix##Map *map)                                                           \
  {                                                                                                              \
    string##type_prefix##PairSetDelete((String##type_prefix##PairSet *)map);                                     \
  }                                                                                                              \
                                                                                                                 \
  int method_prefix##MapInsert(type_prefix##Map *map, const char *key, type_prefix##MapConstValue value)         \
  {                                                                                                              \
    String##type_prefix##PairSetEntry entry;                                                                     \
                                                                                                                 \
    entry.key = key;                                                                                             \
    /* in this case, it is ok to remove the const attribute since a copy is created in the set implementation */ \
    entry.value = (type_prefix##MapValue)value;                                                                  \
    return string##type_prefix##PairSetAdd((String##type_prefix##PairSet *)map, entry);                          \
  }                                                                                                              \
                                                                                                                 \
  int method_prefix##MapInsertDefault(type_prefix##Map *map, const char *key, type_prefix##MapConstValue value)  \
  {                                                                                                              \
    String##type_prefix##PairSetEntry entry;                                                                     \
                                                                                                                 \
    entry.key = key;                                                                                             \
    /* in this case, it is ok to remove the const attribute since a copy is created in the set implementation */ \
    entry.value = (type_prefix##MapValue)value;                                                                  \
    if (!string##type_prefix##PairSetContains((String##type_prefix##PairSet *)map, entry))                       \
      {                                                                                                          \
        return string##type_prefix##PairSetAdd((String##type_prefix##PairSet *)map, entry);                      \
      }                                                                                                          \
    return 0;                                                                                                    \
  }                                                                                                              \
                                                                                                                 \
  int method_prefix##MapAt(const type_prefix##Map *map, const char *key, type_prefix##MapValue *value)           \
  {                                                                                                              \
    String##type_prefix##PairSetEntry entry, saved_entry;                                                        \
                                                                                                                 \
    entry.key = key;                                                                                             \
    if (string##type_prefix##PairSetFind((String##type_prefix##PairSet *)map, entry, &saved_entry))              \
      {                                                                                                          \
        if (value != NULL) *value = saved_entry.value;                                                           \
        return 1;                                                                                                \
      }                                                                                                          \
    return 0;                                                                                                    \
  }                                                                                                              \
                                                                                                                 \
  int string##type_prefix##PairSetEntryCopy(String##type_prefix##PairSetEntry *copy,                             \
                                            const String##type_prefix##PairSetEntry entry)                       \
  {                                                                                                              \
    const char *key_copy;                                                                                        \
    type_prefix##MapValue value_copy;                                                                            \
                                                                                                                 \
    key_copy = gks_strdup(entry.key);                                                                            \
    if (key_copy == NULL) return 0;                                                                              \
    if (!method_prefix##MapValueCopy(&value_copy, (type_prefix##MapConstValue)entry.value))                      \
      {                                                                                                          \
        free((char *)key_copy);                                                                                  \
        return 0;                                                                                                \
      }                                                                                                          \
    copy->key = key_copy;                                                                                        \
    copy->value = value_copy;                                                                                    \
                                                                                                                 \
    return 1;                                                                                                    \
  }                                                                                                              \
                                                                                                                 \
  void string##type_prefix##PairSetEntryDelete(String##type_prefix##PairSetEntry entry)                          \
  {                                                                                                              \
    free((char *)entry.key);                                                                                     \
    method_prefix##MapValueDelete(entry.value);                                                                  \
  }                                                                                                              \
                                                                                                                 \
  size_t string##type_prefix##PairSetEntryHash(const String##type_prefix##PairSetEntry entry)                    \
  {                                                                                                              \
    return djb2Hash(entry.key);                                                                                  \
  }                                                                                                              \
                                                                                                                 \
  int string##type_prefix##PairSetEntryEquals(const String##type_prefix##PairSetEntry entry1,                    \
                                              const String##type_prefix##PairSetEntry entry2)                    \
  {                                                                                                              \
    return strcmp(entry1.key, entry2.key) == 0;                                                                  \
  }
