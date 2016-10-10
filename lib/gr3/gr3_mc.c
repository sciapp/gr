/*!\file gr3_mc.c
 *
 * Optimized OpenMP implementation of the marching cubes algorithm.
 *
 * This code is based on Paul Bourke's Marching Cubes implementation
 * (http://paulbourke.net/geometry/polygonise/)
 *
 * Creates an indexed mesh to reduce the number of vertices to calculate.
 * This is done by caching generated vertices depending on their location.
 * Multiple threads create independent meshes.
 * Caches values between adjacent cubes.
 *
 * Fabian Beule
 * 2014-02-10
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gr3.h"
#include "gr3_mc_data.h"
#ifdef _OPENMP
#include <omp.h>
#endif

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define INDEX(x,y,z) ((x)*mcdata.stride[0]+(y)*mcdata.stride[1]+(z)*mcdata.stride[2])
#define IDX2D(y,z) ((y)*mcdata.dim[2] + (z))

/* speedup does not grow much with a high number of threads */
#define THREADLIMIT 16

/* for smaller function headers */
typedef struct {
    const GR3_MC_DTYPE *data;
    GR3_MC_DTYPE isolevel;
    int dim[3];
    int stride[3];
    double step[3];
    double offset[3];
} mcdata_t;

/* calculate the gradient via difference qoutient */
static gr3_coord_t getgrad(mcdata_t mcdata, int x, int y, int z)
{
    int v[3];
    int neigh[3][2];
    int i;
    gr3_coord_t n;

	v[0] = x;
	v[1] = y;
	v[2] = z;

    for (i = 0; i < 3; i++) {
        if (v[i] > 0)
            neigh[i][0] = v[i] - 1;
        else
            neigh[i][0] = v[i];
        if (v[i] < mcdata.dim[i] - 1)
            neigh[i][1] = v[i] + 1;
        else
            neigh[i][1] = v[i];
    }
    n.x = (float) (mcdata.data[INDEX(neigh[0][1], y, z)]
           - mcdata.data[INDEX(neigh[0][0], y, z)])
          / (neigh[0][1] - neigh[0][0]) / mcdata.step[0];
    n.y = (float) (mcdata.data[INDEX(x, neigh[1][1], z)]
           - mcdata.data[INDEX(x, neigh[1][0], z)])
          / (neigh[1][1] - neigh[1][0]) / mcdata.step[1];
    n.z = (float) (mcdata.data[INDEX(x, y, neigh[2][1])]
           - mcdata.data[INDEX(x, y, neigh[2][0])])
          / (neigh[2][1] - neigh[2][0]) / mcdata.step[2];

    return n;
}

/* interpolate points and calulate normals */
static void interpolate(mcdata_t mcdata, int px, int py, int pz, 
                        GR3_MC_DTYPE v1,
                        int qx,  int qy,  int qz,
                        GR3_MC_DTYPE v2,
                        gr3_coord_t *p, gr3_coord_t *n)
{
    double mu;
    gr3_coord_t n1, n2;
    double norm;

    if (ABS(mcdata.isolevel - v1) < 0.00001)
        mu = 0.0;
    else if (ABS(mcdata.isolevel - v2) < 0.00001)
        mu = 1.0;
    else if (ABS(v1 - v2) < 0.00001)
        mu = 0.5;
    else
        mu = 1.0 * (mcdata.isolevel - v1) / (v2 - v1);

    p->x = (px + mu * (qx - px)) * mcdata.step[0] + mcdata.offset[0];
    p->y = (py + mu * (qy - py)) * mcdata.step[1] + mcdata.offset[1];
    p->z = (pz + mu * (qz - pz)) * mcdata.step[2] + mcdata.offset[2];

    n1 = getgrad(mcdata, px, py, pz);
    n2 = getgrad(mcdata, qx, qy, qz);
    n->x = -(n1.x + mu * (n2.x - n1.x));
    n->y = -(n1.y + mu * (n2.y - n1.y));
    n->z = -(n1.z + mu * (n2.z - n1.z));

    norm = sqrt(n->x * n->x + n->y * n->y + n->z * n->z);
    if (norm > 0.0) {
        n->x /= norm;
        n->y /= norm;
        n->z /= norm;
    }
}

/*!
 * marching cubes algorithm for one x-layer.
 * created vertices are cached between calls using vindex.
 * vindex associates the intersected edge with the vertex index.
 * the edge is identified by its location (low, high), direction (x, y, z)
 * and coordinates (py, pz) of its starting point.
 * direction and location are the first index:
 * (x, y_low, z_low, y_high, z_high) (see mc_edgeprop) 
 * second index is py * mcdata.dim[1] + pz.
 * py and pz are the coordinates of the lower one of both edge vertices
 */
static void layer(mcdata_t mcdata, int x, int **vindex,
                  unsigned int *num_vertices, gr3_coord_t **vertices,
                  gr3_coord_t **normals, unsigned int *vertcapacity,
                  unsigned int *num_faces, unsigned int **indices,
                  unsigned int *facecapacity)
{
    int i, j;
    int y, z;
    int cubeindex;
    GR3_MC_DTYPE cubeval[8]; /* also cache between adjacent cubes */

    for (y = 0; y < mcdata.dim[1] - 1; y++) {
        /* init z-cache */
        for (i = 0; i < 4; i++) {
            int zi = mc_zvertices[0][i];

            cubeval[mc_zvertices[1][i]] = mcdata.data[INDEX(
                x + mc_cubeverts[zi][0],
                y + mc_cubeverts[zi][1],
                0 + mc_cubeverts[zi][2])];
        }
        for (z = 0; z < mcdata.dim[2] - 1; z++) {
            cubeindex = 0;
            /* shift old values (z-cache) */
            for (i = 0; i < 4; i++) {
                int zi = mc_zvertices[0][i];

                cubeval[zi] = cubeval[mc_zvertices[1][i]];
                if (cubeval[zi] < mcdata.isolevel) {
                    cubeindex |= 1 << zi;
                }
            }
            /* read new cube values */
            for (i = 0; i < 4; i++) {
                int zi = mc_zvertices[1][i];

                cubeval[zi] = mcdata.data[INDEX(x + mc_cubeverts[zi][0],
                    y + mc_cubeverts[zi][1], z + mc_cubeverts[zi][2])];
                if (cubeval[zi] < mcdata.isolevel) {
                    cubeindex |= 1 << zi;
                }
            }
            if (cubeindex != 0 && cubeindex != 255) {
                /* create triangles */
                for (i = 0; i < mc_tricount[cubeindex]; i++) {
                    if (*facecapacity <= *num_faces) {
                        (*facecapacity) = (unsigned int) 
                                          (*num_faces * 1.5) + 50;
                        *indices = realloc(*indices, (*facecapacity)
                                           * 3 * sizeof(int));
                    }
                    /* create triangle vertices */
                    for (j = 0; j < 3; j++) {
                        int trival = mc_tritable[cubeindex][i*3+j];
                        const int *edge = mc_cubeedges[trival];
                        int dir = mc_edgeprop[trival];
                        int px = x + mc_cubeverts[edge[0]][0];
                        int py = y + mc_cubeverts[edge[0]][1];
                        int pz = z + mc_cubeverts[edge[0]][2];
                        /* lookup if vertex already exists */
                        int node = vindex[dir][IDX2D(py, pz)];
                        if (node < 0) {
                            /* it does not, create it */
                            GR3_MC_DTYPE v1 = cubeval[edge[0]];
                            GR3_MC_DTYPE v2 = cubeval[edge[1]];
                            if (*vertcapacity <= *num_vertices) {
                                (*vertcapacity) = (unsigned int)
                                                  (*num_vertices * 1.5)
                                                  + 50;
                                *vertices =
                                    realloc(*vertices, (*vertcapacity)
                                            * sizeof(gr3_coord_t));
                                *normals =
                                    realloc(*normals, (*vertcapacity)
                                            * sizeof(gr3_coord_t));
                            }
                            node = *num_vertices;
                            interpolate(mcdata, px, py, pz, v1,
                                        x + mc_cubeverts[edge[1]][0],
                                        y + mc_cubeverts[edge[1]][1],
                                        z + mc_cubeverts[edge[1]][2], v2,
                                        *vertices + node,
                                        *normals + node);
                            vindex[dir][IDX2D(py, pz)] = node;
                            (*num_vertices)++;
                        }
                        /* add vertex index to the element array */
                        (*indices)[*num_faces * 3 + j] = node;
                    }
                    (*num_faces)++;
                }
            }
        }
    }
}

/*!
 * handle consecutive calls to layer
 */
static void layerblock(mcdata_t mcdata, int from, int to,
                       unsigned int *num_vertices, gr3_coord_t **vertices,
                       gr3_coord_t **normals, unsigned int *num_faces,
                       unsigned int **faces)
{
    int x;
    int y;
    int z;
    unsigned int vertcapacity;
    unsigned int facecapacity;
    /* cache for the vertex indices of the x-layer
     * [x, y_bot, z_bot, y_top, z_top] */
    int *vindex[5], *ntmp;

    *num_vertices = 0;
    vertcapacity = 0;
    *vertices = NULL;
    *normals = NULL;
    *num_faces = 0;
    facecapacity = 0;
    *faces = NULL;

    vindex[0] = malloc(5 * mcdata.dim[1] * mcdata.dim[2] * sizeof(int));
    vindex[1] = vindex[0] + mcdata.dim[1] * mcdata.dim[2];
    vindex[2] = vindex[0] + 2 * mcdata.dim[1] * mcdata.dim[2];
    vindex[3] = vindex[0] + 3 * mcdata.dim[1] * mcdata.dim[2];
    vindex[4] = vindex[0] + 4 * mcdata.dim[1] * mcdata.dim[2];

    for (y = 0; y < mcdata.dim[1]; y++) {
        for (z = 0; z < mcdata.dim[2]; z++) {
            vindex[0][IDX2D(y, z)] = -1;
            vindex[3][IDX2D(y, z)] = -1;
            vindex[4][IDX2D(y, z)] = -1;
        }
    }
    /*
     * iterate layer-by-layer through the data
     * create an indexed mesh
     * indices are cached in vindex[direction of the edge][y, z]
     * the top cache becomes the bottom in the next iterarion
     */
    for (x = from; x < to; x++) {
        ntmp = vindex[1];
        vindex[1] = vindex[3];
        vindex[3] = ntmp;
        ntmp = vindex[2];
        vindex[2] = vindex[4];
        vindex[4] = ntmp;
        for (y = 0; y < mcdata.dim[1]; y++) {
            for (z = 0; z < mcdata.dim[2]; z++) {
                vindex[0][IDX2D(y, z)] = -1;
                vindex[3][IDX2D(y, z)] = -1;
                vindex[4][IDX2D(y, z)] = -1;
            }
        }
        layer(mcdata, x, vindex,
              num_vertices, vertices, normals, &vertcapacity,
              num_faces, faces, &facecapacity);
    }
    free(vindex[0]);
}

/*!
 * Create an isosurface (as indexed mesh) from voxel data
 * with the marching cubes algorithm.
 * This function manages the parallelization:
 * Divide the data into blocks along the x-axis. Allocate memory,
 * call layerblock and merge the individual meshes into a single one.
 *
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
 * \param [out] num_vertices  number of vertices created
 * \param [out] vertices      array of vertex coordinates
 * \param [out] normals       array of vertex normal vectors
 * \param [out] num_indices   number of indices created
 *                            (3 times the number of triangles)
 * \param [out] indices       array of vertex indices that make the triangles
 */
GR3API void gr3_triangulateindexed(const GR3_MC_DTYPE *data,
                       GR3_MC_DTYPE isolevel,
                       unsigned int dim_x, unsigned int dim_y,
                       unsigned int dim_z, unsigned int stride_x,
                       unsigned int stride_y, unsigned int stride_z,
                       double step_x, double step_y, double step_z,
                       double offset_x, double offset_y, double offset_z,
                       unsigned int *num_vertices, gr3_coord_t **vertices,
                       gr3_coord_t **normals, unsigned int *num_indices,
                       unsigned int **indices)
{
    int num_threads;
    unsigned int num_faces;
    unsigned int *num_t_vertices, *num_t_faces, **t_faces;
    gr3_coord_t **t_vertices, **t_normals;
    unsigned int *vertblock, *faceblock;
    mcdata_t mcdata;
#if defined(_OPENMP) && defined(THREADLIMIT)
    int max_threads;

    max_threads = omp_get_max_threads();
    if (max_threads > THREADLIMIT)
        omp_set_num_threads(THREADLIMIT);
#endif

    if (stride_x == 0)
        stride_x = dim_z * dim_y;
    if (stride_y == 0)
        stride_y = dim_z;
    if (stride_z == 0)
        stride_z = 1;

    mcdata.data = data;
    mcdata.isolevel = isolevel;
    mcdata.dim[0] = dim_x;
    mcdata.dim[1] = dim_y;
    mcdata.dim[2] = dim_z;
    mcdata.stride[0] = stride_x;
    mcdata.stride[1] = stride_y;
    mcdata.stride[2] = stride_z;
    mcdata.step[0] = step_x;
    mcdata.step[1] = step_y;
    mcdata.step[2] = step_z;
    mcdata.offset[0] = offset_x;
    mcdata.offset[1] = offset_y;
    mcdata.offset[2] = offset_z;

    *num_vertices = 0;
    *vertices = NULL;
    *normals = NULL;
    *num_indices = 0;
    *indices = NULL;

#ifdef _OPENMP
#pragma omp parallel default(none) shared(num_threads, num_t_vertices, \
t_vertices, t_normals, num_t_faces, t_faces, mcdata, \
vertblock, faceblock, num_vertices, num_faces, vertices, normals, indices)
#endif
    {
        int thread_id;
        unsigned int from, to;
        unsigned int i;
#ifdef _OPENMP
#pragma omp single
#endif
        {
            /* allocate temporary memory for each thread */
#ifdef _OPENMP
            num_threads = omp_get_num_threads();
#else
            num_threads = 1;
#endif
            num_t_vertices = malloc(num_threads * sizeof(unsigned int));
            t_vertices = malloc(num_threads * sizeof(gr3_coord_t *));
            t_normals = malloc(num_threads * sizeof(gr3_coord_t *));
            num_t_faces = malloc(num_threads * sizeof(unsigned int));
            t_faces = malloc(num_threads * sizeof(unsigned int *));
        }
        /* create a mesh per thread */
#ifdef _OPENMP
        thread_id = omp_get_thread_num();
#else
        thread_id = 0;
#endif
        from = thread_id * (mcdata.dim[0] - 1) / num_threads;
        to = (thread_id + 1) * (mcdata.dim[0] - 1) / num_threads;
        num_t_vertices[thread_id] = 0;
        t_vertices[thread_id] = NULL;
        t_normals[thread_id] = NULL;
        num_t_faces[thread_id] = 0;
        t_faces[thread_id] = NULL;
        layerblock(mcdata, from, to,
                   num_t_vertices + thread_id,
                   t_vertices + thread_id, t_normals + thread_id,
                   num_t_faces + thread_id, t_faces + thread_id);
#ifdef _OPENMP
#pragma omp barrier
#pragma omp single
#endif
        {
            /* calculate beginning indices of thread blocks */
            vertblock = malloc((num_threads + 1) * sizeof(unsigned int));
            vertblock[0] = 0;
            faceblock = malloc((num_threads + 1) * sizeof(unsigned int));
            faceblock[0] = 0;
            for (i = 0; i < (unsigned int) num_threads; i++) {
                vertblock[i + 1] = vertblock[i] + num_t_vertices[i];
                faceblock[i + 1] = faceblock[i] + num_t_faces[i];
            }
            *num_vertices = vertblock[num_threads];
            num_faces = faceblock[num_threads];
            *vertices =
                realloc(*vertices, *num_vertices * sizeof(gr3_coord_t));
            *normals =
                realloc(*normals, *num_vertices * sizeof(gr3_coord_t));
            *indices =
                realloc(*indices, num_faces * 3 * sizeof(unsigned int));
        }
        /* copy thread meshes into the arrays */
        memmove(*vertices + vertblock[thread_id],
                t_vertices[thread_id],
                num_t_vertices[thread_id] * sizeof(gr3_coord_t));
        memmove(*normals + vertblock[thread_id],
                t_normals[thread_id],
                num_t_vertices[thread_id] * sizeof(gr3_coord_t));
        /* translate thread indices to global indices */
        for (i = 0; i < num_t_faces[thread_id]; i++) {
            (*indices)[(faceblock[thread_id] + i) * 3 + 0]
                = t_faces[thread_id][i * 3 + 0] + vertblock[thread_id];
            (*indices)[(faceblock[thread_id] + i) * 3 + 1]
                = t_faces[thread_id][i * 3 + 1] + vertblock[thread_id];
            (*indices)[(faceblock[thread_id] + i) * 3 + 2]
                = t_faces[thread_id][i * 3 + 2] + vertblock[thread_id];
        }
        free(t_vertices[thread_id]);
        free(t_normals[thread_id]);
        free(t_faces[thread_id]);
    }
    free(faceblock);
    free(vertblock);
    free(t_faces);
    free(num_t_faces);
    free(t_normals);
    free(t_vertices);
    free(num_t_vertices);
    *num_indices = num_faces * 3;
#if defined(_OPENMP) && defined(THREADLIMIT)
    omp_set_num_threads(max_threads);
#endif
}

/*!
 * Create an isosurface (as mesh) from voxel data
 * with the marching cubes algorithm.
 * This function calls gr3_triangulateindexed and copies the values.
 *
 * \param [in]  data          the volume (voxel) data
 * \param [in]  isolevel      value where the isosurface will be extracted
 * \param [in]  dim_x         number of elements in x-direction
 * \param [in]  dim_y         number of elements in y-direction
 * \param [in]  dim_z         number of elements in z-direction
 * \param [in]  stride_x      number of elements to step when traversing
 *                           the data in x-direction
 * \param [in]  stride_y      number of elements to step when traversing
 *                           the data in y-direction
 * \param [in]  stride_z      number of elements to step when traversing
 *                           the data in z-direction
 * \param [in]  step_x        distance between the voxels in x-direction
 * \param [in]  step_y        distance between the voxels in y-direction
 * \param [in]  step_z        distance between the voxels in z-direction
 * \param [in]  offset_x      coordinate origin
 * \param [in]  offset_y      coordinate origin
 * \param [in]  offset_z      coordinate origin
 * \param [out] triangles_p   array of triangle data
 *
 * \returns the number of triangles created
 */
GR3API unsigned int gr3_triangulate(const GR3_MC_DTYPE *data,
                GR3_MC_DTYPE isolevel,
                unsigned int dim_x, unsigned int dim_y, unsigned int dim_z,
                unsigned int stride_x, unsigned int stride_y,
                unsigned int stride_z, double step_x, double step_y,
                double step_z, double offset_x, double offset_y,
                double offset_z, gr3_triangle_t **triangles_p)
{
    unsigned int num_vertices;
    gr3_coord_t *vertices, *normals;
    unsigned int num_indices;
    unsigned int *indices;
    unsigned int i, j;
#if defined(_OPENMP) && defined(THREADLIMIT)
    int max_threads;

    max_threads = omp_get_max_threads();
    if (max_threads > THREADLIMIT)
        omp_set_num_threads(THREADLIMIT);
#endif

    gr3_triangulateindexed(data, isolevel,
                           dim_x, dim_y, dim_z,
                           stride_x, stride_y, stride_z,
                           step_x, step_y, step_z,
                           offset_x, offset_y, offset_z,
                           &num_vertices, &vertices, &normals,
                           &num_indices, &indices);

    *triangles_p = malloc(num_indices / 3 * sizeof(gr3_triangle_t));
#ifdef _OPENMP
#pragma omp parallel for default(none) private(j) \
shared(num_indices, triangles_p, indices, vertices, normals)
#endif
    for (i = 0; i < num_indices / 3; i++) {
        for (j = 0; j < 3; j++) {
            (*triangles_p)[i].vertex[j] = vertices[indices[i * 3 + j]];
            (*triangles_p)[i].normal[j] = normals[indices[i * 3 + j]];
        }
    }
    free(vertices);
    free(normals);
    free(indices);

#if defined(_OPENMP) && defined(THREADLIMIT)
    omp_set_num_threads(max_threads);
#endif
    return num_indices / 3;
}
