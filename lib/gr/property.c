
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#else
#include <winsock.h>
#endif

#define PORT 0x1234

#include "property.h"

static
int status = EXIT_SUCCESS;

static
int s = -1;

static
char *buffer = NULL;

static
char *hostname = NULL;

static
int port = PORT;

static
int nbytes = 0, size = 0;

static
void close_socket(int s)
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

static
int setproperty(char *string)
{
  int pos, i;
  struct hostent *hp;
  struct sockaddr_in sin;
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
	  s = socket(PF_INET,	/* get a socket descriptor */
		 SOCK_STREAM,	/* stream socket           */
		 IPPROTO_TCP);	/* use TCP protocol        */
	  if (s != -1)
	    {
	      int size = 128*128*16;
	      setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(int));

	      if (hostname == NULL)
		{
		  env = (char *) getenv("GR_DISPLAY");
		  if (env != NULL)
		    {
		      display = strdup(env);
		      if ((env = strtok(display, ":")) != NULL)
			hostname = env;
		      if ((env = strtok(NULL, ":")) != NULL)
			port = atoi(env);
		    }
		}
	      if (hostname == NULL)
		hostname = "localhost";
	     
	      if ((hp = gethostbyname(hostname)) != NULL)
		{
		  memset(&sin, 0, sizeof(sin));
		  sin.sin_family = AF_INET;
		  sin.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;
		  sin.sin_port = htons(port);

		  if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) == -1)
		    {
		      perror("connect");
		      status = EXIT_FAILURE;
		    }
		}
	      else
		{
		  perror("gethostbyname");
		  status = EXIT_FAILURE;
		}
	    }
	  else
	    {
	      perror("socket");
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
    }

  return status;
}

static
void addbuffer(char *string)
{
  int len = strlen(string);

  if (buffer == NULL)
    {
      buffer = (char *) malloc(BUFSIZ + 1);
      nbytes = 0;
      size = BUFSIZ;
    }

  if (nbytes + len > size)
    {
      while (nbytes + len > size)
	size += BUFSIZ;

      buffer = (char *) realloc(buffer, size + 1);
    }

  strncpy(buffer + nbytes, string, len);
  nbytes += len;
  buffer[nbytes] = '\0';
}

void gr_setproperty(char *string, ...)
{
  va_list ap;
  char s[BUFSIZ];

  va_start(ap, string);
  vsprintf(s, string, ap);
  va_end(ap);

  setproperty(s);
}

void gr_beginproperty(char *string, ...)
{
  va_list ap;
  char s[BUFSIZ];

  va_start(ap, string);
  vsprintf(s, string, ap);
  va_end(ap);

  nbytes = 0;
  addbuffer(s);
}

void gr_addproperty(char *string, ...)
{
  va_list ap;
  char s[BUFSIZ];

  va_start(ap, string);
  vsprintf(s, string, ap);
  va_end(ap);

  addbuffer(s);
}

void gr_endproperty(void)
{
  if (buffer != NULL)
    {
      setproperty(buffer);

      nbytes = 0;
      *buffer = '\0';
    }
}
