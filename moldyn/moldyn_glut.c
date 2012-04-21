#include <stdlib.h>
#include <stdio.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#include "moldyn.h"

static int is_animating = FALSE;
static int window = 0;
static int window_width = 640;
static int window_height = 480;

static void moldyn_display_callback_(void);
static void moldyn_reshape_callback_(int width, int height);
static void moldyn_idle_callback_(void);

void moldyn_init_glut(void) {
    int num_glut_args = 0;
    glutInit(&num_glut_args, NULL);
    glutInitWindowSize(window_width, window_height);
    window = glutCreateWindow("moldyn");
    glutDisplayFunc(moldyn_display_callback_);
    glutReshapeFunc(moldyn_reshape_callback_);
    moldyn_init_glut_menu();
    moldyn_init_glut_key_bindings();
}

void moldyn_start_glut_mainloop(void) {
    glutMainLoop();
}

void moldyn_terminate_glut(void) {
    moldyn_terminate_glut_menu();
    moldyn_terminate_glut_key_bindings();
    if (window) {
        glutDestroyWindow(window);
    }
    /* normal glut does not have a function to exit the main loop */
}

void moldyn_show_help_overlay(void);

void moldyn_toggle_animation(void) {
    if (is_animating) {
        moldyn_stop_animation();
    } else {
        moldyn_start_animation();
    }
}

void moldyn_start_animation(void) {
    if (!is_animating) {
        glutIdleFunc(moldyn_idle_callback_);
        is_animating = TRUE;
    }
}

void moldyn_stop_animation(void) {
    if (is_animating) {
        glutIdleFunc(NULL);
        is_animating = FALSE;
    }
}

static void moldyn_idle_callback_(void) {
    if (is_animating) {
        if (moldyn_next_frame()) {
            moldyn_stop_animation();
        } else {
            moldyn_update_scene();
            glutPostRedisplay();
        }
    }
}

static void moldyn_display_callback_(void) {
    moldyn_drawimage(0, window_width, 0, window_height, window_width, window_height, MOLDYN_WINDOW_OPENGL);
    glutSwapBuffers();
}

static void moldyn_reshape_callback_(int width, int height) {
    window_width = (width > 0) ? width : 1;
    window_height = (height > 0) ? height : 1;
}
