/*!\file gr3.h
 * The gr3 header
 */
#ifndef GR3_H_INCLUDED
#define GR3_H_INCLUDED

#ifdef _WIN32
    #include <windows.h>	/* required for all Windows applications */
    #define GR3API __declspec(dllexport)

    #ifdef __cplusplus
        extern "C" {
    #endif
#else    
    #ifdef __cplusplus
        #define GR3API extern "C"
    #else
        #define GR3API
    #endif
#endif

/*!
 * These constants can be used to specify various properties 
 * when calling gr3_init().
 */
#define GR3_IA_END_OF_LIST 0         /*!< This constant is used as a delimiter
                                          for the attribute list. */
#define GR3_IA_FRAMEBUFFER_WIDTH 1   /*!< The next element of the attribute list
                                          will be used as the width of the
                                          framebuffer used for rendering.
                                          Default: 512 */
#define GR3_IA_FRAMEBUFFER_HEIGHT 2  /*!< The next element of the attribute list
                                          will be used as the height of the  
                                          framebuffer used for rendering.
                                          Default: 512 */

/*!
 * These contants are error codes returned by gr3 functions.
 */
#define GR3_ERROR_NONE 0                   /*!< The function was successful */
#define GR3_ERROR_INVALID_VALUE 1          /*!< The function failed because of
                                                an invalid value */
#define GR3_ERROR_INVALID_ATTRIBUTE 2      /*!< The function failed because of
                                                an invalid attribute */
#define GR3_ERROR_INIT_FAILED 3            /*!< Initialization failed */
#define GR3_ERROR_OPENGL_ERR 4             /*!< An OpenGL error occured */
#define GR3_ERROR_OUT_OF_MEM 5             /*!< gr3 was unable to allocate
                                                required memory. If this error
                                                occurs, gr3 state is undefined. */
#define GR3_ERROR_NOT_INITIALIZED 6        /*!< A function was called before
                                                initializing gr3. */
#define GR3_ERROR_CAMERA_NOT_INITIALIZED 7 /*!< gr3_getpixmap() was called
                                                before initializing the camera */
#define GR3_ERROR_UNKNOWN_FILE_EXTENSION 8
#define GR3_ERROR_CANNOT_OPEN_FILE 9
#define GR3_ERROR_EXPORT 10

#define GR3_QUALITY_OPENGL_NO_SSAA   0
#define GR3_QUALITY_OPENGL_2X_SSAA   2
#define GR3_QUALITY_OPENGL_4X_SSAA   4
#define GR3_QUALITY_OPENGL_8X_SSAA   8
#define GR3_QUALITY_OPENGL_16X_SSAA  16
#define GR3_QUALITY_POVRAY_NO_SSAA   1
#define GR3_QUALITY_POVRAY_2X_SSAA   3
#define GR3_QUALITY_POVRAY_4X_SSAA   5
#define GR3_QUALITY_POVRAY_8X_SSAA   9
#define GR3_QUALITY_POVRAY_16X_SSAA  17

#define GR3_DRAWABLE_OPENGL 1
#define GR3_DRAWABLE_GKS 2

#define GR3_MC_DTYPE unsigned short

#define GR3_PROJECTION_PERSPECTIVE 0
#define GR3_PROJECTION_PARALLEL 1

#define GR3_SURFACE_DEFAULT     0 /*!< default behavior */
#define GR3_SURFACE_NORMALS     1 /*!< interpolate the vertex normals
                                       from the gradient */
#define GR3_SURFACE_FLAT        2 /*!< set all z-coordinates to zero */
#define GR3_SURFACE_GRTRANSFORM 4 /*!< use gr_inqwindow, gr_inqspace
                                       and gr_inqscale to transform the
                                       data to NDC coordinates */
#define GR3_SURFACE_GRCOLOR     8 /*!< color the surface according to
                                       the current gr colormap */
#define GR3_SURFACE_GRZSHADED  16 /*!< like GR3_SURFACE_GRCOLOR, but
                                       use the z-value directly as
                                       color index */

typedef struct {
  float x, y, z;
} gr3_coord_t;

typedef struct {
  gr3_coord_t vertex[3];
  gr3_coord_t normal[3];
} gr3_triangle_t;

GR3API int          gr3_init(int *attrib_list);
GR3API void         gr3_free(void *pointer);
GR3API void         gr3_terminate(void);
GR3API int          gr3_geterror(int clear, int *line, const char **file);
GR3API const char  *gr3_getrenderpathstring(void);
GR3API const char  *gr3_geterrorstring(int error);
GR3API void         gr3_setlogcallback(void (*gr3_log_func)(const char *log_message));
GR3API int          gr3_clear(void);
GR3API void         gr3_usecurrentframebuffer();
GR3API void         gr3_useframebuffer(unsigned int framebuffer);

GR3API int          gr3_setquality(int quality);
GR3API int          gr3_getimage(int width, int height, int use_alpha, char *pixels);
GR3API int          gr3_export(const char *filename, int width, int height);
GR3API int          gr3_drawimage(float xmin, float xmax, float ymin, float ymax, int width, int height, int drawable_type);

GR3API int          gr3_createmesh_nocopy(int *mesh, int n, float *vertices, float *normals, float *colors);
GR3API int          gr3_createmesh(int *mesh, int n, const float *vertices, const float *normals, const float *colors);
GR3API int          gr3_createindexedmesh_nocopy(int *mesh, int number_of_vertices, float *vertices, float *normals, float *colors, int number_of_indices, int *indices);
GR3API int          gr3_createindexedmesh(int *mesh, int number_of_vertices, const float *vertices, const float *normals, const float *colors, int number_of_indices, const int *indices);
GR3API void         gr3_drawmesh(int mesh, int n, const float *positions, const float *directions, const float *ups, const float *colors, const float *scales);
GR3API void         gr3_deletemesh(int mesh);

GR3API void         gr3_cameralookat(float camera_x, float camera_y, float camera_z, float center_x, float center_y, float center_z, float up_x,  float up_y,  float up_z);
GR3API int          gr3_setcameraprojectionparameters(float vertical_field_of_view, float zNear, float zFar);
GR3API int          gr3_getcameraprojectionparameters(float *vfov, float *znear, float *zfar);
GR3API void         gr3_setlightdirection(float x, float y, float z);
GR3API void         gr3_setbackgroundcolor(float red, float green, float blue, float alpha);
          
GR3API int          gr3_createheightmapmesh(const float *heightmap, int num_columns, int num_rows);
GR3API void         gr3_drawheightmap(const float *heightmap, int num_columns, int num_rows, const float *positions, const float *scales);

GR3API void         gr3_drawconemesh(int n, const float *positions, const float *directions, const float *colors, const float *radii, const float *lengths);
GR3API void         gr3_drawcylindermesh(int n, const float *positions, const float *directions, const float *colors, const float *radii, const float *lengths);
GR3API void         gr3_drawspheremesh(int n, const float *positions, const float *colors, const float *radii);
GR3API void         gr3_drawcubemesh(int n, const float *positions, const float *directions, const float *ups, const float *colors, const float *scales);

          
GR3API void         gr3_setobjectid(int id);
GR3API int          gr3_selectid(int x, int y, int width, int height, int *selection_id);
GR3API void         gr3_getviewmatrix(float *m);
GR3API void         gr3_setviewmatrix(const float *m);
GR3API int          gr3_getprojectiontype(void);
GR3API void         gr3_setprojectiontype(int type);

GR3API unsigned int gr3_triangulate(const GR3_MC_DTYPE *data,
                        GR3_MC_DTYPE isolevel, unsigned int dim_x,
                        unsigned int dim_y, unsigned int dim_z,
                        unsigned int stride_x, unsigned int stride_y,
                        unsigned int stride_z, double step_x, double step_y,
                        double step_z, double offset_x, double offset_y,
                        double offset_z, gr3_triangle_t **triangles_p);

GR3API void         gr3_triangulateindexed(const GR3_MC_DTYPE *data,
                        GR3_MC_DTYPE isolevel,
                        unsigned int dim_x, unsigned int dim_y,
                        unsigned int dim_z, unsigned int stride_x,
                        unsigned int stride_y, unsigned int stride_z,
                        double step_x, double step_y, double step_z,
                        double offset_x, double offset_y, double offset_z,
                        unsigned int *num_vertices, gr3_coord_t **vertices,
                        gr3_coord_t **normals, unsigned int *num_indices,
                        unsigned int **indices);

GR3API int          gr3_createisosurfacemesh(int *mesh, GR3_MC_DTYPE *data,
                        GR3_MC_DTYPE isolevel,
                        unsigned int dim_x, unsigned int dim_y,
                        unsigned int dim_z, unsigned int stride_x,
                        unsigned int stride_y, unsigned int stride_z,
                        double step_x, double step_y, double step_z,
                        double offset_x, double offset_y, double offset_z);

GR3API int gr3_createsurfacemesh(int *mesh, int nx, int ny,
                                 float *px, float *py, float *pz,
                                 int option);

GR3API void gr3_drawmesh_grlike(int mesh, int n, const float *positions,
                                const float *directions, const float *ups,
                                const float *colors, const float *scales);

GR3API void gr3_drawsurface(int mesh);

GR3API void gr3_surface(int nx, int ny, float *px, float *py, float *pz,
                            int option);

GR3API int gr3_drawtubemesh(int n, float *points, float *colors, float *radii,
                            int num_steps, int num_segments);
          
GR3API int gr3_createtubemesh(int *mesh, int n, const float *points,
                              const float *colors, const float *radii,
                              int num_steps, int num_segments);

GR3API void gr3_drawspins(int n,
                          const float *positions,
                          const float *directions,
                          const float *colors,
                          float cone_radius, float cylinder_radius,
                          float cone_height, float cylinder_height);

GR3API void gr3_drawmolecule(int n,
                             const float *positions,
                             const float *colors,
                             const float *radii,
                             float bond_radius,
                             const float bond_color[3],
                             float bond_delta);

            
GR3API void gr3_createxslicemesh(int *mesh, const GR3_MC_DTYPE *data, unsigned int ix,
                                 unsigned int dim_x, unsigned int dim_y,
                                 unsigned int dim_z, unsigned int stride_x,
                                 unsigned int stride_y, unsigned int stride_z,
                                 double step_x, double step_y, double step_z,
                                 double offset_x, double offset_y, double offset_z);

GR3API void gr3_createyslicemesh(int *mesh, const GR3_MC_DTYPE *data, unsigned int iy,
                                 unsigned int dim_x, unsigned int dim_y,
                                 unsigned int dim_z, unsigned int stride_x,
                                 unsigned int stride_y, unsigned int stride_z,
                                 double step_x, double step_y, double step_z,
                                 double offset_x, double offset_y, double offset_z);

GR3API void gr3_createzslicemesh(int *mesh, const GR3_MC_DTYPE *data, unsigned int iz,
                                 unsigned int dim_x, unsigned int dim_y,
                                 unsigned int dim_z, unsigned int stride_x,
                                 unsigned int stride_y, unsigned int stride_z,
                                 double step_x, double step_y, double step_z,
                                 double offset_x, double offset_y, double offset_z);

GR3API void gr3_drawxslicemesh(const GR3_MC_DTYPE *data, unsigned int ix,
                               unsigned int dim_x, unsigned int dim_y,
                               unsigned int dim_z, unsigned int stride_x,
                               unsigned int stride_y, unsigned int stride_z,
                               double step_x, double step_y, double step_z,
                               double offset_x, double offset_y, double offset_z);

GR3API void gr3_drawyslicemesh(const GR3_MC_DTYPE *data, unsigned int iy,
                               unsigned int dim_x, unsigned int dim_y,
                               unsigned int dim_z, unsigned int stride_x,
                               unsigned int stride_y, unsigned int stride_z,
                               double step_x, double step_y, double step_z,
                               double offset_x, double offset_y, double offset_z);

GR3API void gr3_drawzslicemesh(const GR3_MC_DTYPE *data, unsigned int iz,
                               unsigned int dim_x, unsigned int dim_y,
                               unsigned int dim_z, unsigned int stride_x,
                               unsigned int stride_y, unsigned int stride_z,
                               double step_x, double step_y, double step_z,
                               double offset_x, double offset_y, double offset_z);

GR3API void gr3_drawtrianglesurface(int n, const float *triangles);

#ifdef _WIN32
    #ifdef __cplusplus
        }
    #endif
#endif
#endif
