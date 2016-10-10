
#include <stdio.h>
#include <string.h>

#define MD5_BIG_ENDIAN 0

#include "md5.h"

typedef unsigned int md5_uint32;

#define MAX_MD5_UINT32 ((md5_uint32)4294967295U)
#define MD5_BLOCK_SIZE 64
#define BLOCK_SIZE_MASK (MD5_BLOCK_SIZE - 1)

typedef struct
{
  md5_uint32 md_A;
  md5_uint32 md_B;
  md5_uint32 md_C;
  md5_uint32 md_D;
  md5_uint32 md_total[2];
  md5_uint32 md_buf_len;
  char md_buffer[MD5_BLOCK_SIZE * 2];
} md5_t;

#if MD5_BIG_ENDIAN
#define SWAP(n) \
 (((n) << 24) | (((n) & 0xff00) << 8) | (((n) >> 8) & 0xff00) | ((n) >> 24))
#else
#define SWAP(n) (n)
#endif

#define FF(b, c, d)     (d ^ (b & (c ^ d)))
#define FG(b, c, d)     FF(d, b, c)
#define FH(b, c, d)     (b ^ c ^ d)
#define FI(b, c, d)     (c ^ (b | ~d))

#define CYCLIC(w, s)    ((w << s) | (w >> (32 - s)))

#define OP1(a, b, c, d, b_p, c_p, s, T)          \
     do {                                        \
       memmove(c_p, b_p, sizeof(md5_uint32));    \
       *c_p = SWAP(*c_p);                        \
       a += FF (b, c, d) + *c_p + T;             \
       a = CYCLIC (a, s);                        \
       a += b;                                   \
       b_p = (char *)b_p + sizeof(md5_uint32);   \
       c_p++;                                    \
    } while (0)

#define OP234(FUNC, a, b, c, d, k, s, T)         \
    do {                                         \
      a += FUNC (b, c, d) + k + T;               \
      a = CYCLIC (a, s);                         \
      a += b;                                    \
    } while (0)

static
void process_block(md5_t *md5_p, const void *buffer, const unsigned int buf_len)
{
  md5_uint32 correct[16];
  const void *buf_p = buffer, *end_p;
  unsigned int words_n;
  md5_uint32 A, B, C, D;

  words_n = buf_len / sizeof(md5_uint32);
  end_p = (char *)buf_p + words_n * sizeof(md5_uint32);

  A = md5_p->md_A;
  B = md5_p->md_B;
  C = md5_p->md_C;
  D = md5_p->md_D;

  if (md5_p->md_total[0] > MAX_MD5_UINT32 - buf_len)
    {
      md5_p->md_total[1]++;
      md5_p->md_total[0] -= (MAX_MD5_UINT32 + 1 - buf_len);
    }
  else
    md5_p->md_total[0] += buf_len;

  while (buf_p < end_p)
    {
      md5_uint32 A_save, B_save, C_save, D_save;
      md5_uint32 *corr_p = correct;

      A_save = A;
      B_save = B;
      C_save = C;
      D_save = D;

      OP1 (A, B, C, D, buf_p, corr_p,  7, 0xd76aa478);
      OP1 (D, A, B, C, buf_p, corr_p, 12, 0xe8c7b756);
      OP1 (C, D, A, B, buf_p, corr_p, 17, 0x242070db);
      OP1 (B, C, D, A, buf_p, corr_p, 22, 0xc1bdceee);
      OP1 (A, B, C, D, buf_p, corr_p,  7, 0xf57c0faf);
      OP1 (D, A, B, C, buf_p, corr_p, 12, 0x4787c62a);
      OP1 (C, D, A, B, buf_p, corr_p, 17, 0xa8304613);
      OP1 (B, C, D, A, buf_p, corr_p, 22, 0xfd469501);
      OP1 (A, B, C, D, buf_p, corr_p,  7, 0x698098d8);
      OP1 (D, A, B, C, buf_p, corr_p, 12, 0x8b44f7af);
      OP1 (C, D, A, B, buf_p, corr_p, 17, 0xffff5bb1);
      OP1 (B, C, D, A, buf_p, corr_p, 22, 0x895cd7be);
      OP1 (A, B, C, D, buf_p, corr_p,  7, 0x6b901122);
      OP1 (D, A, B, C, buf_p, corr_p, 12, 0xfd987193);
      OP1 (C, D, A, B, buf_p, corr_p, 17, 0xa679438e);
      OP1 (B, C, D, A, buf_p, corr_p, 22, 0x49b40821);

      OP234 (FG, A, B, C, D, correct[  1],  5, 0xf61e2562);
      OP234 (FG, D, A, B, C, correct[  6],  9, 0xc040b340);
      OP234 (FG, C, D, A, B, correct[ 11], 14, 0x265e5a51);
      OP234 (FG, B, C, D, A, correct[  0], 20, 0xe9b6c7aa);
      OP234 (FG, A, B, C, D, correct[  5],  5, 0xd62f105d);
      OP234 (FG, D, A, B, C, correct[ 10],  9, 0x02441453);
      OP234 (FG, C, D, A, B, correct[ 15], 14, 0xd8a1e681);
      OP234 (FG, B, C, D, A, correct[  4], 20, 0xe7d3fbc8);
      OP234 (FG, A, B, C, D, correct[  9],  5, 0x21e1cde6);
      OP234 (FG, D, A, B, C, correct[ 14],  9, 0xc33707d6);
      OP234 (FG, C, D, A, B, correct[  3], 14, 0xf4d50d87);
      OP234 (FG, B, C, D, A, correct[  8], 20, 0x455a14ed);
      OP234 (FG, A, B, C, D, correct[ 13],  5, 0xa9e3e905);
      OP234 (FG, D, A, B, C, correct[  2],  9, 0xfcefa3f8);
      OP234 (FG, C, D, A, B, correct[  7], 14, 0x676f02d9);
      OP234 (FG, B, C, D, A, correct[ 12], 20, 0x8d2a4c8a);

      OP234 (FH, A, B, C, D, correct[  5],  4, 0xfffa3942);
      OP234 (FH, D, A, B, C, correct[  8], 11, 0x8771f681);
      OP234 (FH, C, D, A, B, correct[ 11], 16, 0x6d9d6122);
      OP234 (FH, B, C, D, A, correct[ 14], 23, 0xfde5380c);
      OP234 (FH, A, B, C, D, correct[  1],  4, 0xa4beea44);
      OP234 (FH, D, A, B, C, correct[  4], 11, 0x4bdecfa9);
      OP234 (FH, C, D, A, B, correct[  7], 16, 0xf6bb4b60);
      OP234 (FH, B, C, D, A, correct[ 10], 23, 0xbebfbc70);
      OP234 (FH, A, B, C, D, correct[ 13],  4, 0x289b7ec6);
      OP234 (FH, D, A, B, C, correct[  0], 11, 0xeaa127fa);
      OP234 (FH, C, D, A, B, correct[  3], 16, 0xd4ef3085);
      OP234 (FH, B, C, D, A, correct[  6], 23, 0x04881d05);
      OP234 (FH, A, B, C, D, correct[  9],  4, 0xd9d4d039);
      OP234 (FH, D, A, B, C, correct[ 12], 11, 0xe6db99e5);
      OP234 (FH, C, D, A, B, correct[ 15], 16, 0x1fa27cf8);
      OP234 (FH, B, C, D, A, correct[  2], 23, 0xc4ac5665);

      OP234 (FI, A, B, C, D, correct[  0],  6, 0xf4292244);
      OP234 (FI, D, A, B, C, correct[  7], 10, 0x432aff97);
      OP234 (FI, C, D, A, B, correct[ 14], 15, 0xab9423a7);
      OP234 (FI, B, C, D, A, correct[  5], 21, 0xfc93a039);
      OP234 (FI, A, B, C, D, correct[ 12],  6, 0x655b59c3);
      OP234 (FI, D, A, B, C, correct[  3], 10, 0x8f0ccc92);
      OP234 (FI, C, D, A, B, correct[ 10], 15, 0xffeff47d);
      OP234 (FI, B, C, D, A, correct[  1], 21, 0x85845dd1);
      OP234 (FI, A, B, C, D, correct[  8],  6, 0x6fa87e4f);
      OP234 (FI, D, A, B, C, correct[ 15], 10, 0xfe2ce6e0);
      OP234 (FI, C, D, A, B, correct[  6], 15, 0xa3014314);
      OP234 (FI, B, C, D, A, correct[ 13], 21, 0x4e0811a1);
      OP234 (FI, A, B, C, D, correct[  4],  6, 0xf7537e82);
      OP234 (FI, D, A, B, C, correct[ 11], 10, 0xbd3af235);
      OP234 (FI, C, D, A, B, correct[  2], 15, 0x2ad7d2bb);
      OP234 (FI, B, C, D, A, correct[  9], 21, 0xeb86d391);

      A += A_save;
      B += B_save;
      C += C_save;
      D += D_save;
    }

  md5_p->md_A = A;
  md5_p->md_B = B;
  md5_p->md_C = C;
  md5_p->md_D = D;
}

static
void md5_get_result(const md5_t *md5_p, void *result)
{
  md5_uint32 hold;
  void *res_p = result;

  hold = SWAP(md5_p->md_A);
  memmove(res_p, &hold, sizeof(md5_uint32));
  res_p = (char *)res_p + sizeof(md5_uint32);

  hold = SWAP(md5_p->md_B);
  memmove(res_p, &hold, sizeof(md5_uint32));
  res_p = (char *)res_p + sizeof(md5_uint32);

  hold = SWAP(md5_p->md_C);
  memmove(res_p, &hold, sizeof(md5_uint32));
  res_p = (char *)res_p + sizeof(md5_uint32);

  hold = SWAP(md5_p->md_D);
  memmove(res_p, &hold, sizeof(md5_uint32));
}

static
void md5_init(md5_t *md5_p)
{
  md5_p->md_A = 0x67452301;
  md5_p->md_B = 0xefcdab89;
  md5_p->md_C = 0x98badcfe;
  md5_p->md_D = 0x10325476;

  md5_p->md_total[0] = 0;
  md5_p->md_total[1] = 0;
  md5_p->md_buf_len = 0;
}

static
void md5_process(md5_t *md5_p, const void *buffer, const unsigned int buf_len) 
{
  unsigned int len = buf_len;
  unsigned int in_block, add;

  if (md5_p->md_buf_len > 0)
    {
      in_block = md5_p->md_buf_len;
      if (in_block + len > sizeof(md5_p->md_buffer))
        add = sizeof(md5_p->md_buffer) - in_block;
      else
        add = len;

      memmove(md5_p->md_buffer + in_block, buffer, add);
      md5_p->md_buf_len += add;
      in_block += add;

      if (in_block > MD5_BLOCK_SIZE)
        {
          process_block (md5_p, md5_p->md_buffer, in_block & ~BLOCK_SIZE_MASK);
          memmove(md5_p->md_buffer,
                  md5_p->md_buffer + (in_block & ~BLOCK_SIZE_MASK),
                  in_block & BLOCK_SIZE_MASK);
          md5_p->md_buf_len = in_block & BLOCK_SIZE_MASK;
        }

      buffer = (const char *)buffer + add;
      len -= add;
    }

  if (len > MD5_BLOCK_SIZE)
    {
      process_block (md5_p, buffer, len & ~BLOCK_SIZE_MASK);
      buffer = (const char *) buffer + (len & ~BLOCK_SIZE_MASK);
      len &= BLOCK_SIZE_MASK;
    }

  if (len > 0)
    {
      memmove(md5_p->md_buffer, buffer, len);
      md5_p->md_buf_len = len;
    }
}

static
void md5_finish(md5_t *md5_p, void *signature)
{
  md5_uint32 bytes, hold;
  int pad;

  bytes = md5_p->md_buf_len;

  if (md5_p->md_total[0] > MAX_MD5_UINT32 - bytes)
    {
      md5_p->md_total[1]++;
      md5_p->md_total[0] -= (MAX_MD5_UINT32 + 1 - bytes);
    }
  else
    md5_p->md_total[0] += bytes;

  pad = MD5_BLOCK_SIZE - (sizeof(md5_uint32) * 2) - bytes;
  if (pad <= 0)
    pad += MD5_BLOCK_SIZE;

  if (pad > 0)
    {
      md5_p->md_buffer[bytes] = (unsigned char)0x80;
      if (pad > 1)
        memset (md5_p->md_buffer + bytes + 1, 0, pad - 1);
      bytes += pad;
    }

  hold = SWAP((md5_p->md_total[0] & 0x1FFFFFFF) << 3);
  memmove(md5_p->md_buffer + bytes, &hold, sizeof(md5_uint32));
  bytes += sizeof(md5_uint32);

  hold = SWAP((md5_p->md_total[1] << 3) |
              ((md5_p->md_total[0] & 0xE0000000) >> 29));
  memmove(md5_p->md_buffer + bytes, &hold, sizeof(md5_uint32));
  bytes += sizeof(md5_uint32);

  process_block(md5_p, md5_p->md_buffer, bytes);
  md5_get_result(md5_p, signature);
}

void md5(const char *buffer, char *sum)
{
  md5_t md5;
  unsigned char signature[MD5_SIZE];
  int i;

  md5_init(&md5);
  md5_process(&md5, buffer, strlen(buffer));
  md5_finish(&md5, signature);

  for (i = 0; i < MD5_SIZE; i++)
    sprintf(sum + 2*i, "%02x", signature[i]);
  sum[2 * MD5_SIZE] = '\0';
}
