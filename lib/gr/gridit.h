#ifdef _WIN32

#define HAVE_BOOLEAN

#include <windows.h>	/* required for all Windows applications */
#define DLLEXPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

#else

#ifdef __cplusplus
#define DLLEXPORT extern "C"
#else
#define DLLEXPORT
#endif

#endif

DLLEXPORT void gr_gridit(int nd, float *xd, float *yd, float *zd,
  int nx, int ny, float *x, float *y, float *z);
