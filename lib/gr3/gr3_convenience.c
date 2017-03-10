
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "gr.h"

#include "gr3.h"
#define DONT_USE_RETURN_ERROR
#include "gr3_internals.h"



static void gr3_createcylindermesh_(void);
static void gr3_createspheremesh_(void);
static void gr3_createconemesh_(void);
static void gr3_createcubemesh_(void);

void gr3_init_convenience(void) {
  gr3_createcylindermesh_();
  gr3_createspheremesh_();
  gr3_createconemesh_();
  gr3_createcubemesh_();
}

static void gr3_createcubemesh_(void) {
  float vertices[108] = {
    -1.0,-1.0,-1.0,
    -1.0,-1.0, 1.0,
    -1.0, 1.0, 1.0,
    
    1.0, 1.0,-1.0,
    -1.0,-1.0,-1.0,
    -1.0, 1.0,-1.0,
    
    1.0,-1.0, 1.0,
    -1.0,-1.0,-1.0,
    1.0,-1.0,-1.0,
    
    1.0, 1.0,-1.0,
    1.0,-1.0,-1.0,
    -1.0,-1.0,-1.0,
    
    -1.0,-1.0,-1.0,
    -1.0, 1.0, 1.0,
    -1.0, 1.0,-1.0,
    
    1.0,-1.0, 1.0,
    -1.0,-1.0, 1.0,
    -1.0,-1.0,-1.0,
    
    -1.0, 1.0, 1.0,
    -1.0,-1.0, 1.0,
    1.0,-1.0, 1.0,
    
    1.0, 1.0, 1.0,
    1.0,-1.0,-1.0,
    1.0, 1.0,-1.0,
    
    1.0,-1.0,-1.0,
    1.0, 1.0, 1.0,
    1.0,-1.0, 1.0,
    
    1.0, 1.0, 1.0,
    1.0, 1.0,-1.0,
    -1.0, 1.0,-1.0,
    
    1.0, 1.0, 1.0,
    -1.0, 1.0,-1.0,
    -1.0, 1.0, 1.0,
    
    1.0, 1.0, 1.0,
    -1.0, 1.0, 1.0,
    1.0,-1.0, 1.0
  };
  float normals[108] = {
    -1.0, 0.0, 0.0,
    -1.0, 0.0, 0.0,
    -1.0, 0.0, 0.0,
    0.0, 0.0, -1.0,
    0.0, 0.0, -1.0,
    0.0, 0.0, -1.0,
    0.0, -1.0, 0.0,
    0.0, -1.0, 0.0,
    0.0, -1.0, 0.0,
    0.0, 0.0, -1.0,
    0.0, 0.0, -1.0,
    0.0, 0.0, -1.0,
    -1.0, 0.0, 0.0,
    -1.0, 0.0, 0.0,
    -1.0, 0.0, 0.0,
    0.0, -1.0, 0.0,
    0.0, -1.0, 0.0,
    0.0, -1.0, 0.0,
    0.0, 0.0, 1.0,
    0.0, 0.0, 1.0,
    0.0, 0.0, 1.0,
    1.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0,
    0.0, 0.0, 1.0,
    0.0, 0.0, 1.0
  };
  float colors[108] = {
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0
  };
  gr3_createmesh(&context_struct_.cube_mesh, 36, vertices, normals, colors);
}

GR3API void gr3_drawcubemesh(int n, const float *positions,
                         const float *directions, const float *ups,
                             const float *colors, const float *scales) {
  GR3_DO_INIT;
  gr3_drawmesh(context_struct_.cube_mesh, n, positions, directions, ups, colors, scales);
}

/*!
 * This function creates the context_struct_.cylinder_mesh for simple drawing
 */
static void gr3_createcylindermesh_(void) {
  int i;
  int n;
  float *vertices;
  float *normals;
  float *colors;
  int num_sides = 36;
  n = 12*num_sides;
  vertices = malloc(n*3*sizeof(float));
  normals = malloc(n*3*sizeof(float));
  colors = malloc(n*3*sizeof(float));
  for (i = 0; i < num_sides; i++) {
    vertices[(12*i+0)*3+0] = cos(M_PI*360/num_sides*i/180);
    vertices[(12*i+0)*3+1] = sin(M_PI*360/num_sides*i/180);
    vertices[(12*i+0)*3+2] = 0;
    vertices[(12*i+1)*3+0] = cos(M_PI*360/num_sides*(i+1)/180);
    vertices[(12*i+1)*3+1] = sin(M_PI*360/num_sides*(i+1)/180);
    vertices[(12*i+1)*3+2] = 0;
    vertices[(12*i+2)*3+0] = cos(M_PI*360/num_sides*i/180);
    vertices[(12*i+2)*3+1] = sin(M_PI*360/num_sides*i/180);
    vertices[(12*i+2)*3+2] = 1;
    
    normals[(12*i+0)*3+0] = cos(M_PI*360/num_sides*i/180);
    normals[(12*i+0)*3+1] = sin(M_PI*360/num_sides*i/180);
    normals[(12*i+0)*3+2] = 0;
    normals[(12*i+1)*3+0] = cos(M_PI*360/num_sides*(i+1)/180);
    normals[(12*i+1)*3+1] = sin(M_PI*360/num_sides*(i+1)/180);
    normals[(12*i+1)*3+2] = 0;
    normals[(12*i+2)*3+0] = cos(M_PI*360/num_sides*i/180);
    normals[(12*i+2)*3+1] = sin(M_PI*360/num_sides*i/180);
    normals[(12*i+2)*3+2] = 0;
    
    
    vertices[(12*i+3)*3+0] = cos(M_PI*360/num_sides*i/180);
    vertices[(12*i+3)*3+1] = sin(M_PI*360/num_sides*i/180);
    vertices[(12*i+3)*3+2] = 1;
    vertices[(12*i+4)*3+0] = cos(M_PI*360/num_sides*(i+1)/180);
    vertices[(12*i+4)*3+1] = sin(M_PI*360/num_sides*(i+1)/180);
    vertices[(12*i+4)*3+2] = 0;
    vertices[(12*i+5)*3+0] = cos(M_PI*360/num_sides*(i+1)/180);
    vertices[(12*i+5)*3+1] = sin(M_PI*360/num_sides*(i+1)/180);
    vertices[(12*i+5)*3+2] = 1;
    
    normals[(12*i+3)*3+0] = cos(M_PI*360/num_sides*i/180);
    normals[(12*i+3)*3+1] = sin(M_PI*360/num_sides*i/180);
    normals[(12*i+3)*3+2] = 0;
    normals[(12*i+4)*3+0] = cos(M_PI*360/num_sides*(i+1)/180);
    normals[(12*i+4)*3+1] = sin(M_PI*360/num_sides*(i+1)/180);
    normals[(12*i+4)*3+2] = 0;
    normals[(12*i+5)*3+0] = cos(M_PI*360/num_sides*(i+1)/180);
    normals[(12*i+5)*3+1] = sin(M_PI*360/num_sides*(i+1)/180);
    normals[(12*i+5)*3+2] = 0;
    
    vertices[(12*i+6)*3+0] = cos(M_PI*360/num_sides*i/180);
    vertices[(12*i+6)*3+1] = sin(M_PI*360/num_sides*i/180);
    vertices[(12*i+6)*3+2] = 0;
    vertices[(12*i+7)*3+0] = 0;
    vertices[(12*i+7)*3+1] = 0;
    vertices[(12*i+7)*3+2] = 0;
    vertices[(12*i+8)*3+0] = cos(M_PI*360/num_sides*(i+1)/180);
    vertices[(12*i+8)*3+1] = sin(M_PI*360/num_sides*(i+1)/180);
    vertices[(12*i+8)*3+2] = 0;
    
    normals[(12*i+6)*3+0] = 0;
    normals[(12*i+6)*3+1] = 0;
    normals[(12*i+6)*3+2] = -1;
    normals[(12*i+7)*3+0] = 0;
    normals[(12*i+7)*3+1] = 0;
    normals[(12*i+7)*3+2] = -1;
    normals[(12*i+8)*3+0] = 0;
    normals[(12*i+8)*3+1] = 0;
    normals[(12*i+8)*3+2] = -1;
    
    vertices[(12*i+9)*3+0] = cos(M_PI*360/num_sides*i/180);
    vertices[(12*i+9)*3+1] = sin(M_PI*360/num_sides*i/180);
    vertices[(12*i+9)*3+2] = 1;
    vertices[(12*i+10)*3+0] = cos(M_PI*360/num_sides*(i+1)/180);
    vertices[(12*i+10)*3+1] = sin(M_PI*360/num_sides*(i+1)/180);
    vertices[(12*i+10)*3+2] = 1;
    vertices[(12*i+11)*3+0] = 0;
    vertices[(12*i+11)*3+1] = 0;
    vertices[(12*i+11)*3+2] = 1;
    
    normals[(12*i+9)*3+0] = 0;
    normals[(12*i+9)*3+1] = 0;
    normals[(12*i+9)*3+2] = 1;
    normals[(12*i+10)*3+0] = 0;
    normals[(12*i+10)*3+1] = 0;
    normals[(12*i+10)*3+2] = 1;
    normals[(12*i+11)*3+0] = 0;
    normals[(12*i+11)*3+1] = 0;
    normals[(12*i+11)*3+2] = 1;
  }
  for (i = 0; i < n*3; i++) {
    colors[i] = 1;
  }
  gr3_createmesh(&context_struct_.cylinder_mesh, n, vertices, normals, colors);
  context_struct_.mesh_list_[context_struct_.cylinder_mesh].data.type = kMTCylinderMesh;
  free(vertices);
  free(normals);
  free(colors);
}

/*!
 * This function allows drawing a cylinder without requiring a mesh.
 * \sa gr3_drawmesh()
 */
GR3API void gr3_drawcylindermesh(int n, const float *positions, const float *directions, const float *colors, const float *radii, const float *lengths) {
  int i;
  int j;
  int min_index;
  int min_n;
  float *scales = malloc(n*3*sizeof(float));
  float *ups = malloc(n*3*sizeof(float));
  GR3_DO_INIT;
  for (i = 0; i < n; i++) {
    scales[3*i+0] = radii[i];
    scales[3*i+1] = radii[i];
    scales[3*i+2] = lengths[i];
    min_n = directions[3*i+0];
    min_index = 0;
    for (j = 1; j < 3; j++) {
      if (directions[3*i+j]*directions[3*i+j] < min_n*min_n) {
        min_n = directions[3*i+j];
        min_index = j;
      }
    }
    for (j = 0; j < 3; j++) {
      ups[3*i+j] = 0;
    }
    ups[3*i+min_index] = 1;
  }
  gr3_drawmesh(context_struct_.cylinder_mesh,n,positions,directions,ups,colors,scales);
  free(scales);
  free(ups);
}

/*!
 * This function creates the context_struct_.cone_mesh for simple drawing
 */
static void gr3_createconemesh_(void) {
  int i;
  int n;
  float *vertices;
  float *normals;
  float *colors;
  
  n = 6*36;
  vertices = malloc(n*3*sizeof(float));
  normals = malloc(n*3*sizeof(float));
  colors = malloc(n*3*sizeof(float));
  for (i = 0; i < 36; i++) {
    vertices[(6*i+0)*3+0] = cos(M_PI*10*i/180);
    vertices[(6*i+0)*3+1] = sin(M_PI*10*i/180);
    vertices[(6*i+0)*3+2] = 0;
    vertices[(6*i+1)*3+0] = cos(M_PI*10*(i+1)/180);
    vertices[(6*i+1)*3+1] = sin(M_PI*10*(i+1)/180);
    vertices[(6*i+1)*3+2] = 0;
    vertices[(6*i+2)*3+0] = 0;
    vertices[(6*i+2)*3+1] = 0;
    vertices[(6*i+2)*3+2] = 1;
    
    normals[(6*i+0)*3+0] = cos(M_PI*10*i/180);
    normals[(6*i+0)*3+1] = sin(M_PI*10*i/180);
    normals[(6*i+0)*3+2] = 0;
    normals[(6*i+1)*3+0] = cos(M_PI*10*(i+1)/180);
    normals[(6*i+1)*3+1] = sin(M_PI*10*(i+1)/180);
    normals[(6*i+1)*3+2] = 0;
    normals[(6*i+2)*3+0] = 0;
    normals[(6*i+2)*3+1] = 0;
    normals[(6*i+2)*3+2] = 1;
    
    vertices[(6*i+3)*3+0] = cos(M_PI*10*i/180);
    vertices[(6*i+3)*3+1] = sin(M_PI*10*i/180);
    vertices[(6*i+3)*3+2] = 0;
    vertices[(6*i+4)*3+0] = 0;
    vertices[(6*i+4)*3+1] = 0;
    vertices[(6*i+4)*3+2] = 0;
    vertices[(6*i+5)*3+0] = cos(M_PI*10*(i+1)/180);
    vertices[(6*i+5)*3+1] = sin(M_PI*10*(i+1)/180);
    vertices[(6*i+5)*3+2] = 0;
    
    normals[(6*i+3)*3+0] = 0;
    normals[(6*i+3)*3+1] = 0;
    normals[(6*i+3)*3+2] = -1;
    normals[(6*i+4)*3+0] = 0;
    normals[(6*i+4)*3+1] = 0;
    normals[(6*i+4)*3+2] = -1;
    normals[(6*i+5)*3+0] = 0;
    normals[(6*i+5)*3+1] = 0;
    normals[(6*i+5)*3+2] = -1;
  }
  for (i = 0; i < n*3; i++) {
    colors[i] = 1;
  }
  gr3_createmesh(&context_struct_.cone_mesh, n, vertices, normals, colors);
  context_struct_.mesh_list_[context_struct_.cone_mesh].data.type = kMTConeMesh;
  free(vertices);
  free(normals);
  free(colors);
}

/*!
 * This function allows drawing a cylinder without requiring a mesh.
 * \sa gr3_drawmesh()
 */
GR3API void gr3_drawconemesh(int n,const float *positions, const float *directions, const float *colors, const float *radii, const float *lengths) {
  int i;
  int j;
  int min_index;
  int min_n;
  float *scales = malloc(n*3*sizeof(float));
  float *ups = malloc(n*3*sizeof(float));
  GR3_DO_INIT;

  for (i = 0; i < n; i++) {
    scales[3*i+0] = radii[i];
    scales[3*i+1] = radii[i];
    scales[3*i+2] = lengths[i];
    min_n = directions[3*i+0];
    min_index = 0;
    for (j = 1; j < 3; j++) {
      if (directions[3*i+j]*directions[3*i+j] < min_n*min_n) {
        min_n = directions[3*i+j];
        min_index = j;
      }
    }
    for (j = 0; j < 3; j++) {
      ups[3*i+j] = 0;
    }
    ups[3*i+min_index] = 1;
    
  }
  gr3_drawmesh(context_struct_.cone_mesh,n,positions,directions,ups,colors,scales);
  free(scales);
  free(ups);
}

/*!
 * This function allows drawing a sphere without requiring a mesh.
 * \sa gr3_drawmesh()
 */
GR3API void gr3_drawspheremesh(int n,const float *positions, const float *colors, const float *radii) {
  int i;
  float *directions = malloc(n*3*sizeof(float));
  float *ups = malloc(n*3*sizeof(float));
  float *scales = malloc(n*3*sizeof(float));
  GR3_DO_INIT;

  for (i = 0; i < n; i++) {
    directions[i*3+0] = 0;
    directions[i*3+1] = 0;
    directions[i*3+2] = 1;
    ups[i*3+0] = 0;
    ups[i*3+1] = 1;
    ups[i*3+2] = 0;
    scales[i*3+0] = radii[i];
    scales[i*3+1] = radii[i];
    scales[i*3+2] = radii[i];
  }
  gr3_drawmesh(context_struct_.sphere_mesh,n,positions,directions,ups,colors,scales);
  free(directions);
  free(ups);
  free(scales);
}

/*!
 * This function creates the context_struct_.sphere_mesh for simple drawing
 */
static void gr3_createspheremesh_(void) {
  int i,j;
  int n,iterations = 4;
  float *colors;
  float *vertices_old;
  float *vertices_new;
  float *vertices;
  float *triangle;
  float *triangle_new;
  /* pre-calculated icosahedron vertices */
  float icosahedron[] = {0.52573111211913359, 0, 0.85065080835203988, 0, 0.85065080835203988, 0.52573111211913359, -0.52573111211913359, 0, 0.85065080835203988, 0, 0.85065080835203988, 0.52573111211913359, -0.85065080835203988, 0.52573111211913359, 0, -0.52573111211913359, 0, 0.85065080835203988, 0, 0.85065080835203988, 0.52573111211913359, 0, 0.85065080835203988, -0.52573111211913359, -0.85065080835203988, 0.52573111211913359, 0, 0.85065080835203988, 0.52573111211913359, 0, 0, 0.85065080835203988, -0.52573111211913359, 0, 0.85065080835203988, 0.52573111211913359, 0.52573111211913359, 0, 0.85065080835203988, 0.85065080835203988, 0.52573111211913359, 0, 0, 0.85065080835203988, 0.52573111211913359, 0.52573111211913359, 0, 0.85065080835203988, 0.85065080835203988, -0.52573111211913359, 0, 0.85065080835203988, 0.52573111211913359, 0, 0.85065080835203988, -0.52573111211913359, 0, 0.52573111211913359, 0, -0.85065080835203988, 0.85065080835203988, 0.52573111211913359, 0, 0.85065080835203988, 0.52573111211913359, 0, 0.52573111211913359, 0, -0.85065080835203988, 0, 0.85065080835203988, -0.52573111211913359, 0.52573111211913359, 0, -0.85065080835203988, -0.52573111211913359, 0, -0.85065080835203988, 0, 0.85065080835203988, -0.52573111211913359, 0.52573111211913359, 0, -0.85065080835203988, 0, -0.85065080835203988, -0.52573111211913359, -0.52573111211913359, 0, -0.85065080835203988, 0.52573111211913359, 0, -0.85065080835203988, 0.85065080835203988, -0.52573111211913359, 0, 0, -0.85065080835203988, -0.52573111211913359, 0.85065080835203988, -0.52573111211913359, 0, 0, -0.85065080835203988, 0.52573111211913359, 0, -0.85065080835203988, -0.52573111211913359, 0, -0.85065080835203988, 0.52573111211913359, -0.85065080835203988, -0.52573111211913359, 0, 0, -0.85065080835203988, -0.52573111211913359, 0, -0.85065080835203988, 0.52573111211913359, -0.52573111211913359, 0, 0.85065080835203988, -0.85065080835203988, -0.52573111211913359, 0, 0, -0.85065080835203988, 0.52573111211913359, 0.52573111211913359, 0, 0.85065080835203988, -0.52573111211913359, 0, 0.85065080835203988, 0.85065080835203988, -0.52573111211913359, 0, 0.52573111211913359, 0, 0.85065080835203988, 0, -0.85065080835203988, 0.52573111211913359, -0.85065080835203988, -0.52573111211913359, 0, -0.52573111211913359, 0, 0.85065080835203988, -0.85065080835203988, 0.52573111211913359, 0, -0.52573111211913359, 0, -0.85065080835203988, -0.85065080835203988, -0.52573111211913359, 0, -0.85065080835203988, 0.52573111211913359, 0, 0, 0.85065080835203988, -0.52573111211913359, -0.52573111211913359, 0, -0.85065080835203988, -0.85065080835203988, 0.52573111211913359, 0, -0.85065080835203988, -0.52573111211913359, 0, -0.52573111211913359, 0, -0.85065080835203988, 0, -0.85065080835203988, -0.52573111211913359};
  n = 20;
  vertices_old = malloc(n*3*3*sizeof(float));
  memmove(vertices_old,icosahedron,n*3*3*sizeof(float));
  for (j = 0; j < iterations; j++) {
    vertices_new = malloc(4*n*3*3*sizeof(float));
    for (i = 0; i < n;i++) {
      float a[3], b[3], c[3];
      float len_a, len_b, len_c;
      triangle = &vertices_old[i*3*3];
      triangle_new = &vertices_new[i*3*3*4];
      a[0] = (triangle[2*3 + 0] + triangle[1*3 + 0])*0.5;
      a[1] = (triangle[2*3 + 1] + triangle[1*3 + 1])*0.5;
      a[2] = (triangle[2*3 + 2] + triangle[1*3 + 2])*0.5;
      len_a = sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
      a[0] = a[0]/len_a;
      a[1] = a[1]/len_a;
      a[2] = a[2]/len_a;
      b[0] = (triangle[0*3 + 0] + triangle[2*3 + 0])*0.5;
      b[1] = (triangle[0*3 + 1] + triangle[2*3 + 1])*0.5;
      b[2] = (triangle[0*3 + 2] + triangle[2*3 + 2])*0.5;
      len_b = sqrt(b[0]*b[0]+b[1]*b[1]+b[2]*b[2]);
      b[0] = b[0]/len_b;
      b[1] = b[1]/len_b;
      b[2] = b[2]/len_b;
      c[0] = (triangle[0*3 + 0] + triangle[1*3 + 0])*0.5;
      c[1] = (triangle[0*3 + 1] + triangle[1*3 + 1])*0.5;
      c[2] = (triangle[0*3 + 2] + triangle[1*3 + 2])*0.5;
      len_c = sqrt(c[0]*c[0]+c[1]*c[1]+c[2]*c[2]);
      c[0] = c[0]/len_c;
      c[1] = c[1]/len_c;
      c[2] = c[2]/len_c;
      
      triangle_new[0*3*3 + 0*3 + 0] = triangle[0*3 + 0];
      triangle_new[0*3*3 + 0*3 + 1] = triangle[0*3 + 1];
      triangle_new[0*3*3 + 0*3 + 2] = triangle[0*3 + 2];
      triangle_new[0*3*3 + 1*3 + 0] = c[0];
      triangle_new[0*3*3 + 1*3 + 1] = c[1];
      triangle_new[0*3*3 + 1*3 + 2] = c[2];
      triangle_new[0*3*3 + 2*3 + 0] = b[0];
      triangle_new[0*3*3 + 2*3 + 1] = b[1];
      triangle_new[0*3*3 + 2*3 + 2] = b[2];
      
      triangle_new[1*3*3 + 0*3 + 0] = a[0];
      triangle_new[1*3*3 + 0*3 + 1] = a[1];
      triangle_new[1*3*3 + 0*3 + 2] = a[2];
      triangle_new[1*3*3 + 1*3 + 0] = b[0];
      triangle_new[1*3*3 + 1*3 + 1] = b[1];
      triangle_new[1*3*3 + 1*3 + 2] = b[2];
      triangle_new[1*3*3 + 2*3 + 0] = c[0];
      triangle_new[1*3*3 + 2*3 + 1] = c[1];
      triangle_new[1*3*3 + 2*3 + 2] = c[2];
      
      triangle_new[2*3*3 + 0*3 + 0] = triangle[1*3 + 0];
      triangle_new[2*3*3 + 0*3 + 1] = triangle[1*3 + 1];
      triangle_new[2*3*3 + 0*3 + 2] = triangle[1*3 + 2];
      triangle_new[2*3*3 + 1*3 + 0] = a[0];
      triangle_new[2*3*3 + 1*3 + 1] = a[1];
      triangle_new[2*3*3 + 1*3 + 2] = a[2];
      triangle_new[2*3*3 + 2*3 + 0] = c[0];
      triangle_new[2*3*3 + 2*3 + 1] = c[1];
      triangle_new[2*3*3 + 2*3 + 2] = c[2];
      
      triangle_new[3*3*3 + 0*3 + 0] = a[0];
      triangle_new[3*3*3 + 0*3 + 1] = a[1];
      triangle_new[3*3*3 + 0*3 + 2] = a[2];
      triangle_new[3*3*3 + 1*3 + 0] = triangle[2*3 + 0];
      triangle_new[3*3*3 + 1*3 + 1] = triangle[2*3 + 1];
      triangle_new[3*3*3 + 1*3 + 2] = triangle[2*3 + 2];
      triangle_new[3*3*3 + 2*3 + 0] = b[0];
      triangle_new[3*3*3 + 2*3 + 1] = b[1];
      triangle_new[3*3*3 + 2*3 + 2] = b[2];
    }
    n*=4;
    free(vertices_old);
    vertices_old=vertices_new;
  }
  vertices = vertices_old;
  colors = malloc(n*3*3*sizeof(float));
  for (i = 0; i < n*3*3; i++) {
    colors[i] = 1;
  }
  gr3_createmesh(&context_struct_.sphere_mesh, n*3, vertices, vertices, colors);
  context_struct_.mesh_list_[context_struct_.sphere_mesh].data.type = kMTSphereMesh;
  free(colors);
  free(vertices);
}


GR3API void gr3_drawheightmap(const float *heightmap, int num_columns, int num_rows, const float *positions, const float *scales) {
  int mesh;
  float directions[3] = {0,0,1};
  float ups[3] = {0,1,0};
  float colors[3] = {1,1,1};
  float pos[3];
  GR3_DO_INIT;

  pos[0] = positions[0]-scales[0]/2;
  pos[1] = positions[1]-scales[1]/2;
  pos[2] = positions[2]-scales[2]/2;
  mesh = gr3_createheightmapmesh((float *)heightmap, num_columns, num_rows);
  gr3_drawmesh(mesh, 1, pos, directions, ups, colors, scales);
  gr3_deletemesh(mesh);
}

GR3API int gr3_createheightmapmesh(const float *heightmap, int num_columns, int num_rows) {
  int mesh;
  float colormap[256][3];
  
  /* Find the range of height values */
  int row;
  int column;
  float min_height = heightmap[0*num_columns+0];
  float max_height = heightmap[0*num_columns+0];
  for (row = 0; row < num_rows; row++) {
    for (column = 0; column < num_columns; column++) {
      float height = heightmap[row*num_columns+column];
      min_height = (min_height > height ) ? height : min_height;
      max_height = (max_height < height ) ? height : max_height;
    }
  }
  if (min_height == max_height) {
    max_height+=1;
  }
  
  {
    int i;
    for (i = 0; i < 256; i++) {
      int color;
      gr_inqcolor(1000+i, &color);
      colormap[i][0] =  (color        & 0xff) / 255.0;
      colormap[i][1] = ((color >>  8) & 0xff) / 255.0;
      colormap[i][2] = ((color >> 16) & 0xff) / 255.0;
    }
  }
  
  {
    /* Allocate memory for the vertex data */
    int num_rectangles = (num_columns-1)*(num_rows-1);
    float *positions = malloc(num_rectangles*2*3*3*sizeof(float));
    float *normals = malloc(num_rectangles*2*3*3*sizeof(float));
    float *colors = malloc(num_rectangles*2*3*3*sizeof(float));
    /* For each rectangle... */
    for (row = 0; row < num_rows-1; row++) {
      for (column = 0; column < num_columns-1; column++) {
        /* ... and for each of the 6 vertices per rectangle (2 triangles)... */
        int drow[] = {0,0,1, 1,0,1};
        int dcolumn[] = {0,1,1, 1,0,0};
        int i;
        for (i = 0; i < 6; i++) {
          int array_offset = ((row*(num_columns-1)+column)*6+i)*3;
          /* Set current row and column */
          int crow = row+drow[i];
          int ccolumn = column+dcolumn[i];
          /* Normalize the row and column to [0;1] */
          float nrow = 1.0f*crow/(num_rows-1);
          float ncolumn = 1.0f*ccolumn/(num_columns-1);
          /* And read the height value from the heightmap */
          float height = heightmap[crow*num_columns+ccolumn];
          /* Normalize the height value to [0;1] */
          height = (height-min_height)/(max_height-min_height);
          /* Write the values into the positions array */
          positions[array_offset+0] = ncolumn;
          positions[array_offset+1] = nrow;
          positions[array_offset+2] = height;
          /* Calculate normals via cross product */
          {
            float vector1[3];
            float vector2[3];
            float vector3[3];
            
            vector1[0] = 0;
            vector1[1] = 1.0/num_rows;
            if (crow > 0) {
              float height_updown = heightmap[(crow-1)*num_columns+ccolumn];
              height_updown = (height_updown-min_height)/(max_height-min_height);
              vector1[2] = height_updown-height;
            } else {
              float height_updown = heightmap[(crow+1)*num_columns+ccolumn];
              height_updown = (height_updown-min_height)/(max_height-min_height);
              vector1[2] = height_updown-height;
            }
            
            vector2[0] = 1.0/num_columns;
            vector2[1] = 0;
            if (ccolumn > 0) {
              float height_leftright = heightmap[crow*num_columns+ccolumn-1];
              height_leftright = (height_leftright-min_height)/(max_height-min_height);
              vector2[2] = height_leftright-height;
            } else {
              float height_leftright = heightmap[crow*num_columns+ccolumn+1];
              height_leftright = (height_leftright-min_height)/(max_height-min_height);
              vector2[2] = height_leftright-height;
            }
            
            /* Calculate the cross product */
            vector3[0] = vector2[1]*vector1[2]-vector2[2]*vector1[1];
            vector3[1] = vector2[2]*vector1[0]-vector2[0]*vector1[2];
            vector3[2] = vector2[0]*vector1[1]-vector2[1]*vector1[0];
            
            /* Normalize the cross product */
            {
              int j;
              float tmp = 0;
              for (j = 0; j < 3; j++) {
                tmp += vector3[j]*vector3[j];
              }
              tmp = sqrt(tmp);
              for (j = 0; j < 3; j++) {
                vector3[j] /= tmp;
              }
            }
            normals[array_offset+0] = -vector3[0];
            normals[array_offset+1] = -vector3[1];
            normals[array_offset+2] = vector3[2];
          }
          /* Use fake colors */
          {
              int color = height * 256;
              if (color < 0) color = 0;
              if (color > 255) color = 255;
              colors[array_offset+0] = colormap[color][0];
              colors[array_offset+1] = colormap[color][1];
              colors[array_offset+2] = colormap[color][2];
          }
        }
      }
    }
    
    /* Create a mesh with the data */
    gr3_createmesh(&mesh,(num_columns-1)*(num_rows-1)*2*3,positions,normals,colors);
    
    /* Free the allocated memory */
    free(positions);
    free(normals);
    free(colors);
  }
  return mesh;
}

/*!
 * Create a mesh from an isosurface extracted from voxel data
 * using the marching cubes algorithm.
 *
 * \param [out] mesh          the mesh
 * \param [in]  data          the volume (voxel) data
 * \param [in]  isolevel      value where the isosurface will be extracted
 * \param [in]  dim_x         number of elements in x-direction
 * \param [in]  dim_y         number of elements in y-direction
 * \param [in]  dim_z         number of elements in z-direction
 * \param [in]  stride_x      number of elements to step when traversing
 *                            the data in x-direction
 * \param [in]  stride_y      number of elements to step when traversing
 *                            the data in y-direction
 * \param [in]  stride_z      number of elements to step when traversing
 *                            the data in z-direction
 * \param [in]  step_x        distance between the voxels in x-direction
 * \param [in]  step_y        distance between the voxels in y-direction
 * \param [in]  step_z        distance between the voxels in z-direction
 * \param [in]  offset_x      coordinate origin
 * \param [in]  offset_y      coordinate origin
 * \param [in]  offset_z      coordinate origin
 * 
 * \returns
 *  - ::GR3_ERROR_NONE        on success
 *  - ::GR3_ERROR_OPENGL_ERR  if an OpenGL error occured
 *  - ::GR3_ERROR_OUT_OF_MEM  if a memory allocation failed
 */
GR3API int gr3_createisosurfacemesh(int *mesh, GR3_MC_DTYPE *data,
                        GR3_MC_DTYPE isolevel,
                        unsigned int dim_x, unsigned int dim_y,
                        unsigned int dim_z, unsigned int stride_x,
                        unsigned int stride_y, unsigned int stride_z,
                        double step_x, double step_y, double step_z,
                        double offset_x, double offset_y, double offset_z)
{
    unsigned int num_vertices, num_indices;
    gr3_coord_t *vertices, *normals;
    float *colors;
    unsigned int *indices;
    unsigned int i;
    int err;

    gr3_triangulateindexed(data, isolevel, dim_x, dim_y, dim_z, stride_x,
                           stride_y, stride_z, step_x, step_y, step_z,
                           offset_x, offset_y, offset_z,
                           &num_vertices, &vertices,
                           &normals, &num_indices,
                           &indices);
    colors = malloc(num_vertices * 3 * sizeof(float));
    for (i = 0; i < num_vertices; i++) {
        colors[i * 3 + 0] = 1.0f;
        colors[i * 3 + 1] = 1.0f;
        colors[i * 3 + 2] = 1.0f;
    }
    err = gr3_createindexedmesh_nocopy(mesh, num_vertices, (float *) vertices,
                                       (float *) normals, colors, num_indices,
                                       (int *) indices);
    if (err != GR3_ERROR_NONE && err != GR3_ERROR_OPENGL_ERR) {
        free(vertices);
        free(normals);
        free(colors);
        free(indices);
    }

    return err;
}

#define GR3_INDEX(stride, offset, index) ((index)*(stride)+(offset))
#define GR3_IN(index) in[GR3_INDEX(in_stride, in_offset, (index))]
#define GR3_OUT(index) out[GR3_INDEX(out_stride, out_offset, (index))]
static int cupic_interp(const float *in, int in_offset, int in_stride, float *out, int out_offset, int out_stride, int num_points, int num_steps) {
  int i;
  int num_new_points = num_points * (1 + num_steps) - num_steps;
  double *right_side = (double *)malloc(sizeof(double) * num_points);
  double *diagonal = (double *)malloc(sizeof(double) * num_points);
  double *derivatives = NULL;
  assert(right_side);
  assert(diagonal);
  assert(in);
  assert(out);
  
  right_side[0] = 3*(GR3_IN(1)-GR3_IN(0));
  diagonal[0] = 2;
  for (i = 1; i < num_points-1; i++) {
    right_side[i] = 3*(GR3_IN(i+1)-GR3_IN(i-1));
    diagonal[i] = 4;
  }
  right_side[num_points-1] = 3*(GR3_IN(num_points-1)-GR3_IN(num_points-2));
  diagonal[num_points-1] = 2;
  
  for (i = 0; i < num_points-1; i++) {
    diagonal[i+1] -= 1/diagonal[i];
    right_side[i+1] -= right_side[i]/diagonal[i];
  }
  
  for (i = num_points-1; i > 0; i--) {
    right_side[i] /= diagonal[i];
    right_side[i-1] -= right_side[i];
  }
  right_side[0] /= diagonal[0];
  
  free(diagonal);
  
  derivatives = right_side;
  for (i = 0; i < num_new_points; i++) {
    div_t j = div(i, num_steps+1);
    int section = j.quot;
    int step = j.rem;
    double t = (double)step/(num_steps+1);
    if (t == 0) {
      GR3_OUT(i) = GR3_IN(section);
    } else {
      double a = GR3_IN(section);
      double b = derivatives[section];
      double c = 3*(GR3_IN(section+1)-GR3_IN(section))-2*derivatives[section]-derivatives[section+1];
      double d = 2*(GR3_IN(section)-GR3_IN(section+1))+derivatives[section]+derivatives[section+1];
      GR3_OUT(i) = a+(b+(c+d*t)*t)*t;
    }
  }
  free(derivatives);
  return num_new_points;
}

static int linear_interp(const float *in, int in_offset, int in_stride, float *out, int out_offset, int out_stride, int num_points, int num_steps) {
  int i;
  int num_new_points = num_points * (1 + num_steps) - num_steps;
  for (i = 0; i < num_new_points; i++) {
    div_t j = div(i, num_steps+1);
    int section = j.quot;
    int step = j.rem;
    double t = (double)step/(num_steps+1);
    if (t == 0) {
      GR3_OUT(i) = GR3_IN(section);
    } else {
      double a = GR3_IN(section);
      double b = GR3_IN(section+1)-GR3_IN(section);
      GR3_OUT(i) = a+b*t;
    }
  }
  return num_new_points;
}
#undef GR3_OUT
#undef GR3_IN
#undef GR3_INDEX

static float *generic_interp_nd(const float *points, int n, int num_points, int num_steps, int *p_num_new_points, int (*interp)(const float *, int, int, float *, int, int, int, int)) {
  int i;
  int num_new_points = num_points * (1 + num_steps) - num_steps;
  float *points2 = (float *)malloc(n * sizeof(float) * num_new_points);
  assert(points2);
  for (i = 0; i < n; i++) {
    interp(points, i, n, points2, i, n, num_points, num_steps);
  }
  if (p_num_new_points) {
    *p_num_new_points = num_new_points;
  }
  return points2;
}

static float *cubic_interp_nd(const float *points, int n, int num_points, int num_steps, int *p_num_new_points) {
  return generic_interp_nd(points, n, num_points, num_steps, p_num_new_points, cupic_interp);
}

static float *linear_interp_nd(const float *points, int n, int num_points, int num_steps, int *p_num_new_points) {
  return generic_interp_nd(points, n, num_points, num_steps, p_num_new_points, linear_interp);
}

static double dot(double left[3], double right[3]) {
  return left[0]*right[0]+left[1]*right[1]+left[2]*right[2];
}

static void normalize(double a[3]) {
  double length = sqrt(dot(a, a));
  if (length > 0) {
    a[0] /= length;
    a[1] /= length;
    a[2] /= length;
  }
}

static void cross(const double left[3], const double right[3], double out[3]) {
  out[0] = left[1]*right[2]-left[2]*right[1];
  out[1] = left[2]*right[0]-left[0]*right[2];
  out[2] = left[0]*right[1]-left[1]*right[0];
}

int gr3_createtubemesh(int *mesh, int n, const float *points, const float *colors, const float *radii, int num_steps, int num_segments) {
  int result;
  int num_points = n;
  int i;
  int num_new_points = 0;
  float (*points2)[3];
  float (*colors2)[3];
  float *radii2;
  double (*directions)[3];
  double (*tangents)[3];
  double (*normals)[3];
  double (*binormals)[3];
  float (*points_filtered)[3];
  float (*colors_filtered)[3];
  float *radii_filtered;
  
  double longest_direction_cross[3] = {0, 0, 0};
  double longest_direction_cross_length_squared = 0;
  int longest_direction_cross_index = 0;
  
  int *filtered_indices = (int *)malloc(sizeof(int) * num_points);
  int j = 1;
  filtered_indices[0] = 0;
  for (i = 1; i < num_points; i++) {
    double distance[3];
    distance[0] = points[3*i+0]-points[3*i-3+0];
    distance[1] = points[3*i+1]-points[3*i-3+1];
    distance[2] = points[3*i+2]-points[3*i-3+2];
    if (dot(distance, distance) > 0) {
      filtered_indices[j++] = i;
    }
  }

  num_points = j;
  if (num_points < 2) {
    float v[9] = {0,0,0,0,0,0,0,0,0};
    free(filtered_indices);
    return gr3_createmesh(mesh, 3, (float *)v, (float *)v, (float *)v);
  };
  points_filtered = (float (*)[3])malloc(sizeof(float) * 3 * num_points);
  colors_filtered = (float (*)[3])malloc(sizeof(float) * 3 * num_points);
  radii_filtered = (float *)malloc(sizeof(float) * num_points);
  assert(points_filtered);
  assert(colors_filtered);
  assert(radii_filtered);

  for (i = 0; i < num_points; i++) {
    points_filtered[i][0] = points[filtered_indices[i]*3+0];
    points_filtered[i][1] = points[filtered_indices[i]*3+1];
    points_filtered[i][2] = points[filtered_indices[i]*3+2];
    colors_filtered[i][0] = colors[filtered_indices[i]*3+0];
    colors_filtered[i][1] = colors[filtered_indices[i]*3+1];
    colors_filtered[i][2] = colors[filtered_indices[i]*3+2];
    radii_filtered[i] = radii[filtered_indices[i]];
  }
  free(filtered_indices);

  points2 = (float (*)[3])cubic_interp_nd((float *)points_filtered, 3, num_points, num_steps, &num_new_points);
  colors2 = (float (*)[3])linear_interp_nd((float *)colors_filtered, 3, num_points, num_steps, NULL);
  radii2 = linear_interp_nd(radii_filtered, 1, num_points, num_steps, NULL);
  directions = (double (*)[3])malloc(sizeof(double) * 3 * num_new_points);
  tangents = (double (*)[3])malloc(sizeof(double) * 3 * num_new_points);
  /* aliases to avoid two useless allocations */
  normals = directions;
  binormals = tangents;

  free(points_filtered);
  free(colors_filtered);
  free(radii_filtered);

  for (i = 0; i < num_new_points-1; i++) {
    directions[i][0] = points2[i+1][0] - points2[i][0];
    directions[i][1] = points2[i+1][1] - points2[i][1];
    directions[i][2] = points2[i+1][2] - points2[i][2];
    normalize(directions[i]);
  }

  tangents[0][0] = directions[0][0];
  tangents[0][1] = directions[0][1];
  tangents[0][2] = directions[0][2];
  for (i = 1; i < num_new_points-1; i++) {
    tangents[i][0] = directions[i-1][0]+directions[i][0];
    tangents[i][1] = directions[i-1][1]+directions[i][1];
    tangents[i][2] = directions[i-1][2]+directions[i][2];
    normalize(tangents[i]);
  }
  tangents[num_new_points-1][0] = directions[num_new_points-2][0];
  tangents[num_new_points-1][1] = directions[num_new_points-2][1];
  tangents[num_new_points-1][2] = directions[num_new_points-2][2];
  
  for (i = 0; i < num_new_points-2; i++) {
    double direction_cross[3];
    double direction_cross_length_squared;
    cross(directions[i], directions[i+1], direction_cross);
    direction_cross_length_squared = dot(direction_cross, direction_cross);
    if (direction_cross_length_squared > longest_direction_cross_length_squared) {
      longest_direction_cross_index = i;
      longest_direction_cross_length_squared = direction_cross_length_squared;
      longest_direction_cross[0] = direction_cross[0];
      longest_direction_cross[1] = direction_cross[1];
      longest_direction_cross[2] = direction_cross[2];
    }
  }
  
  if (longest_direction_cross_length_squared > 1e-12) {
    normals[longest_direction_cross_index+1][0] = longest_direction_cross[0];
    normals[longest_direction_cross_index+1][1] = longest_direction_cross[1];
    normals[longest_direction_cross_index+1][2] = longest_direction_cross[2];
  } else {
    if (fabs(directions[0][0]) < fabs(directions[0][1])) {
      double tmp[3] = {1, 0, 0};
      cross(directions[0], tmp, normals[longest_direction_cross_index+1]);
    } else {
      double tmp[3] = {0, 1, 0};
      cross(directions[0], tmp, normals[longest_direction_cross_index+1]);
    }
  }
  normalize(normals[longest_direction_cross_index+1]);
  for (i = longest_direction_cross_index; i >= 0; i--) {
    double tmp[3];
    cross(tangents[i], normals[i+1], tmp);
    cross(tmp, tangents[i], normals[i]);
    normalize(normals[i]);
  }
  for (i = longest_direction_cross_index+2; i < num_new_points; i++) {
    double tmp[3];
    cross(tangents[i], normals[i-1], tmp);
    cross(tmp, tangents[i], normals[i]);
    normalize(normals[i]);
  }
  for (i = 0; i < num_new_points; i++) {
    double tmp[3];
    cross(tangents[i], normals[i], tmp);
    binormals[i][0] = tmp[0];
    binormals[i][1] = tmp[1];
    binormals[i][2] = tmp[2];
    normalize(binormals[i]);
  }
  
  {
    double (*vertices_2d)[2] = (double (*)[2])malloc(sizeof(double) * 2 * num_segments);
    float (*vertices_3d)[3] = (float (*)[3])malloc(sizeof(float) * 3 * 6 * num_segments * num_new_points);
    float (*normals_3d)[3] = (float (*)[3])malloc(sizeof(float) * 3 * 6 * num_segments * num_new_points);
    float (*colors_3d)[3] = (float (*)[3])malloc(sizeof(float) * 3 * 6 * num_segments * num_new_points);
    for (i = 0; i < num_segments; i++) {
      vertices_2d[i][0] = cos(i*2*3.1415/num_segments);
      vertices_2d[i][1] = sin(i*2*3.1415/num_segments);
    }
    for (i = 0; i < num_new_points-1; i++) {
      int j;
      for (j = 0; j < num_segments; j++) {
        normals_3d[(i*num_segments+j)*6+0][0] = vertices_2d[j][0]*normals[i][0]+vertices_2d[j][1]*binormals[i][0];
        normals_3d[(i*num_segments+j)*6+0][1] = vertices_2d[j][0]*normals[i][1]+vertices_2d[j][1]*binormals[i][1];
        normals_3d[(i*num_segments+j)*6+0][2] = vertices_2d[j][0]*normals[i][2]+vertices_2d[j][1]*binormals[i][2];
        vertices_3d[(i*num_segments+j)*6+0][0] = points2[i][0]+radii2[i]*normals_3d[(i*num_segments+j)*6+0][0];
        vertices_3d[(i*num_segments+j)*6+0][1] = points2[i][1]+radii2[i]*normals_3d[(i*num_segments+j)*6+0][1];
        vertices_3d[(i*num_segments+j)*6+0][2] = points2[i][2]+radii2[i]*normals_3d[(i*num_segments+j)*6+0][2];
        colors_3d[(i*num_segments+j)*6+0][0] = colors2[i][0];
        colors_3d[(i*num_segments+j)*6+0][1] = colors2[i][1];
        colors_3d[(i*num_segments+j)*6+0][2] = colors2[i][2];
        
        normals_3d[(i*num_segments+j)*6+1][0] = vertices_2d[(j+1)%num_segments][0]*normals[i][0]+vertices_2d[(j+1)%num_segments][1]*binormals[i][0];
        normals_3d[(i*num_segments+j)*6+1][1] = vertices_2d[(j+1)%num_segments][0]*normals[i][1]+vertices_2d[(j+1)%num_segments][1]*binormals[i][1];
        normals_3d[(i*num_segments+j)*6+1][2] = vertices_2d[(j+1)%num_segments][0]*normals[i][2]+vertices_2d[(j+1)%num_segments][1]*binormals[i][2];
        vertices_3d[(i*num_segments+j)*6+1][0] = points2[i][0]+radii2[i]*normals_3d[(i*num_segments+j)*6+1][0];
        vertices_3d[(i*num_segments+j)*6+1][1] = points2[i][1]+radii2[i]*normals_3d[(i*num_segments+j)*6+1][1];
        vertices_3d[(i*num_segments+j)*6+1][2] = points2[i][2]+radii2[i]*normals_3d[(i*num_segments+j)*6+1][2];
        colors_3d[(i*num_segments+j)*6+1][0] = colors2[i][0];
        colors_3d[(i*num_segments+j)*6+1][1] = colors2[i][1];
        colors_3d[(i*num_segments+j)*6+1][2] = colors2[i][2];
        
        normals_3d[(i*num_segments+j)*6+2][0] = vertices_2d[j][0]*normals[i+1][0]+vertices_2d[j][1]*binormals[i+1][0];
        normals_3d[(i*num_segments+j)*6+2][1] = vertices_2d[j][0]*normals[i+1][1]+vertices_2d[j][1]*binormals[i+1][1];
        normals_3d[(i*num_segments+j)*6+2][2] = vertices_2d[j][0]*normals[i+1][2]+vertices_2d[j][1]*binormals[i+1][2];
        vertices_3d[(i*num_segments+j)*6+2][0] = points2[i+1][0]+radii2[i+1]*normals_3d[(i*num_segments+j)*6+2][0];
        vertices_3d[(i*num_segments+j)*6+2][1] = points2[i+1][1]+radii2[i+1]*normals_3d[(i*num_segments+j)*6+2][1];
        vertices_3d[(i*num_segments+j)*6+2][2] = points2[i+1][2]+radii2[i+1]*normals_3d[(i*num_segments+j)*6+2][2];
        colors_3d[(i*num_segments+j)*6+2][0] = colors2[i+1][0];
        colors_3d[(i*num_segments+j)*6+2][1] = colors2[i+1][1];
        colors_3d[(i*num_segments+j)*6+2][2] = colors2[i+1][2];
        
        normals_3d[(i*num_segments+j)*6+3][0] = normals_3d[(i*num_segments+j)*6+2][0];
        normals_3d[(i*num_segments+j)*6+3][1] = normals_3d[(i*num_segments+j)*6+2][1];
        normals_3d[(i*num_segments+j)*6+3][2] = normals_3d[(i*num_segments+j)*6+2][2];
        vertices_3d[(i*num_segments+j)*6+3][0] = vertices_3d[(i*num_segments+j)*6+2][0];
        vertices_3d[(i*num_segments+j)*6+3][1] = vertices_3d[(i*num_segments+j)*6+2][1];
        vertices_3d[(i*num_segments+j)*6+3][2] = vertices_3d[(i*num_segments+j)*6+2][2];
        colors_3d[(i*num_segments+j)*6+3][0] = colors2[i+1][0];
        colors_3d[(i*num_segments+j)*6+3][1] = colors2[i+1][1];
        colors_3d[(i*num_segments+j)*6+3][2] = colors2[i+1][2];
        
        normals_3d[(i*num_segments+j)*6+4][0] = normals_3d[(i*num_segments+j)*6+1][0];
        normals_3d[(i*num_segments+j)*6+4][1] = normals_3d[(i*num_segments+j)*6+1][1];
        normals_3d[(i*num_segments+j)*6+4][2] = normals_3d[(i*num_segments+j)*6+1][2];
        vertices_3d[(i*num_segments+j)*6+4][0] = vertices_3d[(i*num_segments+j)*6+1][0];
        vertices_3d[(i*num_segments+j)*6+4][1] = vertices_3d[(i*num_segments+j)*6+1][1];
        vertices_3d[(i*num_segments+j)*6+4][2] = vertices_3d[(i*num_segments+j)*6+1][2];
        colors_3d[(i*num_segments+j)*6+4][0] = colors2[i][0];
        colors_3d[(i*num_segments+j)*6+4][1] = colors2[i][1];
        colors_3d[(i*num_segments+j)*6+4][2] = colors2[i][2];
        
        normals_3d[(i*num_segments+j)*6+5][0] = vertices_2d[(j+1)%num_segments][0]*normals[i+1][0]+vertices_2d[(j+1)%num_segments][1]*binormals[i+1][0];
        normals_3d[(i*num_segments+j)*6+5][1] = vertices_2d[(j+1)%num_segments][0]*normals[i+1][1]+vertices_2d[(j+1)%num_segments][1]*binormals[i+1][1];
        normals_3d[(i*num_segments+j)*6+5][2] = vertices_2d[(j+1)%num_segments][0]*normals[i+1][2]+vertices_2d[(j+1)%num_segments][1]*binormals[i+1][2];
        vertices_3d[(i*num_segments+j)*6+5][0] = points2[i+1][0]+radii2[i+1]*normals_3d[(i*num_segments+j)*6+5][0];
        vertices_3d[(i*num_segments+j)*6+5][1] = points2[i+1][1]+radii2[i+1]*normals_3d[(i*num_segments+j)*6+5][1];
        vertices_3d[(i*num_segments+j)*6+5][2] = points2[i+1][2]+radii2[i+1]*normals_3d[(i*num_segments+j)*6+5][2];
        colors_3d[(i*num_segments+j)*6+5][0] = colors2[i+1][0];
        colors_3d[(i*num_segments+j)*6+5][1] = colors2[i+1][1];
        colors_3d[(i*num_segments+j)*6+5][2] = colors2[i+1][2];
      }
    }
    
    {
      int index_offset = (num_new_points-1)*num_segments * 6;
      double normal[3];
      cross(normals[0], binormals[0], normal);
      for (i = 0; i < num_segments; i++) {
        normals_3d[index_offset+i*3+0][0] = normal[0];
        normals_3d[index_offset+i*3+0][1] = normal[1];
        normals_3d[index_offset+i*3+0][2] = normal[2];
        vertices_3d[index_offset+i*3+0][0] = points2[0][0] + radii2[0]*(vertices_2d[i][0]*normals[0][0]+vertices_2d[i][1]*binormals[0][0]);
        vertices_3d[index_offset+i*3+0][1] = points2[0][1] + radii2[0]*(vertices_2d[i][0]*normals[0][1]+vertices_2d[i][1]*binormals[0][1]);
        vertices_3d[index_offset+i*3+0][2] = points2[0][2] + radii2[0]*(vertices_2d[i][0]*normals[0][2]+vertices_2d[i][1]*binormals[0][2]);
        colors_3d[index_offset+i*3+0][0] = colors2[0][0];
        colors_3d[index_offset+i*3+0][1] = colors2[0][1];
        colors_3d[index_offset+i*3+0][2] = colors2[0][2];
        
        normals_3d[index_offset+i*3+1][0] = normal[0];
        normals_3d[index_offset+i*3+1][1] = normal[1];
        normals_3d[index_offset+i*3+1][2] = normal[2];
        vertices_3d[index_offset+i*3+1][0] = points2[0][0] + radii2[0]*(vertices_2d[(i+1) % num_segments][0]*normals[0][0]+vertices_2d[(i+1) % num_segments][1]*binormals[0][0]);
        vertices_3d[index_offset+i*3+1][1] = points2[0][1] + radii2[0]*(vertices_2d[(i+1) % num_segments][0]*normals[0][1]+vertices_2d[(i+1) % num_segments][1]*binormals[0][1]);
        vertices_3d[index_offset+i*3+1][2] = points2[0][2] + radii2[0]*(vertices_2d[(i+1) % num_segments][0]*normals[0][2]+vertices_2d[(i+1) % num_segments][1]*binormals[0][2]);
        colors_3d[index_offset+i*3+1][0] = colors2[0][0];
        colors_3d[index_offset+i*3+1][1] = colors2[0][1];
        colors_3d[index_offset+i*3+1][2] = colors2[0][2];
        
        normals_3d[index_offset+i*3+2][0] = normal[0];
        normals_3d[index_offset+i*3+2][1] = normal[1];
        normals_3d[index_offset+i*3+2][2] = normal[2];
        vertices_3d[index_offset+i*3+2][0] = points2[0][0];
        vertices_3d[index_offset+i*3+2][1] = points2[0][1];
        vertices_3d[index_offset+i*3+2][2] = points2[0][2];
        colors_3d[index_offset+i*3+2][0] = colors2[0][0];
        colors_3d[index_offset+i*3+2][1] = colors2[0][1];
        colors_3d[index_offset+i*3+2][2] = colors2[0][2];
      }
      index_offset += 3 * num_segments;
      cross(normals[num_new_points-1], binormals[num_new_points-1], normal);
      
      for (i = 0; i < num_segments; i++) {
        normals_3d[index_offset+i*3+0][0] = normal[0];
        normals_3d[index_offset+i*3+0][1] = normal[1];
        normals_3d[index_offset+i*3+0][2] = normal[2];
        vertices_3d[index_offset+i*3+0][0] = points2[num_new_points-1][0] + radii2[num_new_points-1]*(vertices_2d[i][0]*normals[num_new_points-1][0]+vertices_2d[i][1]*binormals[num_new_points-1][0]);
        vertices_3d[index_offset+i*3+0][1] = points2[num_new_points-1][1] + radii2[num_new_points-1]*(vertices_2d[i][0]*normals[num_new_points-1][1]+vertices_2d[i][1]*binormals[num_new_points-1][1]);
        vertices_3d[index_offset+i*3+0][2] = points2[num_new_points-1][2] + radii2[num_new_points-1]*(vertices_2d[i][0]*normals[num_new_points-1][2]+vertices_2d[i][1]*binormals[num_new_points-1][2]);
        colors_3d[index_offset+i*3+0][0] = colors2[num_new_points-1][0];
        colors_3d[index_offset+i*3+0][1] = colors2[num_new_points-1][1];
        colors_3d[index_offset+i*3+0][2] = colors2[num_new_points-1][2];
        
        normals_3d[index_offset+i*3+1][0] = normal[0];
        normals_3d[index_offset+i*3+1][1] = normal[1];
        normals_3d[index_offset+i*3+1][2] = normal[2];
        vertices_3d[index_offset+i*3+1][0] = points2[num_new_points-1][0] + radii2[num_new_points-1]*(vertices_2d[(i+1) % num_segments][0]*normals[num_new_points-1][0]+vertices_2d[(i+1) % num_segments][1]*binormals[num_new_points-1][0]);
        vertices_3d[index_offset+i*3+1][1] = points2[num_new_points-1][1] + radii2[num_new_points-1]*(vertices_2d[(i+1) % num_segments][0]*normals[num_new_points-1][1]+vertices_2d[(i+1) % num_segments][1]*binormals[num_new_points-1][1]);
        vertices_3d[index_offset+i*3+1][2] = points2[num_new_points-1][2] + radii2[num_new_points-1]*(vertices_2d[(i+1) % num_segments][0]*normals[num_new_points-1][2]+vertices_2d[(i+1) % num_segments][1]*binormals[num_new_points-1][2]);
        colors_3d[index_offset+i*3+1][0] = colors2[num_new_points-1][0];
        colors_3d[index_offset+i*3+1][1] = colors2[num_new_points-1][1];
        colors_3d[index_offset+i*3+1][2] = colors2[num_new_points-1][2];
        
        normals_3d[index_offset+i*3+2][0] = normal[0];
        normals_3d[index_offset+i*3+2][1] = normal[1];
        normals_3d[index_offset+i*3+2][2] = normal[2];
        vertices_3d[index_offset+i*3+2][0] = points2[num_new_points-1][0];
        vertices_3d[index_offset+i*3+2][1] = points2[num_new_points-1][1];
        vertices_3d[index_offset+i*3+2][2] = points2[num_new_points-1][2];
        colors_3d[index_offset+i*3+2][0] = colors2[num_new_points-1][0];
        colors_3d[index_offset+i*3+2][1] = colors2[num_new_points-1][1];
        colors_3d[index_offset+i*3+2][2] = colors2[num_new_points-1][2];
      }
      
    }
    
    free(vertices_2d);
    free(points2);
    free(radii2);
    free(colors2);
    free(tangents);
    free(directions);
    
    result = gr3_createmesh(mesh, num_new_points*num_segments*6, (float *)vertices_3d, (float *)normals_3d, (float *)colors_3d);
    free(vertices_3d);
    free(normals_3d);
    free(colors_3d);
  }
  return result;
}

int gr3_drawtubemesh(int n, float *points, float *colors, float *radii, int num_steps, int num_segments) {
  int result;
  int mesh;
  float position[] = {0, 0, 0};
  float direction[] = {0, 0, 1};
  float up[] = {0, 1, 0};
  float color[] = {1, 1, 1};
  result = gr3_createtubemesh(&mesh, n, points, colors, radii, num_steps, num_segments);
  gr3_drawmesh(mesh, 1, position, direction, up, color, color);
  gr3_deletemesh(mesh);
  return result;
}

GR3API void gr3_drawspins(int n, const float *positions, const float *directions, const float *colors, float cone_radius, float cylinder_radius, float cone_height, float cylinder_height) {
    int i;
    float zOffset = 0.5*(cylinder_height-cone_height);
    float *cone_positions = malloc(sizeof(float) * n * 3);
    float *cylinder_positions = malloc(sizeof(float) * n * 3);
    float *cone_radii = malloc(sizeof(float) * n);
    float *cylinder_radii = malloc(sizeof(float) * n);
    float *cone_lengths = malloc(sizeof(float) * n);
    float *cylinder_lengths = malloc(sizeof(float) * n);
    assert(cone_positions);
    assert(cylinder_positions);
    assert(cone_radii);
    assert(cylinder_radii);
    assert(cone_lengths);
    assert(cylinder_lengths);
    for (i = 0; i < 3*n; i++) {
        float t = sqrt(directions[i/3*3+0]*directions[i/3*3+0]+directions[i/3*3+1]*directions[i/3*3+1]+directions[i/3*3+2]*directions[i/3*3+2]);
        cone_positions[i] = positions[i]+zOffset*directions[i]/t;
        cylinder_positions[i] = positions[i]+(zOffset-cylinder_height)*directions[i]/t;
    }
    for (i = 0; i < n; i++) {
        cone_radii[i] = cone_radius;
        cylinder_radii[i] = cylinder_radius;
        cone_lengths[i] = cone_height;
        cylinder_lengths[i] = cylinder_height;
    }
    gr3_drawconemesh(n, cone_positions, directions, colors, cone_radii, cone_lengths);
    gr3_drawcylindermesh(n, cylinder_positions, directions, colors, cylinder_radii, cylinder_lengths);
    free(cone_positions);
    free(cylinder_positions);
    free(cone_radii);
    free(cylinder_radii);
    free(cone_lengths);
    free(cylinder_lengths);
}

/* Bond calculation code by Daniel Kaiser <d.kaiser@fz-juelich.de> */
#define CELL_IDX(cell, dim) cell.z*dim.x*dim.y+cell.y*dim.x+cell.x

#define EPS 0.001

typedef struct{
    double x;
    double y;
    double z;
} double3;

typedef struct{
    float x;
    float y;
    float z;
} float3;

typedef struct{
    unsigned char x;
    unsigned char y;
    unsigned char z;
} uchar3;

typedef struct {
    int x;
    int y;
    int z;
} int3;

static int calc_bonds(const float *positions, int num_atoms, float bond_length, float3 **start, float3 **end);

GR3API void gr3_drawmolecule(int n, const float *positions, const float *colors, const float *radii, float bond_radius, const float bond_color[3], float bond_delta) {
  int i;
  int num_bonds;
  float3 *bond_positions;
  float3 *bond_directions;
  float *cylinder_positions;
  float *cylinder_directions;
  float *cylinder_colors;
  float *cylinder_radii;
  float *cylinder_lengths;
  gr3_drawspheremesh(n, positions, colors, radii);
  if (bond_delta < 0) return;
  num_bonds = calc_bonds(positions, n, bond_delta, &bond_positions, &bond_directions);
  if (num_bonds < 0) return;
  cylinder_positions = &(bond_positions[0].x);
  cylinder_directions = &(bond_directions[0].x);
  cylinder_colors = malloc(sizeof(float) * num_bonds * 3);
  cylinder_radii = malloc(sizeof(float) * num_bonds);
  cylinder_lengths = malloc(sizeof(float) * num_bonds);
  assert(cylinder_colors);
  assert(cylinder_radii);
  assert(cylinder_lengths);
  for (i = 0; i < num_bonds; i++) {
    cylinder_directions[3*i+0] -= cylinder_positions[3*i+0];
    cylinder_directions[3*i+1] -= cylinder_positions[3*i+1];
    cylinder_directions[3*i+2] -= cylinder_positions[3*i+2];
    cylinder_colors[3*i+0] = bond_color[0];
    cylinder_colors[3*i+1] = bond_color[1];
    cylinder_colors[3*i+2] = bond_color[2];
    cylinder_radii[i] = bond_radius;
    cylinder_lengths[i] = sqrt(cylinder_directions[3*i+0]*cylinder_directions[3*i+0]+cylinder_directions[3*i+1]*cylinder_directions[3*i+1]+cylinder_directions[3*i+2]*cylinder_directions[3*i+2]);
  }
  gr3_drawcylindermesh(num_bonds, cylinder_positions, cylinder_directions, cylinder_colors, cylinder_radii, cylinder_lengths);
  free(cylinder_positions);
  free(cylinder_directions);
  free(cylinder_colors);
  free(cylinder_radii);
  free(cylinder_lengths);
}

static void put_in_cells(const float *positions, double3 min, uchar3 *cells, unsigned int *position_in_cell, unsigned int *atoms_per_cell, double3 cell_size, int3 dim, int n) {
    uchar3 c;
    int i;
    unsigned int cell;
    for (i = 0; i < n; i++) {
        c.x = (int)((positions[3*i+0]-min.x)/cell_size.x);
        c.y = (int)((positions[3*i+1]-min.y)/cell_size.y);
        c.z = (int)((positions[3*i+2]-min.z)/cell_size.z);
        cell = CELL_IDX(c, dim);
        position_in_cell[i] = (*(atoms_per_cell + cell))++;
        cells[i]=c;
    }
}

static void sort_atoms(const float *positions, float3 *particles, uchar3 *cells_ordered, uchar3 *cells, int3 dim, unsigned int *cell_offset, unsigned int *position_in_cell, int n) {
    int i;
    int c;
    for (i = 0; i < n; i++) {
        float3 p;
        p.x = positions[3*i+0];
        p.y = positions[3*i+1];
        p.z = positions[3*i+2];
        c = CELL_IDX(cells[i], dim);
        particles[cell_offset[c]+position_in_cell[i]] = p;
        cells_ordered[cell_offset[c]+position_in_cell[i]] = cells[i];
    }
}

static unsigned int calculate_bonds(float3 *particles, uchar3* cells, int3 dim, float bond_length, float3 **bond_start_ptr, float3 **bond_end_ptr, unsigned int *cell_offset, unsigned int n) {
    int ix, iy, iz;
    unsigned int i, ic, c2_index, allocated = 0, number_of_bonds = 0;
    float3 p, p2;
    float d;
    uchar3 c, c2;
    float3 *bond_start = NULL;
    float3 *bond_end = NULL;
    for (i = 0; i < n; i++) {
        p = particles[i];
        c = cells[i];
        for (iz=c.z-1; iz<=c.z+1; iz++) {
            if (iz<0 || iz>=dim.z) continue;
            for (iy=c.y-1; iy<=c.y+1; iy++) {
                if (iy<0 || iy>=dim.y) continue;
                for (ix=c.x-1; ix<=c.x+1; ix++) {
                    if (ix<0 || ix>=dim.x) continue;
                    c2.x = ix;
                    c2.y = iy;
                    c2.z = iz;
                    c2_index = CELL_IDX(c2, dim);
                    for (ic=cell_offset[c2_index]; ic<cell_offset[c2_index+1]; ic++) {
                        p2 = particles[ic];
                        if(i<=ic) continue;
                        d = (p.x-p2.x) * (p.x-p2.x) + (p.y-p2.y) * (p.y-p2.y) + (p.z-p2.z) * (p.z-p2.z);
                        if (d + EPS > bond_length)
                            continue;
                        if (++number_of_bonds >= allocated * n) {
                            allocated++;
                            bond_start = (float3*)realloc(bond_start, n * allocated * sizeof(float3));
                            bond_end = (float3*)realloc(bond_end, n * allocated * sizeof(float3));
                            assert(bond_start);
                            assert(bond_end);
                        }
                        bond_start[number_of_bonds-1] = p;
                        bond_end[number_of_bonds-1] = p2;
                    }
                }
            }
        }
    }
    bond_start_ptr[0] = bond_start;
    bond_end_ptr[0] = bond_end;
    return number_of_bonds;
}


static float gr3_min(const float *values, int n, int offset) {
    int i;
    float cur_min;
    cur_min = values[offset];
    for (i=0; i<n; i++) {
        if (values[3*i+offset] < cur_min) {
            cur_min = values[3*i+offset];
        }
    }
    return cur_min;
}

static float gr3_max(const float *values, int n, int offset) {
    int i;
    float cur_max;
    cur_max = values[offset];
    for (i=0; i<n; i++) {
        if (values[3*i+offset] > cur_max) {
            cur_max = values[3*i+offset];
        }
    }
    return cur_max;
}

static int calc_bonds(const float *positions, int num_atoms, float bond_length, float3 **start, float3 **end) {
    int3 dim;
    int i, num_cells, num_bonds = 0;
    unsigned int *position_in_cell, *atoms_per_cell, *cell_offset;
    uchar3 *cells_ordered, *cells;
    float3 *particles;
    double3 _min, cell_size;

    assert(num_atoms > 0);
    cells = calloc(num_atoms, sizeof(uchar3));
    _min.x = gr3_min(positions, num_atoms, 0);
    _min.y = gr3_min(positions, num_atoms, 1);
    _min.z = gr3_min(positions, num_atoms, 2);
    dim.x = (gr3_max(positions, num_atoms, 0) - _min.x) / bond_length + 1;
    dim.y = (gr3_max(positions, num_atoms, 1) - _min.y) / bond_length + 1;
    dim.z = (gr3_max(positions, num_atoms, 2) - _min.z) / bond_length + 1;
    num_cells = dim.x * dim.y * dim.z;

    position_in_cell = calloc(num_atoms, sizeof(unsigned int));
    atoms_per_cell = calloc(num_cells, sizeof(unsigned int));

    cell_size.x = bond_length;
    cell_size.y = bond_length;
    cell_size.z = bond_length;

    put_in_cells(positions, _min, cells, position_in_cell, atoms_per_cell, cell_size, dim, num_atoms);

    cell_offset = (unsigned int *)malloc((num_cells + 1) * sizeof(unsigned int));
    cell_offset[0] = 0;
    for (i=1; i <= num_cells; i++) {
        cell_offset[i] = atoms_per_cell[i-1] + cell_offset[i-1];
    }

    cells_ordered = (uchar3 *)malloc(num_atoms * sizeof(uchar3));
    particles = calloc(num_atoms, sizeof(float3));
    sort_atoms(positions, particles, cells_ordered, cells, dim, cell_offset, position_in_cell, num_atoms);

    *start = NULL;
    *end = NULL;
    num_bonds = calculate_bonds(particles, cells_ordered, dim, bond_length * bond_length, (float3 **)start, (float3 **)end, cell_offset, num_atoms);

    free(cells);
    free(position_in_cell);
    free(atoms_per_cell);
    free(cell_offset);
    free(cells_ordered);
    free(particles);

    return num_bonds;
}
#undef EPS
#undef CELL_IDX
