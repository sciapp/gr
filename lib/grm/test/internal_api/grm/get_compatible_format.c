#ifdef __unix__
#define _POSIX_C_SOURCE 1
#endif

#include "test.h"

#include <grm/plot_int.h>


void test(void)
{
  plot_init_static_variables();

  assert(strcmp(get_compatible_format("bar_color", "i"), "i") == 0);
  assert(strcmp(get_compatible_format("bar_color", "nD"), "D") == 0);
  assert(strcmp(get_compatible_format("bar_color", "D"), "D") == 0);
  assert(strcmp(get_compatible_format("bar_color", "nI"), "i") == 0);
  assert(strcmp(get_compatible_format("bar_color", "I"), "i") == 0);
  assert(strcmp(get_compatible_format("bar_color", "dd"), "D") == 0);
  assert(strcmp(get_compatible_format("bar_color", "ii"), "i") == 0);

  assert(get_compatible_format("bar_color", "s") == NULL);
  assert(get_compatible_format("bar_color", "nS") == NULL);
  assert(get_compatible_format("bar_color", "S") == NULL);
  assert(get_compatible_format("bar_color", "DD") == NULL);
  assert(get_compatible_format("bar_color", "iD") == NULL);
}

DEFINE_TEST_MAIN
