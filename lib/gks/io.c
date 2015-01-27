
#include <stdio.h>
#include <stdlib.h>

#if !defined(VMS) && !defined(_WIN32)
#include <unistd.h>
#endif

#ifdef _WIN32
#include <io.h>
#endif

#if defined(cray) || defined(__SVR4) || defined(_WIN32)
#include <fcntl.h>
#else
#include <sys/file.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

#include "gkscore.h"

int gks_open_file(const char *path, const char *mode)
{
  int fd, flags, omode;

  switch (*mode)
    {
    case 'r':
#if defined(_WIN32)
      flags = O_RDONLY | O_BINARY;
#else
      flags = O_RDONLY;
#endif
      omode = 0;
      break;

    case 'w':
#if defined(_WIN32)
      flags = O_CREAT | O_TRUNC | O_WRONLY | O_BINARY;
      omode = S_IREAD | S_IWRITE;
#else
      flags = O_CREAT | O_TRUNC | O_WRONLY;
      omode = 0644;
#endif
      break;

    default:
      return -1;
    }

  fd = open(path, flags, omode);
  if (fd < 0)
    {
      gks_perror("file open error (%s)", path);
      perror("open");
    }

  return fd;
}

int gks_read_file(int fd, void *buf, int count)
{
  int cc;

  cc = read(fd, buf, count);
  if (cc != count)
    {
      gks_perror("file read error (fd=%d, cc=%d)", fd, cc);
      if (cc == -1)
	perror("read");
    }

  return cc;
}

int gks_write_file(int fd, void *buf, int count)
{
  int cc;

  cc = write(fd, buf, count);
  if (cc != count)
    {
      gks_perror("file write error (fd=%d, cc=%d)", fd, cc);
      if (cc == -1)
	perror("write");
    }

  return cc;
}

int gks_close_file(int fd)
{
  int result;

  result = close(fd);
  if (result < 0)
    {
      gks_perror("file close error (fd=%d)", fd);
      perror("close");
    }

  return result;
}
