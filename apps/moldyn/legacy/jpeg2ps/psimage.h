/* -------------------------- psimage.h ------------------------- */
/* (C) Thomas Merz 1994-2002 */

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifdef min
#undef min
#endif
#define min(a, b) ((a) < (b) ? (a) : (b))

typedef int BOOL;

/* data output mode: binary, ascii85, hex-ascii */
typedef enum
{
  BINARY,
  ASCII85,
  ASCIIHEX
} DATAMODE;

typedef struct
{
  FILE *fp;               /* file pointer for jpeg file		 */
  char *filename;         /* name of image file			 */
  int width;              /* pixels per line			 */
  int height;             /* rows				 */
  int components;         /* number of color components		 */
  int bits_per_component; /* bits per color component		 */
  float dpi;              /* image resolution in dots per inch   */
  DATAMODE mode;          /* output mode: 8bit, ascii, ascii85	 */
  long startpos;          /* offset to jpeg data		 */
  BOOL landscape;         /* rotate image to landscape mode?	 */
  BOOL adobe;             /* image includes Adobe comment marker */
} imagedata;

#define DPI_IGNORE (float)(-1.0)  /* dummy value for imagedata.dpi       */
#define DPI_USE_FILE ((float)0.0) /* dummy value for imagedata.dpi       */
