#ifndef MOLDYN_H
#define MOLDYN_H

#ifndef Bool
#define Bool char
#endif
#define False 0
#define True 1

#define MAX_CYCLES 7500
#define MAX_STRING 256

#include "moldyn_utilities.h"
#include "moldyn_element_information.h"
#include "moldyn_graphics.h"

typedef enum
{
  normal,
  xyz,
  unichem
} format_t;
extern format_t format;
extern FILE *fptr;

extern char name[256];
extern char path[266];
extern char title[MAX_STRING];


extern Bool file_done;

extern Bool jpeg;

extern int resolution;
extern Bool bonds;
extern Bool colors;
extern Bool chain;
extern Bool numbers;
extern Bool hint;
extern Bool box;
extern int povray;
extern int pix;

extern double radius;
extern float range;
extern double dist;

extern fpos_t cycle_position[MAX_CYCLES];
extern int current_cycle;
extern int icycle;
extern double energy0;
extern double energy;


extern int magstep;
extern double magnification;

extern float cyl_rad;

extern int num_atoms;
extern double xmin, ymin, zmin;
extern double xmax, ymax, zmax;

extern float *atom_positions;
extern float *atom_spins;
extern float *atom_radii;
extern float *atom_colors;
extern int *atom_numbers;
extern int *atom_numbers2;
extern char **atom_names;
extern char *atom_adjacency_matrix;

void read_cycle(void);
void moldyn_close_file(void);

#endif
