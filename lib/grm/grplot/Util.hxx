#ifndef UTIL_HXX_INCLUDED
#define UTIL_HXX_INCLUDED

#include <optional>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <QPoint>
#include <vector>

#ifdef _WIN64
#include <stdlib.h>
#include <io.h>
#include <process.h>
#include <direct.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif

namespace util
{
template <typename T> int sgn(T x)
{
  return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
}

template <class... T> void unused(T &&...) {}

class GetExecutablePathError : public virtual std::exception
{
public:
  [[nodiscard]] const char *what() const noexcept override = 0;
};

class SetGrDirError : public virtual std::exception
{
public:
  [[nodiscard]] const char *what() const noexcept override = 0;
};

#if defined __unix__ || defined __APPLE__
class ErrnoError : public virtual std::exception
{
public:
  ErrnoError();
  [[nodiscard]] const char *what() const noexcept override;

private:
  std::string what_str_;
};
#elif defined _WIN32
class FormatMessageError : public std::exception
{
public:
  [[nodiscard]] const char *what() const noexcept override;
};

class GetLastErrorError : public virtual std::exception
{
public:
  GetLastErrorError();
  [[nodiscard]] const char *what() const noexcept override;

private:
  std::string what_str_;
};
#endif

#if defined __APPLE__
class PathTooLongError : public GetExecutablePathError
{
public:
  PathTooLongError(size_t neededSize);
  [[nodiscard]] const char *what() const noexcept override;

private:
  std::string what_str_;
};
#elif defined __unix__
class PathTooLongError : public GetExecutablePathError
{
public:
  PathTooLongError();
  [[nodiscard]] const char *what() const noexcept override;

private:
  std::string what_str_;
};

class ProcessFileLinkNotReadableError : public ErrnoError, public GetExecutablePathError
{
public:
  [[nodiscard]] const char *what() const noexcept override;
};
#endif

#if defined __unix__ || defined __APPLE__
class AbsolutePathError : public ErrnoError, public SetGrDirError
{
public:
  [[nodiscard]] const char *what() const noexcept override;
};

class CorruptedGrDirError : public SetGrDirError
{
public:
  CorruptedGrDirError(const std::string &path);
  [[nodiscard]] const char *what() const noexcept override;

private:
  std::string what_str_;
};

class SetEnvError : public ErrnoError, public SetGrDirError
{
public:
  [[nodiscard]] const char *what() const noexcept override;
};
#elif defined _WIN32
class WideCharToMultiByteError : public GetLastErrorError, public SetGrDirError
{
public:
  [[nodiscard]] const char *what() const noexcept override;
};

class ModulePathError : public GetLastErrorError, public SetGrDirError
{
public:
  [[nodiscard]] const char *what() const noexcept override;
};

class DirnameError : public SetGrDirError
{
public:
  DirnameError(const std::wstring &filepath);
  [[nodiscard]] const char *what() const noexcept override;

private:
  std::string what_str_;
};

class AbsolutePathError : public SetGrDirError
{
public:
  AbsolutePathError(const std::wstring &path);
  [[nodiscard]] const char *what() const noexcept override;

private:
  std::string what_str_;
};

class CorruptedGrDirError : public SetGrDirError
{
public:
  CorruptedGrDirError(const std::wstring &path);
  [[nodiscard]] const char *what() const noexcept override;

private:
  std::string what_str_;
};

class SetEnvError : public GetLastErrorError, public SetGrDirError
{
public:
  [[nodiscard]] const char *what() const noexcept override;
};
#endif

bool endsWith(const std::string &str, const std::string &suffix);
bool startsWith(const std::string &str, const std::string &prefix);
bool isDigits(const std::string &str);
bool isNumber(const std::string &str);
#ifdef _WIN32
bool fileExists(const std::string &file_path);
bool fileExists(const std::wstring &file_path);
std::wstring getEnvVar(const std::wstring &name, const std::wstring &default_value = L"");
#else
bool fileExists(const std::string &file_path);
std::string getEnvVar(const std::string &name, const std::string &default_value = "");
#endif

#ifdef _WIN32
std::wstring getExecutablePath();
#else
std::string getExecutablePath();
#endif

void setGrdir(bool force = false);

template <typename... Args> std::string stringFormat(const std::string &format, Args... args)
{
  // Modified version of <https://stackoverflow.com/a/26221725/5958465>
  const int needed_bytes = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
  if (needed_bytes <= 0) throw std::runtime_error("Error during formatting.");
  std::vector<char> buf(needed_bytes);
  std::snprintf(buf.data(), needed_bytes, format.c_str(), args...);
  return std::string(buf.data());
}

// `overloaded` utility taken from <https://en.cppreference.com/w/cpp/utility/variant/visit>
template <class... Ts> struct Overloaded : Ts...
{
  using Ts::operator()...;
};
template <class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;
} // namespace util

inline std::ostream &operator<<(std::ostream &os, const QPoint &point)
{
  return os << "(" << std::setw(4) << point.x() << ", " << std::setw(4) << point.y() << ")";
}

#endif /* ifndef UTIL_HXX_INCLUDED */
