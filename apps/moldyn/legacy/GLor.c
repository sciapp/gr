/*!\file GLor.c
 * ToDo:
 * -
 * Bugs:
 * - glXCreatePbuffer -> "failed to create drawable" probably caused by being
 *      run in virtual box, other applications show the same error message
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h> /* for sqrt */

#ifdef _WIN32
#define GLORAPI __declspec(dllexport)
#endif

#include "GLor.h"

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279
#endif

#if defined(__APPLE__)
/* Core OpenGL (CGL) on Mac OS X */
#define GLOR_USE_CGL
#include <OpenGL/OpenGL.h>
/* OpenGL.h in Mac OS X 10.7 doesn't include gl.h anymore */
#include <OpenGL/gl.h>
#define GLORAPI
#elif defined(__linux__)
/* OpenGL Extension to the X Window System (GLX) on Linux */
#define GLOR_USE_GLX
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#define GLORAPI
#elif defined(_WIN32)
/* Windows */
#define GLOR_USE_WIN
#include <windows.h>
#include <GL/gl.h>
#include "GL/glext.h"
#else
#error "This operating system is currently not supported by GLor"
#endif

#if !(GL_ARB_framebuffer_object || GL_EXT_framebuffer_object)
#error "Neither GL_ARB_framebuffer_object nor GL_EXT_framebuffer_object \
            are supported!"
#endif

#undef GL_VERSION_2_1
#if GL_VERSION_2_1
#define GLOR_CAN_USE_VBO
#endif

#if defined(GLOR_USE_WIN)
#ifdef GLOR_CAN_USE_VBO
static PFNGLBUFFERDATAPROC glBufferData;
static PFNGLBINDBUFFERPROC glBindBuffer;
static PFNGLGENBUFFERSPROC glGenBuffers;
static PFNGLGENBUFFERSPROC glDeleteBuffers;
static PFNGLUSEPROGRAMPROC glUseProgram;
static PFNGLDELETESHADERPROC glDeleteShader;
static PFNGLLINKPROGRAMPROC glLinkProgram;
static PFNGLATTACHSHADERPROC glAttachShader;
static PFNGLCREATESHADERPROC glCreateShader;
static PFNGLCOMPILESHADERPROC glCompileShader;
static PFNGLCREATEPROGRAMPROC glCreateProgram;
static PFNGLDELETEPROGRAMPROC glDeleteProgram;
static PFNGLUNIFORM3FPROC glUniform3f;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
static PFNGLUNIFORM4FPROC glUniform4f;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
static PFNGLSHADERSOURCEPROC glShaderSource;
#endif
static PFNGLDRAWBUFFERSPROC glDrawBuffers;
static PFNGLBLENDCOLORPROC glBlendColor;
#ifdef GL_ARB_framebuffer_object
static PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
static PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
static PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
static PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
static PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
#endif
#ifdef GL_EXT_framebuffer_object
static PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
static PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
static PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
static PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
static PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
static PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
static PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
static PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
static PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
#endif
#endif

/*!
 * An instance of this struct contains all information from the
 * ::GLORInitAttribute list given to glorInit() or the default values from
 * ::GLORInitStruct_INITIALIZER.
 * \note Except for when glorInit() is run, the only existing instance of this
 * struct should be the one in #context_struct_.
 */
typedef struct _GLORInitStruct_
{
  unsigned int framebuffer_width;  /*!< The width of the framebuffer used for
                                       generating images */
  unsigned int framebuffer_height; /*!< The height of the framebuffer used for
                                      generating images */
} GLORInitStruct_;
/*!
 * The default values for the instances of GLORInitStruct_:\n
 * GLORInitStruct_::framebuffer_width = 512;\n
 * GLORInitStruct_::framebuffer_height = 512;
 */
#define GLORInitStruct_INITIALIZER \
  {                                \
    512, 512                       \
  }

/*!
 * Each call to glorDrawMesh() gets saved in an instance of this struct, so it
 * can be used for later drawing. They form a linked-list, with each draw call
 * pointing to the next one.
 */
typedef struct _GLORDrawList_
{
  GLORMesh mesh;               /*!< The id of the mesh that should be drawn */
  double *positions;           /*!< A list of 3 * _GLORDrawList_::n doubles.
                                    Each triple is one position
                                    for one mesh to be drawn. */
  double *directions;          /*!< A list of 3 * _GLORDrawList_::n doubles.
                                   Each triple is one (forward) direction
                                   for one mesh to be drawn. */
  double *ups;                 /*!< A list of 3 * _GLORDrawList_::n doubles.
                                          Each triple is one up vector
                                          for one mesh to be drawn. */
  double *colors;              /*!< A list of 3 * _GLORDrawList_::n doubles.
                                       Each triple is one color
                                       for one mesh to be drawn. */
  double *scales;              /*!< A list of 3 * _GLORDrawList_::n doubles.
                                       Each triple means the scaling factors
                                       for one mesh to be drawn. */
  unsigned int n;              /*!< The number of meshes to be drawn. */
  struct _GLORDrawList_ *next; /*!< The pointer to the next GLORDrawList_. */
} GLORDrawList_;

/*!
 * This union contains all information required for using a mesh. Either the
 * display list id, or the vertex buffer object id and number of vertices.
 * Instances of this unions are kept and
 * reference counted in the ::GLORMeshList_.
 */
typedef union _GLORMeshData_
{
  unsigned int display_list_id; /*!< The OpenGL display list of the mesh. */
  struct
  {
    unsigned int id;
    unsigned int number_of_vertices;
  } vertex_buffer_object;
} GLORMeshData_;

/*!
 * This struct is for managing ::GLORMeshData_ objects.
 * Instances of this array are meant to be kept in an array so they can be
 * accessed by an index. Each instance refers to the next free instance in the
 * list, so when the user needs a new one, a free one can be found just by
 * indexing the array. They are reference counted and are 'free' if there are
 * no references left and _GLORMeshList_::refcount reaches zero. A user can
 * only get one reference to the mesh by calling glorCreateMesh() and therefore
 * should only be able to release one by calling glorDeleteMesh(). This is
 * realized by saving a flag (whether a mesh is marked for deletion or not)
 * in _GLORMeshList_::marked_for_deletion.
 * \note Instances of this struct should only appear in the #context_struct_.
 * \note Reference counting is handled by calling glorMeshAddReference_() and
 * glorMeshRemoveReference_().
 */
typedef struct _GLORMeshList_
{
  GLORMeshData_ data;               /*!<  The data of the actual mesh managed by this
                                            struct. */
  unsigned int refcount;            /*!< A reference counter. If refcount reaches zero,
                                         the instance is unused or 'free' and can be used
                                         for a different mesh. */
  unsigned int marked_for_deletion; /*!< A flag whether the user called
                              glorDeleteMesh() for this mesh. */
  unsigned int next_free;           /*!< If refcount is zero (so this object is free),
                                        next_free is the index of the next free object
                                        in the array it is stored in. */
} GLORMeshList_;

/*!
 * This function pointer holds the function used by glorLog_(). It can be set
 * with glorSetLogCallback().
 */
static void (*glorLog_func_)(const char *log_message) = NULL;

/*!
 * This char array is returned by glorGetRenderpathString() if it is called
 * without calling glorInit() successfully first.
 */
static char not_initialized_[] = "Not initialized";

/*!
 * This struct holds all context data. All data that is dependent on GLor to
 * be initialized is saved here. It is set up by glorInit() and turned back
 * into its default state by glorTerminate().
 * \note #context_struct_ should be the only instance of this struct.
 */
typedef struct _GLORContextStruct_
{
  GLORInitStruct_ init_struct; /*!< The information given to glorInit().
                                  \note This member and its members must not
                                  be changed outside of glorInit() or
                                  glorTerminate().
                                  \sa _GLORInitStruct_ */

  int is_initialized; /*!< This flag is set to 1 if GLor was
                         initialized successfully. */

  int gl_is_initialized; /*!< This flag is set to 1 if an OpenGL context
                            has been created successfully. */

  void (*terminateGL)(void); /*!< This member holds a pointer to the
                                function which must be used for destroying
                                the OpenGL context.
                                \sa glorTerminateGL_WIN_()
                                \sa glorTerminateGL_GLX_()
                                \sa glorTerminateGL_CGL_()
                                */

  int fbo_is_initialized; /*!< This flag is set to 1 if a framebuffer
                             object has been created successfully. */

  void (*terminateFBO)(void); /*!< This member holds a pointer to the
                                 function which must be used for destroying
                                 the framebuffer object.
                                 \sa glorTerminateFBO_ARB_()
                                 \sa glorTerminateFBO_EXT_()
                                 */

  char *renderpath_string; /*!< This string holds information on the GLor
                              renderpath.
                              \sa glorGetRenderpathString()
                              \sa glorAppendToRenderpathString_()
                              */

  GLORDrawList_ *draw_list_; /*!< This member holds a pointer to the first
                                element of the linked draw list or NULL. */

  GLORMeshList_ *mesh_list_; /*!< This member holds a pointer to the mesh
                                list or NULL. */

  unsigned int mesh_list_first_free_; /*!< The index of the first free element
                                  in the mesh list*/
  unsigned int mesh_list_capacity_;   /*!< The number of elements in the mesh
                                    list */

  GLfloat view_matrix[4][4];     /*!< The view matrix used to transform vertices
                                      from world to eye space. */
  double vertical_field_of_view; /*!< The vertical field of view, used for the
                                  projection marix */
  double zNear;                  /*!< distance to the near clipping plane */
  double zFar;                   /*!< distance to the far clipping plane */

  GLfloat light_dir[4]; /*!< The direction of light + 0 for showing that it is
                                  not a position, but a direction */
  unsigned int use_vbo;

  GLORMesh cylinder_mesh;
  GLORMesh sphere_mesh;
  GLORMesh cone_mesh;
  GLfloat background_color[4];
  GLuint program;
} GLORContextStruct_;
/*!
 * The only instance of ::GLORContextStruct_. For documentation, see
 * ::_GLORContextStruct_.
 */
#define GLORContextStruct_INITIALIZER                                                                                  \
  {                                                                                                                    \
    GLORInitStruct_INITIALIZER, 0, 0, NULL, 0, NULL, not_initialized_, NULL, NULL, 0, 0, {{0}}, 0, 0, 0, {0, 0, 0, 0}, \
        0, 0, 0, 0, {0, 0, 0, 0}, 0                                                                                    \
  }
static GLORContextStruct_ context_struct_ = GLORContextStruct_INITIALIZER;

/* For documentation, see the definition. */
static int glorExtensionSupported_(const char *extension_name);
static void glorAppendToRenderpathString_(const char *string);
static void glorLog_(const char *log_message);
static void glorMeshAddReference_(GLORMesh mesh);
static void glorMeshRemoveReference_(GLORMesh mesh);
static void glorDoDrawMesh_(GLORMesh mesh, unsigned int n, const double *positions, const double *directions,
                            const double *ups, const double *colors, const double *scales);
static void glorCreateCylinderMesh_(void);
static void glorCreateSphereMesh_(void);
static void glorCreateConeMesh_(void);
#ifdef GLOR_USE_CGL
static GLORError glorInitGL_CGL_(void);
static void glorTerminateGL_CGL_(void);
#endif
#ifdef GLOR_USE_GLX
static GLORError glorInitGL_GLX_(void);
static void glorTerminateGL_GLX_Pbuffer_(void);
static void glorTerminateGL_GLX_Pixmap_(void);
#endif
#ifdef GLOR_USE_WIN
static GLORError glorInitGL_WIN_(void);
static void glorTerminateGL_WIN_(void);
#endif

#if GL_EXT_framebuffer_object
static GLORError glorInitFBO_EXT_(void);
static void glorTerminateFBO_EXT_(void);
#endif
#if GL_ARB_framebuffer_object
static GLORError glorInitFBO_ARB_(void);
static void glorTerminateFBO_ARB_(void);
#endif


/*!
 * This method initializes the GLor context.
 *
 * \param [in] attrib_list  This ::kGLORIAEndOfAttributeList-terminated list can
 *                          specify details about context creation. The
 *                          attributes which use unsigned integer values are
 *                          followed by these values.
 * \return
 * - ::kGLORENoError            on success
 * - ::kGLOREInvalidValue       if one of the attributes' values is out of the
 *                              allowed range
 * - ::kGLOREInvalidAttribute   if the list contained an unkown attribute or two
 *                              mutually exclusive attributes were both used
 * - ::kGLOREOpenGLError        if an OpenGL error occured
 * - ::kGLOREInitError          if an error occured during initialization of the
 *                              "window toolkit" (CGL, GLX, WIN...)
 * \sa glorTerminate()
 * \sa context_struct_
 * \sa GLORInitAttribute
 */
GLORAPI GLORError glorInit(GLORInitAttribute *attrib_list)
{
  int i;
  char *renderpath_string = "GLor";
  GLORError error;
  GLORInitStruct_ init_struct = GLORInitStruct_INITIALIZER;
  if (attrib_list)
    {
      for (i = 0; attrib_list[i] != kGLORIAEndOfAttributeList; i++)
        {
          switch (attrib_list[i])
            {
            case kGLORIAFramebufferWidth:
              init_struct.framebuffer_width = attrib_list[++i];
              if (attrib_list[i] <= 0) return kGLOREInvalidValue;
              break;
            case kGLORIAFramebufferHeight:
              init_struct.framebuffer_height = attrib_list[++i];
              if (attrib_list[i] <= 0) return kGLOREInvalidValue;
              break;
            default:
              return kGLOREInvalidAttribute;
            }
        }
    }
  context_struct_.init_struct = init_struct;

  context_struct_.renderpath_string = malloc(strlen(renderpath_string) + 1);
  strcpy(context_struct_.renderpath_string, renderpath_string);

  do
    {
      error = kGLOREInitError;
#if defined(GLOR_USE_CGL)
      error = glorInitGL_CGL_();
      if (error == kGLORENoError)
        {
          break;
        }
#endif
#if defined(GLOR_USE_GLX)
      error = glorInitGL_GLX_();
      if (error == kGLORENoError)
        {
          break;
        }
#endif
#if defined(GLOR_USE_WIN)
      error = glorInitGL_WIN_();
      if (error == kGLORENoError)
        {
          break;
        }
#endif
      glorTerminate();
      return error;
    }
  while (0);

/* GL_ARB_framebuffer_object is core since OpenGL 3.0 */
#if GL_ARB_framebuffer_object
  if (!strncmp((const char *)glGetString(GL_VERSION), "3.", 2) || glorExtensionSupported_("GL_ARB_framebuffer_object"))
    {
      error = glorInitFBO_ARB_();
      if (error)
        {
          glorTerminate();
          return error;
        }
    }
  else
#endif
#if GL_EXT_framebuffer_object
      if (glorExtensionSupported_("GL_EXT_framebuffer_object"))
    {
      error = glorInitFBO_EXT_();
      if (error)
        {
          glorTerminate();
          return error;
        }
    }
  else
#endif
    {
      glorTerminate();
      return kGLOREOpenGLError;
    }

#ifdef GLOR_CAN_USE_VBO
  if (strncmp((const char *)glGetString(GL_VERSION), "2.1", 3) >= 0)
    {
      context_struct_.use_vbo = 1;
    }
  if (context_struct_.use_vbo)
    {
      GLuint program;
      GLuint fragment_shader;
      GLuint vertex_shader;
      char *vertex_shader_source[] = {"#version 120\n",
                                      "uniform mat4 ProjectionMatrix;\n",
                                      "uniform mat4 ViewMatrix;\n",
                                      "uniform mat4 ModelMatrix;\n",
                                      "uniform vec3 LightDirection;\n",
                                      "uniform vec4 Scales;\n",

                                      "attribute vec3 in_Vertex;\n"
                                      "attribute vec3 in_Normal;\n"
                                      "attribute vec3 in_Color;\n"

                                      "varying vec4 Color;\n",
                                      "varying vec3 Normal;\n",

                                      "void main(void) {\n",
                                      "vec4 Position = ViewMatrix*ModelMatrix*(Scales*vec4(in_Vertex,1));\n",
                                      "gl_Position=ProjectionMatrix*Position;\n",
                                      "Normal = mat3(ViewMatrix)*mat3(ModelMatrix)*in_Normal;\n",
                                      "Color = vec4(in_Color,1);\n",
                                      "float diffuse = Normal.z;\n",
                                      "if (dot(LightDirection,LightDirection) > 0.001) {",
                                      "diffuse = dot(normalize(LightDirection),Normal);",
                                      "}",
                                      "diffuse = abs(diffuse);\n",
                                      "Color.rgb = diffuse*Color.rgb;"
                                      "}\n"};

      char *fragment_shader_source[] = {"#version 120\n",
                                        "varying vec4 Color;\n",
                                        "varying vec3 Normal;\n",
                                        "uniform mat4 ViewMatrix;\n",

                                        "void main(void) {\n",
                                        "gl_FragColor=vec4(Color.rgb,Color.a);\n",
                                        "}\n"};
      program = glCreateProgram();
      vertex_shader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertex_shader, sizeof(vertex_shader_source) / sizeof(char *),
                     (const GLchar **)vertex_shader_source, NULL);
      glCompileShader(vertex_shader);
      fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragment_shader, sizeof(fragment_shader_source) / sizeof(char *),
                     (const GLchar **)fragment_shader_source, NULL);
      glCompileShader(fragment_shader);
      glAttachShader(program, vertex_shader);
      glAttachShader(program, fragment_shader);
      glLinkProgram(program);
      glDeleteShader(vertex_shader);
      glDeleteShader(fragment_shader);
      context_struct_.program = program;
      glUseProgram(program);

      glorAppendToRenderpathString_("Vertex Buffer Objects");
    }
  else
#endif
    {
      glorAppendToRenderpathString_("Display Lists");
    }


  context_struct_.is_initialized = 1;

  glorAppendToRenderpathString_((const char *)glGetString(GL_VERSION));
  glorAppendToRenderpathString_((const char *)glGetString(GL_RENDERER));
  glorCreateCylinderMesh_();
  glorCreateSphereMesh_();
  glorCreateConeMesh_();
  return kGLORENoError;
}

/*!
 * This helper function checks whether an OpenGL extension is supported.
 * \param [in] extension_name The extension that should be checked for,
 *                       e.g. GL_EXT_framebuffer_object. The notation has to
 *                       be the same as in the GL_EXTENSIONS string.
 * \returns 1 on success, 0 otherwise.
 */
static int glorExtensionSupported_(const char *extension_name)
{
  const char *extension_string = (const char *)glGetString(GL_EXTENSIONS);
  return strstr(extension_string, extension_name) != NULL;
}

/*!
 * This function terminates the GLor context.
 * After calling this function, GLor is in the same state as when it was first
 * loaded, except for context-independent variables, i.e. the logging callback.
 * \note It is safe to call this function even if the context is not
 * initialized.
 * \sa glorInit()
 * \sa context_struct_
 */
GLORAPI void glorTerminate(void)
{
  if (context_struct_.gl_is_initialized)
    {
#ifdef GLOR_CAN_USE_VBO
      if (context_struct_.use_vbo)
        {
          glUseProgram(0);
          glDeleteProgram(context_struct_.program);
        }
#endif
      glorDeleteMesh(context_struct_.cylinder_mesh);
      glorDeleteMesh(context_struct_.sphere_mesh);
      glorDeleteMesh(context_struct_.cone_mesh);
      if (context_struct_.fbo_is_initialized)
        {
          unsigned int i;
          glorClear();
          for (i = 0; i < context_struct_.mesh_list_capacity_; i++)
            {
              if (context_struct_.mesh_list_[i].data.display_list_id != 0)
                {
                  glDeleteLists(context_struct_.mesh_list_[i].data.display_list_id, 1);
                  context_struct_.mesh_list_[i].data.display_list_id = 0;
                  context_struct_.mesh_list_[i].refcount = 0;
                  context_struct_.mesh_list_[i].marked_for_deletion = 0;
                }
            }

          free(context_struct_.mesh_list_);
          context_struct_.mesh_list_ = NULL;
          context_struct_.mesh_list_capacity_ = 0;
          context_struct_.mesh_list_first_free_ = 0;

          context_struct_.terminateFBO();
        }
      context_struct_.terminateGL();
    }
  context_struct_.is_initialized = 0;
  if (context_struct_.renderpath_string != not_initialized_)
    {
      free(context_struct_.renderpath_string);
      context_struct_.renderpath_string = not_initialized_;
    }
  {
    GLORContextStruct_ initializer = GLORContextStruct_INITIALIZER;
    context_struct_ = initializer;
  }
}

/*!
 * This function clears the draw list.
 * \returns
 * - ::kGLORENoError        on success
 * - ::kGLOREOpenGLError    if an OpenGL error occured
 * - ::kGLORENotInitialized if the function was called without
 *                          calling glorInit() first
 */
GLORAPI GLORError glorClear(void)
{
  glorLog_("glorClear();");

  if (context_struct_.is_initialized)
    {
      GLORDrawList_ *draw;
      while (context_struct_.draw_list_)
        {
          draw = context_struct_.draw_list_;
          context_struct_.draw_list_ = draw->next;
          glorMeshRemoveReference_(draw->mesh);
          free(draw->positions);
          free(draw->directions);
          free(draw->ups);
          free(draw->colors);
          free(draw->scales);
          free(draw);
        }

      if (glGetError() == GL_NO_ERROR)
        {
          return kGLORENoError;
        }
      else
        {
          return kGLOREOpenGLError;
        }
    }
  else
    {
      return kGLORENotInitialized;
    }
}

/*!
 * This function sets the background color.
 */
GLORAPI void glorSetBackgroundColor(float red, float green, float blue, float alpha)
{
  if (context_struct_.is_initialized)
    {
      context_struct_.background_color[0] = red;
      context_struct_.background_color[1] = green;
      context_struct_.background_color[2] = blue;
      context_struct_.background_color[3] = alpha;
    }
}

/*!
 * This function creates a GLORMesh from vertex position, normal and color data.
 * \param [out] mesh        a pointer to a GLORMesh
 * \param [in] vertices     the vertex positions
 * \param [in] normals      the vertex normals
 * \param [in] colors       the vertex colors, they should be white (1,1,1) if
 *                          you want to change the color for each drawn mesh
 * \param [in] n            the number of vertices in the mesh
 *
 * \returns
 *  - ::kGLORENoError       on success
 *  - ::kGLOREOpenGLError if an OpenGL error occured
 *  - ::kGLOREOutOfMemory if a memory allocation failed
 */
GLORAPI GLORError glorCreateMesh(GLORMesh *mesh, unsigned int n, const double *vertices, const double *normals,
                                 const double *colors)
{

  unsigned int i;
  void *mem;

  if (!context_struct_.is_initialized)
    {
      return kGLORENotInitialized;
    }

  *mesh = context_struct_.mesh_list_first_free_;
  if (context_struct_.mesh_list_first_free_ >= context_struct_.mesh_list_capacity_)
    {
      unsigned int new_capacity = context_struct_.mesh_list_capacity_ * 2;
      if (context_struct_.mesh_list_capacity_ == 0)
        {
          new_capacity = 8;
        }
      mem = realloc(context_struct_.mesh_list_, new_capacity * sizeof(*context_struct_.mesh_list_));
      if (mem == NULL)
        {
          return kGLOREOutOfMemory;
        }
      context_struct_.mesh_list_ = mem;
      while (context_struct_.mesh_list_capacity_ < new_capacity)
        {
          context_struct_.mesh_list_[context_struct_.mesh_list_capacity_].next_free =
              context_struct_.mesh_list_capacity_ + 1;
          context_struct_.mesh_list_[context_struct_.mesh_list_capacity_].refcount = 0;
          context_struct_.mesh_list_[context_struct_.mesh_list_capacity_].marked_for_deletion = 0;
          context_struct_.mesh_list_[context_struct_.mesh_list_capacity_].data.display_list_id = 0;
          context_struct_.mesh_list_[context_struct_.mesh_list_capacity_].data.vertex_buffer_object.id = 0;
          context_struct_.mesh_list_[context_struct_.mesh_list_capacity_].data.vertex_buffer_object.number_of_vertices =
              0;
          context_struct_.mesh_list_capacity_++;
        }
    }
  context_struct_.mesh_list_first_free_ = context_struct_.mesh_list_[*mesh].next_free;

  glorMeshAddReference_(*mesh);
#ifdef GLOR_CAN_USE_VBO
  if (context_struct_.use_vbo)
    {
      glGenBuffers(1, &context_struct_.mesh_list_[*mesh].data.vertex_buffer_object.id);
      context_struct_.mesh_list_[*mesh].data.vertex_buffer_object.number_of_vertices = n;
      glBindBuffer(GL_ARRAY_BUFFER, context_struct_.mesh_list_[*mesh].data.vertex_buffer_object.id);
      mem = malloc(n * 3 * 3 * sizeof(GLfloat));
      if (mem == NULL)
        {
          return kGLOREOutOfMemory;
        }
      for (i = 0; i < n; i++)
        {
          GLfloat *data = ((GLfloat *)mem) + i * 3 * 3;
          data[0] = vertices[i * 3 + 0];
          data[1] = vertices[i * 3 + 1];
          data[2] = vertices[i * 3 + 2];
          data[3] = normals[i * 3 + 0];
          data[4] = normals[i * 3 + 1];
          data[5] = normals[i * 3 + 2];
          data[6] = colors[i * 3 + 0];
          data[7] = colors[i * 3 + 1];
          data[8] = colors[i * 3 + 2];
        }
      glBufferData(GL_ARRAY_BUFFER, n * 3 * 3 * sizeof(GLfloat), mem, GL_STATIC_DRAW);
      free(mem);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
  else
#endif
    {
      context_struct_.mesh_list_[*mesh].data.display_list_id = glGenLists(1);
      glNewList(context_struct_.mesh_list_[*mesh].data.display_list_id, GL_COMPILE);
      glBegin(GL_TRIANGLES);
      for (i = 0; i < n; i++)
        {
          glColor3dv(colors + i * 3);
          glNormal3dv(normals + i * 3);
          glVertex3dv(vertices + i * 3);
        }
      glEnd();
      glEndList();
    }

  if (glGetError() != GL_NO_ERROR)
    return kGLOREOpenGLError;
  else
    return kGLORENoError;
}

/*!
 * This function adds a mesh to the draw list, so it will be drawn when the user
 * calls glorGetPixmap(). The given data stays owned by the user, a copy will be
 * saved in the draw list and the mesh reference counter will be increased.
 * \param [in] mesh         The mesh to be drawn
 * \param [in] positions    The positions where the meshes should be drawn
 * \param [in] directions   The forward directions the meshes should be facing
 *                          at
 * \param [in] ups          The up directions
 * \param [in] colors       The colors the meshes should be drawn in, it will be
 *                          multiplied with each vertex color
 * \param [in] scales       The scaling factors
 * \param [in] n            The number of meshes to be drawn
 * \note This function does not return an error code, because of its
 * asynchronous nature. If glorGetPixmap() returns a ::kGLOREOpenGLError, this
 * might be caused by this function saving unuseable data into the draw list.
 */
GLORAPI void glorDrawMesh(GLORMesh mesh, unsigned int n, const double *positions, const double *directions,
                          const double *ups, const double *colors, const double *scales)
{

  if (!context_struct_.is_initialized)
    {
      return;
    }
  else
    {
      GLORDrawList_ *p;

      GLORDrawList_ *draw = malloc(sizeof(GLORDrawList_));
      draw->mesh = mesh;
      draw->positions = malloc(sizeof(double) * n * 3);
      memcpy(draw->positions, positions, sizeof(double) * n * 3);
      draw->directions = malloc(sizeof(double) * n * 3);
      memcpy(draw->directions, directions, sizeof(double) * n * 3);
      draw->ups = malloc(sizeof(double) * n * 3);
      memcpy(draw->ups, ups, sizeof(double) * n * 3);
      draw->colors = malloc(sizeof(double) * n * 3);
      memcpy(draw->colors, colors, sizeof(double) * n * 3);
      draw->scales = malloc(sizeof(double) * n * 3);
      memcpy(draw->scales, scales, sizeof(double) * n * 3);
      draw->n = n;
      draw->next = NULL;
      glorMeshAddReference_(mesh);
      if (context_struct_.draw_list_ == NULL)
        {
          context_struct_.draw_list_ = draw;
        }
      else
        {
          p = context_struct_.draw_list_;
          while (p->next)
            {
              p = p->next;
            }
          p->next = draw;
        }
    }
}

/*!
 * This function does the actual mesh drawing. It will be called with the data
 * in the draw list.
 * \param [in] mesh         The mesh to be drawn
 * \param [in] positions    The positions where the meshes should be drawn
 * \param [in] directions   The forward directions the meshes should be facing
 *                          at
 * \param [in] ups          The up directions
 * \param [in] colors       The colors the meshes should be drawn in, it will be
 *                          multiplied with each vertex color
 * \param [in] scales       The scaling factors
 * \param [in] n            The number of meshes to be drawn
 * \returns
 *  - ::kGLORENoError on success
 *  - ::kGLOREOpenGLError if an OpenGL error occured
 */
static void glorDoDrawMesh_(GLORMesh mesh, unsigned int n, const double *positions, const double *directions,
                            const double *ups, const double *colors, const double *scales)
{
  unsigned int i, j;
  GLfloat forward[3], up[3], left[3];
  GLfloat model_matrix[4][4] = {{0}};
  double tmp;
  for (i = 0; i < n; i++)
    {
      {

        /* Calculate an orthonormal base in IR^3, correcting the up vector
         * in case it is not perpendicular to the forward vector. This base
         * is used to create the model matrix as a base-transformation
         * matrix.
         */
        /* forward = normalize(&directions[i*3]); */
        tmp = 0;
        for (j = 0; j < 3; j++)
          {
            tmp += directions[i * 3 + j] * directions[i * 3 + j];
          }
        tmp = sqrt(tmp);
        for (j = 0; j < 3; j++)
          {
            forward[j] = directions[i * 3 + j] / tmp;
          } /* up = normalize(&ups[i*3]); */
        tmp = 0;
        for (j = 0; j < 3; j++)
          {
            tmp += ups[i * 3 + j] * ups[i * 3 + j];
          }
        tmp = sqrt(tmp);
        for (j = 0; j < 3; j++)
          {
            up[j] = ups[i * 3 + j] / tmp;
          }
        /* left = cross(forward,up); */
        for (j = 0; j < 3; j++)
          {
            left[j] = forward[(j + 1) % 3] * up[(j + 2) % 3] - up[(j + 1) % 3] * forward[(j + 2) % 3];
          }
        /* up = cross(left,forward); */
        for (j = 0; j < 3; j++)
          {
            up[j] = left[(j + 1) % 3] * forward[(j + 2) % 3] - forward[(j + 1) % 3] * left[(j + 2) % 3];
          }
        if (!context_struct_.use_vbo)
          {
            for (j = 0; j < 3; j++)
              {
                model_matrix[0][j] = -left[j] * scales[i * 3 + 0];
                model_matrix[1][j] = up[j] * scales[i * 3 + 1];
                model_matrix[2][j] = forward[j] * scales[i * 3 + 2];
                model_matrix[3][j] = positions[i * 3 + j];
              }
          }
        else
          {
            for (j = 0; j < 3; j++)
              {
                model_matrix[0][j] = -left[j];
                model_matrix[1][j] = up[j];
                model_matrix[2][j] = forward[j];
                model_matrix[3][j] = positions[i * 3 + j];
              }
          }
        model_matrix[3][3] = 1;
      }
      glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      {
        float nil[4] = {0, 0, 0, 1};
        float one[4] = {1, 1, 1, 1};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, nil);
        glLightfv(GL_LIGHT0, GL_AMBIENT, nil);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, one);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, one);
      }
      glBlendColor(colors[i * 3 + 0], colors[i * 3 + 1], colors[i * 3 + 2], 1);
      glBlendFunc(GL_CONSTANT_COLOR, GL_ZERO);
      glEnable(GL_BLEND);
#ifdef GLOR_CAN_USE_VBO
      if (context_struct_.use_vbo)
        {
          glUniform4f(glGetUniformLocation(context_struct_.program, "Scales"), scales[3 * i + 0], scales[3 * i + 1],
                      scales[3 * i + 2], 1);
          glUniformMatrix4fv(glGetUniformLocation(context_struct_.program, "ModelMatrix"), 1, GL_FALSE,
                             &model_matrix[0][0]);
          glBindBuffer(GL_ARRAY_BUFFER, context_struct_.mesh_list_[mesh].data.vertex_buffer_object.id);
          glVertexAttribPointer(glGetAttribLocation(context_struct_.program, "in_Vertex"), 3, GL_FLOAT, GL_FALSE,
                                sizeof(GLfloat) * 3 * 3, (void *)(sizeof(GLfloat) * 3 * 0));
          glVertexAttribPointer(glGetAttribLocation(context_struct_.program, "in_Normal"), 3, GL_FLOAT, GL_FALSE,
                                sizeof(GLfloat) * 3 * 3, (void *)(sizeof(GLfloat) * 3 * 1));
          glVertexAttribPointer(glGetAttribLocation(context_struct_.program, "in_Color"), 3, GL_FLOAT, GL_FALSE,
                                sizeof(GLfloat) * 3 * 3, (void *)(sizeof(GLfloat) * 3 * 2));
          glEnableVertexAttribArray(glGetAttribLocation(context_struct_.program, "in_Vertex"));
          glEnableVertexAttribArray(glGetAttribLocation(context_struct_.program, "in_Normal"));
          glEnableVertexAttribArray(glGetAttribLocation(context_struct_.program, "in_Color"));
          glDrawArrays(GL_TRIANGLES, 0, context_struct_.mesh_list_[mesh].data.vertex_buffer_object.number_of_vertices);
        }
      else
#endif
        {
          glPushMatrix();
          glMultMatrixf(&model_matrix[0][0]);
          glCallList(context_struct_.mesh_list_[mesh].data.display_list_id);
          glPopMatrix();
        }
      glDisable(GL_BLEND);
    }
}

/*!
 * This function marks a mesh for deletion and removes the user's reference
 * from the mesh's referenc counter, so a user must not use the mesh after
 * calling this function. If the mesh is still in use for draw calls, the mesh
 * will not be truly deleted until glorClear() is called.
 * \param [in] mesh     The mesh that should be marked for deletion
 */
GLORAPI void glorDeleteMesh(GLORMesh mesh)
{
  glorLog_("glorDeleteMesh_();");
  if (!context_struct_.is_initialized)
    {
      return;
    }
  if (!context_struct_.mesh_list_[mesh].marked_for_deletion)
    {
      glorMeshRemoveReference_(mesh);
      context_struct_.mesh_list_[mesh].marked_for_deletion = 1;
    }
  else
    {
      glorLog_("Mesh already marked for deletion!");
    }
}

/*!
 * This function adds a reference to the meshes reference counter.
 * \param [in] mesh     The mesh to which a reference will be added
 */
static void glorMeshAddReference_(GLORMesh mesh)
{
  context_struct_.mesh_list_[mesh].refcount++;
}

/*!
 * This function removes a reference from the meshes reference counter and
 * deletes the mesh if the reference counter reaches zero.
 * \param [in] mesh     The mesh from which a reference will be removed
 */
static void glorMeshRemoveReference_(GLORMesh mesh)
{
  if (context_struct_.mesh_list_[mesh].refcount > 0)
    {
      context_struct_.mesh_list_[mesh].refcount--;
    }
  if (context_struct_.mesh_list_[mesh].refcount <= 0)
    {
#ifdef GLOR_CAN_USE_VBO
      if (context_struct_.use_vbo)
        {
          glDeleteBuffers(1, &context_struct_.mesh_list_[mesh].data.vertex_buffer_object.id);
        }
      else
#endif
        {
          glDeleteLists(context_struct_.mesh_list_[mesh].data.display_list_id, 1);
        }
      context_struct_.mesh_list_[mesh].data.display_list_id = 0;
      context_struct_.mesh_list_[mesh].refcount = 0;
      context_struct_.mesh_list_[mesh].marked_for_deletion = 0;
      if (context_struct_.mesh_list_first_free_ > mesh)
        {
          context_struct_.mesh_list_[mesh].next_free = context_struct_.mesh_list_first_free_;
          context_struct_.mesh_list_first_free_ = mesh;
        }
      else
        {
          unsigned int lastf = context_struct_.mesh_list_first_free_;
          unsigned int nextf = context_struct_.mesh_list_[lastf].next_free;
          while (nextf < mesh)
            {
              lastf = nextf;
              nextf = context_struct_.mesh_list_[lastf].next_free;
            }
          context_struct_.mesh_list_[lastf].next_free = mesh;
          context_struct_.mesh_list_[mesh].next_free = nextf;
        }
    }
}

/*!
 * This function sets the direction of light. If it is called with (0, 0, 0),
 * the light is always pointing into the same direction as the camera.
 * \param [in] x The x-component of the light's direction
 * \param [in] y The y-component of the light's direction
 * \param [in] z The z-component of the light's direction
 *
 */
GLORAPI void glorLightDirection(double x, double y, double z)
{
  if (!context_struct_.is_initialized)
    {
      return;
    }
  context_struct_.light_dir[0] = x;
  context_struct_.light_dir[1] = y;
  context_struct_.light_dir[2] = z;
}

/*!
 * This function sets the view matrix by getting the position of the camera, the
 * position of the center of focus and the direction which should point up. This
 * function takes effect when the next image is created. Therefore if you want
 * to take pictures of the same data from different perspectives, you can call
 * and  glorCameraLookAt(), glorGetPixmap(), glorCameraLookAt(),
 * glorGetPixmap(), ... without calling glorClear() and glorDrawMesh() again.
 * \param [in] camera_x The x-coordinate of the camera
 * \param [in] camera_y The y-coordinate of the camera
 * \param [in] camera_z The z-coordinate of the camera
 * \param [in] center_x The x-coordinate of the center of focus
 * \param [in] center_y The y-coordinate of the center of focus
 * \param [in] center_z The z-coordinate of the center of focus
 * \param [in] up_x The x-component of the up direction
 * \param [in] up_y The y-component of the up direction
 * \param [in] up_z The z-component of the up direction
 * \note Source: http://www.opengl.org/sdk/docs/man/xhtml/gluLookAt.xml
 * (as of 10/24/2011, licensed under SGI Free Software Licence B)
 */
GLORAPI void glorCameraLookAt(double camera_x, double camera_y, double camera_z, double center_x, double center_y,
                              double center_z, double up_x, double up_y, double up_z)
{
  int i, j;
  GLfloat view_matrix[4][4] = {{0}};
  GLdouble camera_pos[3];
  GLdouble center_pos[3];
  GLdouble up_dir[3];

  GLdouble F[3];
  GLdouble f[3];
  GLdouble up[3];
  GLdouble s[3];
  GLdouble u[3];
  GLdouble tmp;

  if (!context_struct_.is_initialized)
    {
      return;
    }

  camera_pos[0] = camera_x;
  camera_pos[1] = camera_y;
  camera_pos[2] = camera_z;
  center_pos[0] = center_x;
  center_pos[1] = center_y;
  center_pos[2] = center_z;
  up_dir[0] = up_x;
  up_dir[1] = up_y;
  up_dir[2] = up_z;


  for (i = 0; i < 3; i++)
    {
      F[i] = center_pos[i] - camera_pos[i];
    }
  /* f = normalize(F); */
  tmp = 0;
  for (i = 0; i < 3; i++)
    {
      tmp += F[i] * F[i];
    }
  tmp = sqrt(tmp);
  for (i = 0; i < 3; i++)
    {
      f[i] = F[i] / tmp;
    }
  /* up = normalize(up_dir); */
  tmp = 0;
  for (i = 0; i < 3; i++)
    {
      tmp += up_dir[i] * up_dir[i];
    }
  tmp = sqrt(tmp);
  for (i = 0; i < 3; i++)
    {
      up[i] = up_dir[i] / tmp;
    }
  /* s = cross(f,up); */
  for (i = 0; i < 3; i++)
    {
      s[i] = f[(i + 1) % 3] * up[(i + 2) % 3] - up[(i + 1) % 3] * f[(i + 2) % 3];
    }
  /* s = normalize(s); */
  tmp = 0;
  for (i = 0; i < 3; i++)
    {
      tmp += s[i] * s[i];
    }
  tmp = sqrt(tmp);
  for (i = 0; i < 3; i++)
    {
      s[i] = s[i] / tmp;
    }
  /* u = cross(s,f); */
  for (i = 0; i < 3; i++)
    {
      u[i] = s[(i + 1) % 3] * f[(i + 2) % 3] - f[(i + 1) % 3] * s[(i + 2) % 3];
    }

  /* u = normalize(u); */
  tmp = 0;
  for (i = 0; i < 3; i++)
    {
      tmp += u[i] * u[i];
    }
  tmp = sqrt(tmp);
  for (i = 0; i < 3; i++)
    {
      u[i] = u[i] / tmp;
    }
  for (i = 0; i < 3; i++)
    {
      view_matrix[i][0] = s[i];
      view_matrix[i][1] = u[i];
      view_matrix[i][2] = -f[i];
    }
  view_matrix[3][3] = 1;
  for (i = 0; i < 3; i++)
    {
      view_matrix[3][i] = 0;
      for (j = 0; j < 3; j++)
        {
          view_matrix[3][i] -= view_matrix[j][i] * camera_pos[j];
        }
    }
  memcpy(&context_struct_.view_matrix[0][0], &view_matrix[0][0], sizeof(view_matrix));
}

/*!
 * This function sets the projection parameters. This function takes effect
 * when the next image is created.
 * \param [in] vertical_field_of_view   This parameter is the vertical field of
 *                                      view in degrees. It must be greater than
 *                                      0 and less than 180.
 * \param [in] zNear                    The distance to the near clipping plane.
 * \param [in] zFar                     The distance to the far clipping plane.
 * \returns
 * - ::kGLORENoError      on success
 * - ::kGLOREInvalidValue if one (or more) of the arguments is out of its range
 * \note The ratio between zFar and zNear influences the precision of the depth
 * buffer, the greater \f$ \frac{zFar}{zNear} \f$, the more likely are errors. So
 * you should try to keep both values as close to each other as possible while
 * making sure everything you want to be visible, is visible.
 */
GLORAPI GLORError glorCameraProjectionParameters(double vertical_field_of_view, double zNear, double zFar)
{
  if (!context_struct_.is_initialized)
    {
      return kGLORENotInitialized;
    }
  if (zFar < zNear || zNear <= 0 || vertical_field_of_view >= 180 || vertical_field_of_view <= 0)
    {
      return kGLOREInvalidValue;
    }
  context_struct_.vertical_field_of_view = vertical_field_of_view;
  context_struct_.zNear = zNear;
  context_struct_.zFar = zFar;
  return kGLORENoError;
}

/*!
 * This function iterates over the draw list and draws the image using OpenGL.
 */
static void glorDraw_(GLuint width, GLuint height)
{
  glorLog_("glorDraw_();");
  {
    GLdouble fovy = context_struct_.vertical_field_of_view;
    GLdouble zNear = context_struct_.zNear;
    GLdouble zFar = context_struct_.zFar;

    GLfloat projection_matrix[4][4] = {{0}};

    {
      /* Source: http://www.opengl.org/sdk/docs/man/xhtml/gluPerspective.xml */
      GLdouble aspect = (GLdouble)width / height;
      GLdouble f = 1 / tan(fovy * M_PI / 360.0);
      projection_matrix[0][0] = f / aspect;
      projection_matrix[1][1] = f;
      projection_matrix[2][2] = (zFar + zNear) / (zNear - zFar);
      projection_matrix[3][2] = 2 * zFar * zNear / (zNear - zFar);
      projection_matrix[2][3] = -1;
    }
#ifdef GLOR_CAN_USE_VBO
    if (context_struct_.use_vbo)
      {
        glUniformMatrix4fv(glGetUniformLocation(context_struct_.program, "ProjectionMatrix"), 1, GL_FALSE,
                           &projection_matrix[0][0]);
      }
    else
#endif
      {
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(&projection_matrix[0][0]);
      }

#ifdef GLOR_CAN_USE_VBO
    if (context_struct_.use_vbo)
      {
        glUniformMatrix4fv(glGetUniformLocation(context_struct_.program, "ViewMatrix"), 1, GL_FALSE,
                           &(context_struct_.view_matrix[0][0]));
      }
    else
#endif
      {
        glMatrixMode(GL_MODELVIEW);
        if (context_struct_.light_dir[0] == 0 && context_struct_.light_dir[1] == 0 && context_struct_.light_dir[2] == 0)
          {
            GLfloat def[4] = {0, 0, 1, 0};
            glLoadIdentity();
            glLightfv(GL_LIGHT0, GL_POSITION, &def[0]);
          }
        glLoadMatrixf(&(context_struct_.view_matrix[0][0]));
      }
#ifdef GLOR_CAN_USE_VBO
    if (context_struct_.use_vbo)
      {
        glUniform3f(glGetUniformLocation(context_struct_.program, "LightDirection"), context_struct_.light_dir[0],
                    context_struct_.light_dir[1], context_struct_.light_dir[2]);
      }
#endif
  }
  glEnable(GL_NORMALIZE);
  if (!context_struct_.use_vbo)
    {
      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);
      if (context_struct_.light_dir[0] != 0 || context_struct_.light_dir[1] != 0 || context_struct_.light_dir[2] != 0)
        {
          glLightfv(GL_LIGHT0, GL_POSITION, &context_struct_.light_dir[0]);
        }
    }
  /*glEnable(GL_CULL_FACE);
   */
  glClearColor(context_struct_.background_color[0], context_struct_.background_color[1],
               context_struct_.background_color[2], context_struct_.background_color[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  {
    GLORDrawList_ *draw;
    draw = context_struct_.draw_list_;
    while (draw)
      {
        glorDoDrawMesh_(draw->mesh, draw->n, draw->positions, draw->directions, draw->ups, draw->colors, draw->scales);
        draw = draw->next;
      }
  }
}

/*!
 * This function fills a bitmap of the given size (width x height) with the
 * image created by GLor.
 * \param [in] bitmap       The bitmap that the function has to fill
 * \param [in] width        The width of the bitmap
 * \param [in] height       The height of the bitmap
 * \returns
 * - ::kGLORENoError                on success
 * - ::kGLORENotInitialized         if GLor has not been initialized
 * - ::kGLOREOpenGLError            if an OpenGL error occured
 * - ::kGLORECameraNotInitialized   if the camera has not been initialized
 * \note The memory bitmap points to must be \f$sizeof(GLORPixel) \cdot width
 * \cdot height\f$ bytes in size, so the whole image can be stored.
 */
GLORAPI GLORError glorGetPixmap(GLORPixel *bitmap, unsigned int width, unsigned int height)
{
  unsigned int x, y;
  unsigned int fb_width, fb_height;
  unsigned int dx, dy;
  unsigned int x_patches, y_patches;
  int view_matrix_all_zeros;
  if (context_struct_.is_initialized)
    {
      if (width == 0 || height == 0 || bitmap == NULL)
        {
          return kGLOREInvalidValue;
        }
      view_matrix_all_zeros = 1;
      for (x = 0; x < 4; x++)
        {
          for (y = 0; y < 4; y++)
            {
              if (context_struct_.view_matrix[x][y] != 0)
                {
                  view_matrix_all_zeros = 0;
                }
            }
        }
      if (view_matrix_all_zeros)
        {
          /* glorCameraLookAt has not been called */
          return kGLORECameraNotInitialized;
        }
      if (context_struct_.zFar < context_struct_.zNear || context_struct_.zNear <= 0 ||
          context_struct_.vertical_field_of_view >= 180 || context_struct_.vertical_field_of_view <= 0)
        {
          /* glorCameraProjectionParameters has not been called */
          return kGLORECameraNotInitialized;
        }

      fb_width = context_struct_.init_struct.framebuffer_width;
      fb_height = context_struct_.init_struct.framebuffer_height;
      x_patches = width / fb_width + (width / fb_width * fb_width < width);
      y_patches = height / fb_height + (height / fb_height * fb_height < height);
      for (y = 0; y < y_patches; y++)
        {
          for (x = 0; x < x_patches; x++)
            {
              glViewport(-x * fb_width, -y * fb_height, width, height);
              glorDraw_(width, height);
              if ((x + 1) * fb_width <= width)
                {
                  dx = fb_width;
                }
              else
                {
                  dx = width - fb_width * x;
                }
              if ((y + 1) * fb_height <= height)
                {
                  dy = fb_height;
                }
              else
                {
                  dy = height - fb_height * y;
                }
              glPixelStorei(GL_PACK_ALIGNMENT, 1); /* byte-wise alignment */
#ifdef GLOR_USE_WIN
              /* There seems to be a driver error on windows considering
                  GL_PACK_ROW_LENGTH, so I have to roll my own loop to
                  read the pixels row-wise instead of copying whole
                  images.
              */
              {
                unsigned int i;
                for (i = 0; i < dy; i++)
                  {
                    glReadPixels(0, i, dx, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                                 bitmap + y * width * fb_height + i * width + x * fb_width);
                  }
              }
#else
              /* On other systems, GL_PACK_ROW_LENGTH works fine. */
              glPixelStorei(GL_PACK_ROW_LENGTH, width);
              glReadPixels(0, 0, dx, dy, GL_RGBA, GL_UNSIGNED_BYTE, bitmap + y * width * fb_height + x * fb_width);
#endif
            }
        }
      if (glGetError() == GL_NO_ERROR)
        {
          return kGLORENoError;
        }
      else
        {
          return kGLOREOpenGLError;
        }
    }
  else
    {
      return kGLORENotInitialized;
    }
}

/*!
 * This function is used by other GLor functions to provide the user with debug
 * information. If the user has set a logging function with
 * glorSetLogCallback(), this function will be called. Otherwise logging
 * messages will be ignored.
 * \param [in] log_message  The logging message
 */
static void glorLog_(const char *log_message)
{
  if (glorLog_func_)
    {
      glorLog_func_(log_message);
    }
}

/*!
 * During software development it will often be helpful to get debug output
 * from GLor. This information is not printed, but reported directly to the
 * user by calling a logging callback. This function allows to set this
 * callback or disable it again by calling it with NULL.
 * \param [in] glorLog_func The logging callback, a function which gets a const
 *                          char pointer as its only argument and returns
 *                          nothing.
 */
GLORAPI void glorSetLogCallback(void (*glorLog_func)(const char *log_message))
{
  glorLog_func_ = glorLog_func;
}

/*!
 * This array holds string representations of the different GLor error codes as
 * defined in ::GLORError. The elements of this array will be returned by
 * glorErrorString. The last element should always be "kGLOREUnknownError" in
 * case an unkown error code is requested.
 */
static char *error_strings_[] = {"kGLORENoError",        "kGLOREInvalidValue",         "kGLOREInvalidAttribute",
                                 "kGLOREInitError",      "kGLOREOpenGLError",          "kGLOREOutOfMemory",
                                 "kGLORENotInitialized", "kGLORECameraNotInitialized", "kGLOREUnknownError"};

/*!
 * This function returns a string representation of a given error code.
 * \param [in] error    The error code whose represenation will be returned.
 */
GLORAPI const char *glorErrorString(GLORError error)
{
  unsigned int num_errors = sizeof(error_strings_) / sizeof(char *) - 1;
  if (error >= num_errors) error = num_errors;
  return error_strings_[error];
}

/*!
 * This function allows the user to find out how his commands are rendered.
 * \returns If GLor is initialized, a string in the format:
 *          "GLor - " + window toolkit + " - " + framebuffer extension + " - "
 *          + OpenGL version + " - " + OpenGL renderer string
 *          For example "GLor - GLX - GL_ARB_framebuffer_object - 2.1 Mesa
 *          7.10.2 - Software Rasterizer" might be returned on a Linux system
 *          (using GLX) with an available GL_ARB_framebuffer_object
 *          implementation.
 *          If GLor is not initialized "Not initialized" is returned.
 */
GLORAPI const char *glorGetRenderpathString(void)
{
  return context_struct_.renderpath_string;
}

/*!
 * This function appends a string to the renderpath string returned by
 * glorGetRenderpathString().
 * \param [in] string The string to append
 */
static void glorAppendToRenderpathString_(const char *string)
{
  char *tmp = malloc(strlen(context_struct_.renderpath_string) + 3 + strlen(string) + 1);
  strcpy(tmp, context_struct_.renderpath_string);
  strcpy(tmp + strlen(context_struct_.renderpath_string), " - ");
  strcpy(tmp + strlen(context_struct_.renderpath_string) + 3, string);
  if (context_struct_.renderpath_string != not_initialized_)
    {
      free(context_struct_.renderpath_string);
    }
  context_struct_.renderpath_string = tmp;
}

#if defined(GLOR_USE_WIN)
/* OpenGL Context creation on windows */
static HINSTANCE g_hInstance;
static HWND hWnd;
static HDC dc;
static HGLRC glrc;

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
  if (fdwReason == DLL_PROCESS_ATTACH)
    {
      g_hInstance = hInstance;
      /*fprintf(stderr,"DLL attached to a process\n");*/
    }
  return TRUE;
}

static GLORError glorInitGL_WIN_(void)
{
  WNDCLASS wndclass;
  glorLog_("glorInitGL_WIN_();");

  /* Register the frame class */
  wndclass.style = 0;
  wndclass.lpfnWndProc = DefWindowProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = g_hInstance;
  wndclass.hIcon = NULL;
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = NULL;
  wndclass.lpszMenuName = "OpenGLWindow";
  wndclass.lpszClassName = "OpenGLWindow";

  if (RegisterClass(&wndclass))
    {
      /*fprintf(stderr,"Window Class registered successfully.\n"); */
    }
  else
    {
      return FALSE;
    }
  hWnd = CreateWindow("OpenGLWindow", "Generic OpenGL Sample", 0, 0, 0, 1, 1, NULL, NULL, g_hInstance, NULL);
  if (hWnd != NULL)
    {
      /*fprintf(stderr,"Window created successfully.\n"); */
    }
  else
    {
      return kGLOREInitError;
    }

  dc = GetDC(hWnd);

  /* Pixel Format selection */ {
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormat;
    BOOL result;
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;
    iPixelFormat = ChoosePixelFormat(dc, &pfd);
    result = SetPixelFormat(dc, iPixelFormat, &pfd);
    if (result)
      {
        /*fprintf(stderr,"Pixel Format set for Device Context successfully.\n");*/
      }
    else
      {
        return kGLOREInitError;
      }
  }

  /* OpenGL Rendering Context creation */ {
    BOOL result;
    glrc = wglCreateContext(dc);
    if (glrc != NULL)
      {
        /*fprintf(stderr,"OpenGL Rendering Context was created successfully.\n");*/
      }
    else
      {
        return kGLOREInitError;
      }
    result = wglMakeCurrent(dc, glrc);
    if (result)
      {
        /*fprintf(stderr,"OpenGL Rendering Context made current successfully.\n");*/
      }
    else
      {
        return kGLOREInitError;
      }
  }
  /* Load Function pointers */ {
#ifdef GLOR_CAN_USE_VBO
    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    glDeleteBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
    glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
    glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
#endif
    glDrawBuffers = (PFNGLDRAWBUFFERSPROC)wglGetProcAddress("glDrawBuffers");
    glBlendColor = (PFNGLBLENDCOLORPROC)wglGetProcAddress("glBlendColor");
#ifdef GL_ARB_framebuffer_object
    glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
    glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
    glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");
    glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
    glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
    glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");
#endif
#ifdef GL_EXT_framebuffer_object
    glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
    glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
    glFramebufferRenderbufferEXT =
        (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
    glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
    glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
    glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
    glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
    glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
    glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffersEXT");
#endif
  }
  context_struct_.terminateGL = glorTerminateGL_WIN_;
  context_struct_.gl_is_initialized = 1;
  glorAppendToRenderpathString_("Windows");
  return kGLORENoError;
}
static void glorTerminateGL_WIN_(void)
{
  glorLog_("glorTerminateGL_WIN_();");
  wglDeleteContext(glrc);
  ReleaseDC(hWnd, dc);
  DestroyWindow(hWnd);
  UnregisterClass("OpenGLWindow", g_hInstance);
}
#endif


#if defined(GLOR_USE_CGL)
/* OpenGL Context creation using CGL */

static CGLContextObj cgl_ctx; /*!< A reference to the CGL context */

/*!
 * This function implements OpenGL context creation using CGL and a Pbuffer.
 * \returns
 * - ::kGLORENoError    on success
 * - ::kGLOREInitError  if an error occurs, additional information might be
 *                      available via the logging callback.
 */
static GLORError glorInitGL_CGL_(void)
{
  CGLContextObj ctx;
  CGLPixelFormatObj pix; /* pixel format */
  GLint npix;            /* number of virtual screens referenced by pix after
                            call to CGLChoosePixelFormat*/
  const CGLPixelFormatAttribute pf_attributes[] = {
      kCGLPFAColorSize,
      24,
      kCGLPFAAlphaSize,
      8,
      kCGLPFADepthSize,
      24,
      /*kCGLPFAOffScreen,*/ /* Using a PBuffer is hardware accelerated, so */
      kCGLPFAPBuffer,       /* we want to use that. */
      0,
      0};
  glorLog_("glorInitGL_CGL_();");

  CGLChoosePixelFormat(pf_attributes, &pix, &npix);

  CGLCreateContext(pix, NULL, &ctx);
  CGLReleasePixelFormat(pix);

  CGLSetCurrentContext(ctx);
  cgl_ctx = ctx;

  context_struct_.terminateGL = glorTerminateGL_CGL_;
  context_struct_.gl_is_initialized = 1;
  glorAppendToRenderpathString_("CGL");
  return kGLORENoError;
}

/*!
 * This function destroys the OpenGL context using CGL.
 */
static void glorTerminateGL_CGL_(void)
{
  glorLog_("glorTerminateGL_CGL_();");

  CGLReleaseContext(cgl_ctx);
  context_struct_.gl_is_initialized = 0;
}
#endif

#if defined(GLOR_USE_GLX)
/* OpenGL Context creation using GLX */

static Display *display;   /*!< The used X display */
static Pixmap pixmap;      /*!< The XPixmap (GLX < 1.4)*/
static GLXPbuffer pbuffer; /*!< The GLX Pbuffer (GLX >=1.4) */
static GLXContext context; /*!< The GLX context */

/*!
 * This function implements OpenGL context creation using GLX
 * and a Pbuffer if GLX version is 1.4 or higher, or a XPixmap
 * otherwise.
 * \returns
 * - ::kGLORENoError    on success
 * - ::kGLOREInitError  if initialization failed
 */
static GLORError glorInitGL_GLX_(void)
{
  int major, minor;
  int fbcount;
  GLXFBConfig *fbc;
  GLXFBConfig fbconfig;
  glorLog_("glorInitGL_GLX_();");

  display = XOpenDisplay(0);
  if (!display)
    {
      glorLog_("Not connected to an X server!");
      return kGLOREInitError;
    }

  glXQueryVersion(display, &major, &minor);
  if (major > 1 || minor >= 4)
    {
      int fb_attribs[] = {GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT, GLX_RENDER_TYPE, GLX_RGBA_BIT, None};
      int pbuffer_attribs[] = {GLX_PBUFFER_WIDTH, 1, GLX_PBUFFER_HEIGHT, 1, None};

      fbc = glXChooseFBConfig(display, DefaultScreen(display), fb_attribs, &fbcount);
      if (fbcount == 0)
        {
          XFree(fbc);
          XCloseDisplay(display);
          return kGLOREInitError;
        }
      fbconfig = fbc[0];
      XFree(fbc);
#ifdef GLOR_USE_SGIX_EXTENSION_FOR_PBUFFERS
      glorLog_("(SGIX Pbuffer)");
      pbuffer = glXCreateGLXPbufferSGIX(display, fbconfig, 1, 1, pbuffer_attribs);
      context = glXCreateContextWithConfigSGIX(display, fbconfig, GLX_RGBA_TYPE, None, True);
      glXMakeCurrent(display, pbuffer, context);
#else
      glorLog_("(Pbuffer)");
      pbuffer = glXCreatePbuffer(display, fbconfig, pbuffer_attribs);
      context = glXCreateNewContext(display, fbconfig, GLX_RGBA_TYPE, None, True);
      glXMakeContextCurrent(display, pbuffer, pbuffer, context);
#endif

      context_struct_.terminateGL = glorTerminateGL_GLX_Pbuffer_;
      context_struct_.gl_is_initialized = 1;
      glorAppendToRenderpathString_("GLX (Pbuffer)");
      return kGLORENoError;
    }
  else
    {
      XVisualInfo *visual;
      int fb_attribs[] = {GLX_DRAWABLE_TYPE, GLX_PIXMAP_BIT, GLX_RENDER_TYPE, GLX_RGBA_TYPE, None};
      glorLog_("(XPixmap)");
      fbc = glXChooseFBConfig(display, DefaultScreen(display), fb_attribs, &fbcount);
      if (fbcount == 0)
        {
          XFree(fbc);
          XCloseDisplay(display);
          glorLog_("No FBConfig found.");
          return kGLOREInitError;
        }
      fbconfig = fbc[0];
      XFree(fbc);

      context = glXCreateNewContext(display, fbconfig, GLX_RGBA_TYPE, None, True);
      visual = glXGetVisualFromFBConfig(display, fbconfig);
      pixmap = XCreatePixmap(display, XRootWindow(display, DefaultScreen(display)), 1, 1, visual->depth);

      if (glXMakeContextCurrent(display, pixmap, pixmap, context))
        {
          context_struct_.terminateGL = glorTerminateGL_GLX_Pixmap_;
          context_struct_.gl_is_initialized = 1;
          glorAppendToRenderpathString_("GLX (XPixmap)");
          return kGLORENoError;
        }
      else
        {
          glXDestroyContext(display, context);
          XFreePixmap(display, pixmap);
          XCloseDisplay(display);
          return kGLOREInitError;
        }
    }
}

/*!
 * This function destroys the OpenGL context using GLX with a Pbuffer.
 */
static void glorTerminateGL_GLX_Pbuffer_(void)
{
  glorLog_("glorTerminateGL_GLX_Pbuffer_();");

  glXMakeContextCurrent(display, None, None, NULL);
  glXDestroyContext(display, context);
  /*glXDestroyPbuffer(display, pbuffer);*/
  XCloseDisplay(display);
  context_struct_.gl_is_initialized = 0;
}

/*!
 * This function destroys the OpenGL context using GLX with a XPixmap.
 */
static void glorTerminateGL_GLX_Pixmap_(void)
{
  glorLog_("glorTerminateGL_GLX_Pixmap_();");

  glXMakeContextCurrent(display, None, None, NULL);
  glXDestroyContext(display, context);
  XFreePixmap(display, pixmap);
  XCloseDisplay(display);
  context_struct_.gl_is_initialized = 0;
}
#endif


/*!
 * The id of the framebuffer object used for rendering.
 */
static GLuint framebuffer = 0;

/*!
 * The id of the renderbuffer object used for color data.
 */
static GLuint color_renderbuffer = 0;

/*!
 * The id of the renderbuffer object used for depth data.
 */
static GLuint depth_renderbuffer = 0;

#if GL_ARB_framebuffer_object
/* Framebuffer Object using OpenGL 3.0 or GL_ARB_framebuffer_object */
/*!
 * This function implements Framebuffer creation using the
 * ARB_framebuffer_object extension.
 * \returns
 * - ::kGLORENoError        on success
 * - ::kGLOREOpenGLError    if an OpenGL error occurs
 */
static GLORError glorInitFBO_ARB_(void)
{
  GLenum framebuffer_status;
  GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0};
  GLuint _width = context_struct_.init_struct.framebuffer_width;
  GLuint _height = context_struct_.init_struct.framebuffer_height;

  glorLog_("glorInitFBO_ARB_();");

  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glGenRenderbuffers(1, &color_renderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, _width, _height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_renderbuffer);

  glGenRenderbuffers(2, &depth_renderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _width, _height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer);

  glDrawBuffers(1, draw_buffers);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
    {
      return kGLOREOpenGLError;
    }
  glViewport(0, 0, _width, _height);
  glEnable(GL_DEPTH_TEST);
  if (glGetError() != GL_NO_ERROR)
    {
      glorTerminateFBO_ARB_();
      return kGLOREOpenGLError;
    }

  context_struct_.terminateFBO = glorTerminateFBO_ARB_;
  context_struct_.fbo_is_initialized = 1;
  glorAppendToRenderpathString_("GL_ARB_framebuffer_object");
  return kGLORENoError;
}

/*!
 * This function destroys the Framebuffer Object using the
 * ARB_framebuffer_object extension.
 */
static void glorTerminateFBO_ARB_(void)
{
  glorLog_("glorTerminateFBO_ARB_();");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glDeleteFramebuffers(1, &framebuffer);
  glDeleteRenderbuffers(1, &color_renderbuffer);
  glDeleteRenderbuffers(1, &depth_renderbuffer);

  context_struct_.fbo_is_initialized = 0;
}
#endif


#if GL_EXT_framebuffer_object
/* Framebuffer Object using GL_EXT_framebuffer_object */
/*!
 * This function implements Framebuffer creation using the
 * EXT_framebuffer_object extension.
 * \returns
 * - ::kGLORENoError        on success
 * - ::kGLOREOpenGLError    if an OpenGL error occurs
 */
static GLORError glorInitFBO_EXT_(void)
{
  GLenum framebuffer_status;
  GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0_EXT};
  GLuint _width = context_struct_.init_struct.framebuffer_width;
  GLuint _height = context_struct_.init_struct.framebuffer_height;

  glorLog_("glorInitFBO_EXT_();");

  glGenFramebuffersEXT(1, &framebuffer);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);

  glGenRenderbuffersEXT(1, &color_renderbuffer);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, color_renderbuffer);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, _width, _height);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, color_renderbuffer);

  glGenRenderbuffersEXT(2, &depth_renderbuffer);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_renderbuffer);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, _width, _height);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_renderbuffer);

  glDrawBuffers(1, draw_buffers);
  glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
  framebuffer_status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
      return kGLOREOpenGLError;
    }
  glViewport(0, 0, _width, _height);
  glEnable(GL_DEPTH_TEST);
  if (glGetError() != GL_NO_ERROR)
    {
      glorTerminateFBO_EXT_();
      return kGLOREOpenGLError;
    }

  context_struct_.terminateFBO = glorTerminateFBO_EXT_;
  context_struct_.fbo_is_initialized = 1;
  glorAppendToRenderpathString_("GL_EXT_framebuffer_object");
  return kGLORENoError;
}

/*!
 * This function destroys the Framebuffer Object using the
 * EXT_framebuffer_object extension.
 */
static void glorTerminateFBO_EXT_(void)
{
  glorLog_("glorTerminateFBO_EXT_();");

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

  glDeleteFramebuffersEXT(1, &framebuffer);
  glDeleteRenderbuffersEXT(1, &color_renderbuffer);
  glDeleteRenderbuffersEXT(1, &depth_renderbuffer);

  context_struct_.fbo_is_initialized = 0;
}
#endif

/*!
 * This function creates the context_struct_.cylinder_mesh for simple drawing
 */
static void glorCreateCylinderMesh_(void)
{
  unsigned int i;
  unsigned int n;
  double *vertices;
  double *normals;
  double *colors;

  n = 12 * 36;
  vertices = malloc(n * 3 * sizeof(double));
  normals = malloc(n * 3 * sizeof(double));
  colors = malloc(n * 3 * sizeof(double));
  for (i = 0; i < 36; i++)
    {
      vertices[(12 * i + 0) * 3 + 0] = cos(M_PI * 10 * i / 180);
      vertices[(12 * i + 0) * 3 + 1] = sin(M_PI * 10 * i / 180);
      vertices[(12 * i + 0) * 3 + 2] = 0;
      vertices[(12 * i + 1) * 3 + 0] = cos(M_PI * 10 * (i + 1) / 180);
      vertices[(12 * i + 1) * 3 + 1] = sin(M_PI * 10 * (i + 1) / 180);
      vertices[(12 * i + 1) * 3 + 2] = 0;
      vertices[(12 * i + 2) * 3 + 0] = cos(M_PI * 10 * i / 180);
      vertices[(12 * i + 2) * 3 + 1] = sin(M_PI * 10 * i / 180);
      vertices[(12 * i + 2) * 3 + 2] = 1;

      normals[(12 * i + 0) * 3 + 0] = cos(M_PI * 10 * i / 180);
      normals[(12 * i + 0) * 3 + 1] = sin(M_PI * 10 * i / 180);
      normals[(12 * i + 0) * 3 + 2] = 0;
      normals[(12 * i + 1) * 3 + 0] = cos(M_PI * 10 * (i + 1) / 180);
      normals[(12 * i + 1) * 3 + 1] = sin(M_PI * 10 * (i + 1) / 180);
      normals[(12 * i + 1) * 3 + 2] = 0;
      normals[(12 * i + 2) * 3 + 0] = cos(M_PI * 10 * i / 180);
      normals[(12 * i + 2) * 3 + 1] = sin(M_PI * 10 * i / 180);
      normals[(12 * i + 2) * 3 + 2] = 0;


      vertices[(12 * i + 3) * 3 + 0] = cos(M_PI * 10 * i / 180);
      vertices[(12 * i + 3) * 3 + 1] = sin(M_PI * 10 * i / 180);
      vertices[(12 * i + 3) * 3 + 2] = 1;
      vertices[(12 * i + 4) * 3 + 0] = cos(M_PI * 10 * (i + 1) / 180);
      vertices[(12 * i + 4) * 3 + 1] = sin(M_PI * 10 * (i + 1) / 180);
      vertices[(12 * i + 4) * 3 + 2] = 0;
      vertices[(12 * i + 5) * 3 + 0] = cos(M_PI * 10 * (i + 1) / 180);
      vertices[(12 * i + 5) * 3 + 1] = sin(M_PI * 10 * (i + 1) / 180);
      vertices[(12 * i + 5) * 3 + 2] = 1;

      normals[(12 * i + 3) * 3 + 0] = cos(M_PI * 10 * i / 180);
      normals[(12 * i + 3) * 3 + 1] = sin(M_PI * 10 * i / 180);
      normals[(12 * i + 3) * 3 + 2] = 0;
      normals[(12 * i + 4) * 3 + 0] = cos(M_PI * 10 * (i + 1) / 180);
      normals[(12 * i + 4) * 3 + 1] = sin(M_PI * 10 * (i + 1) / 180);
      normals[(12 * i + 4) * 3 + 2] = 0;
      normals[(12 * i + 5) * 3 + 0] = cos(M_PI * 10 * (i + 1) / 180);
      normals[(12 * i + 5) * 3 + 1] = sin(M_PI * 10 * (i + 1) / 180);
      normals[(12 * i + 5) * 3 + 2] = 0;

      vertices[(12 * i + 6) * 3 + 0] = cos(M_PI * 10 * i / 180);
      vertices[(12 * i + 6) * 3 + 1] = sin(M_PI * 10 * i / 180);
      vertices[(12 * i + 6) * 3 + 2] = 0;
      vertices[(12 * i + 7) * 3 + 0] = 0;
      vertices[(12 * i + 7) * 3 + 1] = 0;
      vertices[(12 * i + 7) * 3 + 2] = 0;
      vertices[(12 * i + 8) * 3 + 0] = cos(M_PI * 10 * (i + 1) / 180);
      vertices[(12 * i + 8) * 3 + 1] = sin(M_PI * 10 * (i + 1) / 180);
      vertices[(12 * i + 8) * 3 + 2] = 0;

      normals[(12 * i + 6) * 3 + 0] = 0;
      normals[(12 * i + 6) * 3 + 1] = 0;
      normals[(12 * i + 6) * 3 + 2] = -1;
      normals[(12 * i + 7) * 3 + 0] = 0;
      normals[(12 * i + 7) * 3 + 1] = 0;
      normals[(12 * i + 7) * 3 + 2] = -1;
      normals[(12 * i + 8) * 3 + 0] = 0;
      normals[(12 * i + 8) * 3 + 1] = 0;
      normals[(12 * i + 8) * 3 + 2] = -1;

      vertices[(12 * i + 9) * 3 + 0] = cos(M_PI * 10 * i / 180);
      vertices[(12 * i + 9) * 3 + 1] = sin(M_PI * 10 * i / 180);
      vertices[(12 * i + 9) * 3 + 2] = 1;
      vertices[(12 * i + 10) * 3 + 0] = cos(M_PI * 10 * (i + 1) / 180);
      vertices[(12 * i + 10) * 3 + 1] = sin(M_PI * 10 * (i + 1) / 180);
      vertices[(12 * i + 10) * 3 + 2] = 1;
      vertices[(12 * i + 11) * 3 + 0] = 0;
      vertices[(12 * i + 11) * 3 + 1] = 0;
      vertices[(12 * i + 11) * 3 + 2] = 1;

      normals[(12 * i + 9) * 3 + 0] = 0;
      normals[(12 * i + 9) * 3 + 1] = 0;
      normals[(12 * i + 9) * 3 + 2] = 1;
      normals[(12 * i + 10) * 3 + 0] = 0;
      normals[(12 * i + 10) * 3 + 1] = 0;
      normals[(12 * i + 10) * 3 + 2] = 1;
      normals[(12 * i + 11) * 3 + 0] = 0;
      normals[(12 * i + 11) * 3 + 1] = 0;
      normals[(12 * i + 11) * 3 + 2] = 1;
    }
  for (i = 0; i < n * 3; i++)
    {
      colors[i] = 1;
    }
  glorCreateMesh(&context_struct_.cylinder_mesh, n, vertices, normals, colors);
  free(vertices);
  free(normals);
  free(colors);
}

/*!
 * This function allows drawing a cylinder without requiring a mesh.
 * \sa glorDrawMesh()
 */
GLORAPI void glorDrawCylinderMesh(unsigned int n, const double *positions, const double *directions,
                                  const double *colors, const double *radii, const double *lengths)
{
  unsigned int i;
  unsigned int j;
  unsigned int min_index;
  unsigned int min_n;
  double *scales = malloc(n * 3 * sizeof(double));
  double *ups = malloc(n * 3 * sizeof(double));
  for (i = 0; i < n; i++)
    {
      scales[3 * i + 0] = radii[i];
      scales[3 * i + 1] = radii[i];
      scales[3 * i + 2] = lengths[i];
      min_n = directions[3 * i + 0];
      min_index = 0;
      for (j = 1; j < 3; j++)
        {
          if (directions[3 * i + j] * directions[3 * i + j] < min_n * min_n)
            {
              min_n = directions[3 * i + j];
              min_index = j;
            }
        }
      for (j = 0; j < 3; j++)
        {
          ups[3 * i + j] = 0;
        }
      ups[3 * i + min_index] = 1;
    }
  glorDrawMesh(context_struct_.cylinder_mesh, n, positions, directions, ups, colors, scales);
  free(scales);
  free(ups);
}

/*!
 * This function creates the context_struct_.cone_mesh for simple drawing
 */
static void glorCreateConeMesh_(void)
{
  unsigned int i;
  unsigned int n;
  double *vertices;
  double *normals;
  double *colors;

  n = 6 * 36;
  vertices = malloc(n * 3 * sizeof(double));
  normals = malloc(n * 3 * sizeof(double));
  colors = malloc(n * 3 * sizeof(double));
  for (i = 0; i < 36; i++)
    {
      vertices[(6 * i + 0) * 3 + 0] = cos(M_PI * 10 * i / 180);
      vertices[(6 * i + 0) * 3 + 1] = sin(M_PI * 10 * i / 180);
      vertices[(6 * i + 0) * 3 + 2] = 0;
      vertices[(6 * i + 1) * 3 + 0] = cos(M_PI * 10 * (i + 1) / 180);
      vertices[(6 * i + 1) * 3 + 1] = sin(M_PI * 10 * (i + 1) / 180);
      vertices[(6 * i + 1) * 3 + 2] = 0;
      vertices[(6 * i + 2) * 3 + 0] = 0;
      vertices[(6 * i + 2) * 3 + 1] = 0;
      vertices[(6 * i + 2) * 3 + 2] = 1;

      normals[(6 * i + 0) * 3 + 0] = cos(M_PI * 10 * i / 180);
      normals[(6 * i + 0) * 3 + 1] = sin(M_PI * 10 * i / 180);
      normals[(6 * i + 0) * 3 + 2] = 0;
      normals[(6 * i + 1) * 3 + 0] = cos(M_PI * 10 * (i + 1) / 180);
      normals[(6 * i + 1) * 3 + 1] = sin(M_PI * 10 * (i + 1) / 180);
      normals[(6 * i + 1) * 3 + 2] = 0;
      normals[(6 * i + 2) * 3 + 0] = 0;
      normals[(6 * i + 2) * 3 + 1] = 0;
      normals[(6 * i + 2) * 3 + 2] = 1;

      vertices[(6 * i + 3) * 3 + 0] = cos(M_PI * 10 * i / 180);
      vertices[(6 * i + 3) * 3 + 1] = sin(M_PI * 10 * i / 180);
      vertices[(6 * i + 3) * 3 + 2] = 0;
      vertices[(6 * i + 4) * 3 + 0] = 0;
      vertices[(6 * i + 4) * 3 + 1] = 0;
      vertices[(6 * i + 4) * 3 + 2] = 0;
      vertices[(6 * i + 5) * 3 + 0] = cos(M_PI * 10 * (i + 1) / 180);
      vertices[(6 * i + 5) * 3 + 1] = sin(M_PI * 10 * (i + 1) / 180);
      vertices[(6 * i + 5) * 3 + 2] = 0;

      normals[(6 * i + 3) * 3 + 0] = 0;
      normals[(6 * i + 3) * 3 + 1] = 0;
      normals[(6 * i + 3) * 3 + 2] = -1;
      normals[(6 * i + 4) * 3 + 0] = 0;
      normals[(6 * i + 4) * 3 + 1] = 0;
      normals[(6 * i + 4) * 3 + 2] = -1;
      normals[(6 * i + 5) * 3 + 0] = 0;
      normals[(6 * i + 5) * 3 + 1] = 0;
      normals[(6 * i + 5) * 3 + 2] = -1;
    }
  for (i = 0; i < n * 3; i++)
    {
      colors[i] = 1;
    }
  glorCreateMesh(&context_struct_.cone_mesh, n, vertices, normals, colors);
  free(vertices);
  free(normals);
  free(colors);
}

/*!
 * This function allows drawing a cylinder without requiring a mesh.
 * \sa glorDrawMesh()
 */
GLORAPI void glorDrawConeMesh(unsigned int n, const double *positions, const double *directions, const double *colors,
                              const double *radii, const double *lengths)
{
  unsigned int i;
  unsigned int j;
  unsigned int min_index;
  unsigned int min_n;
  double *scales = malloc(n * 3 * sizeof(double));
  double *ups = malloc(n * 3 * sizeof(double));
  for (i = 0; i < n; i++)
    {
      scales[3 * i + 0] = radii[i];
      scales[3 * i + 1] = radii[i];
      scales[3 * i + 2] = lengths[i];
      min_n = directions[3 * i + 0];
      min_index = 0;
      for (j = 1; j < 3; j++)
        {
          if (directions[3 * i + j] * directions[3 * i + j] < min_n * min_n)
            {
              min_n = directions[3 * i + j];
              min_index = j;
            }
        }
      for (j = 0; j < 3; j++)
        {
          ups[3 * i + j] = 0;
        }
      ups[3 * i + min_index] = 1;
    }
  glorDrawMesh(context_struct_.cone_mesh, n, positions, directions, ups, colors, scales);
  free(scales);
  free(ups);
}

/*!
 * This function allows drawing a sphere without requiring a mesh.
 * \sa glorDrawMesh()
 */
GLORAPI void glorDrawSphereMesh(unsigned int n, const double *positions, const double *colors, const double *radii)
{
  unsigned int i;
  double *directions = malloc(n * 3 * sizeof(double));
  double *ups = malloc(n * 3 * sizeof(double));
  double *scales = malloc(n * 3 * sizeof(double));
  for (i = 0; i < n; i++)
    {
      directions[i * 3 + 0] = 0;
      directions[i * 3 + 1] = 0;
      directions[i * 3 + 2] = 1;
      ups[i * 3 + 0] = 0;
      ups[i * 3 + 1] = 1;
      ups[i * 3 + 2] = 0;
      scales[i * 3 + 0] = radii[i];
      scales[i * 3 + 1] = radii[i];
      scales[i * 3 + 2] = radii[i];
    }
  glorDrawMesh(context_struct_.sphere_mesh, n, positions, directions, ups, colors, scales);
  free(directions);
  free(ups);
  free(scales);
}

/*!
 * This function creates the context_struct_.sphere_mesh for simple drawing
 */
static void glorCreateSphereMesh_(void)
{
  unsigned int i, j;
  unsigned int n, iterations = 4;
  double *colors;
  double *vertices_old;
  double *vertices_new;
  double *vertices;
  double *triangle;
  double *triangle_new;
  /* pre-calculated icosahedron vertices */
  double icosahedron[] = {0.52573111211913359,
                          0,
                          0.85065080835203988,
                          0,
                          0.85065080835203988,
                          0.52573111211913359,
                          -0.52573111211913359,
                          0,
                          0.85065080835203988,
                          0,
                          0.85065080835203988,
                          0.52573111211913359,
                          -0.85065080835203988,
                          0.52573111211913359,
                          0,
                          -0.52573111211913359,
                          0,
                          0.85065080835203988,
                          0,
                          0.85065080835203988,
                          0.52573111211913359,
                          0,
                          0.85065080835203988,
                          -0.52573111211913359,
                          -0.85065080835203988,
                          0.52573111211913359,
                          0,
                          0.85065080835203988,
                          0.52573111211913359,
                          0,
                          0,
                          0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          0.85065080835203988,
                          0.52573111211913359,
                          0.52573111211913359,
                          0,
                          0.85065080835203988,
                          0.85065080835203988,
                          0.52573111211913359,
                          0,
                          0,
                          0.85065080835203988,
                          0.52573111211913359,
                          0.52573111211913359,
                          0,
                          0.85065080835203988,
                          0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          0.85065080835203988,
                          0.52573111211913359,
                          0,
                          0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          0.85065080835203988,
                          0.52573111211913359,
                          0,
                          0.85065080835203988,
                          0.52573111211913359,
                          0,
                          0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          0,
                          0.85065080835203988,
                          -0.52573111211913359,
                          0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          0,
                          0.85065080835203988,
                          -0.52573111211913359,
                          0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          0,
                          -0.85065080835203988,
                          -0.52573111211913359,
                          -0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          0,
                          -0.85065080835203988,
                          -0.52573111211913359,
                          0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          0,
                          -0.85065080835203988,
                          0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          0.52573111211913359,
                          -0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          0,
                          -0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          0.52573111211913359,
                          -0.52573111211913359,
                          0,
                          0.85065080835203988,
                          -0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          0,
                          -0.85065080835203988,
                          0.52573111211913359,
                          0.52573111211913359,
                          0,
                          0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          0.85065080835203988,
                          0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          0.52573111211913359,
                          0,
                          0.85065080835203988,
                          0,
                          -0.85065080835203988,
                          0.52573111211913359,
                          -0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          -0.52573111211913359,
                          0,
                          0.85065080835203988,
                          -0.85065080835203988,
                          0.52573111211913359,
                          0,
                          -0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          -0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          0.52573111211913359,
                          0,
                          0,
                          0.85065080835203988,
                          -0.52573111211913359,
                          -0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          -0.85065080835203988,
                          0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          -0.52573111211913359,
                          0,
                          -0.52573111211913359,
                          0,
                          -0.85065080835203988,
                          0,
                          -0.85065080835203988,
                          -0.52573111211913359};
  n = 20;
  vertices_old = malloc(n * 3 * 3 * sizeof(double));
  memcpy(vertices_old, icosahedron, n * 3 * 3 * sizeof(double));
  for (j = 0; j < iterations; j++)
    {
      vertices_new = malloc(4 * n * 3 * 3 * sizeof(double));
      for (i = 0; i < n; i++)
        {
          double a[3], b[3], c[3];
          double len_a, len_b, len_c;
          triangle = &vertices_old[i * 3 * 3];
          triangle_new = &vertices_new[i * 3 * 3 * 4];
          a[0] = (triangle[2 * 3 + 0] + triangle[1 * 3 + 0]) * 0.5;
          a[1] = (triangle[2 * 3 + 1] + triangle[1 * 3 + 1]) * 0.5;
          a[2] = (triangle[2 * 3 + 2] + triangle[1 * 3 + 2]) * 0.5;
          len_a = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
          a[0] = a[0] / len_a;
          a[1] = a[1] / len_a;
          a[2] = a[2] / len_a;
          b[0] = (triangle[0 * 3 + 0] + triangle[2 * 3 + 0]) * 0.5;
          b[1] = (triangle[0 * 3 + 1] + triangle[2 * 3 + 1]) * 0.5;
          b[2] = (triangle[0 * 3 + 2] + triangle[2 * 3 + 2]) * 0.5;
          len_b = sqrt(b[0] * b[0] + b[1] * b[1] + b[2] * b[2]);
          b[0] = b[0] / len_b;
          b[1] = b[1] / len_b;
          b[2] = b[2] / len_b;
          c[0] = (triangle[0 * 3 + 0] + triangle[1 * 3 + 0]) * 0.5;
          c[1] = (triangle[0 * 3 + 1] + triangle[1 * 3 + 1]) * 0.5;
          c[2] = (triangle[0 * 3 + 2] + triangle[1 * 3 + 2]) * 0.5;
          len_c = sqrt(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]);
          c[0] = c[0] / len_c;
          c[1] = c[1] / len_c;
          c[2] = c[2] / len_c;

          triangle_new[0 * 3 * 3 + 0 * 3 + 0] = triangle[0 * 3 + 0];
          triangle_new[0 * 3 * 3 + 0 * 3 + 1] = triangle[0 * 3 + 1];
          triangle_new[0 * 3 * 3 + 0 * 3 + 2] = triangle[0 * 3 + 2];
          triangle_new[0 * 3 * 3 + 1 * 3 + 0] = c[0];
          triangle_new[0 * 3 * 3 + 1 * 3 + 1] = c[1];
          triangle_new[0 * 3 * 3 + 1 * 3 + 2] = c[2];
          triangle_new[0 * 3 * 3 + 2 * 3 + 0] = b[0];
          triangle_new[0 * 3 * 3 + 2 * 3 + 1] = b[1];
          triangle_new[0 * 3 * 3 + 2 * 3 + 2] = b[2];

          triangle_new[1 * 3 * 3 + 0 * 3 + 0] = a[0];
          triangle_new[1 * 3 * 3 + 0 * 3 + 1] = a[1];
          triangle_new[1 * 3 * 3 + 0 * 3 + 2] = a[2];
          triangle_new[1 * 3 * 3 + 1 * 3 + 0] = b[0];
          triangle_new[1 * 3 * 3 + 1 * 3 + 1] = b[1];
          triangle_new[1 * 3 * 3 + 1 * 3 + 2] = b[2];
          triangle_new[1 * 3 * 3 + 2 * 3 + 0] = c[0];
          triangle_new[1 * 3 * 3 + 2 * 3 + 1] = c[1];
          triangle_new[1 * 3 * 3 + 2 * 3 + 2] = c[2];

          triangle_new[2 * 3 * 3 + 0 * 3 + 0] = triangle[1 * 3 + 0];
          triangle_new[2 * 3 * 3 + 0 * 3 + 1] = triangle[1 * 3 + 1];
          triangle_new[2 * 3 * 3 + 0 * 3 + 2] = triangle[1 * 3 + 2];
          triangle_new[2 * 3 * 3 + 1 * 3 + 0] = a[0];
          triangle_new[2 * 3 * 3 + 1 * 3 + 1] = a[1];
          triangle_new[2 * 3 * 3 + 1 * 3 + 2] = a[2];
          triangle_new[2 * 3 * 3 + 2 * 3 + 0] = c[0];
          triangle_new[2 * 3 * 3 + 2 * 3 + 1] = c[1];
          triangle_new[2 * 3 * 3 + 2 * 3 + 2] = c[2];

          triangle_new[3 * 3 * 3 + 0 * 3 + 0] = a[0];
          triangle_new[3 * 3 * 3 + 0 * 3 + 1] = a[1];
          triangle_new[3 * 3 * 3 + 0 * 3 + 2] = a[2];
          triangle_new[3 * 3 * 3 + 1 * 3 + 0] = triangle[2 * 3 + 0];
          triangle_new[3 * 3 * 3 + 1 * 3 + 1] = triangle[2 * 3 + 1];
          triangle_new[3 * 3 * 3 + 1 * 3 + 2] = triangle[2 * 3 + 2];
          triangle_new[3 * 3 * 3 + 2 * 3 + 0] = b[0];
          triangle_new[3 * 3 * 3 + 2 * 3 + 1] = b[1];
          triangle_new[3 * 3 * 3 + 2 * 3 + 2] = b[2];
        }
      n *= 4;
      free(vertices_old);
      vertices_old = vertices_new;
    }
  vertices = vertices_old;
  colors = malloc(n * 3 * 3 * sizeof(double));
  for (i = 0; i < n * 3 * 3; i++)
    {
      colors[i] = 1;
    }
  glorCreateMesh(&context_struct_.sphere_mesh, n * 3, vertices, vertices, colors);
  free(colors);
  free(vertices);
}
