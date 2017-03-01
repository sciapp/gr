#include <assert.h>
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

#ifndef FLT_MAX
#define FLT_MAX 1.701411735e+38
#endif

#define arc(angle) (M_PI * (angle) / 180.0)

#define DEFAULT_FIRST_COLOR 8
#define DEFAULT_LAST_COLOR 79

#define OPTION_X_LOG (1 << 0)
#define OPTION_Y_LOG (1 << 1)
#define OPTION_Z_LOG (1 << 2)
#define OPTION_FLIP_X (1 << 3)
#define OPTION_FLIP_Y (1 << 4)
#define OPTION_FLIP_Z (1 << 5)

typedef struct {
  double a1, a2, b, c1, c2, c3, d;
} gr_world_xform_t;

typedef enum
{
  OPTION_LINES, OPTION_MESH, OPTION_FILLED_MESH, OPTION_Z_SHADED_MESH,
  OPTION_COLORED_MESH, OPTION_CELL_ARRAY, OPTION_SHADED_MESH
}
gr_surface_option_t;

typedef struct {
    double a, b;
    int o_log, o_flip;
} trans_t;

int gr3_drawimage_gks_(float xmin, float xmax, float ymin, float ymax, int width, int height) {
  double _xmin = (double) xmin, _xmax = (double) xmax;
  double _ymin = (double) ymin, _ymax = (double) ymax;
  char *pixels;
  int err;
  gr3_log_("gr3_drawimage_gks_();");
  pixels = (char *)malloc(sizeof(int)*width*height);
  if (!pixels) {
    RETURN_ERROR(GR3_ERROR_OUT_OF_MEM);
  }
  err = gr3_getimage(width,height,1,pixels);
  if (err != GR3_ERROR_NONE) {
    free(pixels);
    return err;
  }
  gr_drawimage(_xmin, _xmax, _ymax, _ymin, width, height, (int *)pixels, 0);
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

static double gr3_log10_(double x)
{
    if (x > 0) {
        return log10(x);
    } else {
        return -FLT_MAX;
    }
}

/* create a transformation into NDC coordinates */
static void gr3_ndctrans_(double xmin, double xmax, trans_t *tx,
                          int option_log, int option_flip)
{
    if (option_log) {
        tx->a = 1.0 / gr3_log10_((xmax / xmin));
        tx->b = -gr3_log10_(xmin) * tx->a;
    } else {
        tx->a = 1.0 / (xmax - xmin);
        tx->b = -xmin * tx->a;
    }
    tx->o_log = option_log;
    tx->o_flip = option_flip;
}

static float gr3_transform_(float x, trans_t tx)
{
    if (tx.o_log) {
        x = gr3_log10_(x);
    }
    x = tx.a * x + tx.b;
    if (tx.o_flip) {
        x = 1.0 - x;
    }

    return x;
}

/*!
 * Create a mesh of a surface plot similar to gr_surface.
 * Uses the current colormap. To apply changes of the colormap
 * a new mesh has to be created.
 * \param [out] mesh    the mesh handle
 * \param [in]  nx      number of points in x-direction
 * \param [in]  ny      number of points in y-direction
 * \param [in]  px      an array containing the x-coordinates
 * \param [in]  py      an array containing the y-coordinates
 * \param [in]  pz      an array of length nx * ny containing
 *                      the z-coordinates
 * \param [in]  option  option for the surface mesh; the GR3_SURFACE
 *                      constants can be combined with bitwise or.
 */
GR3API int gr3_createsurfacemesh(int *mesh, int nx, int ny,
                                 float *px, float *py, float *pz,
                                 int option)
{
    double xmin, xmax, ymin, ymax, zmin, zmax;
    int rotation, tilt;
    int i, j;
    int num_vertices;
    float *vertices, *normals, *colors;
    int num_indices;
    int *indices;
    int result;
    int scale;
    int cmap;
    int  first_color = DEFAULT_FIRST_COLOR, last_color = DEFAULT_LAST_COLOR;
    trans_t tx, ty, tz;

    num_vertices = nx * ny;
    vertices = malloc(num_vertices * 3 * sizeof(float));
    if (!vertices) {
        RETURN_ERROR(GR3_ERROR_OUT_OF_MEM);
    }
    normals = malloc(num_vertices * 3 * sizeof(float));
    if (!normals) {
        free(vertices);
        RETURN_ERROR(GR3_ERROR_OUT_OF_MEM);
    }
    colors = malloc(num_vertices * 3 * sizeof(float));
    if (!colors) {
        free(vertices);
        free(normals);
        RETURN_ERROR(GR3_ERROR_OUT_OF_MEM);
    }
    num_indices = (nx - 1) * (ny - 1) * 6; /* 2 triangles per square */
    indices = malloc(num_indices * sizeof(int));
    if (!indices) {
        free(vertices);
        free(normals);
        free(colors);
        RETURN_ERROR(GR3_ERROR_OUT_OF_MEM);
    }

    if (option & GR3_SURFACE_GRTRANSFORM) {
        gr_inqwindow(&xmin, &xmax, &ymin, &ymax);
        gr_inqspace(&zmin, &zmax, &rotation, &tilt);
        gr_inqscale(&scale);
    } else {
        xmin = px[0];
        xmax = px[nx - 1];
        ymin = py[0];
        ymax = py[ny - 1];
        zmin = pz[0];
        zmax = pz[0];
        for (i = 1; i < nx * ny; i++) {
            if (pz[i] < zmin) zmin = pz[i];
            if (pz[i] > zmax) zmax = pz[i];
        }
        scale = 0;
    }
    if (option & (GR3_SURFACE_GRCOLOR | GR3_SURFACE_GRZSHADED)) {
        gr_inqcolormap(&cmap);
        if (abs(cmap) >= 100) {
            first_color = 1000;
            last_color = 1255;
        } else {
            first_color = DEFAULT_FIRST_COLOR;
            last_color = DEFAULT_LAST_COLOR;
        }
    }


    gr3_ndctrans_(xmin, xmax, &tx, scale & OPTION_X_LOG,
                  scale & OPTION_FLIP_X);
    /* flip because y-axis is projected to the negative z-axis */
    gr3_ndctrans_(ymin, ymax, &ty, scale & OPTION_Y_LOG,
                  !(scale & OPTION_FLIP_Y));
    gr3_ndctrans_(zmin, zmax, &tz, scale & OPTION_Z_LOG,
                  scale & OPTION_FLIP_Z);

    for (j = 0; j < ny; j++) {
        for (i = 0; i < nx; i++) {
            int k = j * nx + i;
            float *v = vertices + 3 * k;
            float *n = normals + 3 * k;
            float *c = colors + 3 * k;
            float zvalue;

            v[0] = gr3_transform_(px[i], tx);
            zvalue = gr3_transform_(pz[k], tz);
            if (option & GR3_SURFACE_FLAT) {
                v[1] = 0.0f;
            } else {
                v[1] = zvalue;
            }
            v[2] = gr3_transform_(py[j], ty);

            if (option & GR3_SURFACE_FLAT || !(option & GR3_SURFACE_NORMALS)) {
                n[0] = 0.0f;
                n[1] = 1.0f;
                n[2] = 0.0f;
            }

            if (option & (GR3_SURFACE_GRCOLOR | GR3_SURFACE_GRZSHADED)) {
                int color, rgb;

                if (option & GR3_SURFACE_GRZSHADED)
                    color = (int) pz[k] + first_color;
                else
                    color = (int) (zvalue * (last_color - first_color)
                                   + first_color);
                if (color < first_color)
                    color = first_color;
                else if (color > last_color)
                    color = last_color;

                gr_inqcolor(color, &rgb);
                c[0] = (float) ( rgb        & 0xff) / 255;
                c[1] = (float) ((rgb >>  8) & 0xff) / 255;
                c[2] = (float) ((rgb >> 16) & 0xff) / 255;
            } else {
                c[0] = 1.0;
                c[1] = 1.0;
                c[2] = 1.0;
            }
        }
    }

    /* interpolate normals from the gradient */
    if (option & GR3_SURFACE_NORMALS && !(option & GR3_SURFACE_FLAT)) {
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

    /* create triangles */
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
 * Draw a mesh with the projection of gr. It uses the current
 * projection parameters (rotation, tilt) of gr.
 * This function alters the projection type, the projection parameters,
 * the viewmatrix and the light direction. If necessary, the user has to
 * save them before the call to this function and restore them after
 * the call to gr3_drawimage.
 * \param [in] mesh       the mesh to be drawn
 * \param [in] n          the number of meshes to be drawn
 * \param [in] positions  the positions where the meshes should be drawn
 * \param [in] directions the forward directions the meshes should be
 *                        facing at
 * \param [in] ups        the up directions
 * \param [in] colors     the colors the meshes should be drawn in,
 *                        it will be multiplied with each vertex color
 * \param [in] scales     the scaling factors
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
    if (gr3_geterror(0, NULL, NULL)) return;
    gr3_setcameraprojectionparameters(90.0f, 1.0f, 200.0f);
    if (gr3_geterror(0, NULL, NULL)) return;
    gr3_setlightdirection(0.0f, 1.0f, 0.0f);
    if (gr3_geterror(0, NULL, NULL)) return;

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
 * \param [in] mesh the mesh to be drawn
 */
GR3API void gr3_drawsurface(int mesh)
{
    float positions[3] = {-1.0f, -1.0f, -1.0f};
    float directions[3] = {0.0f, 0.0f, 1.0f};
    float ups[3] = {0.0f, 1.0f, 0.0f};
    float colors[3] = {1.0f, 1.0f, 1.0f};
    float scales[3] = {2.0f, 2.0f, 2.0f};

    gr3_setbackgroundcolor(1.0f, 1.0f, 1.0f, 0.0f);
    if (gr3_geterror(0, NULL, NULL)) return;
    gr3_clear();
    if (gr3_geterror(0, NULL, NULL)) return;
    gr3_drawmesh_grlike(mesh, 1, positions, directions, ups, colors, scales);
    if (gr3_geterror(0, NULL, NULL)) return;
}

/*!
 * Create a surface plot with gr3 and draw it with gks as cellarray
 * \param [in]  nx      number of points in x-direction
 * \param [in]  ny      number of points in y-direction
 * \param [in]  px      an array containing the x-coordinates
 * \param [in]  py      an array containing the y-coordinates
 * \param [in]  pz      an array of length nx * ny containing
 *                      the z-coordinates
 * \param [in]  option  see the option parameter of gr_surface
 */
GR3API void gr3_surface(int nx, int ny, float *px, float *py, float *pz,
                        int option)
{
    if (option == OPTION_Z_SHADED_MESH || option == OPTION_COLORED_MESH) {
        int mesh;
        double xmin, xmax, ymin, ymax;
        int scale;
        int surfaceoption;

        surfaceoption = GR3_SURFACE_GRTRANSFORM;
        if (option == OPTION_Z_SHADED_MESH) {
            surfaceoption |= GR3_SURFACE_GRZSHADED;
        } else {
            surfaceoption |= GR3_SURFACE_GRCOLOR;
        }
        gr3_createsurfacemesh(&mesh, nx, ny, px, py, pz, surfaceoption);
        if (gr3_geterror(0, NULL, NULL)) return;
        gr3_drawsurface(mesh);
        if (gr3_geterror(0, NULL, NULL)) return;
        gr3_deletemesh(mesh);
        if (gr3_geterror(0, NULL, NULL)) return;
        gr_inqwindow(&xmin, &xmax, &ymin, &ymax);
        gr_inqscale(&scale);
        if (scale & OPTION_FLIP_X) {
            double tmp = xmin;
            xmin = xmax;
            xmax = tmp;
        }
        if (scale & OPTION_FLIP_Y) {
            double tmp = ymin;
            ymin = ymax;
            ymax = tmp;
        }
        /* TODO: inquire the required resolution */
        gr3_drawimage((float) xmin, (float) xmax, (float) ymin, (float) ymax, 500, 500, GR3_DRAWABLE_GKS);
        if (gr3_geterror(0, NULL, NULL)) return;
    } else {
        double *dpx, *dpy, *dpz;
        int i;

        dpx = malloc(nx * sizeof(double));
        dpy = malloc(ny * sizeof(double));
        dpz = malloc(nx * ny * sizeof(double));
        if (dpx != NULL && dpy != NULL && dpz != NULL) {
            for (i = 0; i < nx; i++) {
                dpx[i] = (double) px[i];
            }
            for (i = 0; i < ny; i++) {
                dpy[i] = (double) py[i];
            }
            for (i = 0; i < nx * ny; i++) {
                dpz[i] = (double) pz[i];
            }
            gr_surface(nx, ny, dpx, dpy, dpz, option);
        }
        free(dpz);
        free(dpy);
        free(dpx);
    }
}


/*!
 * Draw the given triangles using the current GR colormap
 * \param [in]  n          number of triangles
 * \param [in]  triangles  pointer to an array of 3*3*n float values
 */
GR3API void gr3_drawtrianglesurface(int n, const float *positions) {
    int i;
    int j;
    int mesh;
    int scale;
    double z_min;
    double z_max;
    struct {
        double x_min;
        double x_max;
        double y_min;
        double y_max;
    } window;
    float *normals;
    float *colors;
    if (n < 1) {
        return;
    }
    z_min = positions[2];
    z_max = positions[2];
    for (i = 0; i < n; i++) {
        for (j = 0; j < 3; j++) {
            if (z_min > positions[(i*3+j)*3+2]) {
                z_min = positions[(i*3+j)*3+2];
            }
            if (z_max < positions[(i*3+j)*3+2]) {
                z_max = positions[(i*3+j)*3+2];
            }
        }
    }
    if (z_min == z_max) {
        /* if all z are equal, use the central color of the colormap */
        z_max += 0.5;
        z_min -= 0.5;
    }

    normals = (float *)malloc(n * 3 * 3 * sizeof(float));
    colors = (float *)malloc(n * 3 * 3 * sizeof(float));
    assert(positions);
    assert(normals);
    assert(colors);
    for (i = 0; i < n; i++) {
        for (j = 0; j < 3; j++) {
            int color;
            int rgb;
            /* light direction in gr3_drawmesh_grlike() is fixed to (0, 1, 0) */
            normals[(i*3+j)*3+0] = 0;
            normals[(i*3+j)*3+1] = 1;
            normals[(i*3+j)*3+2] = 0;
            color = 1000 + 255 * (positions[(i*3+j)*3+2] - z_min) / (z_max - z_min);
            gr_inqcolor(color, &rgb);
            colors[(i*3+j)*3+0] = (float) ( rgb        & 0xff) / 255;
            colors[(i*3+j)*3+1] = (float) ((rgb >>  8) & 0xff) / 255;
            colors[(i*3+j)*3+2] = (float) ((rgb >> 16) & 0xff) / 255;
        }
    }
    mesh = 0;
    gr3_createmesh(&mesh, 3 * n, positions, normals, colors);
    free(normals);
    free(colors);

    if (gr3_geterror(0, NULL, NULL)) return;
    gr3_drawsurface(mesh);
    if (gr3_geterror(0, NULL, NULL)) return;
    gr3_deletemesh(mesh);
    if (gr3_geterror(0, NULL, NULL)) return;
    gr_inqwindow(&window.x_min, &window.x_max, &window.y_min, &window.y_max);
    scale = 0;
    gr_inqscale(&scale);
    if (scale & OPTION_FLIP_X) {
        double tmp = window.x_min;
        window.x_min = window.x_max;
        window.x_max = tmp;
    }
    if (scale & OPTION_FLIP_Y) {
        double tmp = window.y_min;
        window.y_min = window.y_max;
        window.y_max = tmp;
    }

    /* TODO: inquire the required resolution */
    gr3_drawimage((float)window.x_min, (float)window.x_max, (float)window.y_min, (float)window.y_max, 500, 500, GR3_DRAWABLE_GKS);
    if (gr3_geterror(0, NULL, NULL)) return;
}
