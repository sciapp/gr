/* OpenGL via network might require LIBGL_ALWAYS_INDIRECT to be set */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include <jpeglib.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
/* The escape key is missing as a GLUT_KEY_x macro, but is reported via glutKeyboardFunc() as 27. */
#define GLUT_KEY_ESCAPE 27

#include "color_names.h"

#include "GLor.h"
#include "jpeg2ps/jpeg2ps.h"

#define MAX_SLICES 40
#define MAX_STACKS 20

#ifndef Bool
#define Bool char
#endif
#define False 0
#define True 1

#define Version "MolDyn - 7.0.1"
#ifndef Revision
#define Revision ""
#endif

#define MAX_CYCLES 7500
#define MAX_STRING 256
#define MAX_ARGS 32

#define SEPARATORS " \t"

#define torad(alpha) ((alpha) * (M_PI / 180.0))
#define todeg(alpha) ((alpha) * (180.0 / M_PI))

typedef enum
{
  normal,
  xyz,
  unichem
} format_t;

static int ac;     /* argument counter */
static char **avp; /* argument pointer */

static GLuint theBox, theScreen; /* GL display lists */
static GLUquadricObj *sph, *cyl;
static GLint slices, stacks;
/* GLUT fonts */
static void *font_12 = GLUT_BITMAP_HELVETICA_12;
static void *font_18 = GLUT_BITMAP_HELVETICA_18;
static float cyl_rad; /* bond cylinder radius */

static GLfloat xeye = 0.0, yeye = 0.0, zeye;   /* camera position */
static double rotation = 0;                    /* camera rotation (negative rotation around y axis)*/
static double tilt = 0;                        /* camera tilt (rotation around x axis)*/
static int moving = 0, startx = 0, starty = 0; /* values for camera movement with the mouse */
static int magstep = 0;
static double magnification = 1;

#define ROTATE 1
#define TRANSLATE 2
static int move_stat = ROTATE; /* mode of movement: Rotation/Translation */

#define SHOW_ALL 222
#define SHOW_NEXT 111
#define HOLD 333
#define SHOW_PREV 444
static int show_stat = HOLD;

static Bool file_done = False; /* file was read until the end? */
static int last_init_done = False;

static Bool jpeg = False; /* create a jpeg file? */

static FILE *fptr = NULL;
static int icycle = 0, current_cycle = 0;
static fpos_t cycle[MAX_CYCLES];
static double energy0 = 0, energy = 0; /* energy levels (?)*/
static int step = 10;                  /* number of cycles skipped when reading cycles */

static GLfloat c_white[] = {1.0, 1.0, 1.0};
static GLfloat c_black[] = {0.0, 0.0, 0.0};

static int resolution = 555; /* resolution for POV-Ray output */

/* current window size */
static int window_width = 555;
static int window_height = 555;

static Bool hint = False;      /* show help text? */
static Bool autoscale = False; /* read the whole file for scaling information of ALL frames instead of the first? */

static GLfloat range; /* range of coordinate values in all three dimensions*/
static double dist;
static double factor; /* used for macro p_xform */
static double sscale; /* 1/scale */
#define S(a) ((a)*sscale)

#define p_xform(x, y, z, r, dx, dy, dz, dr) \
  factor = 1 - z / (1 + dist + z);          \
  dx = x * factor;                          \
  dy = y * factor;                          \
  dz = z * factor;                          \
  dr = r * factor

static int max_atoms = 40;
static double linewidth = 1;

static char *program_name;
static int povray = 0, pix = 0;
static char name[80], path[90];
static format_t format;
static char title[MAX_STRING];

static Bool numbers = True; /* show atom numbers? */
static Bool bonds = True;   /* show atom bonds? */
static Bool chain = False;  /* form atom bonds as a chain? */
static Bool colors = True;
static Bool box = False; /* show bounding box? */
static double size = 0;  /* bounding box size */
static double radius = 0;

static double *atom_positions = NULL;
static double *atom_radii = NULL;
static int *atom_numbers = NULL;
static int *atom_numbers2 = NULL;
static char *atom_adjacency_matrix = NULL;
static char **atom_names = NULL; /* An atom name is three or less characters long. */

static char *CP = NULL; /* misc. string (?) */
static int CP_LEN;

static double delta = 0;
static double tolerance = 0;
static int num_atoms = 0;
static double xmin = HUGE_VAL, ymin = HUGE_VAL, zmin = HUGE_VAL;
static double xmax = -HUGE_VAL, ymax = -HUGE_VAL, zmax = -HUGE_VAL;
static double meanx, meany, meanz;

static double global_xmin, global_xmax, global_ymin, global_ymax, global_zmin, global_zmax;
static double global_meanx, global_meany, global_meanz;

static const char *help_message[] = {"where options include:                      " Version " " Revision "",
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

static void drawStr(float x, float y, const char *str, void *fnt);
static void drawStr2(float x, float y, float z, const char *str, void *fnt);
static void beginText(void);
static void endText(void);
static void init_List(void);
static void animate(void);
static void writeJpeg(char *filename, int jpeg_quality);
static void makeJpeg(void);
static void createPixmap(int width, int height);
static void makePov(void);
static void writePov(FILE *pf);

static void findRGB(char *newcolor, int *R, int *G, int *B);
static int atomname2atomnumber(char *atom_name);

static void read_dat();
static void read_check(FILE *fptr, int *n, char **argv);
static void read_cycle(void);
static void analyze(void);

static void usage();
static void allocate_memory(void);
static void s_error(const char *problem_string);
static void moldyn_exit(int error_code);

static void init()
{
  GLfloat light_position[] = {0.3, 0.3, 1.0, 0.0};

  glClearColor(1.0, 1.0, 1.0, 0.0);
  glClearDepth(1.0);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);
  glColorMaterial(GL_FRONT, GL_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
}

static void init_List(void)
{
  float cyl_len, ang;
  float vx, vy, vz;
  double rx, ry;
  int i, j, k;

  /* init list */

  glDeleteLists(theBox, 1);
  glDeleteLists(theScreen, 1);

  if (file_done) last_init_done = True;

  if (num_atoms > 300)
    {
      slices = 0.3 * MAX_SLICES;
      stacks = 0.3 * MAX_STACKS;
    }
  else
    {
      slices = 0.65 * MAX_SLICES;
      stacks = 0.65 * MAX_STACKS;
    }

  theBox = glGenLists(1);
  glNewList(theBox, GL_COMPILE);
  glLineWidth(1.0);
  glColor3f(0.0, 0.0, 0.0);

  glBegin(GL_LINE_LOOP);
  glVertex3f(xmin, ymax, zmax);
  glVertex3f(xmax, ymax, zmax);
  glVertex3f(xmax, ymin, zmax);
  glVertex3f(xmin, ymin, zmax);
  glVertex3f(xmin, ymin, zmin);
  glVertex3f(xmax, ymin, zmin);
  glVertex3f(xmax, ymax, zmin);
  glVertex3f(xmin, ymax, zmin);
  glEnd();

  glBegin(GL_LINES);
  glVertex3f(xmin, ymax, zmax);
  glVertex3f(xmin, ymin, zmax);
  glVertex3f(xmax, ymin, zmax);
  glVertex3f(xmax, ymin, zmin);
  glVertex3f(xmin, ymin, zmin);
  glVertex3f(xmin, ymax, zmin);
  glVertex3f(xmax, ymax, zmin);
  glVertex3f(xmax, ymax, zmax);
  glEnd();

  glEndList();

  theScreen = glGenLists(1);
  glNewList(theScreen, GL_COMPILE);

  if (!colors) glColor3fv(c_white);

  for (i = 0; i < num_atoms; i++)
    {
      if (atom_numbers[i] != 0)
        {
          sph = gluNewQuadric();
          gluQuadricNormals(sph, GLU_SMOOTH);

          glPushMatrix();
          glTranslatef(atom_positions[0 + 3 * i], atom_positions[1 + 3 * i], atom_positions[2 + 3 * i]);
          /* printf("%g %g %g\n", atom_positions[0+3*i], atom_positions[1+3*i], atom_positions[2+3*i]); */

          if (colors) glColor3ubv(f_ptable[atom_numbers[i] - 1]);

          gluSphere(sph, atom_radii[i], slices, stacks);

          glPopMatrix();

          gluDeleteQuadric(sph);
        }
      else
        continue;

      if (!bonds) continue;

      for (j = i + 1; j < num_atoms; j++)
        {
          if (atom_numbers[j] && atom_adjacency_matrix[i * num_atoms + j])
            {
              if (atom_positions[2 + 3 * j] > atom_positions[2 + 3 * i])
                {
                  vx = atom_positions[0 + 3 * j] - atom_positions[0 + 3 * i];
                  vy = atom_positions[1 + 3 * j] - atom_positions[1 + 3 * i];
                  vz = atom_positions[2 + 3 * j] - atom_positions[2 + 3 * i];
                  k = i;
                }
              else
                {
                  vx = atom_positions[0 + 3 * i] - atom_positions[0 + 3 * j];
                  vy = atom_positions[1 + 3 * i] - atom_positions[1 + 3 * j];
                  vz = atom_positions[2 + 3 * i] - atom_positions[2 + 3 * j];
                  k = j;
                }

              cyl_len = sqrt(vx * vx + vy * vy + vz * vz);

              if (fabs(vz) > 1e-3)
                {
                  ang = todeg(acos(vz / cyl_len));
                }
              else
                {
                  ang = 90;
                }

              rx = -vy * vz;
              ry = vx * vz;

              cyl = gluNewQuadric();
              gluQuadricNormals(cyl, GLU_SMOOTH);

              glPushMatrix();
              glTranslatef(atom_positions[0 + 3 * k], atom_positions[1 + 3 * k], atom_positions[2 + 3 * k]);
              if (fabs(vz) > 1e-3)
                {
                  glRotatef(ang, rx, ry, 0.0);
                }
              else
                {
                  glRotatef(ang, -vy, vx, 0.0);
                }

              glColor3fv(c_white);

              gluCylinder(cyl, cyl_rad, cyl_rad, cyl_len, stacks, 1);
              glPopMatrix();

              gluDeleteQuadric(cyl);
            }
        }
    }
  glEndList();
}

static void reshape(int w, int h)
{

  w = w <= 0 ? 4 : (w + 3) & ~0x03;
  h = h <= 0 ? 4 : (h + 3) & ~0x03;

  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(45.0f, (GLfloat)w / (GLfloat)h, 0.5, 7.0 * range);

  glMatrixMode(GL_MODELVIEW);

  window_width = w;
  window_height = h;
}

static void DrawScene(void)
{
  char text[MAX_STRING];
  char atom_name[8];
  int i;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  glColor3fv(c_black);
  glDisable(GL_LIGHTING);

  beginText();
  if (hint)
    {
      drawStr(0.02, 0.95, "<ESC> Exit", font_12);
      drawStr(0.02, 0.92, "<0> Reset view", font_12);

      drawStr(0.02, 0.35, "<a>rt", font_12);
      drawStr(0.02, 0.32, "<b>ack", font_12);
      drawStr(0.02, 0.29, "<c>apture", font_12);
      drawStr(0.02, 0.26, "<h>old", font_12);
      drawStr(0.02, 0.23, "<m>agnify", font_12);
      drawStr(0.02, 0.20, "<n>ext", font_12);
      drawStr(0.02, 0.17, "<p>rint", font_12);
      drawStr(0.02, 0.14, "<r>educe", font_12);
      drawStr(0.02, 0.11, "<s>witch rotate/translate", font_12);
      drawStr(0.02, 0.08, "<t>ext on/off", font_12);
      drawStr(0.02, 0.05, "<?> hints on/off", font_12);
    }

  if (format == normal && numbers)
    {
      sprintf(text, "#%d, E=%f", icycle, energy0 - energy);
      drawStr(0.65, 0.95, text, font_18);
    }
  else if (numbers)
    {
      drawStr(1 - strlen(title) / 50.0, 0.95, title, font_18);
    }
  endText();

  glTranslatef(xeye, yeye, zeye);

  glRotatef(-rotation, 0.0, 1.0, 0.0);
  glRotatef(tilt, 1.0, 0.0, 0.0);

  if (numbers)
    {
      for (i = 0; i < num_atoms; i++)
        {
          if (atom_numbers[i] != 0)
            {
              if (format == normal)
                {
                  sprintf(atom_name, "%d.%d", atom_numbers[i], atom_numbers2[i]);
                }
              else if (format == xyz)
                {
                  sprintf(atom_name, "%s", atom_names[i]);
                }
              else
                {
                  sprintf(atom_name, "%d", atom_numbers[i]);
                }

              glPushMatrix();
              glTranslatef(atom_positions[0 + 3 * i], atom_positions[1 + 3 * i], atom_positions[2 + 3 * i]);
              glRotatef(-tilt, 1.0, 0.0, 0.0);
              glRotatef(rotation, 0.0, 1.0, 0.0);

              drawStr2(0.0, 0.0, atom_radii[i], atom_name, font_12);
              glPopMatrix();
            }
        }
    }

  glEnable(GL_LIGHTING);

  if (show_stat == HOLD)
    {
      glutIdleFunc(NULL);
    }
  else if (show_stat == SHOW_NEXT || show_stat == SHOW_PREV)
    {
      show_stat = HOLD;
    }
  else if (show_stat == SHOW_ALL)
    {
      return;
    }

  if (box)
    {
      glDisable(GL_LIGHTING);
      glCallList(theBox);
      glEnable(GL_LIGHTING);
    }

  glCallList(theScreen);

  glutSwapBuffers();
}

static void animate(void)
{
  float cyl_len, ang;
  float vx, vy, vz;
  double rx, ry;
  int i, j, k;
  char string[MAX_STRING];

  if (file_done && jpeg)
    {
      glutIdleFunc(NULL);
      show_stat = HOLD;
      jpeg = False;

      init_List();

      glutPostRedisplay();

      sprintf(string, "cat %s????.jpg | ffmpeg -y -v 0 -r 30 -f image2pipe -vcodec mjpeg -i - -vcodec mpeg4 %s.mov",
              name, name);
      system(string);
      return;
    }

  /* Redraw for animation (no display lists) */
  read_cycle();

  if (box)
    {
      glLineWidth(1.0);
      glColor3f(0.0, 0.0, 0.0);

      glBegin(GL_LINE_LOOP);
      glVertex3f(xmin, ymax, zmax);
      glVertex3f(xmax, ymax, zmax);
      glVertex3f(xmax, ymin, zmax);
      glVertex3f(xmin, ymin, zmax);
      glVertex3f(xmin, ymin, zmin);
      glVertex3f(xmax, ymin, zmin);
      glVertex3f(xmax, ymax, zmin);
      glVertex3f(xmin, ymax, zmin);
      glEnd();

      glBegin(GL_LINES);
      glVertex3f(xmin, ymax, zmax);
      glVertex3f(xmin, ymin, zmax);
      glVertex3f(xmax, ymin, zmax);
      glVertex3f(xmax, ymin, zmin);
      glVertex3f(xmin, ymin, zmin);
      glVertex3f(xmin, ymax, zmin);
      glVertex3f(xmax, ymax, zmin);
      glVertex3f(xmax, ymax, zmax);
      glEnd();
    }

  if (!colors) glColor3fv(c_white);

  for (i = 0; i < num_atoms; i++)
    {
      if (atom_numbers[i] != 0)
        {
          sph = gluNewQuadric();
          gluQuadricNormals(sph, GLU_SMOOTH);

          glPushMatrix();
          glTranslatef(atom_positions[0 + 3 * i], atom_positions[1 + 3 * i], atom_positions[2 + 3 * i]);

          if (colors) glColor3ubv(f_ptable[atom_numbers[i] - 1]);

          gluSphere(sph, atom_radii[i], slices, stacks);
          glPopMatrix();

          gluDeleteQuadric(sph);
        }
      else
        continue;

      if (!bonds) continue;

      for (j = i + 1; j < num_atoms; j++)
        {
          if (atom_numbers[j] && atom_adjacency_matrix[i * num_atoms + j])
            {
              if (atom_positions[2 + 3 * j] > atom_positions[2 + 3 * i])
                {
                  vx = atom_positions[0 + 3 * j] - atom_positions[0 + 3 * i];
                  vy = atom_positions[1 + 3 * j] - atom_positions[1 + 3 * i];
                  vz = atom_positions[2 + 3 * j] - atom_positions[2 + 3 * i];
                  k = i;
                }
              else
                {
                  vx = atom_positions[0 + 3 * i] - atom_positions[0 + 3 * j];
                  vy = atom_positions[1 + 3 * i] - atom_positions[1 + 3 * j];
                  vz = atom_positions[2 + 3 * i] - atom_positions[2 + 3 * j];
                  k = j;
                }
              cyl_len = sqrt(vx * vx + vy * vy + vz * vz);

              if (fabs(vz) > 1e-3)
                ang = todeg(acos(vz / cyl_len));
              else
                ang = 90;

              rx = -vy * vz;
              ry = vx * vz;

              cyl = gluNewQuadric();
              gluQuadricNormals(cyl, GLU_SMOOTH);

              glPushMatrix();
              glTranslatef(atom_positions[0 + 3 * k], atom_positions[1 + 3 * k], atom_positions[2 + 3 * k]);
              if (fabs(vz) > 1e-3)
                glRotatef(ang, rx, ry, 0.0);
              else
                glRotatef(ang, -vy, vx, 0.0);

              glColor3fv(c_white);

              gluCylinder(cyl, cyl_rad, cyl_rad, cyl_len, stacks, 1);
              glPopMatrix();

              gluDeleteQuadric(cyl);
            }
        }
    }

  glutSwapBuffers();
  glutPostRedisplay();

  if (jpeg)
    {
      makeJpeg();
    }
}

static void motion(int x, int y)
{
  if (moving)
    {
      if (rotation > 360 || rotation < -360)
        {
          rotation = 0;
        }
      else
        {
          rotation = rotation - (x - startx);
        }
      if (tilt > 360 || tilt < -360)
        {
          tilt = 0;
        }
      else
        {
          tilt = tilt + (y - starty);
        }
      startx = x;
      starty = y;
      glutPostRedisplay();
    }
}

static void moldyn_exit(int error_code)
{
  if (fptr != NULL)
    {
      fclose(fptr);
    }
  free(atom_numbers);
  free(atom_numbers2);
  free(atom_positions);
  free(atom_radii);
  free(atom_adjacency_matrix);
  if (atom_names != NULL)
    {
      free(*atom_names);
      free(atom_names);
    }
  if (CP != NULL)
    {
      CP -= CP_LEN;
      free(CP);
    }
  exit(error_code);
}


static void mouse(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
      moving = 1;
      startx = x;
      starty = y;
    }
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    {
      moving = 0;
    }
}

static void keyboard(unsigned char key, int x, int y)
{
  char ename[256];

  switch (key)
    {
    case 'a':
      povray = pix = -9999;
      makePov();
      break;

    case 'r':
      if (magstep <= 10)
        {
          magstep++;
          magnification = pow(1.2, (double)magstep);
          zeye = -2 * range * magnification;
          glutPostRedisplay();
        }
      break;

    case 'm':
      if (magstep >= -10)
        {
          magstep--;
          magnification = pow(1.2, (double)magstep);
          zeye = -2 * range * magnification;
          glutPostRedisplay();
        }
      break;

    case 's':
      if (move_stat == ROTATE)
        {
          move_stat = TRANSLATE;
        }
      else if (move_stat == TRANSLATE)
        {
          move_stat = ROTATE;
        }
      break;

    case 'n':
      if (file_done && last_init_done)
        {
          break;
        }
      else
        {
          show_stat = SHOW_NEXT;
          read_cycle();
          init_List();
          glutPostRedisplay();
          break;
        }

    case 'b':
      if (current_cycle == 1)
        {
          break;
        }
      show_stat = SHOW_PREV;
      current_cycle -= 2;
      file_done = 0;
      if (feof(fptr)) rewind(fptr);

      if (fsetpos(fptr, &cycle[current_cycle])) s_error("can't position file");

      read_cycle();
      init_List();
      glutPostRedisplay();
      break;

    case 'h':
    case 'j':
      if (show_stat == HOLD && !file_done)
        {
          if (key == 'j')
            {
              jpeg = True;
              makeJpeg();
            }
          glDeleteLists(theBox, 1);
          glDeleteLists(theScreen, 1);

          glutIdleFunc(animate);
          show_stat = SHOW_ALL;
        }
      else if (show_stat == SHOW_ALL)
        {
          jpeg = False;
          show_stat = HOLD;
          init_List();
          glutPostRedisplay();
        }
      break;

    case 'c':
    case 'C':
      createPixmap(800, 800);
      break;

    case 'J':
      sprintf(ename, "%s.jpg", name);
      writeJpeg(ename, 100);
      break;

    case 'p':
      createPixmap(2000, 2000);
      {
        char *cp;
        sprintf(path, "%s%4d.jpg", name, current_cycle);
        for (cp = path; *cp; cp++)
          if (*cp == ' ') *cp = '0';
      }
      {
        char *argv[] = {"jpeg2ps", "-o", "moldyn.eps", path};
        jpeg2ps_main(sizeof(argv) / sizeof(char *), argv);
      }
#ifdef _WIN32
      system("copy moldyn.eps %PRINTER%");
#else
      system("lpr -h moldyn.eps");
#endif
      break;

    case '0':
      xeye = 0.0;
      yeye = 0.0;
      zeye = -2.0 * range;
      glutPostRedisplay();
      break;

    case '?':
      hint = (hint) ? False : True;
      glutPostRedisplay();
      break;

    case 'T':
    case 't':
      numbers = (numbers) ? False : True;
      glutPostRedisplay();
      break;

    case 'q':
    case 'Q':
    case GLUT_KEY_ESCAPE:
      moldyn_exit(0);
      break;
    }
}

static void specialKey(int key, int x, int y)
{
  switch (key)
    {
    case GLUT_KEY_PAGE_UP:
      if (zeye > range / 2) break;
      zeye += 0.4f;
      if (magstep > -4 && zeye > -2 * range * magnification)
        {
          magstep--;
          magnification = pow(1.2, (double)magstep);
        }
      glutPostRedisplay();
      break;

    case GLUT_KEY_PAGE_DOWN:
      if (zeye < -2 * range) break;
      zeye -= 0.4f;
      if (magstep < 4 && zeye < -2 * range * magnification)
        {
          magstep++;
          magnification = pow(1.2, (double)magstep);
        }
      glutPostRedisplay();
      break;

    case GLUT_KEY_UP:
      if (move_stat == ROTATE)
        {
          if (tilt < 360 && tilt > -360)
            tilt -= 5.0;
          else
            tilt = 0.0;
        }
      else if (move_stat == TRANSLATE)
        {
          yeye += 0.2;
        }
      glutPostRedisplay();
      break;

    case GLUT_KEY_DOWN:
      if (move_stat == ROTATE)
        {
          if (tilt < 360 && tilt > -360)
            tilt += 5.0;
          else
            tilt = 0.0;
        }
      else if (move_stat == TRANSLATE)
        {
          yeye -= 0.2;
        }

      glutPostRedisplay();
      break;

    case GLUT_KEY_LEFT:
      if (move_stat == ROTATE)
        {
          if (rotation < 360 && rotation > -360)
            {
              rotation += 5.0;
            }
          else
            {
              rotation = 0.0;
            }
        }
      else if (move_stat == TRANSLATE)
        {
          xeye -= 0.2;
        }
      glutPostRedisplay();
      break;

    case GLUT_KEY_RIGHT:
      if (move_stat == ROTATE)
        {
          if (rotation < 360 && rotation > -360)
            {
              rotation -= 5.0;
            }
          else
            {
              rotation = 0.0;
            }
        }
      else if (move_stat == TRANSLATE)
        {
          xeye += 0.2;
        }
      glutPostRedisplay();
      break;

    default:
      break;
    }
}

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
    s_error("unknown format");

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
    s_error("can't obtain number of atoms");

  return;
}

static void usage(void)
{
  char **cpp;
  fprintf(stderr, "usage:  %s file [-options]\n", program_name);

  for (cpp = (char **)help_message; *cpp; cpp++)
    {
      fprintf(stderr, "%s\n", *cpp);
    }

  moldyn_exit(0);
}

static void s_error(const char *problem_string)
{
  fprintf(stderr, "%s: %s\n", program_name, problem_string);
  moldyn_exit(1);
}

static void read_dat(void)
{

  char **arg, *arg_vector[MAX_ARGS], *option;
  static int pass = 0;

  char *fn = NULL;
  int i;

  program_name = arg_vector[0] = avp[0];
  if (ac <= 1) usage();

  fn = *++avp;

  if (*fn == '-') usage();

  fptr = fopen(fn, "r");
  if (fptr == NULL) s_error("can't open file");

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
              usage();
            }
          if (!strcmp(option, "-atoms"))
            {
              num_atoms = atoi(*arg);
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
                  usage();
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
                  usage();
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
                  usage();
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
                  radii[ord - 1] = fabs(atof(*arg));
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
                  usage();
                }
            }
          else if (!strncmp(option, "-color", 6))
            {
              int R, G, B, ord;
              ord = atoi(&option[6]);
              findRGB(*arg, &R, &G, &B);
              f_ptable[ord - 1][0] = R;
              f_ptable[ord - 1][1] = G;
              f_ptable[ord - 1][2] = B;
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
                  usage();
                }
            }
          else
            {
              usage();
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

      allocate_memory();
    }

  pix = 0;
}

static void beginText()
{
  glClear(GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0.0, 1.0, 0.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
}

static void drawStr(float x, float y, const char *str, void *fnt)
{
  char *c;
  glRasterPos2f(x, y);

  for (c = (char *)str; *c != '\0'; c++)
    {
      glutBitmapCharacter(fnt, *c);
    }
}

static void drawStr2(float x, float y, float z, const char *str, void *fnt)
{
  char *c;
  glRasterPos3f(x, y, z);

  for (c = (char *)str; *c != '\0'; c++)
    {
      glutBitmapCharacter(fnt, *c);
    }
}

static void endText()
{
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

static void read_cycle()
{
  static int dim;
  static double scale;
  static Bool init = False;
  double dummy1, dummy2;
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
      fgetpos(fptr, &cycle[current_cycle++]);
    }
  else
    {
      s_error("too many cycles");
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
              s_error("can't read cycle record");
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
              allocate_memory();
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

              s_error("missing data record");
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

              num_atoms++;
            }
          else if (format == xyz)
            {
              cp = line;
              while (isspace(*cp))
                {
                  cp++;
                }
              if (sscanf(cp, "%3s %lg %lg %lg", s, &atom_positions[0 + 3 * i], &atom_positions[1 + 3 * i],
                         &atom_positions[2 + 3 * i]) != 4)
                {
                  s_error("can't read data record");
                }
              c = s[0];
              atom_numbers[i] = atomname2atomnumber(s);
              atom_numbers2[i] = 1;

              strcpy(atom_names[i], s);
            }
          else
            {
              cp = line;
              while (isspace(*cp))
                {
                  cp++;
                }
              if (sscanf(cp, "%d %lg %lg %lg", &atom_numbers[i], &atom_positions[0 + 3 * i], &atom_positions[1 + 3 * i],
                         &atom_positions[2 + 3 * i]) != 4)
                {
                  s_error("can't read data record");
                }
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
        if (radius * radii[i] > rmax) rmax = radius * radii[i];

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
      atom_radii[i] = atom_numbers[i] > 0 ? radius * radii[atom_numbers[i] - 1] : 0;
    }
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

static void writeJpegForPixels(char *filename, int jpeg_quality, unsigned char *pixels, int width, int height)
{
  int i;
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE *outfile;           /* target file */
  JSAMPROW row_pointer[1]; /* pointer to JSAMPLE row[s] */
  int row_stride;          /* physical row width in image buffer */

  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);
  if ((outfile = fopen(filename, "wb")) == NULL)
    {
      printf("can't write screenshot\n");
      exit(-1);
    }
  jpeg_stdio_dest(&cinfo, outfile);
  cinfo.image_width = width; /* image width and height, in pixels */
  cinfo.image_height = height;
  cinfo.input_components = 3;     /* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; /* colorspace of input image */
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, jpeg_quality, TRUE /* limit to baseline-JPEG values */);
  jpeg_start_compress(&cinfo, TRUE);

  row_stride = width * 3;
  i = cinfo.image_height - 1;
  while (i >= 0)
    {
      /* jpeg_write_scanlines expects an array of pointers to scanlines.
       * Here the array is only one element long, but you could pass
       * more than one scanline at a time if that's more convenient.
       */
      row_pointer[0] = &pixels[i * row_stride];
      (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
      i--;
    }
  jpeg_finish_compress(&cinfo);

  fclose(outfile);
  jpeg_destroy_compress(&cinfo);
}

static void writeJpeg(char *filename, int jpeg_quality)
{
  int width, height;
  unsigned char *pixels;
  width = window_width;
  height = window_height;

  pixels = (unsigned char *)malloc(height * width * 4);
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
  writeJpegForPixels(filename, jpeg_quality, pixels, width, height);
  free(pixels);
}

static void makeJpeg(void)
{
  char *cp;

  sprintf(path, "%s%4d.jpg", name, current_cycle);
  for (cp = path; *cp; cp++)
    if (*cp == ' ') *cp = '0';
  writeJpeg(path, 100);

  puts(path);
}

static void makePov(void)
{
  static FILE *pov_file;

  char *cp;
  char string[MAX_STRING];
  int xres, yres;

  xres = resolution * 1.25 + 0.5;
  yres = 0.8 * xres + 0.5;

  while (TRUE)
    {
      if (1.0 * xres / yres > 1.25)
        yres++;
      else if (1.0 * xres / yres < 1.25)
        xres++;
      else
        break;
    }

  if (povray == -9999)
    {
      sprintf(path, "%s.pov", name);

      pov_file = fopen(path, "w");
      writePov(pov_file);
      fclose(pov_file);

      sprintf(string, "povray %s.pov +W%d +H%d +A", path, xres, yres);
      povray = 0;

#if !defined(VMS) && !defined(_WIN32)
      if (pix != current_cycle)
        {
          if (povray != -9999)
            {
              if (povray < 0)
                {
                  printf("%s.pov\n", path);
                }
              else
                {
                  puts(path);
                }
              if (povray >= 0)
                {
                  system(string);
                }
            }
          pix = 0;
        }
#endif
    }

  else if (povray > 0)
    {
      rewind(fptr);
      while (True)
        {
          sprintf(path, "%s%4d.pov", name, current_cycle);
          for (cp = path; *cp; cp++)
            if (*cp == ' ') *cp = '0';
          pov_file = fopen(path, "w");
          read_cycle();
          if (file_done) break;
          writePov(pov_file);
          fclose(pov_file);

          sprintf(string, "povray %s.pov +W%d +H%d +A -D0 >/dev/null 2>&1", path, xres, yres);

#if !defined(VMS) && !defined(_WIN32)
          if (pix != current_cycle)
            {
              if (povray != -9999)
                {
                  if (povray < 0)
                    {
                      printf("%s.pov\n", path);
                    }
                  else
                    {
                      puts(path);
                    }
                  if (povray >= 0)
                    {
                      system(string);
                    }
                }
              pix = 0;
            }
#endif
        }
    }
}

static void writePov(FILE *pf)
{
  int i, j;

  int n_pov;

  double xn0, yn0, zn0, rn0;
  double xn1, yn1, zn1, rn1;
  double rin;
  double *x = NULL, *y = NULL, *z = NULL;
  double xbox[8], ybox[8], zbox[8];

  static int bmask[] = {26, 37, 74, 133, 161, 82, 164, 88};
  double cosr, sinr, cost, sint;
  double xt, yt, zt;

  x = (double *)malloc(num_atoms * sizeof(double));
  y = (double *)malloc(num_atoms * sizeof(double));
  z = (double *)malloc(num_atoms * sizeof(double));

  xbox[0] = xmin;
  xbox[1] = xmax;
  xbox[2] = xmax;
  xbox[3] = xmin;
  ybox[0] = ymin;
  ybox[1] = ymin;
  ybox[2] = ymax;
  ybox[3] = ymax;
  zbox[0] = zmin;
  zbox[1] = zmin;
  zbox[2] = zmin;
  zbox[3] = zmin;

  xbox[4] = xmin;
  xbox[5] = xmax;
  xbox[6] = xmax;
  xbox[7] = xmin;
  ybox[4] = ymin;
  ybox[5] = ymin;
  ybox[6] = ymax;
  ybox[7] = ymax;
  zbox[4] = zmax;
  zbox[5] = zmax;
  zbox[6] = zmax;
  zbox[7] = zmax;

  fprintf(pf, "\
            background { color <1, 1, 1> }\n\
            \n\
            camera {\n\
            perspective\n\
            angle 56.25\n\
            location <%g, %g, %g>\n\
            look_at <%g, %g, %g>\n\
            }\n\
            \n\
            light_source {\n\
            <%g, %g, %g>\n\
            color rgb <1, 1, 1>\n\
            }\n\
            light_source {\n\
            <%g, %g, %g>\n\
            color rgb <1, 1, 1>\n\
            }\n",
          -xeye, -yeye, zeye, -xeye, -yeye, range / 2, -range * 5, -range * 5, -range * 5, range * 5, range * 5,
          -range * 5);

  strtok(path, ".");

  cosr = cos(torad(rotation));
  sinr = sin(torad(rotation));

  cost = cos(torad(tilt));
  sint = sin(torad(tilt));

  n_pov = num_atoms;

  if (box)
    {
      j = 1;
      n_pov = num_atoms + 8;
      for (i = num_atoms; i < n_pov; i++)
        {
          atom_numbers[i] = -j;
          atom_numbers2[i] = -bmask[i - num_atoms];
          atom_positions[0 + 3 * i] = xbox[i - num_atoms];
          atom_positions[1 + 3 * i] = ybox[i - num_atoms];
          atom_positions[2 + 3 * i] = zbox[i - num_atoms];
          j *= 2;
        }
    }

  for (i = 0; i < n_pov; i++)
    {
      yt = atom_positions[1 + 3 * i];
      zt = -atom_positions[2 + 3 * i];
      y[i] = yt * cost + zt * sint;
      z[i] = -yt * sint + zt * cost;

      xt = atom_positions[0 + 3 * i];
      zt = z[i];

      x[i] = xt * cosr + zt * sinr;
      z[i] = -xt * sinr + zt * cosr;
    }

  for (i = num_atoms; i < n_pov; i++)
    {
      for (j = i + 1; j < n_pov; j++)
        {
          if ((xbox[i - num_atoms] == xbox[j - num_atoms] && ybox[i - num_atoms] == ybox[j - num_atoms]) ||
              (xbox[i - num_atoms] == xbox[j - num_atoms] && zbox[i - num_atoms] == zbox[j - num_atoms]) ||
              (ybox[i - num_atoms] == ybox[j - num_atoms] && zbox[i - num_atoms] == zbox[j - num_atoms]))
            {
              rin = 0.03 * radius;

              p_xform(x[i], y[i], z[i], rin, xn0, yn0, zn0, rn0);
              p_xform(x[j], y[j], z[j], rin, xn1, yn1, zn1, rn1);

              fprintf(pf, "\n// Bbox\n\
                        cylinder {\n\
                        <%g, %g, %g>,\n\
                        <%g, %g, %g>,\n\
                        %g\n\
                        pigment { color rgb <0, 0, 0> } finish { ambient 0.3 phong 1.0 }\n\
                        }\n",
                      x[i], y[i], z[i], x[j], y[j], z[j], rin);
            }
        }
    }

  for (i = 0; i < num_atoms; i++)
    {
      if (atom_numbers[i] != 0)
        {
          p_xform(x[i], y[i], z[i], atom_radii[i], xn0, yn0, zn0, rn0);

          fprintf(pf, "\n// %d.%d\n\
                    sphere {\n\
                    <%g, %g, %g>, %g\n\
                    pigment { color rgb <%f, %f, %f> } finish { ambient 0.3 phong 1.0 }\n\
                    }\n",
                  atom_numbers[i], atom_numbers2[i], x[i], y[i], z[i], atom_radii[i],
                  (f_ptable[atom_numbers[i] - 1][0]) / 255.0, (f_ptable[atom_numbers[i] - 1][1]) / 255.0,
                  (f_ptable[atom_numbers[i] - 1][2]) / 255.0);
        }
      else
        continue;

      if (!bonds) continue;

      for (j = i + 1; j < num_atoms; j++)
        {
          if (atom_numbers[j] && atom_adjacency_matrix[i * num_atoms + j])
            {
              rin = 0.15 * radius;

              p_xform(x[j], y[j], z[j], rin, xn1, yn1, zn1, rn1);

              fprintf(pf, "\n// %d.%d - %d.%d\n\
                        cylinder {\n\
                        <%g, %g, %g>,\n\
                        <%g, %g, %g>,\n\
                        %g\n\
                        pigment { color rgb <0.5, 0.5, 0.5> } finish { ambient 0.3 phong 1.0 }\n\
                        }\n",
                      atom_numbers[j], atom_numbers2[j], atom_numbers[i], atom_numbers2[i], x[i], y[i], z[i], x[j],
                      y[j], z[j], rin);
            }
        }
    }

  free(x);
  free(y);
  free(z);
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
      s_error("unknown atom name");
    }

  for (i = 0; i < l; i++)
    {
      str[i] = toupper(atom_name[i]);
    }
  str[l] = '\0';

  for (i = 0; i < 118; i++)
    {
      if (!strcmp(ptable[i], str))
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
      s_error("can't read data record");
    }

  if (format == normal)
    {
      if (strtok(line, SEPARATORS) == NULL || strtok(NULL, SEPARATORS) == NULL)
        {
          s_error("can't read data record");
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
          s_error("can't read data record");
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
                      s_error("can't read cycle record");
                    }

                  if (strchr(tc, '.') != NULL)
                    {
                      break;
                    }

                  tc = strtok(NULL, SEPARATORS);
                  if (tc == NULL)
                    {
                      s_error("missing data");
                    }
                  tx = atof(tc);

                  tc = strtok(NULL, SEPARATORS);
                  if (tc == NULL)
                    {
                      s_error("missing data");
                    }
                  ty = atof(tc);

                  tc = strtok(NULL, SEPARATORS);
                  if (tc == NULL)
                    {
                      s_error("missing data");
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
          if (tn < 1) s_error("missing atom number in cycle record");

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
              if (tc == NULL) s_error("missing data");
              tx = atof(tc);

              tc = strtok(NULL, SEPARATORS);
              if (tc == NULL) s_error("missing data");
              ty = atof(tc);

              tc = strtok(NULL, SEPARATORS);
              if (tc == NULL) s_error("missing data");
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
          if (tn < 1) s_error("missing atom number in cycle record");

          if (tn > max_atoms)
            {
              max_atoms = tn;
            }

          for (t = 0; t < tn; t++)
            {
              fgets(line, MAX_STRING, fptr);
              strtok(line, SEPARATORS);

              tc = strtok(NULL, SEPARATORS);
              if (tc == NULL) s_error("missing data");
              tx = atof(tc);

              tc = strtok(NULL, SEPARATORS);
              if (tc == NULL) s_error("missing data");
              ty = atof(tc);

              tc = strtok(NULL, SEPARATORS);
              if (tc == NULL) s_error("missing data");
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

  allocate_memory();

  return;
}

static void allocate_memory(void)
{
  int i;

  free(atom_numbers);
  free(atom_numbers2);
  free(atom_positions);
  free(atom_radii);
  free(atom_adjacency_matrix);
  if (atom_names != NULL)
    {
      free(*atom_names);
      free(atom_names);
    }

  if (max_atoms > 0)
    {
      atom_numbers = (int *)malloc(max_atoms * sizeof(int));
      atom_numbers2 = (int *)malloc(max_atoms * sizeof(int));
      atom_positions = (double *)malloc(max_atoms * 3 * sizeof(double));
      atom_radii = (double *)malloc(max_atoms * sizeof(double));
      atom_adjacency_matrix = (char *)malloc(max_atoms * max_atoms * sizeof(char));

      atom_names = (char **)malloc(max_atoms * sizeof(char *));
      *atom_names = (char *)malloc(max_atoms * 4 * sizeof(char));
      for (i = 1; i < max_atoms; i++)
        {
          atom_names[i] = atom_names[i - 1] + 4;
        }

      if (atom_numbers == NULL || atom_numbers2 == NULL || atom_positions == NULL || atom_radii == NULL ||
          atom_adjacency_matrix == NULL)
        {
          s_error("can't allocate memory");
        }
    }
}

/*
void glorlogfunc(const char *message) {
    fprintf(stderr, "GLor: %s\n", message);
}
 */

static void createPixmap(int width, int height)
{
  GLORInitAttribute attrs[] = {kGLORIAFramebufferWidth, 512, kGLORIAFramebufferHeight, 1024, kGLORIAEndOfAttributeList};
  double x, y, z, fx, fy, fz, ux, uy, uz;
  double c1 = cos(torad(-rotation));
  double s1 = sin(torad(-rotation));
  double c2 = cos(torad(-tilt));
  double s2 = sin(torad(-tilt));

  /* glorSetLogCallback(glorlogfunc); */
  GLORError glor_error = glorInit(attrs);
  if (glor_error != kGLORENoError)
    {
      fprintf(stderr, "glorInit: %s\n", glorErrorString(glor_error));
    }
  glorSetBackgroundColor(1, 1, 1, 1);
  glorCameraProjectionParameters(45, 0.5, 7.0 * range);

  x = -c1 * xeye + s1 * zeye;
  y = s1 * s2 * xeye - c2 * yeye + s2 * c1 * zeye;
  z = -c2 * s1 * xeye - s2 * yeye - c2 * c1 * zeye;
  fx = x + s1;
  fy = y + s2 * c1;
  fz = z - c2 * c1;
  ux = 0;
  uy = c2;
  uz = s2;
  glorCameraLookAt(x, y, z, fx, fy, fz, ux, uy, uz);

  if (box)
    {
#define n_boxlines 12
      int i = 0;
      double positions[3 * n_boxlines];
      double directions[3 * n_boxlines];
      double colors[3 * n_boxlines];
      double radii[n_boxlines];
      double lengths[n_boxlines];
      double edges[2 * 3 * n_boxlines] = {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1,
                                          0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1,
                                          1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1};

      for (i = 0; i < n_boxlines; i++)
        {
          int x1, y1, z1, x2, y2, z2;
          x1 = edges[2 * 3 * i + 0];
          y1 = edges[2 * 3 * i + 1];
          z1 = edges[2 * 3 * i + 2];
          x2 = edges[2 * 3 * i + 3];
          y2 = edges[2 * 3 * i + 4];
          z2 = edges[2 * 3 * i + 5];
          positions[3 * i + 0] = xmin + x1 * (xmax - xmin);
          positions[3 * i + 1] = ymin + y1 * (ymax - ymin);
          positions[3 * i + 2] = zmin + z1 * (zmax - zmin);
          directions[3 * i + 0] = x2 - x1;
          directions[3 * i + 1] = y2 - y1;
          directions[3 * i + 2] = z2 - z1;
          colors[3 * i + 0] = 0;
          colors[3 * i + 1] = 0;
          colors[3 * i + 2] = 0;
          radii[i] = 0.05;
          lengths[i] = (x2 - x1) * (xmax - xmin) + (y2 - y1) * (ymax - ymin) + (z2 - z1) * (zmax - zmin);
        }
      glorDrawCylinderMesh(n_boxlines, positions, directions, colors, radii, lengths);
#undef n_boxlines
    }

  {
    int i, j;
    int n_atoms = 0;
    double *positions;
    double *colors;
    double *radii;

    for (i = 0; i < num_atoms; i++)
      {
        if (atom_numbers[i] != 0)
          {
            n_atoms++;
          }
      }
    positions = (double *)malloc(sizeof(double) * 3 * n_atoms);
    colors = (double *)malloc(sizeof(double) * 3 * n_atoms);
    radii = (double *)malloc(sizeof(double) * n_atoms);

    for (i = 0, j = 0; i < num_atoms; i++)
      {
        if (atom_numbers[i] != 0)
          {
            positions[j * 3 + 0] = atom_positions[0 + 3 * i];
            positions[j * 3 + 1] = atom_positions[1 + 3 * i];
            positions[j * 3 + 2] = atom_positions[2 + 3 * i];
            colors[j * 3 + 0] = f_ptable[atom_numbers[i] - 1][0] / 255.0;
            colors[j * 3 + 1] = f_ptable[atom_numbers[i] - 1][1] / 255.0;
            colors[j * 3 + 2] = f_ptable[atom_numbers[i] - 1][2] / 255.0;
            radii[j++] = atom_radii[i];
          }
        else
          continue;
      }
    glorDrawSphereMesh(n_atoms, positions, colors, radii);

    free(positions);
    free(colors);
    free(radii);
  }
  if (bonds)
    {
      int i, j, k, l;
      double vx, vy, vz, cyl_len;
      int n_bonds = 0;
      double *positions;
      double *directions;
      double *colors;
      double *radii;
      double *lengths;

      for (i = 0; i < num_atoms; i++)
        {
          if (atom_numbers[i] == 0) continue;
          for (j = i + 1; j < num_atoms; j++)
            {
              if (atom_numbers[j] && atom_adjacency_matrix[i * num_atoms + j])
                {
                  n_bonds++;
                }
            }
        }

      positions = (double *)malloc(sizeof(double) * 3 * n_bonds);
      directions = (double *)malloc(sizeof(double) * 3 * n_bonds);
      colors = (double *)malloc(sizeof(double) * 3 * n_bonds);
      radii = (double *)malloc(sizeof(double) * n_bonds);
      lengths = (double *)malloc(sizeof(double) * n_bonds);

      for (i = 0, j = 0, l = 0; i < num_atoms; i++)
        {
          if (atom_numbers[i] == 0) continue;
          for (j = i + 1; j < num_atoms; j++)
            {
              if (atom_numbers[j] && atom_adjacency_matrix[i * num_atoms + j])
                {
                  if (atom_positions[2 + 3 * j] > atom_positions[2 + 3 * i])
                    {
                      vx = atom_positions[0 + 3 * j] - atom_positions[0 + 3 * i];
                      vy = atom_positions[1 + 3 * j] - atom_positions[1 + 3 * i];
                      vz = atom_positions[2 + 3 * j] - atom_positions[2 + 3 * i];
                      k = i;
                    }
                  else
                    {
                      vx = atom_positions[0 + 3 * i] - atom_positions[0 + 3 * j];
                      vy = atom_positions[1 + 3 * i] - atom_positions[1 + 3 * j];
                      vz = atom_positions[2 + 3 * i] - atom_positions[2 + 3 * j];
                      k = j;
                    }

                  cyl_len = sqrt(vx * vx + vy * vy + vz * vz);
                  positions[3 * l + 0] = atom_positions[0 + 3 * k];
                  positions[3 * l + 1] = atom_positions[1 + 3 * k];
                  positions[3 * l + 2] = atom_positions[2 + 3 * k];
                  directions[3 * l + 0] = vx;
                  directions[3 * l + 1] = vy;
                  directions[3 * l + 2] = vz;
                  lengths[l] = cyl_len;
                  radii[l] = cyl_rad;
                  colors[3 * l + 0] = 1;
                  colors[3 * l + 1] = 1;
                  colors[3 * l + 2] = 1;
                  l++;
                }
            }
        }
      glorDrawCylinderMesh(n_bonds, positions, directions, colors, radii, lengths);

      free(positions);
      free(directions);
      free(colors);
      free(radii);
      free(lengths);
    }

  {
    int i;
    char *cp;
    GLORPixel *pixmap = (GLORPixel *)malloc(sizeof(GLORPixel) * width * height);
    unsigned char *pixels = (unsigned char *)malloc(sizeof(unsigned char) * 3 * width * height);

    glorGetPixmap(pixmap, width, height);
    for (i = 0; i < width * height; i++)
      {
        pixels[3 * i + 0] = pixmap[i].r;
        pixels[3 * i + 1] = pixmap[i].g;
        pixels[3 * i + 2] = pixmap[i].b;
      }

    sprintf(path, "%s%4d.jpg", name, current_cycle);
    for (cp = path; *cp; cp++)
      if (*cp == ' ') *cp = '0';
    writeJpegForPixels(path, 100, (unsigned char *)pixels, width, height);
    puts(path);
    free(pixels);
    free(pixmap);
  }
  glorClear();
  glorTerminate();
}

int main(int argc, char **argv)
{
  ac = argc;
  avp = argv;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(window_width, window_height);
  glutInitWindowPosition(600, 400);
  glutCreateWindow(argv[1]);
  init();
  glutDisplayFunc(DrawScene);

  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(specialKey);

  read_dat();

  if (autoscale)
    {
      analyze();
    }

  read_cycle();
  init_List();

  range = fabs(zmax) + fabs(zmin);
  if (range < fabs(xmax) + fabs(xmin)) range = fabs(xmax) + fabs(xmin);

  if (range < fabs(ymax) + fabs(ymin)) range = fabs(ymax) + fabs(ymin);

  magnification = pow(1.2, (double)magstep);
  zeye = -2.0 * range * magnification;

  if (povray <= 0)
    {
      glutMainLoop();
    }
  else
    {
      makePov();
    }
  return 0;
}
