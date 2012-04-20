#include <stdlib.h>

#include <gr3.h>

#include "moldyn.h"

static int tilt = 0;
static int rotation = 0;

static int export_quality = MOLDYN_QUALITY_OPENGL_2X_SSAA;
static int normal_quality = MOLDYN_QUALITY_OPENGL_NO_SSAA;

static int moldyn_get_gr3_quality_for_moldyn_quality_(int quality);

void moldyn_init_gr3(void) {
    int init_attrs[] = {GR3_IA_FRAMEBUFFER_WIDTH, 1024, 
        GR3_IA_FRAMEBUFFER_HEIGHT, 1024, 
        GR3_IA_END_OF_LIST};
    gr3_setlogcallback(moldyn_log);
    gr3_init(init_attrs);
    /* TODO: use original moldyn's parameters */
    gr3_setcameraprojectionparameters(45,1,200);
    gr3_cameralookat(0,0,14, 0,0,0, 0,1,0);
    gr3_setbackgroundcolor(1,1,1,0);
    moldyn_log(gr3_getrenderpathstring());
    gr3_setquality(normal_quality);
}

void moldyn_terminate_gr3(void) {
    gr3_terminate();
}

void moldyn_update_scene(void) {
    gr3_clear();
    if (num_atoms > 0) {
        gr3_drawspheremesh(num_atoms, atom_positions, atom_colors, atom_radii);
    }
}

void moldyn_export(const char *filename, int width, int height) {
    gr3_setquality(export_quality);
    if (gr3_export(filename, width, height)) {
        moldyn_log("failed to export scene to file!");
    }
    gr3_setquality(normal_quality);
}

int moldyn_drawimage(float xmin, float xmax, float ymin, float ymax, int width, int height, int window) {
    int window_gr3 = 0;
    switch (window) {
        case MOLDYN_WINDOW_OPENGL:
            window_gr3 = GR3_WINDOW_OPENGL;
            break;
        default:
            return 1;
    }
    if (gr3_drawimage(xmin, xmax, ymin, ymax, width, height, window_gr3)) {
        return 1;
    } else {
        return 0;
    }
}

int moldyn_set_export_quality(int quality) {
    int gr3_quality = moldyn_get_gr3_quality_for_moldyn_quality_(quality);
    if (gr3_quality == -1) {
        return 1;
    } else {
        export_quality = gr3_quality;
        return 0;
    }
}

int moldyn_set_normal_quality(int quality) {
    int gr3_quality = moldyn_get_gr3_quality_for_moldyn_quality_(quality);
    if (gr3_quality == -1) {
        return 1;
    } else {
        normal_quality = gr3_quality;
        return 0;
    }
}

static int moldyn_get_gr3_quality_for_moldyn_quality_(int quality) {
    int gr3_quality;
    switch (quality) {
        case MOLDYN_QUALITY_OPENGL_NO_SSAA:
            gr3_quality = GR3_QUALITY_OPENGL_NO_SSAA;
            break;
        case MOLDYN_QUALITY_OPENGL_2X_SSAA:
            gr3_quality = GR3_QUALITY_OPENGL_2X_SSAA;
            break;
        case MOLDYN_QUALITY_OPENGL_4X_SSAA:
            gr3_quality = GR3_QUALITY_OPENGL_4X_SSAA;
            break;
        case MOLDYN_QUALITY_OPENGL_8X_SSAA:
            gr3_quality = GR3_QUALITY_OPENGL_8X_SSAA;
            break;
        case MOLDYN_QUALITY_OPENGL_16X_SSAA:
            gr3_quality = GR3_QUALITY_OPENGL_16X_SSAA;
            break;
        case MOLDYN_QUALITY_POVRAY_NO_SSAA:
            gr3_quality = GR3_QUALITY_POVRAY_NO_SSAA;
            break;
        case MOLDYN_QUALITY_POVRAY_2X_SSAA:
            gr3_quality = GR3_QUALITY_POVRAY_2X_SSAA;
            break;
        case MOLDYN_QUALITY_POVRAY_4X_SSAA:
            gr3_quality = GR3_QUALITY_POVRAY_4X_SSAA;
            break;
        case MOLDYN_QUALITY_POVRAY_8X_SSAA:
            gr3_quality = GR3_QUALITY_POVRAY_8X_SSAA;
            break;
        case MOLDYN_QUALITY_POVRAY_16X_SSAA:
            gr3_quality = GR3_QUALITY_POVRAY_16X_SSAA;
            break;
        default:
            moldyn_log("unknown quality");
            return -1;
    }
    return gr3_quality;
}