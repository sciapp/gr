#ifdef __unix__
#define _POSIX_C_SOURCE 1
#endif

#include <string.h>

#include "test.h"

#include <grm/args_int.h>


void test(void)
{
  const int int_value = 42;
  const char *str_values[] = {"Hello", "World!"};
  grm_args_t *args;
  grm_args_iterator_t *it;
  grm_arg_t *arg;
  int *int_ptr;
  const char **str_ptr;
  char *str1, *str2;
  unsigned int int_array_length, str_array_length;

  args = grm_args_new();
  grm_args_push(args, "int_value", "i", int_value);
  grm_args_push(args, "str_value", "s", str_values[0]);
  grm_args_push(args, "str_array", "nS", arraySize(str_values), str_values);

  it = grm_args_iter(args);
  assert((arg = it->next(it)) != NULL);
  assert(strcmp(arg->value_format, "i") == 0);
  assert((arg = it->next(it)) != NULL);
  assert(strcmp(arg->value_format, "s") == 0);
  assert((arg = it->next(it)) != NULL);
  assert(strcmp(arg->value_format, "nS") == 0);
  argsIteratorDelete(it);

  assert(grm_args_first_value(args, "int_value", "I", &int_ptr, &int_array_length));
  assert(int_array_length == 1);
  assert(*int_ptr == int_value);

  assert(grm_args_first_value(args, "str_value", "S", &str_ptr, &str_array_length));
  assert(str_array_length == 1);
  assert(strcmp(*str_ptr, str_values[0]) == 0);
  assert(str_ptr[1] == NULL); /* arrays of pointer types must be terminated by a NULL value */

  assert(grm_args_first_value(args, "str_array", "S", &str_ptr, &str_array_length));
  assert(str_array_length == 2);
  assert(strcmp(str_ptr[0], str_values[0]) == 0);
  assert(strcmp(str_ptr[1], str_values[1]) == 0);
  assert(str_ptr[2] == NULL); /* arrays of pointer types must be terminated by a NULL value */

  assert(grm_args_values(args, "int_value", "I", &int_ptr));
  assert(*int_ptr == int_value);

  assert(grm_args_values(args, "str_value", "S", &str_ptr));
  assert(strcmp(*str_ptr, str_values[0]) == 0);
  assert(str_ptr[1] == NULL); /* arrays of pointer types must be terminated by a NULL value */

  assert(grm_args_values(args, "str_array", "S", &str_ptr));
  assert(strcmp(str_ptr[0], str_values[0]) == 0);
  assert(strcmp(str_ptr[1], str_values[1]) == 0);
  assert(str_ptr[2] == NULL); /* arrays of pointer types must be terminated by a NULL value */

  assert(grm_args_values(args, "str_array", "ss", &str1, &str2));
  assert(strcmp(str1, str_values[0]) == 0);
  assert(strcmp(str2, str_values[1]) == 0);

  grm_args_delete(args);
}

DEFINE_TEST_MAIN
