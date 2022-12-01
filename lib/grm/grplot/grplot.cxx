#include "grplot_mainwindow.hxx"
#include <QApplication>

int main(int argc, char **argv)
{
  const char *colms = "", *csv_file = "", *plot_type = "line";

  if (argc > 1)
    {
      csv_file = argv[1];
    }
  else
    {
      fprintf(stderr, "Please specify a file to run grplot.\n");
      exit(0);
    }
  if (argc > 2)
    {
      plot_type = argv[2];
    }
  if (argc > 3)
    {
      colms = argv[3];
    }

  QApplication app(argc, argv);
  MainWindow window(csv_file, plot_type, colms);

  window.show();

  return app.exec();
}
