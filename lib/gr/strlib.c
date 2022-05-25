#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#ifdef _MSC_VER
typedef __int64 int64_t;
#else
#include <inttypes.h>
#include <stdint.h>
#endif

#if !defined(VMS) && !defined(_WIN32)
#include <unistd.h>
#endif

#include "strlib.h"

/*
 * Maximum number of decimal digits: 15
 * Maximum length of the exponent: 4
 * Mathtex symbols: 11 \times10^{}
 * Negative signs: 2
 */
#define STR_MAX 32

/*
 * maximum number of digits to be used in standard notation
 */
#if !defined(LENGTH_DIGITS)
#define LENGTH_DIGITS 14
#endif

#if !defined(SCIENTIFIC_EXP_LOW)
#define SCIENTIFIC_EXP_LOW -14
#endif

#if !defined(SCIENTIFIC_EXP_HIGH)
#define SCIENTIFIC_EXP_HIGH 14
#endif

#ifndef isnan
#define isnan(x) ((x) != (x))
#endif
#ifndef isinf
#define isinf(x) (!isnan(x) && isnan((x) - (x)))
#endif
#ifndef round
#define round(x) (((x) < 0) ? ceil((x)-.5) : floor((x) + .5))
#endif

format_reference_t *str_get_format_reference(format_reference_t *result, double origin, double min, double max,
                                             double tick_width, int major)

/*
 * str_get_reference - Checks for each value between min and max (step-size: tick_width) if there is one value which
 *                      needs scientific notation. Also counts the maximum number of decimal places.
 *                      Both values are stored in the result reference
 */

{
  int i, steps;
  int64_t exponent;

  result->decimal_digits = 0;
  result->scientific = 0;

  if (major != 0)
    {
      tick_width = major * tick_width;
    }

  steps = round((max - min) / tick_width);

  /* Check if there is one value between min and max which needs scientific notation */
  for (i = 0; i <= steps; i++)
    {
      double current = min + i * tick_width;
      if (current != origin || origin == min || origin == max)
        {
          if (current != 0)
            {
              exponent = (int64_t)floor(log10(fabs(current)));
              if (exponent <= SCIENTIFIC_EXP_LOW || exponent >= SCIENTIFIC_EXP_HIGH)
                {
                  result->scientific = 1;
                  break;
                }
            }
        }
    }
  /* Count number of decimal digits */
  if (!result->scientific)
    {
      double tick_width_multiplied = tick_width;
      while ((int64_t)tick_width_multiplied < tick_width_multiplied &&
             log10(tick_width_multiplied - (int64_t)tick_width_multiplied) >= result->decimal_digits - LENGTH_DIGITS &&
             result->decimal_digits < LENGTH_DIGITS)
        {
          result->decimal_digits++;
          tick_width_multiplied = (tick_width + 1e-15) * pow(10.0, result->decimal_digits);
        }
    }
  return result;
}

char *str_remove(char *str, char ch)

/*
 *  str_remove - remove trailing characters
 */

{
  int i;

  i = strlen(str) - 1;
  while (i >= 0 && str[i] == ch) str[i--] = '\0';

  return str;
}


char *str_pad(char *str, char fill, int size)

/*
 *  str_pad - pad string with fill character
 */

{
  int i, len;

  len = strlen(str);
  for (i = len; i < size; i++) str[i] = fill;

  if (size < 0) size = 0;
  str[size] = '\0';

  return str;
}


static char *str_write_decimal(char *result, int64_t mantissa, int decimal_point, int decimal_digits)

/*
 * str_write_decimal - Writes the decimal number (mantissa split by the decimal point) into result.
 */

{
  int i;
  char *end = NULL;

  /* skip non-significant digits */
  for (i = 0; i < decimal_point; i++)
    {
      if (mantissa % 10 != 0 || decimal_point - i <= decimal_digits)
        {
          end = &result[LENGTH_DIGITS - i + 1];
          break;
        }
      mantissa /= 10;
    }

  /* write or skip decimal point */
  if (end)
    {
      result[LENGTH_DIGITS - decimal_point] = '.';
    }
  else
    {
      end = &result[LENGTH_DIGITS - decimal_point];
    }

  /* write significant digits */
  for (; i < LENGTH_DIGITS + 1; i++)
    {
      if (i == decimal_point)
        {
          /* decimal point was handled already */
          continue;
        }
      result[LENGTH_DIGITS - i] = '0' + mantissa % 10;
      mantissa /= 10;
    }

  /* end string with null byte */
  end[0] = '\0';

  return end;
}


char *str_ftoa(char *result, double value, format_reference_t *reference, int format_option)

/*
 * str_ftoa - Convert a real value to string in scientific notation. The reference is used to specify the number of
 *            decimal places and the usage of scientific notation.
 */

{
  char *mantissa_str = result;
  double abs_value = fabs(value);

  int64_t exponent, decimal_places, mantissa;

  format_reference_t local_reference;

  if (value == 0)
    {
      result[0] = '0';
      result[1] = '\0';
    }
  else if (isnan(value))
    {
      result[0] = 'n';
      result[1] = 'a';
      result[2] = 'n';
      result[3] = '\0';
    }
  else if (isinf(abs_value))
    {
      char *inf_str = result;
      if (value < 0)
        {
          result[0] = '-';
          inf_str = &result[1];
        }
      inf_str[0] = 'i';
      inf_str[1] = 'n';
      inf_str[2] = 'f';
      inf_str[3] = '\0';
    }
  else
    {

      exponent = (int64_t)(floor(log10(abs_value)));
      decimal_places = (LENGTH_DIGITS - 1) - exponent;

      /* When reference is null check wether scientific notation is needed or not */
      if (!reference)
        {
          local_reference.scientific = (exponent <= SCIENTIFIC_EXP_LOW || exponent >= SCIENTIFIC_EXP_HIGH);
          local_reference.decimal_digits = 0;
          reference = &local_reference;
        }

      if (exponent < 0 && !reference->scientific)
        {
          mantissa = (int64_t)(round(abs_value * pow(10.0, LENGTH_DIGITS - 1)));
        }
      else
        {
          mantissa = (int64_t)(round(abs_value * pow(10.0, decimal_places)));
        }

      if (value < 0)
        {
          result[0] = '-';
          mantissa_str = &result[1];
        }

      if (reference->scientific)
        {
          char *exponent_str = NULL;
          int64_t exponent_abs = exponent;
          int exponent_length;
          int i = 0;

          exponent_str = str_write_decimal(mantissa_str, mantissa, LENGTH_DIGITS - 1, reference->decimal_digits);

          if (format_option == SCIENTIFIC_FORMAT_OPTION_E)
            {
              exponent_str[0] = 'E';
            }
          else
            {
              if (format_option == SCIENTIFIC_FORMAT_OPTION_TEXTEX)
                {
                  exponent_str[0] = 0xC3;
                  exponent_str[1] = 0x97;
                  exponent_str = &exponent_str[2];
                }
              else if (format_option == SCIENTIFIC_FORMAT_OPTION_MATHTEX)
                {
                  exponent_str[0] = '\\';
                  exponent_str[1] = 't';
                  exponent_str[2] = 'i';
                  exponent_str[3] = 'm';
                  exponent_str[4] = 'e';
                  exponent_str[5] = 's';
                  exponent_str = &exponent_str[6];
                }
              exponent_str[0] = '1';
              exponent_str[1] = '0';
              exponent_str[2] = '^';
              exponent_str[3] = '{';
              exponent_str = &exponent_str[3];
            }

          if (exponent < 0)
            {
              exponent_abs *= -1;
              exponent_str[1] = '-';
              exponent_str = &exponent_str[1];
            }

          exponent_length = log10(exponent_abs) + 1;


          for (i = exponent_length; i > 0; i--)
            {
              exponent_str[i] = ('0' + exponent_abs % 10);
              exponent_abs /= 10;
            }

          if (format_option == SCIENTIFIC_FORMAT_OPTION_TEXTEX || format_option == SCIENTIFIC_FORMAT_OPTION_MATHTEX)
            {
              exponent_str[exponent_length + 1] = '}';
              exponent_str[exponent_length + 2] = '\0';
            }
          else
            {
              exponent_str[exponent_length + 1] = '\0';
            }
        }
      else
        {
          if (exponent >= 0)
            {
              str_write_decimal(mantissa_str, mantissa, decimal_places, reference->decimal_digits);
            }
          else
            {
              str_write_decimal(mantissa_str, mantissa, LENGTH_DIGITS - 1, reference->decimal_digits);
            }
        }
    }

  return result;
}


int str_casecmp(char *s1, char *s2)
{
  while ((*s1 != '\0') && (tolower(*(unsigned char *)s1) == tolower(*(unsigned char *)s2)))
    {
      s1++;
      s2++;
    }

  return tolower(*(unsigned char *)s1) - tolower(*(unsigned char *)s2);
}


int str_utf8_to_unicode(const unsigned char *utf8_str, int *length)
{
  int codepoint;

  if ((utf8_str[0] & 0x80U) == 0x00U)
    {
      *length = 1;
      codepoint = utf8_str[0U];
    }
  else if ((utf8_str[0] & 0xe0U) == 0xc0U && (utf8_str[1] & 0xc0U) == 0x80U)
    {
      *length = 2;
      codepoint = ((utf8_str[0] & 0x1fU) << 6U) + (utf8_str[1] & 0x3fU);
    }
  else if ((utf8_str[0] & 0xf0U) == 0xe0U && (utf8_str[1] & 0xc0U) == 0x80U && (utf8_str[2] & 0xc0U) == 0x80U)
    {
      *length = 3;
      codepoint = ((utf8_str[0] & 0xfU) << 12U) + ((utf8_str[1] & 0x3fU) << 6U) + (utf8_str[2] & 0x3fU);
    }
  else if ((utf8_str[0] & 0xf8U) == 0xf0U && (utf8_str[1] & 0xc0U) == 0x80U && (utf8_str[2] & 0xc0U) == 0x80U &&
           (utf8_str[3] & 0xc0U) == 0x80U)
    {
      *length = 4;
      codepoint = ((utf8_str[0] & 0x7U) << 18U) + ((utf8_str[1] & 0x3fU) << 12U) + ((utf8_str[2] & 0x3fU) << 6U) +
                  (utf8_str[3] & 0x3fU);
    }
  else
    {
      /* invalid byte combination */
      *length = 1;
      codepoint = (unsigned int)'?';
    }
  return codepoint;
}
