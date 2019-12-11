#define GR3_GLX_C
#include "gr3_glx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "gr3.h"
#define DONT_USE_RETURN_ERROR
#include "gr3_internals.h"
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

struct platform_
{
  void (*terminate)(void);
  void (*(*getProcAddress)(const char *))();
};
static struct platform_ *platform;
static void *platform_library = NULL;
static struct platform_ *(*platform_loader)(void (*log_callback)(const char *),
                                            void (*appendtorenderpathstring_callback)(const char *)) = NULL;

static void gr3_terminateGL_(void)
{
  gr3_log_("gr3_terminateGL_();");
  context_struct_.gl_is_initialized = 0;
  platform->terminate();
  platform_loader = NULL;
  platform = NULL;
  dlclose(platform_library);
  platform_library = NULL;
}

int gr3_platform_initGL_(void)
{
  gr3_log_("gr3_platform_initGL_();");
  if (!platform_library)
    {
      const char *grdir = getenv("GRDIR");
      if (grdir == NULL)
        {
          grdir = GRDIR;
        }
      if (strlen(grdir) + strlen("libGR3platform.so") < MAXPATHLEN)
        {
          char pathname[MAXPATHLEN];
          sprintf(pathname, "%s/lib/libGR3platform.so", grdir);
          platform_library = dlopen(pathname, RTLD_NOW | RTLD_LOCAL);
        }
    }
  if (!platform_library)
    {
      platform_library = dlopen("libGR3platform.so", RTLD_NOW | RTLD_LOCAL);
    }
  if (!platform_library)
    {
      char *error_message = dlerror();
      gr3_log_("Failed to load GR3 platform library");
      gr3_log_(error_message);
      return GR3_ERROR_INIT_FAILED;
    }
  platform_loader = (struct platform_ * (*)(void (*)(const char *), void (*)(const char *)))
      dlsym(platform_library, "gr3_platform_initGL_dynamic_");
  if (!platform_loader)
    {
      char *error_message = dlerror();
      gr3_log_("Failed to load GR3 platform loader");
      gr3_log_(error_message);
      dlclose(platform_library);
      platform_library = NULL;
      return GR3_ERROR_INIT_FAILED;
    }
  platform = platform_loader(gr3_log_, gr3_appendtorenderpathstring_);
  if (!platform)
    {
      return GR3_ERROR_INIT_FAILED;
    }
  context_struct_.gl_is_initialized = 1;
  context_struct_.terminateGL = gr3_terminateGL_;
  return GR3_ERROR_NONE;
}

/*!
 * This function implements OpenGL context creation using GLX
 * and a Pbuffer if GLX version is 1.4 or higher, or a XPixmap
 * otherwise.
 * \returns
 * - ::GR3_ERROR_NONE         on success
 * - ::GR3_ERROR_INIT_FAILED  if initialization failed
 */
int gr3_initGL_GLX_(void)
{
  int error = GR3_ERROR_NONE;
  gr3_log_("gr3_initGL_CGL_();");
  error = gr3_platform_initGL_();
  if (error != GR3_ERROR_NONE)
    {
      return error;
    }


#ifndef NO_GL
  /* Load Function pointers */ {
#ifdef GR3_CAN_USE_VBO
    gr3_glBufferData = (PFNGLBUFFERDATAPROC)platform->getProcAddress("glBufferData");
    gr3_glBindBuffer = (PFNGLBINDBUFFERPROC)platform->getProcAddress("glBindBuffer");
    gr3_glGenBuffers = (PFNGLGENBUFFERSPROC)platform->getProcAddress("glGenBuffers");
    gr3_glDeleteBuffers = (PFNGLGENBUFFERSPROC)platform->getProcAddress("glDeleteBuffers");
    gr3_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)platform->getProcAddress("glVertexAttribPointer");
    gr3_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)platform->getProcAddress("glGetAttribLocation");
    gr3_glEnableVertexAttribArray =
        (PFNGLENABLEVERTEXATTRIBARRAYPROC)platform->getProcAddress("glEnableVertexAttribArray");
    gr3_glUseProgram = (PFNGLUSEPROGRAMPROC)platform->getProcAddress("glUseProgram");
    gr3_glDeleteShader = (PFNGLDELETESHADERPROC)platform->getProcAddress("glDeleteShader");
    gr3_glLinkProgram = (PFNGLLINKPROGRAMPROC)platform->getProcAddress("glLinkProgram");
    gr3_glAttachShader = (PFNGLATTACHSHADERPROC)platform->getProcAddress("glAttachShader");
    gr3_glCreateShader = (PFNGLCREATESHADERPROC)platform->getProcAddress("glCreateShader");
    gr3_glCompileShader = (PFNGLCOMPILESHADERPROC)platform->getProcAddress("glCompileShader");
    gr3_glCreateProgram = (PFNGLCREATEPROGRAMPROC)platform->getProcAddress("glCreateProgram");
    gr3_glDeleteProgram = (PFNGLDELETEPROGRAMPROC)platform->getProcAddress("glDeleteProgram");
    gr3_glUniform1i = (PFNGLUNIFORM1IPROC)platform->getProcAddress("glUniform1i");
    gr3_glUniform3f = (PFNGLUNIFORM3FPROC)platform->getProcAddress("glUniform3f");
    gr3_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)platform->getProcAddress("glUniformMatrix4fv");
    gr3_glUniform4f = (PFNGLUNIFORM4FPROC)platform->getProcAddress("glUniform4f");
    gr3_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)platform->getProcAddress("glGetUniformLocation");
    gr3_glShaderSource = (PFNGLSHADERSOURCEPROC)platform->getProcAddress("glShaderSource");
    gr3_glGetShaderiv = (PFNGLGETSHADERIVPROC)platform->getProcAddress("glGetShaderiv");
    gr3_glGetProgramiv = (PFNGLGETPROGRAMIVPROC)platform->getProcAddress("glGetProgramiv");
    gr3_glActiveTexture = (PFNGLACTIVETEXTUREPROC)platform->getProcAddress("glActiveTexture");
    gr3_glTexImage3D = (PFNGLTEXIMAGE3DPROC)platform->getProcAddress("glTexImage3D");
#endif
    gr3_glDrawBuffers = (PFNGLDRAWBUFFERSPROC)platform->getProcAddress("glDrawBuffers");
    /*glBlendColor = (PFNGLBLENDCOLORPROC)platform->getProcAddress("glBlendColor");*/
#ifdef GL_ARB_framebuffer_object
    gr3_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)platform->getProcAddress("glBindRenderbuffer");
    gr3_glCheckFramebufferStatus =
        (PFNGLCHECKFRAMEBUFFERSTATUSPROC)platform->getProcAddress("glCheckFramebufferStatus");
    gr3_glFramebufferRenderbuffer =
        (PFNGLFRAMEBUFFERRENDERBUFFERPROC)platform->getProcAddress("glFramebufferRenderbuffer");
    gr3_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)platform->getProcAddress("glRenderbufferStorage");
    gr3_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)platform->getProcAddress("glBindFramebuffer");
    gr3_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)platform->getProcAddress("glGenFramebuffers");
    gr3_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)platform->getProcAddress("glGenRenderbuffers");
    gr3_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)platform->getProcAddress("glDeleteFramebuffers");
    gr3_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)platform->getProcAddress("glDeleteRenderbuffers");
    gr3_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)platform->getProcAddress("glFramebufferTexture2D");
#endif
#ifdef GL_EXT_framebuffer_object
    gr3_glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)platform->getProcAddress("glBindRenderbufferEXT");
    gr3_glCheckFramebufferStatusEXT =
        (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)platform->getProcAddress("glCheckFramebufferStatusEXT");
    gr3_glFramebufferRenderbufferEXT =
        (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)platform->getProcAddress("glFramebufferRenderbufferEXT");
    gr3_glRenderbufferStorageEXT =
        (PFNGLRENDERBUFFERSTORAGEEXTPROC)platform->getProcAddress("glRenderbufferStorageEXT");
    gr3_glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)platform->getProcAddress("glBindFramebufferEXT");
    gr3_glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)platform->getProcAddress("glGenFramebuffersEXT");
    gr3_glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)platform->getProcAddress("glGenRenderbuffersEXT");
    gr3_glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)platform->getProcAddress("glDeleteFramebuffersEXT");
    gr3_glDeleteRenderbuffersEXT =
        (PFNGLDELETERENDERBUFFERSEXTPROC)platform->getProcAddress("glDeleteRenderbuffersEXT");
    gr3_glFramebufferTexture2DEXT =
        (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)platform->getProcAddress("glFramebufferTexture2DEXT");
#endif
    gr3_glBegin = (void (*)(GLenum))platform->getProcAddress("glBegin");
    gr3_glBlendColor = (void (*)(GLclampf, GLclampf, GLclampf, GLclampf))platform->getProcAddress("glBlendColor");
    gr3_glBlendFunc = (void (*)(GLenum, GLenum))platform->getProcAddress("glBlendFunc");
    gr3_glCallList = (void (*)(GLuint))platform->getProcAddress("glCallList");
    gr3_glClear = (void (*)(GLenum))platform->getProcAddress("glClear");
    gr3_glClearColor = (void (*)(GLclampf, GLclampf, GLclampf, GLclampf))platform->getProcAddress("glClearColor");
    gr3_glColor3fv = (void (*)(const GLfloat *))platform->getProcAddress("glColor3fv");
    gr3_glColorMaterial = (void (*)(GLenum, GLenum))platform->getProcAddress("glColorMaterial");
    gr3_glDeleteLists = (void (*)(GLuint, GLsizei))platform->getProcAddress("glDeleteLists");
    gr3_glDisable = (void (*)(GLenum))platform->getProcAddress("glDisable");
    gr3_glDrawArrays = (void (*)(GLenum, GLint, GLsizei))platform->getProcAddress("glDrawArrays");
    gr3_glDrawElements = (void (*)(GLenum, GLsizei, GLenum, const GLvoid *))platform->getProcAddress("glDrawElements");
    gr3_glEnable = (void (*)(GLenum))platform->getProcAddress("glEnable");
    gr3_glEnd = (void (*)(void))platform->getProcAddress("glEnd");
    gr3_glEndList = (void (*)(void))platform->getProcAddress("glEndList");
    gr3_glGenLists = (GLuint(*)(GLsizei))platform->getProcAddress("glGenLists");
    gr3_glGetError = (GLenum(*)(void))platform->getProcAddress("glGetError");
    gr3_glGetBooleanv = (void (*)(GLenum, GLboolean *))platform->getProcAddress("glGetBooleanv");
    gr3_glGetFloatv = (void (*)(GLenum, GLfloat *))platform->getProcAddress("glGetFloatv");
    gr3_glGetIntegerv = (void (*)(GLenum, GLint *))platform->getProcAddress("glGetIntegerv");
    gr3_glGetString = (const GLubyte *(*)(GLenum))platform->getProcAddress("glGetString");
    gr3_glLightfv = (void (*)(GLenum, GLenum, const GLfloat *))platform->getProcAddress("glLightfv");
    gr3_glLoadIdentity = (void (*)(void))platform->getProcAddress("glLoadIdentity");
    gr3_glLoadMatrixf = (void (*)(const GLfloat *))platform->getProcAddress("glLoadMatrixf");
    gr3_glMaterialfv = (void (*)(GLenum, GLenum, const GLfloat *))platform->getProcAddress("glMaterialfv");
    gr3_glMatrixMode = (void (*)(GLenum))platform->getProcAddress("glMatrixMode");
    gr3_glMultMatrixf = (void (*)(const GLfloat *))platform->getProcAddress("glMultMatrixf");
    gr3_glNewList = (void (*)(GLuint, GLenum))platform->getProcAddress("glNewList");
    gr3_glNormal3fv = (void (*)(const GLfloat *))platform->getProcAddress("glNormal3fv");
    gr3_glPixelStorei = (void (*)(GLenum, GLint))platform->getProcAddress("glPixelStorei");
    gr3_glPopMatrix = (void (*)(void))platform->getProcAddress("glPopMatrix");
    gr3_glPushMatrix = (void (*)(void))platform->getProcAddress("glPushMatrix");
    gr3_glReadBuffer = (void (*)(GLenum))platform->getProcAddress("glReadBuffer");
    gr3_glReadPixels =
        (void (*)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *))platform->getProcAddress("glReadPixels");
    gr3_glVertex3fv = (void (*)(const GLfloat *))platform->getProcAddress("glVertex3fv");
    gr3_glViewport = (void (*)(GLint, GLint, GLsizei, GLsizei))platform->getProcAddress("glViewport");
    gr3_glCullFace = (void (*)(GLenum))platform->getProcAddress("glCullFace");
    gr3_glGenTextures = (void (*)(GLsizei, GLuint *))platform->getProcAddress("glGenTextures");
    gr3_glDeleteTextures = (void (*)(GLsizei, const GLuint *))platform->getProcAddress("glDeleteTextures");
    gr3_glBindTexture = (void (*)(GLenum, GLuint))platform->getProcAddress("glBindTexture");
    gr3_glTexParameteri = (void (*)(GLenum, GLenum, GLint))platform->getProcAddress("glTexParameteri");
    gr3_glTexImage2D = (void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum,
                                 const GLvoid *))platform->getProcAddress("glTexImage2D");
  }
#endif
  return GR3_ERROR_NONE;
}
