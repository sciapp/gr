#if defined __unix__ || defined __APPLE__
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#endif
#include <array>
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#if defined __APPLE__
#include <libgen.h>
#include <mach-o/dyld.h>
#elif defined __unix__
#define _XOPEN_SOURCE 600
#include <libgen.h>
#include <unistd.h>
#elif defined _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <stringapiset.h>
#include <strsafe.h>
#else
#error "Unsupported system"
#endif

#include "util.hxx"

namespace util
{
const unsigned int MAXPATHLEN = 1024;


#if defined __unix__ || defined __APPLE__
ErrnoError::ErrnoError()
{
  std::stringstream what_stream;

  what_stream << "Error " << errno << ": " << strerror(errno);
  what_str_ = what_stream.str();
}

const char *ErrnoError::what() const noexcept
{
  return what_str_.c_str();
}
#elif defined _WIN32
const char *FormatMessageError::what() const noexcept
{
  return "No message could be created, FormatMessage failed.";
}

GetLastErrorError::GetLastErrorError()
{
  unsigned int error_code = GetLastError();
  std::stringstream what_stream;
  LPTSTR error_message = nullptr;

  if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                     nullptr, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&error_message, 0,
                     nullptr))
    {
      throw(FormatMessageError());
    }
  what_stream << "Error " << error_code << ": " << error_message;
  LocalFree(error_message);
  what_str_ = what_stream.str();
}

const char *GetLastErrorError::what() const noexcept
{
  return what_str_.c_str();
}
#endif

#if defined __APPLE__
PathTooLongError::PathTooLongError(size_t neededSize)
{
  std::stringstream what_stream;

  what_stream << MAXPATHLEN << " Bytes are not sufficient for storing the path. " << neededSize << " Bytes are needed.";
  what_str_ = what_stream.str();
}

const char *PathTooLongError::what() const noexcept
{
  return what_str_.c_str();
}

#elif defined __unix__
PathTooLongError::PathTooLongError()
{
  std::stringstream what_stream;

  what_stream << MAXPATHLEN << " Bytes are not sufficient for storing the path.";
  what_str_ = what_stream.str();
}

const char *PathTooLongError::what() const noexcept
{
  return what_str_.c_str();
}

const char *ProcessFileLinkNotReadableError::what() const noexcept
{
  return ErrnoError::what();
}
#endif

#if defined __unix__ || defined __APPLE__
const char *AbsolutePathError::what() const noexcept
{
  return ErrnoError::what();
}

CorruptedGrDirError::CorruptedGrDirError(const std::string &path)
{
  std::stringstream what_stream;

  what_stream << "The directory \"" << path << "\" is not a valid GR directory (missing \"include/gr.h\")";
  what_str_ = what_stream.str();
}

const char *CorruptedGrDirError::what() const noexcept
{
  return what_str_.c_str();
}

const char *SetEnvError::what() const noexcept
{
  return ErrnoError::what();
}
#elif defined _WIN32
const char *WideCharToMultiByteError::what() const noexcept
{
  return GetLastErrorError::what();
}

const char *ModulePathError::what() const noexcept
{
  return GetLastErrorError::what();
}

DirnameError::DirnameError(const std::wstring &file_path)
{
  std::wstringstream what_stream;

  what_stream << "Could not extract the directory name of \"" << file_path << "\".";

  int needed_bytes_utf_8_string =
      WideCharToMultiByte(CP_UTF8, 0, what_stream.str().c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (needed_bytes_utf_8_string == 0) throw(WideCharToMultiByteError());
  std::vector<char> utf_8_bytes(needed_bytes_utf_8_string);
  int written_bytes_utf_8_string = WideCharToMultiByte(CP_UTF8, 0, what_stream.str().c_str(), -1, utf_8_bytes.data(),
                                                       needed_bytes_utf_8_string, nullptr, nullptr);
  if (written_bytes_utf_8_string == 0) throw(WideCharToMultiByteError());
  what_str_ = utf_8_bytes.data();
}

const char *DirnameError::what() const noexcept
{
  return what_str_.c_str();
}

AbsolutePathError::AbsolutePathError(const std::wstring &path)
{
  std::wstringstream what_stream;

  what_stream << "Could not determine the absolute path of \"" << path << "\".";

  int needed_bytes_utf_8_string =
      WideCharToMultiByte(CP_UTF8, 0, what_stream.str().c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (needed_bytes_utf_8_string == 0) throw(WideCharToMultiByteError());
  std::vector<char> utf_8_bytes(needed_bytes_utf_8_string);
  int written_bytes_utf_8_string = WideCharToMultiByte(CP_UTF8, 0, what_stream.str().c_str(), -1, utf_8_bytes.data(),
                                                       needed_bytes_utf_8_string, nullptr, nullptr);
  if (written_bytes_utf_8_string == 0) throw(WideCharToMultiByteError());
  what_str_ = utf_8_bytes.data();
}

const char *AbsolutePathError::what() const noexcept
{
  return what_str_.c_str();
}

CorruptedGrDirError::CorruptedGrDirError(const std::wstring &path)
{
  std::wstringstream what_stream;

  what_stream << "The directory \"" << path << "\" is not a valid GR directory (missing \"include\\gr.h\")";

  int needed_bytes_utf_8_string =
      WideCharToMultiByte(CP_UTF8, 0, what_stream.str().c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (needed_bytes_utf_8_string == 0) throw(WideCharToMultiByteError());
  std::vector<char> utf_8_bytes(needed_bytes_utf_8_string);
  int written_bytes_utf_8_string = WideCharToMultiByte(CP_UTF8, 0, what_stream.str().c_str(), -1, utf_8_bytes.data(),
                                                       needed_bytes_utf_8_string, nullptr, nullptr);
  if (written_bytes_utf_8_string == 0) throw(WideCharToMultiByteError());
  what_str_ = utf_8_bytes.data();
}

const char *CorruptedGrDirError::what() const noexcept
{
  return what_str_.c_str();
}

const char *SetEnvError::what() const noexcept
{
  return GetLastErrorError::what();
}
#else
#error "Unsupported system"
#endif

bool endsWith(const std::string &str, const std::string &suffix)
{
  return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

bool startsWith(const std::string &str, const std::string &prefix)
{
  return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

bool isDigits(const std::string &str)
{
  return str.find_first_not_of("0123456789") == std::string::npos;
}

bool isNumber(const std::string &str)
{
  const char minus[] = {(char)0xe2, (char)0x88, (char)0x92, '\0'}; // gr minus sign
  auto em_dash = std::string(minus);
  size_t start_pos = 0;
  if (startsWith(str, em_dash)) start_pos = em_dash.size();
  auto pos = str.find_first_not_of(".-0123456789", start_pos);
  return pos == std::string::npos;
}

#ifdef _WIN32
std::wstring getEnvVar(const std::wstring &name, const std::wstring &default_value)
#else
std::string getEnvVar(const std::string &name, const std::string &default_value)
#endif
{
#ifdef _WIN32
  DWORD needed_wide_chars = GetEnvironmentVariableW(name.c_str(), nullptr, 0);
  if (GetLastError() != ERROR_ENVVAR_NOT_FOUND)
    {
      std::vector<wchar_t> value_wide(needed_wide_chars);
      GetEnvironmentVariableW(name.c_str(), value_wide.data(), needed_wide_chars);
      return std::wstring(value_wide.data());
    }
  else
    {
      return default_value;
    }
#else
  const char *value_c_ptr = getenv(name.c_str());
  if (value_c_ptr != nullptr) return std::string(value_c_ptr);
  return default_value;
#endif
}

#ifdef _WIN32
std::wstring getExecutablePath()
#else
std::string getExecutablePath()
#endif
{
#ifdef _WIN32
  std::array<wchar_t, MAXPATHLEN> exe_path{L""};
#else
  std::array<char, MAXPATHLEN> exe_path{""};
#endif

#if defined __APPLE__
  unsigned int path_len = MAXPATHLEN - 1;

  if (_NSGetExecutablePath(exe_path.data(), &path_len)) throw(PathTooLongError(path_len));
#elif defined __FreeBSD__ || defined __NetBSD__ || defined __OpenBSD__
  ssize_t path_len = -1;

  path_len = readlink("/proc/curproc/file", exe_path.data(), MAXPATHLEN);
  if (path_len < 0)
    {
      throw(ProcessFileLinkNotReadableError());
    }
  else if (path_len == MAXPATHLEN)
    {
      throw(PathTooLongError());
    }
#elif defined __linux__
  ssize_t path_len = -1;

  path_len = readlink("/proc/self/exe", exe_path.data(), MAXPATHLEN);
  if (path_len < 0) throw(ProcessFileLinkNotReadableError());
  if (path_len == MAXPATHLEN) throw(PathTooLongError());
#elif defined _WIN32
  unsigned int path_len = 0;

  path_len = GetModuleFileNameW(nullptr, exe_path.data(), MAXPATHLEN);
  if (GetLastError() != ERROR_SUCCESS) throw(ModulePathError());
#else
#error "Unsupported system"
#endif
  exe_path.at(path_len) = '\0';

  return exe_path.data();
}

#ifdef _WIN32
bool fileExists(const std::string &file_path)
{
  int needed_wide_chars = MultiByteToWideChar(CP_UTF8, 0, file_path.c_str(), -1, nullptr, 0);
  std::vector<wchar_t> file_path_wide(needed_wide_chars);
  MultiByteToWideChar(CP_UTF8, 0, file_path.c_str(), -1, file_path_wide.data(), needed_wide_chars);
  return fileExists(file_path_wide.data());
}

bool fileExists(const std::wstring &file_path)
{
  DWORD file_attributes = GetFileAttributesW(file_path.c_str());
  return (file_attributes != INVALID_FILE_ATTRIBUTES && !(file_attributes & FILE_ATTRIBUTE_DIRECTORY));
}
#else
bool fileExists(const std::string &file_path)
{
  struct stat file_stat;
  return stat(file_path.c_str(), &file_stat) == 0 && S_ISREG(file_stat.st_mode);
}
#endif

void setGrdir(bool force)
{
  if (!force)
    {
#ifdef _WIN32
      GetEnvironmentVariableW(L"GRDIR", nullptr, 0);
      if (GetLastError() != ERROR_ENVVAR_NOT_FOUND) return;
#else
      if (getenv("GRDIR") != nullptr) return;
#endif
    }

  auto exe_path = getExecutablePath();
#ifdef _WIN32
  std::array<wchar_t, MAXPATHLEN> exe_dirname;
  std::wstringstream gr_dir_relative_stream;
  std::array<wchar_t, MAXPATHLEN> gr_dir_absolute;
  std::wstringstream gr_header_path_stream;

  if (_wsplitpath_s(exe_path.c_str(), nullptr, 0, exe_dirname.data(), MAXPATHLEN, nullptr, 0, nullptr, 0))
    {
      throw(DirnameError(exe_path));
    }
  gr_dir_relative_stream << exe_dirname.data() << L"/..";
  if (_wfullpath(gr_dir_absolute.data(), gr_dir_relative_stream.str().c_str(), MAXPATHLEN) == nullptr)
    {
      throw(AbsolutePathError(gr_dir_relative_stream.str()));
    }
  gr_header_path_stream << gr_dir_absolute.data() << L"/include/gr.h";
  if (!fileExists(gr_header_path_stream.str())) throw(CorruptedGrDirError(gr_dir_absolute.data()));
  if (!SetEnvironmentVariableW(L"GRDIR", gr_dir_absolute.data())) throw(SetEnvError());
#else
  std::stringstream gr_dir_relative_stream, gr_header_path_stream;

  gr_dir_relative_stream
      << dirname(&std::string(exe_path)[0])
#ifdef __APPLE__
      // On macOS, the `grplot` executable is located in `${GRDIR}/Applications/grplot.app/Contents/MacOS`
      << "/../../../..";
#else
      // On other Unixes, the `grplot` executable is located in `${GRDIR}/bin`
      << "/..";
#endif
  std::unique_ptr<char, decltype(std::free) *> gr_dir_absolute_c_ptr{
      realpath(gr_dir_relative_stream.str().c_str(), nullptr), std::free};
  if (!gr_dir_absolute_c_ptr) throw(AbsolutePathError());
  gr_header_path_stream << gr_dir_absolute_c_ptr.get() << "/include/gr.h";
  if (!fileExists(gr_header_path_stream.str())) throw(CorruptedGrDirError(gr_dir_absolute_c_ptr.get()));
  if (setenv("GRDIR", gr_dir_absolute_c_ptr.get(), 1) != 0) throw(SetEnvError());
#endif
}
} // namespace util
