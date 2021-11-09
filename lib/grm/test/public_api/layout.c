#include "grm.h"

void test_layout(void)
{
  layout_t *layout = layout_new();

  layout_print_test(layout);

  layout_delete(layout);
}

int main(void)
{
  test_layout();

  return 0;
}
