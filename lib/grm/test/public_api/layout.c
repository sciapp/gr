#include "grm.h"
#include "gr.h"

int STD_COLORS[] = {989, 982, 980, 981, 996, 983, 995, 988, 986, 990, 991, 984, 992, 993, 994, 987, 985, 997, 998, 999};

void visualize(element_t **elements, int nelems)
{
  int i;
  double *subplot;

  gr_setwindow(0, 1, 0, 1);
  gr_setviewport(0, 1, 0, 1);
  gr_setfillintstyle(1);

  for (i = 0; i < nelems; i++)
    {
      subplot = element_getSubplot(elements[i]);
      gr_setfillcolorind(STD_COLORS[i]);
      gr_fillrect(subplot[0], subplot[1], subplot[2], subplot[3]);
      //        printf("[%f %f %f %f]\n", subplot[0], subplot[1], subplot[2], subplot[3]);
    }
  getchar();
  gr_clearws();
}

void test_dynamic_grid(void)
{
  int i, nelems = 6;

  grid_t *grid1 = grid_new(1, 1);

  element_t *elements[nelems];

  for (i = 0; i < nelems; i++)
    {
      elements[i] = element_new();
    }

  grid_setElement(0, 1, 0, 1, elements[0], grid1);
  grid_setElement(0, 1, 1, 2, elements[1], grid1);

  grid_finalize(grid1);

  visualize(elements, nelems);

  element_setAbsWidth(elements[2], 0.25);
  element_setAbsHeight(elements[2], 0.2);
  element_setFitParentsHeight(elements[2], 1);
  grid_setElement(0, 1, 2, 3, elements[2], grid1);

  grid_finalize(grid1);

  visualize(elements, nelems);

  element_setFitParentsWidth(elements[2], 0);
  grid_setElement(1, 2, 0, 2, elements[2], grid1);

  grid_finalize(grid1);

  visualize(elements, nelems);

  trim(grid1);

  grid_finalize(grid1);

  visualize(elements, nelems);

  grid_setElement(0, 1, 2, 3, elements[3], grid1);
  grid_setElement(1, 2, 2, 3, elements[4], grid1);

  grid_finalize(grid1);

  visualize(elements, nelems);

  grid_t *grid2 = grid_new(2, 1);
  grid_setElement(0, 1, 0, 1, elements[3], grid2);
  grid_setElement(1, 2, 0, 1, elements[4], grid2);
  grid_setElement(0, 2, 2, 3, grid2, grid1);

  grid_finalize(grid1);

  visualize(elements, nelems);

  grid_delete(grid1);
}

int main(void)
{
  test_dynamic_grid();

  return 0;
}
