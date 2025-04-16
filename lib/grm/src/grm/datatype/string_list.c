/* ######################### includes ############################################################################### */

#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

#include "gkscore.h"
#include "string_list_int.h"


/* ######################### implementation ######################################################################### */

/* ========================= methods ================================================================================ */

/* ------------------------- string_list ---------------------------------------------------------------------------- */

DEFINE_LIST_METHODS(String, string)

grm_error_t stringListEntryCopy(StringListEntry *copy, const StringListConstEntry entry)
{
  StringListEntry tmp_copy;

  tmp_copy = gks_strdup(entry);
  if (tmp_copy == NULL) return GRM_ERROR_MALLOC;
  *copy = tmp_copy;

  return GRM_ERROR_NONE;
}

grm_error_t stringListEntryDelete(StringListEntry entry)
{
  free(entry);
  return GRM_ERROR_NONE;
}


#undef DEFINE_LIST_METHODS
