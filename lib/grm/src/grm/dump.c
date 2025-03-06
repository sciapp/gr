#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include <limits.h>
#include <string.h>
#ifndef _WIN32
#include <sys/ioctl.h>
#endif

#include "dump_int.h"
#include "json_int.h"
#include "bson_int.h"
#include "memwriter_int.h"
#include "plot_int.h"


/* ######################### public implementation ################################################################## */

/* ========================= functions ============================================================================== */

/* ------------------------- dump ----------------------------------------------------------------------------------- */

void grm_dump(const grm_args_t *args, FILE *f)
{
#define BUFFER_LEN 200
#define DEFAULT_ARRAY_PRINT_ELEMENTS_COUNT 10
#define DARK_BACKGROUND_ENV_KEY "GRM_DARK_BACKGROUND"
#define ARRAY_PRINT_TRUNCATION_ENV_KEY "GRM_ARRAY_PRINT_TRUNCATION"
  grm_args_iterator_t *it;
  grm_args_value_iterator_t *value_it;
  grm_arg_t *arg;
  unsigned int i;
  char buffer[BUFFER_LEN];
  int count_characters;
  static int recursion_level = -1;
  int columns, cursor_xpos = 0;
  int use_color_codes;
  int has_dark_bg;
  struct GrmDumpColorCodes
  {
    unsigned char k, i, d, c, s;
  } color_codes;
  unsigned int array_print_elements_count = DEFAULT_ARRAY_PRINT_ELEMENTS_COUNT;
#if defined(_WIN32) || defined(__EMSCRIPTEN__)
  columns = INT_MAX;
  use_color_codes = 0;
  has_dark_bg = 0;
#else
  struct winsize w;
  use_color_codes = isatty(fileno(f));
  ioctl(0, TIOCGWINSZ, &w);
  columns = w.ws_col;
  if (getenv(DARK_BACKGROUND_ENV_KEY) != NULL &&
      strEqualsAny(getenv(DARK_BACKGROUND_ENV_KEY), "1", "yes", "YES", "on", "ON", NULL))
    {
      has_dark_bg = 1;
      color_codes.k = 122;
      color_codes.i = 81;
      color_codes.d = 215;
      color_codes.c = 228;
      color_codes.s = 155;
    }
  else
    {
      has_dark_bg = 0;
      color_codes.k = 18;
      color_codes.i = 25;
      color_codes.d = 88;
      color_codes.c = 55;
      color_codes.s = 22;
    }
#endif
  if (getenv(ARRAY_PRINT_TRUNCATION_ENV_KEY) != NULL)
    {
      if (strEqualsAny(getenv(ARRAY_PRINT_TRUNCATION_ENV_KEY), "", "0", "inf", "INF", "unlimited", "UNLIMITED", "off",
                       "OFF", NULL))
        {
          array_print_elements_count = UINT_MAX;
        }
      else
        {
          strToUint(getenv(ARRAY_PRINT_TRUNCATION_ENV_KEY), &array_print_elements_count);
        }
    }

#define INDENT 2

#define printIndent                                                                                                    \
  do                                                                                                                   \
    {                                                                                                                  \
      if (use_color_codes)                                                                                             \
        {                                                                                                              \
          int i;                                                                                                       \
          for (i = 0; i < recursion_level; ++i)                                                                        \
            {                                                                                                          \
              fprintf(f, "\033[48;5;%dm%*s\033[0m", has_dark_bg ? (235 + (5 * i) % 25) : (255 - (5 * i) % 25), INDENT, \
                      "");                                                                                             \
            }                                                                                                          \
        }                                                                                                              \
      else                                                                                                             \
        {                                                                                                              \
          fprintf(f, "%*s", INDENT *recursion_level, "");                                                              \
        }                                                                                                              \
    }                                                                                                                  \
  while (0)

#define printKey                                                           \
  do                                                                       \
    {                                                                      \
      printIndent;                                                         \
      if (use_color_codes)                                                 \
        {                                                                  \
          fprintf(f, "\033[38;5;%dm%s\033[0m: ", color_codes.k, arg->key); \
        }                                                                  \
      else                                                                 \
        {                                                                  \
          fprintf(f, "%s: ", arg->key);                                    \
        }                                                                  \
    }                                                                      \
  while (0)

#define printValue(value_type, format_string, color_code)                                                        \
  do                                                                                                             \
    {                                                                                                            \
      if (use_color_codes)                                                                                       \
        {                                                                                                        \
          fprintf(f, "\033[38;5;%dm" format_string "\033[0m", color_code, *((value_type *)value_it->value_ptr)); \
        }                                                                                                        \
      else                                                                                                       \
        {                                                                                                        \
          fprintf(f, format_string, *((value_type *)value_it->value_ptr));                                       \
        }                                                                                                        \
    }                                                                                                            \
  while (0)

#define printValues(value_type, format_string, color_code)                                                          \
  do                                                                                                                \
    {                                                                                                               \
      int print_last_element = 0;                                                                                   \
      fputc('[', f);                                                                                                \
      cursor_xpos += strlen(arg->key) + 3;                                                                          \
      for (i = 0; i < grm_min(value_it->array_length, array_print_elements_count); i++)                             \
        {                                                                                                           \
          if (print_last_element)                                                                                   \
            {                                                                                                       \
              i = value_it->array_length - 1;                                                                       \
            }                                                                                                       \
          if (array_print_elements_count >= value_it->array_length || i != array_print_elements_count - 2)          \
            {                                                                                                       \
              if (use_color_codes)                                                                                  \
                {                                                                                                   \
                  count_characters =                                                                                \
                      snprintf(buffer, BUFFER_LEN,                                                                  \
                               "\033[38;5;%dm" format_string "\033[0m"                                              \
                               "%s",                                                                                \
                               color_code, (*((value_type **)value_it->value_ptr))[i],                              \
                               (i < grm_min(value_it->array_length, array_print_elements_count) - 1) ? ", " : "]"); \
                  count_characters -= (color_code >= 100) ? 15 : ((color_code >= 10) ? 14 : 13);                    \
                }                                                                                                   \
              else                                                                                                  \
                {                                                                                                   \
                  count_characters =                                                                                \
                      snprintf(buffer, BUFFER_LEN, format_string "%s", (*((value_type **)value_it->value_ptr))[i],  \
                               (i < grm_min(value_it->array_length, array_print_elements_count) - 1) ? ", " : "]"); \
                }                                                                                                   \
            }                                                                                                       \
          else                                                                                                      \
            {                                                                                                       \
              count_characters = snprintf(buffer, BUFFER_LEN, "..., ");                                             \
              print_last_element = 1;                                                                               \
            }                                                                                                       \
          if (cursor_xpos + count_characters > columns)                                                             \
            {                                                                                                       \
              fputc('\n', f);                                                                                       \
              printIndent;                                                                                          \
              cursor_xpos = INDENT * recursion_level + fprintf(f, "%*s", (int)strlen(arg->key) + 3, "") - 1;        \
            }                                                                                                       \
          fputs(buffer, f);                                                                                         \
          cursor_xpos += count_characters;                                                                          \
        }                                                                                                           \
      if (value_it->array_length == 0) fputc(']', f);                                                               \
    }                                                                                                               \
  while (0)

#define printType(value_type, format_string, color_code)      \
  do                                                          \
    {                                                         \
      if (value_it->is_array)                                 \
        {                                                     \
          printKey;                                           \
          printValues(value_type, format_string, color_code); \
        }                                                     \
      else                                                    \
        {                                                     \
          printKey;                                           \
          printValue(value_type, format_string, color_code);  \
        }                                                     \
      fputc('\n', f);                                         \
    }                                                         \
  while (0)

  ++recursion_level;

  it = grm_args_iter(args);
  while ((arg = it->next(it)) != NULL)
    {
      if (*arg->value_format)
        {
          value_it = grm_arg_value_iter(arg);
          while (value_it->next(value_it) != NULL)
            {
              cursor_xpos = INDENT * recursion_level;
              switch (value_it->format)
                {
                case 'i':
                  printType(int, "% d", color_codes.i);
                  break;
                case 'd':
                  printType(double, "% lf", color_codes.d);
                  break;
                case 'c':
                  printType(char, "'%c'", color_codes.c);
                  break;
                case 's':
                  printType(char *, "\"%s\"", color_codes.s);
                  break;
                case 'a':
                  if (value_it->is_array)
                    {
                      printKey;
                      fprintf(f, "[\n");
                      for (i = 0; i < value_it->array_length; i++)
                        {
                          grm_dump((*((grm_args_t ***)value_it->value_ptr))[i], f);
                          if (i < value_it->array_length - 1)
                            {
                              printIndent;
                              fprintf(f, ",\n");
                            }
                        }
                      printIndent;
                      fprintf(f, "]\n");
                    }
                  else
                    {
                      printKey;
                      fprintf(f, "\n");
                      grm_dump(*((grm_args_t **)value_it->value_ptr), f);
                      fprintf(f, "\n");
                    }
                  break;
                default:
                  break;
                }
            }
          argsValueIteratorDelete(value_it);
        }
      else
        {
          printKey;
          fprintf(f, "(none)\n");
        }
    }
  argsIteratorDelete(it);

  --recursion_level;

#undef BUFFER_LEN
#undef DEFAULT_ARRAY_PRINT_ELEMENTS_COUNT
#undef DARK_BACKGROUND_ENV_KEY
#undef ARRAY_PRINT_TRUNCATION_ENV_KEY
#undef INDENT
#undef printIndent
#undef printKey
#undef printType
}

void grm_dump_json(const grm_args_t *args, FILE *f)
{
  static Memwriter *memwriter = NULL;

  if (memwriter == NULL) memwriter = memwriterNew();
  toJsonWriteArgs(memwriter, args);
  if (toJsonIsComplete())
    {
      memwriterPutc(memwriter, '\0');
      fprintf(f, "%s\n", memwriterBuf(memwriter));
      memwriterDelete(memwriter);
      memwriter = NULL;
    }
}

char *grm_dump_json_str(void)
{
  static Memwriter *memwriter = NULL;
  char *result;

  if (memwriter == NULL) memwriter = memwriterNew();
  /* toJsonWriteArgs(memwriter, global_root_args); */
  toJsonWriteArgs(memwriter, active_plot_args);
  if (toJsonIsComplete())
    {
      memwriterPutc(memwriter, '\0');
      result = malloc(memwriterSize(memwriter) + 1);
      strcpy(result, memwriterBuf(memwriter));
      memwriterDelete(memwriter);
      memwriter = NULL;
      return result;
    }
  return "";
}

char *grm_dump_html(char *plot_id)
{
  return grm_dump_html_args(plot_id, active_plot_args);
}

char *grm_dump_html_args(char *plot_id, grm_args_t *args)
{
  static Memwriter *memwriter = NULL, *memwriter2 = NULL;
  char *result;

  if (memwriter == NULL)
    {
      memwriter = memwriterNew();
    }
  if (memwriter2 == NULL)
    {
      memwriter2 = memwriterNew();
    }
  toJsonWriteArgs(memwriter, args);
  if (!toJsonIsComplete())
    {
      memwriterDelete(memwriter);
      memwriter = NULL;
      memwriterDelete(memwriter2);
      memwriter2 = NULL;
      return "";
    }
  memwriterPutc(memwriter, '\0');

  memwriterPrintf(memwriter2, "<div id=\"jsterm-display-%s\"></div>\n", plot_id);
  memwriterPuts(memwriter2, "<script type=\"text/javascript\">\n"
                            "if (typeof jsterm === \"undefined\") {\n"
                            "  var jsterm = null;\n"
                            "}\n"
                            "function run_on_start(data, display) {\n"
                            "  if (typeof JSTerm === \"undefined\") {\n"
                            "    setTimeout(function() {run_on_start(data, display)}, 100);\n"
                            "    return;\n"
                            "  }\n"
                            "  if (jsterm === null) {\n"
                            "    jsterm = new JSTerm(true);\n"
                            "  }\n"
                            "  jsterm.draw({\n"
                            "    \"json\": data,\n"
                            "    \"display\": display\n"
                            "  })\n"
                            "}\n"
                            "run_on_start(");
  toJsonStringifyStringValue(memwriter2, memwriterBuf(memwriter));
  if (toJsonIsComplete())
    {
      memwriterDelete(memwriter);
      memwriter = NULL;

      memwriterPrintf(memwriter2, ", '%s');\n</script>", plot_id);
      memwriterPutc(memwriter2, '\0');
      size_t slen = memwriterSize(memwriter2);
      result = malloc(slen + 1);
      memcpy(result, memwriterBuf(memwriter2), slen);
      result[slen] = '\0';

      memwriterDelete(memwriter2);
      memwriter2 = NULL;

      return result;
    }
  memwriterDelete(memwriter);
  memwriter = NULL;
  memwriterDelete(memwriter2);
  memwriter2 = NULL;
  return "";
}

void grm_dump_bson(const grm_args_t *args, FILE *f)
{
  static Memwriter *memwriter = NULL;
  char *buf;
  int length;
  int i;

  if (memwriter == NULL) memwriter = memwriterNew();
  toBsonWriteArgs(memwriter, args);
  if (toBsonIsComplete())
    {
      buf = memwriterBuf(memwriter);
      bytesToInt(&length, buf);

      for (i = 0; i < length; i++, buf++)
        {
          fprintf(f, "%.2X", (unsigned char)*(buf));
          if (i % 16 == 15)
            {
              putc('\n', f);
            }
          else if (i % 2 == 1)
            {
              putc(' ', f);
            }
        }
      fprintf(f, "\n");

      memwriterDelete(memwriter);
      memwriter = NULL;
    }
}
