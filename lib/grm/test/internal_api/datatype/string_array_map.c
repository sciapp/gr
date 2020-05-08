#include "test.h"

#include "datatype/string_array_map_int.h"


static const string_map_entry_t test_data[] = {{"test", "abc|def|ghi"}};


void test(void)
{
  string_array_map_t *string_array_map;
  char **string_array;

  string_array_map = string_array_map_new_from_string_split(1, test_data, '|');
  assert(string_array_map_at(string_array_map, "test", &string_array));
  assert(strcmp(string_array[0], "abc") == 0);
  assert(strcmp(string_array[1], "def") == 0);
  assert(strcmp(string_array[2], "ghi") == 0);
  string_array_map_delete(string_array_map);
}

DEFINE_TEST_MAIN
