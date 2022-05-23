#include <stdio.h>

#include "grm.h"

#define array_size(x) (sizeof(x) / sizeof((x)[0]))

static void test_pie(void)
{
  const double x[] = {188.6, 107.8, 100.3, 99.0};
  const double c[] = {93 / 255.0,  57 / 255.0,  101 / 255.0, 175 / 255.0, 130 / 255.0, 185 / 255.0,
                      207 / 255.0, 180 / 255.0, 213 / 255.0, 223 / 255.0, 205 / 255.0, 227 / 255.0};
  const char *labels[] = {"Czech Republic", "Austria", "Romania", "Germany"};
  grm_args_t *args;

  printf("plot a pie chart with x\n");
  args = grm_args_new();
  grm_args_push(args, "x", "nD", array_size(x), x);
  grm_args_push(args, "kind", "s", "pie");
  grm_args_push(args, "labels", "nS", array_size(labels), labels);
  grm_args_push(args, "title", "s", "Beer consumption per capita in 2018 (litres per year)");
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  printf("plot a pie chart with x and custom colors\n");
  grm_args_push(args, "c", "nD", array_size(c), c);

  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}


int main(void)
{
  test_pie();
  grm_finalize();

  return 0;
}
