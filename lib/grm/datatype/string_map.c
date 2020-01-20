#ifdef __unix__
#define _POSIX_C_SOURCE 1
#endif

/* ######################### includes ############################################################################### */

#include <string.h>

#include "gkscore.h"
#include "string_map_int.h"


/* ######################### implementation ######################################################################### */

/* ========================= methods ================================================================================ */

/* ------------------------- string_map ----------------------------------------------------------------------------- */

DEFINE_MAP_METHODS(string)

int string_map_value_copy(char **copy, const char *value)
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

void string_map_value_delete(char *value)
{
  free(value);
}


#undef DEFINE_MAP_METHODS
