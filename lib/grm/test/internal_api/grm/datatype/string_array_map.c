#ifdef __unix__
#define _POSIX_C_SOURCE 1
#endif

#include "test.h"

#include <grm/datatype/string_array_map_int.h>


static const StringMapEntry test_data[] = {{"test", "abc|def|ghi"}};


void test(void)
{
  StringArrayMap *string_array_map;
  char **string_array;

  string_array_map = stringArrayMapNewFromStringSplit(1, test_data, '|');
  assert(stringArrayMapAt(string_array_map, "test", &string_array));
  assert(strcmp(string_array[0], "abc") == 0);
  assert(strcmp(string_array[1], "def") == 0);
  assert(strcmp(string_array[2], "ghi") == 0);
  stringArrayMapDelete(string_array_map);
}

DEFINE_TEST_MAIN
