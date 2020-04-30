#include <assert.h>
#include "datatype/string_array_map_int.h"


static const string_map_entry_t test_data[] = {{"test", "abc|def|ghi"}};


int main(void)
{
  string_array_map_t *string_array_map;
  char **string_array;
  char **current_string_ptr;

  string_array_map = string_array_map_new_from_string_split(1, test_data, '|');
  assert(string_array_map_at(string_array_map, "test", &string_array));
  current_string_ptr = string_array;
  while (*current_string_ptr != NULL)
    {
      fprintf(stderr, "Read string \"%s\"\n", *current_string_ptr);
      ++current_string_ptr;
    }
  string_array_map_delete(string_array_map);

  return 0;
}
