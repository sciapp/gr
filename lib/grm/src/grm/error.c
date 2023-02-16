/* ######################### includes ############################################################################### */

#include <stdarg.h>

#include "error_int.h"


/* ######################### internal interface ##################################################################### */

/* ========================= global variables ======================================================================= */

/* ------------------------- error handling ------------------------------------------------------------------------- */

const char *error_names[] = {ENUM_ELEMENTS(STRING_ARRAY_VALUE, STRING_ARRAY_VALUE)};

/* ========================= functions ============================================================================== */

/* ------------------------- error handling ------------------------------------------------------------------------- */

#ifndef NDEBUG
void debug_printf(const char *format, ...)
{
  va_list vl;
  va_start(vl, format);
  vfprintf(stderr, format, vl);
  va_end(vl);
}
#endif


#undef ENUM_ELEMENTS
#undef ENUM_VALUE
#undef ENUM_LAST_VALUE
#undef STRING_ARRAY_VALUE
