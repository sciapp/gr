#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "gr3.h"
#include "gr.h"

#define SLICE_OFFSET 0.001

static float colormap[768];
static float *getcolormap() {
    int i;
    for (i = 0; i < 255; i++) {
        int packed_color = 0;
        gr_inqcolor(1000+i, &packed_color);
        colormap[3*i+0] = (packed_color & 0xff)/255.0;
        colormap[3*i+1] = ((packed_color >> 8) & 0xff)/255.0;
        colormap[3*i+2] = ((packed_color >> 16) & 0xff)/255.0;
    }
    return &(colormap[0]);
}

GR3API void gr3_createxslicemesh(int *mesh, const GR3_MC_DTYPE *data, unsigned int ix,
                                 unsigned int dim_x, unsigned int dim_y,
                                 unsigned int dim_z, unsigned int stride_x,
                                 unsigned int stride_y, unsigned int stride_z,
                                 double step_x, double step_y, double step_z,
                                 double offset_x, double offset_y, double offset_z) {
    float dtype_max = (1UL << (8*sizeof(GR3_MC_DTYPE)))-1;
    float *colormap = getcolormap();
    unsigned int iy;
    unsigned int iz;
    unsigned int number_of_vertices = dim_z * dim_y * 2;
    unsigned int number_of_indices = (dim_z-1)*(dim_y-1) * 2 * 2 * 3;
    float *vertices = (float *)malloc(number_of_vertices * 3 * sizeof(float));
    float *normals = (float *)malloc(number_of_vertices * 3 * sizeof(float));
    float *colors = (float *)malloc(number_of_vertices * 3 * sizeof(float));
    int *indices = (int *)malloc(number_of_indices * sizeof(int));
    assert(vertices);
    assert(normals);
    assert(colors);
    assert(indices);
    
    if (ix >= dim_x) {
        ix = dim_x-1;
    }
    
    for (iz = 0; iz < dim_z; iz++) {
        for (iy = 0; iy < dim_y; iy++) {
            float value = data[ix*stride_x+iy*stride_y+iz*stride_z]/dtype_max;
            int color_index1 = floor(value*255);
            int color_index2 = ceil(value*255);
            float alpha = 1-(value*255-color_index1);
            float red = colormap[color_index1*3+0]*alpha+colormap[color_index2*3+0]*(1-alpha);
            float green = colormap[color_index1*3+1]*alpha+colormap[color_index2*3+1]*(1-alpha);
            float blue = colormap[color_index1*3+2]*alpha+colormap[color_index2*3+2]*(1-alpha);
            vertices[(iz*dim_y+iy)*3+0] = ix*step_x+offset_x-SLICE_OFFSET;
            vertices[(iz*dim_y+iy)*3+1] = iy*step_y+offset_y;
            vertices[(iz*dim_y+iy)*3+2] = iz*step_z+offset_z;
            normals[(iz*dim_y+iy)*3+0] = -1;
            normals[(iz*dim_y+iy)*3+1] = 0;
            normals[(iz*dim_y+iy)*3+2] = 0;
            colors[(iz*dim_y+iy)*3+0] = red;
            colors[(iz*dim_y+iy)*3+1] = green;
            colors[(iz*dim_y+iy)*3+2] = blue;
            vertices[dim_z*dim_y*3+(iz*dim_y+iy)*3+0] = ix*step_x+offset_x+SLICE_OFFSET;
            vertices[dim_z*dim_y*3+(iz*dim_y+iy)*3+1] = iy*step_y+offset_y;
            vertices[dim_z*dim_y*3+(iz*dim_y+iy)*3+2] = iz*step_z+offset_z;
            normals[dim_z*dim_y*3+(iz*dim_y+iy)*3+0] = 1;
            normals[dim_z*dim_y*3+(iz*dim_y+iy)*3+1] = 0;
            normals[dim_z*dim_y*3+(iz*dim_y+iy)*3+2] = 0;
            colors[dim_z*dim_y*3+(iz*dim_y+iy)*3+0] = red;
            colors[dim_z*dim_y*3+(iz*dim_y+iy)*3+1] = green;
            colors[dim_z*dim_y*3+(iz*dim_y+iy)*3+2] = blue;
        }
    }
    for (iz = 0; iz < dim_z-1; iz++) {
        for (iy = 0; iy < dim_y-1; iy++) {
            indices[(iz*(dim_y-1)+iy)*12+0] = iz*dim_y+iy;
            indices[(iz*(dim_y-1)+iy)*12+1] = iz*dim_y+iy+1;
            indices[(iz*(dim_y-1)+iy)*12+2] = (iz+1)*dim_y+iy;
            indices[(iz*(dim_y-1)+iy)*12+3] = (iz+1)*dim_y+iy;
            indices[(iz*(dim_y-1)+iy)*12+4] = iz*dim_y+iy+1;
            indices[(iz*(dim_y-1)+iy)*12+5] = (iz+1)*dim_y+iy+1;
            indices[(iz*(dim_y-1)+iy)*12+6] = dim_z*dim_y+(iz+1)*dim_y+iy+1;
            indices[(iz*(dim_y-1)+iy)*12+7] = dim_z*dim_y+iz*dim_y+iy+1;
            indices[(iz*(dim_y-1)+iy)*12+8] = dim_z*dim_y+(iz+1)*dim_y+iy;
            indices[(iz*(dim_y-1)+iy)*12+9] = dim_z*dim_y+(iz+1)*dim_y+iy;
            indices[(iz*(dim_y-1)+iy)*12+10] = dim_z*dim_y+iz*dim_y+iy+1;
            indices[(iz*(dim_y-1)+iy)*12+11] = dim_z*dim_y+iz*dim_y+iy;
        }
    }
    gr3_createindexedmesh(mesh, number_of_vertices, vertices, normals, colors, number_of_indices, indices);
}

GR3API void gr3_createyslicemesh(int *mesh, const GR3_MC_DTYPE *data, unsigned int iy,
                                 unsigned int dim_x, unsigned int dim_y,
                                 unsigned int dim_z, unsigned int stride_x,
                                 unsigned int stride_y, unsigned int stride_z,
                                 double step_x, double step_y, double step_z,
                                 double offset_x, double offset_y, double offset_z) {
    float dtype_max = (1UL << (8*sizeof(GR3_MC_DTYPE)))-1;
    float *colormap = getcolormap();
    unsigned int ix;
    unsigned int iz;
    unsigned int number_of_vertices = dim_x * dim_z * 2;
    unsigned int number_of_indices = (dim_x-1)*(dim_z-1) * 2 * 2 * 3;
    float *vertices = (float *)malloc(number_of_vertices * 3 * sizeof(float));
    float *normals = (float *)malloc(number_of_vertices * 3 * sizeof(float));
    float *colors = (float *)malloc(number_of_vertices * 3 * sizeof(float));
    int *indices = (int *)malloc(number_of_indices * sizeof(int));
    assert(vertices);
    assert(normals);
    assert(colors);
    assert(indices);
    
    if (iy >= dim_y) {
        iy = dim_y-1;
    }
    
    for (iz = 0; iz < dim_z; iz++) {
        for (ix = 0; ix < dim_x; ix++) {
            float value = data[ix*stride_x+iy*stride_y+iz*stride_z]/dtype_max;
            int color_index1 = floor(value*255);
            int color_index2 = ceil(value*255);
            float alpha = 1-(value*255-color_index1);
            float red = colormap[color_index1*3+0]*alpha+colormap[color_index2*3+0]*(1-alpha);
            float green = colormap[color_index1*3+1]*alpha+colormap[color_index2*3+1]*(1-alpha);
            float blue = colormap[color_index1*3+2]*alpha+colormap[color_index2*3+2]*(1-alpha);
            vertices[(iz*dim_x+ix)*3+0] = ix*step_x+offset_x;
            vertices[(iz*dim_x+ix)*3+1] = iy*step_y+offset_y+SLICE_OFFSET;
            vertices[(iz*dim_x+ix)*3+2] = iz*step_z+offset_z;
            normals[(iz*dim_x+ix)*3+0] = 0;
            normals[(iz*dim_x+ix)*3+1] = 1;
            normals[(iz*dim_x+ix)*3+2] = 0;
            colors[(iz*dim_x+ix)*3+0] = red;
            colors[(iz*dim_x+ix)*3+1] = green;
            colors[(iz*dim_x+ix)*3+2] = blue;
            vertices[dim_x*dim_z*3+(iz*dim_x+ix)*3+0] = ix*step_x+offset_x;
            vertices[dim_x*dim_z*3+(iz*dim_x+ix)*3+1] = iy*step_y+offset_y-SLICE_OFFSET;
            vertices[dim_x*dim_z*3+(iz*dim_x+ix)*3+2] = iz*step_z+offset_z;
            normals[dim_x*dim_z*3+(iz*dim_x+ix)*3+0] = 0;
            normals[dim_x*dim_z*3+(iz*dim_x+ix)*3+1] = -1;
            normals[dim_x*dim_z*3+(iz*dim_x+ix)*3+2] = 0;
            colors[dim_x*dim_z*3+(iz*dim_x+ix)*3+0] = red;
            colors[dim_x*dim_z*3+(iz*dim_x+ix)*3+1] = green;
            colors[dim_x*dim_z*3+(iz*dim_x+ix)*3+2] = blue;
        }
    }
    for (iz = 0; iz < dim_z-1; iz++) {
        for (ix = 0; ix < dim_x-1; ix++) {
            indices[(iz*(dim_x-1)+ix)*12+0] = iz*dim_x+ix;
            indices[(iz*(dim_x-1)+ix)*12+1] = iz*dim_x+ix+1;
            indices[(iz*(dim_x-1)+ix)*12+2] = (iz+1)*dim_x+ix;
            indices[(iz*(dim_x-1)+ix)*12+3] = (iz+1)*dim_x+ix;
            indices[(iz*(dim_x-1)+ix)*12+4] = iz*dim_x+ix+1;
            indices[(iz*(dim_x-1)+ix)*12+5] = (iz+1)*dim_x+ix+1;
            indices[(iz*(dim_x-1)+ix)*12+6] = dim_x*dim_z+(iz+1)*dim_x+ix+1;
            indices[(iz*(dim_x-1)+ix)*12+7] = dim_x*dim_z+iz*dim_x+ix+1;
            indices[(iz*(dim_x-1)+ix)*12+8] = dim_x*dim_z+(iz+1)*dim_x+ix;
            indices[(iz*(dim_x-1)+ix)*12+9] = dim_x*dim_z+(iz+1)*dim_x+ix;
            indices[(iz*(dim_x-1)+ix)*12+10] = dim_x*dim_z+iz*dim_x+ix+1;
            indices[(iz*(dim_x-1)+ix)*12+11] = dim_x*dim_z+iz*dim_x+ix;
        }
    }
    gr3_createindexedmesh(mesh, number_of_vertices, vertices, normals, colors, number_of_indices, indices);
}

GR3API void gr3_createzslicemesh(int *mesh, const GR3_MC_DTYPE *data, unsigned int iz,
                                 unsigned int dim_x, unsigned int dim_y,
                                 unsigned int dim_z, unsigned int stride_x,
                                 unsigned int stride_y, unsigned int stride_z,
                                 double step_x, double step_y, double step_z,
                                 double offset_x, double offset_y, double offset_z) {
    float dtype_max = (1UL << (8*sizeof(GR3_MC_DTYPE)))-1;
    float *colormap = getcolormap();
    unsigned int ix;
    unsigned int iy;
    unsigned int number_of_vertices = dim_x * dim_y * 2;
    unsigned int number_of_indices = (dim_x-1)*(dim_y-1) * 2 * 2 * 3;
    float *vertices = (float *)malloc(number_of_vertices * 3 * sizeof(float));
    float *normals = (float *)malloc(number_of_vertices * 3 * sizeof(float));
    float *colors = (float *)malloc(number_of_vertices * 3 * sizeof(float));
    int *indices = (int *)malloc(number_of_indices * sizeof(int));
    assert(vertices);
    assert(normals);
    assert(colors);
    assert(indices);
    
    if (iz >= dim_z) {
        iz = dim_z-1;
    }
    
    for (iy = 0; iy < dim_y; iy++) {
        for (ix = 0; ix < dim_x; ix++) {
            float value = data[ix*stride_x+iy*stride_y+iz*stride_z]/dtype_max;
            int color_index1 = floor(value*255);
            int color_index2 = ceil(value*255);
            float alpha = 1-(value*255-color_index1);
            float red = colormap[color_index1*3+0]*alpha+colormap[color_index2*3+0]*(1-alpha);
            float green = colormap[color_index1*3+1]*alpha+colormap[color_index2*3+1]*(1-alpha);
            float blue = colormap[color_index1*3+2]*alpha+colormap[color_index2*3+2]*(1-alpha);
            vertices[(iy*dim_x+ix)*3+0] = ix*step_x+offset_x;
            vertices[(iy*dim_x+ix)*3+1] = iy*step_y+offset_y;
            vertices[(iy*dim_x+ix)*3+2] = iz*step_z+offset_z+SLICE_OFFSET;
            normals[(iy*dim_x+ix)*3+0] = 0;
            normals[(iy*dim_x+ix)*3+1] = 0;
            normals[(iy*dim_x+ix)*3+2] = 1;
            colors[(iy*dim_x+ix)*3+0] = red;
            colors[(iy*dim_x+ix)*3+1] = green;
            colors[(iy*dim_x+ix)*3+2] = blue;
            vertices[dim_x*dim_y*3+(iy*dim_x+ix)*3+0] = ix*step_x+offset_x;
            vertices[dim_x*dim_y*3+(iy*dim_x+ix)*3+1] = iy*step_y+offset_y;
            vertices[dim_x*dim_y*3+(iy*dim_x+ix)*3+2] = iz*step_z+offset_z-SLICE_OFFSET;
            normals[dim_x*dim_y*3+(iy*dim_x+ix)*3+0] = 0;
            normals[dim_x*dim_y*3+(iy*dim_x+ix)*3+1] = 0;
            normals[dim_x*dim_y*3+(iy*dim_x+ix)*3+2] = -1;
            colors[dim_x*dim_y*3+(iy*dim_x+ix)*3+0] = red;
            colors[dim_x*dim_y*3+(iy*dim_x+ix)*3+1] = green;
            colors[dim_x*dim_y*3+(iy*dim_x+ix)*3+2] = blue;
        }
    }
    for (iy = 0; iy < dim_y-1; iy++) {
        for (ix = 0; ix < dim_x-1; ix++) {
            indices[(iy*(dim_x-1)+ix)*12+0] = iy*dim_x+ix;
            indices[(iy*(dim_x-1)+ix)*12+1] = iy*dim_x+ix+1;
            indices[(iy*(dim_x-1)+ix)*12+2] = (iy+1)*dim_x+ix;
            indices[(iy*(dim_x-1)+ix)*12+3] = (iy+1)*dim_x+ix;
            indices[(iy*(dim_x-1)+ix)*12+4] = iy*dim_x+ix+1;
            indices[(iy*(dim_x-1)+ix)*12+5] = (iy+1)*dim_x+ix+1;
            indices[(iy*(dim_x-1)+ix)*12+6] = dim_x*dim_y+(iy+1)*dim_x+ix+1;
            indices[(iy*(dim_x-1)+ix)*12+7] = dim_x*dim_y+iy*dim_x+ix+1;
            indices[(iy*(dim_x-1)+ix)*12+8] = dim_x*dim_y+(iy+1)*dim_x+ix;
            indices[(iy*(dim_x-1)+ix)*12+9] = dim_x*dim_y+(iy+1)*dim_x+ix;
            indices[(iy*(dim_x-1)+ix)*12+10] = dim_x*dim_y+iy*dim_x+ix+1;
            indices[(iy*(dim_x-1)+ix)*12+11] = dim_x*dim_y+iy*dim_x+ix;
        }
    }
    gr3_createindexedmesh(mesh, number_of_vertices, vertices, normals, colors, number_of_indices, indices);
}

GR3API void gr3_drawxslicemesh(const GR3_MC_DTYPE *data, unsigned int ix,
                               unsigned int dim_x, unsigned int dim_y,
                               unsigned int dim_z, unsigned int stride_x,
                               unsigned int stride_y, unsigned int stride_z,
                               double step_x, double step_y, double step_z,
                               double offset_x, double offset_y, double offset_z) {
    int mesh = 0;
    float position[3] = {0, 0, 0};
    float direction[3] = {0, 0, 1};
    float up[3] = {0, 1, 0};
    float color[3] = {1, 1, 1};
    float scales[3] = {1, 1, 1};
    gr3_createxslicemesh(&mesh, data, ix,
                        dim_x, dim_y, dim_z,
                        stride_x, stride_y, stride_z,
                        step_x, step_y, step_z,
                        offset_x, offset_y, offset_z);
    gr3_drawmesh(mesh, 1, position, direction, up, color, scales);
    gr3_deletemesh(mesh);
}

GR3API void gr3_drawyslicemesh(const GR3_MC_DTYPE *data, unsigned int iy,
                               unsigned int dim_x, unsigned int dim_y,
                               unsigned int dim_z, unsigned int stride_x,
                               unsigned int stride_y, unsigned int stride_z,
                               double step_x, double step_y, double step_z,
                               double offset_x, double offset_y, double offset_z) {
    int mesh = 0;
    float position[3] = {0, 0, 0};
    float direction[3] = {0, 0, 1};
    float up[3] = {0, 1, 0};
    float color[3] = {1, 1, 1};
    float scales[3] = {1, 1, 1};
    gr3_createyslicemesh(&mesh, data, iy,
                        dim_x, dim_y, dim_z,
                        stride_x, stride_y, stride_z,
                        step_x, step_y, step_z,
                        offset_x, offset_y, offset_z);
    gr3_drawmesh(mesh, 1, position, direction, up, color, scales);
    gr3_deletemesh(mesh);
}

GR3API void gr3_drawzslicemesh(const GR3_MC_DTYPE *data, unsigned int iz,
                               unsigned int dim_x, unsigned int dim_y,
                               unsigned int dim_z, unsigned int stride_x,
                               unsigned int stride_y, unsigned int stride_z,
                               double step_x, double step_y, double step_z,
                               double offset_x, double offset_y, double offset_z) {
    int mesh = 0;
    float position[3] = {0, 0, 0};
    float direction[3] = {0, 0, 1};
    float up[3] = {0, 1, 0};
    float color[3] = {1, 1, 1};
    float scales[3] = {1, 1, 1};
    gr3_createzslicemesh(&mesh, data, iz,
                        dim_x, dim_y, dim_z,
                        stride_x, stride_y, stride_z,
                        step_x, step_y, step_z,
                        offset_x, offset_y, offset_z);
    gr3_drawmesh(mesh, 1, position, direction, up, color, scales);
    gr3_deletemesh(mesh);
}
