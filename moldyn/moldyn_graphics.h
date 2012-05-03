#ifndef MOLDYN_GRAPHICS_H
#define MOLDYN_GRAPHICS_H

#define MAX_SLICES 40
#define MAX_STACKS 20


extern float xeye, yeye, zeye; /* camera position */
extern double rotation; /* camera rotation (negative rotation around y axis)*/
extern double tilt; /* camera tilt (rotation around x axis)*/
extern Bool gr3_debug;

void makePov(void);

void animate(void);

void moldyn_init_graphics(int *argcp, char **argv, const char *window_name);

void start_mainloop(void);

void moldyn_update_graphics(void);
#endif
