
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gks.h"
#include "gkscore.h"

static
char *path = NULL;

static
int wstype = GKS_K_WSTYPE_DEFAULT;

static
int orientation = 0;

static
double factor = 1.0;

static
void usage(int help)
{
  fprintf(stderr, "\
Usage: gksm [-Oorientation] [-factor f] [-h] [-t wstype] [file]\n\
             -Oorientation   Select the orientation (landscape, portrait).\n\
                 -factor f   Specifies the magnification factor.\n\
                        -h   Prints this information.\n\
                 -t wstype   Use GKS workstation type wstype.\n");

  if (help)
    fprintf(stderr, "\n\
The present workstation types recognized by gksm are:\n\
  61:    PostScript (b/w)                     2:    GKS Metafile\n\
  62:    Color PostScript                     7:    CGM Binary\n\
  63:    PostScript (b/w, landscape)          8:    CGM Clear Text\n\
  64:    Color PostScript (landscape)\n\
 101:    Portable Document Format (PDF)\n\
 102:    Portable Document Format (PDF, compressed)\n\
 214:    X display w\\ Sun rle rasterfile dump\n\
 215:    X display w\\ Compuserve GIF dump\n\
 320:    Windows Bitmap File (BMP)\n\
 321:    JPEG File\n\
 322:    Portable Network Graphics (PNG)\n\
 323:    Tagged Image File (TIFF)\n\
 370:    Fig Format 3.2 (Xfig)\n\
 382:    Scalable Vector Graphics (SVG)\n\
 390:    Windows Metafile (WMF)\n\
 400:    Quartz 2D Graphics (Mac OS X)\n\
 410:    Java Web Plug-in\n");

  exit(1);
}

static
void parse(int argc, char **argv)
{
  char *option;
  int ret;

  argv++;
  while ((option = *argv++) != NULL)
    {
      if (!strcmp(option, "-Olandscape"))
        {
          orientation = 1;
        }
      else if (!strcmp(option, "-Oportrait"))
        {
          orientation = 2;
        }
      else if (!strcmp(option, "-factor"))
        {
          if (*argv)
            factor = atof(*argv++);
          else
            usage(0);
        }
      else if (!strcmp(option, "-h"))
	{
	  usage(1);
	}
      else if (!strcmp(option, "-t"))
	{
	  if (*argv)
	    wstype = atoi(*argv++);
	  else
	    usage(0);
	}
      else if (*option == '-')
	{
	  fprintf(stderr, "Invalid option: '%s'\n", option);
	  usage(0);
	}
      else
	path = option;
    }

  if (path != NULL)
    {
      ret = access(path, R_OK);
      if (ret)
	{
	  perror("open");
	  exit(-1);
	}
    }
  else
    {
      usage(0);
      exit(1);
    }
}

int main(int argc, char *argv[])
{
  int i, asf[13];
  int type, lenodr, maxodr = 100;
  char *item;

  parse(argc, argv);

  for (i = 0; i < 13; i++)
    asf[i] = GKS_K_ASF_INDIVIDUAL;

  gks_open_gks(6);
  gks_set_asf(asf);
  gks_open_ws(1, path, GKS_K_WSTYPE_MI);
  gks_activate_ws(1);
  gks_open_ws(2, GKS_K_CONID_DEFAULT, wstype);
  gks_activate_ws(2);

  switch (orientation)
    {
      case 1 :
        gks_set_ws_viewport(2, 0.0, 0.297, 0.0, 0.21);
        gks_set_ws_window(2, 0.0, 1.0, 0.0, 0.21 / 0.297);
        break;

      case 2 :
        gks_set_ws_viewport(2, 0.0, 0.21, 0.0, 0.297);
        gks_set_ws_window(2, 0.0, 0.21 / 0.297, 0.0, 1.0);
        break;
    }

  if (factor != 1.0)
    {
      double transx = 0, transy = 0, mat[3][2];

      if (factor < 1)
        {
          if (factor > 0)
	    {
	      transx = 0.5 * (factor - 1);
	      transy = -transx;
	    }
	  else
	    factor = -factor;
        }
      gks_create_seg(1);
      gks_eval_xform_matrix(
	0.5, 0.5, transx, transy, 0, factor, factor, GKS_K_COORDINATES_NDC, mat);
      gks_set_seg_xform(1, mat);
    }
 
  item = gks_malloc(maxodr * 80);

  gks_get_item(1, &type, &lenodr);
  while (type)
    {
      if (lenodr > maxodr)
	{
	  while (lenodr > maxodr)
	    maxodr *= 2;
	  item = gks_realloc(item, maxodr * 80);
	}
      gks_read_item(1, lenodr, maxodr, item);
      gks_interpret_item(type, lenodr, maxodr, item);
      gks_get_item(1, &type, &lenodr);
    }
  gks_update_ws(2, GKS_K_POSTPONE_FLAG);

  if (wstype == GKS_K_WSTYPE_DEFAULT)
    {
      printf("Press RETURN to continue ...");
      fflush(stdout);
      getchar();
    }

  gks_emergency_close();

  return 0;
}
