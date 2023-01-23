#include <stdlib.h>
#include <math.h>
#include "gr3.h"
#include "gr.h"

static void ellipsoid_data(int nx, int ny, int nz, float rx, float ry, float rz, float *data, int stride_x,
                           int stride_y, int stride_z)
{
  int ix, iy, iz;

  for (ix = 0; ix < nx; ix += 1)
    {
      float x = (2.0 * ix / (nx - 1) - 1.0) / rx;
      for (iy = 0; iy < nx; iy += 1)
        {
          float y = (2.0 * iy / (ny - 1) - 1.0) / ry;
          for (iz = 0; iz < nx; iz += 1)
            {
              float z = (2.0 * iz / (nz - 1) - 1.0) / rz;
              int index = stride_x * ix + stride_y * iy + stride_z * iz;
              data[index] = sqrt(x * x + y * y + z * z);
            }
        }
    }
}

int main(void)
{
  int i;
  int nx = 20;
  int ny = 20;
  int nz = 20;
  int stride_x = nz * ny;
  int stride_y = nz;
  int stride_z = 1;
  float *data = malloc(nx * ny * nz * sizeof(float));

  gr_setviewport(0, 1, 0, 1);
  for (i = 0; i < 150; i++)
    {
      ellipsoid_data(nx, ny, nz, 1.5, 1.0, sin(i * 0.05) * 0.5 + 1.0, data, stride_x, stride_y, stride_z);
      gr_clearws();
      gr_setspace3d(40, 60, 0, 2.5);
      gr_axes3d(0.5, 0.5, 0.5, -1, -1, -1, 2, 2, 2, 0.007);
      gr3_isosurface(nx, ny, nz, data, 0.5, NULL, NULL);
      gr_updatews();
    }
}
