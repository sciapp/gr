#ifndef GRM_STRING_ARRAY_MAP_INT_H_INCLUDED
#define GRM_STRING_ARRAY_MAP_INT_H_INCLUDED

/* ######################### includes ############################################################################### */

#include "string_map_int.h"
#include "template/map_int.h"


/* ######################### interface ############################################################################## */

/* ========================= datatypes ============================================================================== */

/* ------------------------- string_map ----------------------------------------------------------------------------- */

DECLARE_MAP_TYPE(string_array, char **)


/* ========================= methods ================================================================================ */

/* ------------------------- string_map ----------------------------------------------------------------------------- */

DECLARE_MAP_METHODS(string_array)

string_array_map_t *string_array_map_new_from_string_split(size_t count, const string_map_entry_t *entries,
                                                           char split_char);


#undef DECLARE_MAP_TYPE
#undef DECLARE_MAP_METHODS

#endif /* ifndef GRM_STRING_ARRAY_MAP_INT_H_INCLUDED */
