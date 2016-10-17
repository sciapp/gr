#define GR3_WIN_C

#include "gr3.h"
#include "gr3_win.h"
#include "gr3_internals.h"

/* OpenGL Context creation on windows */

static HINSTANCE g_hInstance;
static HWND hWnd;
static HDC dc;
static HGLRC glrc;

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD  fdwReason, LPVOID lpvReserved) {
  (void)lpvReserved;
  if (fdwReason == DLL_PROCESS_ATTACH) {
    g_hInstance = hInstance;
    /*fprintf(stderr,"DLL attached to a process\n");*/
  }
  return TRUE;
}

int gr3_initGL_WIN_(void) {
  WNDCLASS   wndclass;
  gr3_log_("gr3_initGL_WIN_();");
  glrc = wglGetCurrentContext();
  if (!glrc) {
    /* Register the frame class */
    wndclass.style         = 0;
    wndclass.lpfnWndProc   = DefWindowProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = g_hInstance;
    wndclass.hIcon         = NULL;
    wndclass.hCursor       = LoadCursor (NULL,IDC_ARROW);
    wndclass.hbrBackground = NULL;
    wndclass.lpszMenuName  = "OpenGLWindow";
    wndclass.lpszClassName = "OpenGLWindow";
    
    if (RegisterClass(&wndclass))  {
      /*fprintf(stderr,"Window Class registered successfully.\n"); */
    } else {
      gr3_log_("failed to register a window class");
      return FALSE;
    }
    hWnd = CreateWindow ("OpenGLWindow",
                         "Generic OpenGL Sample",
                         0,
                         0,
                         0,
                         1,
                         1,
                         NULL,
                         NULL,
                         g_hInstance,
                         NULL);
    if (hWnd != NULL) {
      /*fprintf(stderr,"Window created successfully.\n"); */
    } else {
      gr3_log_("failed to create a window");
      RETURN_ERROR(GR3_ERROR_INIT_FAILED);
    }
    
    dc = GetDC(hWnd);
    
    /* Pixel Format selection */ {
      PIXELFORMATDESCRIPTOR pfd;
      int iPixelFormat;
      BOOL result;
      memset(&pfd,0,sizeof(pfd));
      pfd.nSize = sizeof(pfd);
      pfd.nVersion = 1;
      pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
      pfd.iPixelType = PFD_TYPE_RGBA;
      pfd.cColorBits = 24;
      pfd.cAlphaBits = 8;
      pfd.cDepthBits = 24;
      pfd.iLayerType = PFD_MAIN_PLANE;
      iPixelFormat = ChoosePixelFormat(dc,&pfd);
      result = SetPixelFormat(dc,iPixelFormat, &pfd);
      if (result) {
        /*fprintf(stderr,"Pixel Format set for Device Context successfully.\n");*/
      } else {
        gr3_log_("failed to set a pixel format for the device context");
        RETURN_ERROR(GR3_ERROR_INIT_FAILED);
      }
    }
    
    /* OpenGL Rendering Context creation */ {
      BOOL result;
      glrc = wglCreateContext(dc);
      if (glrc != NULL) {
        /*fprintf(stderr,"OpenGL Rendering Context was created successfully.\n");*/
      } else {
        gr3_log_("failed to create an OpenGL context for the device context");
        RETURN_ERROR(GR3_ERROR_INIT_FAILED);
      }
      result = wglMakeCurrent(dc,glrc);
      if (result) {
        /*fprintf(stderr,"OpenGL Rendering Context made current successfully.\n");*/
      } else {
        gr3_log_("failed to make OpenGL context current with the device context");
        RETURN_ERROR(GR3_ERROR_INIT_FAILED);
      }
    }
  }
  /* Load Function pointers */ {
#ifdef GR3_CAN_USE_VBO
    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    glDeleteBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
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
    glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
    glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
    glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
    glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
    glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
    glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
    glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffersEXT");
#endif
  }
  context_struct_.terminateGL = gr3_terminateGL_WIN_;
  context_struct_.gl_is_initialized = 1;
  gr3_appendtorenderpathstring_("Windows");
  return GR3_ERROR_NONE;
}
void gr3_terminateGL_WIN_(void) {
  gr3_log_("gr3_terminateGL_WIN_();");
  if (dc) {
    wglDeleteContext(glrc);
    ReleaseDC(hWnd,dc);
    DestroyWindow(hWnd);
    UnregisterClass("OpenGLWindow", g_hInstance);
  }
}
