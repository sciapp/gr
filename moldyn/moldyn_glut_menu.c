#include <stdlib.h>
#include <stdio.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "moldyn.h"

#define MENU_ENTRY_OPEN_FILE 0
#define MENU_ENTRY_EXPORT_PNG 0
#define MENU_ENTRY_EXPORT_JPEG 1
#define MENU_ENTRY_EXPORT_POV 2
#define MENU_ENTRY_EXPORT_HTML 3
#define MENU_ENTRY_QUALITY_OPENGL_NO_SSAA 0
#define MENU_ENTRY_QUALITY_OPENGL_2X_SSAA 1
#define MENU_ENTRY_QUALITY_OPENGL_4X_SSAA 2
#define MENU_ENTRY_QUALITY_OPENGL_8X_SSAA 3
#define MENU_ENTRY_QUALITY_OPENGL_16X_SSAA 4
#define MENU_ENTRY_QUALITY_POVRAY_NO_SSAA 0
#define MENU_ENTRY_QUALITY_POVRAY_2X_SSAA 1
#define MENU_ENTRY_QUALITY_POVRAY_4X_SSAA 2
#define MENU_ENTRY_QUALITY_POVRAY_8X_SSAA 3
#define MENU_ENTRY_QUALITY_POVRAY_16X_SSAA 4
#define MENU_ENTRY_ROTATE_LEFT 0
#define MENU_ENTRY_ROTATE_RIGHT 1
#define MENU_ENTRY_ROTATE_UP 2
#define MENU_ENTRY_ROTATE_DOWN 3
#define MENU_ENTRY_TRANSLATE_LEFT 4
#define MENU_ENTRY_TRANSLATE_RIGHT 5
#define MENU_ENTRY_TRANSLATE_UP  6
#define MENU_ENTRY_TRANSLATE_DOWN 7
#define MENU_ENTRY_ZOOM_IN 8
#define MENU_ENTRY_ZOOM_OUT 9
#define MENU_ENTRY_NEXT_FRAME 0
#define MENU_ENTRY_PREVIOUS_FRAME 1
#define MENU_ENTRY_PLAY_ALL_FRAMES 2
#define MENU_ENTRY_QUIT_MOLDYN 5

static void moldyn_on_main_menu_(int entry);
static void moldyn_on_export_menu_(int entry);
static void moldyn_on_quality_menu_(int entry);
static void moldyn_on_opengl_quality_menu_(int entry);
static void moldyn_on_povray_quality_menu_(int entry);
static void moldyn_on_navigation_menu_(int entry);
static void moldyn_on_animation_menu_(int entry);


void moldyn_init_glut_menu(void) {
    int main_menu;
    int export_menu;
    int quality_menu;
    int opengl_quality_menu;
    int povray_quality_menu;
    int navigation_menu;
    int animation_menu;
    export_menu = glutCreateMenu(moldyn_on_export_menu_);
    glutAddMenuEntry("PNG image (*.png)", MENU_ENTRY_EXPORT_PNG);
    glutAddMenuEntry("JPEG image (*.jpg)", MENU_ENTRY_EXPORT_JPEG);
    glutAddMenuEntry("POV-Ray scene (.pov)", MENU_ENTRY_EXPORT_POV);
    glutAddMenuEntry("HTML5/WebGL site (*.html)", MENU_ENTRY_EXPORT_HTML);
    opengl_quality_menu = glutCreateMenu(moldyn_on_opengl_quality_menu_);
    glutAddMenuEntry("No SSAA", MENU_ENTRY_QUALITY_OPENGL_NO_SSAA);
    glutAddMenuEntry("2x SSAA", MENU_ENTRY_QUALITY_OPENGL_2X_SSAA);
    glutAddMenuEntry("4x SSAA", MENU_ENTRY_QUALITY_OPENGL_4X_SSAA);
    glutAddMenuEntry("8x SSAA", MENU_ENTRY_QUALITY_OPENGL_8X_SSAA);
    glutAddMenuEntry("16x SSAA", MENU_ENTRY_QUALITY_OPENGL_16X_SSAA);
    povray_quality_menu = glutCreateMenu(moldyn_on_povray_quality_menu_);
    glutAddMenuEntry("No SSAA", MENU_ENTRY_QUALITY_POVRAY_NO_SSAA);
    glutAddMenuEntry("2x SSAA", MENU_ENTRY_QUALITY_POVRAY_2X_SSAA);
    glutAddMenuEntry("4x SSAA", MENU_ENTRY_QUALITY_POVRAY_4X_SSAA);
    glutAddMenuEntry("8x SSAA", MENU_ENTRY_QUALITY_POVRAY_8X_SSAA);
    glutAddMenuEntry("16x SSAA", MENU_ENTRY_QUALITY_POVRAY_16X_SSAA);
    quality_menu = glutCreateMenu(moldyn_on_quality_menu_);
    glutAddSubMenu("OpenGL...",opengl_quality_menu);
    glutAddSubMenu("POV-Ray...",povray_quality_menu);
    navigation_menu = glutCreateMenu(moldyn_on_navigation_menu_);
    glutAddMenuEntry("Rotate left [right]", MENU_ENTRY_ROTATE_LEFT);
    glutAddMenuEntry("Rotate right [left]", MENU_ENTRY_ROTATE_RIGHT);
    glutAddMenuEntry("Rotate up [up]", MENU_ENTRY_ROTATE_UP);
    glutAddMenuEntry("Rotate down [down]", MENU_ENTRY_ROTATE_DOWN);
    glutAddMenuEntry("Move left [shift+left]", MENU_ENTRY_TRANSLATE_LEFT);
    glutAddMenuEntry("Move right [shift+right]", MENU_ENTRY_TRANSLATE_RIGHT);
    glutAddMenuEntry("Move up [shift+up]", MENU_ENTRY_TRANSLATE_UP );
    glutAddMenuEntry("Move down [shift+down]", MENU_ENTRY_TRANSLATE_DOWN);
    glutAddMenuEntry("Zoom in [page up]", MENU_ENTRY_ZOOM_IN);
    glutAddMenuEntry("Zoom out [page down]", MENU_ENTRY_ZOOM_OUT);
    animation_menu = glutCreateMenu(moldyn_on_animation_menu_);
    glutAddMenuEntry("Next frame [n]", MENU_ENTRY_NEXT_FRAME);
    glutAddMenuEntry("Previous frame [b]", MENU_ENTRY_PREVIOUS_FRAME);
    glutAddMenuEntry("Play all frames [m]", MENU_ENTRY_PLAY_ALL_FRAMES);
    main_menu = glutCreateMenu(moldyn_on_main_menu_);
    glutAddMenuEntry("Open File", MENU_ENTRY_OPEN_FILE);
    glutAddSubMenu("Export as...",export_menu);
    glutAddSubMenu("Set export quality...",quality_menu);
    glutAddSubMenu("Navigation...",navigation_menu);
    glutAddSubMenu("Animation...",animation_menu);
    glutAddMenuEntry("Quit moldyn [escape]", MENU_ENTRY_QUIT_MOLDYN);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void moldyn_terminate_glut_menu(void) {
    /* do nothing, the menu will be destroyed together with the window */
}

static void moldyn_on_main_menu_(int entry) {
    switch (entry) {
        case MENU_ENTRY_QUIT_MOLDYN:
            moldyn_exit(0);
            break;
        default:
            break;
    }
}
static void moldyn_on_export_menu_(int entry) {
    switch (entry) {
        case MENU_ENTRY_EXPORT_PNG:
            moldyn_export("scene.png",1024,1024);
            break;
        case MENU_ENTRY_EXPORT_JPEG:
            moldyn_export("scene.jpg",1024,1024);
            break;
        case MENU_ENTRY_EXPORT_POV:
            moldyn_export("scene.pov",1024,1024);
            break;
        case MENU_ENTRY_EXPORT_HTML:
            moldyn_export("scene.html",1024,1024);
            break;
        default:
            break;
    }
}
static void moldyn_on_quality_menu_(int entry) {
    moldyn_log("unknown menu entry");
}

static void moldyn_on_opengl_quality_menu_(int entry) {
    int quality;
    switch (entry) {
        case MENU_ENTRY_QUALITY_OPENGL_NO_SSAA:
            quality = MOLDYN_QUALITY_OPENGL_NO_SSAA;
            break;
        case MENU_ENTRY_QUALITY_OPENGL_2X_SSAA:
            quality = MOLDYN_QUALITY_OPENGL_2X_SSAA;
            break;
        case MENU_ENTRY_QUALITY_OPENGL_4X_SSAA:
            quality = MOLDYN_QUALITY_OPENGL_4X_SSAA;
            break;
        case MENU_ENTRY_QUALITY_OPENGL_8X_SSAA:
            quality = MOLDYN_QUALITY_OPENGL_8X_SSAA;
            break;
        case MENU_ENTRY_QUALITY_OPENGL_16X_SSAA:
            quality = MOLDYN_QUALITY_OPENGL_16X_SSAA;
            break;
        default:
            moldyn_log("unknown menu entry");
            return;
    }
    moldyn_set_export_quality(quality);
}

static void moldyn_on_povray_quality_menu_(int entry) {
    int quality;
    switch (entry) {
        case MENU_ENTRY_QUALITY_POVRAY_NO_SSAA:
            quality = MOLDYN_QUALITY_POVRAY_NO_SSAA;
            break;
        case MENU_ENTRY_QUALITY_POVRAY_2X_SSAA:
            quality = MOLDYN_QUALITY_POVRAY_2X_SSAA;
            break;
        case MENU_ENTRY_QUALITY_POVRAY_4X_SSAA:
            quality = MOLDYN_QUALITY_POVRAY_4X_SSAA;
            break;
        case MENU_ENTRY_QUALITY_POVRAY_8X_SSAA:
            quality = MOLDYN_QUALITY_POVRAY_8X_SSAA;
            break;
        case MENU_ENTRY_QUALITY_POVRAY_16X_SSAA:
            quality = MOLDYN_QUALITY_POVRAY_16X_SSAA;
            break;
        default:
            moldyn_log("unknown menu entry");
            return;
    }
    moldyn_set_export_quality(quality);
}

static void moldyn_on_navigation_menu_(int entry) {
    
}
static void moldyn_on_animation_menu_(int entry) {
    switch (entry) {
        case MENU_ENTRY_NEXT_FRAME:
            moldyn_next_frame();
            moldyn_update_scene();
            glutPostRedisplay();
            break;
        case MENU_ENTRY_PREVIOUS_FRAME:
            moldyn_previous_frame();
            moldyn_update_scene();
            glutPostRedisplay();
            break;
        case MENU_ENTRY_PLAY_ALL_FRAMES:
            moldyn_toggle_animation();
            break;
        default:
            moldyn_log("unknown menu entry");
            break;
    }
}