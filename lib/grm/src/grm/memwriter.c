#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include <stdarg.h>
#include <string.h>

#include "error_int.h"
#include "memwriter_int.h"


/* ######################### internal implementation ################################################################ */

/* ========================= methods ================================================================================ */

/* ------------------------- memwriter ------------------------------------------------------------------------------ */

Memwriter *memwriterNew()
{
  Memwriter *memwriter;

  memwriter = malloc(sizeof(Memwriter));
  if (memwriter == NULL)
    {
      debugPrintMallocError();
      return NULL;
    }
  memwriter->buf = malloc(MEMWRITER_INITIAL_SIZE);
  if (memwriter->buf == NULL)
    {
      free(memwriter);
      debugPrintMallocError();
      return NULL;
    }
  memwriter->size = 0;
  memwriter->capacity = MEMWRITER_INITIAL_SIZE;

  return memwriter;
}

void memwriterDelete(Memwriter *memwriter)
{
  if (memwriter != NULL)
    {
      free(memwriter->buf);
      free(memwriter);
    }
}

void memwriterClear(Memwriter *memwriter)
{
  memwriter->size = 0;
  *memwriter->buf = '\0';
}

grm_error_t memwriterReplace(Memwriter *memwriter, int index, int count, const char *replacement_str)
{
  int replacement_str_len = (replacement_str != NULL) ? strlen(replacement_str) : 0;
  grm_error_t error = GRM_ERROR_NONE;

  if ((replacement_str_len > count) &&
      (error = memwriterEnsureBuf(memwriter, replacement_str_len - count)) != GRM_ERROR_NONE)
    {
      return error;
    }
  if (count != replacement_str_len)
    {
      memmove(memwriter->buf + index + replacement_str_len, memwriter->buf + index + count,
              memwriter->size - (index + count));
    }
  if (replacement_str != NULL) memcpy(memwriter->buf + index, replacement_str, replacement_str_len);
  memwriter->size += replacement_str_len - count;

  return error;
}

grm_error_t memwriterErase(Memwriter *memwriter, int index, int count)
{
  return memwriterReplace(memwriter, index, count, NULL);
}

grm_error_t memwriterInsert(Memwriter *memwriter, int index, const char *str)
{
  return memwriterReplace(memwriter, index, 0, str);
}

grm_error_t memwriterEnlargeBuf(Memwriter *memwriter, size_t size_increment)
{
  void *new_buf;

  if (size_increment == 0)
    {
      if (memwriter->capacity >= MEMWRITER_EXPONENTIAL_INCREASE_UNTIL)
        {
          size_increment = MEMWRITER_LINEAR_INCREMENT_SIZE;
        }
      else
        {
          size_increment = memwriter->capacity;
        }
    }
  else
    {
      /* round up to the next `MEMWRITER_LINEAR_INCREMENT_SIZE` step */
      if (memwriter->capacity >= MEMWRITER_EXPONENTIAL_INCREASE_UNTIL)
        {
          size_increment =
              ((size_increment - 1) / MEMWRITER_LINEAR_INCREMENT_SIZE + 1) * MEMWRITER_LINEAR_INCREMENT_SIZE;
        }
      else
        {
          size_increment = nextOrEqualPower2(memwriter->capacity + size_increment) - memwriter->capacity;
        }
    }
  new_buf = realloc(memwriter->buf, memwriter->capacity + size_increment);
  if (new_buf == NULL)
    {
      debugPrintMallocError();
      return GRM_ERROR_MALLOC;
    }
  memwriter->buf = new_buf;
  memwriter->capacity += size_increment;

  return GRM_ERROR_NONE;
}

grm_error_t memwriterEnsureBuf(Memwriter *memwriter, size_t needed_additional_size)
{
  if (memwriter->size + needed_additional_size > memwriter->capacity)
    {
      return memwriterEnlargeBuf(memwriter, memwriter->size + needed_additional_size - memwriter->capacity);
    }
  return GRM_ERROR_NONE;
}

grm_error_t memwriterPrintf(Memwriter *memwriter, const char *format, ...)
{
  va_list vl;
  grm_error_t error = GRM_ERROR_NONE;

  while (1)
    {
      int chars_needed;
      va_start(vl, format);
      chars_needed = vsnprintf(&memwriter->buf[memwriter->size], memwriter->capacity - memwriter->size, format, vl);
      va_end(vl);
      if (chars_needed < 0) return GRM_ERROR_INTERNAL;
      /* we need one more char because `vsnprintf` does exclude the trailing '\0' character in its calculations */
      if ((size_t)chars_needed < (memwriter->capacity - memwriter->size))
        {
          memwriter->size += chars_needed;
          break;
        }
      if ((error = memwriterEnsureBuf(memwriter, chars_needed + 1)) != GRM_ERROR_NONE) break;
    }

  return error;
}

grm_error_t memwriterPuts(Memwriter *memwriter, const char *s)
{
  return memwriterPrintf(memwriter, "%s", s);
}

grm_error_t memwriterPutsWithLen(Memwriter *memwriter, char *s, size_t length)
{
  grm_error_t error = GRM_ERROR_NONE;

  while (length > 0)
    {
      if ((error = memwriterPutc(memwriter, *(s++))) != GRM_ERROR_NONE) return error;
      --length;
    }

  return error;
}

grm_error_t memwriterPutc(Memwriter *memwriter, char c)
{
  return memwriterPrintf(memwriter, "%c", c);
}

grm_error_t memwriterMemcpy(Memwriter *memwriter, const void *source, size_t num)
{
  memwriterEnsureBuf(memwriter, num);

  memcpy(&memwriter->buf[memwriter->size], source, num);

  memwriter->size += num;

  return GRM_ERROR_NONE;
}

grm_error_t memwriterMemcpyRevChunks(Memwriter *memwriter, const void *source, size_t num, int chunk_size)
{
  memwriterEnsureBuf(memwriter, num);

  char *d = &memwriter->buf[memwriter->size];
  const char *s = source;
  int i, j;

  for (i = 0; i < num; i += chunk_size)
    {
      for (j = 0; j < chunk_size; j++)
        {
          d[i + chunk_size - j - 1] = s[i + j];
        }
    }

  memwriter->size += num;

  return GRM_ERROR_NONE;
}

char *memwriterBuf(const Memwriter *memwriter)
{
  return memwriter->buf;
}

size_t memwriterSize(const Memwriter *memwriter)
{
  return memwriter->size;
}
