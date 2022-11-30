#include "grmplots_mainwindow.hxx"
#include <QApplication>

bool string_starts_with(const std::string &str, const std::string &prefix)
{
  return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

int main(int argc, char **argv)
{
  unsigned int i;
  std::string token;
  const char *colms = "", *csv_file = "", *plot_type = "line";

  for (i = 1; i < argc; i++)
    {
      token = argv[i];
      if (string_starts_with(token, "file:"))
        {
          csv_file = token.substr(5, token.length() - 1).c_str();
        }
      else if (string_starts_with(token, "type:"))
        {
          plot_type = token.substr(5, token.length() - 1).c_str();
        }
      else if (string_starts_with(token, "columns:"))
        {
          colms = token.substr(8, token.length() - 1).c_str();
        }
    }
  if (strcmp(csv_file, "") == 0)
    {
      fprintf(stderr, "Please specify a file to run grm-plots.\n");
      exit(0);
    }

  QApplication app(argc, argv);
  MainWindow window(csv_file, plot_type, colms);

  window.show();

  return app.exec();
}
