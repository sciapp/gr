#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "gr3.h"
#include "gr3_internals.h"

int gr3_getpovray_(char *pixels, int width, int height, int use_alpha, int ssaa_factor) {
  int i;
#ifdef GR3_USE_WIN
  char *povfile = malloc(40);
  char *pngfile = malloc(40);
  sprintf(povfile,"./gr3.%lu.pov",(long unsigned)GetCurrentProcessId());
  sprintf(pngfile,"./gr3.%lu.png",(long unsigned)GetCurrentProcessId());
#else
  char *povfile = malloc(40);
  char *pngfile = malloc(40);
  sprintf(povfile,"/tmp/gr3.%d.pov",getpid());
  sprintf(pngfile,"/tmp/gr3.%d.png",getpid());
#endif
  gr3_export_pov_(povfile, width, height);
  {
    int res;
    char *povray_call = malloc(strlen(povfile)+strlen(povfile)+80);
#ifdef GR3_USE_WIN
    sprintf(povray_call,"megapov +I%s +O%s +W%d +H%d -D +UA +FN +A +R%d 2>NUL",povfile,pngfile,width,height, ssaa_factor);
#else
    sprintf(povray_call,"povray +I%s +O%s +W%d +H%d -D +UA +FN +A +R%d 2>/dev/null",povfile,pngfile,width,height, ssaa_factor);
#endif
    res = system(povray_call);
    free(povray_call);
    if (use_alpha) {
      res = gr3_readpngtomemory_((int *)pixels,pngfile,width,height);
      if (res) {
        RETURN_ERROR(GR3_ERROR_EXPORT);
      }
    } else {
      char *raw_pixels = malloc(width*height*4);
      if (!raw_pixels) {
        RETURN_ERROR(GR3_ERROR_OUT_OF_MEM);
      }
      res = gr3_readpngtomemory_((int *)raw_pixels,pngfile,width,height);
      if (res) {
        free(raw_pixels);
        RETURN_ERROR(GR3_ERROR_EXPORT);
      }
      for (i = 0; i < width*height; i++) {
        pixels[3*i+0] = raw_pixels[4*i+0];
        pixels[3*i+1] = raw_pixels[4*i+1];
        pixels[3*i+2] = raw_pixels[4*i+2];
      }
      free(raw_pixels);
    }
    
  }
  remove(povfile);
  remove(pngfile);
  free(povfile);
  free(pngfile);
  return GR3_ERROR_NONE;
}

int gr3_export_pov_(const char *filename, int width, int height) {
  int i, j, k, l;
  FILE *povfp;
  GR3_DrawList_t_ *draw;
  
  
  povfp = fopen(filename, "w");
  if (!povfp) {
    RETURN_ERROR(GR3_ERROR_CANNOT_OPEN_FILE);
  }
  
  fprintf(povfp,"camera {\n");
  fprintf(povfp,"  up <0,1,0>\n");
  fprintf(povfp,"  right <-%f,0,0>\n",1.0*width/height);
  fprintf(povfp,"  location <%f, %f, %f>\n", context_struct_.camera_x, context_struct_.camera_y, context_struct_.camera_z);
  fprintf(povfp,"  look_at <%f, %f, %f>\n", context_struct_.center_x, context_struct_.center_y, context_struct_.center_z);
  fprintf(povfp,"  sky <%f, %f, %f>\n", context_struct_.up_x, context_struct_.up_y, context_struct_.up_z);
  fprintf(povfp,"  angle %f\n", context_struct_.vertical_field_of_view);
  fprintf(povfp,"}\n");
  
  if (context_struct_.light_dir[0] == 0 &&
      context_struct_.light_dir[1] == 0 &&
      context_struct_.light_dir[2] == 0
      ) {
    GLfloat camera_pos[3];
    camera_pos[0] = context_struct_.camera_x;
    camera_pos[1] = context_struct_.camera_y;
    camera_pos[2] = context_struct_.camera_z;
    fprintf(povfp,"light_source { <%f, %f, %f> color rgb <1.0, 1.0, 1.0> }\n", camera_pos[0], camera_pos[1], camera_pos[2]);
    fprintf(povfp,"light_source { <%f, %f, %f> color rgb <1.0, 1.0, 1.0> }\n", -camera_pos[0], -camera_pos[1], camera_pos[2]);
  } else {
    fprintf(povfp,"light_source { <%f, %f, %f> color rgb <1.0, 1.0, 1.0> }\n", context_struct_.light_dir[0], context_struct_.light_dir[1], context_struct_.light_dir[2]);
    fprintf(povfp,"light_source { <%f, %f, %f> color rgb <1.0, 1.0, 1.0> }\n", -context_struct_.light_dir[0], -context_struct_.light_dir[1], context_struct_.light_dir[2]);
  }
  fprintf(povfp,"background { color rgb <%f, %f, %f> }\n", context_struct_.background_color[0], context_struct_.background_color[1], context_struct_.background_color[2]);
  draw = context_struct_.draw_list_;
  while (draw) {
    gr3_sortindexedmeshdata(draw->mesh);
    switch(context_struct_.mesh_list_[draw->mesh].data.type) {
      case kMTSphereMesh:
        for(i = 0; i < draw->n; i++) {
          fprintf(povfp,"sphere {\n");
          fprintf(povfp,"  <%f, %f, %f>, %f\n",draw->positions[i*3+0],draw->positions[i*3+1],draw->positions[i*3+2],draw->scales[i*3+0]);
          fprintf(povfp,"  texture {\n");
          fprintf(povfp,"    pigment { color rgb <%f, %f, %f> } finish { ambient 0.3 phong 1.0 }\n",draw->colors[i*3+0],draw->colors[i*3+1],draw->colors[i*3+2]);
          fprintf(povfp,"  }\n");
          fprintf(povfp,"}\n");
        }
        break;
      case kMTCylinderMesh:
        for(i = 0; i < draw->n; i++) {
          float len_sq = draw->directions[i*3+0]*draw->directions[i*3+0] + draw->directions[i*3+1]*draw->directions[i*3+1] + draw->directions[i*3+2]*draw->directions[i*3+2];
          float len = sqrt(len_sq);
          fprintf(povfp,"cylinder {\n");
          fprintf(povfp,"  <%f, %f, %f>, <%f, %f, %f>, %f\n",draw->positions[i*3+0],draw->positions[i*3+1],draw->positions[i*3+2],draw->positions[i*3+0]+draw->directions[i*3+0]/len*draw->scales[i*3+2],draw->positions[i*3+1]+draw->directions[i*3+1]/len*draw->scales[i*3+2],draw->positions[i*3+2]+draw->directions[i*3+2]/len*draw->scales[i*3+2],draw->scales[i*3+0]);
          fprintf(povfp,"  texture {\n");
          fprintf(povfp,"    pigment { color rgb <%f, %f, %f> } finish { ambient 0.3 phong 1.0 }\n",draw->colors[i*3+0],draw->colors[i*3+1],draw->colors[i*3+2]);
          fprintf(povfp,"  }\n");
          fprintf(povfp,"}\n");
        }
        break;
      case kMTConeMesh:
        for(i = 0; i < draw->n; i++) {
          float len_sq = draw->directions[i*3+0]*draw->directions[i*3+0] + draw->directions[i*3+1]*draw->directions[i*3+1] + draw->directions[i*3+2]*draw->directions[i*3+2];
          float len = sqrt(len_sq);
          fprintf(povfp,"cone {\n");
          fprintf(povfp,"  <%f, %f, %f>, %f, <%f, %f, %f>, %f\n",draw->positions[i*3+0],draw->positions[i*3+1],draw->positions[i*3+2],draw->scales[i*3+0],draw->positions[i*3+0]+draw->directions[i*3+0]/len*draw->scales[i*3+2],draw->positions[i*3+1]+draw->directions[i*3+1]/len*draw->scales[i*3+2],draw->positions[i*3+2]+draw->directions[i*3+2]/len*draw->scales[i*3+2],0.0);
          fprintf(povfp,"  texture {\n");
          fprintf(povfp,"    pigment { color rgb <%f, %f, %f> } finish { ambient 0.3 phong 1.0 }\n",draw->colors[i*3+0],draw->colors[i*3+1],draw->colors[i*3+2]);
          fprintf(povfp,"  }\n");
          fprintf(povfp,"}\n");
        }
        break;
      case kMTIndexedMesh:
      case kMTNormalMesh:
        for(i = 0; i < draw->n; i++) {
          GLfloat model_matrix[4][4] = {{0}};
          const float *vertices = context_struct_.mesh_list_[draw->mesh].data.vertices;
          const float *normals = context_struct_.mesh_list_[draw->mesh].data.normals;
          const float *colors = context_struct_.mesh_list_[draw->mesh].data.colors;
          {
            int m;
            GLfloat forward[3], up[3], left[3];
            float tmp;
            /* Calculate an orthonormal base in IR^3, correcting the up vector
             * in case it is not perpendicular to the forward vector. This base
             * is used to create the model matrix as a base-transformation
             * matrix.
             */
            /* forward = normalize(&directions[i*3]); */
            tmp = 0;
            for (m = 0; m < 3; m++) {
              tmp+= draw->directions[i*3+m]*draw->directions[i*3+m];
            }
            tmp = sqrt(tmp);
            for (m = 0; m < 3; m++) {
              forward[m] = draw->directions[i*3+m]/tmp;
            }/* up = normalize(&ups[i*3]); */
            tmp = 0;
            for (m = 0; m < 3; m++) {
              tmp+= draw->ups[i*3+m]*draw->ups[i*3+m];
            }
            tmp = sqrt(tmp);
            for (m = 0; m < 3; m++) {
              up[m] = draw->ups[i*3+m]/tmp;
            }
            /* left = cross(forward,up); */
            for (m = 0; m < 3; m++) {
              left[m] = forward[(m+1)%3]*up[(m+2)%3] - up[(m+1)%3]*forward[(m+2)%3];
            }
            /* up = cross(left,forward); */
            for (m = 0; m < 3; m++) {
              up[m] = left[(m+1)%3]*forward[(m+2)%3] - forward[(m+1)%3]*left[(m+2)%3];
            }
            for (m = 0; m < 3; m++) {
              model_matrix[0][m] = -left[m];
              model_matrix[1][m] = up[m];
              model_matrix[2][m] = forward[m];
              model_matrix[3][m] = draw->positions[i*3+m];
            }
            model_matrix[3][3] = 1;
          }
          fprintf(povfp,"mesh {\n");
          for (j = 0; j < context_struct_.mesh_list_[draw->mesh].data.number_of_vertices/3; j++) {
            float red = (colors[j*9+0]+colors[j*9+3]+colors[j*9+6])/3.0;
            float green = (colors[j*9+1]+colors[j*9+4]+colors[j*9+7])/3.0;
            float blue = (colors[j*9+2]+colors[j*9+5]+colors[j*9+8])/3.0;
            fprintf(povfp,"#local tex = texture { pigment { color rgb <%f, %f, %f> } finish { ambient 0.3 phong 1.0 } }\n",draw->colors[i*3+0]*red,draw->colors[i*3+1]*green,draw->colors[i*3+2]*blue);
            fprintf(povfp,"  smooth_triangle {\n");
            for (k = 0; k < 3; k++) {
              float vertex1[4];
              float vertex2[4];
              float normal1[3];
              float normal2[3];
              for (l = 0; l < 3; l++) {
                vertex1[l] = draw->scales[i*3+l]*vertices[j*9+k*3+l];
              }
              vertex1[3] = 1;
              for (l = 0; l < 4; l++) {
                vertex2[l] = model_matrix[0][l]*vertex1[0]+model_matrix[1][l]*vertex1[1]+model_matrix[2][l]*vertex1[2]+model_matrix[3][l]*vertex1[3];
              }
              for (l = 0; l < 3; l++) {
                normal1[l] = normals[j*9+k*3+l];
              }
              vertex1[3] = 1;
              for (l = 0; l < 3; l++) {
                normal2[l] = model_matrix[0][l]*normal1[0]+model_matrix[1][l]*normal1[1]+model_matrix[2][l]*normal1[2];
              }
              fprintf(povfp,"    <%f, %f, %f>,",vertex2[0],vertex2[1],vertex2[2]);
              fprintf(povfp," <%f, %f, %f>",normal2[0],normal2[1],normal2[2]);
              if (k < 2) {
                fprintf(povfp,",");
              }
              fprintf(povfp,"\n");
            }
            fprintf(povfp,"    texture { tex }\n");
            fprintf(povfp,"  }\n");
          }
          fprintf(povfp,"}\n");
        }
        break;
      default:
        gr3_log_("Unknown mesh type");
        break;
    }
    draw = draw->next;
  }
  fclose(povfp);
  return 0;
}
