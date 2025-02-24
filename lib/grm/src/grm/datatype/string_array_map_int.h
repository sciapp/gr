#ifndef GRM_STRING_ARRAY_MAP_INT_H_INCLUDED
#define GRM_STRING_ARRAY_MAP_INT_H_INCLUDED

/* ######################### includes ############################################################################### */

#include "string_map_int.h"
#include "template/map_int.h"


/* ######################### interface ############################################################################## */

/* ========================= datatypes ============================================================================== */

/* ------------------------- string_map ----------------------------------------------------------------------------- */

DECLARE_MAP_TYPE(StringArray, char **)


/* ========================= methods ================================================================================ */

/* ------------------------- string_map ----------------------------------------------------------------------------- */

DECLARE_MAP_METHODS(StringArray, stringArray)

StringArrayMap *stringArrayMapNewFromStringSplit(size_t count, const StringMapEntry *entries, char split_char);


#undef DECLARE_MAP_TYPE
#undef DECLARE_MAP_METHODS

#endif /* ifndef GRM_STRING_ARRAY_MAP_INT_H_INCLUDED */
