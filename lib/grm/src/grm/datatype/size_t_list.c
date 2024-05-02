#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include "size_t_list_int.h"


/* ######################### implementation ######################################################################### */

/* ========================= methods ================================================================================ */

/* ------------------------ size_t list ----------------------------------------------------------------------------- */

DEFINE_LIST_METHODS(size_t)

err_t size_t_list_entry_copy(size_t_list_entry_t *copy, size_t_list_const_entry_t entry)
{
  *copy = entry;

  return ERROR_NONE;
}

err_t size_t_list_entry_delete(size_t_list_entry_t entry)
{
  return ERROR_NONE;
}


#undef DEFINE_MAP_METHODS
