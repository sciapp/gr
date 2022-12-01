#include "grplot_mainwindow.hxx"
#include <QApplication>

bool string_starts_with(const std::string &str, const std::string &prefix)
{
  return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

int main(int argc, char **argv)
{
  int i;
  std::string token, csv_file, plot_type = "line", colms;

  for (i = 1; i < argc; i++)
    {
      token = argv[i];
      if (string_starts_with(token, "file:"))
        {
          csv_file = token.substr(5, token.length() - 1);
        }
      else if (string_starts_with(token, "type:"))
        {
          plot_type = token.substr(5, token.length() - 1);
        }
      else if (string_starts_with(token, "columns:"))
        {
          colms = token.substr(8, token.length() - 1);
        }
    }
  if (csv_file.empty())
    {
      fprintf(stderr, "Missing input file name\n");
      exit(0);
    }

  QApplication app(argc, argv);
  MainWindow window(csv_file.c_str(), plot_type.c_str(), colms.c_str());

  window.show();

  return app.exec();
}
