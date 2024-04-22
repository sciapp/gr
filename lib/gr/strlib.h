#ifndef _STRLIB_H_
#define _STRLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SCIENTIFIC_FORMAT_OPTION_E: 1.2E5
 * SCIENTIFIC_FORMAT_OPTION_TEXTEX: 1.2x10^{5} (x replaced with unicode-sign)
 * SCIENTIFIC_FORMAT_OPTION_MATHTEX: 1.2\times10^{5}
 */

#define SCIENTIFIC_FORMAT_OPTION_E 1
#define SCIENTIFIC_FORMAT_OPTION_TEXTEX 2
#define SCIENTIFIC_FORMAT_OPTION_MATHTEX 3

typedef struct
{
  int scientific;
  int decimal_digits;
} str_format_reference_t;

str_format_reference_t *str_get_format_reference(str_format_reference_t *, double, double, double, double, int);
char *str_remove(char *, char);
char *str_pad(char *, char, int);
char *str_ftoa(char *, double, str_format_reference_t *, int);
int str_casecmp(char *, char *);
int str_utf8_to_unicode(const unsigned char *utf8_str, int *length);

#ifdef __cplusplus
}
#endif

#endif
