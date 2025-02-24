#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include "double_map_int.h"


/* ######################### implementation ######################################################################### */

/* ========================= methods ================================================================================ */

/* ----------------------- string-to-double map --------------------------------------------------------------------- */

DEFINE_MAP_METHODS(Double, double)

int doubleMapValueCopy(double *copy, const double value)
{
  *copy = value;

  return 1;
}

void doubleMapValueDelete(double value UNUSED) {}


#undef DEFINE_MAP_METHODS
