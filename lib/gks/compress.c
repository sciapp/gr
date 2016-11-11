
#include <stdio.h>
#include <string.h>

#include "gkscore.h"

typedef unsigned char byte;

#define MAXBITS  12
#define HSIZE  5003			/* 80% occupancy */

#define MAXCODE(n_bits)	 ((1 << (n_bits)) - 1)

static unsigned long cur_accum = 0;
static int           cur_bits = 0;

static int n_bits;			/* number of bits/code */
static int maxbits = MAXBITS;		/* user settable max # bits/code */
static int maxcode;			/* maximum code, given n_bits */
static int maxmaxcode = 1 << MAXBITS;

static long           htab[HSIZE];
static unsigned short codetab[HSIZE];

static int hsize = HSIZE;		/* for dynamic table sizing */

static int free_ent = 0;		/* first unused entry */

/*
 * block compression parameters -- after all codes are used up,
 * and compression rate changes, start over.
 */
static int clear_flg = 0;

static int init_bits;

static byte *s;				/* pointer to output stream */
static int s_len;

static int ClearCode;
static int EOFCode;


/*
 * Number of characters so far in this 'packet'
 */
static int a_count;

/*
 * Set up the 'byte output' routine
 */
static void char_init()
{
  a_count = 0;
}

/*
 * Define the storage for the packet accumulator
 */
static char accum[256];

/*
 * Flush the packet to disk, and reset the accumulator
 */
static void flush_char()
{
  int i;

  if (a_count > 0) {
    *s++ = a_count;
    for (i = 0; i < a_count; i++)
      *s++ = accum[i];
    s_len += a_count + 1;
    a_count = 0;
  }
}	

/*
 * Add a character to the end of the current packet, and if it is 254
 * characters, flush the packet to disk.
 */
static void char_out(int c)
{
  accum[a_count++] = c;
  if (a_count >= 254) 
    flush_char();
}


static
unsigned long masks[] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F,
                                  0x001F, 0x003F, 0x007F, 0x00FF,
                                  0x01FF, 0x03FF, 0x07FF, 0x0FFF,
                                  0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

static void output(int code)
{
  cur_accum &= masks[cur_bits];

  if (cur_bits > 0)
    cur_accum |= ((long)code << cur_bits);
  else
    cur_accum = code;
	
  cur_bits += n_bits;

  while (cur_bits >= 8) {
    char_out ((unsigned int) (cur_accum & 0xff));
    cur_accum >>= 8;
    cur_bits -= 8;
  }

  /*
   * If the next entry is going to be too big for the code size,
   * then increase it, if possible.
   */

  if (free_ent > maxcode || clear_flg) {

    if (clear_flg) {
      maxcode = MAXCODE(n_bits = init_bits);
      clear_flg = 0;
    }
    else {
      n_bits++;
      if (n_bits == maxbits)
	maxcode = maxmaxcode;
      else
	maxcode = MAXCODE(n_bits);
    }
  }
	
  if (code == EOFCode) {
    while (cur_bits > 0) {
      char_out ((unsigned int) (cur_accum & 0xff));
      cur_accum >>= 8;
      cur_bits -= 8;
    }

    flush_char();
  }
}


static void cl_hash(register long hsize) /* reset code table */
{
  int i;

  for (i = 0; i < hsize; i++)
    htab[i] = -1;
}


static void cl_block ()			/* table clear for block compress */
{
  /* Clear out the hash table */

  cl_hash ((long) hsize);
  free_ent = ClearCode + 2;
  clear_flg = 1;

  output(ClearCode);
}


void gks_compress(int bits, byte *in, int in_len, byte *out, int *out_len)
{
  long fcode;
  int i = 0;
  int c;
  int ent;
  int disp;
  int hsize_reg;
  int hshift;

  init_bits = bits;

  /* initialize 'gkscompress' globals */

  maxbits = MAXBITS;
  maxmaxcode = 1 << MAXBITS;
  memset((void *) htab, 0, sizeof(htab));
  memset((void *) codetab, 0, sizeof(codetab));
  hsize = HSIZE;
  free_ent = 0;
  clear_flg = 0;
  cur_accum = 0;
  cur_bits = 0;

  /*
   * Set up the necessary values
   */
  clear_flg = 0;
  maxcode = MAXCODE(n_bits = init_bits);

  ClearCode = (1 << (bits - 1));
  EOFCode = ClearCode + 1;
  free_ent = ClearCode + 2;

  char_init();
  s = out;
  s_len = 0;
  ent = *in++;  in_len--;

  hshift = 0;
  for (fcode = (long) hsize;  fcode < 65536L; fcode *= 2L)
    hshift++;
  hshift = 8 - hshift;			/* set hash code range bound */

  hsize_reg = hsize;
  cl_hash ((long) hsize_reg);		/* clear hash table */

  output(ClearCode);
    
  while (in_len) {
    c = *in++;  in_len--;

    fcode = (long) (((long) c << maxbits) + ent);
    i = (((int) c << hshift) ^ ent);	/* xor hashing */

    if (htab[i] == fcode) {
      ent = codetab[i];
      continue;
    }

    else if ((long) htab[i] < 0)	/* empty slot */
      goto nomatch;

    disp = hsize_reg - i;		/* secondary hash (after G. Knott) */
    if (i == 0)
      disp = 1;

probe:
    if ((i -= disp) < 0)
      i += hsize_reg;

    if (htab[i] == fcode) {
      ent = codetab[i];
      continue;
    }

    if ((long)htab[i] >= 0) 
      goto probe;

nomatch:
    output(ent);
    ent = c;

    if (free_ent < maxmaxcode) {
      codetab[i] = free_ent++;		/* code -> hashtable */
      htab[i] = fcode;
    }
    else
      cl_block();
  }

  /* Put out the final code */
  output(ent);
  output(EOFCode);

  *out_len = s_len;
}
