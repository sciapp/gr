#if !(defined(__MINGW32__) && !defined(__MINGW64__))
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stringapiset.h>
#endif
#include <iostream>
#include <sstream>
#include <QApplication>
#include "grplotMainwindow.hxx"
#include "util.hxx"

const unsigned int WIDTH = 600;
const unsigned int HEIGHT = 450;

static QString test_commands_file_path = "";

int main(int argc, char **argv)
{
  bool pass = false, listen_mode = false, test_mode = false, help_mode = false;
  int width = WIDTH, height = HEIGHT;

  // Ensure that the `GRDIR` environment variable is set, so GR can find its components like fonts.
  try
    {
      util::setGrdir();
    }
  // Catch an exception, print an error message but ignore it. If GR is located in its install location,
  // no environment variables need to be set at all.
  catch (std::exception &e)
    {
#ifdef _WIN32
      int needed_wide_chars = MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, nullptr, 0);
      std::vector<wchar_t> what_wide(needed_wide_chars);
      MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, what_wide.data(), needed_wide_chars);
      std::wcerr << what_wide.data() << std::endl;
#else
      std::cerr << e.what() << std::endl;
#endif
      std::cerr << "Failed to set the \"GRDIR\" environment variable, falling back to GRDIR=\"" << GRDIR << "\"."
                << std::endl;
    }

  if (argc > 1)
    {
      for (int i = 1; i < argc; i++) // parse the -- attributes
        {
          if (strcmp(argv[i], "--size") == 0)
            {
              if (argc > i + 2)
                {
                  std::vector<std::string> strings;
                  std::string size;
                  auto s = argv[i + 1];
                  std::istringstream size_stream(s);
                  while (getline(size_stream, size, ','))
                    {
                      strings.push_back(size);
                    }
                  if (!strings.empty() && strings.size() == 2)
                    {
                      width = stoi(strings[0]);
                      height = stoi(strings[1]);
                      i += 1;
                    }
                  else
                    {
                      fprintf(stderr, "Given size is invalid, use default size (%d,%d).\n", WIDTH, HEIGHT);
                    }
                }
              else
                {
                  fprintf(stderr, "No size given after \"--size\" parameter, use default size (%d, %d).\n", WIDTH,
                          HEIGHT);
                }
            }
          else if (strcmp(argv[i], "--listen") == 0)
            {
              listen_mode = true;
            }
          else if (strcmp(argv[i], "--test") == 0)
            {
              if (argc > i + 2 && !util::startsWith(argv[i + 1], "--"))
                {
                  test_commands_file_path = argv[i + 1];
                  test_mode = true;
                  i += 1;
                }
              else
                {
                  fprintf(stderr, "No test commands file given after \"--test\" parameter.\n");
                }
            }
          else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) /* help page should be shown */
            {
#ifdef _WIN32
              std::wstringstream path_stream;
              path_stream << util::getEnvVar(L"GRDIR", L"" GRDIR)
#else
              std::stringstream path_stream;
              path_stream << util::getEnvVar("GRDIR", GRDIR)
#endif
                          << "/share/doc/grplot/grplot.man.md";

              if (!util::fileExists(path_stream.str()))
                {
                  fprintf(stderr, "Helpfile not found\n");
                  return 1;
                }
              pass = true;
              help_mode = true;
              i += 1;
            }
          else
            {
              argv += (i - 1);
              argc -= (i - 1);
              break;
            }
          if (argc <= 2 && !listen_mode && !help_mode)
            {
              fprintf(stderr, "Not enough command line arguments: specify an input file or the \"--listen\" option.\n");
              return 1;
            }
        }
    }
  else
    {
      fprintf(stderr, "Usage: grplot <FILE> [<KEY:VALUE>] ...\n  -h, --help\n");
      return 0;
    }

  if (!pass && getenv("GKS_WSTYPE") != nullptr) return (grm_plot_from_file(argc, argv) != 1);

  QApplication app(argc, argv);
  GRPlotMainWindow window(argc, argv, width, height, listen_mode, test_mode, test_commands_file_path, help_mode);

  if (!listen_mode) window.show();
  return app.exec();
}
#else
#include <iostream>

int main(int argc, char **argv)
{
  std::cerr << "grplot is not supported on MinGW 32-bit." << std::endl;
  return 1;
}
#endif
