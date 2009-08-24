/*****************************************************
 * Ping Tool                                         *
 * Pings remote hosts on TCP/IP                      *
 * (c) 1997    Patrick Haller Systemtechnik          *
 *****************************************************/

 /* Remark:
    During the port to NT there turned out to be quite many
   race conditions and internal "non-beauties" inside the
   Winsock stack. Especially WinSock requires the explicit
   WSA_FLAG_OVERLAPPED flag, otherwise sockets become blocked
        for non-obvious reasons. Very, very strange.

        Design problem: the READ thread should be asynchronous,
        not the SEND thread. Reason: slow packets are lost
        from the statistics.
  */

 /*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Muuss.
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
 * This product includes software developed by the University of
 * California, Berkeley and its contributors.
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
 * From: @(#)ping.c                      5.9 (Berkeley) 5/12/91
 */
/*
 *       P I N G . C
 *
 * Using the InterNet Control Message Protocol (ICMP) "ECHO" facility,
 * measure round-trip-delays and packet loss across network paths.
 *
 * Author -
 * Mike Muuss
 * U. S. Army Ballistic Research Laboratory
 * December, 1983
 *
 * Status -
 * Public Domain.  Distribution Unlimited.
 * Bugs -
 * More statistics could always be gathered.
 */

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <osfree.h>

  #pragma pack(1)

  #define TCPV40HDRS
  #include <types.h>
//  #include <arpa/inet.h>
  #include <netinet/in_systm.h>
  #include <netinet/in.h>
  #include <netinet/ip.h>
  #include <netinet/ip_icmp.h>
  #include <sys/socket.h>
  #include <sys/select.h>
//  #include <unistd.h>
  #include <netdb.h>
  #include <nerrno.h>

  #ifdef __BORLANDC__
    typedef void (__stdcall *THREAD)(PVOID pParameters);
  #endif

  #ifdef __IBMC__
    typedef void (*THREAD)(PVOID pParameters);
  #endif

  #ifdef __WATCOMC__
    typedef void (*THREAD)(PVOID pParameters);
  #endif

  #define sockErrno errno
  #define SOCKET_ERROR -1


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <process.h>
#include <conio.h>
#include <ctype.h>

#include <tcpcmd_shared.h>

#define ERROR_TOOLS_INVALID_ARGUMENT   1003                       /* SYS1003 */

                                                          /* own error codes */
#define ERROR_TOOLS                      0xC0000000
#define ERROR_TOOLS_INVALID_PARAMETER    ERROR_TOOLS + 0

typedef struct
{
  char         *pszToken;                      /* Token fБr die Command Line */
  char         *pszDescription;                              /* Beschreibung */
  void         *pTarget;                      /* Zeiger auf die Zielvariable */
  unsigned char ucTargetFormat;                   /* Format der Zielvariable */
  char         *pTargetSpecified;  /* Flag, ob dieses Kommando benutzt wurde */
} ARGUMENT, *PARGUMENT;

#define ARG_NULL        0x00
#define ARG_ULONG       0x04
#define ARG_UCHAR       0x02
#define ARG_MANDATORY   0x40
#define ARG_MUST        ARG_MANDATORY
#define ARG_PSZ         0x07
#define ARG_USHORT      0x06
#define ARG_DEFAULT     0x80
#define ARG_ENDOFLIST   0xFF

#define ARG_HIDDEN      0x20            /* does not show up in the help list */

APIRET ToolsDumpHex (ULONG ulPosition,
                                    ULONG ulLength,
                                    PVOID pSource);

APIRET ToolsDumpHex (ULONG ulPosition,
                                    ULONG ulLength,
                                    PVOID pSource)
{
  ULONG ulCounter;                                 /* temporary loop counter */
  CHAR  szBuffer[128];            /* local, temporary buffer for debug lines */
  CHAR  szBuffer2[80];            /* local, temporary buffer for debug lines */
  CHAR  szBuffer3[80];            /* local, temporary buffer for debug lines */
  CHAR  szBuffer4[20];            /* local, temporary buffer for debug lines */
  UCHAR ucData;                                                 /* data byte */
  PCHAR pchTemp;                                /* pointer to that data byte */
  ULONG ulTotalLength;           /* total length of printed buffer + padding */

  sprintf(szBuffer, "%08x│", ulPosition);

  szBuffer2[0] = 0;
  szBuffer3[0] = 0;

  ulTotalLength = (ulLength + 0x0000000f) & 0xfffffff0;

  for (ulCounter = 0;
       ulCounter < ulTotalLength;                           /* round up to 8 */
       ulCounter++)
  {
    if (ulCounter < ulLength)
    {
      pchTemp = (PCHAR)pSource + ulCounter;
      ucData = *pchTemp;
      sprintf(szBuffer4, "%02x ", ucData);
      strcat (szBuffer2, szBuffer4);                        /* add hex token */

      if (ucData < ' ')
        strcat (szBuffer3,"·");                     /* unprintable character */
      else
      {
        szBuffer4[0] = ucData;
        szBuffer4[1] = 0;
        strcat(szBuffer3, szBuffer4);
      }
    }
    else
    {
      strcat(szBuffer2,"·· ");                    /* insert dummy characters */
      strcat(szBuffer3," ");
    }

    if (ulCounter % 0x00000010 == 0x0000000f)         /* check for linebreak */
    {
      strcat(szBuffer, szBuffer2);
      strcat(szBuffer,"│");
      strcat(szBuffer, szBuffer3);

      printf ("\n%s",szBuffer);

      szBuffer2[0] = 0;
      szBuffer3[0] = 0;
      sprintf(szBuffer, "%08x│", ulCounter+1+ulPosition);
    }
  }

  return (NO_ERROR);                                                   /* OK */
}


APIRET ToolsPerfQuery(double *ps);

APIRET ToolsPerfQuery(double *ps)
{
  APIRET rc;
  ULONG  f;
  QWORD  t;
  double x;

  rc=DosTmrQueryFreq(&f);
  if (rc!=NO_ERROR)
    return(rc);

  rc=DosTmrQueryTime(&t);
  if (rc!=NO_ERROR)
    return(rc);

  x = (double)t.ulHi;
  x *= 65536.0;
  x *= 65536.0;
  x += (double)t.ulLo;

  x = x / (double)f;

  ps= &x;

  return NO_ERROR;
}


/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/

/* #define DEBUG 1 */

#define ERROR_USER                  0xC000
#define ERROR_IP_UNKNOWN_PROTOCOL   ERROR_USER + 0
#define ERROR_IP_UNUSABLE_STACK     ERROR_USER + 1
#define ERROR_IP_SOCKET_ERROR       ERROR_USER + 2


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef int SOCKET;
typedef SOCKET *PSOCKET;

typedef struct
{
  char fsHelp;                     /* help is requested from command line */
  char fsHost;                                            /* base address */
  char fsSend;                             /* number of packets specified */
  char fsInterval;                                /* time between packets */
  char fsPreload;                                            /* preload ? */
  char fsSize;                                             /* packet size */
  char fsTTL;                                             /* time to live */
  char fsOptionDebug;                           /* socket options - DEBUG */
  char fsOptionFlood;                                       /* Flood mode */
  char fsOptionNumeric;                                   /* numeric mode */
  char fsOptionPattern;                  /* fill packet with user pattern */
  char fsOptionQuiet;                                       /* quiet mode */
  char fsOptionRoute;                                   /* record routing */
  char fsOptionNoRouting;                        /* bypass routing tables */
  char fsOptionVerbose;                                   /* verbose mode */
  char fsOptionDumpIP;                                    /* dump IP hdr  */

  ULONG   ulSend;                               /* number of packets to send */
  ULONG   ulInterval;                                /* time between packets */
  ULONG   ulPreload;                                              /* preload */
  USHORT  usSize;                                             /* packet size */
  UCHAR   ucTTL;                                             /* time to live */

  PSZ     pszHost;                        /* start with this base IP address */
} OPTIONS, *POPTIONS;


typedef struct _Globals
{
  /*
   * Note: on some systems dropping root makes the process dumpable or
   * traceable. In that case if you enable dropping root and someone
   * traces ping, they get control of a raw socket and can start
   * spoofing whatever packets they like. SO BE CAREFUL.
   */

  #define   DEFDATALEN                   (64 - 8)       /* default data length */
  #define   MAXIPLEN                     60
  #define   MAXICMPLEN                   76
  #define   MAXPACKET                    (65536 - 60 - 8)          /* max packet size */
  #define   MAXWAIT                         10  /* max seconds to wait for response */
  #define   NROUTES                         9       /* number of record route slots */
                                                   /* identify byte in array */
  #define   A(bit)                          Globals.rcvd_tbl[(bit)>>3]
  #define   B(bit)                          (1 << ((bit) & 0x07))/* identify bit in byte */
  #define   SET(bit)                     (A(bit) |= B(bit))
  #define   CLR(bit)                     (A(bit) &= (~B(bit)))
  #define   TST(bit)                     (A(bit) & B(bit))

  /* multicast options */
  int moptions;
  #define MULTICAST_NOLOOP               0x001
  #define MULTICAST_TTL                     0x002
  #define MULTICAST_IF                      0x004

  /*
   * MAX_DUP_CHK is the number of bits in received table, i.e. the maximum
   * number of received sequence numbers we can keep track of.  Change 128
   * to 8192 for complete accuracy...
   */
  #define   MAX_DUP_CHK                  (8 * 128)
  int             mx_dup_ck;
  char            rcvd_tbl[MAX_DUP_CHK / 8];
  struct sockaddr saTarget;                                            /* who to ping */
  SOCKET          sockPing;                        /* socket file descriptor */
  u_char          PacketOutput[MAXPACKET];
  char            BSPACE;                    /* characters written for flood */
  char            DOT;
  PSZ             pszHostname;
  int             pidIdentifier;       /* process id to identify our packets */

  /* counters */
  ULONG ulPktReceived;                           /* # of packets we got back */
  ULONG ulPktDuplicate;                              /* number of duplicates */
  ULONG ulPktSent;                /* sequence # for outbound packets = #sent */
  ULONG ulPktReceivedForeign;      /* foreign packets that came in by socket */

  /* timing */
  int    timing;                                        /* flag to do timing */
  double  fTimeMin;                               /* minimum round trip time */
  double  fTimeMax;                               /* maximum round trip time */
  double  fTimeSum;                   /* sum of all times, for doing average */

  double  fTimeSending;                            /* total time for sending */
  double  fBytesSent;                         /* counter of total bytes sent */
  double  fTimeReceiving;                        /* total time for receiving */
  double  fBytesReceived;                 /* counter of total bytes received */

  double psSendStart;                   /* benchmark sending performance */
  double psSendEnd;                     /* benchmark sending performance */

  BOOL   fThreadQuit;       /* this flag turns to TRUE when ThdCatcher quits */
  TID    tidFire;                            /* thread id of the fire thread */
} GLOBALS, *PGLOBALS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;


#define ARG_TERMINATE {(PSZ)NULL,(PSZ)NULL,(PVOID)NULL,ARG_ENDOFLIST,NULL}

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
-?{"/?",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
lastpar{"/PACKETS=",  "Number of packets to transmit.",
                                       &Options.ulSend,      ARG_ULONG,      &Options.fsSend},
-d{"/DEBUG",     "Debug mode.",        NULL,                 ARG_NULL,       &Options.fsOptionDebug},
  {"/FLOOD",     "Flood mode.",        NULL,                 ARG_NULL,       &Options.fsOptionFlood},
  {"/DUMP.IP",   "Dump the IP header.",NULL,                 ARG_NULL,       &Options.fsOptionDumpIP},
  {"/INTERVAL=", "Timing interval between packets (ms).",
                                       &Options.ulInterval,  ARG_ULONG,      &Options.fsInterval},
  {"/PRELOAD=",  "Preload value.",     &Options.ulPreload,   ARG_ULONG,      &Options.fsPreload},
  {"/NUMERIC",   "Numeric mode.",      NULL,                 ARG_NULL,       &Options.fsOptionNumeric},
  {"/PATTERN",   "Fill buffer with user pattern/",
                                       NULL,                 ARG_NULL,       &Options.fsOptionPattern},
  {"/QUIET",     "Quiet mode.",        NULL,                 ARG_NULL,       &Options.fsOptionQuiet},
  {"/ROUTE",     "Record route mode.", NULL,                 ARG_NULL,       &Options.fsOptionRoute},
-r{"/!ROUTING",  "Dont use routing.",  NULL,                 ARG_NULL,       &Options.fsOptionNoRouting},
predlast{"/SIZE=",     "Packet size.",       &Options.usSize,      ARG_USHORT,     &Options.fsSize},
-v{"/VERBOSE",   "Verbose mode.",      NULL,                 ARG_NULL,       &Options.fsOptionVerbose},
  {"/TTL=",      "Time to live.",      &Options.ucTTL,       ARG_UCHAR,      &Options.fsTTL},
  {"1",          "Start with this "
                 "IP address.",        &Options.pszHost,     ARG_PSZ     |
                                                             ARG_DEFAULT |
                                                             ARG_MUST,       &Options.fsHost},
  ARG_TERMINATE
};


Использование:  ping [-?drv] <хост> [размер [пакеты]]

Где:
-d      - Включить отладку.
-r      - Не использовать обычную таблицу маршрутизации.
-v      - Подробный вывод. Включать все полученные пакеты ICMP.
host    - Адресат.
size    - Размер части пакета, содержащей данные.
packets - Число посланных пакетов эхо-запросов.

/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                  (void);

APIRET NetPing               (void);

char   *pr_addr              (u_long);

int    in_cksum              (u_short *addr,
                              int     len);

void   ThdCatcher            (PVOID pParameter);

void   finish        (int ignore);

void   pinger                (void);

void   fill                  (void *bp,
                              char *patp);

void   pr_pack               (char   *buf,
                              int    cc,
                              struct sockaddr_in *from);

void   pr_icmph              (struct icmp *icp);

void   pr_retip              (struct ip *ip);

APIRET initialize            (void);

int    main                  (int,
                              char **);


/***********************************************************************
 * Name      : void help
 * Funktion  : Anzeigen des Hilfetextes.
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 ***********************************************************************/

void help (void)
{
//  TOOLVERSION("Ping",                                   /* application name */
//              0x00010007,                           /* application revision */
//              0x00010100,          /* minimum required revision of PHSTOOLS */
//              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
//              NULL,                                               /* Remark */
//              NULL);                                /* additional copyright */
}


/***********************************************************************
 * Name      : APIRET NetPing
 * Funktion  : the REAL ping code
 * Parameter : void
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 ***********************************************************************/

APIRET NetPing(void)
{
  struct hostent     *pHostEntry;
  struct sockaddr_in *to;
  struct protoent    *proto;
  struct in_addr     ifaddr;
  int                i;
  int                hold,
                     packlen;
  ULONG              ulPreloadCounter;
  u_char             *datap,
                     *packet;
  char               *pszTarget,
                     hnamebuf[MAXHOSTNAMELEN];
  u_char             ttl,
                     loop;
  int                iAddress;

  struct sockaddr_in from;
  int                cc;
  size_t             fromlen;

  double         psReceiveStart;        /* for benchmarking the recvfrom */
  double         psReceiveEnd;
  ULONG              ulTimeout;             /* timeout value for the sockets */
  APIRET             rc;                                   /* API returncode */
  int                irc;                              /* integer returncode */
  BOOL               fTimeValid = FALSE;     /* initial timestamp is invalid */

#ifdef IP_OPTIONS
  char               rspace[3 + 4 * NROUTES + 1];      /* record route space */
#endif


                 /* Pull this stuff up front so we can drop root if desired. */
  proto = getprotobyname("icmp");
  if (proto == NULL)
    return (ERROR_IP_UNKNOWN_PROTOCOL);             /* raise error condition */


  /***************************************************************************
   * Opening the Ping socket                                                 *
   ***************************************************************************/
  Globals.sockPing = socket(AF_INET,                      /* create a socket */
                            SOCK_RAW,
                            proto->p_proto);
  if ( Globals.sockPing < 0)
    return (sockErrno);                             /* raise error condition */

  ulTimeout = 32000;                                  /* timeout is 1 second */
  rc = setsockopt(Globals.sockPing,                       /* set the timeout */
             SOL_SOCKET,
                  SO_SNDBUF,
                  (PSZ)&ulTimeout,
                  sizeof(ulTimeout));
  if (rc == SOCKET_ERROR)                                /* check for errors */
   return (ERROR_IP_SOCKET_ERROR);                 /* raise error condition */

  ulTimeout = 32000;                                  /* timeout is 1 second */
  rc = setsockopt(Globals.sockPing,                       /* set the timeout */
             SOL_SOCKET,
                  SO_RCVBUF,
                  (PSZ)&ulTimeout,
                  sizeof(ulTimeout));
  if (rc == SOCKET_ERROR)                                /* check for errors */
   return (ERROR_IP_SOCKET_ERROR);                 /* raise error condition */

  ulPreloadCounter = 0;
  datap = &Globals.PacketOutput[8 + sizeof(double)];

  pszTarget = Options.pszHost;
  memset(&Globals.saTarget,
         0,
         sizeof(struct sockaddr));

  to = (struct sockaddr_in *)&Globals.saTarget;
  to->sin_family = AF_INET;

  iAddress = inet_addr(pszTarget);
  if (iAddress != -1)
  {
    to->sin_addr.s_addr = iAddress;
    Globals.pszHostname = pszTarget;
  }
  else
  {
    pHostEntry = gethostbyname(pszTarget);

    if (!pHostEntry)
    {
      fprintf(stderr,
              "ping: unknown host %s\n",
              pszTarget);
      exit(2);
    }

    to->sin_family = pHostEntry->h_addrtype;

    if (pHostEntry->h_length > sizeof(to->sin_addr))
            pHostEntry->h_length = sizeof(to->sin_addr);

    memcpy(&to->sin_addr,
           pHostEntry->h_addr,
           pHostEntry->h_length);
    strncpy(hnamebuf,
            pHostEntry->h_name,
            sizeof(hnamebuf) - 1);
    Globals.pszHostname = hnamebuf;
  }

  if (Options.fsOptionFlood && Options.fsInterval)
  {
    fprintf (stderr,
             "\nError: /FLOOD and /INTERVAL are incompatible options.");
    exit(2);
  }

  if (Options.usSize >= sizeof(double))          /* can we time transfer */
    Globals.timing = 1;

  packlen = Options.usSize + MAXIPLEN + MAXICMPLEN;
  packet = malloc((u_int)packlen);
  if (!packet)
  {
    fprintf(stderr, "ping: out of memory.\n");
    exit(2);
  }

  if (!Options.fsOptionPattern)      /* fill the packet with default pattern */
    for (i = 8;
         i < Options.usSize;
         ++i)
      *datap++ = i;


  Globals.pidIdentifier = getpid() & 0xFFFF;  /* calculate packet identifier */
  hold = 1;

  if (Options.fsOptionDebug)                         /* debug sockets option */
    setsockopt(Globals.sockPing,
               SOL_SOCKET,
               SO_DEBUG,
               (char *)&hold,
               sizeof(hold));

  if (Options.fsOptionNoRouting)                     /* loose routing option */
    setsockopt(Globals.sockPing,
               SOL_SOCKET,
               SO_DONTROUTE,
               (char *)&hold,
               sizeof(hold));

                            /* this is necessary for broadcast pings to work */
  setsockopt(Globals.sockPing,
             SOL_SOCKET,
             SO_BROADCAST,
             (char *)&hold,
             sizeof(hold));

  if (Options.fsOptionRoute)                          /* record route option */
  {
#ifdef IP_OPTIONS
    rspace[IPOPT_OPTVAL] = IPOPT_RR;
    rspace[IPOPT_OLEN] = sizeof(rspace)-1;
    rspace[IPOPT_OFFSET] = IPOPT_MINOFF;

    if (setsockopt(Globals.sockPing,
                   IPPROTO_IP,
                   IP_OPTIONS,
                   rspace,
                   sizeof(rspace)) < 0)
      return (sockErrno);                               /* raise error condition */
#else
    fprintf(stderr,
            "ping: record route not available in this implementation.\n");
    exit(2);
#endif /* IP_OPTIONS */
  }

  /*
   * When pinging the broadcast address, you can get a lot of answers.
   * Doing something so evil is useful if you are trying to stress the
   * ethernet, or just want to fill the arp cache to get some stuff for
   * /etc/ethers.
   */
  hold = 48 * 1024;
  setsockopt(Globals.sockPing,
             SOL_SOCKET,
             SO_RCVBUF,
             (char *)&hold,
             sizeof(hold));

/*#if 0*/
  if (Globals.moptions & MULTICAST_NOLOOP)
  {
    if (setsockopt(Globals.sockPing,
                   IPPROTO_IP,
                   IP_MULTICAST_LOOP,
                   &loop,
                   1) == -1)
      return (sockErrno);                               /* raise error condition */
  }

  if (Globals.moptions & MULTICAST_TTL)
  {
    if (setsockopt(Globals.sockPing,
                   IPPROTO_IP,
                   IP_MULTICAST_TTL,
                   &ttl,
                   1) == -1)
      return (sockErrno);                               /* raise error condition */
  }

  if (Globals.moptions & MULTICAST_IF)
  {
    if (setsockopt(Globals.sockPing,
                   IPPROTO_IP,
                   IP_MULTICAST_IF,
                   (PSZ)&ifaddr,
                   sizeof(ifaddr)) == -1)
      return (sockErrno);                               /* raise error condition */
  }

/*#endif*/
  if (to->sin_family == AF_INET)
    printf("PING %s (%s): %d data bytes\n",
           Globals.pszHostname,
           inet_ntoa(*(struct in_addr *)&to->sin_addr.s_addr),
           Options.usSize);
  else
    printf("PING %s: %d data bytes\n",
           Globals.pszHostname,
           Options.usSize);

  signal(SIGINT,
         finish);

  ToolsPerfQuery(&Globals.psSendStart);                      /* measure time */

  if (Options.fsPreload)                     /* check for preload conditions */
    for (ulPreloadCounter=0;                       /* quickly fire n packets */
         ulPreloadCounter < Options.ulPreload;
         ulPreloadCounter++)
      pinger();


  Globals.tidFire =
  #ifdef __BORLANDC__
             _beginthread(ThdCatcher,                                   /* flood thread */
               1024000,                               /* 128k thread stack ! */
               NULL);
  #endif

  #ifdef __WATCOMC__
             _beginthread(ThdCatcher,
                          NULL,
                          32768,
                          NULL);
  #endif

  #ifdef __IBMC__
             _beginthread(ThdCatcher,
                          NULL,
                          32768,
                          NULL);
  #endif


  if (Globals.tidFire == -1)               /* means the thread did not start */
  {
    fprintf (stderr,
             "\nError: fire thread could not be launched.");
    exit (1);
  }


  ulTimeout = 1000;                                         /* 1 sec timeout */
  fromlen = sizeof(from);

  for (;                                       /* this is the main ping loop */
       (Globals.fThreadQuit == FALSE) &&
       (!kbhit());
      )
  {
    if (fTimeValid == FALSE)          /* if our current timestamp is invalid */
    {
      ToolsPerfQuery(&psReceiveStart);                       /* measure time */
      fTimeValid = TRUE;                       /* now our timestamp is valid */
    }

    irc = select(&Globals.sockPing,             /* wait for incoming packets */
                 1,
                 0,
                 0,
                 ulTimeout);

    if (irc > 0)                              /* means a packet is available */
    {

      cc = recvfrom(Globals.sockPing,                      /* receive packet */
                    (char *)packet,
                    packlen,
                    0,
                    (struct sockaddr *)&from,
                    (int *)&fromlen);

      ToolsPerfQuery(&psReceiveEnd);                         /* measure time */
      fTimeValid = FALSE;        /* next time we need to get a new timestamp */

      if (cc > 0)                    /* if successfully received a few bytes */
      {
        Globals.fTimeReceiving += psReceiveEnd -
                                  psReceiveStart;
        Globals.fBytesReceived += cc;

        if (Options.fsOptionFlood)
          if (Globals.ulPktSent % 100 == 0)
            printf("\rSent (%7u/%12.0f) Received (%7u/%12.0f)",
                   Globals.ulPktSent,
                   Globals.fBytesSent,
                   Globals.ulPktReceived,
                   Globals.fBytesReceived);
      }
      else
        if (cc < 0)
        {
          cmd_ShowSystemMessage(sockErrno, 0L);
          continue;
        }

      pr_pack((char *)packet,
              cc,
              &from);

      if (Options.ulSend &&
          (Globals.ulPktReceived >= Options.ulSend) )
        break;
    }
    else
      if (irc < 0)                   /* means an error on the socket occured */
        psock_errno("readthd::select");
  }

  finish(0);
  /* NOTREACHED */
  return 0;
}

/* x
 * ThdCatcher --
 * This routine causes another PING to be transmitted, and then
 * schedules another SIGALRM for 1 second from now.
 *
 * bug --
 * Our sense of time will slowly skew (i.e., packets will not be
 * launched exactly at 1-second intervals).  This does not affect the
 * quality of the delay and loss statistics.
 */
void ThdCatcher (PVOID pParameter)
{
  int waittime;

  for(;Globals.fThreadQuit == FALSE;)
  {
    pinger();                                         /* send an ICMP packet */

    if (!Options.ulSend ||
        (Globals.ulPktSent < Options.ulSend) )
    {
      if (!Options.fsOptionFlood)
        DosSleep(Options.ulInterval);           /* send this thread to sleep */
    }
    else
    {
      if (Globals.ulPktReceived)
      {
                                       /* 2x max. trip-around time */
        waittime = 2 * (ULONG)Globals.fTimeMax * 1000;
        if (waittime > MAXWAIT)
          waittime = MAXWAIT;
      }
      else
        waittime = MAXWAIT;

      if (!Options.fsOptionFlood)
        DosSleep(waittime);                                 /* really wait ? */

      Globals.fThreadQuit = TRUE;                    /* used for thread sync */
      /* @@@PH we should end up in finish() */
    }
  }
}


/*
 * pinger --
 *    Compose and transmit an ICMP ECHO REQUEST packet.  The IP packet
 * will be added on by the kernel.  The ID field is our UNIX process ID,
 * and the sequence number is an ascending integer.  The first 8 bytes
 * of the data portion are used to hold a double variable
 * to compute the round-trip time.
 */
static void pinger(void)
{
  struct icmp *icp;
  int    cc;
  int    i;
  int    irc;                                         /* integer return code */

  irc = select(&Globals.sockPing, /* wait for the socket to become writeable */
               0,
               1,
               0,
               1000);

  if (irc > 0)
  {
    icp             = (struct icmp *)Globals.PacketOutput;
    icp->icmp_type  = ICMP_ECHO;
    icp->icmp_code  = 0;
    icp->icmp_cksum = 0;
    icp->icmp_seq   = (USHORT)Globals.ulPktSent++;
    icp->icmp_id    = Globals.pidIdentifier;                           /* ID */
    CLR(icp->icmp_seq % Globals.mx_dup_ck);

    cc = Options.usSize + 8;                          /* skips ICMP portion */


    if (Globals.timing)
      ToolsPerfQuery(&(double)Globals.PacketOutput[8]);

        /* checksum MUST of course be done immediately before packet is sent */
    icp->icmp_cksum = in_cksum((u_short *)icp, /* compute ICMP checksum here */
                               cc);

    i = sendto(Globals.sockPing,
               (char *)Globals.PacketOutput,
               cc,
               0,
               &Globals.saTarget,
               sizeof(struct sockaddr));
    if (i < 0 || i != cc)
    {
      if (i < 0)
        cmd_ShowSystemMessage(sockErrno, 0L);

      psock_errno("sendto");
      fprintf(stderr,
              "\nping: wrote %s %d chars, ret=%d",
              Globals.pszHostname,
              cc,
              i);
    }
    else
    {
      Globals.fBytesSent += i;             /* keep track of total bytes sent */

      if (Options.fsOptionFlood)
        if (Globals.ulPktSent % 100 == 0)
          printf("\rSent (%7u/%12.0f) Received (%7u/%12.0f)",
                 Globals.ulPktSent,
                 Globals.fBytesSent,
                 Globals.ulPktReceived,
                 Globals.fBytesReceived);
    }

    if (!Options.fsOptionQuiet && !Options.fsOptionFlood)
      putch(Globals.DOT);
  }
  else
    if (irc < 0)                     /* means an error on the socket occured */
      psock_errno("pinger::select");
}


/*
 * pr_pack --
 * Print out the packet, if it came from us.  This logic is necessary
 * because ALL readers of the ICMP socket get a copy of ALL ICMP packets
 * which arrive ('tis only fair).  This permits multiple copies of this
 * program to be run without having intermingled output (or statistics!).
 */
void pr_pack(char              *buf,
             int                cc,
             struct sockaddr_in *from)
{
  struct icmp    *icp;
  int            i;
  u_char         *cp,*dp;

  struct ip      *ip;
  double        tv;
  double        *tp;
  double          fTriptime = 0.0;
  int            hlen, dupflag;

  ToolsPerfQuery(&tv);

  /* Check the IP header */
  ip = (struct ip *)buf;
  hlen = ip->ip_hl << 2;

  if (cc < Options.usSize + ICMP_MINLEN)
  {
    if (Options.fsOptionVerbose)
      fprintf(stderr,

              "ping: packet too short (%d bytes) from %s\n",
              cc,
         inet_ntoa(*(struct in_addr *)&from->sin_addr.s_addr));
      return;

  }

  /* Now the ICMP part */
  cc -= hlen;
  icp = (struct icmp *)(buf + hlen);

  /* PH 98/05/29 filter foreign packets immediately since otherwise multiple */
  /* pings will confuse each other.                                          */
  if (icp->icmp_id != Globals.pidIdentifier)       /* check identifier = PID */
  {
    Globals.ulPktReceivedForeign++;               /* count as foreign packet */
    return;                                            /* 'Twas not our ECHO */
  }


  if (icp->icmp_type == ICMP_ECHOREPLY)     /* check for correct packet type */
  {
    ++Globals.ulPktReceived;

    if (Globals.timing)
    {
      tp = (double *)icp->icmp_data;
      fTriptime = ((tv - *tp) * 1000.0);
      Globals.fTimeSum += fTriptime;

      if (fTriptime < Globals.fTimeMin) Globals.fTimeMin = fTriptime;
      if (fTriptime > Globals.fTimeMax) Globals.fTimeMax = fTriptime;
    }

    if (TST(icp->icmp_seq % Globals.mx_dup_ck))
    {
      ++Globals.ulPktDuplicate;
      --Globals.ulPktReceived;
      dupflag = 1;
    }
    else
    {
      SET(icp->icmp_seq % Globals.mx_dup_ck);
      dupflag = 0;
    }

    if (Options.fsOptionQuiet)
      return;

    if (!Options.fsOptionFlood)
    {
      printf("\r%d bytes from %s: icmp_seq=%u",
             cc,
             inet_ntoa(*(struct in_addr *)&from->sin_addr.s_addr),
             icp->icmp_seq);

      printf(" ttl=%d",
             ip->ip_ttl);

      if (Globals.timing)
        printf(" time=%10.3f ms",
               fTriptime);

      if (dupflag)
        printf(" (DUP!)");

      /* check the data */
      cp = (u_char*)icp + 8 + sizeof(double);
      dp = &Globals.PacketOutput[8 + sizeof(double)];

      for (i = 8 + sizeof(double);
           i < Options.usSize;
           ++i, ++cp, ++dp)
      {
        if (*cp != *dp)
        {
          printf("\nwrong data byte #%d should be 0x%x but was 0x%x",
                 i,
                 *dp,
                 *cp);
          ToolsDumpHex (0,
                        Options.usSize - 8,
                        icp);
          break;
        }
      }
    }
  }
  else
  {
    printf("%d bytes from %s (bad): ",
           cc,
           pr_addr(from->sin_addr.s_addr));

    /* We've got something other than an ECHOREPLY */
    if (!Options.fsOptionVerbose)
      return;

    pr_icmph(icp);
  }

  if (Options.fsOptionDumpIP)                          /* dump the IP header */
     pr_retip( (struct ip *)buf);

  if (!Options.fsOptionFlood)
  {
    putchar('\n');
    fflush(stdout);
  }
}
/*
 * in_cksum --
 * Checksum routine for Internet Protocol family headers (C Version)
 */
static int in_cksum(u_short *addr,
                    int     len)
{
  int nleft      = len;
  u_short *w     = addr;
  int sum        = 0;
  u_short answer = 0;

   /*
    * Our algorithm is simple, using a 32 bit accumulator (sum), we add
    * sequential 16 bit words to it, and at the end, fold back all the
    * carry bits from the top 16 bits into the lower 16 bits.
    */
  while (nleft > 1)
  {
    sum += *w++;
    nleft -= 2;
  }

                                         /* mop up an odd byte, if necessary */
  if (nleft == 1)
  {
    *(u_char *)(&answer) = *(u_char *)w ;
    sum += answer;
  }

                      /* add back carry outs from top 16 bits to low 16 bits */
  sum = (sum >> 16) + (sum & 0xffff);                 /* add hi 16 to low 16 */
  sum += (sum >> 16);                                                  /* add carry */
  answer = ~sum;                                     /* truncate to 16 bits */
  return(answer);
}


/*
 * finish --
 * Print out statistics, and give up.
 */
void finish(int ignore)
{
  ToolsPerfQuery(&Globals.psSendEnd);                        /* measure time */

  signal(SIGINT,
         SIG_IGN);

  /* stop 2nd thread */
  Globals.fThreadQuit = TRUE;

  DosWaitThread(&Globals.tidFire,
                DCWW_WAIT);

  Globals.fTimeSending = Globals.psSendEnd -
                         Globals.psSendStart;


  putchar('\n');
  fflush(stdout);

  printf("--- %s ping statistics ---\n"
         "%ld packets transmitted, "
         "%ld packets received, ",
         Globals.pszHostname,
         Globals.ulPktSent,
         Globals.ulPktReceived);

  if (Globals.ulPktDuplicate)
    printf("%ld duplicates, ",
           Globals.ulPktDuplicate);

  if (Globals.ulPktReceivedForeign)
    printf("%ld foreign, ",
           Globals.ulPktReceivedForeign);

  if (Globals.ulPktSent)
    if (Globals.ulPktReceived > Globals.ulPktSent)
      printf("-- somebody's printing up packets!");
    else
      printf("%d%% packet loss",
             (int) (((Globals.ulPktSent - Globals.ulPktReceived) * 100) /
                    Globals.ulPktSent));

  if (Globals.ulPktReceived && Globals.timing)
    printf("\nround-trip min/avg/max = %10.3f %10.3f %10.3f ms\n",
            Globals.fTimeMin,
            Globals.fTimeSum /
              (Globals.ulPktReceived + Globals.ulPktDuplicate),
            Globals.fTimeMax);

  printf ("\nSending    bytes sent  %10f.0",
          Globals.fBytesSent);

  if (Globals.fTimeSending != 0.0)
    printf ("\n           time        %10.3fs"
            "\n           pkts / sec  %10.3f"
            "\n           throughput  %10.3fb/s",
            Globals.fTimeSending,
            Globals.ulPktSent / Globals.fTimeSending,
            Globals.fBytesSent / Globals.fTimeSending);

  printf ("\nReceiving  bytes recv  %10.0f",
          Globals.fBytesReceived);

  if (Globals.fTimeReceiving != 0.0)
    printf ("\n           time        %10.3fs"
            "\n           pkts / sec  %10.3f"
            "\n           throughput  %10.3fb/s",
            Globals.fTimeReceiving,
            Globals.ulPktReceived / Globals.fTimeReceiving,
            Globals.fBytesReceived / Globals.fTimeReceiving);

  soclose(Globals.sockPing);                            /* close the sockets */
  /*soclose(Globals.sockRecv);*/

  if (Globals.ulPktReceived==0)
    exit(1);

  exit(0);
}


#ifdef notdef
static char *ttab[] = {
   "Echo Reply",                         /* ip + seq + udata */
   "Dest Unreachable",                   /* net, host, proto, port, frag, sr + IP */
   "Source Quench",                      /* IP */
   "Redirect",    /* redirect type, gateway, + IP  */
   "Echo",
   "Time Exceeded",                      /* transit, frag reassem + IP */
   "Parameter Problem",                  /* pointer + IP */
   "Timestamp",                          /* id + seq + three timestamps */
   "Timestamp Reply",                    /* " */
   "Info Request",                          /* id + sq */
   "Info Reply"                          /* " */
};
#endif


/*
 * pr_icmph --
 * Print a descriptive string about an ICMP header.
 */
void pr_icmph(struct icmp *icp)
{
#define ICMPCODE(a,b) case a: printf(b); break;


  switch(icp->icmp_type)
  {
    ICMPCODE(ICMP_ECHOREPLY,"Echo Reply\n")           /* XXX ID + Seq + Data */

    /*************************************************************************
     * Destination unreachable                                               *
     *************************************************************************/

    case ICMP_UNREACH:
      switch(icp->icmp_code)
      {
        ICMPCODE(ICMP_UNREACH_NET,      "Destination Net Unreachable\n")
        ICMPCODE(ICMP_UNREACH_HOST,     "Destination Host Unreachable\n")
        ICMPCODE(ICMP_UNREACH_PROTOCOL, "Destination Protocol Unreachable\n")
        ICMPCODE(ICMP_UNREACH_PORT,     "Destination Port Unreachable\n")
        ICMPCODE(ICMP_UNREACH_NEEDFRAG, "frag needed and DF set\n")
        ICMPCODE(ICMP_UNREACH_SRCFAIL,  "Source Route Failed\n")
/*
        ICMPCODE(ICMP_NET_UNKNOWN,  "Network Unknown\n")
        ICMPCODE(ICMP_HOST_UNKNOWN, "Host Unknown\n");
        ICMPCODE(ICMP_HOST_ISOLATED,"Host Isolated\n");
        ICMPCODE(ICMP_NET_UNR_TOS,  "Destination Network Unreachable At "
                                    "This TOS\n")
        ICMPCODE(ICMP_HOST_UNR_TOS, "Destination Host Unreachable At "
                                    "This TOS\n")
        ICMPCODE(ICMP_PKT_FILTERED, "Packet Filtered\n")
        ICMPCODE(ICMP_PREC_VIOLATION,"Precedence Violation\n")
        ICMPCODE(ICMP_PREC_CUTOFF,  "Precedence Cutoff\n")
*/

        default: printf("Dest Unreachable, Unknown Code: %d\n",
                    icp->icmp_code);
          break;
      }

      pr_retip((struct ip *)icp->icmp_data);/* Print returned IP header info */
      break;


    /*************************************************************************
     * Source Quench                                                         *
     *************************************************************************/

    case ICMP_SOURCEQUENCH:
      printf("Source Quench\n");
      pr_retip((struct ip *)icp->icmp_data);
      break;

    /*************************************************************************
     * Redirection                                                           *
     *************************************************************************/

    case ICMP_REDIRECT:
      switch(icp->icmp_code)
      {
        ICMPCODE(ICMP_REDIRECT_NET,    "Redirect Network")
        ICMPCODE(ICMP_REDIRECT_HOST,   "Redirect Host")
        ICMPCODE(ICMP_REDIRECT_TOSNET, "Redirect Type of Service and Network")
        ICMPCODE(ICMP_REDIRECT_TOSHOST,"Redirect Type of Service and Host")
        default:
          printf("Redirect, Bad Code: %d",
                 icp->icmp_code);
          break;
      }
      printf("(New addr: 0x%08lx)\n",
             icp->icmp_gwaddr);
      pr_retip((struct ip *)icp->icmp_data);
      break;

    case ICMP_ECHO: printf("Echo Request\n"); /* XXX ID + Seq + Data */
      break;

    case ICMP_TIMXCEED:
      switch(icp->icmp_code)
      {
        ICMPCODE(ICMP_TIMXCEED_INTRANS,"Time to live exceeded\n")
        ICMPCODE(ICMP_TIMXCEED_REASS,  "Frag reassembly time exceeded\n")
        default:
          printf("Time exceeded, Bad Code: %d\n",
                 icp->icmp_code);
          break;
      }
      pr_retip((struct ip *)icp->icmp_data);
      break;

    case ICMP_PARAMPROB:
      printf("Parameter problem: pointer = 0x%02x\n",
             icp->icmp_gwaddr);
      pr_retip((struct ip *)icp->icmp_data);
      break;

    ICMPCODE(ICMP_TSTAMP,     "Timestamp\n")  /* XXX ID + Seq + 3 timestamps */
    ICMPCODE(ICMP_TSTAMPREPLY,"Timestamp Reply\n") /* XXX ID + Seq + 3 tstps */
    ICMPCODE(ICMP_IREQ,       "Information Request\n")       /* XXX ID + Seq */
    ICMPCODE(ICMP_IREQREPLY,  "Information Reply\n")         /* XXX ID + Seq */
    ICMPCODE(ICMP_MASKREQ,    "Address Mask Request\n")
    ICMPCODE(ICMP_MASKREPLY,  "Address Mask Reply\n")
    default:
            printf("Bad ICMP type: %d\n", icp->icmp_type);
  }
}


/*
 * pr_iph --
 * Print an IP header with options.
 */

 /* remark: 20 is the IP header length ! */
void pr_iph(struct ip *ip)
{
  int hlen;
  u_char *cp;

  hlen = ip->ip_hl << 2;
  cp = (u_char *)ip + 20;                   /* point to options */

  printf("\nVr HL TOS  Len ID   Flg  off TTL Pro cks  Src             Dst\n"
         " %1x  %1x  %02x %04x %04x",
         ip->ip_v,
         ip->ip_hl,
         ip->ip_tos,
         ip->ip_len,
         ip->ip_id);

  printf("   %1x %04x",
         ((ip->ip_off) & 0xe000) >> 13,
         (ip->ip_off) & 0x1fff);

  printf("  %02x  %02x %04x",
         ip->ip_ttl,
         ip->ip_p,
         ip->ip_sum);

  printf(" %-15s",
         inet_ntoa(*((struct in_addr *) &ip->ip_src)));

  printf(" %-15s",
         inet_ntoa(*((struct in_addr *) &ip->ip_dst)));

  ToolsDumpHex(0,
               hlen,
               cp);

  putchar('\n');
}

/*
 * pr_addr --
 * Return an ascii host address as a dotted quad and optionally with
 * a hostname.
 */
static char *pr_addr(u_long l)
{
  struct hostent *pHostEntry;
  static char buf[256];

  pHostEntry = gethostbyaddr((char *)&l,
                             4,
                             AF_INET);

  if (Options.fsOptionNumeric || !pHostEntry)
    strcpy(buf,
           inet_ntoa(*(struct in_addr *)&l));
  else
    sprintf(buf,
            "%s (%s)",
            pHostEntry->h_name,
            inet_ntoa(*(struct in_addr *)&l));
  return(buf);
}


/***********************************************************************
 * Name      : void PacketFill
 * Funktion  : Dump some info on a returned (via ICMP) IP packet.
 * Parameter : struckt ip *ip
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Wednesday, 97/06/12]
 ***********************************************************************/

void pr_retip(struct ip *ip)
{
  int    hlen;
  u_char *cp;

  pr_iph(ip);
  hlen = ip->ip_hl << 2;
  cp = (u_char *)ip + hlen;

  if (ip->ip_p == 6)
    printf("TCP: from port %u, to port %u (decimal)\n",
           (*cp * 256 + *(cp + 1)),
           (*(cp + 2) * 256 + *(cp + 3)));
  else
    if (ip->ip_p == 17)
      printf("UDP: from port %u, to port %u (decimal)\n",
             (*cp * 256 + *(cp + 1)),
             (*(cp + 2) * 256 + *(cp + 3)));
}


/***********************************************************************
 * Name      : void PacketFill
 * Funktion  : Fill packet with a pattern
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Wednesday, 97/06/12]
 ***********************************************************************/

void PacketFill(void *bp1,
                char *patp)
{
  int  ii,
       jj,
       kk;
  int  pat[16];
  char *cp,
       *bp = (char *)bp1;

  for (cp = patp;
       *cp;
       cp++)
    if (!isxdigit(*cp))
    {
      fprintf(stderr,
              "ping: patterns must be specified as hex digits.\n");
      exit(2);
    }

  ii = sscanf(patp,
              "%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",
              &pat[0],  &pat[1],  &pat[2], &pat[3],  &pat[4],  &pat[5], &pat[6],
              &pat[7],  &pat[8],  &pat[9], &pat[10], &pat[11], &pat[12],
              &pat[13], &pat[14], &pat[15]);

  if (ii > 0)
    for (kk = 0;
         kk <= MAXPACKET - (8 + ii);
         kk += ii)
      for (jj = 0;
           jj < ii;
           ++jj)
        bp[jj + kk] = pat[jj];

  if (!Options.fsOptionQuiet)                /* shall we print the pattern ? */
  {
    printf("PATTERN: 0x");

    for (jj = 0;
         jj < ii;
         ++jj)
      printf("%02x",
             bp[jj] & 0xFF);

    printf("\n");
  }
}


/***********************************************************************
 * Name      : void initialize
 * Funktion  : Initialisierung einiger Variablen
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

APIRET initialize (void)
{
  memset(&Options,
         0L,
         sizeof(Options));

  memset(&Globals,
         0L,
         sizeof(Globals));

  sock_init();                                    /* initialize TCP/IP stack */

  Globals.mx_dup_ck = MAX_DUP_CHK;
  Options.usSize   = DEFDATALEN;
  Globals.BSPACE    = '\b';                  /* characters written for flood */
  Globals.DOT       = '.';
  Options.ulInterval  = 1000;                    /* interval between packets */
  Globals.fTimeMin      = 1000000.0;              /* minimum round trip time */
  Globals.fTimeMax      = 0.0;                    /* maximum round trip time */

  Globals.fThreadQuit = FALSE;                       /* used for thread sync */

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : int main
 * Funktion  : Hauptroutine
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 ***********************************************************************/

int main (int argc, char *argv[])
{
  int rc;                                                    /* RБckgabewert */

  rc = initialize ();                                     /* Initialisierung */
  if (rc != NO_ERROR)
  {
    cmd_ShowSystemMessage(rc, 0L);                    /* print error message */
    exit(1);                                                /* abort program */
  }

  rc = ArgStandard (argc,                          /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    cmd_ShowSystemMessage(rc, 0L);                    /* print error message */
    exit(1);                                                /* abort program */
  }

  if ( Options.fsHelp )                                /* user requests help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }


  if (Options.fsOptionFlood)                     /* if we're about to flood, */
     setvbuf(stdout,                             /* disable stream buffering */
             NULL,
             _IONBF,
             0);

                                           /* perform some parameter mapping */
  rc = NetPing();                                    /* let's ping again ... */
  if (rc != NO_ERROR)
    cmd_ShowSystemMessage(rc, 0L);

  return (rc);
}
