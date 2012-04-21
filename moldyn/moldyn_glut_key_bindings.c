#include <stdlib.h>
#include <stdio.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#define GLUT_KEY_ESCAPE 27

#include "moldyn.h"

static void moldyn_keyboard_callback_(unsigned char key, int x, int y);
static void moldyn_special_callback_(int key, int x, int y);

void moldyn_init_glut_key_bindings(void) {
    glutKeyboardFunc(moldyn_keyboard_callback_);
    glutSpecialFunc(moldyn_special_callback_);
}

void moldyn_terminate_glut_key_bindings(void) {
    glutKeyboardFunc(NULL);
    glutSpecialFunc(NULL);
}

static void moldyn_keyboard_callback_(unsigned char key, int x, int y) {
    int modifiers = glutGetModifiers();
    int has_shift = modifiers & GLUT_ACTIVE_SHIFT;
    int has_alt = modifiers & GLUT_ACTIVE_ALT;
    int has_ctrl = modifiers & GLUT_ACTIVE_CTRL;
    switch (key) {
        case GLUT_KEY_ESCAPE:
            moldyn_exit(0);
            break;
        case 'b':
        case 'B':
            if (!moldyn_previous_frame()) {
                moldyn_update_scene();
                glutPostRedisplay();
            }
            break;
        case 'n':
        case 'N':
            if (!moldyn_next_frame()) {
                moldyn_update_scene();
                glutPostRedisplay();
            }
            break;
        case 'm':
        case 'M':
            moldyn_toggle_animation();
            break;
        case 'v':
        case 'V':
            if (!moldyn_first_frame()) {
                moldyn_update_scene();
                glutPostRedisplay();
            }
            break;
        default:
            break;
    }
}

static void moldyn_special_callback_(int key, int x, int y) {
    int modifiers = glutGetModifiers();
    int has_shift = modifiers & GLUT_ACTIVE_SHIFT;
    int has_alt = modifiers & GLUT_ACTIVE_ALT;
    int has_ctrl = modifiers & GLUT_ACTIVE_CTRL;
    switch (key) {
        default:
            break;
    }
}