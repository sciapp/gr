#if !(defined(__MINGW32__) && !defined(__MINGW64__))
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stringapiset.h>
#endif
#include <iostream>
#include <sstream>
#include <QApplication>
#include "grplot_mainwindow.hxx"
#include "util.hxx"

int main(int argc, char **argv)
{
  int pass = 0;
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
      /* help page should be shown */
      if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
        {
#ifdef _WIN32
          std::wstringstream pathStream;
          pathStream << util::getEnvVar(L"GRDIR", L"" GRDIR)
#else
          std::stringstream pathStream;
          pathStream << util::getEnvVar("GRDIR", GRDIR)
#endif
                     << "/share/doc/grplot/grplot.man.md";

          if (!util::fileExists(pathStream.str()))
            {
              fprintf(stderr, "Helpfile not found\n");
              return 1;
            }
          pass = 1;
        }
    }
  else
    {
      fprintf(stderr, "Usage: grplot <FILE> [<KEY:VALUE>] ...\n  -h, --help\n");
      return 0;
    }

  if (!pass && getenv("GKS_WSTYPE") != nullptr)
    {
      return (grm_plot_from_file(argc, argv) != 1);
    }
  else
    {
      QApplication app(argc, argv);
      GRPlotMainWindow window(argc, argv);

      if (strcmp(argv[1], "--listen") != 0)
        {
          window.show();
        }

      return app.exec();
    }
}
#else
#include <iostream>

int main(int argc, char **argv)
{
  std::cerr << "grplot is not supported on MinGW 32-bit." << std::endl;
  return 1;
}
#endif
