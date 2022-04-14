#ifndef _STRLIB_H_
#define _STRLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  int scientific;
  int decimal_digits;
} format_reference_t;

format_reference_t *str_get_format_reference(format_reference_t *, double, double, double, double, int);
char *str_remove(char *, char);
char *str_pad(char *, char, int);
char *str_ftoa(char *, double, format_reference_t *);
int str_casecmp(char *, char *);

#ifdef __cplusplus
}
#endif

#endif
