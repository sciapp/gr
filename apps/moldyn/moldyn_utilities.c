#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32
#include <conio.h>
#endif
#include "moldyn.h"

#ifndef MOLDYN_VERSION
#define MOLDYN_VERSION "MolDyn"
#endif
#ifndef MOLDYN_REVISION
#define MOLDYN_REVISION ""
#endif

char *program_name = "moldyn";
static const char *help_message[] = {"where options include:                      " MOLDYN_VERSION " " MOLDYN_REVISION
                                     "",
                                     "    -povray n             povray [0]         by J.Heinen (initial version),",
                                     "    -atoms n              number of atoms         A.Peters, F.Rhiem",
                                     "    -bonds (yes|no|chain) display bonds",
                                     "    -box (yes|no|f)       display bounding box",
                                     "    -colors (yes|no)      use colors",
                                     "    -color[1-118] c       color names",
                                     "    -delta f              delta criterion",
                                     "    -linewidth f          linewidth scale factor [1]",
                                     "    -magstep f            magnification (1.2**magstep)",
                                     "    -numbers (on|off)     display atom numbers",
                                     "    -radius[1-118] f      radius",
                                     "    -rot f                angle of rotation [0]",
                                     "    -step n               step [10]",
                                     "    -tilt f               angle of tilt [0]",
                                     "    -tolerance f          tolerance for above delta criterion",
                                     "    -resolution n         resolution [555]",
                                     "    -autoscale (yes|no)   do scaling regarding all scenes [yes]",
                                     "Keyboard bindings:",
                                     " <Leftarrow>          rotate left   a/j  write povray/jpeg file(s)/movie",
                                     "<Rightarrow>          rotate right  b/n  back/next (previous/next cycle)",
                                     "   <Uparrow>          tilt up       c/p  capture/print (moldyn.[jpg|eps])",
                                     " <Downarrow>          tilt down     h    hold",
                                     "    <Return>,<Esc>,q  quit          m/r  magnify/reduce",
                                     NULL};

void moldyn_log(const char *log_message)
{
  fprintf(stderr, "%s: %s\n", program_name, log_message);
}

void moldyn_error(const char *error_message)
{
  fprintf(stderr, "%s: %s\n", program_name, error_message);
  moldyn_exit(1);
}

void moldyn_exit(int error_code)
{
  moldyn_close_file();
  exit(error_code);
}

void moldyn_usage(void)
{
  char **cpp;
  fprintf(stderr, "usage:  %s file [-options]\n", program_name);

  for (cpp = (char **)help_message; *cpp; cpp++)
    {
      fprintf(stderr, "%s\n", *cpp);
    }
#ifdef _WIN32
  fprintf(stderr, "(Press any key to exit)");
  _getch();
#endif
  moldyn_exit(0);
}
