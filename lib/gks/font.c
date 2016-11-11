
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if !defined(VMS) && !defined(_WIN32)
#include <unistd.h>
#else
#include <io.h>
#define lseek _lseek
#endif

#include "gks.h"
#include "gkscore.h"

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

static
int font_cache[95], bufcache[95][256], gks = -1;

int gks_open_font(void)
{
  const char *path;
  char fontdb[MAXPATHLEN];
  int fd;

  path = gks_getenv("GKS_FONTPATH");
  if (path == NULL)
    {
      path = gks_getenv("GRDIR");
      if (path == NULL)
        path = GRDIR;
    }
  strcpy(fontdb, (char *) path);
#ifndef _WIN32
  strcat(fontdb, "/fonts/gksfont.dat");
#else
  strcat(fontdb, "\\FONTS\\GKSFONT.DAT");
#endif
  fd = gks_open_file(fontdb, "r");

  return fd;
}

void gks_lookup_font(int fd, int version, int font, int chr, stroke_data_t *s)
{
  /*  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 */
  static int map[] = {
      1,18, 1, 6,12, 3, 8,11, 4, 7,10, 2,13,14, 5, 9,15,16,17,20,21,19,22,23
  };
  static int gksgralmap[] = {
      1,12, 6, 9, 8,11, 5,13,18,17,19, 1, 4, 7,24, 1, 1, 1, 1, 1, 1, 1,23,24
  };
  static int s_map[] = {
      4, 4, 4, 4, 4, 7, 7, 7,10,10,10, 7, 7, 7, 4, 4, 7, 7, 7, 4, 4, 4, 4, 4
  };

  static int german[] = {
    196, 214, 220, 228, 246, 252, 223, 171, 187, 183, 169
  };
  static char ansi[] = {
    'A', 'O', 'U', 'a', 'o', 'u', 'b', '<', '>', '.', '@'
  };
  static char greek[] = {
    'j', 'o', 'q', 'u', 'v', 'w', 'y', 'J', 'O', 'Q', 'U', 'V', 'W', 'Y'
  };
  static char g_map[] = {
    ' ', 'w', ' ', 'o', 'y', 'v', 'q', ' ', 'W', ' ', 'O', 'Y', 'V', 'Q'
  };

  char buf[256];
  int umlaut, sharp_s, offset;
  int i, *elptr;
  char *ebptr;

  if (gks == -1)
    {
      for (i = 0; i < 95; i++)
	font_cache[i] = -1;
    }

  if (fd != -1)
    {
      umlaut = sharp_s = 0;

      if (chr < 0)
        chr += 256;

      if (chr >= 127)
	{
	  for (i = 0; i <= 10; i++)
	    {
	      if (chr == german[i])
		{
		  chr = ansi[i];
		  if (i < 6)
		    umlaut = 1;
		  else if (i == 6)
		    sharp_s = 1;
		}
	    }
	}
      if (chr < ' ' || chr >= 127)
	chr = ' ';

      font = abs(font) % 100;
      if (font == 51)
	font = 23;		/* fill font */
      else if (font > 23)
	font = 1;

      if (chr == '_')
	{
	  if (font < 20)
	    font = 23;
	}
      else if (sharp_s)
	{
	  if (font != 23)
	    font = s_map[font - 1];
	  else
	    chr = 126;		/* ~ */
	}
      else if (version == GRALGKS)
	{
	  if (font == 13 || font == 14)
	    {
	      for (i = 0; i < 14; i++)
		{
		  if (chr == greek[i])
		    {
		      chr = g_map[i];
		      break;
		    }
		}
            }
          font = gksgralmap[font - 1];
        }

      chr -= ' ';
      offset = ((map[font - 1] - 1) * 95 + chr) * 256;

      if (font_cache[chr] != offset)
	{
	  if (lseek(fd, offset, 0) != -1)
	    {
	      if (gks_read_file(fd, buf, 256) != -1)
		{
		  font_cache[chr] = offset;

		  elptr = bufcache[chr];
		  ebptr = buf;
		  for (i = 0; i < 256; i++)
		    *elptr++ = *ebptr++;
		}
	      else
		gks_fatal_error("GKS (gksio): file read error");
	    }
	  else
	    gks_fatal_error("GKS (gksio): file position error");
	}
      memmove((void *) s, (void *) bufcache[chr], 256 * sizeof(int));

      if (umlaut && (s->length < 120 - 20))
	s->length += 10;
    }
  else
    {
      gks_fatal_error("GKS (gksio): can't access font database");
    }
}

void gks_close_font(int fd)
{
  if (fd != -1)
    gks_close_file(fd);
}
