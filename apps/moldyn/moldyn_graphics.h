#ifndef MOLDYN_GRAPHICS_H
#define MOLDYN_GRAPHICS_H

#define MAX_SLICES 40
#define MAX_STACKS 20


extern float xeye, yeye, zeye; /* camera position */
extern double rotation;        /* camera rotation (negative rotation around y axis)*/
extern double tilt;            /* camera tilt (rotation around x axis)*/
extern Bool gr3_debug;

#define MOLDYN_EXPORT_TO_JPEG 1
#define MOLDYN_EXPORT_TO_PNG 2
#define MOLDYN_EXPORT_TO_POV 3
#define MOLDYN_EXPORT_TO_HTML 4
void makePov(int type);

void animate(void);

void moldyn_init_graphics(int *argcp, char **argv, const char *window_name);

void start_mainloop(void);

void moldyn_update_graphics(void);
#endif
