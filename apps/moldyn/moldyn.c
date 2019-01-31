/* OpenGL via network might require LIBGL_ALWAYS_INDIRECT to be set */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "moldyn.h"

#define MAX_ARGS 32
#ifdef _WIN32
#define HUGE 100000000
#else
#ifndef HUGE
#define HUGE HUGE_VAL
#endif
#endif


#define SEPARATORS " \t"

static int ac;     /* argument counter */
static char **avp; /* argument pointer */

float cyl_rad; /* bond cylinder radius */

int magstep = 0;
double magnification = 1;

Bool file_done = False; /* file was read until the end? */

Bool jpeg = False; /* create a jpeg file? */

FILE *fptr = NULL;
int icycle = 0;
int current_cycle = 0;
fpos_t cycle_position[MAX_CYCLES];
double energy0 = 0, energy = 0; /* energy levels (?)*/
static int step = 10;           /* number of cycles skipped when reading cycles */

int resolution = 555; /* base-resolution for POV-Ray output */

Bool hint = False;             /* show help text? */
static Bool autoscale = False; /* read the whole file for scaling information of ALL frames instead of the first? */

float range; /* range of coordinate values in all three dimensions*/
double dist;
static double sscale; /* 1/scale */
#define S(a) ((a)*sscale)

static int max_atoms = 40;
static double linewidth = 1;

int povray = 0, pix = 0;
char name[256], path[266];
format_t format;
char title[MAX_STRING];

Bool numbers = True; /* show atom numbers? */
Bool bonds = True;   /* show atom bonds? */
Bool chain = False;  /* form atom bonds as a chain? */
Bool colors = True;
Bool box = False;       /* show bounding box? */
static double size = 0; /* bounding box size */
double radius = 0;

float *atom_positions = NULL;
float *atom_radii = NULL;
float *atom_colors = NULL;
int *atom_numbers = NULL;
int *atom_numbers2 = NULL;
char *atom_adjacency_matrix = NULL;
char **atom_names = NULL; /* An atom name is three or less characters long. */
float *atom_spins = NULL;

static char *CP = NULL; /* misc. string (?) */
static int CP_LEN;

static double delta = 0;
static double tolerance = 0;
int num_atoms = 0;
double xmin = HUGE, ymin = HUGE, zmin = HUGE;
double xmax = -HUGE, ymax = -HUGE, zmax = -HUGE;

static double global_xmin, global_xmax, global_ymin, global_ymax, global_zmin, global_zmax;
static double global_meanx, global_meany, global_meanz;

static void findRGB(char *newcolor, int *R, int *G, int *B);
static int atomname2atomnumber(char *atom_name);

static void read_dat();
static void read_check(FILE *fptr, int *n, char **argv);
static void analyze(void);


static void allocate_atom_memory(void);
static void free_atom_memory(void);

static void read_check(FILE *fptr, int *n, char **argv)
{
  int argc = 1;
  char line[MAX_STRING], *cp;
  double energy, dummy1, dummy2;
  int nitems;

  fgets(line, MAX_STRING, fptr);

  if (*line == '#')
    {
      CP = (char *)malloc(MAX_STRING * sizeof(char));
      strcpy(CP, line + 1);
      strtok(CP, "\n");
      CP_LEN = strlen(CP);

      while (*CP)
        {
          while (isspace(*CP)) CP++;
          if (*CP)
            {
              argv[argc++] = CP;
              while (*CP && !isspace(*CP)) CP++;
              while (isspace(*CP)) *CP++ = '\0';
            }
          else
            argv[argc] = NULL;
        }

      fgets(line, MAX_STRING, fptr);
    }
  else
    argv[argc] = NULL;

  if (sscanf(line, "%d %lg %lg %lg", &icycle, &dummy1, &energy, &dummy2) >= 4)
    format = normal;

  else if (sscanf(line, "%d", n) == 1)
    {
      format = xyz;
      rewind(fptr);
      return;
    }
  else if (fscanf(fptr, "%d", n) == 1)
    {
      format = unichem;
      rewind(fptr);
      return;
    }
  else
    moldyn_error("unknown format");

  *n = 0;

  while (True)
    {
      fgets(line, MAX_STRING, fptr);

      if (feof(fptr)) break;

      cp = line;
      nitems = 0;
      while (*cp)
        {
          while (isspace(*cp)) cp++;
          if (*cp)
            {
              while (!isspace(*cp)) cp++;
              nitems++;
            }
        }
      if (nitems != 5) break;

      (*n)++;
    }

  if (*n)
    rewind(fptr);
  else
    moldyn_error("can't obtain number of atoms");

  return;
}

static void read_dat(void)
{

  char **arg, *arg_vector[MAX_ARGS], *option;
  static int pass = 0;

  char *fn = NULL;
  int i;

  program_name = arg_vector[0] = avp[0];
  if (ac <= 1) moldyn_usage();

  fn = *++avp;

  if (*fn == '-') moldyn_usage();

  fptr = fopen(fn, "r");
  if (fptr == NULL) moldyn_error("can't open file");

  strcpy(name, fn);
  strtok(name, ".");

  for (i = 0; i < MAX_ARGS; i++) arg_vector[i] = NULL;

  read_check(fptr, &num_atoms, arg_vector);

  if (num_atoms > 40)
    {
      numbers = False;
      delta = -1;
      linewidth = 0.5;
    }

  for (pass = 0; pass < 2; pass++)
    {
      if (pass == 1)
        arg = avp;
      else
        arg = arg_vector;

      while (*++arg)
        {
          option = *arg++;
          if (!*arg)
            {
              moldyn_usage();
            }
          if (!strcmp(option, "-atoms"))
            {
              num_atoms = atoi(*arg);
              if (num_atoms >= 40000) bonds = False;
            }
          else if (!strcmp(option, "-bonds"))
            {
              if (!strcmp(*arg, "yes"))
                {
                  bonds = True;
                }
              else if (!strcmp(*arg, "no"))
                {
                  bonds = False;
                }
              else if (!strcmp(*arg, "chain"))
                {
                  bonds = chain = True;
                }
              else
                {
                  moldyn_usage();
                }
            }
          else if (!strcmp(option, "-box"))
            {
              if (!strcmp(*arg, "yes"))
                {
                  box = True;
                }
              else if (!strcmp(*arg, "no"))
                {
                  box = False;
                }
              else if (sscanf(*arg, "%lg", &size) != 1)
                {
                  moldyn_usage();
                }
              if (size > 0)
                {
                  box = True;
                }
            }
          else if (!strcmp(option, "-delta"))
            {
              delta = atof(*arg);
            }
          else if (!strcmp(option, "-tolerance"))
            {
              tolerance = atof(*arg);
            }
          else if (!strcmp(option, "-linewidth"))
            {
              linewidth = atof(*arg);
            }
          else if (!strcmp(option, "-magstep"))
            {
              magstep = -atof(*arg);
            }
          else if (!strcmp(option, "-numbers"))
            {
              if (!strcmp(*arg, "on"))
                {
                  numbers = True;
                }
              else if (!strcmp(*arg, "off"))
                {
                  numbers = False;
                }
              else
                {
                  moldyn_usage();
                }
            }
          else if (!strncmp(option, "-radius", 7))
            {
              if (option[7] == '\0')
                {
                  radius = atof(*arg);
                }
              else
                {
                  int ord;
                  ord = atoi(&option[7]);
                  element_radii[ord - 1] = fabs(atof(*arg));
                }
            }
          else if (!strcmp(option, "-rot"))
            {
              rotation = atof(*arg);
            }
          else if (!strcmp(option, "-tilt"))
            {
              tilt = atof(*arg);
            }
          else if (!strcmp(option, "-step"))
            {
              step = atoi(*arg);
            }
          else if (!strcmp(option, "-povray"))
            {
              povray = atoi(*arg);
              step = abs(povray);
            }
          else if (!strcmp(option, "-resolution"))
            {
              resolution = atoi(*arg);
              if (resolution < 256)
                {
                  resolution = 256;
                }
              else if (resolution > 2560)
                {
                  resolution = 2560;
                }
            }
          else if (!strcmp(option, "-colors"))
            {
              if (!strcmp(*arg, "yes"))
                {
                  colors = True;
                }
              else if (!strcmp(*arg, "no"))
                {
                  colors = False;
                }
              else
                {
                  moldyn_usage();
                }
            }
          else if (!strncmp(option, "-color", 6))
            {
              int R, G, B, ord;
              ord = atoi(&option[6]);
              findRGB(*arg, &R, &G, &B);
              element_colors[ord - 1][0] = R;
              element_colors[ord - 1][1] = G;
              element_colors[ord - 1][2] = B;
            }
          else if (!strcmp(option, "-autoscale"))
            {
              if (!strcmp(*arg, "yes"))
                {
                  autoscale = True;
                }
              else if (!strcmp(*arg, "no"))
                {
                  autoscale = False;
                }
              else
                {
                  moldyn_usage();
                }
            }
          else
            {
              moldyn_usage();
            }
        }
    }

  if (!autoscale)
    {
      if (num_atoms > max_atoms)
        {
          max_atoms = num_atoms * 1.2;
        }
      else
        {
          max_atoms += 5;
        }

      allocate_atom_memory();
    }

  pix = 0;
}

void read_cycle()
{
  static int dim;
  static double scale;
  static Bool init = False;
  double dummy1, dummy2;
  double meanx, meany, meanz;
  char c, s[4], *cp, *temp;
  int i, j, k;
  char line[MAX_STRING];
  Bool read_it = False;
  Bool done = False;
  int nbonds;
  fpos_t fpos;
  float dt;

  if (file_done)
    {
      return;
    }

  if (current_cycle < MAX_CYCLES)
    {
      fgetpos(fptr, &cycle_position[current_cycle++]);
    }
  else
    {
      moldyn_error("too many cycles");
    }

  for (k = 0; k < step; k++)
    {
      if (format == normal)
        {
          fgets(line, MAX_STRING, fptr);

          if (feof(fptr))
            {
              file_done = True;
              if (read_it)
                {
                  break;
                }
              current_cycle--;
              return;
            }
          else if (*line == '#')
            {
              fgets(line, MAX_STRING, fptr);
            }

          if (sscanf(line, "%d %lg %lg %lg", &icycle, &dummy1, &energy, &dummy2) < 4)
            {
              moldyn_error("can't read cycle record");
            }
          read_it = True;

          num_atoms = 0;
        }
      else if (format == xyz)
        {
          fgets(line, MAX_STRING, fptr);
          if (feof(fptr))
            {
              file_done = True;
              if (read_it) break;
              current_cycle--;
              return;
            }
          else if (*line == '#')
            {
              fgets(line, MAX_STRING, fptr);
              sscanf(line, "%d", &num_atoms);
            }
          else
            {
              sscanf(line, "%d", &num_atoms);
            }
          fgets(title, MAX_STRING, fptr);
          strtok(title, "\n");

          read_it = True;
        }
      else
        {
          fgets(line, MAX_STRING, fptr);
          if (feof(fptr))
            {
              file_done = True;
              if (read_it)
                {
                  break;
                }
              current_cycle--;
              return;
            }
          else if (*line == '#')
            fgets(line, MAX_STRING, fptr);

          strcpy(title, line);
          strtok(title, "\n");
          if (!isalnum(*title)) *title = '\0';

          fgets(line, MAX_STRING, fptr);
          sscanf(line, "%d", &num_atoms);

          if (feof(fptr))
            {
              file_done = True;
              if (read_it)
                {
                  break;
                }
              return;
            }
          read_it = True;
        }

      if (format != normal)
        {
          if (num_atoms > max_atoms)
            {
              max_atoms = num_atoms;
              allocate_atom_memory();
            }
          else
            {
              max_atoms = num_atoms;
            }
        }

      for (i = 0; i < max_atoms; i++)
        {
          fgetpos(fptr, &fpos);

          fgets(line, MAX_STRING, fptr);
          if (feof(fptr))
            {
              file_done = True;
              if (read_it)
                {
                  done = True;
                  break;
                }

              moldyn_error("missing data record");
              return;
            }

          if (format == normal)
            {
              atom_numbers[i] = atoi(strtok(line, SEPARATORS));
              temp = strtok(NULL, SEPARATORS);
              if (strchr(temp, '.') != NULL)
                {
                  fsetpos(fptr, &fpos);
                  break;
                }

              atom_numbers2[i] = atoi(temp);
              atom_positions[0 + 3 * i] = atof(strtok(NULL, SEPARATORS));
              atom_positions[1 + 3 * i] = atof(strtok(NULL, SEPARATORS));
              atom_positions[2 + 3 * i] = atof(strtok(NULL, SEPARATORS));

              atom_colors[0 + 3 * i] = element_colors[atom_numbers[i] - 1][0] / 255.0;
              atom_colors[1 + 3 * i] = element_colors[atom_numbers[i] - 1][1] / 255.0;
              atom_colors[2 + 3 * i] = element_colors[atom_numbers[i] - 1][2] / 255.0;

              num_atoms++;
            }
          else if (format == xyz)
            {
              cp = line;
              while (isspace(*cp))
                {
                  cp++;
                }
              if (sscanf(cp, "%3s %g %g %g", s, &atom_positions[0 + 3 * i], &atom_positions[1 + 3 * i],
                         &atom_positions[2 + 3 * i]) != 4)
                {
                  moldyn_error("can't read data record");
                }
              c = s[0];
              atom_numbers[i] = atomname2atomnumber(s);
              atom_numbers2[i] = 1;
              atom_colors[0 + 3 * i] = element_colors[atom_numbers[i] - 1][0] / 255.0;
              atom_colors[1 + 3 * i] = element_colors[atom_numbers[i] - 1][1] / 255.0;
              atom_colors[2 + 3 * i] = element_colors[atom_numbers[i] - 1][2] / 255.0;
              if (sscanf(cp, "%3s %g %g %g %g %g %g", s, &atom_positions[0 + 3 * i], &atom_positions[1 + 3 * i],
                         &atom_positions[2 + 3 * i], &atom_spins[0 + 3 * i], &atom_spins[1 + 3 * i],
                         &atom_spins[2 + 3 * i]) != 7)
                {
                  atom_spins[3 * i] = 0;
                  atom_spins[3 * i + 1] = 0;
                  atom_spins[3 * i + 2] = 0;
                }

              strcpy(atom_names[i], s);
            }
          else
            {
              cp = line;
              while (isspace(*cp))
                {
                  cp++;
                }
              if (sscanf(cp, "%d %g %g %g", &atom_numbers[i], &atom_positions[0 + 3 * i], &atom_positions[1 + 3 * i],
                         &atom_positions[2 + 3 * i]) != 4)
                {
                  moldyn_error("can't read data record");
                }
              atom_colors[0 + 3 * i] = element_colors[atom_numbers[i] - 1][0] / 255.0;
              atom_colors[1 + 3 * i] = element_colors[atom_numbers[i] - 1][1] / 255.0;
              atom_colors[2 + 3 * i] = element_colors[atom_numbers[i] - 1][2] / 255.0;
              atom_numbers2[i] = 1;
            }
          atom_positions[2 + 3 * i] = -atom_positions[2 + 3 * i];
        }
      if (done)
        {
          break;
        }
    }

  if (!init)
    {
      init = True;
      energy0 = energy;
    }

  xmin = atom_positions[0 + 3 * 0];
  xmax = atom_positions[0 + 3 * 0];
  ymin = atom_positions[1 + 3 * 0];
  ymax = atom_positions[1 + 3 * 0];
  zmin = atom_positions[2 + 3 * 0];
  zmax = atom_positions[2 + 3 * 0];

  for (i = 0; i < num_atoms; i++)
    {
      if (atom_positions[0 + 3 * i] < xmin) xmin = atom_positions[0 + 3 * i];
      if (atom_positions[0 + 3 * i] > xmax) xmax = atom_positions[0 + 3 * i];
      if (atom_positions[1 + 3 * i] < ymin) ymin = atom_positions[1 + 3 * i];
      if (atom_positions[1 + 3 * i] > ymax) ymax = atom_positions[1 + 3 * i];
      if (atom_positions[2 + 3 * i] < zmin) zmin = atom_positions[2 + 3 * i];
      if (atom_positions[2 + 3 * i] > zmax) zmax = atom_positions[2 + 3 * i];
    }

  meanx = (xmin + xmax) / 2;
  meany = (ymin + ymax) / 2;
  meanz = (zmin + zmax) / 2;

  xmin -= meanx;
  xmax -= meanx;
  ymin -= meany;
  ymax -= meany;
  zmin -= meanz;
  zmax -= meanz;

  if (size > 0)
    {
      xmin = -size;
      xmax = size;
      ymin = -size;
      ymax = size;
      zmin = -size;
      zmax = size;
    }

  dim = 3;
  if (xmax == xmin || ymax == ymin || zmax == zmin)
    {
      dim--;
    }
  scale = (xmax - xmin + ymax - ymin + zmax - zmin) / dim;
  while (scale < (zmax - zmin) / 2)
    {
      scale = scale * 1.5;
    }
  dist = 3 * scale;
  sscale = 1 / scale;

  if (delta <= 0)
    {
      if (bonds && delta < 0)
        {
          delta = 0;
          for (j = 1; j < num_atoms; j++)
            {
              i = j - 1;
              delta += sqrt((atom_positions[0 + 3 * i] - atom_positions[0 + 3 * j]) *
                                (atom_positions[0 + 3 * i] - atom_positions[0 + 3 * j]) +
                            (atom_positions[1 + 3 * i] - atom_positions[1 + 3 * j]) *
                                (atom_positions[1 + 3 * i] - atom_positions[1 + 3 * j]) +
                            (atom_positions[2 + 3 * i] - atom_positions[2 + 3 * j]) *
                                (atom_positions[2 + 3 * i] - atom_positions[2 + 3 * j]));
            }

          delta = 1.125 * delta / (num_atoms - 1);
        }
      else
        {
          delta = 2.25 * scale / sqrt((double)num_atoms);
        }

      while (bonds)
        {
          nbonds = 0;
          for (i = 0; i < num_atoms; i++)
            for (j = i + 1; j < num_atoms; j++)
              if (sqrt((atom_positions[0 + 3 * i] - atom_positions[0 + 3 * j]) *
                           (atom_positions[0 + 3 * i] - atom_positions[0 + 3 * j]) +
                       (atom_positions[1 + 3 * i] - atom_positions[1 + 3 * j]) *
                           (atom_positions[1 + 3 * i] - atom_positions[1 + 3 * j]) +
                       (atom_positions[2 + 3 * i] - atom_positions[2 + 3 * j]) *
                           (atom_positions[2 + 3 * i] - atom_positions[2 + 3 * j])) < delta)
                nbonds++;

          if (nbonds > 3 * num_atoms)
            {
              delta *= 0.75;
            }
          else
            {
              break;
            }
        }
    }

  if (radius == 0)
    {
      radius = 0.15 * scale / ((num_atoms > 3) ? log(0.5 * num_atoms) : 1);
      while (radius > 0.25 * delta)
        {
          radius *= 0.75;
        }
    }
  cyl_rad = 0.15 * radius;

  if (autoscale)
    {
      xmin = global_xmin;
      xmax = global_xmax;
      ymin = global_ymin;
      ymax = global_ymax;
      zmin = global_zmin;
      zmax = global_zmax;

      meanx = global_meanx;
      meany = global_meany;
      meanz = global_meanz;
    }

  if (size <= 0)
    {
      double rmax;
      rmax = 0;
      for (i = 0; i < 8; i++)
        if (radius * element_radii[i] > rmax) rmax = radius * element_radii[i];

      xmin -= rmax;
      xmax += rmax;
      ymin -= rmax;
      ymax += rmax;
      zmin -= rmax;
      zmax += rmax;
    }

  for (i = 0; i < num_atoms; i++)
    {
      atom_positions[0 + 3 * i] -= meanx;
      atom_positions[1 + 3 * i] -= meany;
      atom_positions[2 + 3 * i] -= meanz;
      atom_radii[i] = atom_numbers[i] > 0 ? radius * element_radii[atom_numbers[i] - 1] : 0;
    }
  if (atom_adjacency_matrix != NULL)
    {
      double del = delta * delta;
      double tol = tolerance * tolerance;

      if (tolerance > 0)
        {
          for (i = 0; i < num_atoms; i++)
            {
              for (j = i; j < num_atoms; j++)
                {
                  dt = fabs((atom_positions[0 + 3 * i] - atom_positions[0 + 3 * j]) *
                                (atom_positions[0 + 3 * i] - atom_positions[0 + 3 * j]) +
                            (atom_positions[1 + 3 * i] - atom_positions[1 + 3 * j]) *
                                (atom_positions[1 + 3 * i] - atom_positions[1 + 3 * j]) +
                            (atom_positions[2 + 3 * i] - atom_positions[2 + 3 * j]) *
                                (atom_positions[2 + 3 * i] - atom_positions[2 + 3 * j]) -
                            del);
                  atom_adjacency_matrix[i * num_atoms + j] = atom_adjacency_matrix[j * num_atoms + i] =
                      ((dt - del) < tol) || (atom_numbers2[i] < 0 && atom_numbers2[j] < 0) ? True : False;
                }
            }
        }
      else if (!chain)
        {
          for (i = 0; i < num_atoms; i++)
            {
              for (j = i; j < num_atoms; j++)
                {
                  dt = (atom_positions[0 + 3 * i] - atom_positions[0 + 3 * j]) *
                           (atom_positions[0 + 3 * i] - atom_positions[0 + 3 * j]) +
                       (atom_positions[1 + 3 * i] - atom_positions[1 + 3 * j]) *
                           (atom_positions[1 + 3 * i] - atom_positions[1 + 3 * j]) +
                       (atom_positions[2 + 3 * i] - atom_positions[2 + 3 * j]) *
                           (atom_positions[2 + 3 * i] - atom_positions[2 + 3 * j]);
                  atom_adjacency_matrix[i * num_atoms + j] = atom_adjacency_matrix[j * num_atoms + i] =
                      (dt < del) || (atom_numbers2[i] < 0 && atom_numbers2[j] < 0) ? True : False;
                }
            }
        }
      else
        {
          for (i = 0; i < num_atoms; i++)
            {
              for (j = i; j < num_atoms; j++)
                {
                  atom_adjacency_matrix[i * num_atoms + j] = atom_adjacency_matrix[j * num_atoms + i] =
                      (j == i + 1) ? True : False;
                }
            }
        }
    }
  return;
}

static void findRGB(char *newcolor, int *R, int *G, int *B)
{
  int i;

  for (i = 0; i < 753; i++)
    {
      if (!strcmp(all_color_names[i], newcolor))
        {
          *R = all_color_rgb[i][0];
          *G = all_color_rgb[i][1];
          *B = all_color_rgb[i][2];
          return;
        }
    }
  fprintf(stderr, "'%s' is not known.\n", newcolor);
  exit(0);
}

static int atomname2atomnumber(char *atom_name)
{
  int i, l;
  char str[4];

  l = strlen(atom_name);
  if (l > 4)
    {
      moldyn_error("unknown atom name");
    }

  for (i = 0; i < l; i++)
    {
      str[i] = toupper(atom_name[i]);
    }
  str[l] = '\0';

  for (i = 0; i < 118; i++)
    {
      if (!strcmp(element_names[i], str))
        {
          return i + 1;
        }
    }

  return 118; /* Ununoctium is the default element (!?) */
}

static void analyze(void)
{
  int t, tn = 0;
  float tx, ty, tz;
  char line[MAX_STRING], *tc;

  max_atoms = 0;

  fgets(line, MAX_STRING, fptr);
  if (*line != '#')
    {
      rewind(fptr);
    }

  if (fgets(line, MAX_STRING, fptr) == NULL || fgets(line, MAX_STRING, fptr) == NULL)
    {
      moldyn_error("can't read data record");
    }

  if (format == normal)
    {
      if (strtok(line, SEPARATORS) == NULL || strtok(NULL, SEPARATORS) == NULL)
        {
          moldyn_error("can't read data record");
        }

      global_xmin = global_xmax = atof(strtok(NULL, SEPARATORS));
      global_ymin = global_ymax = atof(strtok(NULL, SEPARATORS));
      global_zmin = global_zmax = -atof(strtok(NULL, SEPARATORS));
      rewind(fptr);
    }
  else
    {
      fgets(line, MAX_STRING, fptr);
      if (strtok(line, SEPARATORS) == NULL)
        {
          moldyn_error("can't read data record");
        }

      global_xmin = global_xmax = atof(strtok(NULL, SEPARATORS));
      global_ymin = global_ymax = atof(strtok(NULL, SEPARATORS));
      global_zmin = global_zmax = -atof(strtok(NULL, SEPARATORS));

      rewind(fptr);
    }

  fgets(line, MAX_STRING, fptr);

  if (*line != '#')
    {
      rewind(fptr);
    }

  if (format == normal)
    {
      fgets(line, MAX_STRING, fptr);
      while (!feof(fptr))
        {
          if (tn > max_atoms)
            {
              max_atoms = tn;
            }

          tn = 0;

          while (!feof(fptr))
            {
              if (fgets(line, MAX_STRING, fptr) != NULL)
                {
                  strtok(line, SEPARATORS);

                  tc = strtok(NULL, SEPARATORS);
                  if (tc == NULL)
                    {
                      moldyn_error("can't read cycle record");
                    }

                  if (strchr(tc, '.') != NULL)
                    {
                      break;
                    }

                  tc = strtok(NULL, SEPARATORS);
                  if (tc == NULL)
                    {
                      moldyn_error("missing data");
                    }
                  tx = atof(tc);

                  tc = strtok(NULL, SEPARATORS);
                  if (tc == NULL)
                    {
                      moldyn_error("missing data");
                    }
                  ty = atof(tc);

                  tc = strtok(NULL, SEPARATORS);
                  if (tc == NULL)
                    {
                      moldyn_error("missing data");
                    }
                  tz = -atof(tc);

                  tn++;

                  if (tx < global_xmin) global_xmin = tx;
                  if (tx > global_xmax) global_xmax = tx;
                  if (ty < global_ymin) global_ymin = ty;
                  if (ty > global_ymax) global_ymax = ty;
                  if (tz < global_zmin) global_zmin = tz;
                  if (tz > global_zmax) global_zmax = tz;
                }
              else
                break;
            }
        }
    }
  else if (format == xyz)
    {
      while (!feof(fptr))
        {
          if (fgets(line, MAX_STRING, fptr) == NULL)
            {
              break;
            }

          tc = strtok(line, SEPARATORS);

          tn = atoi(tc);
          if (tn < 1) moldyn_error("missing atom number in cycle record");

          if (tn > max_atoms)
            {
              max_atoms = tn;
            }

          fgets(line, MAX_STRING, fptr);
          for (t = 0; t < tn; t++)
            {
              fgets(line, MAX_STRING, fptr);
              strtok(line, SEPARATORS);

              tc = strtok(NULL, SEPARATORS);
              if (tc == NULL) moldyn_error("missing data");
              tx = atof(tc);

              tc = strtok(NULL, SEPARATORS);
              if (tc == NULL) moldyn_error("missing data");
              ty = atof(tc);

              tc = strtok(NULL, SEPARATORS);
              if (tc == NULL) moldyn_error("missing data");
              tz = -atof(tc);

              if (tx < global_xmin) global_xmin = tx;
              if (tx > global_xmax) global_xmax = tx;
              if (ty < global_ymin) global_ymin = ty;
              if (ty > global_ymax) global_ymax = ty;
              if (tz < global_zmin) global_zmin = tz;
              if (tz > global_zmax) global_zmax = tz;
            }
        }
    }
  else if (format == unichem)
    {
      while (!feof(fptr))
        {
          if (fgets(line, MAX_STRING, fptr) == NULL) break;
          fgets(line, MAX_STRING, fptr);

          tc = strtok(line, SEPARATORS);
          tn = atoi(tc);
          if (tn < 1) moldyn_error("missing atom number in cycle record");

          if (tn > max_atoms)
            {
              max_atoms = tn;
            }

          for (t = 0; t < tn; t++)
            {
              fgets(line, MAX_STRING, fptr);
              strtok(line, SEPARATORS);

              tc = strtok(NULL, SEPARATORS);
              if (tc == NULL) moldyn_error("missing data");
              tx = atof(tc);

              tc = strtok(NULL, SEPARATORS);
              if (tc == NULL) moldyn_error("missing data");
              ty = atof(tc);

              tc = strtok(NULL, SEPARATORS);
              if (tc == NULL) moldyn_error("missing data");
              tz = -atof(tc);

              if (tx < global_xmin) global_xmin = tx;
              if (tx > global_xmax) global_xmax = tx;
              if (ty < global_ymin) global_ymin = ty;
              if (ty > global_ymax) global_ymax = ty;
              if (tz < global_zmin) global_zmin = tz;
              if (tz > global_zmax) global_zmax = tz;
            }
        }
    }

  global_meanx = (global_xmin + global_xmax) / 2;
  global_meany = (global_ymin + global_ymax) / 2;
  global_meanz = (global_zmin + global_zmax) / 2;

  global_xmin -= global_meanx;
  global_xmax -= global_meanx;
  global_ymin -= global_meany;
  global_ymax -= global_meany;
  global_zmin -= global_meanz;
  global_zmax -= global_meanz;

  fgets(line, MAX_STRING, fptr);
  if (*line != '#')
    {
      rewind(fptr);
    }

  allocate_atom_memory();

  return;
}

int main(int argc, char **argv)
{
  ac = argc;
  avp = argv;

  reset_element_information();

  if (argc < 2)
    {
      moldyn_usage();
    }

  moldyn_init_graphics(&argc, argv, argv[1]);


  read_dat();

  if (autoscale)
    {
      analyze();
    }

  read_cycle();

  range = fabs(zmax) + fabs(zmin);
  if (range < fabs(xmax) + fabs(xmin)) range = fabs(xmax) + fabs(xmin);

  if (range < fabs(ymax) + fabs(ymin)) range = fabs(ymax) + fabs(ymin);

  magnification = pow(1.2, (double)magstep);
  zeye = -2.0 * range * magnification;

  moldyn_update_graphics();

  if (povray <= 0)
    {
      start_mainloop();
    }
  else
    {
      makePov(MOLDYN_EXPORT_TO_PNG);
    }
  return 0;
}

static void allocate_atom_memory(void)
{
  int i;

  free_atom_memory();

  if (max_atoms > 0)
    {
      atom_numbers = (int *)malloc(max_atoms * sizeof(int));
      atom_numbers2 = (int *)malloc(max_atoms * sizeof(int));
      atom_positions = (float *)malloc(max_atoms * 3 * sizeof(float));
      atom_spins = (float *)malloc(max_atoms * 3 * sizeof(float));
      atom_colors = (float *)malloc(max_atoms * 3 * sizeof(float));
      atom_radii = (float *)malloc(max_atoms * sizeof(float));
      if (max_atoms < 40000)
        atom_adjacency_matrix = (char *)malloc(max_atoms * max_atoms * sizeof(char));
      else
        atom_adjacency_matrix = NULL;

      atom_names = (char **)malloc(max_atoms * sizeof(char *));
      *atom_names = (char *)malloc(max_atoms * 4 * sizeof(char));
      for (i = 1; i < max_atoms; i++)
        {
          atom_names[i] = atom_names[i - 1] + 4;
        }

      if (atom_numbers == NULL || atom_numbers2 == NULL || atom_positions == NULL || atom_radii == NULL ||
          (atom_adjacency_matrix == NULL && num_atoms < 40000))
        {
          moldyn_error("can't allocate memory");
        }
    }
}

static void free_atom_memory(void)
{
  free(atom_numbers);
  free(atom_numbers2);
  free(atom_positions);
  free(atom_spins);
  free(atom_radii);
  free(atom_colors);
  if (atom_adjacency_matrix != NULL) free(atom_adjacency_matrix);
  if (atom_names != NULL)
    {
      free(*atom_names);
      free(atom_names);
    }
}

void moldyn_close_file(void)
{
  if (fptr != NULL)
    {
      fclose(fptr);
    }
  free_atom_memory();
  if (CP != NULL)
    {
      CP -= CP_LEN;
      free(CP);
    }
}
