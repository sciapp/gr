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

#ifdef NO_EXCEPTIONS
#ifdef _WIN32
#define throwWithoutReturn_(e)                                                         \
  do                                                                                   \
    {                                                                                  \
      int neededWideChars = MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, nullptr, 0); \
      std::vector<wchar_t> whatWide(neededWideChars);                                  \
      MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, whatWide.data(), neededWideChars); \
      std::wcerr << whatWide.data() << std::endl;                                      \
    }                                                                                  \
  while (0)
#else
#define throwWithoutReturn_(e)            \
  do                                      \
    {                                     \
      std::cerr << e.what() << std::endl; \
      return {};                          \
    }                                     \
  while (0)
#endif
#define throw_(e)             \
  do                          \
    {                         \
      throwWithoutReturn_(e); \
      return {};              \
    }                         \
  while (0)
#define throwOrTerminate_(e)  \
  do                          \
    {                         \
      throwWithoutReturn_(e); \
      std::terminate();       \
    }                         \
  while (0)
#else
#define throwWithoutReturn_(e) throw(e)
#define throw_(e) throw(e)
#define throwOrTerminate_(e) throw(e)
#endif

namespace util
{
const unsigned int MAXPATHLEN = 1024;


#if defined __unix__ || defined __APPLE__
ErrnoError::ErrnoError()
{
  std::stringstream whatStream;

  whatStream << "Error " << errno << ": " << strerror(errno);
  whatStr_ = whatStream.str();
}

const char *ErrnoError::what() const noexcept
{
  return whatStr_.c_str();
}
#elif defined _WIN32
const char *FormatMessageError::what() const noexcept
{
  return "No message could be created, FormatMessage failed.";
}

GetLastErrorError::GetLastErrorError()
{
  unsigned int errorCode = GetLastError();
  std::stringstream whatStream;
  LPTSTR errorMessage = nullptr;

  if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                     nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errorMessage, 0, nullptr))
    {
      throwOrTerminate_(FormatMessageError());
    }
  whatStream << "Error " << errorCode << ": " << errorMessage;
  LocalFree(errorMessage);
  whatStr_ = whatStream.str();
}

const char *GetLastErrorError::what() const noexcept
{
  return whatStr_.c_str();
}
#endif

#if defined __APPLE__
PathTooLongError::PathTooLongError(size_t neededSize)
{
  std::stringstream whatStream;

  whatStream << MAXPATHLEN << " Bytes are not sufficient for storing the path. " << neededSize << " Bytes are needed.";
  whatStr_ = whatStream.str();
}

const char *PathTooLongError::what() const noexcept
{
  return whatStr_.c_str();
}

#elif defined __unix__
PathTooLongError::PathTooLongError()
{
  std::stringstream whatStream;

  whatStream << MAXPATHLEN << " Bytes are not sufficient for storing the path.";
  whatStr_ = whatStream.str();
}

const char *PathTooLongError::what() const noexcept
{
  return whatStr_.c_str();
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
  std::stringstream whatStream;

  whatStream << "The directory \"" << path << "\" is not a valid GR directory (missing \"include/gr.h\")";
  whatStr_ = whatStream.str();
}

const char *CorruptedGrDirError::what() const noexcept
{
  return whatStr_.c_str();
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

DirnameError::DirnameError(const std::wstring &filepath)
{
  std::wstringstream whatStream;

  whatStream << "Could not extract the directory name of \"" << filepath << "\".";

  int neededBytesUtf8String =
      WideCharToMultiByte(CP_UTF8, 0, whatStream.str().c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (neededBytesUtf8String == 0)
    {
      throwOrTerminate_(WideCharToMultiByteError());
    }
  std::vector<char> utf8Bytes(neededBytesUtf8String);
  int writtenBytesUtf8String = WideCharToMultiByte(CP_UTF8, 0, whatStream.str().c_str(), -1, utf8Bytes.data(),
                                                   neededBytesUtf8String, nullptr, nullptr);
  if (writtenBytesUtf8String == 0)
    {
      throwOrTerminate_(WideCharToMultiByteError());
    }
  whatStr_ = utf8Bytes.data();
}

const char *DirnameError::what() const noexcept
{
  return whatStr_.c_str();
}

AbsolutePathError::AbsolutePathError(const std::wstring &path)
{
  std::wstringstream whatStream;

  whatStream << "Could not determine the absolute path of \"" << path << "\".";

  int neededBytesUtf8String =
      WideCharToMultiByte(CP_UTF8, 0, whatStream.str().c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (neededBytesUtf8String == 0)
    {
      throwOrTerminate_(WideCharToMultiByteError());
    }
  std::vector<char> utf8Bytes(neededBytesUtf8String);
  int writtenBytesUtf8String = WideCharToMultiByte(CP_UTF8, 0, whatStream.str().c_str(), -1, utf8Bytes.data(),
                                                   neededBytesUtf8String, nullptr, nullptr);
  if (writtenBytesUtf8String == 0)
    {
      throwOrTerminate_(WideCharToMultiByteError());
    }
  whatStr_ = utf8Bytes.data();
}

const char *AbsolutePathError::what() const noexcept
{
  return whatStr_.c_str();
}

CorruptedGrDirError::CorruptedGrDirError(const std::wstring &path)
{
  std::wstringstream whatStream;

  whatStream << "The directory \"" << path << "\" is not a valid GR directory (missing \"include\\gr.h\")";

  int neededBytesUtf8String =
      WideCharToMultiByte(CP_UTF8, 0, whatStream.str().c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (neededBytesUtf8String == 0)
    {
      throwOrTerminate_(WideCharToMultiByteError());
    }
  std::vector<char> utf8Bytes(neededBytesUtf8String);
  int writtenBytesUtf8String = WideCharToMultiByte(CP_UTF8, 0, whatStream.str().c_str(), -1, utf8Bytes.data(),
                                                   neededBytesUtf8String, nullptr, nullptr);
  if (writtenBytesUtf8String == 0)
    {
      throwOrTerminate_(WideCharToMultiByteError());
    }
  whatStr_ = utf8Bytes.data();
}

const char *CorruptedGrDirError::what() const noexcept
{
  return whatStr_.c_str();
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

bool file_exists(const std::string &name)
{
  return (access(name.c_str(), F_OK) != -1);
}

int grplot_overview(int argc, char **argv)
{
  std::string line, kind;
  int given_help = 0, removed_start = 0;
  static char function[50];
  static char path[MAXPATHLEN];
  std::snprintf(path, MAXPATHLEN, "%s/bin/grplot.man.md", GRDIR);
  std::ifstream file(path);

  while (getline(file, line))
    {
      if (argc < 3)
        {
          if (strcmp(line.c_str(), "## Advanced information for each plot type") != 0)
            {
              if (strcmp(line.c_str(), "# GR Plot Overview") == 0)
                {
                  fprintf(stderr, "--------------------------------------------------------------------------------\n");
                  fprintf(stderr, "%s\n", line.erase(0, 2).c_str());
                  fprintf(stderr, "--------------------------------------------------------------------------------\n");
                  continue;
                }
              /* remove the # from the Markdown text */
              while (util::startsWith(line, "#"))
                {
                  line.erase(0, 1);
                  removed_start = 1;
                }
              if (removed_start)
                {
                  removed_start = 0;
                  fprintf(stderr, ">");
                }
              if (util::startsWith(line, "```")) continue;
              fprintf(stderr, "%s\n", line.c_str());
            }
          else
            {
              return 0;
            }
        }
      else
        {
          kind = argv[2];
          snprintf(function, 50, "# %s", kind.c_str());
          if (given_help)
            {
              if (util::startsWith(line, "#")) return 0;
              fprintf(stderr, "%s\n", line.c_str());
            }
          if (strcmp(line.c_str(), function) == 0)
            {
              given_help = 1;
              fprintf(stderr, "--------------------------------------------------------------------------------\n");
              fprintf(stderr, "%s\n", kind.c_str());
              fprintf(stderr, "--------------------------------------------------------------------------------\n");
            }
        }
    }
  if (!given_help)
    {
      fprintf(stderr, "No plot type with the name %s was found. Use a correct plot type and try it again.\n",
              kind.c_str());
      return 1;
    }
  return 0;
}

#ifdef NO_EXCEPTIONS
#ifdef _WIN32
std::optional<std::wstring> getExecutablePath()
#else
std::optional<std::string> getExecutablePath()
#endif
#else
#ifdef _WIN32
std::wstring getExecutablePath()
#else
std::string getExecutablePath()
#endif
#endif
{
#ifdef _WIN32
  std::array<wchar_t, MAXPATHLEN> exePath{L""};
#else
  std::array<char, MAXPATHLEN> exePath{""};
#endif

#if defined __APPLE__
  unsigned int pathLen = MAXPATHLEN - 1;

  if (_NSGetExecutablePath(exePath.data(), &pathLen))
    {
      throw_(PathTooLongError(pathLen));
    }
#elif defined __FreeBSD__ || defined __NetBSD__ || defined __OpenBSD__
  ssize_t pathLen = -1;

  pathLen = readlink("/proc/curproc/file", exePath.data(), MAXPATHLEN);
  if (pathLen < 0)
    {
      throw_(ProcessFileLinkNotReadableError());
    }
  else if (pathLen == MAXPATHLEN)
    {
      throw_(PathTooLongError());
    }
#elif defined __linux__
  ssize_t pathLen = -1;

  pathLen = readlink("/proc/self/exe", exePath.data(), MAXPATHLEN);
  if (pathLen < 0)
    {
      throw_(ProcessFileLinkNotReadableError());
    }
  if (pathLen == MAXPATHLEN)
    {
      throw_(PathTooLongError());
    }
#elif defined _WIN32
  unsigned int pathLen = 0;

  pathLen = GetModuleFileNameW(nullptr, exePath.data(), MAXPATHLEN);
  if (GetLastError() != ERROR_SUCCESS)
    {
      throw_(ModulePathError());
    }
#else
#error "Unsupported system"
#endif
  exePath.at(pathLen) = '\0';

  return exePath.data();
}

#ifdef _WIN32
bool fileExists(const std::wstring &filePath)
{
  DWORD fileAttributes = GetFileAttributesW(filePath.c_str());
  return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}
#else
bool fileExists(const std::string &filePath)
{
  struct stat fileStat;
  return stat(filePath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode);
}
#endif

#ifdef NO_EXCEPTIONS
bool
#else
void
#endif
setGrdir(bool force)
{
  if (!force)
    {
#ifdef _WIN32
      GetEnvironmentVariableW(L"GRDIR", nullptr, 0);
      if (GetLastError() != ERROR_ENVVAR_NOT_FOUND)
        {
          return
#ifdef NO_EXCEPTIONS
              false
#endif
              ;
        }
#else
      if (getenv("GRDIR") != nullptr)
        {
          return
#ifdef NO_EXCEPTIONS
              false
#endif
              ;
        }
#endif
    }

#ifdef NO_EXCEPTIONS
  decltype(getExecutablePath())::value_type exePath;
  if (auto exePath_ = getExecutablePath())
    {
      exePath = *exePath_;
    }
  else
    {
      return false;
    }
#else
  auto exePath = getExecutablePath();
#endif
#ifdef _WIN32
  std::array<wchar_t, MAXPATHLEN> exeDirname;
  std::wstringstream grDirRelativeStream;
  std::array<wchar_t, MAXPATHLEN> grDirAbsolute;
  std::wstringstream grHeaderPathStream;

  if (_wsplitpath_s(exePath.c_str(), nullptr, 0, exeDirname.data(), MAXPATHLEN, nullptr, 0, nullptr, 0))
    {
      throw_(DirnameError(exePath));
    }
  grDirRelativeStream << exeDirname.data() << L"/..";
  if (_wfullpath(grDirAbsolute.data(), grDirRelativeStream.str().c_str(), MAXPATHLEN) == nullptr)
    {
      throw_(AbsolutePathError(grDirRelativeStream.str()));
    }
  grHeaderPathStream << grDirAbsolute.data() << L"/include/gr.h";
  if (!fileExists(grHeaderPathStream.str()))
    {
      throw_(CorruptedGrDirError(grDirAbsolute.data()));
    }
  if (!SetEnvironmentVariableW(L"GRDIR", grDirAbsolute.data()))
    {
      throw_(SetEnvError());
    }
#else
  std::stringstream grDirRelativeStream;
  std::stringstream grHeaderPathStream;

  grDirRelativeStream
      << dirname(&std::string(exePath)[0])
#ifdef __APPLE__
      // On macOS, the `grplot` executable is located in `${GRDIR}/Applications/grplot.app/Contents/MacOS`
      << "/../../../..";
#else
      // On other Unixes, the `grplot` executable is located in `${GRDIR}/bin`
      << "/..";
#endif
  std::unique_ptr<char, decltype(std::free) *> grDirAbsoluteCptr{realpath(grDirRelativeStream.str().c_str(), nullptr),
                                                                 std::free};
  if (!grDirAbsoluteCptr)
    {
      throw_(AbsolutePathError());
    }
  grHeaderPathStream << grDirAbsoluteCptr.get() << "/include/gr.h";
  if (!fileExists(grHeaderPathStream.str()))
    {
      throw_(CorruptedGrDirError(grDirAbsoluteCptr.get()));
    }
  if (setenv("GRDIR", grDirAbsoluteCptr.get(), 1) != 0)
    {
      throw_(SetEnvError());
    }
#endif
#ifdef NO_EXCEPTIONS
  return true;
#endif
}
} // namespace util
