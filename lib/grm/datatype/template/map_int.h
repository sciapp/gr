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
#include "util_int.h"


/* ######################### interface ############################################################################## */

/* ========================= datatypes ============================================================================== */

/* ------------------------- string-to-generic map ------------------------------------------------------------------ */

#undef DECLARE_MAP_TYPE
#define DECLARE_MAP_TYPE(prefix, value_type)                     \
  typedef value_type prefix##_map_value_t;                       \
  typedef const value_type prefix##_map_const_value_t;           \
                                                                 \
  typedef struct                                                 \
  {                                                              \
    const char *key;                                             \
    prefix##_map_value_t value;                                  \
  } prefix##_map_entry_t;                                        \
                                                                 \
  DECLARE_SET_TYPE(string_##prefix##_pair, prefix##_map_entry_t) \
  typedef string_##prefix##_pair_set_t prefix##_map_t;


/* ========================= methods ================================================================================ */

/* ------------------------- string-to-generic map ------------------------------------------------------------------ */

#undef DECLARE_MAP_METHODS
#define DECLARE_MAP_METHODS(prefix)                                                                                   \
  DECLARE_SET_METHODS(string_##prefix##_pair)                                                                         \
                                                                                                                      \
  prefix##_map_t *prefix##_map_new(size_t capacity) MAYBE_UNUSED;                                                     \
  prefix##_map_t *prefix##_map_new_with_data(size_t count, prefix##_map_entry_t *entries) MAYBE_UNUSED;               \
  prefix##_map_t *prefix##_map_copy(const prefix##_map_t *map) MAYBE_UNUSED;                                          \
  void prefix##_map_delete(prefix##_map_t *prefix##_map) MAYBE_UNUSED;                                                \
  int prefix##_map_insert(prefix##_map_t *prefix##_map, const char *key, prefix##_map_const_value_t value)            \
      MAYBE_UNUSED;                                                                                                   \
  int prefix##_map_insert_default(prefix##_map_t *prefix##_map, const char *key, prefix##_map_const_value_t value)    \
      MAYBE_UNUSED;                                                                                                   \
  int prefix##_map_at(const prefix##_map_t *prefix##_map, const char *key, prefix##_map_value_t *value) MAYBE_UNUSED; \
                                                                                                                      \
  int prefix##_map_value_copy(prefix##_map_value_t *copy, prefix##_map_const_value_t value);                          \
  void prefix##_map_value_delete(prefix##_map_value_t value);


/* ######################### implementation ######################################################################### */

/* ========================= methods ================================================================================ */

/* ------------------------- string-to-generic map ------------------------------------------------------------------ */

#undef DEFINE_MAP_METHODS
#define DEFINE_MAP_METHODS(prefix)                                                                                 \
  DEFINE_SET_METHODS(string_##prefix##_pair)                                                                       \
                                                                                                                   \
  prefix##_map_t *prefix##_map_new(size_t capacity)                                                                \
  {                                                                                                                \
    string_##prefix##_pair_set_t *string_##prefix##_pair_set;                                                      \
                                                                                                                   \
    string_##prefix##_pair_set = string_##prefix##_pair_set_new(capacity);                                         \
    if (string_##prefix##_pair_set == NULL)                                                                        \
      {                                                                                                            \
        debug_print_malloc_error();                                                                                \
        return NULL;                                                                                               \
      }                                                                                                            \
                                                                                                                   \
    return (prefix##_map_t *)string_##prefix##_pair_set;                                                           \
  }                                                                                                                \
                                                                                                                   \
  prefix##_map_t *prefix##_map_new_with_data(size_t count, prefix##_map_entry_t *entries)                          \
  {                                                                                                                \
    return (prefix##_map_t *)string_##prefix##_pair_set_new_with_data(count, entries);                             \
  }                                                                                                                \
                                                                                                                   \
  prefix##_map_t *prefix##_map_copy(const prefix##_map_t *map)                                                     \
  {                                                                                                                \
    string_##prefix##_pair_set_t *string_##prefix##_pair_set;                                                      \
                                                                                                                   \
    string_##prefix##_pair_set = string_##prefix##_pair_set_copy((string_##prefix##_pair_set_t *)map);             \
    if (string_##prefix##_pair_set == NULL)                                                                        \
      {                                                                                                            \
        debug_print_malloc_error();                                                                                \
        return NULL;                                                                                               \
      }                                                                                                            \
                                                                                                                   \
    return (prefix##_map_t *)string_##prefix##_pair_set;                                                           \
  }                                                                                                                \
                                                                                                                   \
  void prefix##_map_delete(prefix##_map_t *prefix##_map)                                                           \
  {                                                                                                                \
    string_##prefix##_pair_set_delete((string_##prefix##_pair_set_t *)prefix##_map);                               \
  }                                                                                                                \
                                                                                                                   \
  int prefix##_map_insert(prefix##_map_t *prefix##_map, const char *key, prefix##_map_const_value_t value)         \
  {                                                                                                                \
    string_##prefix##_pair_set_entry_t entry;                                                                      \
                                                                                                                   \
    entry.key = key;                                                                                               \
    /* in this case, it is ok to remove the const attribute since a copy is created in the set implementation */   \
    entry.value = (prefix##_map_value_t)value;                                                                     \
    return string_##prefix##_pair_set_add((string_##prefix##_pair_set_t *)prefix##_map, entry);                    \
  }                                                                                                                \
                                                                                                                   \
  int prefix##_map_insert_default(prefix##_map_t *prefix##_map, const char *key, prefix##_map_const_value_t value) \
  {                                                                                                                \
    string_##prefix##_pair_set_entry_t entry;                                                                      \
                                                                                                                   \
    entry.key = key;                                                                                               \
    /* in this case, it is ok to remove the const attribute since a copy is created in the set implementation */   \
    entry.value = (prefix##_map_value_t)value;                                                                     \
    if (!string_##prefix##_pair_set_contains((string_##prefix##_pair_set_t *)prefix##_map, entry))                 \
      {                                                                                                            \
        return string_##prefix##_pair_set_add((string_##prefix##_pair_set_t *)prefix##_map, entry);                \
      }                                                                                                            \
    return 0;                                                                                                      \
  }                                                                                                                \
                                                                                                                   \
  int prefix##_map_at(const prefix##_map_t *prefix##_map, const char *key, prefix##_map_value_t *value)            \
  {                                                                                                                \
    string_##prefix##_pair_set_entry_t entry, saved_entry;                                                         \
                                                                                                                   \
    entry.key = key;                                                                                               \
    if (string_##prefix##_pair_set_find((string_##prefix##_pair_set_t *)prefix##_map, entry, &saved_entry))        \
      {                                                                                                            \
        if (value != NULL)                                                                                         \
          {                                                                                                        \
            *value = saved_entry.value;                                                                            \
          }                                                                                                        \
        return 1;                                                                                                  \
      }                                                                                                            \
    else                                                                                                           \
      {                                                                                                            \
        return 0;                                                                                                  \
      }                                                                                                            \
  }                                                                                                                \
                                                                                                                   \
  int string_##prefix##_pair_set_entry_copy(string_##prefix##_pair_set_entry_t *copy,                              \
                                            const string_##prefix##_pair_set_entry_t entry)                        \
  {                                                                                                                \
    const char *key_copy;                                                                                          \
    prefix##_map_value_t value_copy;                                                                               \
                                                                                                                   \
    key_copy = gks_strdup(entry.key);                                                                              \
    if (key_copy == NULL)                                                                                          \
      {                                                                                                            \
        return 0;                                                                                                  \
      }                                                                                                            \
    if (!prefix##_map_value_copy(&value_copy, (prefix##_map_const_value_t)entry.value))                            \
      {                                                                                                            \
        free((char *)key_copy);                                                                                    \
        return 0;                                                                                                  \
      }                                                                                                            \
    copy->key = key_copy;                                                                                          \
    copy->value = value_copy;                                                                                      \
                                                                                                                   \
    return 1;                                                                                                      \
  }                                                                                                                \
                                                                                                                   \
  void string_##prefix##_pair_set_entry_delete(string_##prefix##_pair_set_entry_t entry)                           \
  {                                                                                                                \
    free((char *)entry.key);                                                                                       \
    prefix##_map_value_delete(entry.value);                                                                        \
  }                                                                                                                \
                                                                                                                   \
  size_t string_##prefix##_pair_set_entry_hash(const string_##prefix##_pair_set_entry_t entry)                     \
  {                                                                                                                \
    return djb2_hash(entry.key);                                                                                   \
  }                                                                                                                \
                                                                                                                   \
  int string_##prefix##_pair_set_entry_equals(const string_##prefix##_pair_set_entry_t entry1,                     \
                                              const string_##prefix##_pair_set_entry_t entry2)                     \
  {                                                                                                                \
    return strcmp(entry1.key, entry2.key) == 0;                                                                    \
  }
