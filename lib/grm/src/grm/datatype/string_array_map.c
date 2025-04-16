#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include <string.h>

#include "gkscore.h"
#include "string_array_map_int.h"


/* ######################### implementation ######################################################################### */

/* ========================= methods ================================================================================ */

/* ------------------------- string_map ----------------------------------------------------------------------------- */

DEFINE_MAP_METHODS(StringArray, stringArray)

int stringArrayMapValueCopy(char ***copy, const char **value)
{
  size_t entry_length;
  char **tmp_copy = NULL;
  const char **entry;
  char **copy_entry;

  entry_length = 0;
  while (value[entry_length] != NULL)
    {
      ++entry_length;
    }

  tmp_copy = calloc(entry_length + 1, sizeof(char *));
  if (tmp_copy == NULL) goto cleanup;

  entry = value;
  copy_entry = tmp_copy;
  while (*entry != NULL)
    {
      *copy_entry = gks_strdup(*entry);
      if (*copy_entry == NULL) goto cleanup;
      ++entry;
      ++copy_entry;
    }
  *copy = tmp_copy;

  return 1;

cleanup:
  if (tmp_copy != NULL)
    {
      copy_entry = tmp_copy;
      while (*copy_entry != NULL)
        {
          free(*copy_entry);
          ++copy_entry;
        }
      free(tmp_copy);
    }

  return 0;
}

void stringArrayMapValueDelete(char **value)
{
  char **entry;

  entry = value;
  while (*entry != NULL)
    {
      free(*entry);
      ++entry;
    }

  free(value);
}

StringArrayMap *stringArrayMapNewFromStringSplit(size_t count, const StringMapEntry *entries, char split_char)
{
  StringArrayMap *map = NULL;
  char *split_string = NULL;
  char *current_char_ptr;
  char **string_array = NULL;
  char **current_array_entry;
  size_t array_length;
  size_t i;

  map = stringArrayMapNew(count);
  if (map == NULL) goto cleanup;
  for (i = 0; i < count; ++i)
    {
      split_string = gks_strdup(entries[i].value);
      if (split_string == NULL) goto cleanup;
      array_length = 1;
      current_char_ptr = split_string;
      while (*current_char_ptr != '\0')
        {
          if (*current_char_ptr == split_char) ++array_length;
          ++current_char_ptr;
        }
      string_array = calloc(array_length + 1, sizeof(char *));
      if (string_array == NULL) goto cleanup;
      current_array_entry = string_array;
      *current_array_entry = split_string;
      ++current_array_entry;
      current_char_ptr = split_string;
      while (*current_char_ptr != '\0')
        {
          if (*current_char_ptr == split_char)
            {
              *current_char_ptr = '\0';
              *current_array_entry = current_char_ptr + 1;
              ++current_array_entry;
            }
          ++current_char_ptr;
        }
      *current_array_entry = NULL;
      if (!stringArrayMapInsert(map, entries[i].key, (const char **)string_array)) goto cleanup;
      free(split_string);
      free(string_array);
    }

  return map;

cleanup:
  if (map == NULL) stringArrayMapDelete(map);
  if (split_string != NULL) free(split_string);
  if (string_array != NULL) free(string_array);

  return NULL;
}


#undef DEFINE_MAP_METHODS
