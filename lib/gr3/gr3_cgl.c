#define GR3_CGL_C
#include <stdlib.h>
#include "gr3.h"
#include "gr3_cgl.h"
#define DONT_USE_RETURN_ERROR
#include "gr3_internals.h"

/* OpenGL Context creation using CGL */

static CGLContextObj cgl_ctx; /*!< A reference to the CGL context */

/*!
 * This function implements OpenGL context creation using CGL and a Pbuffer.
 * \returns
 * - ::GR3_ERROR_NONE         on success
 * - ::GR3_ERROR_INIT_FAILED  if an error occurs, additional information might be
 *                            available via the logging callback.
 */
int gr3_initGL_CGL_(void) {
  CGLContextObj ctx;
  CGLPixelFormatObj pix; /* pixel format */
  GLint npix; /* number of virtual screens referenced by pix after
               call to CGLChoosePixelFormat*/
  const CGLPixelFormatAttribute pf_attributes[] = {
    kCGLPFAColorSize, 24,
    kCGLPFAAlphaSize, 8,
    kCGLPFADepthSize, 24,
    /*kCGLPFAOffScreen,*//* Using a PBuffer is hardware accelerated, so */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    kCGLPFAPBuffer,    /* we want to use that. */
#pragma clang diagnostic pop
    0, 0
  };
  gr3_log_("gr3_initGL_CGL_();");
  ctx = CGLGetCurrentContext();
  if (ctx == NULL) {
    CGLChoosePixelFormat(pf_attributes, &pix, &npix);
    
    CGLCreateContext(pix,NULL,&ctx);
    CGLReleasePixelFormat(pix);
    
    CGLSetCurrentContext(ctx);
    gr3_appendtorenderpathstring_("CGL");
  } else {
    CGLRetainContext(ctx);
    gr3_appendtorenderpathstring_("CGL (existing context)");
  }
  cgl_ctx = ctx;
  
  context_struct_.terminateGL = gr3_terminateGL_CGL_;
  context_struct_.gl_is_initialized = 1;
  return GR3_ERROR_NONE;
}

/*!
 * This function destroys the OpenGL context using CGL.
 */
void gr3_terminateGL_CGL_(void) {
  gr3_log_("gr3_terminateGL_CGL_();");
  
  CGLReleaseContext(cgl_ctx);
  context_struct_.gl_is_initialized = 0;
}
