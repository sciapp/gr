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

DEFINE_MAP_METHODS(string)

int string_map_value_copy(const char **copy, const char *value)
{
  char *_copy;

  _copy = gks_strdup(value);
  if (_copy == NULL)
    {
      return 0;
    }
  *copy = _copy;

  return 1;
}

void string_map_value_delete(const char *value)
{
  free((void *)value);
}


#undef DEFINE_MAP_METHODS
