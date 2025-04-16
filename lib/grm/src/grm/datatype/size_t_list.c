#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include "size_t_list_int.h"


/* ######################### implementation ######################################################################### */

/* ========================= methods ================================================================================ */

/* ------------------------ size_t list ----------------------------------------------------------------------------- */

DEFINE_LIST_METHODS(SizeT, sizeT)

grm_error_t sizeTListEntryCopy(SizeTListEntry *copy, SizeTListConstEntry entry)
{
  *copy = entry;

  return GRM_ERROR_NONE;
}

grm_error_t sizeTListEntryDelete(SizeTListEntry entry)
{
  return GRM_ERROR_NONE;
}


#undef DEFINE_MAP_METHODS
