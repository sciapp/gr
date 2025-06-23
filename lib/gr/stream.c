#ifndef __FreeBSD__
#ifdef __unix__
#define _POSIX_C_SOURCE 200809L
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdarg.h>

#ifndef _WIN32
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/errno.h>
#else
#define _WIN32_WINNT 0x0602
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <strsafe.h>
#endif

#include "gr.h"
#include "gks.h"
#include "gkscore.h"

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

static int status = EXIT_SUCCESS;

static FILE *stream = NULL;

static int s = -1;

static char *buffer = NULL, *static_buffer = NULL;

static char *server = NULL;

static char *port = "4660"; /* 0x1234 */

static int nbytes = 0, size = 0, static_size = 0;

static int is_running = 0;

static int grplot_port = 8002;

static void close_socket(int s)
{
#ifndef _WIN32
  close(s);
#else
  closesocket(s);
#endif

#if defined(_WIN32) && !defined(__GNUC__)
  WSACleanup();
#endif
}

static void save(char *string, int nbytes)
{
  if (nbytes > static_size)
    {
      static_buffer = (char *)realloc(static_buffer, nbytes + 1);
      static_size = nbytes + 1;
    }
#ifdef _WIN32
  StringCchCopyA(static_buffer, static_size, string);
#else
  strcpy(static_buffer, string);
#endif
}

static int sendstream(char *string)
{
  int rc, pos, i;
  struct addrinfo hints, *res = NULL;
  char buf[BUFSIZ + 1];
  char *env, *display;

  if (status == EXIT_SUCCESS)
    {
#if defined(_WIN32) && !defined(__GNUC__)
      WORD wVersionRequested = MAKEWORD(1, 1);
      WSADATA wsaData;

      if (WSAStartup(wVersionRequested, &wsaData) != 0)
        {
          fprintf(stderr, "Can't find a usable WinSock DLL\n");
          status = EXIT_FAILURE;
          return status;
        }
#endif

      if (s == -1)
        {
          if (server == NULL)
            {
              env = (char *)getenv("GR_DISPLAY");
              if (env != NULL)
                {
                  display = gks_strdup(env);
                  if ((env = strtok(display, ":")) != NULL) server = env;
                  if ((env = strtok(NULL, ":")) != NULL) port = env;
                }
            }
          if (server == NULL) server = "localhost";

          memset(&hints, 0x00, sizeof(hints));
          hints.ai_family = AF_INET;
          hints.ai_socktype = SOCK_STREAM;

          if ((rc = getaddrinfo(server, port, &hints, &res)) != 0)
            {
              hints.ai_family = AF_INET6;
              rc = getaddrinfo(server, port, &hints, &res);
            }

          if (rc == 0)
            {
              s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
              if (s != -1)
                {
                  int size = 128 * 128 * 16;
                  setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(int));

                  if (connect(s, res->ai_addr, res->ai_addrlen) == -1)
                    {
                      perror("connect");
                      status = EXIT_FAILURE;
                    }
                }
              else
                {
                  perror("socket");
                  status = EXIT_FAILURE;
                }
            }
          else
            {
              fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
              status = EXIT_FAILURE;
            }
        }

      if (status == EXIT_SUCCESS)
        {
          pos = 0;
          for (i = 0; string[i] != 0; ++i)
            {
              buf[pos++] = string[i];
              if (pos == BUFSIZ)
                {
                  buf[pos] = 0;
                  if (send(s, buf, pos, 0) == -1)
                    {
                      perror("send");
                      status = EXIT_FAILURE;
                      break;
                    }
                  pos = 0;
                }
            }
          if (pos && status != EXIT_FAILURE)
            {
              if (send(s, buf, pos, 0) == -1)
                {
                  perror("send");
                  status = EXIT_FAILURE;
                }
            }
        }
      else if (s != -1)
        close_socket(s);

      if (res != NULL) freeaddrinfo(res);
    }

  return status;
}

static void append(char *string)
{
  int len = strlen(string);

  if (buffer == NULL)
    {
      buffer = (char *)malloc(BUFSIZ + 1);
      nbytes = 0;
      size = BUFSIZ;
    }

  if (nbytes + len > size)
    {
      while (nbytes + len > size) size += BUFSIZ;

      buffer = (char *)realloc(buffer, size + 1);
    }

  memcpy(buffer + nbytes, string, len);
  nbytes += len;
  buffer[nbytes] = '\0';
}

int gr_openstream(const char *path)
{
#ifdef _WIN32
  wchar_t w_path[MAX_PATH];
#endif

  if (path != NULL)
    {
      if (*path == '\0')
        status = -1;
      else if (strchr(path, ':') == NULL)
        {
#ifdef _WIN32
          MultiByteToWideChar(CP_UTF8, 0, path, strlen(path) + 1, w_path, MAX_PATH);
          stream = _wfopen(w_path, L"w");
#else
          stream = fopen(path, "w");
#endif
          if (stream == NULL)
            {
              perror("fopen");
              status = EXIT_FAILURE;
              return -1;
            }
        }
    }

  if (buffer == NULL)
    {
      buffer = (char *)malloc(BUFSIZ + 1);
      size = BUFSIZ;
    }
  nbytes = 0;
  *buffer = '\0';

  return 0;
}

void gr_writestream(char *string, ...)
{
  va_list ap;
  char s[BUFSIZ];

  va_start(ap, string);
  vsnprintf(s, BUFSIZ, string, ap);
  va_end(ap);

  if (gr_debug())
    {
      if (*s == '<')
        fprintf(stdout, "[DEBUG:GR] %s", s);
      else
        fprintf(stdout, "%s", s);
    }

  if (stream != NULL) append(s);
}

void gr_flushstream(int discard)
{
  if (buffer != NULL)
    {
      if (!discard)
        {
          if (stream != NULL)
            fwrite(buffer, nbytes, 1, stream);
          else if (status != -1)
            sendstream(buffer);
          else
            save(buffer, nbytes);
        }
      nbytes = 0;
      *buffer = '\0';
    }
}

void gr_closestream(void)
{
  gr_flushstream(0);

  if (stream)
    if (stream != NULL) fclose(stream);

  free(buffer);
  buffer = NULL;
}

char *gr_getgraphics(void)
{
  return static_buffer;
}

#ifdef _WIN32

#define CMD_LINE_LEN (32767 + 10)
/*
 * The maximum length of an environment variable is 32767 characters plus 10 characters for 'cmd /c ""'
 */

static DWORD WINAPI grplot_thread(LPVOID parm)
{
  wchar_t *cmd = (char *)parm;
  wchar_t w_cmd_line[CMD_LINE_LEN];
  STARTUPINFOW startupInfo;
  PROCESS_INFORMATION processInformation;

  StringCbPrintfW(w_cmd_line, CMD_LINE_LEN, L"cmd /c \"%ls\"", cmd);

  ZeroMemory(&startupInfo, sizeof(startupInfo));
  startupInfo.cb = sizeof(startupInfo);
  ZeroMemory(&processInformation, sizeof(processInformation));

  is_running = 1;
  CreateProcessW(NULL, w_cmd_line, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE | CREATE_NO_WINDOW | DETACHED_PROCESS,
                 NULL, NULL, &startupInfo, &processInformation);
  WaitForSingleObject(processInformation.hThread, INFINITE);
  is_running = 0;

  CloseHandle(processInformation.hProcess);
  CloseHandle(processInformation.hThread);

  return 0;
}

#else

static void *grplot_thread(void *arg)
{
  int retstat = 0;
#ifdef __APPLE__
  sigset_t blockMask, origMask;
  struct sigaction saIgnore, saOrigQuit, saOrigInt, saDefault;
  pid_t pid;

  sigemptyset(&blockMask);
  sigaddset(&blockMask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &blockMask, &origMask);

  saIgnore.sa_handler = SIG_IGN;
  saIgnore.sa_flags = 0;
  sigemptyset(&saIgnore.sa_mask);
  sigaction(SIGINT, &saIgnore, &saOrigInt);
  sigaction(SIGQUIT, &saIgnore, &saOrigQuit);

  is_running = 1;
  if ((pid = fork()) == 0)
    {
      saDefault.sa_handler = SIG_DFL;
      saDefault.sa_flags = 0;
      sigemptyset(&saDefault.sa_mask);

      if (saOrigInt.sa_handler != SIG_IGN) sigaction(SIGINT, &saDefault, NULL);
      if (saOrigQuit.sa_handler != SIG_IGN) sigaction(SIGQUIT, &saDefault, NULL);

      sigprocmask(SIG_SETMASK, &origMask, NULL);
      execl("/bin/sh", "sh", "-c", (char *)arg, (char *)NULL);

      _exit(127);
    }
  if (pid == -1)
    {
      fprintf(stderr, "Fork failed\n");
      retstat = -1;
    }
  else
    {
      int status;
      while (waitpid(pid, &status, 0) == -1)
        {
          if (errno != EINTR)
            {
              retstat = WIFEXITED(status) != 0 ? WEXITSTATUS(status) : -1;
              break;
            }
        }
    }
  is_running = 0;

  sigprocmask(SIG_SETMASK, &origMask, NULL);
  sigaction(SIGINT, &saOrigInt, NULL);
  sigaction(SIGQUIT, &saOrigQuit, NULL);
#else
  is_running = 1;
  retstat = system((char *)arg);
  is_running = 0;
#endif

  return retstat == 0 ? arg : NULL;
}

#endif

static int start(void *cmd)
{
#ifdef _WIN32
  DWORD thread;

  if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)grplot_thread, cmd, 0, &thread) == NULL) return -1;
#else
  pthread_t thread;

  if (pthread_create(&thread, NULL, grplot_thread, cmd)) return -1;
#endif
  return 0;
}

static int connect_socket(int quiet, int used_port)
{
  int rc, s;
  struct addrinfo hints, *res = NULL;
  int opt;
  char port[6];
  snprintf(port, 6, "%i", used_port);

#if defined(_WIN32)
  WORD wVersionRequested = MAKEWORD(1, 1);
  WSADATA wsaData;

  if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
      fprintf(stderr, "Can't find a usable WinSock DLL\n");
      return -1;
    }
#endif

  memset(&hints, 0x00, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if ((rc = getaddrinfo("localhost", port, &hints, &res)) != 0)
    {
      hints.ai_family = AF_INET6;
      if ((rc = getaddrinfo("localhost", port, &hints, &res)) != 0)
        {
          if (!quiet)
            {
              fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
            }
          return -1;
        }
    }

  s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (s < 0)
    {
      if (!quiet) perror("socket");
      freeaddrinfo(res);
      return -1;
    }

  opt = 1;
#ifdef SO_REUSEADDR
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
#endif

  if (connect(s, res->ai_addr, res->ai_addrlen) < 0)
    {
      if (!quiet) perror("connect");
      freeaddrinfo(res);
      return -1;
    }

  freeaddrinfo(res);

  return s;
}

int gr_inqgrplotport()
{
  return grplot_port;
}

int gr_setgrplotport(int port)
{
  if (port <= 0 || port > 65535)
    {
      /* select a random port if port is invalid */
      port = (rand() % 32768) + 30000;
    }
  grplot_port = port;
  return grplot_port;
}

int gr_startlistener(void)
{
#ifdef _WIN32
  wchar_t command[CMD_LINE_LEN], w_env[MAXPATHLEN];
#else
  const char *command = NULL, *env;
  char *cmd = NULL;
#endif
  size_t retry_count, max_retry_count = 50;
  int s;

  /* In order to not sleep an excessive amount start with a short sleep time and then ramp
     it up to `max_sleep_time` */
  int sleep_ms;
  int ms_to_ns = 1000000;
  int initial_sleep_time_ms[] = {5, 10, 25, 50, 100};
  int max_sleep_time_ms = 300;
  size_t n_initial_times = sizeof(initial_sleep_time_ms) / sizeof(initial_sleep_time_ms[0]);
  max_retry_count += n_initial_times;

#ifdef _WIN32
  if (!GetEnvironmentVariableW(L"GR_PLOT", command, CMD_LINE_LEN))
    {
      if (!GetEnvironmentVariableW(L"GRDIR", w_env, MAXPATHLEN))
        {
          StringCbPrintfW(command, CMD_LINE_LEN, L"%S\\bin\\grplot.exe --listen %i", GRDIR, grplot_port);
        }
      else
        {
          StringCbPrintfW(command, CMD_LINE_LEN, L"%s\\bin\\grplot.exe --listen %i", w_env, grplot_port);
        }
    }
#else
  command = gks_getenv("GR_PLOT");
  if (command == NULL)
    {
      env = gks_getenv("GRDIR");
      if (env == NULL) env = GRDIR;

      cmd = (char *)gks_malloc(MAXPATHLEN);
#ifdef __APPLE__
      snprintf(cmd, MAXPATHLEN, "%s/Applications/grplot.app/Contents/MacOS/grplot --listen %i", env, grplot_port);
#else
      snprintf(cmd, MAXPATHLEN, "%s/bin/grplot --listen %i", env, grplot_port);
#endif
      command = cmd;
    }
#endif
  if (!gks_getenv("QT_AUTO_SCREEN_SCALE_FACTOR"))
    {
#ifdef _WIN32
      putenv("QT_AUTO_SCREEN_SCALE_FACTOR=1");
#else
      setenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1", 1);
#endif
    }

  for (retry_count = 1; retry_count <= max_retry_count; retry_count++)
    {
      if ((s = connect_socket(retry_count != max_retry_count, grplot_port)) == -1)
        {
          if (command != NULL && retry_count == 1)
            {
              /* For Julia BinaryBuilder environments the command string can be set to ""
                 because in this case grplot is started by the GR.jl wrapper script */
              if (*command)
                {
                  if (start((void *)command) != 0) gks_perror("could not auto-start GR Plot application");
                }
            }
          sleep_ms = retry_count <= n_initial_times ? initial_sleep_time_ms[retry_count - 1] : max_sleep_time_ms;
#ifndef _WIN32
          {
            struct timespec delay;
            delay.tv_sec = 0;
            delay.tv_nsec = sleep_ms * ms_to_ns;
            while (nanosleep(&delay, &delay) == -1)
              ;
          }
#else
          Sleep(sleep_ms);
#endif
        }
      else
        break;
    }

  close_socket(s);

  is_running = (retry_count <= max_retry_count);

#ifndef _WIN32
  if (cmd != NULL) free(cmd);
#endif

  return s;
}
