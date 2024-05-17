#ifndef TEST_H_INCLUDED
#define TEST_H_INCLUDED

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#ifndef NDEBUG
#define DEFINE_TEST_MAIN                                \
  int main(void)                                        \
  {                                                     \
    test();                                             \
                                                        \
    if (isatty(fileno(stdout)))                         \
      {                                                 \
        printf("\033[32;1mAll tests passed!\033[0m\n"); \
      }                                                 \
    else                                                \
      {                                                 \
        printf("All tests passed!\n");                  \
      }                                                 \
                                                        \
    return 0;                                           \
  }
#else
#define DEFINE_TEST_MAIN                                \
  int main(void)                                        \
  {                                                     \
    fprintf(stderr, "Please compile in debug mode!\n"); \
                                                        \
    return 1;                                           \
  }
#endif

#endif /* ifndef TEST_H_INCLUDED */
