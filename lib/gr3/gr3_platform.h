#ifndef GR3_PLATFORM_H_INCLUDED
#define GR3_PLATFORM_H_INCLUDED

int gr3_platform_initGL_(void);
void (*(gr3_platform_getProcAddress)(const char *))();

#endif
