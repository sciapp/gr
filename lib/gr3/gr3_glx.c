#define GR3_GLX_C
#include <stdlib.h>
#include "gr3.h"
#include "gr3_glx.h"
#include "gr3_internals.h"

/* OpenGL Context creation using GLX */

static Display *display; /*!< The used X display */
static Pixmap pixmap; /*!< The XPixmap (GLX < 1.4)*/
static GLXPbuffer pbuffer = (GLXPbuffer) NULL; /*!< The GLX Pbuffer (GLX >=1.4) */
static GLXContext context; /*!< The GLX context */

/*!
 * This function implements OpenGL context creation using GLX
 * and a Pbuffer if GLX version is 1.4 or higher, or a XPixmap
 * otherwise.
 * \returns
 * - ::GR3_ERROR_NONE         on success
 * - ::GR3_ERROR_INIT_FAILED  if initialization failed
 */
int gr3_initGL_GLX_(void) {
  int major = 0, minor = 0;
  int fbcount = 0;
  GLXFBConfig *fbc;
  GLXFBConfig fbconfig = (GLXFBConfig) NULL;
  gr3_log_("gr3_initGL_GLX_();");
  
  display = XOpenDisplay(0);
  if (!display) {
    gr3_log_("Not connected to an X server!");
    RETURN_ERROR(GR3_ERROR_INIT_FAILED);
  }
  if (!glXQueryExtension(display, NULL, NULL)) {
    gr3_log_("GLX not supported!");
    RETURN_ERROR(GR3_ERROR_INIT_FAILED);
  }
  
  context = glXGetCurrentContext();
  if (context != NULL) {
    gr3_appendtorenderpathstring_("GLX (existing context)");
  } else {
    /* call glXQueryVersion twice to prevent bugs in virtualbox */
    if (!glXQueryVersion(display,&major,&minor) && !glXQueryVersion(display,&major,&minor)) {
      RETURN_ERROR(GR3_ERROR_INIT_FAILED);
    }
    if (major > 1 || minor >=4) {
      int i;
      int fb_attribs[] =
      {
        GLX_DRAWABLE_TYPE   , GLX_PBUFFER_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        None
      };
      int pbuffer_attribs[] =
      {
        GLX_PBUFFER_WIDTH   , 1,
        GLX_PBUFFER_HEIGHT   , 1,
        None
      };
      gr3_log_("(Pbuffer)");
      
      fbc = glXChooseFBConfig(display, DefaultScreen(display), fb_attribs,
                              &fbcount);
      if (fbcount == 0) {
        gr3_log_("failed to find a valid a GLX FBConfig for a RGBA PBuffer");
        XFree(fbc);
        XCloseDisplay(display);
        RETURN_ERROR(GR3_ERROR_INIT_FAILED);
      }
      for (i = 0; i < fbcount && !pbuffer; i++) {
        fbconfig = fbc[i];
        pbuffer = glXCreatePbuffer(display, fbconfig, pbuffer_attribs);
      }
      XFree(fbc);
      if (!pbuffer) {
        gr3_log_("failed to create a RGBA PBuffer");
        XCloseDisplay(display);
        RETURN_ERROR(GR3_ERROR_INIT_FAILED);
      }
      
      context = glXCreateNewContext(display, fbconfig, GLX_RGBA_TYPE, None, True);
      glXMakeContextCurrent(display,pbuffer,pbuffer,context);
      
      context_struct_.terminateGL = gr3_terminateGL_GLX_Pbuffer_;
      context_struct_.gl_is_initialized = 1;
      gr3_appendtorenderpathstring_("GLX (Pbuffer)");
    } else {
      XVisualInfo *visual;
      int fb_attribs[] =
      {
        GLX_DRAWABLE_TYPE   , GLX_PIXMAP_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        None
      };
      gr3_log_("(XPixmap)");
      fbc = glXChooseFBConfig(display, DefaultScreen(display), fb_attribs,
                              &fbcount);
      if (fbcount == 0) {
        gr3_log_("failed to find a valid a GLX FBConfig for a RGBA Pixmap");
        XFree(fbc);
        XCloseDisplay(display);
        RETURN_ERROR(GR3_ERROR_INIT_FAILED);
      }
      fbconfig = fbc[0];
      XFree(fbc);
      
      context = glXCreateNewContext(display, fbconfig, GLX_RGBA_TYPE, None, True);
      visual = glXGetVisualFromFBConfig(display,fbconfig);
      pixmap = XCreatePixmap(display,XRootWindow(display,DefaultScreen(display)),1,1,visual->depth);
      
      if (glXMakeContextCurrent(display,pixmap,pixmap,context)) {
        context_struct_.terminateGL = gr3_terminateGL_GLX_Pixmap_;
        context_struct_.gl_is_initialized = 1;
        gr3_appendtorenderpathstring_("GLX (XPixmap)");
      } else {
        gr3_log_("failed to make GLX OpenGL Context current with a Pixmap");
        glXDestroyContext(display, context);
        XFreePixmap(display,pixmap);
        XCloseDisplay(display);
        RETURN_ERROR(GR3_ERROR_INIT_FAILED);
      }
    }
  }
  /* Load Function pointers */ {
#ifdef GR3_CAN_USE_VBO
    gr3_glBufferData = (PFNGLBUFFERDATAPROC)glXGetProcAddress((const GLubyte *)"glBufferData");
    gr3_glBindBuffer = (PFNGLBINDBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBindBuffer");
    gr3_glGenBuffers = (PFNGLGENBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glGenBuffers");
    gr3_glDeleteBuffers = (PFNGLGENBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glDeleteBuffers");
    gr3_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)glXGetProcAddress((const GLubyte *)"glVertexAttribPointer");
    gr3_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)glXGetProcAddress((const GLubyte *)"glGetAttribLocation");
    gr3_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glXGetProcAddress((const GLubyte *)"glEnableVertexAttribArray");
    gr3_glUseProgram = (PFNGLUSEPROGRAMPROC)glXGetProcAddress((const GLubyte *)"glUseProgram");
    gr3_glDeleteShader = (PFNGLDELETESHADERPROC)glXGetProcAddress((const GLubyte *)"glDeleteShader");
    gr3_glLinkProgram = (PFNGLLINKPROGRAMPROC)glXGetProcAddress((const GLubyte *)"glLinkProgram");
    gr3_glAttachShader = (PFNGLATTACHSHADERPROC)glXGetProcAddress((const GLubyte *)"glAttachShader");
    gr3_glCreateShader = (PFNGLCREATESHADERPROC)glXGetProcAddress((const GLubyte *)"glCreateShader");
    gr3_glCompileShader = (PFNGLCOMPILESHADERPROC)glXGetProcAddress((const GLubyte *)"glCompileShader");
    gr3_glCreateProgram = (PFNGLCREATEPROGRAMPROC)glXGetProcAddress((const GLubyte *)"glCreateProgram");
    gr3_glDeleteProgram = (PFNGLDELETEPROGRAMPROC)glXGetProcAddress((const GLubyte *)"glDeleteProgram");
    gr3_glUniform3f = (PFNGLUNIFORM3FPROC)glXGetProcAddress((const GLubyte *)"glUniform3f");
    gr3_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)glXGetProcAddress((const GLubyte *)"glUniformMatrix4fv");
    gr3_glUniform4f = (PFNGLUNIFORM4FPROC)glXGetProcAddress((const GLubyte *)"glUniform4f");
    gr3_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)glXGetProcAddress((const GLubyte *)"glGetUniformLocation");
    gr3_glShaderSource = (PFNGLSHADERSOURCEPROC)glXGetProcAddress((const GLubyte *)"glShaderSource");
#endif
    gr3_glDrawBuffers = (PFNGLDRAWBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glDrawBuffers");
    /*glBlendColor = (PFNGLBLENDCOLORPROC)glXGetProcAddress((const GLubyte *)"glBlendColor");*/
#ifdef GL_ARB_framebuffer_object
    gr3_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBindRenderbuffer");
    gr3_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)glXGetProcAddress((const GLubyte *)"glCheckFramebufferStatus");
    gr3_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)glXGetProcAddress((const GLubyte *)"glFramebufferRenderbuffer");
    gr3_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)glXGetProcAddress((const GLubyte *)"glRenderbufferStorage");
    gr3_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBindFramebuffer");
    gr3_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glGenFramebuffers");
    gr3_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glGenRenderbuffers");
    gr3_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glDeleteFramebuffers");
    gr3_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glDeleteRenderbuffers");
#endif
#ifdef GL_EXT_framebuffer_object
    gr3_glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)glXGetProcAddress((const GLubyte *)"glBindRenderbufferEXT");
    gr3_glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)glXGetProcAddress((const GLubyte *)"glCheckFramebufferStatusEXT");
    gr3_glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)glXGetProcAddress((const GLubyte *)"glFramebufferRenderbufferEXT");
    gr3_glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)glXGetProcAddress((const GLubyte *)"glRenderbufferStorageEXT");
    gr3_glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)glXGetProcAddress((const GLubyte *)"glBindFramebufferEXT");
    gr3_glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)glXGetProcAddress((const GLubyte *)"glGenFramebuffersEXT");
    gr3_glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)glXGetProcAddress((const GLubyte *)"glGenRenderbuffersEXT");
    gr3_glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)glXGetProcAddress((const GLubyte *)"glDeleteFramebuffersEXT");
    gr3_glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)glXGetProcAddress((const GLubyte *)"glDeleteRenderbuffersEXT");
#endif
  }
  return GR3_ERROR_NONE;
}

/*!
 * This function destroys the OpenGL context using GLX with a Pbuffer.
 */
void gr3_terminateGL_GLX_Pbuffer_(void) {
  gr3_log_("gr3_terminateGL_GLX_Pbuffer_();");
  
  glXMakeContextCurrent(display,None,None,NULL);
  glXDestroyContext(display, context);
  /*glXDestroyPbuffer(display, pbuffer);*/
  XCloseDisplay(display);
  context_struct_.gl_is_initialized = 0;
}

/*!
 * This function destroys the OpenGL context using GLX with a XPixmap.
 */
void gr3_terminateGL_GLX_Pixmap_(void) {
  gr3_log_("gr3_terminateGL_GLX_Pixmap_();");
  
  glXMakeContextCurrent(display,None,None,NULL);
  glXDestroyContext(display, context);
  XFreePixmap(display,pixmap);
  XCloseDisplay(display);
  context_struct_.gl_is_initialized = 0;
}
