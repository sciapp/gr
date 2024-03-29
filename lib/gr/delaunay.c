#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libqhull_r/qhull_ra.h"

#include "gr.h"

#ifdef isnan
#define is_nan(a) isnan(a)
#else
#define is_nan(x) ((x) != (x))
#endif

void gr_delaunay(int npoints, const double *x, const double *y, int *ntri, int **triangles)
{
  qhT qh_qh;
  qhT *qh = &qh_qh;
  coordT *points = NULL;
  facetT *facet;
  vertexT *vertex, **vertexp;
  int i, max_facet_id;
  int *tri_indices = NULL;
  int indices[3], *indicesp;
  int curlong, totlong;
  const int ndim = 2;
  int *tri = NULL;
  int cnt = 0;

  *ntri = 0;
  *triangles = NULL;

  points = (coordT *)malloc(npoints * ndim * sizeof(coordT));
  if (points != NULL)
    {

      for (i = 0; i < npoints; ++i)
        {
          if (is_nan(x[i]) || is_nan(y[i])) continue;
          points[2 * cnt] = x[i];
          points[2 * cnt + 1] = y[i];
          cnt += 1;
        }

      qh_meminit(qh, stderr);

      /* Perform Delaunay triangulation */
      if (qh_new_qhull(qh, ndim, cnt, points, False, "qhull d Qt QbB Qz", NULL, stderr) == qh_ERRnone)
        {
          /* Split facets so that they only have 3 points each */
          qh_triangulate(qh);

          /* Determine ntri and max_facet_id */
          FORALLfacets
          {
            if (!facet->upperdelaunay) (*ntri)++;
          }

          max_facet_id = qh->facet_id - 1;

          /* Create array to map facet id to triangle index */
          tri_indices = (int *)malloc((max_facet_id + 1) * sizeof(int));
          if (tri_indices != NULL)
            {

              tri = (int *)malloc(*ntri * 3 * sizeof(int));
              if (tri != NULL)
                {
                  *triangles = tri;

                  /* Determine triangles array and set tri_indices array */
                  i = 0;
                  FORALLfacets
                  {
                    if (!facet->upperdelaunay)
                      {
                        tri_indices[facet->id] = i++;

                        indicesp = indices;
                        FOREACHvertex_(facet->vertices) *indicesp++ = qh_pointid(qh, vertex->point);

                        *tri++ = (facet->toporient ? indices[0] : indices[2]);
                        *tri++ = indices[1];
                        *tri++ = (facet->toporient ? indices[2] : indices[0]);
                      }
                    else
                      tri_indices[facet->id] = -1;
                  }
                }
              else
                fprintf(stderr, "Could not allocate triangle array\n");

              free(tri_indices);
            }
          else
            fprintf(stderr, "Could not allocate triangle map\n");
        }
      else
        fprintf(stderr, "Error in Delaunay triangulation calculation\n");

      qh_freeqhull(qh, !qh_ALL);
      qh_memfreeshort(qh, &curlong, &totlong);
      if (curlong || totlong) fprintf(stderr, "Could not free all allocated memory\n");

      free(points);
    }
  else
    fprintf(stderr, "Could not allocate point array\n");
}
