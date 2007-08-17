/*
 * Copyright (c) 1983,1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Hacks for OS/2 by Jochen Friedrich <jochen@audio.pfalz.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Inetd - Internet super-server
 *
 * This program invokes all internet services as needed.
 * connection-oriented services are invoked each time a
 * connection is made, by creating a process.  This process
 * is passed the connection as file descriptor 0 and is
 * expected to do a getpeername to find out the source host
 * and port.
 *
 * Datagram oriented services are invoked when a datagram
 * arrives; a process is created and passed a pending message
 * on file descriptor 0.  Datagram servers may either connect
 * to their peer, freeing up the original socket for inetd
 * to receive further messages on, or ``take over the socket'',
 * processing all arriving datagrams and, eventually, timing
 * out.  The first type of server is said to be ``multi-threaded'';
 * the second type of server ``single-threaded''. 
 *
 * Inetd uses a configuration file which is read at startup
 * and, possibly, at some later time in response to a hangup signal.
 * The configuration file is ``free format'' with fields given in the
 * order shown below.  Continuation lines for an entry must being with
 * a space or tab.  All fields must be present in each entry.
 *
 *      service name                    must be in /etc/services
 *      socket type                     stream/dgram
 *      protocol                        must be in /etc/protocols
 *      wait/nowait                     single-threaded/multi-threaded
 *      user                            user to run daemon as
 *      server program                  full path name
 *      server program arguments        maximum of MAXARGS (20)
 *
 * Comment lines are indicated by a `#' in column 1.
 */

#define INCL_DOS
#define INCL_BASE
#define INCL_DOSEXCEPTIONS
#include <os2.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <nerrno.h>
#include <direct.h>
#include <process.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <types.h>

#include "syslog.h"
#include "pwd.h"
#include "fix_env.h"

#define TOOMANY         40              /* don't start more than TOOMANY */
#define CNT_INTVL       60              /* servers in CNT_INTVL sec. */
#define RETRYTIME       (60*10)         /* retry after bind or server fail */
#define MAXSOCK         256

void    config(int);
void    reapchild(void *);
void    retry(void *);
struct  servtab *getconfigent(void);
struct  servtab *enter(struct servtab *);
void    echo_stream(int*);
void    discard_stream(int*);
void    machtime_stream(int*);
void    daytime_stream(int*);
void    chargen_stream(int*);
void    echo_dg(int*);
void    discard_dg(int*);
void    machtime_dg(int*);
void    daytime_dg(int*);
void    chargen_dg(int*);
void    setup(struct servtab *);
char    *skip(char **);
char    *nextline(FILE *);
void    endconfig(void);
void    freeconfig(struct servtab *);
void    print_service(char *, struct servtab *);
int     fd_isset(int, int *, int);
void    fd_set(int, int *, int *);
void    fd_clr(int, int *, int *);

int     retrytime = RETRYTIME;
int     debug = 0;
int     nsock, maxsock;
int     allsock[MAXSOCK];
int     options;
int     timingout;
int     cansock;
int     reapthread;
ULONG   reapsem;
struct  servent *sp;
char    userenv[1024];

struct  servtab {
        char    *se_service;            /* name of service */
        int     se_socktype;            /* type of socket to use */
        char    *se_proto;              /* protocol used */
        short   se_wait;                /* single threaded server */
        short   se_checked;             /* looked at during merge */
        char    *se_user;               /* user name to run as */
        struct  biltin *se_bi;          /* if built-in, description */
        char    *se_server;             /* server program */
#define MAXARGV 20
        char    *se_argv[MAXARGV+1];    /* program arguments */
        int     se_fd;                  /* open descriptor */
        int     se_sock;                /* descriptor for stream */
        struct  sockaddr_in se_ctrladdr;/* bound address */
        int     se_count;               /* number started since se_time */
        struct  timeval se_time;        /* start of se_count */
        struct  servtab *se_next;
} *servtab;

struct biltin {
        char    *bi_service;            /* internally provided service name */
        int     bi_socktype;            /* type of socket supported */
        short   bi_fork;                /* 1 if should fork before call */
        short   bi_wait;                /* 1 if should wait for child */
        void    (* _Optlink bi_fn)(int*); /* function which performs it */
} biltins[] = {
        /* Echo received data */
        "echo",         SOCK_STREAM,    1, 0,   echo_stream,
        "echo",         SOCK_DGRAM,     0, 0,   echo_dg,

        /* Internet /dev/null */
        "discard",      SOCK_STREAM,    1, 0,   discard_stream,
        "discard",      SOCK_DGRAM,     0, 0,   discard_dg,

        /* Return 32 bit time since 1970 */
        "time",         SOCK_STREAM,    0, 0,   machtime_stream,
        "time",         SOCK_DGRAM,     0, 0,   machtime_dg,

        /* Return human-readable time */
        "daytime",      SOCK_STREAM,    0, 0,   daytime_stream,
        "daytime",      SOCK_DGRAM,     0, 0,   daytime_dg,

        /* Familiar character generator */
        "chargen",      SOCK_STREAM,    1, 0,   chargen_stream,
        "chargen",      SOCK_DGRAM,     0, 0,   chargen_dg,
        0
};

#define NUMINT  (sizeof(intab) / sizeof(struct inent))
char    CONFIG[255] = "c:\\etc\\inetd.cnf";
char    **Argv;
char    *LastArg;

int main(int argc, char *argv[], char *envp[])
{
  extern char *optarg;
  extern int optind;
  register struct servtab *sep;
  register int tmpint;
  int ch, pid, dofork;
  char buf[50];
  char *wdir;
  struct passwd *pass;

  if (sock_init()) {
    fprintf(stderr,"Can't initialize TCP/IP");
    exit(-1);
  }
  if (!(cansock = socket(AF_INET, SOCK_STREAM, 0))) {
    fprintf(stderr,"Can't open sync socket");
    exit(-1);
  }
  fix_env(CONFIG,"ETC");
  Argv = argv;
  if (envp == 0 || *envp == 0)
    envp = argv;
  while (*envp)
    envp++;
  LastArg = envp[-1] + strlen(envp[-1]);

  while ((ch = getopt(argc, argv, "d")) != EOF)
    switch(ch) {
      case 'd':
        debug = 1;
        options |= SO_DEBUG;
        break;
      case '?':
      default:
        fprintf(stderr, "usage: inetd [-d]");
        exit(1);
    }
  argc -= optind;
  argv += optind;

  if (argc > 0)
    strcpy(CONFIG,argv[0]);
  openlog("inetd", LOG_PID | LOG_NOWAIT, LOG_DAEMON);
  config(0);
  if (DosCreateEventSem(NULL, &reapsem, 0L, FALSE)) {
    fprintf(stderr,"Can't create semaphore");
    exit(-1);
  }
  if (!( reapthread = _beginthread(reapchild, NULL, 8192, NULL))) {
    fprintf(stderr,"Can't create thread for reap");
    exit(-1);
  }
  for (;;) {
    int n;
    int readable[257];

    for(n=0;n<nsock;n++) readable[n] = allsock[n];
    readable[nsock]=cansock;
    if (debug)
      fprintf(stderr, "enter select with %d sockets\n",nsock);
    if ((n = select(readable, nsock, 0, 1, -1L)) <= 0) {
      errno = sock_errno();
      if (n < 0 && errno != SOCEINTR) {
        syslog(LOG_WARNING, "select: %m\n");
        DosSleep(1000);
      }
      continue;
    }
    for (sep = servtab; n && sep; sep = sep->se_next) {
      if (sep->se_fd != -1 && fd_isset(sep->se_fd, readable, nsock)) {
        n--;
        if (debug)
          fprintf(stderr, "someone wants %s\n",
                  sep->se_service);
        if (sep->se_socktype == SOCK_STREAM) {
          sep->se_sock = accept(sep->se_fd, (struct sockaddr *)0,
                        (int *)0);
          errno = sock_errno();
          if (debug)
            fprintf(stderr, "accept, socket %d\n", sep->se_sock);
          if (sep->se_sock < 0) {
            if (errno == SOCEINTR)
              continue;
            syslog(LOG_WARNING, "accept (for %s): %m",
                   sep->se_service);
            continue;
          }
        } else
          sep->se_sock = sep->se_fd;
        dofork = (sep->se_bi == 0 || sep->se_bi->bi_fork);
        if (dofork) {
          if (sep->se_count++ == 0)
            gettimeofday(&sep->se_time,
                         (struct timezone *)0);
          else if (sep->se_count >= TOOMANY) {
            struct timeval now;
            gettimeofday(&now, (struct timezone *)0);
            if (now.tv_sec - sep->se_time.tv_sec >
                CNT_INTVL) {
              sep->se_time = now;
              sep->se_count = 1;
            } else {
              syslog(LOG_ERR,
                     "%s/%s server failing (looping), service terminated\n",
                     sep->se_service, sep->se_proto);
              fd_clr(sep->se_fd, allsock, &nsock);
              soclose(sep->se_fd);
              sep->se_fd = -1;
              sep->se_count = 0;
              if (!timingout) {
                timingout = 1;
                _beginthread(retry,NULL,32768,&retrytime);
              }
            }
          }
        }
        pid=0;
        if (sep->se_bi) {
          int *ctrl;
          ctrl=malloc(sizeof(int));
          *ctrl=sep->se_sock;
          if (dofork)
            _beginthread( (void(*)(void*)) (sep->se_bi->bi_fn),NULL,32768,ctrl);
          else
            (*sep->se_bi->bi_fn)(ctrl);
          }
        else {
          char* argv[MAXARGV+1];
          int i;
          char socknum[10];

          sprintf(socknum,"%d",sep->se_sock);

          if (debug)
            fprintf(stderr, "%d execl %s\n",
          getpid(), sep->se_server);

          for (i=0;i<=MAXARGV;i++)
            if ((sep->se_argv[i])&&(strcmp(sep->se_argv[i],"%s")==0))
              argv[i]=socknum;
            else
              argv[i]=sep->se_argv[i];
          DosSuspendThread(reapthread);
          sprintf(userenv,"LOGNAME=%s",sep->se_user);
          putenv(userenv);

          if ((wdir = getenv("ETC")) == NULL) wdir = "c:\\etc";
          pass = getpwnam(sep->se_user);
          if (pass && pass->pw_dir) wdir = pass->pw_dir;

          chdir(wdir);
          if (wdir[1] == ':')
            _chdrive(toupper(wdir[0])-'A'+1);

          pid=spawnvp(P_NOWAIT, sep->se_server, argv);
          if (pid) {
            removesocketfromlist(sep->se_sock);
            DosPostEventSem(reapsem);
          }
          if (pid && sep->se_wait) {
            sep->se_wait = pid;
            if (sep->se_fd >=0) {
              fd_clr(sep->se_fd, allsock, &nsock);
            }
          }
          DosResumeThread(reapthread);
        }
      }
    }
  }
}

int     fd_isset(int sock, int arr[], int nsock)
{
  int i=0;

  while((i < nsock) && (arr[i]!=sock) && (i<MAXSOCK)) i++;
  if (i==MAXSOCK) return 0;
  if (i== nsock) return 0;
  return 1;
}

void    fd_set(int sock, int arr[], int *nsock)
{
  int i=0;

  if (*nsock>=MAXSOCK) return;
  while((i < *nsock) && (arr[i]!=sock)) i++;
  if (i!= *nsock) return;
  arr[*nsock]=sock;  
  (*nsock)++;
}

void    fd_clr(int sock, int arr[], int *nsock)
{
  int i=0;

  if (*nsock==0) return;
  while((i < *nsock) && (arr[i]!=sock) && (i<256)) i++;
  if (i==256) return;
  if (i == *nsock) return;
  (*nsock)--;
  while (i< *nsock)
  {
    arr[i]=arr[i+1];
    i++;
  }  
}  

void reapchild(void *dummy)
{
  ULONG pid;
  ULONG cnt;
  RESULTCODES result;
  register struct servtab *sep;

  while(1) {
    cnt = DosWaitChild(DCWA_PROCESS, DCWW_WAIT, &result, &pid, 0);
    if (cnt) {
      if (cnt == ERROR_WAIT_NO_CHILDREN) {
        DosWaitEventSem(reapsem, -1L);
        DosResetEventSem(reapsem,&cnt);
      } else syslog(LOG_WARNING, "wait result 0x%x", errno);
    } else {
      if (debug)
        fprintf(stderr, "%d reaped\n", pid);
      for (sep = servtab; sep; sep = sep->se_next)
        if (sep->se_wait == pid) {
          if (result.codeResult)
            syslog(LOG_WARNING,
              "%s: exit status 0x%x",
              sep->se_server, result.codeResult);
          if (result.codeTerminate)
            syslog(LOG_WARNING,
              "%s: exit termination 0x%x",
              sep->se_server, result.codeTerminate);
          if (debug)
            fprintf(stderr, "restored %s, fd %d\n",
              sep->se_service, sep->se_fd);
          fd_set(sep->se_fd,allsock, &nsock);
          sep->se_wait = 1;
          if (sep->se_socktype==SOCK_STREAM) {
            so_cancel(sep->se_sock);
            soclose(sep->se_sock);
          }
          so_cancel(cansock);
        }
    }
  }
}

void config(int x)
{
  register struct servtab *sep, *cp, **sepp;

  if (!setconfig()) {
    syslog(LOG_ERR, "%s: %m", CONFIG);
    return;
  }
  for (sep = servtab; sep; sep = sep->se_next)
    sep->se_checked = 0;
  while (cp = getconfigent()) {
    for (sep = servtab; sep; sep = sep->se_next)
      if (strcmp(sep->se_service, cp->se_service) == 0 &&
        strcmp(sep->se_proto, cp->se_proto) == 0 &&
        (sep->se_socktype==cp->se_socktype))
        break;
    if (sep != 0) {
      int i;

      /*
       * sep->se_wait may be holding the pid of a daemon
       * that we're waiting for.  If so, don't overwrite
       * it unless the config file explicitly says don't 
       * wait.
       */
      if (cp->se_bi == 0 && 
        (sep->se_wait == 1 || cp->se_wait == 0))
        sep->se_wait = cp->se_wait;
#define SWAP(a, b) { char *c = a; a = b; b = c; }
      if (cp->se_user)
        SWAP(sep->se_user, cp->se_user);
      if (cp->se_server)
        SWAP(sep->se_server, cp->se_server);
      for (i = 0; i < MAXARGV; i++)
        SWAP(sep->se_argv[i], cp->se_argv[i]);
      freeconfig(cp);
      if (debug)
        print_service("REDO", sep);
    } else {
      sep = enter(cp);
      if (debug)
        print_service("ADD ", sep);
    }
    sep->se_checked = 1;
    sp = getservbyname(sep->se_service, sep->se_proto);
    if (sp == 0) {
      syslog(LOG_ERR, "%s/%s: unknown service",
        sep->se_service, sep->se_proto);
      if (sep->se_fd != -1)
        soclose(sep->se_fd);
      sep->se_fd = -1;
      continue;
    }
    if (sp->s_port != sep->se_ctrladdr.sin_port) {
      bzero(&sep->se_ctrladdr, sizeof(struct sockaddr_in));
      sep->se_ctrladdr.sin_family = AF_INET;
      sep->se_ctrladdr.sin_port = sp->s_port;
      if (sep->se_fd != -1)
        soclose(sep->se_fd);
      sep->se_fd = -1;
    }
    if (sep->se_fd == -1)
      setup(sep);
  }
  endconfig();
  /*
   * Purge anything not looked at above.
   */
  sepp = &servtab;
  while (sep = *sepp) {
    if (sep->se_checked) {
      sepp = &sep->se_next;
      continue;
    }
    *sepp = sep->se_next;
    if (sep->se_fd != -1) {
      fd_clr(sep->se_fd,allsock, &nsock);
      soclose(sep->se_fd);
    }
    if (debug)
      print_service("FREE", sep);
    freeconfig(sep);
    free((char *)sep);
  }
  signal(SIGINT, config);
}

void retry(void *time)
{
  register struct servtab *sep;

  DosSleep( (* (int*) time) * 1000);
  for (sep = servtab; sep; sep = sep->se_next)
    if (sep->se_fd == -1)
      setup(sep);
  timingout = 0;
}

void setup(struct servtab *sep)
{
  int on = 1;

  if ((sep->se_fd = socket(AF_INET, sep->se_socktype, 0)) < 0) {
    errno = sock_errno();
    syslog(LOG_ERR, "%s/%s: socket: %m",
    sep->se_service, sep->se_proto);
    return;
  }
#define turnon(fd, opt) \
setsockopt(fd, SOL_SOCKET, opt, (char *)&on, sizeof (on))
  if (strcmp(sep->se_proto, "tcp") == 0 && (options & SO_DEBUG) &&
    turnon(sep->se_fd, SO_DEBUG) < 0)
    syslog(LOG_ERR, "setsockopt (SO_DEBUG): %m");
  if (turnon(sep->se_fd, SO_REUSEADDR) < 0)
    syslog(LOG_ERR, "setsockopt (SO_REUSEADDR): %m");
#undef turnon
  if (bind(sep->se_fd, (struct sockaddr *)&sep->se_ctrladdr,
    sizeof (sep->se_ctrladdr)) < 0) {
    errno = sock_errno();
    syslog(LOG_ERR, "%s/%s: bind: %m",
    sep->se_service, sep->se_proto);
    soclose(sep->se_fd);
    sep->se_fd = -1;
    if (!timingout) {
      timingout = 1;
      _beginthread(retry,NULL,8192,&retrytime);
    }
    return;
  }
  if (sep->se_socktype == SOCK_STREAM)
    listen(sep->se_fd, 10);
  fd_set(sep->se_fd, allsock, &nsock);
    if (sep->se_fd > maxsock)
  maxsock = sep->se_fd;
}

struct servtab *enter(struct servtab *cp)
{
  register struct servtab *sep;

  sep = (struct servtab *)malloc(sizeof (*sep));
  if (sep == (struct servtab *)0) {
    syslog(LOG_ERR, "Out of memory.");
    exit(-1);
  }
  *sep = *cp;
  sep->se_fd = -1;
  sep->se_next = servtab;
  servtab = sep;
  return (sep);
}

FILE    *fconfig = NULL;
struct  servtab serv;
char    line[256];

int setconfig(void)
{

  if (fconfig != NULL) {
    fseek(fconfig, 0L, 0);
    return (1);
  }
  fconfig = fopen(CONFIG, "r");
  return (fconfig != NULL);
}

void endconfig(void)
{
  if (fconfig) {
    (void) fclose(fconfig);
    fconfig = NULL;
  }
}

struct servtab * getconfigent(void)
{
  register struct servtab *sep = &serv;
  int argc;
  char *cp, *arg, *newstr();

more:
  while ((cp = nextline(fconfig)) && *cp == '#')
    ;
  if (cp == NULL)
    return ((struct servtab *)0);
  sep->se_service = newstr(skip(&cp));
  arg = newstr(skip(&cp));
  if (strcmp(arg, "stream") == 0)
    sep->se_socktype = SOCK_STREAM;
  else if (strcmp(arg, "dgram") == 0)
    sep->se_socktype = SOCK_DGRAM;
  else
    sep->se_socktype = -1;
  free(arg);
  sep->se_proto = newstr(skip(&cp));
  arg = newstr(skip(&cp));
  sep->se_wait = strcmp(arg, "wait") == 0;
  free(arg);
  sep->se_user = newstr(skip(&cp));
  sep->se_server = newstr(skip(&cp));
  if (strcmp(sep->se_server, "internal") == 0) {
    register struct biltin *bi;

    for (bi = biltins; bi->bi_service; bi++)
      if (bi->bi_socktype == sep->se_socktype &&
        strcmp(bi->bi_service, sep->se_service) == 0)
        break;
    if (bi->bi_service == 0) {
      syslog(LOG_ERR, "internal service %s unknown\n",
        sep->se_service);
      goto more;
    }
    sep->se_bi = bi;
    sep->se_wait = bi->bi_wait;
  } else
    sep->se_bi = NULL;

  sep->se_argv[0]=newstr(sep->se_server);
  sep->se_argv[1]=newstr("            ");
  argc = 1;
  for (arg = skip(&cp); cp; arg = skip(&cp))
    if (argc < MAXARGV)
      sep->se_argv[argc++] = newstr(arg);
  while (argc <= MAXARGV)
    sep->se_argv[argc++] = NULL;
  return (sep);
}

void freeconfig(struct servtab *cp)
{
  int i;

  if (cp->se_service)
    free(cp->se_service);
  if (cp->se_proto)
    free(cp->se_proto);
  if (cp->se_user)
    free(cp->se_user);
  if (cp->se_server)
    free(cp->se_server);
  for (i = 0; i < MAXARGV; i++)
    if (cp->se_argv[i])
      free(cp->se_argv[i]);
}

char * skip(char **cpp)
{
  register char *cp = *cpp;
  char *start;

  if (cp==(char *) 0) return cp;
again:
  while (*cp == ' ' || *cp == '\t')
    cp++;
  if (*cp == '\0') {
    int c;

    c = getc(fconfig);
    (void) ungetc(c, fconfig);
    if (c == ' ' || c == '\t')
      if (cp = nextline(fconfig))
        goto again;
    *cpp = (char *)0;
    return ((char *)0);
  }
  start = cp;
  while (*cp && *cp != ' ' && *cp != '\t')
    cp++;
  if (*cp != '\0')
    *cp++ = '\0';
  *cpp = cp;
  return (start);
}

char * nextline(FILE *fd)
{
  char *cp;

  if (fgets(line, sizeof (line), fd) == NULL)
    return ((char *)0);
  cp = index(line, '\n');
  if (cp)
    *cp = '\0';
  return (line);
}

char * newstr(char *cp)
{
  if (cp = strdup(cp ? cp : ""))
    return(cp);
  syslog(LOG_ERR, "strdup: %m");
  exit(-1);
}

/*
 * Internet services provided internally by inetd:
 */
#define BUFSIZE 8192

void echo_stream(int *s) /* Echo service -- echo data back */
{
  char buffer[BUFSIZE];
  int i;

  while ((i = recv(*s, buffer, sizeof(buffer),0)) > 0 &&
    send(*s, buffer, i, 0) > 0)
    ;
  so_cancel(*s);
  soclose(*s);
  free(s);
}

void echo_dg(int *s) /* Echo service -- echo data back */
{
  char buffer[BUFSIZE];
  int i, size;
  struct sockaddr sa;

  size = sizeof(sa);
  if ((i = recvfrom(*s, buffer, sizeof(buffer), 0, &sa, &size)) < 0)
    return;
  sendto(*s, buffer, i, 0, &sa, sizeof(sa));
  free(s);
}

void discard_stream(int *s) /* Discard service -- ignore data */
{
  int ret;
  char buffer[BUFSIZE];

  while (1) {
    while ((ret = recv(*s, buffer, sizeof(buffer),0)) > 0)
      ;
    errno = sock_errno();
    if (ret == 0 || errno != SOCEINTR)
      break;
  }
  so_cancel(*s);
  soclose(*s);
  free(s);
}

void discard_dg(int *s) /* Discard service -- ignore data */
{
  char buffer[BUFSIZE];

  recv(*s, buffer, sizeof(buffer),0);
  free(s);
}

#include <ctype.h>
#define LINESIZ 72
char ring[128];
char *endring;

void initring(void)
{
  register int i;

  endring = ring;

  for (i = 0; i <= 128; ++i)
    if (isprint(i))
      *endring++ = i;
}

void chargen_stream(int *s) /* Character generator */
{
  register char *rs;
  int len;
  char text[LINESIZ+2];

  if (!endring) {
    initring();
    rs = ring;
  }

  text[LINESIZ] = '\r';
  text[LINESIZ + 1] = '\n';
  for (rs = ring;;) {
    if ((len = endring - rs) >= LINESIZ)
      bcopy(rs, text, LINESIZ);
    else {
      bcopy(rs, text, len);
      bcopy(ring, text + len, LINESIZ - len);
    }
    if (++rs == endring)
      rs = ring;
    if (send(*s, text, sizeof(text),0) != sizeof(text))
      break;
  }
  so_cancel(*s);
  soclose(*s);
  free(s);
}

void chargen_dg(int *s) /* Character generator */
{
  struct sockaddr sa;
  static char *rs;
  int len, size;
  char text[LINESIZ+2];

  if (endring == 0) {
    initring();
    rs = ring;
  }

  size = sizeof(sa);
  if (recvfrom(*s, text, sizeof(text), 0, &sa, &size) < 0)
    return;

  if ((len = endring - rs) >= LINESIZ)
    bcopy(rs, text, LINESIZ);
  else {
    bcopy(rs, text, len);
    bcopy(ring, text + len, LINESIZ - len);
  }
  if (++rs == endring)
    rs = ring;
  text[LINESIZ] = '\r';
  text[LINESIZ + 1] = '\n';
  sendto(*s, text, sizeof(text), 0, &sa, sizeof(sa));
  free(s);
}

/*
 * Return a machine readable date and time, in the form of the
 * number of seconds since midnight, Jan 1, 1900.  Since gettimeofday
 * returns the number of seconds since midnight, Jan 1, 1970,
 * we must add 2208988800 seconds to this figure to make up for
 * some seventy years Bell Labs was asleep.
 */

long machtime(void)
{
  struct timeval tv;

  if (gettimeofday(&tv, (struct timezone *)0) < 0) {
    syslog(LOG_ERR, "Unable to get time of day");
    return (0L);
  }
  return (htonl((long)tv.tv_sec + 2208988800));
}

void machtime_stream(int *s)
{
  long result;

  result = machtime();
  send(*s, (char *) &result, sizeof(result),0);
  so_cancel(*s);
  soclose(*s);
  free(s);
}

void machtime_dg(int *s)
{
  long result;
  struct sockaddr sa;
  int size;

  size = sizeof(sa);
  if (recvfrom(*s, (char *)&result, sizeof(result), 0, &sa, &size) < 0)
    return;
  result = machtime();
  sendto(*s, (char *) &result, sizeof(result), 0, &sa, sizeof(sa));
  free(s);
}

void daytime_stream(int *s) /* Return human-readable time of day */
{
  char buffer[256];
  time_t time(), clock;
  char *ctime();

  clock = time((time_t *) 0);

  sprintf(buffer, "%.24s\r\n", ctime(&clock));
  send(*s, buffer, strlen(buffer),0);
  so_cancel(*s);
  soclose(*s);
  free(s);
}

void daytime_dg(int *s) /* Return human-readable time of day */
{
  char buffer[256];
  time_t time(), clock;
  struct sockaddr sa;
  int size;
  char *ctime();

  clock = time((time_t *) 0);

  size = sizeof(sa);
  if (recvfrom(*s, buffer, sizeof(buffer), 0, &sa, &size) < 0)
    return;
  sprintf(buffer, "%.24s\r\n", ctime(&clock));
  sendto(*s, buffer, strlen(buffer), 0, &sa, sizeof(sa));
  free(s);
}

/*
 * print_service:
 *      Dump relevant information to stderr
 */
void print_service(char *action, struct servtab *sep)
{
  fprintf(stderr,
    "%s: %s proto=%s, wait=%d, user=%s builtin=%x server=%s\n",
    action, sep->se_service, sep->se_proto,
    sep->se_wait, sep->se_user, (int)sep->se_bi, sep->se_server);
}
