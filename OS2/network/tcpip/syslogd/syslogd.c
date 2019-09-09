/*  
**  Modified by Jochen Friedrich <jochen@audio.pfalz.de> for OS/2.
*/
/*  $Revision: 1.3 $
**  This file has been modified to get it to compile more easily
**  on pre-4.4BSD (e.g., SysVr4 :-) systems.  Rich $alz, June 1991.
*/

#define WAITVALUE       int

/*
 * Copyright (c) 1983, 1988 Regents of the University of California.
 * All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
 *
 * Wed Sep 14 21:56:59 1994: Applied patches from Alan Modra
 * (alan@spri.levels.unisa.edu.au):
 * 1) Add O_CREAT to open flags so that syslogd doesn't complain about
 *    non-existent files
 * 2) Modified f_pmask initialisation and testing to allow logging of
 *    messages at a particular priority level, rather that all messages
 *    at or above a given priority level.
 *
 * Sat Jun 3 12:48:16 1995: Applied patches from
 * Jochen.Hein@informatik.tu-clausthal.de to allow spaces *AND* tabs to
 * separate the selector from the action. This means that no whitespace is
 * allowed inside the selector.
 * */

/*
 *  syslogd -- log system messages
 *
 * This program implements a system log. It takes a series of lines.
 * Each line may have a priority, signified as "<n>" as
 * the first characters of the line.  If this is
 * not present, a default priority is used.
 *
 * To kill syslogd, send a signal 15 (terminate).  A signal 1 (hup) will
 * cause it to reread its configuration file.
 *
 * Defined Constants:
 *
 * MAXLINE -- the maximimum line length that can be handled.
 * DEFUPRI -- the default priority for user messages
 * DEFSPRI -- the default priority for kernel messages
 *
 * Author: Eric Allman
 * extensive changes by Ralph Campbell
 * more extensive changes by Eric Allman (again)
 */

#define MAXLINE         1024            /* maximum line length */
#define MAXSVLINE       120             /* maximum saved line length */
#define DEFUPRI         (LOG_USER|LOG_NOTICE)
#define DEFSPRI         (LOG_KERN|LOG_CRIT)
#define TIMERINTVL      30              /* interval for checking flush, mark */

#define INCL_DOS
#include <os2.h>

#include <errno.h>
#include <signal.h>
#include <process.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <types.h>
#include "getopt.h"
#include "fix_env.h"

#define MSG_BSIZE       4096


#define SYSLOG_NAMES
#include "syslog.h"

#define UT_NAMESIZE     8

char    ConfFile[512]="c:\\etc\\syslog.cnf";
char    PidFile[512]="c:\\etc\\syslog.pid";

#define dprintf         if (Debug) printf

#define MAXUNAMES       20      /* maximum number of user names */

#include <fcntl.h>

void setlinebuf(FILE *f)
{
    setbuf(f, (char *)NULL);
}

int our_writev(int fd, struct iovec iov[], int vcount)
{
  int i;
  for (i=0;i<vcount ;i++) 
  {
    write(fd,iov[i].iov_base,iov[i].iov_len);
  } /* endfor */
}

/*
 * Flags to logmsg().
 */

#define IGN_CONS        0x001   /* don't print on console */
#define SYNC_FILE       0x002   /* do fsync on file after printing */
#define ADDDATE         0x004   /* add a date to the message */
#define MARK            0x008   /* this message is a mark */

/*
 * This structure represents the files that will have log
 * copies printed.
 */

struct filed {
        struct  filed *f_nextone;       /* next in linked list */
        short   f_type;                 /* entry type, see below */
        short   f_file;                 /* file descriptor */
        time_t  f_time;                 /* time this was last written */
        char    f_pmask[LOG_NFACILITIES+1];     /* priority mask */
        union {
                char    f_uname[MAXUNAMES][UT_NAMESIZE+1];
                struct {
                        char    f_hname[MAXHOSTNAMELEN+1];
                        struct sockaddr_in      f_addr;
                } f_forw;               /* forwarding address */
                char    f_fname[MAXPATHLEN];
        } f_un;
        char    f_prevline[MAXSVLINE];          /* last message logged */
        char    f_lasttime[16];                 /* time of last occurrence */
        char    f_prevhost[MAXHOSTNAMELEN+1];   /* host from which recd. */
        int     f_prevpri;                      /* pri of f_prevline */
        int     f_prevlen;                      /* length of f_prevline */
        int     f_prevcount;                    /* repetition cnt of prevline */
        int     f_repeatcount;                  /* number of "repeated" msgs */
};

/*
 * Intervals at which we flush out "message repeated" messages,
 * in seconds after previous message is logged.  After each flush,
 * we move to the next interval until we reach the largest.
 */
int     repeatinterval[] = { 30, 120, 600 };    /* # of secs before flush */
#define MAXREPEAT ((sizeof(repeatinterval) / sizeof(repeatinterval[0])) - 1)
#define REPEATTIME(f)   ((f)->f_time + repeatinterval[(f)->f_repeatcount])
#define BACKOFF(f)      { if (++(f)->f_repeatcount > MAXREPEAT) \
                                 (f)->f_repeatcount = MAXREPEAT; \
                        }

/* values for f_type */
#define F_UNUSED        0               /* unused entry */
#define F_FILE          1               /* regular file */
#define F_TTY           2               /* terminal */
#define F_CONSOLE       3               /* console terminal */
#define F_FORW          4               /* remote machine */

char    *TypeNames[5] = {
        "UNUSED",       "FILE",         "TTY",          "CONSOLE",
        "FORW"
};

struct  filed *Files;
struct  filed consfile;

int     Debug;                  /* debug flag */
char    LocalHostName[MAXHOSTNAMELEN+1];        /* our hostname */
char    *LocalDomain;           /* our local domain name */
int     InetInuse = 0;          /* non-zero if INET sockets are being used */
int     finet;                  /* Internet datagram socket */
int     LogPort;                /* port number for INET connections */
int     Initialized = 0;        /* set when we have initialized ourselves */
int     MarkInterval = 20 * 60; /* interval between marks in seconds */
int     MarkSeq = 0;            /* mark sequence number */

void init(int);
void die(int);
void domark(void *v);
void usage(void);
void logerror(char *);

int main(int argc, char **argv)
{
        register int i;
        register char *p;
        int len;
        struct sockaddr_in sin, frominet;
        FILE *fp;
        int ch;
        char line[MSG_BSIZE + 1];
        extern int optind;
        extern char *optarg;

        fix_env(PidFile,"ETC");
        fix_env(ConfFile,"ETC");
 
        if (sock_init()) exit(-1);
 
        while ((ch = getopt(argc, argv, "df:m:")) != EOF)
                switch((char)ch) {
                case 'd':               /* debug */
                        Debug++;
                        break;
                case 'f':               /* configuration file */
                        strcpy(ConfFile,optarg);
                        break;
                case 'm':               /* mark interval */
                        MarkInterval = atoi(optarg) * 60;
                        break;
                case '?':
                default:
                        usage();
                }
        if (argc -= optind)
                usage();

        setlinebuf(stdout);

        consfile.f_type = F_CONSOLE;
        (void) strcpy(consfile.f_un.f_fname, "CON");
        strcpy(LocalHostName,getenv("HOSTNAME"));
        if (p = strchr(LocalHostName, '.')) {
                *p++ = '\0';
                LocalDomain = p;
        }
        else
                LocalDomain = "";
        (void) signal(SIGTERM, die);

        finet = socket(AF_INET, SOCK_DGRAM, 0);
        if (finet >= 0) {
                struct servent * sp;

                sp = getservbyname("syslog", "udp");
                if (sp == NULL) {
                        errno = 0;
                        logerror("syslog/udp: unknown service");
                        die(0);
                }
                bzero((char *)&sin, sizeof(sin)); /* added uunet!rbj */
                sin.sin_family = AF_INET;
                sin.sin_addr.s_addr = INADDR_ANY;
                sin.sin_port = LogPort = htons(sp->s_port);
                if (bind(finet, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
                        logerror("bind");
                        if (!Debug)
                                die(0);
                } else {
                        InetInuse = 1;
                }
        }

        /* tuck my process id away */
        fp = fopen(PidFile, "w");
        if (fp != NULL) {
                fprintf(fp, "%d\n", getpid());
                (void) fclose(fp);
        }

        i= _beginthread(domark, NULL, 8192, NULL);

        dprintf("thread 2 started TID=%d...\n",i);

        dprintf("off & running....\n");

        init(0);

        for (;;) {
                errno = 0;
                len = sizeof frominet;
                i = recvfrom(finet, line, MAXLINE, 0,
                    (struct sockaddr * ) &frominet, &len);
                if (i > 0) {
                        extern char *cvthname();

                        line[i] = '\0';
                        printmulti(cvthname(&frominet), line,i);
		} else if (i < 0 && errno != EINTR)
			logerror("recvfrom inet");
        }
}

void usage(void)
{
        (void) fprintf(stderr,
            "usage: syslogd [-d] [-f conffile] [-m markinterval]\n");
        exit(1);
}

/*
 * Break up null terminated lines for printline.
 */

printmulti(char *hname, char *msg, int len)
{
        int  i;
        char *pt;

        dprintf("strlen = %d, len = %d\n", strlen(msg), len );
        for (pt = msg, i = 0; i <= len; i++) {
                if (msg[i] == '\0') {
                        printline(hname,pt);
                        pt = &msg[i+1];
                }
        }
}

/*
 * Take a raw input line, decode the message, and print the message
 * on the appropriate log files.
 */

printline(char *hname, char *msg)
{
        register char *p, *q;
        register int c;
        char line[MAXLINE + 1];
        int pri;

        /* test for special codes */
        pri = DEFUPRI;
        p = msg;
        if (*p == '<') {
                pri = 0;
                while (isdigit(*++p))
                        pri = 10 * pri + (*p - '0');
                if (*p == '>')
                        ++p;
        }
        if (pri &~ (LOG_FACMASK|LOG_PRIMASK))
                pri = DEFUPRI;

        /* don't allow users to log kernel messages */
        if (LOG_FAC(pri) == LOG_KERN)
                pri = LOG_MAKEPRI(LOG_USER, LOG_PRI(pri));

        q = line;

        while ((c = *p++ & 0177) != '\0' &&
            q < &line[sizeof(line) - 1])
                if (iscntrl(c))
                        if (c == '\n')
                                *q++ = ' ';
                        else if (c == '\r')
                                *q++ = ' ';
                        else if (c == '\t')
                                *q++ = '\t';
                        else {
                                *q++ = '^';
                                *q++ = c ^ 0100;
                        }
                else
                        *q++ = c;
        *q = '\0';

        logmsg(pri, line, hname, 0);
}

time_t  now;

/*
 * Log a message to the appropriate log files, users, etc. based on
 * the priority.
 */

logmsg(int pri, char *msg, char *from, int flags)
{
        register struct filed *f;
        int fac, prilev;
        int omask, msglen;
        char *timestamp;
        time_t time();

        dprintf("logmsg: pri %o, flags %x, from %s, msg %s\n", pri, flags, from, msg);

        /*
         * Check to see if msg looks non-standard.
         */
        msglen = strlen(msg);
        if (msglen < 16 || msg[3] != ' ' || msg[6] != ' ' ||
            msg[9] != ':' || msg[12] != ':' || msg[15] != ' ')
                flags |= ADDDATE;

        (void) time(&now);
        if (flags & ADDDATE)
                timestamp = (char*)((int)ctime(&now) + 4);
        else {
                timestamp = msg;
                msg += 16;
                msglen -= 16;
        }

        /* extract facility and priority level */
        if (flags & MARK)
                fac = LOG_NFACILITIES;
        else
                fac = LOG_FAC(pri);
        prilev = 1 << (LOG_PRIMASK - LOG_PRI(pri));

        /* log the message to the particular outputs */
        if (!Initialized) {
                f = &consfile;
                f->f_file = _open("CON", O_WRONLY, 0);

                if (f->f_file >= 0) {
                        fprintlog(f, flags, msg);
                        (void) _close(f->f_file);
                }
                return;
        }
        for (f = Files; f; f = f->f_nextone) {
                /* skip messages that are incorrect priority */
                if (!(f->f_pmask[fac] & prilev))
                        continue;

                if (f->f_type == F_CONSOLE && (flags & IGN_CONS))
                        continue;

                /* don't output marks to recently written files */
                if ((flags & MARK) && (now - f->f_time) < MarkInterval / 2)
                        continue;

                /*
                 * suppress duplicate lines to this file
                 */
                if ((flags & MARK) == 0 && msglen == f->f_prevlen &&
                    !strcmp(msg, f->f_prevline) &&
                    !strcmp(from, f->f_prevhost)) {
                        (void) strncpy(f->f_lasttime, timestamp, 15);
                        f->f_prevcount++;
                        dprintf("msg repeated %d times, %ld sec of %d\n",
                            f->f_prevcount, now - f->f_time,
                            repeatinterval[f->f_repeatcount]);
                        /*
                         * If domark would have logged this by now,
                         * flush it now (so we don't hold isolated messages),
                         * but back off so we'll flush less often
                         * in the future.
                         */
                        if (now > REPEATTIME(f)) {
                                fprintlog(f, flags, (char *)NULL);
                                BACKOFF(f);
                        }
                } else {
                        /* new line, save it */
                        if (f->f_prevcount)
                                fprintlog(f, 0, (char *)NULL);
                        f->f_repeatcount = 0;
                        (void) strncpy(f->f_lasttime, timestamp, 15);
                        (void) strncpy(f->f_prevhost, from,
                                        sizeof(f->f_prevhost));
                        if (msglen < MAXSVLINE) {
                                f->f_prevlen = msglen;
                                f->f_prevpri = pri;
                                (void) strcpy(f->f_prevline, msg);
                                fprintlog(f, flags, (char *)NULL);
                        } else {
                                f->f_prevline[0] = 0;
                                f->f_prevlen = 0;
                                fprintlog(f, flags, msg);
                        }
                }
        }
}

fprintlog(struct filed *f, int flags, char *msg)
{
        struct iovec iov[6];
        register struct iovec *v;
        register int l;
        char line[MAXLINE + 1], repbuf[80], greetings[200];

        v = iov;
        v->iov_base = f->f_lasttime;
        v->iov_len = 15;
        v++;
        v->iov_base = " ";
        v->iov_len = 1;
        v++;
        v->iov_base = f->f_prevhost;
        v->iov_len = strlen(v->iov_base);
        v++;
        v->iov_base = " ";
        v->iov_len = 1;
        v++;

        if (msg) {
                v->iov_base = msg;
                v->iov_len = strlen(msg);
        } else if (f->f_prevcount > 1) {
                v->iov_base = repbuf;
                sprintf(repbuf, "last message repeated %d times",
                    f->f_prevcount);
                v->iov_len = strlen(repbuf);
        } else {
                v->iov_base = f->f_prevline;
                v->iov_len = f->f_prevlen;
        }
        v++;

        dprintf("Logging to %s", TypeNames[f->f_type]);
        f->f_time = now;

        switch (f->f_type) {
        case F_UNUSED:
                dprintf("\n");
                break;

        case F_FORW:
                dprintf(" %s\n", f->f_un.f_forw.f_hname);
                l = sprintf(line, "<%d>%.15s %s", f->f_prevpri,
                    (char *)iov[0].iov_base, 
                    (char *)iov[4].iov_base);
                if (l > MAXLINE)
                        l = MAXLINE;
                if (errno=sendto(finet, line, l, 0, (struct sockaddr*)(&f->f_un.f_forw.f_addr),
                    sizeof f->f_un.f_forw.f_addr) != l) {
                        int e = errno;
                        (void) soclose(f->f_file);
                        f->f_type = F_UNUSED;
                        errno = e;
                        logerror("sendto");
                }
                break;

        case F_CONSOLE:
                if (flags & IGN_CONS) {
                        dprintf(" (ignored)\n");
                        break;
                }
                /* FALLTHROUGH */

        case F_TTY:
        case F_FILE:
                dprintf(" %s\n", f->f_un.f_fname);
                if (f->f_type != F_FILE) {
                        v->iov_base = "\r\n";
                        v->iov_len = 2;
                } else {
                        v->iov_base = "\n";
                        v->iov_len = 1;
                }
        again:
                if (our_writev(f->f_file, iov, 6) < 0) {
                        int e = errno;
                        (void) _close(f->f_file);
                        /*
                         * Check for errors on TTY's due to loss of tty
                         */
                        f->f_type = F_UNUSED;
                        errno = e;
                        logerror(f->f_un.f_fname);
                }
/*                fsync(f->f_file); */
                break;

        }
        f->f_prevcount = 0;
}

/*
 * Return a printable representation of a host address.
 */
char * cvthname(struct sockaddr_in *f)
{
        struct hostent * hp;
        register char *p;

        dprintf("cvthname(%s)\n", (char *) inet_ntoa(f->sin_addr));

        if (f->sin_family != AF_INET) {
                dprintf("Malformed from address\n");
                return ("???");
        }
        hp = gethostbyaddr((char *)&(f->sin_addr), sizeof(struct in_addr), f->sin_family);
        if (hp == 0) {
                dprintf("Host name for your address (%s) unknown\n",
                        (char *) inet_ntoa(f->sin_addr));
                return (inet_ntoa(f->sin_addr));
        }
        if ((p = strchr(hp->h_name, '.')) && strcmp(p + 1, LocalDomain) == 0)
                *p = '\0';
        return (hp->h_name);
}

void domark(void *v)
{
        for (;;) {
                register struct filed *f;

                DosSleep(TIMERINTVL*1000);

                now = time((time_t *)NULL);
                MarkSeq += TIMERINTVL;
                if (MarkSeq >= MarkInterval) {
                        logmsg(LOG_INFO, "-- MARK --", LocalHostName, ADDDATE|MARK);
                        MarkSeq = 0;
                }

                for (f = Files; f; f = f->f_nextone) {
                        if (f->f_prevcount && now >= REPEATTIME(f)) {
                                dprintf("flush %s: repeated %d times, %d sec.\n",
                                    TypeNames[f->f_type], f->f_prevcount,
                                    repeatinterval[f->f_repeatcount]);
                                fprintlog(f, 0, (char *)NULL);
                                BACKOFF(f);
                        }
                }
        } /* endfor */
}

/*
 * Print syslogd errors some place.
 */
void logerror(char *type)
{
        char buf[100];

        if (errno)
                (void) sprintf(buf, "syslogd: %s: %s", type, strerror(errno));
        else
                (void) sprintf(buf, "syslogd: %s", type);
        errno = 0;
        dprintf("%s\n", buf);
        logmsg(LOG_SYSLOG|LOG_ERR, buf, LocalHostName, ADDDATE);
}

void die(int sig)
{
        register struct filed *f;
        char buf[100];

        for (f = Files; f != NULL; f = f->f_nextone) {
                /* flush any pending output */
                if (f->f_prevcount)
                        fprintlog(f, 0, (char *)NULL);
        }
        if (sig) {
                dprintf("syslogd: exiting on signal %d\n", sig);
                (void) sprintf(buf, "exiting on signal %d", sig);
                errno = 0;
                logerror(buf);
        }
        exit(0);
}

/*
 *  INIT -- Initialize syslogd from configuration table
 */

void init(int x)
{
        register int i;
        register FILE *cf;
        register struct filed *f, *next, **nextp;
        register char *p;
        char cline[BUFSIZ];

        dprintf("init\n");

        /*
         *  Close all open log files.
         */
        Initialized = 0;
        for (f = Files; f != NULL; f = next) {
                /* flush any pending output */
                if (f->f_prevcount)
                        fprintlog(f, 0, (char *)NULL);

                switch (f->f_type) {
                  case F_FILE:
                  case F_TTY:
                  case F_CONSOLE:
                        (void) _close(f->f_file);
                        break;
                  case F_FORW:
                        soclose(f->f_file);
                        break;
                }
                next = f->f_nextone;
                free((char *) f);
        }
        Files = NULL;
        nextp = &Files;

        /* open the configuration file */
        if ((cf = fopen(ConfFile, "r")) == NULL) {
                dprintf("cannot open %s\n", ConfFile);
                *nextp = (struct filed *)malloc(sizeof(*f));
                cfline("*.ERR\tCON", *nextp);
                (*nextp)->f_nextone = (struct filed *)malloc(sizeof(*f));
                cfline("*.PANIC\t*", (*nextp)->f_nextone);
                Initialized = 1;
                return;
        }

        /*
         *  Foreach line in the conf table, open that file.
         */
        f = NULL;
        while (fgets(cline, sizeof cline, cf) != NULL) {
                /*
                 * check for end-of-section, comments, strip off trailing
                 * spaces and newline character.
                 */
                for (p = cline; isspace(*p); ++p);
                if (*p == '\0' || *p == '#')
                        continue;
                for (p = strchr(cline, '\0'); isspace(*--p););
                *++p = '\0';
                f = (struct filed *)malloc(sizeof(*f));
                *nextp = f;
                nextp = &f->f_nextone;
                cfline(cline, f);
        }

        /* close the configuration file */
        (void) fclose(cf);

        Initialized = 1;

        if (Debug) {
                for (f = Files; f; f = f->f_nextone) {
                        for (i = 0; i <= LOG_NFACILITIES; i++)
                                if (f->f_pmask[i] == 0)
                                        printf("X ");
                                else
                                        printf("0x%02x ", f->f_pmask[i]);
                        printf("%s: ", TypeNames[f->f_type]);
                        switch (f->f_type) {
                        case F_FILE:
                        case F_TTY:
                        case F_CONSOLE:
                                printf("%s", f->f_un.f_fname);
                                break;

                        case F_FORW:
                                printf("%s", f->f_un.f_forw.f_hname);
                                break;

                        }
                        printf("\n");
                }
        }

        if (Debug) {
            (void) signal(SIGINT, die);
        }
        else {
            (void) signal(SIGINT, init);
        }


        logmsg(LOG_SYSLOG|LOG_INFO, "syslogd: restart", LocalHostName, ADDDATE);
        dprintf("syslogd: restarted\n");
}

/*
 * Crack a configuration file line
 */

cfline(line, f)
        char *line;
        register struct filed *f;
{
        register char *p;
        register char *q;
        register int i;
        char *bp;
        int pri;
        struct hostent *hp;
        char buf[MAXLINE], ebuf[100];

        dprintf("cfline(%s)\n", line);

        errno = 0;      /* keep strerror() stuff out of logerror messages */

        /* clear out file entry */
        bzero((char *) f, sizeof *f);
        for (i = 0; i <= LOG_NFACILITIES; i++)
                f->f_pmask[i] = 0;

        /* scan through the list of selectors */
        for (p = line; *p && *p != '\t' && *p != ' ';) {

                /* find the end of this facility name list */
                for (q = p; *q && *q != '\t' && *q != ' ' && *q++ != '.'; )
                        continue;

                /* collect priority name */
                for (bp = buf; *q && !index("\t,; ", *q); )
                        *bp++ = *q++;
                *bp = '\0';

                /* skip cruft */
                while (index(",;", *q)) /* spaces no longer allowed! */
                        q++;

                /* decode priority name */
                if (*buf == '*')
                        pri = -1;
                else {
                        pri = decode(*buf == '=' ? buf+1 : buf, prioritynames);
                        if (pri == INTERNAL_NOPRI)
                                pri = 0;
                        else if (pri < 0 || pri > LOG_PRIMASK) {
                                (void) sprintf(ebuf, "unknown priority name \"%s\"", buf);
                                logerror(ebuf);
                                return;
                        }
                        else
                                pri = (*buf == '=' ? 1 : -1) << (LOG_PRIMASK - pri);
			      
                }


                /* scan facilities */
                while (*p && !index("\t.; ", *p)) {
                        for (bp = buf; *p && !index("\t,;. ", *p); )
                                *bp++ = *p++;
                        *bp = '\0';
                        if (*buf == '*')
                                for (i = 0; i < LOG_NFACILITIES; i++)
                                        if (pri == 0)
                                                f->f_pmask[i] = pri;
                                        else
                                                f->f_pmask[i] |= pri;
                        else {
                                i = decode(buf, facilitynames);
                                if (i < 0) {
                                        (void) sprintf(ebuf,
                                            "unknown facility name \"%s\"",
                                            buf);
                                        logerror(ebuf);
                                        return;
                                }
                                if (pri == 0)
                                        f->f_pmask[i >> 3] = pri;
                                else
                                       f->f_pmask[i >> 3] |= pri;
                        }
                        while (*p == ',' || *p == ' ')
                                p++;
                }

                p = q;
        }

        /* skip to action part */
        while (*p == '\t' || *p == ' ')
                p++;

        switch (*p)
        {
        case '@':
                if (!InetInuse)
                        break;
                (void) strcpy(f->f_un.f_forw.f_hname, ++p);
                hp = gethostbyname(p);
                if (hp == NULL) {
                        logerror("hostname lookup error");
                        break;
                }
                bzero((char *) &f->f_un.f_forw.f_addr,
                         sizeof f->f_un.f_forw.f_addr);
                f->f_un.f_forw.f_addr.sin_family = AF_INET;
                f->f_un.f_forw.f_addr.sin_port = LogPort;
                bcopy(hp->h_addr, (char *) &f->f_un.f_forw.f_addr.sin_addr, hp->h_length);
                f->f_type = F_FORW;
                break;

        default:
                if ( memcmp(p, "COM",3) == 0)
                        {
                        if ((f->f_file = _open(p, O_WRONLY|O_APPEND, 0)) < 0)
                                {
                                f->f_file = F_UNUSED;
                                logerror(p);
                                break;
                                }
                        f->f_type = F_TTY;
                        break;
                        }
                if (strcmp(p, "CON") == 0)
                        {
                        if ((f->f_file = _open(p, O_WRONLY|O_APPEND,0)) < 0)
                                {
                                f->f_file = F_UNUSED;
                                logerror(p);
                                break;
                                }
                        f->f_type = F_CONSOLE;
                        break;
                        }
                (void) strcpy(f->f_un.f_fname, p);
                if ((f->f_file = _open(p, O_WRONLY|O_APPEND|O_CREAT, S_IWRITE|S_IREAD)) < 0) {
                        f->f_file = F_UNUSED;
                        logerror(p);
                        break;
                }
                f->f_type = F_FILE;
                break;
        }
}

/*
 *  Decode a symbolic name to a numeric value
 */

decode(name, codetab)
        char *name;
        CODE *codetab;
{
        register CODE *c;
        register char *p;
        char buf[40];

        if (isdigit(*name))
                return (atoi(name));

        (void) strcpy(buf, name);
        for (p = buf; *p; p++)
                if (isupper(*p))
                        *p = tolower(*p);
        for (c = codetab; c->c_name; c++)
                if (!strcmp(buf, c->c_name))
                        return (c->c_val);

        return (-1);
}


