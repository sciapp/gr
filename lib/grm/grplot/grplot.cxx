#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stringapiset.h>
#endif
#include <iostream>
#include <QApplication>
#include "grplot_mainwindow.hxx"
#include "util.hxx"

const unsigned int MAXPATHLEN = 1024;

int main(int argc, char **argv)
{
  // Ensure that the `GRDIR` envionment variable is set, so GR can find its components like fonts.
#ifndef NO_EXCEPTIONS
  try
    {
#endif
      util::setGrdir();
#ifndef NO_EXCEPTIONS
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
      std::cerr << "Failed to set the \"GRDIR\" envionment variable, falling back to GRDIR=\"" << GRDIR << "\"."
                << std::endl;
    }
#endif

  /* help page should be shown */
  if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
    {
      static char path[MAXPATHLEN];
      std::snprintf(path, MAXPATHLEN, "%s/bin/grplot.man.md", GRDIR);
      if (!util::file_exists(path))
        {
          fprintf(stderr, "Helpfile not found\n");
          return 1;
        }
      return util::grplot_overview(argc, argv);
    }

  if (getenv("GKS_WSTYPE") != nullptr)
    {
      return (grm_plot_from_file(argc, argv) != 1);
    }
  else
    {
      QApplication app(argc, argv);
      GRPlotMainWindow window(argc, argv);

      window.show();

      return app.exec();
    }
}
