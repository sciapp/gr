/* OpenGL via network might require LIBGL_ALWAYS_INDIRECT to be set */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
/* The escape key is missing as a GLUT_KEY_x macro, but is reported via glutKeyboardFunc() as 27. */
#define GLUT_KEY_ESCAPE 27

#include "moldyn.h"

#include "gr3.h"

#include "jpeg2ps/jpeg2ps.h"
#ifndef M_PI
#define M_PI 3.1415926535
#endif

#define torad(alpha) ((alpha) * (M_PI / 180.0))
#define todeg(alpha) ((alpha) * (180.0 / M_PI))

#define ROTATE 1
#define TRANSLATE 2
static int move_stat = ROTATE; /* mode of movement: Rotation/Translation */

static int moving = 0, startx = 0, starty = 0; /* values for camera movement with the mouse */

GLfloat xeye = 0.0, yeye = 0.0, zeye; /* camera position */
double rotation = 0;                  /* camera rotation (negative rotation around y axis)*/
double tilt = 0;                      /* camera tilt (rotation around x axis)*/
Bool gr3_debug = False;

/* current window size */
static int window_width = 555;
static int window_height = 555;
static int last_init_done = False;

#define SHOW_ALL 222
#define SHOW_NEXT 111
#define HOLD 333
#define SHOW_PREV 444
static int show_stat = HOLD;

static void motion(int x, int y);
static void reshape(int w, int h);
static void mouse(int button, int state, int x, int y);
static void keyboard(unsigned char key, int x, int y);
static void specialKey(int key, int x, int y);
static void moldyn_init_glut(int *argcp, char **argv, const char *window_name);
static void moldyn_init_gr3(void);

static void drawStr(float x, float y, const char *str, void *fnt);
static void moldyn_display_callback(void);

static void moldyn_display_text_(void);
static void moldyn_display_box_(void);

static void moldyn_export_(int type, int width, int height);

void start_mainloop()
{
  glutMainLoop();
}

void moldyn_update_graphics(void)
{
  double x, y, z, fx, fy, fz, ux, uy, uz;
  double c1 = cos(torad(-rotation));
  double s1 = sin(torad(-rotation));
  double c2 = cos(torad(-tilt));
  double s2 = sin(torad(-tilt));
  /* Transform the view */
  x = -c1 * xeye + s1 * zeye;
  y = s1 * s2 * xeye - c2 * yeye + s2 * c1 * zeye;
  z = -c2 * s1 * xeye - s2 * yeye - c2 * c1 * zeye;
  fx = x + s1;
  fy = y + s2 * c1;
  fz = z - c2 * c1;
  ux = 0;
  uy = c2;
  uz = s2;

  gr3_setcameraprojectionparameters(45, 0.5, 7.0 * range);
  gr3_cameralookat(x, y, z, fx, fy, fz, ux, uy, uz);
  if (file_done)
    { /* TODO: disable NEXT in a better way */
      last_init_done = True;
    }

  gr3_clear();
  if (num_atoms > 0)
    {
      if (colors)
        {
          gr3_drawspheremesh(num_atoms, atom_positions, atom_colors, atom_radii);
        }
      else
        {
          float *atom_color_replacement = (float *)malloc(num_atoms * sizeof(float) * 3);
          if (!atom_color_replacement)
            {
              moldyn_error("Failed to allocate memory for atom_color_replacement.");
            }
          else
            {
              int i;
              for (i = 0; i < num_atoms * 3; i++)
                {
                  atom_color_replacement[i] = 1.0f;
                }
              gr3_drawspheremesh(num_atoms, atom_positions, atom_color_replacement, atom_radii);
              free(atom_color_replacement);
            }
        }
    }

  if (format == xyz)
    {
      int i;
      int num_spins = 0;
      float spin_len = 1;
      float spintop_len = 0.4;
      float *spin_positions;
      float *spin_directions;
      float *spin_colors;
      float *spin_radii;
      float *spin_lengths;
      for (i = 0; i < num_atoms; i++)
        {
          if (atom_spins[3 * i + 0] != 0 || atom_spins[3 * i + 1] != 0 || atom_spins[3 * i + 2] != 0)
            {
              num_spins++;
            }
        }
      spin_positions = (float *)malloc(sizeof(float) * 3 * num_spins);
      spin_directions = (float *)malloc(sizeof(float) * 3 * num_spins);
      spin_colors = (float *)malloc(sizeof(float) * 3 * num_spins);
      spin_radii = (float *)malloc(sizeof(float) * num_spins);
      spin_lengths = (float *)malloc(sizeof(float) * num_spins);
      for (i = 0; i < num_atoms; i++)
        {
          if (atom_spins[3 * i + 0] != 0 || atom_spins[3 * i + 1] != 0 || atom_spins[3 * i + 2] != 0)
            {
              spin_positions[3 * i + 0] = atom_positions[3 * i + 0] - atom_spins[3 * i + 0] / 2 * spin_len;
              spin_positions[3 * i + 1] = atom_positions[3 * i + 1] - atom_spins[3 * i + 1] / 2 * spin_len;
              spin_positions[3 * i + 2] = atom_positions[3 * i + 2] - atom_spins[3 * i + 2] / 2 * spin_len;
              spin_directions[3 * i + 0] = atom_spins[3 * i + 0];
              spin_directions[3 * i + 1] = atom_spins[3 * i + 1];
              spin_directions[3 * i + 2] = atom_spins[3 * i + 2];
              spin_colors[3 * i + 0] = 1;
              spin_colors[3 * i + 1] = 1;
              spin_colors[3 * i + 2] = 1;
              spin_lengths[i] = spin_len;
              spin_radii[i] = cyl_rad;
            }
        }
      gr3_drawcylindermesh(num_spins, spin_positions, spin_directions, spin_colors, spin_radii, spin_lengths);
      for (i = 0; i < num_atoms; i++)
        {
          if (atom_spins[3 * i + 0] != 0 || atom_spins[3 * i + 1] != 0 || atom_spins[3 * i + 2] != 0)
            {
              spin_positions[3 * i + 0] = atom_positions[3 * i + 0] + atom_spins[3 * i + 0] / 2 * spin_len;
              spin_positions[3 * i + 1] = atom_positions[3 * i + 1] + atom_spins[3 * i + 1] / 2 * spin_len;
              spin_positions[3 * i + 2] = atom_positions[3 * i + 2] + atom_spins[3 * i + 2] / 2 * spin_len;
              spin_directions[3 * i + 0] = atom_spins[3 * i + 0];
              spin_directions[3 * i + 1] = atom_spins[3 * i + 1];
              spin_directions[3 * i + 2] = atom_spins[3 * i + 2];
              spin_colors[3 * i + 0] = 1;
              spin_colors[3 * i + 1] = 1;
              spin_colors[3 * i + 2] = 1;
              spin_lengths[i] = spintop_len;
              spin_radii[i] = 2 * cyl_rad;
            }
        }
      gr3_drawconemesh(num_spins, spin_positions, spin_directions, spin_colors, spin_radii, spin_lengths);
      free(spin_positions);
      free(spin_directions);
      free(spin_colors);
      free(spin_radii);
      free(spin_lengths);
    }


  if (bonds)
    {
      int i, j, k, l;
      double vx, vy, vz, cyl_len;
      int num_bonds = 0;
      float *bond_positions;
      float *bond_directions;
      float *bond_colors;
      float *bond_radii;
      float *bond_lengths;

      for (i = 0; i < num_atoms; i++)
        {
          if (atom_numbers[i] == 0) continue;
          for (j = i + 1; j < num_atoms; j++)
            {
              if (atom_numbers[j] && atom_adjacency_matrix[i * num_atoms + j])
                {
                  num_bonds++;
                }
            }
        }

      bond_positions = (float *)malloc(sizeof(float) * 3 * num_bonds);
      bond_directions = (float *)malloc(sizeof(float) * 3 * num_bonds);
      bond_colors = (float *)malloc(sizeof(float) * 3 * num_bonds);
      bond_radii = (float *)malloc(sizeof(float) * num_bonds);
      bond_lengths = (float *)malloc(sizeof(float) * num_bonds);

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
                  bond_positions[3 * l + 0] = atom_positions[0 + 3 * k];
                  bond_positions[3 * l + 1] = atom_positions[1 + 3 * k];
                  bond_positions[3 * l + 2] = atom_positions[2 + 3 * k];
                  bond_directions[3 * l + 0] = vx;
                  bond_directions[3 * l + 1] = vy;
                  bond_directions[3 * l + 2] = vz;
                  bond_lengths[l] = cyl_len;
                  bond_radii[l] = cyl_rad;
                  bond_colors[3 * l + 0] = 1;
                  bond_colors[3 * l + 1] = 1;
                  bond_colors[3 * l + 2] = 1;
                  l++;
                }
            }
        }
      gr3_drawcylindermesh(num_bonds, bond_positions, bond_directions, bond_colors, bond_radii, bond_lengths);

      free(bond_positions);
      free(bond_directions);
      free(bond_colors);
      free(bond_radii);
      free(bond_lengths);
    }
}

void moldyn_display_callback(void)
{
  int err;
  double x, y, z, fx, fy, fz, ux, uy, uz;
  double c1 = cos(torad(-rotation));
  double s1 = sin(torad(-rotation));
  double c2 = cos(torad(-tilt));
  double s2 = sin(torad(-tilt));
  /* Transform the view */
  x = -c1 * xeye + s1 * zeye;
  y = s1 * s2 * xeye - c2 * yeye + s2 * c1 * zeye;
  z = -c2 * s1 * xeye - s2 * yeye - c2 * c1 * zeye;
  fx = x + s1;
  fy = y + s2 * c1;
  fz = z - c2 * c1;
  ux = 0;
  uy = c2;
  uz = s2;

  gr3_setcameraprojectionparameters(45, 0.5, 7.0 * range);
  gr3_cameralookat(x, y, z, fx, fy, fz, ux, uy, uz);
  err = gr3_drawimage(0, window_width, 0, window_height, window_width, window_height, GR3_DRAWABLE_OPENGL);
  if (err)
    {
      moldyn_error(gr3_geterrorstring(err));
    }

  moldyn_display_text_();

  if (box)
    {
      moldyn_display_box_();
    }

  glutSwapBuffers();

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
}

static void moldyn_init_gr3(void)
{
  int attrib_list[] = {GR3_IA_FRAMEBUFFER_WIDTH, 1024, GR3_IA_FRAMEBUFFER_HEIGHT, 1024, GR3_IA_END_OF_LIST};
  int err;
  if (gr3_debug)
    {
      gr3_setlogcallback(moldyn_log);
    }
  err = gr3_init(attrib_list);
  if (err)
    {
      moldyn_error(gr3_geterrorstring(err));
    }
  gr3_setbackgroundcolor(1, 1, 1, 1);

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
}

static void moldyn_display_text_(void)
{
  char text[MAX_STRING];
  char atom_name[8];
  int i;
  /* GLUT fonts */
  static void *font_12 = GLUT_BITMAP_HELVETICA_12;
  static void *font_18 = GLUT_BITMAP_HELVETICA_18;

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor3f(0, 0, 0);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0.0, 1.0, 0.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
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
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  if (numbers)
    {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(45.0f, (GLfloat)window_width / (GLfloat)window_height, 0.5, 7.0 * range);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glTranslatef(xeye, yeye, zeye);

      glRotatef(-rotation, 0.0, 1.0, 0.0);
      glRotatef(tilt, 1.0, 0.0, 0.0);
      for (i = 0; i < num_atoms; i++)
        {
          if (atom_numbers[i] != 0)
            {
              char *c;
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

              glRasterPos3f(0, 0, atom_radii[i]);
              for (c = atom_name; *c != '\0'; c++)
                {
                  glutBitmapCharacter(font_12, *c);
                }
              glPopMatrix();
            }
        }
    }
}

static void moldyn_display_box_(void)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0f, (GLfloat)window_width / (GLfloat)window_height, 0.5, 7.0 * range);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(xeye, yeye, zeye);

  glRotatef(-rotation, 0.0, 1.0, 0.0);
  glRotatef(tilt, 1.0, 0.0, 0.0);

  glDisable(GL_LIGHTING);
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
  glEnable(GL_LIGHTING);
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

void makePov(int type)
{
  int xres, yres;
  float x, y, z;
  double c1 = cos(torad(-rotation - 10));
  double s1 = sin(torad(-rotation - 10));
  double c2 = cos(torad(-tilt - 20));
  double s2 = sin(torad(-tilt - 20));

  xres = resolution * 1.25 + 0.5;
  yres = 0.8 * xres + 0.5;

  while (True)
    {
      if (1.0 * xres / yres > 1.25)
        {
          yres++;
        }
      else if (1.0 * xres / yres < 1.25)
        {
          xres++;
        }
      else
        {
          break;
        }
    }

  x = -c1 * xeye + s1 * zeye;
  y = s1 * s2 * xeye - c2 * yeye + s2 * c1 * zeye;
  z = -c2 * s1 * xeye - s2 * yeye - c2 * c1 * zeye;

  gr3_setlightdirection(x, y, z);
  gr3_setquality(GR3_QUALITY_POVRAY_8X_SSAA);
  if (povray == -9999)
    {
      moldyn_export_(type, xres, yres);
    }
  else if (povray > 0)
    {
      rewind(fptr);
      current_cycle = 0;
      while (True)
        {
          read_cycle();
          if (file_done) break;
          moldyn_update_graphics();
          moldyn_export_(type, xres, yres);
        }
    }
  gr3_setquality(GR3_QUALITY_OPENGL_NO_SSAA);
  gr3_setlightdirection(0, 0, 0);
}

static void moldyn_export_(int type, int width, int height)
{
  int err;
  char *cp;
  char *extension;
  switch (type)
    {
    case MOLDYN_EXPORT_TO_JPEG:
      extension = "jpg";
      break;
    case MOLDYN_EXPORT_TO_PNG:
      extension = "png";
      break;
    case MOLDYN_EXPORT_TO_POV:
      extension = "pov";
      break;
    case MOLDYN_EXPORT_TO_HTML:
      extension = "html";
      break;
    }
  sprintf(path, "%s%4d.%s", name, current_cycle, extension);
  for (cp = path; *cp; cp++)
    if (*cp == ' ') *cp = '0';
  err = gr3_export(path, width, height);
  moldyn_log(path);
  if (err)
    {
      moldyn_error(gr3_geterrorstring(err));
    }
}

void animate(void)
{
  char string[MAX_STRING];

  if (file_done)
    {
      glutIdleFunc(NULL);
      show_stat = HOLD;
    }
  if (file_done && jpeg)
    {
      jpeg = False;

      moldyn_update_graphics();

      glutPostRedisplay();

      sprintf(string, "cat %s????.jpg | ffmpeg -y -v 0 -r 30 -f image2pipe -vcodec mjpeg -i - -vcodec mpeg4 %s.mov",
              name, name);
      system(string);
      return;
    }

  /* Redraw for animation (no display lists) */
  read_cycle();
  moldyn_update_graphics();
  glutPostRedisplay();

  if (jpeg)
    {
      moldyn_export_(MOLDYN_EXPORT_TO_JPEG, window_width, window_height);
    }
}

void moldyn_init_graphics(int *argcp, char **argv, const char *window_name)
{
  moldyn_init_glut(argcp, argv, window_name);
  moldyn_init_gr3();
}

static void moldyn_menu_(int value)
{
  switch (value)
    {
    case 0:
      moldyn_exit(0);
      break;
    case 1:
      {
        char *argv[] = {"jpeg2ps", "-o", "moldyn.eps", path};
        moldyn_export_(MOLDYN_EXPORT_TO_JPEG, 2000, 2000);
        jpeg2ps_main(sizeof(argv) / sizeof(char *), argv);
      }
#ifdef _WIN32
      system("copy moldyn.eps %PRINTER%");
#else
      system("lpr -h moldyn.eps");
#endif

      break;
    case 2:
      hint = (hint) ? False : True;
      glutPostRedisplay();
      break;
    case 3:
      numbers = (numbers) ? False : True;
      glutPostRedisplay();
      break;
    }
}

static void moldyn_menu_export_(int value)
{
  switch (value)
    {
    case MOLDYN_EXPORT_TO_PNG:
    case MOLDYN_EXPORT_TO_JPEG:
    case MOLDYN_EXPORT_TO_HTML:
    case MOLDYN_EXPORT_TO_POV:
      gr3_setquality(GR3_QUALITY_OPENGL_2X_SSAA);
      moldyn_export_(value, 500, 500);
      gr3_setquality(GR3_QUALITY_OPENGL_NO_SSAA);
      break;
    case 0:
      if (show_stat == HOLD && !file_done)
        {
          jpeg = True;
          moldyn_export_(MOLDYN_EXPORT_TO_JPEG, window_width, window_height);

          glutIdleFunc(animate);
          show_stat = SHOW_ALL;
        }
      else if (show_stat == SHOW_ALL)
        {
          jpeg = False;
          show_stat = HOLD;
          moldyn_update_graphics();
          glutPostRedisplay();
        }
      break;
    }
}

static void moldyn_menu_view_(int value)
{
  switch (value)
    {
    case 0:
      xeye = 0.0;
      yeye = 0.0;
      zeye = -2.0 * range;
      glutPostRedisplay();
      break;
    case 1:
      if (magstep <= 10)
        {
          magstep++;
          magnification = pow(1.2, (double)magstep);
          zeye = -2 * range * magnification;
          glutPostRedisplay();
        }
      break;

    case 2:
      if (magstep >= -10)
        {
          magstep--;
          magnification = pow(1.2, (double)magstep);
          zeye = -2 * range * magnification;
          glutPostRedisplay();
        }
      break;
    case 3:
      if (move_stat == ROTATE)
        {
          move_stat = TRANSLATE;
        }
      else if (move_stat == TRANSLATE)
        {
          move_stat = ROTATE;
        }
      break;
    }
}


static void moldyn_menu_frame_(int value)
{
  switch (value)
    {
    case 0:
      if (current_cycle == 1)
        {
          break;
        }
      show_stat = SHOW_PREV;
      current_cycle -= 2;
      file_done = 0;
      if (feof(fptr)) rewind(fptr);

      if (fsetpos(fptr, &cycle_position[current_cycle])) moldyn_error("can't position file");

      read_cycle();
      moldyn_update_graphics();
      glutPostRedisplay();
      break;
    case 1:
      if (file_done && last_init_done)
        {
          break;
        }
      else
        {
          show_stat = SHOW_NEXT;
          read_cycle();
          moldyn_update_graphics();
          glutPostRedisplay();
          break;
        }

    case 2:
      if (show_stat == HOLD && !file_done)
        {
          glutIdleFunc(animate);
          show_stat = SHOW_ALL;
        }
      else if (show_stat == SHOW_ALL)
        {
          jpeg = False;
          show_stat = HOLD;
          moldyn_update_graphics();
          glutPostRedisplay();
        }
      break;
    }
}

static void moldyn_init_glut(int *argcp, char **argv, const char *window_name)
{
  int i = 0;
  if (argcp == NULL || argv == NULL)
    {
      argcp = &i;
      argv = NULL;
    }
#ifdef __linux__
  char *argv2[] = {"moldyn", "-indirect"};
  int argcp2[] = {2};
  if (getenv("SSH_CLIENT") != NULL)
    {
      argv = (char **)argv2;
      argcp = argcp2;
      fprintf(stderr, "Indirect rendering over SSH\n");
    }
#endif

  glutInit(argcp, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(window_width, window_height);
  glutInitWindowPosition(600, 400);
  glutCreateWindow(window_name);

  glutDisplayFunc(moldyn_display_callback);

  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(specialKey);

  {
    int menu_view;
    int menu_export;
    int menu_frame;
    menu_frame = glutCreateMenu(moldyn_menu_frame_);
    glutAddMenuEntry("Previous frame [b]", 0);
    glutAddMenuEntry("Next frame [n]", 1);
    glutAddMenuEntry("Animaton On/Off [h]", 2);
    menu_view = glutCreateMenu(moldyn_menu_view_);
    glutAddMenuEntry("Switch rotation/translation [s]", 3);
    glutAddMenuEntry("Zoom in [m]", 2);
    glutAddMenuEntry("Zoom out [r]", 1);
    glutAddMenuEntry("Reset zoom and translation [0]", 0);
    menu_export = glutCreateMenu(moldyn_menu_export_);
    glutAddMenuEntry("PNG image (.png)", MOLDYN_EXPORT_TO_PNG);
    glutAddMenuEntry("JPEG image (.jpg)", MOLDYN_EXPORT_TO_JPEG);
    glutAddMenuEntry("HTML5 document (.html)", MOLDYN_EXPORT_TO_HTML);
    glutAddMenuEntry("POV-Ray scene (.pov)", MOLDYN_EXPORT_TO_POV);
    glutAddMenuEntry("QuickTime movie (.mov)", 0);
    glutCreateMenu(moldyn_menu_);
    glutAddMenuEntry("Show hints [?]", 2);
    glutAddMenuEntry("Show numbers [t]", 3);
    glutAddSubMenu("Frames...", menu_frame);
    glutAddSubMenu("Adjust view...", menu_view);
    glutAddSubMenu("Export as...", menu_export);
    glutAddMenuEntry("Print [p]", 1);
    glutAddMenuEntry("Quit [q]", 0);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
  }
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

  switch (key)
    {
    case 'a':
      povray = pix = -9999;
      makePov(MOLDYN_EXPORT_TO_PNG);
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
          moldyn_update_graphics();
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

      if (fsetpos(fptr, &cycle_position[current_cycle])) moldyn_error("can't position file");

      read_cycle();
      moldyn_update_graphics();
      glutPostRedisplay();
      break;

    case 'h':
    case 'j':
      if (show_stat == HOLD && !file_done)
        {
          if (key == 'j')
            {
              jpeg = True;
              moldyn_export_(MOLDYN_EXPORT_TO_JPEG, window_width, window_height);
            }

          glutIdleFunc(animate);
          show_stat = SHOW_ALL;
        }
      else if (show_stat == SHOW_ALL)
        {
          jpeg = False;
          show_stat = HOLD;
          moldyn_update_graphics();
          glutPostRedisplay();
        }
      break;

    case 'c':
    case 'C':
      moldyn_export_(MOLDYN_EXPORT_TO_JPEG, 800, 800);
      break;

    case 'J':
      moldyn_export_(MOLDYN_EXPORT_TO_JPEG, window_width, window_height);
      break;

    case 'p':
      {
        char *argv[] = {"jpeg2ps", "-o", "moldyn.eps", path};
        moldyn_export_(MOLDYN_EXPORT_TO_JPEG, 2000, 2000);
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
