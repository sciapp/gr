#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "gr.h"
#include "gr3.h"
#include "gr3_internals.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define arc(angle) (M_PI * (angle) / 180.0)

#define FIRST_COLOR 8
#define LAST_COLOR 79

typedef struct {
  double a1, a2, b, c1, c2, c3, d;
} gr_world_xform_t;

typedef enum
{
  OPTION_LINES, OPTION_MESH, OPTION_FILLED_MESH, OPTION_Z_SHADED_MESH,
  OPTION_COLORED_MESH, OPTION_CELL_ARRAY, OPTION_SHADED_MESH
}
gr_surface_option_t;

int gr3_drawimage_gks_(float xmin, float xmax, float ymin, float ymax, int width, int height) {
  double _xmin = (double) xmin, _xmax = (double) xmax;
  double _ymin = (double) ymin, _ymax = (double) ymax;
  char *pixels;
  int err;
  gr3_log_("gr3_drawimage_gks_();");
  pixels = (char *)malloc(sizeof(int)*width*height);
  if (!pixels) {
    return GR3_ERROR_OUT_OF_MEM;
  }
  err = gr3_getimage(width,height,1,pixels);
  if (err != GR3_ERROR_NONE) {
    free(pixels);
    return err;
  }
  gr_drawimage(_xmin, _xmax, _ymax, _ymin, width, height, (int *)pixels);
  free(pixels);
  return GR3_ERROR_NONE;
}

static void gr3_matmul_(float *a, float *b)
{
    int i, j, k;
    float row[4];

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            row[j] = a[i + j * 4];
        }
        for (j = 0; j < 4; j++) {
            a[i + j * 4] = 0.0;
            for (k = 0; k < 4; k++) {
                a[i + j * 4] += row[k] * b[k + j * 4];
            }
        }
    }
}

static void gr3_identity_(float *a)
{
    int i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < i; j++) {
            a[j + i * 4] = 0.0f;
        }
        a[i + i * 4] = 1.0;
        for (j = i + 1; j < 4; j++) {
            a[j + i * 4] = 0.0f;
        }
    }
}

static void gr3_crossprod_(float *c, float *a, float *b)
{
    c[0] = a[1] * b[2] - a[2] * b[1];
    c[1] = a[2] * b[0] - a[0] * b[2];
    c[2] = a[0] * b[1] - a[1] * b[0];
}

static float gr3_norm_(float *v)
{
    int i;
    float s;

    s = 0.0f;
    for (i = 0; i < 3; i++) {
        s += v[i] * v[i];
    }
    return sqrtf(s);
}

static void gr3_normalize_(float *v)
{
    double s;

    s = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];

    if (s > 0.0) {
        s = sqrt(s);
        v[0] /= s;
        v[1] /= s;
        v[2] /= s;
    }
}

static gr_world_xform_t gr3_grworldxform_(int rotation, int tilt)
{
    gr_world_xform_t wx;
    double xmin, xmax, ymin, ymax, zmin, zmax;
    double r, t, a, c;

    xmin = -0.5;
    xmax = 0.5;
    ymin = -0.5;
    ymax = 0.5;
    zmin = -0.5;
    zmax = 0.5;

    r = arc(rotation);
    wx.a1 = cos(r);
    wx.a2 = sin(r);

    a = (xmax - xmin) / (wx.a1 + wx.a2);
    wx.b = xmin;

    wx.a1 = a * wx.a1 / (xmax - xmin);
    wx.a2 = a * wx.a2 / (ymax - ymin);
    wx.b = wx.b - wx.a1 * xmin - wx.a2 * ymin;

    t = arc(tilt);
    wx.c1 = (pow(cos(r), 2.) - 1.) * tan(t / 2.);
    wx.c2 = -(pow(sin(r), 2.) - 1.) * tan(t / 2.);
    wx.c3 = cos(t);

    c = (ymax - ymin) / (wx.c2 + wx.c3 - wx.c1);
    wx.d = ymin - c * wx.c1;

    wx.c1 = c * wx.c1 / (xmax - xmin);
    wx.c2 = c * wx.c2 / (ymax - ymin);
    wx.c3 = c * wx.c3 / (zmax - zmin);
    wx.d = wx.d - wx.c1 * xmin - wx.c2 * ymin - wx.c3 * zmin;

    return wx;
}

static void gr3_grtransformation_(float *a, int rotation, int tilt)
{
    gr_world_xform_t wx;
    float row0[3], row1[3], row2[3];
    int i;

    wx = gr3_grworldxform_(rotation, tilt);
    row0[0] = wx.a1;
    row0[1] = 0.0f;
    row0[2] = -wx.a2;
    row1[0] = wx.c1;
    row1[1] = wx.c3;
    row1[2] = -wx.c2;
    gr3_crossprod_(row2, row0, row1);

    gr3_identity_(a);
    for (i = 0; i < 3; i++) {
        a[0 + i * 4] = row0[i];
        a[1 + i * 4] = row1[i];
        a[2 + i * 4] = row2[i];
    }
}

/*!
 * Create a mesh of a surface plot similar to gr_surface.
 * Uses the current colormap. To apply changes of the colormap
 * a new mesh has to be created.
 */
GR3API int gr3_createsurfacemesh(int *mesh, int nx, int ny,
                                 float *px, float *py, float *pz,
                                 int surface, int option)
{
    float xrange[2], yrange[2];
    double zmin, zmax;
    int rotation, tilt;
    int i, j;
    int num_vertices;
    float *vertices, *normals, *colors;
    int num_indices;
    int *indices;
    int result;

    num_vertices = nx * ny;
    vertices = malloc(num_vertices * 3 * sizeof(float));
    if (!vertices) {
        return GR3_ERROR_OUT_OF_MEM;
    }
    normals = malloc(num_vertices * 3 * sizeof(float));
    if (!normals) {
        free(vertices);
        return GR3_ERROR_OUT_OF_MEM;
    }
    colors = malloc(num_vertices * 3 * sizeof(float));
    if (!colors) {
        free(vertices);
        free(normals);
        return GR3_ERROR_OUT_OF_MEM;
    }
    num_indices = (nx - 1) * (ny - 1) * 6; /* 2 triangles per square */
    indices = malloc(num_indices * sizeof(int));
    if (!indices) {
        free(vertices);
        free(normals);
        free(colors);
        return GR3_ERROR_OUT_OF_MEM;
    }

    xrange[0] = px[0];
    xrange[1] = px[nx - 1];
    yrange[0] = py[0];
    yrange[1] = py[ny - 1];
    gr_inqspace(&zmin, &zmax, &rotation, &tilt);

    for (j = 0; j < ny; j++) {
        for (i = 0; i < nx; i++) {
            int k = j * nx + i;
            float *v = vertices + 3 * k;
            float *n = normals + 3 * k;
            float *c = colors + 3 * k;

            v[0] = ((px[i] - xrange[0]) / (xrange[1] - xrange[0]));
            if (surface & GR3_SURFACE_FLAT) {
                v[1] = 0.0f;
            } else {
                v[1] = ((pz[k] - zmin) / (zmax - zmin));
            }
            /* flip because y-axis is projected to the negative z-axis */
            v[2] = ((yrange[1] - py[j]) / (yrange[1] - yrange[0]));

            if (surface & GR3_SURFACE_FLAT || !(surface & GR3_SURFACE_NORMAL)) {
                n[0] = 0.0f;
                n[1] = 1.0f;
                n[2] = 0.0f;
            }

            if (surface & GR3_SURFACE_NOCOLOR) {
                c[0] = 1.0;
                c[1] = 1.0;
                c[2] = 1.0;
            } else {
                int color, rgb;

                if (option == OPTION_Z_SHADED_MESH)
                    color = (int) pz[k] + FIRST_COLOR;
                else
                    color = (int) ((pz[k] - zmin) / (zmax - zmin)
                                   * (LAST_COLOR - FIRST_COLOR) + FIRST_COLOR);
                if (color < FIRST_COLOR)
                    color = FIRST_COLOR;
                else if (color > LAST_COLOR)
                    color = LAST_COLOR;

                gr_inqcolor(color, &rgb);
                c[0] = (float) ( rgb        & 0xff) / 255;
                c[1] = (float) ((rgb >>  8) & 0xff) / 255;
                c[2] = (float) ((rgb >> 16) & 0xff) / 255;
            }
        }
    }

    /* interpolate normals */
    if (surface & GR3_SURFACE_NORMAL && !(surface & GR3_SURFACE_FLAT)) {
        int dirx = 3, diry = 3 * nx;

        for (j = 0; j < ny; j++) {
            for (i = 0; i < nx; i++) {
                int k = j * nx + i;
                float *v = vertices + 3 * k;
                float *n = normals + 3 * k;
                float dx, dy;

                if (i == 0) {
                    dx = (v[dirx + 1] - v[1]) / (v[dirx + 0] - v[0]);
                } else if (i == nx - 1) {
                    dx = (v[1] - v[-dirx + 1]) / (v[0] - v[-dirx + 0]);
                } else {
                    dx = ((v[1] - v[-dirx + 1]) / (v[0] - v[-dirx + 0])
                         + (v[dirx + 1] - v[1]) / (v[dirx + 0] - v[0]))
                          / 2.0;
                }

                if (j == 0) {
                    dy = (v[diry + 1] - v[1]) / (v[diry + 2] - v[2]);
                } else if (j == ny - 1) {
                    dy = (v[1] - v[-diry + 1]) / (v[2] - v[-diry + 2]);
                } else {
                    dy = ((v[1] - v[-diry + 1]) / (v[2] - v[-diry + 2])
                         + (v[diry + 1] - v[1]) / (v[diry + 2] - v[2]))
                          / 2.0;
                }

                n[0] = -dx;
                n[1] = 1.0f;
                n[2] = -dy;
                gr3_normalize_(n);
            }
        }
    }

    for (j = 0; j < ny - 1; j++) {
        for (i = 0; i < nx - 1; i++) {
            int k = j * nx + i;
            int *idx = indices + 6 * (j * (nx - 1) + i);

            idx[0] = k;
            idx[1] = k + 1;
            idx[2] = k + nx;
            idx[3] = k + nx;
            idx[4] = k + 1;
            idx[5] = k + nx + 1;
        }
    }

    result = gr3_createindexedmesh_nocopy(mesh, num_vertices, vertices,
                                          normals, colors, num_indices,
                                          indices);
    if (result != GR3_ERROR_NONE && result != GR3_ERROR_OPENGL_ERR) {
        free(indices);
        free(colors);
        free(normals);
        free(vertices);
    }

    return result;
}

/*!
 * Draw a mesh with the current projection parameters (rotation, tilt).
 * This function alters the projection type, the projection parameters,
 * the viewmatrix and the light direction. If necessary, the user has to
 * save them before the call to this function and restore them after
 * the call to gr3_drawimage.
 */
GR3API void gr3_drawmesh_grlike(int mesh, int n, const float *positions,
                                const float *directions, const float *ups,
                                const float *colors, const float *scales)
{
    double zmin, zmax;
    int rotation, tilt;
    float grmatrix[16], grviewmatrix[16];
    float grscales[4];
    float *modelscales, *modelpos;
    int i, j;

    gr3_setprojectiontype(GR3_PROJECTION_PARALLEL);
    gr3_setcameraprojectionparameters(90.0f, 1.0f, 200.0f);
    gr3_setlightdirection(0.0f, 1.0f, 0.0f);

    gr_inqspace(&zmin, &zmax, &rotation, &tilt);
    gr3_grtransformation_(grmatrix, rotation, tilt);

    /* extract non-uniform scaling */
    for (i = 0; i < 3; i++) {
        grscales[i] = gr3_norm_(&grmatrix[i * 4]);
        for (j = 0; j < 4; j++) {
            grmatrix[j + i * 4] /= grscales[i];
        }
    }
    grscales[3] = 1.0f;

    /* translate */
    gr3_identity_(grviewmatrix);
    grviewmatrix[2 + 3 * 4] = -4;
    gr3_matmul_(grviewmatrix, grmatrix);
    gr3_setviewmatrix(grviewmatrix);

    /* apply the scaling where it does not affect the normals */
    modelpos = malloc(n * 3 * sizeof(float));
    modelscales = malloc(n * 3 * sizeof(float));
    for (i = 0; i < n; i++) {
        for (j = 0; j < 3; j++) {
            modelscales[i * 3 + j] = scales[i * 3 + j] * grscales[j];
            modelpos[i * 3 + j] = positions[i * 3 + j] * grscales[j];
        }
    }
    gr3_drawmesh(mesh, n, modelpos, directions, ups, colors, modelscales);
    free(modelpos);
    free(modelscales);
}

/*!
 * Convenience function for drawing a surfacemesh
 */
GR3API void gr3_drawsurface(int mesh)
{
    float positions[3] = {-1.0f, -1.0f, -1.0f};
    float directions[3] = {0.0f, 0.0f, 1.0f};
    float ups[3] = {0.0f, 1.0f, 0.0f};
    float colors[3] = {1.0f, 1.0f, 1.0f};
    float scales[3] = {2.0f, 2.0f, 2.0f};

    gr3_setbackgroundcolor(1.0f, 1.0f, 1.0f, 0.0f);
    gr3_clear();
    gr3_drawmesh_grlike(mesh, 1, positions, directions, ups, colors, scales);
}

GR3API void gr3_surface(int nx, int ny, float *px, float *py, float *pz,
                        int option)
{
    int mesh;
    double xmin, xmax, ymin, ymax;

    gr3_createsurfacemesh(&mesh, nx, ny, px, py, pz, 0, option);
    gr3_drawsurface(mesh);
    gr3_deletemesh(mesh);
    gr_inqwindow(&xmin, &xmax, &ymin, &ymax);
    /* TODO: inquire the required resolution */
    gr3_drawimage((float) xmin, (float) xmax, (float) ymin, (float) xmax, 500, 500, GR3_DRAWABLE_GKS);
}

