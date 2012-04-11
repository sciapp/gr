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
                                                occurs, gr3 state is undefined.
                                            */
#define GR3_ERROR_NOT_INITIALIZED 6        /*!< A function was called before
                                                initializing gr3. */
#define GR3_ERROR_CAMERA_NOT_INITIALIZED 7 /*!< gr3_getpixmap() was called
                                                before initializing the camera
                                            */

GR3API int         gr3_init(int *attrib_list);
GR3API void        gr3_terminate(void);
GR3API const char *gr3_getrenderpathstring(void);
GR3API int         gr3_clear(void);
GR3API int         gr3_drawscene(float xmin, float ymin, float xmax, float ymax, int pixelWidth, int pixelHeight);
GR3API int         gr3_getpovray(int *pixmap, int width, int height);
GR3API int         gr3_getpixmap(int *pixmap, int width, int height);
GR3API int         gr3_writehtml(const char *filename, int width, int height);
GR3API int         gr3_createmesh(int *mesh, int n, const float *vertices, const float *normals, const float *colors);
GR3API void        gr3_drawmesh(int mesh, int n, const float *positions, const float *directions, const float *ups, const float *colors, const float *scales);
GR3API void        gr3_deletemesh(int mesh);
GR3API void        gr3_cameralookat(float camera_x, float camera_y, float camera_z, float center_x, float center_y, float center_z, float up_x,  float up_y,  float up_z);
GR3API int         gr3_setcameraprojectionparameters(float vertical_field_of_view, float zNear, float zFar);
GR3API void        gr3_setlightdirection(float x, float y, float z);
GR3API void        gr3_setbackgroundcolor(float red, float green, float blue, float alpha);

GR3API const char *gr3_geterrorstring(int error);
GR3API void        gr3_setlogcallback(void (*gr3_log_func)(const char *log_message));

GR3API void        gr3_drawconemesh(int n, const float *positions, const float *directions, const float *colors, const float *radii, const float *lengths);
GR3API void        gr3_drawcylindermesh(int n, const float *positions, const float *directions, const float *colors, const float *radii, const float *lengths);
GR3API void        gr3_drawspheremesh(int n, const float *positions, const float *colors, const float *radii);

#ifdef _WIN32
    #ifdef __cplusplus
        }
    #endif
#endif
#endif
