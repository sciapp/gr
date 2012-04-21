#include <stdlib.h>
#include <stdio.h>

#include "moldyn.h"

void *moldyn_reallocf(void *ptr, size_t size) {
    void *new_ptr;
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        free(ptr);
        moldyn_log("failed to allocate memory!");
        moldyn_exit(1);
    }
    return new_ptr;
}

void moldyn_log(const char *log_message) {
    fprintf(stderr, "moldyn: %s\n", log_message);
}