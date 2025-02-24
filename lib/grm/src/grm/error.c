/* ######################### includes ############################################################################### */

#include <stdarg.h>

#include "error_int.h"


/* ######################### internal interface ##################################################################### */

/* ========================= global variables ======================================================================= */

/* ------------------------- error handling ------------------------------------------------------------------------- */

const char *grm_error_names[] = {GRM_ERROR_ENUM_ELEMENTS(GRM_STRING_ARRAY_VALUE, GRM_STRING_ARRAY_VALUE)};

/* ========================= functions ============================================================================== */

/* ------------------------- error handling ------------------------------------------------------------------------- */

#ifndef NDEBUG
void debugPrintf(const char *format, ...)
{
  va_list vl;
  va_start(vl, format);
  vfprintf(stderr, format, vl);
  va_end(vl);
}
#endif


#undef GRM_ERROR_ENUM_ELEMENTS
#undef GRM_ENUM_VALUE
#undef GRM_ENUM_LAST_VALUE
#undef GRM_STRING_ARRAY_VALUE
