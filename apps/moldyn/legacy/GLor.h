/*!\file GLor.h
 * The GLor header
 */
#ifndef GLOR_H_INCLUDED
#define GLOR_H_INCLUDED

#ifndef GLORAPI
#define GLORAPI
#endif

/*!
 * The constants in this enumeration can be used to specify various properties
 * when calling glorInit().
 */
typedef enum _GLORInitAttribute
{
  kGLORIAEndOfAttributeList, /*!< This constant is used as a delimiter for the
                                  attribute list. It is guaranteed to be zero.
                              */
  kGLORIAFramebufferWidth,   /*!< The next element of the attribute list will
                                  be used as the width of the framebuffer used
                                  for rendering. Default: 512 */
  kGLORIAFramebufferHeight   /*!< The next element of the attribute list will
                                  be used as the height of the framebuffer
                                  used for rendering. Default: 512 */
} GLORInitAttribute;

/*!
 * The contants in this enumeration are error codes returned by GLor functions.
 */
typedef enum _GLORError
{
  kGLORENoError,             /*!< The function was successful */
  kGLOREInvalidValue,        /*!< The function failed because of an invalid value */
  kGLOREInvalidAttribute,    /*!< The function failed because of an invalid attribute */
  kGLOREInitError,           /*!< Initialization failed */
  kGLOREOpenGLError,         /*!< An OpenGL error occured */
  kGLOREOutOfMemory,         /*!< GLor was unable to allocate required memory. If this
                                  error occurs, GLor state is undefined. */
  kGLORENotInitialized,      /*!< A function was called before initializing GLor. */
  kGLORECameraNotInitialized /*!< glorGetPixmap() was called before initializing the Camera */
} GLORError;

/*!
 * This struct holds the color information of a pixel.
 */
typedef struct _GLORPixel
{
  unsigned char r; /*!< The red color component */
  unsigned char g; /*!< The green color component */
  unsigned char b; /*!< The blue color component */
  unsigned char a; /*!< The alpha color component */
} GLORPixel;

/*!
 * This type is used for meshes in GLor. It is an index to an internal array
 * of meshes.
 */
typedef unsigned int GLORMesh;

GLORAPI GLORError glorInit(GLORInitAttribute *attrib_list);
GLORAPI void glorTerminate(void);
GLORAPI const char *glorGetRenderpathString(void);
GLORAPI GLORError glorClear(void);
GLORAPI GLORError glorGetPixmap(GLORPixel *pixmap, unsigned int width, unsigned int height);
GLORAPI GLORError glorCreateMesh(GLORMesh *mesh, unsigned int n, const double *vertices, const double *normals,
                                 const double *colors);
GLORAPI void glorDrawMesh(GLORMesh mesh, unsigned int n, const double *positions, const double *directions,
                          const double *ups, const double *colors, const double *scales);
GLORAPI void glorDeleteMesh(GLORMesh mesh);
GLORAPI void glorCameraLookAt(double camera_x, double camera_y, double camera_z, double center_x, double center_y,
                              double center_z, double up_x, double up_y, double up_z);
GLORAPI GLORError glorCameraProjectionParameters(double vertical_field_of_view, double zNear, double zFar);
GLORAPI void glorLightDirection(double x, double y, double z);
GLORAPI void glorSetBackgroundColor(float red, float green, float blue, float alpha);

GLORAPI const char *glorErrorString(GLORError error);
GLORAPI void glorSetLogCallback(void (*glorLog_func)(const char *log_message));


GLORAPI void glorDrawConeMesh(unsigned int n, const double *positions, const double *directions, const double *colors,
                              const double *radii, const double *lengths);
GLORAPI void glorDrawCylinderMesh(unsigned int n, const double *positions, const double *directions,
                                  const double *colors, const double *radii, const double *lengths);
GLORAPI void glorDrawSphereMesh(unsigned int n, const double *positions, const double *colors, const double *radii);
#endif
