#include "gr3.h"

int main()
{
  int mesh;
  int indexed_mesh;
  gr3_setalphamode(GR3_TRANSPARENCY_FILTER);
  {
    float vertex_positions[9] = {0, 0, 0, 1, 0, 0, 0, 1, 0};
    float vertex_normals[9] = {0, 0, 1, 0, 0, 1, 0, 0, 1};
    float vertex_colors[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
    int indices[3] = {0, 1, 2};
    gr3_createmesh(&mesh, 3, vertex_positions, vertex_normals, vertex_colors);
    gr3_createindexedmesh(&indexed_mesh, 3, vertex_positions, vertex_normals, vertex_colors, 3, indices);
  }
  gr3_setbackgroundcolor(0, 0, 0, 0);
  gr3_cameralookat(0, 0, 1, 0, 0, 0, 0, 1, 0);
  gr3_setorthographicprojection(-6, 6, -6, 6, -6, 6);
  {
    int i, j;
    for (i = 0; i < 11; i++)
      {
        for (j = 0; j < 11; j++)
          {
            float positions[3] = {i - 5, j - 5, 0};
            float directions[3] = {0, 0, 1};
            float ups[3] = {0, 1, 0};
            float colors[6] = {1, j / 10., j / 10., i / 10.0, i / 10.0, i / 10.0};
            float scales[3] = {0.5, 0.5, 0.5};
            float radii[1] = {0.25};
            gr3_drawmesh(indexed_mesh, 1, positions, directions, ups, colors, scales);
            gr3_drawspheremesh(1, positions, colors, radii);
          }
      }
  }
  gr3_export("transparency_test.png", 1000, 1000);
  return 0;
}