/* ######################### includes ############################################################################### */

#ifdef __unix__
#define _POSIX_C_SOURCE 200809L
#endif

#include <string.h>

#include "gkscore.h"
#include "string_list_int.h"


/* ######################### implementation ######################################################################### */

/* ========================= methods ================================================================================ */

/* ------------------------- string_list ---------------------------------------------------------------------------- */

DEFINE_LIST_METHODS(string)

error_t string_list_entry_copy(string_list_entry_t *copy, const string_list_const_entry_t entry)
{
  string_list_entry_t _copy;

  _copy = strdup(entry);
  if (_copy == NULL)
    {
      return ERROR_MALLOC;
    }
  *copy = _copy;

  return NO_ERROR;
}

error_t string_list_entry_delete(string_list_entry_t entry)
{
  free(entry);
  return NO_ERROR;
}


#undef DEFINE_LIST_METHODS
