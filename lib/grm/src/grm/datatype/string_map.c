#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include <string.h>

#include "gkscore.h"
#include "string_map_int.h"


/* ######################### implementation ######################################################################### */

/* ========================= methods ================================================================================ */

/* ------------------------- string_map ----------------------------------------------------------------------------- */

DEFINE_MAP_METHODS(String, string)

int stringMapValueCopy(const char **copy, const char *value)
{
  char *tmp_copy;

  tmp_copy = gks_strdup(value);
  if (tmp_copy == NULL) return 0;
  *copy = tmp_copy;

  return 1;
}

void stringMapValueDelete(const char *value)
{
  free((void *)value);
}


#undef DEFINE_MAP_METHODS
