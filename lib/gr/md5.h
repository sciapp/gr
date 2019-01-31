#ifndef _MD5_H_
#define _MD5_H_

#define MD5_SIZE 16

#ifdef __cplusplus
extern "C"
{
#endif

  extern void md5(const char *buffer, char *sum);

#ifdef __cplusplus
}
#endif

#endif
