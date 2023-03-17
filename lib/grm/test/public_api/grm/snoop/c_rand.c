#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "c_rand.h"

#define RAND_SEED 1234

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int rand_initialized = 0;

static void init_rand(void)
{
  srand(RAND_SEED);
  rand_initialized = 1;
}

double *rnd(double *buf, unsigned int num_elements)
{
  unsigned int i;

  if (!rand_initialized)
    {
      init_rand();
    }
  if (buf == NULL)
    {
      buf = malloc(num_elements * sizeof(double));
      if (buf == NULL)
        {
          return NULL;
        }
    }

  for (i = 0; i < num_elements; ++i)
    {
      buf[i] = (double)(rand)() / ((unsigned int)RAND_MAX + 1);
    }

  return buf;
}

double *randn(double *buf, unsigned int num_elements)
{
  unsigned int i;

  if (!rand_initialized)
    {
      init_rand();
    }
  if (buf == NULL)
    {
      buf = malloc(num_elements * sizeof(double));
      if (buf == NULL)
        {
          return NULL;
        }
    }

  /* Use the Box-Muller transform to generate normally distributed random numbers from uniformly distributed numbers
   * See <https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform#Implementation> as a reference
   */
  for (i = 0; i < num_elements; i += 2)
    {
      double u1, u2, mag;

      /* u1 should be larger than the machine epsilon to avoid numerical problems with the later `log` transform */
      do
        {
          u1 = (double)(rand)() / ((unsigned int)RAND_MAX + 1);
        }
      while (u1 <= DBL_EPSILON);
      u2 = (double)(rand)() / ((unsigned int)RAND_MAX + 1);

      mag = sqrt(-2.0 * log(u1));
      buf[i] = mag * cos(2 * M_PI * u2);
      if (i < num_elements - 1)
        {
          buf[i + 1] = mag * sin(2 * M_PI * u2);
        }
    }

  return buf;
}
