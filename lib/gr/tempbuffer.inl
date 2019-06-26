/*!
 * This file implements a generic buffer for small, temporary objects.
 *
 * It is meant for use when keeping track of a large number of allocations
 * would be worse in code complexity and performance than keeping some of
 * them around until all objects can be discarded at once.
 */

#include <gkscore.h>

#ifndef VALUE_TYPE
#error "VALUE_TYPE must be defined"
#endif

#ifndef VALUE_NAME
#error "VALUE_NAME must be defined"
#endif

#define XCONCAT(LEFT, RIGHT) LEFT ## RIGHT
#define CONCAT(LEFT, RIGHT) XCONCAT(LEFT, RIGHT)

#define MEMORY_NAME CONCAT(VALUE_NAME, _memory_)
#define NEXT_INDEX_NAME CONCAT(VALUE_NAME, _next_index_)
#define SIZE_NAME CONCAT(VALUE_NAME, _memory_size_)

VALUE_TYPE *MEMORY_NAME = NULL;
size_t SIZE_NAME = 0;
size_t NEXT_INDEX_NAME = 0;

size_t CONCAT(copy_, VALUE_NAME)(VALUE_TYPE VALUE_NAME) {
    size_t index = NEXT_INDEX_NAME;
    if (VALUE_NAME.index) {
        return VALUE_NAME.index;
    }
    if (NEXT_INDEX_NAME >= SIZE_NAME) {
        SIZE_NAME += 1024;
        MEMORY_NAME = (VALUE_TYPE *)gks_realloc(MEMORY_NAME, sizeof(VALUE_TYPE) * SIZE_NAME);
    }
    MEMORY_NAME[index] = VALUE_NAME;
    MEMORY_NAME[index].index = index + 1;
    NEXT_INDEX_NAME += 1;
    return index + 1;
}

VALUE_TYPE *CONCAT(get_, VALUE_NAME)(size_t index) {
    assert(0 <= index);
    assert(index <= SIZE_NAME);
    if (index == 0) {
        return NULL;
    }
    return MEMORY_NAME + index - 1;
}

void CONCAT(CONCAT(free_, VALUE_NAME), _buffer)() {
    gks_free(MEMORY_NAME);
    MEMORY_NAME = 0;
    SIZE_NAME = 0;
    NEXT_INDEX_NAME = 0;
}

#undef SIZE_NAME
#undef NEXT_INDEX_NAME
#undef MEMORY_NAME
#undef CONCAT
#undef XCONCAT
#undef VALUE_NAME
#undef VALUE_TYPE
