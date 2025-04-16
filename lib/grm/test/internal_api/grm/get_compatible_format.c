#ifdef __unix__
#define _POSIX_C_SOURCE 1
#endif

#include "test.h"

#include <grm/plot_int.h>


void test(void)
{
  plotInitStaticVariables();

  assert(strcmp(getCompatibleFormat("bar_color", "i"), "i") == 0);
  assert(strcmp(getCompatibleFormat("bar_color", "nD"), "D") == 0);
  assert(strcmp(getCompatibleFormat("bar_color", "D"), "D") == 0);
  assert(strcmp(getCompatibleFormat("bar_color", "nI"), "i") == 0);
  assert(strcmp(getCompatibleFormat("bar_color", "I"), "i") == 0);
  assert(strcmp(getCompatibleFormat("bar_color", "dd"), "D") == 0);
  assert(strcmp(getCompatibleFormat("bar_color", "ii"), "i") == 0);

  assert(getCompatibleFormat("bar_color", "s") == NULL);
  assert(getCompatibleFormat("bar_color", "nS") == NULL);
  assert(getCompatibleFormat("bar_color", "S") == NULL);
  assert(getCompatibleFormat("bar_color", "DD") == NULL);
  assert(getCompatibleFormat("bar_color", "iD") == NULL);
}

DEFINE_TEST_MAIN
