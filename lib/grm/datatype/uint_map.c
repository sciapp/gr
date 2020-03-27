#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include "uint_map_int.h"


/* ######################### implementation ######################################################################### */

/* ========================= methods ================================================================================ */

/* ------------------------- string-to-uint map --------------------------------------------------------------------- */

DEFINE_MAP_METHODS(uint)

int uint_map_value_copy(unsigned int *copy, const unsigned int value)
{
  *copy = value;

  return 1;
}

void uint_map_value_delete(unsigned int value UNUSED) {}


#undef DEFINE_MAP_METHODS
