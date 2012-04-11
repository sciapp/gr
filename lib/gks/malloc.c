/* malloc.c -- safe versions of malloc and realloc */

/* Return a pointer to free()able block of memory large enough
   to hold BYTES number of bytes.  If the memory cannot be allocated,
   print an error message and abort. */

#include <stdlib.h>

#include "gkscore.h"

char *gks_malloc(int size)
{
  char *temp;

  temp = (char *) calloc(1, size);
  if (temp == 0)
    {
      gks_fatal_error("gks_malloc: cannot allocate memory");
    }

  return (temp);
}

char *gks_realloc(void *ptr, int size)
{
  char *temp;

  temp = ptr ? (char *) realloc(ptr, size) : (char *) malloc(size);
  if (temp == 0)
    {
      gks_fatal_error("gks_realloc: cannot allocate memory");
    }

  return (temp);
}

/* Use this as the function to call when adding unwind protects so we
   don't need to know what free() returns. */

void gks_free(void *ptr)
{
  if (ptr)
    free(ptr);
}
