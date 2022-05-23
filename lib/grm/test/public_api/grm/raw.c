#include "gr.h"
#include "grm.h"


static const char *graphics_filename = "gr.xml.base64";


static void create_example_graphics_export(void)
{
  double x[] = {0.0, 1.0};

  gr_begingraphics((char *)graphics_filename);
  gr_polyline(sizeof(x) / sizeof(x[0]), x, x);
  gr_endgraphics();
}


static void test_raw(void)
{
  FILE *graphics_file;
  long graphics_file_size;
  char *graphics_data;
  grm_args_t *args;

  graphics_file = fopen(graphics_filename, "r");
  if (graphics_file == NULL)
    {
      fprintf(stderr, "Could not open \"%s\"\n", graphics_filename);
      return;
    }
  fseek(graphics_file, 0, SEEK_END);
  graphics_file_size = ftell(graphics_file);
  rewind(graphics_file);
  graphics_data = malloc(graphics_file_size + 1);
  graphics_file_size = fread(graphics_data, 1, graphics_file_size, graphics_file);
  graphics_data[graphics_file_size] = '\0';
  fclose(graphics_file);

  args = grm_args_new();
  grm_args_push(args, "raw", "s", graphics_data);
  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
  free(graphics_data);
  grm_finalize();
}


int main(void)
{
  test_raw();

  return 0;
}
