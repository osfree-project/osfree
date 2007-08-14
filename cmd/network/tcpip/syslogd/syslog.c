/*  
**  Modified by Jochen Friedrich <jochen@audio.pfalz.de> for OS/2.
*/
/*
 * Copyright (c) 1983, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * SYSLOG -- print message on log file
 *
 * This routine looks a lot like printf, except that it outputs to the
 * log file instead of the standard output.  Also:
 *      adds a timestamp,
 *      prints the module name in front of the message,
 *      has some other formatting types (or will sometime),
 *      adds a newline on the end of the message.
 *
 * The output of this routine is intended to be read by syslogd(8).
 *
 * Author: Eric Allman
 * Modified to use UNIX domain IPC by Ralph Campbell
 */

#include <os2.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <time.h>
#include <process.h>
#include <errno.h>
#include <utils.h>
#include "syslog.h"

static int      LogFile = -1;           /* fd for log */
static int      connected;              /* have done connect */
static int      LogStat = 0;            /* status bits, set by openlog() */
static char     *LogTag = "syslog";     /* string to tag the entry with */
static int      LogFacility = LOG_USER; /* default facility code */

void syslog(int pri, char *fmt, ...)
{
        va_list ap;

        va_start(ap,fmt);
        vsyslog(pri, fmt, ap);
        va_end(ap);
}

void vsyslog(int pri, char *fmt, va_list ap)
{
        extern int errno;
        register int cnt;
        register char *p;
        time_t now;
        int fd, saved_errno;
        char tbuf[2048], fmt_cpy[1024], *stdp;

        saved_errno = errno;

        /* see if we should just throw out this message */
        if (!LOG_MASK(LOG_PRI(pri)) || (pri &~ (LOG_PRIMASK|LOG_FACMASK)))
                return;
        if (LogFile < 0 || !connected)
                openlog(LogTag, LogStat | LOG_NDELAY, 0);

        /* set default facility if none specified */
        if ((pri & LOG_FACMASK) == 0)
                pri |= LogFacility;

        /* build the message */
        (void)time(&now);
        (void)sprintf(tbuf, "<%d>%.15s ", pri, ctime(&now) + 4);
        for (p = tbuf; *p; ++p);
        if (LogStat & LOG_PERROR)
                stdp = p;
        if (LogTag) {
                (void)strcpy(p, LogTag);
                for (; *p; ++p);
        }
        if (LogStat & LOG_PID) {
                (void)sprintf(p, "[%d]", getpid() );
                for (; *p; ++p);
        }
        if (LogTag) {
                *p++ = ':';
                *p++ = ' ';
        }

        /* substitute error message for %m */
        {
                register char ch, *t1, *t2;

                for (t1 = fmt_cpy; ch = *fmt; ++fmt)
                        if (ch == '%' && fmt[1] == 'm') {
                                ++fmt;
                                for (t2 = strerror(saved_errno);
                                    *t1 = *t2++; ++t1);
                        }
                        else
                                *t1++ = ch;
                *t1 = '\0';
        }

        (void)vsprintf(p, fmt_cpy, ap);

        cnt = strlen(tbuf);

        /* output to stderr if requested */
        if (LogStat & LOG_PERROR) {
                write(2, tbuf,cnt);
                write(2,"\r\n",2);
        }

        /* output the message to the local logger */
        if (send(LogFile, tbuf, cnt, 0) >= 0 || !(LogStat&LOG_CONS))
                return;

}

static struct sockaddr_in SyslogAddr;   /* AF_INET address of local logger */
/*
 * OPENLOG -- open system log
 */
void openlog(char *ident, int logstat, int logfac)
{
        sock_init();
        if (ident != NULL)
                LogTag = ident;
        LogStat = logstat;
        if (logfac != 0 && (logfac &~ LOG_FACMASK) == 0)
                LogFacility = logfac;
        if (LogFile == -1) {
                SyslogAddr.sin_family = AF_INET;
                SyslogAddr.sin_port = htons(514);
                SyslogAddr.sin_addr.s_addr = INADDR_ANY;
                if (LogStat & LOG_NDELAY) {
                        LogFile = socket(AF_INET, SOCK_DGRAM, 0);
                }
        }
        if (LogFile != -1 && !connected &&
            connect(LogFile, (struct sockaddr*) &SyslogAddr, sizeof(SyslogAddr)) != -1)
                connected = 1;
}

/*
 * CLOSELOG -- close the system log
 */
void closelog(void)
{
        (void) soclose(LogFile);
        LogFile = -1;
        connected = 0;
}

static int      LogMask = 0xff;         /* mask of priorities to be logged */
/*
 * SETLOGMASK -- set the log mask level
 */
int setlogmask(int pmask)
{
        int omask;

        omask = LogMask;
        if (pmask != 0)
                LogMask = pmask;
        return (omask);
}
