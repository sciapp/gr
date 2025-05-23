#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _MSC_VER
#include <unistd.h>
#else
#include <io.h>
#endif

#include "base64_int.h"
#include "logging_int.h"
#include "util_int.h"


/* ######################### internal implementation ################################################################ */

/* ========================= static variables ======================================================================= */

static const char BASE64_DECODE_TABLE[128] = {
    0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      '>',    0,      0,      0,      '?',    '4',    '5',    '6',    '7',
    '8',    '9',    ':',    ';',    '<',    '=',    0,      0,      0,      0,      0,      0,      0,
    '\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\x08', '\t',   '\n',   '\x0b', '\x0c',
    '\r',   '\x0e', '\x0f', '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\x18', '\x19',
    0,      0,      0,      0,      0,      0,      '\x1a', '\x1b', '\x1c', '\x1d', '\x1e', '\x1f', ' ',
    '!',    '"',    '#',    '$',    '%',    '&',    '\'',   '(',    ')',    '*',    '+',    ',',    '-',
    '.',    '/',    '0',    '1',    '2',    '3',    0,      0,      0,      0,      0,
};

static const char BASE64_ENCODE_TABLE[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
};

static const char PADDING_CHAR = '=';


/* ========================= functions ============================================================================== */

grm_error_t blockDecode(char dst[3], const char src[4], int block_len, int *decoded_block_len)
{
  /*
   * Transform the four input characters with the `base64_decode_table` and interpret the result as four 6-bit values.
   * Write these values to the given three 8-bit character array.
   */

  unsigned char lookup_buffer[4];
  int i;

  /* The C standard does not require that a char has 8 bits (but almost all systems have 8 bit chars). */
  grm_static_assert(CHAR_BIT == 8, "A char must consist of 8 bits for this code to work.");

  /* Ignore padding characters */
  while (src[block_len - 1] == PADDING_CHAR && block_len > 0)
    {
      --block_len;
    }
  if (block_len < 2)
    {
#ifndef NDEBUG
      if (block_len > 0)
        {
          logger((stderr, "At least two characters are needed for decoding. The character \"%c\" will be ignored.\n",
                  src[0]));
        }
      else
        {
          logger((stderr, "At least two characters are needed for decoding.\n"));
        }
#endif
      return GRM_ERROR_BASE64_BLOCK_TOO_SHORT;
    }

  /*
   * First, lookup the input characters in the `base64_decode_table` and interpret the result as sextets.
   *
   * Then, interpret the sextets a, b, c and d as bytes x, y and z:
   *
   *   aaaaaabbbbbbccccccdddddd
   *   xxxxxxxxyyyyyyyyzzzzzzzz
   *
   * If less than four sextets are given, ignore as many trailing zero bits as necessary in the last sextet.
   * (The following implementation ignores trailing bits regardless how they are set.)
   *
   * Only two sextets given:
   *
   *           0000  <- the last four bits of b must be zero
   *   aaaaaabbbbbb
   *   xxxxxxxx
   *
   * The sextet b only has two significant bits for x. The rest is ignored and must be zero bits.
   *
   * Only two sextets given:
   *
   *                   00  <- the last two bits of c must be zero
   *   aaaaaabbbbbbcccccc
   *   xxxxxxxxyyyyyyyy
   *
   * The sextet c only has four significant bits for y. The rest is ignored and must be zero bits.
   *
   */
  for (i = 0; i < block_len; ++i)
    {
      if (!(('A' <= src[i] && src[i] <= 'Z') || ('a' <= src[i] && src[i] <= 'z') || ('0' <= src[i] && src[i] <= '9') ||
            src[i] == '+' || src[i] == '/'))
        {
          logger((stderr, "The character \"%c\" is not a valid Base64 input character. Aborting.\n", src[i]));
          return GRM_ERROR_BASE64_INVALID_CHARACTER;
        }
      lookup_buffer[i] = BASE64_DECODE_TABLE[(int)src[i]];
    }

  dst[0] = (lookup_buffer[0] << 2) | (lookup_buffer[1] >> 4);
  if (block_len >= 3)
    {
      dst[1] = (lookup_buffer[1] << 4) | (lookup_buffer[2] >> 2);
      if (block_len == 4)
        {
          dst[2] = (lookup_buffer[2] << 6) | lookup_buffer[3];
        }
    }

  if (decoded_block_len != NULL) *decoded_block_len = block_len - 1;

  return GRM_ERROR_NONE;
}


grm_error_t blockEncode(char dst[4], const char src[3], int block_len)
{
  /* Interpret three 8-bit characters as four 6-bit indices for the `base64_encode_table` */

  /* The C standard does not require that a char has 8 bits (but almost all systems have 8 bit chars). */
  grm_static_assert(CHAR_BIT == 8, "A char must consist of 8 bits for this code to work.");

  if (block_len < 1)
    {
      logger((stderr, "At least one byte is needed for encoding.\n"));
      return GRM_ERROR_BASE64_BLOCK_TOO_SHORT;
    }

  /*
   * Interpret bytes x, y and z as sextets a, b, c and d:
   *
   *   xxxxxxxxyyyyyyyyzzzzzzzz
   *   aaaaaabbbbbbccccccdddddd
   *
   * Assign the sextets to bytes and therefore zero out the two highest bits (`& '\x3f'`).
   *
   * If less than three bytes are given:
   *   - If a sextet does not get any bits, set a padding character (`=`)
   *   - Otherwise, fill missing bits with zeros
   *
   * Only one byte given:
   *
   *   xxxxxxxx
   *   aaaaaabb
   *
   * The sextet b only gets two bits from x. Fill the other 4 bits with zeros. Set a padding character for c and d.
   *
   * Only two bytes given:
   *
   *   xxxxxxxxyyyyyyyy
   *   aaaaaabbbbbbcccc
   *
   * The sextet c only gets four bits from y. Fill the missing 2 bits with zeros. Set a padding character for d.
   *
   */
  dst[0] = BASE64_ENCODE_TABLE[(((unsigned char)src[0]) >> 2) & '\x3f'];
  if (block_len < 2)
    {
      dst[1] = BASE64_ENCODE_TABLE[(((unsigned char)src[0]) << 4) & '\x3f'];
      dst[2] = dst[3] = PADDING_CHAR;
    }
  else
    {
      dst[1] = BASE64_ENCODE_TABLE[((((unsigned char)src[0]) << 4) | (((unsigned char)src[1]) >> 4)) & '\x3f'];
      if (block_len < 3)
        {
          dst[2] = BASE64_ENCODE_TABLE[(((unsigned char)src[1]) << 2) & '\x3f'];
          dst[3] = PADDING_CHAR;
        }
      else
        {
          dst[2] = BASE64_ENCODE_TABLE[((((unsigned char)src[1]) << 2) | (((unsigned char)src[2]) >> 6)) & '\x3f'];
          dst[3] = BASE64_ENCODE_TABLE[((unsigned char)src[2]) & '\x3f'];
        }
    }

  return GRM_ERROR_NONE;
}


char *base64Decode(char *dst, const char *src, size_t *dst_len, grm_error_t *error)
{
  size_t src_len, max_dst_len;
  size_t dst_index, src_index;
  int decoded_block_len;
  grm_error_t err = GRM_ERROR_NONE;

  src_len = strlen(src);

  /* Always round up to multiple of 3 */
  max_dst_len = (3 * src_len) / 4;
  max_dst_len += (3 - max_dst_len % 3) % 3;

  if (dst == NULL)
    {
      /* Allocate an extra byte for string termination */
      dst = malloc(max_dst_len + 1);
      if (dst == NULL)
        {
          logger((stderr, "Could not allocate memory for the destination buffer. Aborting.\n"));
          err = GRM_ERROR_MALLOC;
          goto finally;
        }
    }

  for (dst_index = 0, src_index = 0; src_index < src_len; dst_index += decoded_block_len, src_index += 4)
    {
      err = blockDecode(dst + dst_index, src + src_index, grm_min(src_len - src_index, 4), &decoded_block_len);
      if (err != GRM_ERROR_NONE) break;
    }
  if (dst_len != NULL) *dst_len = dst_index;

finally:
  if (dst != NULL) dst[dst_index] = '\0'; /* Always add a string terminator */
  if (error != NULL) *error = err;

  return dst;
}


char *base64Encode(char *dst, const char *src, size_t src_len, grm_error_t *error)
{
  size_t dst_len;
  size_t dst_index, src_index;
  grm_error_t err = GRM_ERROR_NONE;

  /* Always round up to multiple of 4 */
  dst_len = (4 * src_len) / 3;
  dst_len += (4 - dst_len % 4) % 4;

  if (dst == NULL)
    {
      /* Allocate an extra byte for string termination */
      dst = malloc(dst_len + 1);
      if (dst == NULL)
        {
          logger((stderr, "Could not allocate memory for the destination buffer. Aborting.\n"));
          err = GRM_ERROR_MALLOC;
          goto finally;
        }
    }

  for (dst_index = 0, src_index = 0; src_index < src_len; dst_index += 4, src_index += 3)
    {
      err = blockEncode(dst + dst_index, src + src_index, grm_min(src_len - src_index, 3));
      if (err != GRM_ERROR_NONE) break;
    }

finally:
  if (dst != NULL) dst[dst_index] = '\0'; /* Always add a string terminator */
  if (error != NULL) *error = err;

  return dst;
}


/* ######################### public implementation ################################################################## */

/* ========================= functions ============================================================================== */

char *grm_base64_decode(char *dst, const char *src, size_t *dst_len, int *was_successful)
{
  grm_error_t error;
  char *decoded_string;

  decoded_string = base64Decode(dst, src, dst_len, &error);

  if (was_successful != NULL) *was_successful = error == GRM_ERROR_NONE;

  return decoded_string;
}

char *grm_base64_encode(char *dst, const char *src, size_t src_len, int *was_successful)
{
  grm_error_t error;
  char *encoded_string;

  encoded_string = base64Encode(dst, src, src_len, &error);

  if (was_successful != NULL) *was_successful = error == GRM_ERROR_NONE;

  return encoded_string;
}
