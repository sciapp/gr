#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stringapiset.h>
#endif
#include <iostream>
#include <QApplication>
#include "grplot_mainwindow.hxx"
#include "util.hxx"

int main(int argc, char **argv)
{
  // Ensure that the `GRDIR` envionment variable is set, so GR can find its components like fonts.
  try
    {
      util::setGrdir();
    }
  // Catch an exception, print an error message but ignore it. If GR is located in its install location,
  // no environment variablaes need to be set at all.
  catch (std::exception &e)
    {
      std::cerr << "Failed to set the \"GRDIR\" envionment variable." << std::endl;
#ifdef _WIN32
      int needed_wide_chars = MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, nullptr, 0);
      std::vector<wchar_t> what_wide(needed_wide_chars);
      MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, what_wide.data(), needed_wide_chars);
      std::wcerr << what_wide.data() << std::endl;
#else
      std::cerr << e.what() << std::endl;
#endif
    }

  QApplication app(argc, argv);
  GRPlotMainWindow window(argc, argv);

  window.show();

  return app.exec();
}
