#ifndef __FreeBSD__
#ifdef __unix__
#define _POSIX_C_SOURCE 200809L
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef _WIN32
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#else
#include <windows.h>
#endif

#include "gks.h"
#include "gkscore.h"


#define SOCKET_FUNCTION_UNKNOWN 0
#define SOCKET_FUNCTION_CREATE_WINDOW 1
#define SOCKET_FUNCTION_DRAW 2
#define SOCKET_FUNCTION_IS_ALIVE 3
#define SOCKET_FUNCTION_CLOSE_WINDOW 4
#define SOCKET_FUNCTION_IS_RUNNING 5
#define SOCKET_FUNCTION_INQ_WS_STATE 6


#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#define PORT 8410

typedef struct
{
  int s;
  int wstype;
  gks_display_list_t dl;
  double aspect_ratio;
} ws_state_list;

static gks_state_list_t *gkss;

static int is_running = 0;

#ifdef _WIN32

#define CMD_LINE_LEN 8192

static DWORD WINAPI gksqt_tread(LPVOID parm)
{
  char *cmd = (char *)parm;
  wchar_t *w_cmd;
  int len = strlen(cmd);
  int w_len = MultiByteToWideChar(CP_UTF8, 0, cmd, len, NULL, 0) + 1;
  wchar_t w_cmd_line[CMD_LINE_LEN];
  STARTUPINFOW startupInfo;
  PROCESS_INFORMATION processInformation;

  w_cmd = (wchar_t *)gks_malloc(sizeof(wchar_t) * w_len);
  MultiByteToWideChar(CP_UTF8, 0, cmd, len + 1, w_cmd, w_len);

  swprintf(w_cmd_line, CMD_LINE_LEN, L"cmd /c \"%ls\"", w_cmd);

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

  free(w_cmd);

  return 0;
}

#else

static void *gksqt_tread(void *arg)
{
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

  pid = fork();
  if (pid < 0)
    {
      fprintf(stderr, "Fork failed\n");
      return NULL;
    }
  else if (pid == 0)
    {
      saDefault.sa_handler = SIG_DFL;
      saDefault.sa_flags = 0;
      sigemptyset(&saDefault.sa_mask);

      if (saOrigInt.sa_handler != SIG_IGN) sigaction(SIGINT, &saDefault, NULL);
      if (saOrigQuit.sa_handler != SIG_IGN) sigaction(SIGQUIT, &saDefault, NULL);

      sigprocmask(SIG_SETMASK, &origMask, NULL);

      is_running = 1;
      execl("/bin/sh", "sh", "-c", (char *)arg, (char *)NULL);
      is_running = 0;

      exit(127);
    }

  sigprocmask(SIG_SETMASK, &origMask, NULL);
  sigaction(SIGINT, &saOrigInt, NULL);
  sigaction(SIGQUIT, &saOrigQuit, NULL);
#else
  is_running = 1;
  system((char *)arg);
  is_running = 0;
#endif

  return NULL;
}

#endif

static int start(const char *cmd)
{
#ifdef _WIN32
  DWORD thread;

  if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)gksqt_tread, (void *)cmd, 0, &thread) == NULL) return -1;
#else
  pthread_t thread;

  if (pthread_create(&thread, NULL, gksqt_tread, (void *)cmd)) return -1;
#endif
  return 0;
}

static int connect_socket(int quiet)
{
  int s;
  char *env;
  struct hostent *hp;
  struct sockaddr_in sin;
  int opt;

#if defined(_WIN32)
  WORD wVersionRequested = MAKEWORD(1, 1);
  WSADATA wsaData;

  if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
      fprintf(stderr, "Can't find a usable WinSock DLL\n");
      return -1;
    }
#endif

  s = socket(PF_INET,      /* get a socket descriptor */
             SOCK_STREAM,  /* stream socket           */
             IPPROTO_TCP); /* use TCP protocol        */
  if (s == -1)
    {
      if (!quiet) perror("socket");
      return -1;
    }

  opt = 1;
#ifdef SO_REUSEADDR
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
#endif

  env = (char *)gks_getenv("GKS_CONID");
  if (env)
    if (!*env) env = NULL;
  if (!env) env = (char *)gks_getenv("GKSconid");

  if ((hp = gethostbyname(env != NULL ? env : "127.0.0.1")) == 0)
    {
      if (!quiet) perror("gethostbyname");
      return -1;
    }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr_list[0]))->s_addr;
  sin.sin_port = htons(PORT);

  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) == -1)
    {
      if (!quiet) perror("connect");
      return -1;
    }

  return s;
}

static int open_socket(int wstype)
{
  const char *command = NULL, *env;
  int retry_count;
  int max_retry_count = 20;
  char *cmd = NULL;
  int s;

  if (wstype == 411)
    {
      command = gks_getenv("GKS_QT");
      if (command == NULL)
        {
          env = gks_getenv("GRDIR");
          if (env == NULL) env = GRDIR;

          cmd = (char *)gks_malloc(MAXPATHLEN);
#ifndef _WIN32
#ifdef __APPLE__
          snprintf(cmd, MAXPATHLEN, "%s/Applications/gksqt.app/Contents/MacOS/gksqt", env);
#else
          snprintf(cmd, MAXPATHLEN, "%s/bin/gksqt", env);
#endif
#else
          snprintf(cmd, MAXPATHLEN, "%s\\bin\\gksqt.exe", env);
#endif
          command = cmd;
        }
    }

  for (retry_count = 1; retry_count <= max_retry_count; retry_count++)
    {
      if ((s = connect_socket(retry_count != max_retry_count)) == -1)
        {
          if (command != NULL && retry_count == 1)
            {
              /* For Julia BinaryBuilder environments the command string can be set to ""
                 because in this case gksqt is started by the GR.jl wrapper script */
              if (*command)
                {
                  if (start(command) != 0) gks_perror("could not auto-start GKS Qt application");
                }
            }
#ifndef _WIN32
          {
            struct timespec delay = {0, 300000000};
            while (nanosleep(&delay, &delay) == -1)
              ;
          }
#else
          Sleep(300);
#endif
        }
      else
        break;
    }

  is_running = (retry_count <= max_retry_count);

  if (cmd != NULL) free(cmd);

  return s;
}

#ifdef _WIN32
static void win_perror(char *text)
{
  wchar_t *s = NULL;
  FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                 WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&s, 0, NULL);
  fprintf(stderr, "%s: %S\n", text, s);
  LocalFree(s);
}
#endif

static int send_socket(int s, char *buf, int size, int ignore_error)
{
  int sent, n = 0;

  for (sent = 0; sent < size; sent += n)
    {
      if ((n = send(s, buf + sent, size - sent, 0)) == -1)
        {
          if (!ignore_error)
            {
#ifdef _WIN32
              win_perror("send");
#else
              perror("send");
#endif
            }
          is_running = 0;
          return -1;
        }
    }
  return sent;
}

static int read_socket(int s, char *buf, int size, int ignore_error)
{
  int read, n = 0;
  for (read = 0; read < size; read += n)
    {
      if ((n = recv(s, buf + read, size - read, 0)) <= 0)
        {
          if (n != 0 && !ignore_error)
            {
#ifdef _WIN32
              win_perror("read");
#else
              perror("read");
#endif
            }
          is_running = 0;
          return -1;
        }
    }
  return read;
}

static int close_socket(int s)
{
#if defined(_WIN32)
  closesocket(s);
#else
  close(s);
#endif
#if defined(_WIN32) && !defined(__GNUC__)
  WSACleanup();
#endif
  return 0;
}

static int check_socket_connection(ws_state_list *wss)
{
  if (wss->s != -1 && wss->wstype == 411)
    {
      char request_type = SOCKET_FUNCTION_IS_ALIVE;
      char reply;
      if (!(send_socket(wss->s, &request_type, sizeof(request_type), 1) == sizeof(request_type) &&
            read_socket(wss->s, &reply, sizeof(reply), 1) == sizeof(reply) && reply == SOCKET_FUNCTION_IS_ALIVE))
        {
          is_running = 0;
        }
    }
  if (!is_running)
    {
      close_socket(wss->s);
      wss->s = open_socket(wss->wstype);
      if (wss->s != -1 && wss->wstype == 411)
        {
          /* workstation information was already read during OPEN_WS */
          int nbytes;
          if (read_socket(wss->s, (char *)&nbytes, sizeof(int), 0) == sizeof(int))
            {
              char *buf = gks_malloc(nbytes - (int)sizeof(int));
              read_socket(wss->s, buf, nbytes - (int)sizeof(int), 0);
              gks_free(buf);
            }
        }
    }
}

void gks_drv_socket(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                    char *chars, void **ptr)
{
  ws_state_list *wss;
  char request_type;

  wss = (ws_state_list *)*ptr;

  switch (fctid)
    {
    case 2:
      gkss = (gks_state_list_t *)*ptr;
      wss = (ws_state_list *)gks_malloc(sizeof(ws_state_list));

      wss->wstype = ia[2];
      wss->s = open_socket(ia[2]);
      if (wss->s == -1)
        {
          gks_perror("can't connect to GKS socket application\n");

          gks_free(wss);
          wss = NULL;

          ia[0] = ia[1] = 0;
        }
      else
        {
          *ptr = wss;
          if (wss->wstype == 411)
            {
              /* get workstation information */
              int nbytes;
              struct
              {
                int nbytes;
                double mwidth;
                double mheight;
                int width;
                int height;
                char name[6];
              } workstation_information = {sizeof(workstation_information), 0, 0, 0, 0, ""};
              if (read_socket(wss->s, (char *)&nbytes, sizeof(int), 0) == sizeof(int) &&
                  nbytes == workstation_information.nbytes)
                {
                  read_socket(wss->s, (char *)&workstation_information + sizeof(int), nbytes - (int)sizeof(int), 0);
                  ia[0] = workstation_information.width;
                  ia[1] = workstation_information.height;
                  r1[0] = workstation_information.mwidth;
                  r2[0] = workstation_information.mheight;
                }
            }
          wss->aspect_ratio = 1.0;
          /*
           * TODO: Send `CREATE_WINDOW` on open workstation or implicit window creation?
           * request_type = SOCKET_FUNCTION_CREATE_WINDOW;
           * send_socket(wss->s, &request_type, 1, 0);
           */
        }
      break;

    case 3:
      if (wss->wstype == 411)
        {
          request_type = SOCKET_FUNCTION_CLOSE_WINDOW;
          send_socket(wss->s, &request_type, 1, 0);
        }
      close_socket(wss->s);
      if (wss->dl.buffer)
        {
          free(wss->dl.buffer);
        }
      gks_free(wss);
      wss = NULL;
      break;

    case 8:
      if (ia[1] & GKS_K_PERFORM_FLAG)
        {
          check_socket_connection(wss);
          request_type = SOCKET_FUNCTION_DRAW;
          if (wss->wstype == 411)
            {
              send_socket(wss->s, &request_type, 1, 0);
            }
          send_socket(wss->s, (char *)&wss->dl.nbytes, sizeof(int), 0);
          send_socket(wss->s, wss->dl.buffer, wss->dl.nbytes, 0);
        }
      break;

    case 54: /* set workstation window */
      wss->aspect_ratio = (r1[1] - r1[0]) / (r2[1] - r2[0]);
      break;

    case 209: /* inq_ws_state */
      check_socket_connection(wss);
      if (wss->wstype == 411)
        {
          char reply[1 + sizeof(gks_ws_state_t)];
          request_type = SOCKET_FUNCTION_INQ_WS_STATE;
          if (send_socket(wss->s, &request_type, 1, 0) <= 0)
            {
              break;
            }
          if (read_socket(wss->s, reply, sizeof(reply), 0) <= 0)
            {
              break;
            }
          if (reply[0] == SOCKET_FUNCTION_INQ_WS_STATE)
            {
              const gks_ws_state_t *state = (const gks_ws_state_t *)&reply[1];
              if (state->width > state->height * wss->aspect_ratio)
                {
                  ia[0] = (int)(state->height * wss->aspect_ratio + 0.5);
                  ia[1] = state->height;
                }
              else
                {
                  ia[0] = state->width;
                  ia[1] = (int)(state->width / wss->aspect_ratio + 0.5);
                }
              r1[0] = state->device_pixel_ratio;
            }
          else
            {
              ia[0] = 500;
              ia[1] = 500;
              r1[0] = 1.0;
            }
        }
      break;
    }

  if (wss != NULL)
    {
      gks_dl_write_item(&wss->dl, fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, gkss);
    }
}
