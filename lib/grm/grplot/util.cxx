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
#if defined __unix__ || defined __APPLE__
#include <cerrno>
#include <cstdlib>
#include <cstring>
#endif
#include <array>
#include <exception>
#include <memory>
#include <sstream>
#include <vector>

#include "util.hxx"

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
  char *errorMessage = nullptr;

  if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                     nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorMessage, 0, nullptr))
    {
      throw FormatMessageError();
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

  whatStream << MAXPATHLEN << " Bytes are not sufficient for storing the path. ";
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

  whatStream << "Could not extract the directory name of \"" << filepath << "\"." << std::endl;

  int neededBytesUtf8String =
      WideCharToMultiByte(CP_UTF8, 0, whatStream.str().c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (neededBytesUtf8String == 0)
    {
      throw WideCharToMultiByteError();
    }
  std::vector<char> utf8Bytes(neededBytesUtf8String);
  int writtenBytesUtf8String = WideCharToMultiByte(CP_UTF8, 0, whatStream.str().c_str(), -1, utf8Bytes.data(),
                                                   neededBytesUtf8String, nullptr, nullptr);
  if (writtenBytesUtf8String == 0)
    {
      throw WideCharToMultiByteError();
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

  whatStream << "Could not determine the absolute path of \"" << path << "\"." << std::endl;

  int neededBytesUtf8String =
      WideCharToMultiByte(CP_UTF8, 0, whatStream.str().c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (neededBytesUtf8String == 0)
    {
      throw WideCharToMultiByteError();
    }
  std::vector<char> utf8Bytes(neededBytesUtf8String);
  int writtenBytesUtf8String = WideCharToMultiByte(CP_UTF8, 0, whatStream.str().c_str(), -1, utf8Bytes.data(),
                                                   neededBytesUtf8String, nullptr, nullptr);
  if (writtenBytesUtf8String == 0)
    {
      throw WideCharToMultiByteError();
    }
  whatStr_ = utf8Bytes.data();
}

const char *AbsolutePathError::what() const noexcept
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

#ifdef _WIN32
std::wstring getExecutablePath()
#else
std::string getExecutablePath()
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
      throw PathTooLongError(pathLen);
    }
#elif defined __FreeBSD__ || defined __NetBSD__ || defined __OpenBSD__
  ssize_t pathLen = -1;

  pathLen = readlink("/proc/curproc/file", exePath.data(), MAXPATHLEN);
  if (pathLen < 0)
    {
      throw ProcessFileLinkNotReadableError();
    }
  else if (pathLen == MAXPATHLEN)
    {
      throw PathTooLongError();
    }
#elif defined __linux__
  ssize_t pathLen = -1;

  pathLen = readlink("/proc/self/exe", exePath.data(), MAXPATHLEN);
  if (pathLen < 0)
    {
      throw ProcessFileLinkNotReadableError();
    }
  if (pathLen == MAXPATHLEN)
    {
      throw PathTooLongError();
    }
#elif defined _WIN32
  unsigned int pathLen = 0;

  pathLen = GetModuleFileNameW(nullptr, exePath.data(), MAXPATHLEN);
  if (GetLastError() != ERROR_SUCCESS)
    {
      throw ModulePathError();
    }
#else
#error "Unsupported system"
#endif
  exePath.at(pathLen) = '\0';

  return exePath.data();
}

void setGrdir(bool force)
{
#ifdef _WIN32
  std::wstring exePath;
#else
  std::string exePath;
#endif

  if (!force)
    {
#ifdef _WIN32
      GetEnvironmentVariableW(L"GRDIR", nullptr, 0);
      if (GetLastError() != ERROR_ENVVAR_NOT_FOUND)
        {
          return;
        }
#else
      if (getenv("GRDIR") != nullptr)
        {
          return;
        }
#endif
    }

  exePath = getExecutablePath();
#ifdef _WIN32
  std::array<wchar_t, MAXPATHLEN> exeDirname;
  std::wstringstream grDirRelativeStream;
  std::array<wchar_t, MAXPATHLEN> grDirAbsolute;

  if (_wsplitpath_s(exePath.c_str(), nullptr, 0, exeDirname.data(), MAXPATHLEN, nullptr, 0, nullptr, 0))
    {
      throw DirnameError(exePath);
    }
  grDirRelativeStream << exeDirname.data() << L"/..";
  if (_wfullpath(grDirAbsolute.data(), grDirRelativeStream.str().c_str(), MAXPATHLEN) == nullptr)
    {
      throw AbsolutePathError(grDirRelativeStream.str());
    }
  if (!SetEnvironmentVariableW(L"GRDIR", grDirAbsolute.data()))
    {
      throw SetEnvError();
    }
#else
  std::stringstream grDirRelativeStream;

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
      throw AbsolutePathError();
    }
  if (setenv("GRDIR", grDirAbsoluteCptr.get(), 1) != 0)
    {
      throw SetEnvError();
    }
#endif
}
