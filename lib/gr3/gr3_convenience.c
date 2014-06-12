
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "gr.h"

#include "gr3.h"
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
  memcpy(vertices_old,icosahedron,n*3*3*sizeof(float));
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
  float colormap[72][3];
  
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
    for (i = 0; i < 72; i++) {
      int color;
      gr_inqcolor(i+8,&color);
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
          
          colors[array_offset+0] = colormap[(int)(height*71.5)][0];
          colors[array_offset+1] = colormap[(int)(height*71.5)][1];
          colors[array_offset+2] = colormap[(int)(height*71.5)][2];
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

