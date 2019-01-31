#ifndef _STRLIB_H_
#define _STRLIB_H_

#ifdef __cplusplus
extern "C"
{
#endif

  char *str_remove(char *, char);
  char *str_pad(char *, char, int);
  char *str_ftoa(char *, double, double);
  int str_casecmp(char *, char *);

#ifdef __cplusplus
}
#endif

#endif
