/*****************************************************
 * Traceroute Tool                                   *
 * Traces TCP/IP packets to their target destination.*
 * (c) 1997    Patrick Haller Systemtechnik          *
 *****************************************************/

/* This program was based on the following: */

/* Ported to OS/2 by wouter.cloetens@ping.be */

/*-
 * Copyright (c) 1990, 1993
 * The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Van Jacobson.
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
 * traceroute host  - trace the route ip packets follow going to "host".
 *
 * Attempt to trace the route an ip packet would follow to some
 * internet host.  We find out intermediate hops by launching probe
 * packets with a small ttl (time to live) then listening for an
 * icmp "time exceeded" reply from a gateway.  We start our probes
 * with a ttl of one and increase by one until we get an icmp "port
 * unreachable" (which means we got to "host") or hit a max (which
 * defaults to 30 hops & can be changed with the -m flag).  Three
 * probes (change with -q flag) are sent at each ttl setting and a
 * line is printed showing the ttl, address of the gateway and
 * round trip time of each probe.  If the probe answers come from
 * different gateways, the address of each responding system will
 * be printed.  If there is no response within a 5 sec. timeout
 * interval (changed with the -w flag), a "*" is printed for that
 * probe.
 *
 * Probe packets are UDP format.  We don't want the destination
 * host to process them so the destination port is set to an
 * unlikely value (if some clod on the destination is using that
 * value, it can be changed with the -p flag).
 *
 * A sample use might be:
 *
 *     [yak 71]% traceroute nis.nsf.net.
 *     traceroute to nis.nsf.net (35.1.1.48), 30 hops max, 56 byte packet
 *      1  helios.ee.lbl.gov (128.3.112.1)  19 ms  19 ms  0 ms
 *      2  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  39 ms  19 ms
 *      3  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  39 ms  19 ms
 *      4  ccngw-ner-cc.Berkeley.EDU (128.32.136.23)  39 ms  40 ms  39 ms
 *      5  ccn-nerif22.Berkeley.EDU (128.32.168.22)  39 ms  39 ms  39 ms
 *      6  128.32.197.4 (128.32.197.4)  40 ms  59 ms  59 ms
 *      7  131.119.2.5 (131.119.2.5)  59 ms  59 ms  59 ms
 *      8  129.140.70.13 (129.140.70.13)  99 ms  99 ms  80 ms
 *      9  129.140.71.6 (129.140.71.6)  139 ms  239 ms  319 ms
 *     10  129.140.81.7 (129.140.81.7)  220 ms  199 ms  199 ms
 *     11  nic.merit.edu (35.1.1.48)  239 ms  239 ms  239 ms
 *
 * Note that lines 2 & 3 are the same.  This is due to a buggy
 * kernel on the 2nd hop system -- lbl-csam.arpa -- that forwards
 * packets with a zero ttl.
 *
 * A more interesting example is:
 *
 *     [yak 72]% traceroute allspice.lcs.mit.edu.
 *     traceroute to allspice.lcs.mit.edu (18.26.0.115), 30 hops max
 *      1  helios.ee.lbl.gov (128.3.112.1)  0 ms  0 ms  0 ms
 *      2  lilac-dmc.Berkeley.EDU (128.32.216.1)  19 ms  19 ms  19 ms
 *      3  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  19 ms  19 ms
 *      4  ccngw-ner-cc.Berkeley.EDU (128.32.136.23)  19 ms  39 ms  39 ms
 *      5  ccn-nerif22.Berkeley.EDU (128.32.168.22)  20 ms  39 ms  39 ms
 *      6  128.32.197.4 (128.32.197.4)  59 ms  119 ms  39 ms
 *      7  131.119.2.5 (131.119.2.5)  59 ms  59 ms  39 ms
 *      8  129.140.70.13 (129.140.70.13)  80 ms  79 ms  99 ms
 *      9  129.140.71.6 (129.140.71.6)  139 ms  139 ms  159 ms
 *     10  129.140.81.7 (129.140.81.7)  199 ms  180 ms  300 ms
 *     11  129.140.72.17 (129.140.72.17)  300 ms  239 ms  239 ms
 *     12  * * *
 *     13  128.121.54.72 (128.121.54.72)  259 ms  499 ms  279 ms
 *     14  * * *
 *     15  * * *
 *     16  * * *
 *     17  * * *
 *     18  ALLSPICE.LCS.MIT.EDU (18.26.0.115)  339 ms  279 ms  279 ms
 *
 * (I start to see why I'm having so much trouble with mail to
 * MIT.)  Note that the gateways 12, 14, 15, 16 & 17 hops away
 * either don't send ICMP "time exceeded" messages or send them
 * with a ttl too small to reach us.  14 - 17 are running the
 * MIT C Gateway code that doesn't send "time exceeded"s.  God
 * only knows what's going on with 12.
 *
 * The silent gateway 12 in the above may be the result of a bug in
 * the 4.[23]BSD network code (and its derivatives):  4.x (x <= 3)
 * sends an unreachable message using whatever ttl remains in the
 * original datagram.  Since, for gateways, the remaining ttl is
 * zero, the icmp "time exceeded" is guaranteed to not make it back
 * to us.  The behavior of this bug is slightly more interesting
 * when it appears on the destination system:
 *
 *      1  helios.ee.lbl.gov (128.3.112.1)  0 ms  0 ms  0 ms
 *      2  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  19 ms  39 ms
 *      3  lilac-dmc.Berkeley.EDU (128.32.216.1)  19 ms  39 ms  19 ms
 *      4  ccngw-ner-cc.Berkeley.EDU (128.32.136.23)  39 ms  40 ms  19 ms
 *      5  ccn-nerif35.Berkeley.EDU (128.32.168.35)  39 ms  39 ms  39 ms
 *      6  csgw.Berkeley.EDU (128.32.133.254)  39 ms  59 ms  39 ms
 *      7  * * *
 *      8  * * *
 *      9  * * *
 *     10  * * *
 *     11  * * *
 *     12  * * *
 *     13  rip.Berkeley.EDU (128.32.131.22)  59 ms !  39 ms !  39 ms !
 *
 * Notice that there are 12 "gateways" (13 is the final
 * destination) and exactly the last half of them are "missing".
 * What's really happening is that rip (a Sun-3 running Sun OS3.5)
 * is using the ttl from our arriving datagram as the ttl in its
 * icmp reply.  So, the reply will time out on the return path
 * (with no notice sent to anyone since icmp's aren't sent for
 * icmp's) until we probe with a ttl that's at least twice the path
 * length.  I.e., rip is really only 7 hops away.  A reply that
 * returns with a ttl of 1 is a clue this problem exists.
 * Traceroute prints a "!" after the time if the ttl is <= 1.
 * Since vendors ship a lot of obsolete (DEC's Ultrix, Sun 3.x) or
 * non-standard (HPUX) software, expect to see this problem
 * frequently and/or take care picking the target host of your
 * probes.
 *
 * Other possible annotations after the time are !H, !N, !P (got a host,
 * network or protocol unreachable, respectively), !S or !F (source
 * route failed or fragmentation needed -- neither of these should
 * ever occur and the associated gateway is busted if you see one).  If
 * almost all the probes result in some kind of unreachable, traceroute
 * will give up and exit.
 *
 * Notes
 * -----
 * This program must be run by root or be setuid.  (I suggest that
 * you *don't* make it setuid -- casual use could result in a lot
 * of unnecessary traffic on our poor, congested nets.)
 *
 * This program requires a kernel mod that does not appear in any
 * system available from Berkeley:  A raw ip socket using proto
 * IPPROTO_RAW must interpret the data sent as an ip datagram (as
 * opposed to data to be wrapped in a ip datagram).  See the README
 * file that came with the source to this program for a description
 * of the mods I made to /sys/netinet/raw_ip.c.  Your mileage may
 * vary.  But, again, ANY 4.x (x < 4) BSD KERNEL WILL HAVE TO BE
 * MODIFIED TO RUN THIS PROGRAM.
 *
 * The udp port usage may appear bizarre (well, ok, it is bizarre).
 * The problem is that an icmp message only contains 8 bytes of
 * data from the original datagram.  8 bytes is the size of a udp
 * header so, if we want to associate replies with the original
 * datagram, the necessary information must be encoded into the
 * udp header (the ip id could be used but there's no way to
 * interlock with the kernel's assignment of ip id's and, anyway,
 * it would have taken a lot more kernel hacking to allow this
 * code to set the ip id).  So, to allow two or more users to
 * use traceroute simultaneously, we use this task's pid as the
 * source port (the high bit is set to move the port number out
 * of the "likely" range).  To keep track of which probe is being
 * replied to (so times and/or hop counts don't get confused by a
 * reply that was delayed in transit), we increment the destination
 * port number before each probe.
 *
 * Don't use this as a coding example.  I was trying to find a
 * routing problem and this code sort-of popped out after 48 hours
 * without sleep.  I was amazed it ever compiled, much less ran.
 *
 * I stole the idea for this program from Steve Deering.  Since
 * the first release, I've learned that had I attended the right
 * IETF working group meetings, I also could have stolen it from Guy
 * Almes or Matt Mathis.  I don't know (or care) who came up with
 * the idea first.  I envy the originators' perspicacity and I'm
 * glad they didn't keep the idea a secret.
 *
 * Tim Seaver, Ken Adelman and C. Philip Wood provided bug fixes and/or
 * enhancements to the original distribution.
 *
 * I've hacked up a round-trip-route version of this that works by
 * sending a loose-source-routed udp datagram through the destination
 * back to yourself.  Unfortunately, SO many gateways botch source
 * routing, the thing is almost worthless.  Maybe one day...
 *
 *  -- Van Jacobson (van@helios.ee.lbl.gov)
 *     Tue Dec 20 03:50:13 PST 1988
 */


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>

  #pragma pack(1)

//  #include <arpa/inet.h>
//  #include <unistd.h>

  #define TCPV40HDRS
  #include <types.h>
  #include <netinet/in_systm.h>
  #include <netinet/in.h>
  #include <netinet/ip.h>
  #include <netinet/ip_icmp.h>
  #include <netinet/udp.h>
  #include <sys/socket.h>
  #define BSD_SELECT
  #include <sys/select.h>
  #include <sys/types.h>
  #include <netdb.h>
  #include <nerrno.h>
  #include <net/if_arp.h>
  #include <utils.h>

  #undef SO_SNDBUF

#endif

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <process.h>
#include <timeval.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"

#ifndef MAXPATHLEN
  #define MAXPATHLEN 260
#endif


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef int SOCKET;
typedef SOCKET *PSOCKET;


typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsHost;                            /* host IP or address specified */
  ARGFLAG fsNumeric;   /* numeric mode requested, don't resolve IP addresses */
  ARGFLAG fsDontRoute;                       /* bypass normal routing tables */
  ARGFLAG fsDebug;                            /* socket debug mode requested */
  ARGFLAG fsVerbose;                          /* verbose mode      requested */
  ARGFLAG fsSource;                                /* special source address */
  ARGFLAG fsPort;                           /* special port number requested */
  ARGFLAG fsSize;            /* special size for the probe packets specified */
  ARGFLAG fsTOS;                                /* type-of-service specified */
  ARGFLAG fsTTL;                                /* time to live    specified */
  ARGFLAG fsQueries;                      /* number of probe packets to send */
  ARGFLAG fsWait;                        /* how long to wait between packets */

  PSZ     pszHost;                           /* host IP or address specified */
  PSZ     pszSource;                               /* special source address */
  USHORT  usPort;                           /* special port number requested */
  USHORT  usSize;            /* special size for the probe packets specified */
  UCHAR   ucTOS;                                /* type-of-service specified */
  UCHAR   ucTTL;                                /* time to live    specified */
  ULONG   ulQueries;                      /* number of probe packets to send */
  ULONG   ulWait;                        /* how long to wait between packets */
} OPTIONS, *POPTIONS;

struct udphdr
{
  USHORT uh_sport; /* source port */
  USHORT uh_dport; /* destination port */
  USHORT uh_ulen;  /* length of datagram */
  USHORT uh_sum;   /* checksum */
};

typedef struct _PacketProbe
{
  struct ip     IP;                                             /* IP packet */
  struct udphdr UDP;                                  /* ICMP part of packet */

  ULONG      ulSequence;                                  /* sequence number */
  UCHAR      ucTimeToLive;                                            /* TTL */
  PERFSTRUCT psSent;         /* exact timer information when packet was sent */
} PACKETPROBE, *PPACKETPROBE;


typedef struct _Globals
{
  u_char PacketBuffer[512];                     /* last inbound (icmp) packet */
  PPACKETPROBE pPacketOutput;                   /* last output (udp) packet */
  int           sockRecv;           /* receive (icmp) socket file descriptor */
  int           sockSend;               /* send (udp) socket file descriptor */
  struct timezone tz;                                                        /* leftover */
  struct sockaddr saTarget;                           /* Who to try to reach */
  PSZ           pszHostname;
  PID           pidIdentifier;                     /* our process identifier */
} GLOBALS, *PGLOBALS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/PORT=",     "Port number.",       &Options.usPort,      ARG_USHORT,     &Options.fsPort},
  {"/SIZE=",     "Size of the probe packets.",
                                       &Options.usSize,      ARG_USHORT,     &Options.fsSize},
  {"/TOS=",      "Code for Type-of-"
                 "service. (0..255)",  &Options.ucTOS,       ARG_UCHAR,      &Options.fsTOS},
  {"/TTL=",      "Time to live for "
                 "the ICMP packets.",  &Options.ucTTL,       ARG_UCHAR,      &Options.fsTTL},
  {"/QUERIES=",  "Number of queries.", &Options.ulQueries,   ARG_ULONG,      &Options.fsQueries},
  {"/WAIT=",     "Wait between probe "
                 "packets.",           &Options.ulWait,      ARG_ULONG,      &Options.fsWait},
  {"/SOURCE=",   "Source address.",    &Options.pszSource,   ARG_PSZ,        &Options.fsSource},
  {"/DEBUG",     "Debug mode.",        NULL,                 ARG_NULL,       &Options.fsDebug},
  {"/D",         "Debug mode.",        NULL,                 ARG_NULL |
                                                             ARG_HIDDEN,     &Options.fsDebug},
  {"/VERBOSE",   "Verbose mode.",      NULL,                 ARG_NULL,       &Options.fsVerbose},
  {"/V",         "Verbose mode.",      NULL,                 ARG_NULL |
                                                             ARG_HIDDEN,     &Options.fsVerbose},
  {"/!ROUTE",    "Disable normal rouing tables.",
                                       NULL,                 ARG_NULL,       &Options.fsDontRoute},
  {"/NUMERIC",   "Numeric mode, don't resolve"
                 "IPs to hostnames.",  NULL,                 ARG_NULL,       &Options.fsNumeric},
  {"1",          "Start with this "
                 "IP address.",        &Options.pszHost,     ARG_PSZ     |
                                                             ARG_DEFAULT |
                                                             ARG_MUST,       &Options.fsHost},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help               (void);

APIRET IPTraceRoute       (void);

APIRET IPWaitForReply     (SOCKET             sock,
                           struct sockaddr_in *from,
                           int                reset_timer);

APIRET IPSendProbe        (int seq,
                           int ttl);

PSZ    IPICMPPacketType   (UCHAR ucICMPType);

int    IPICMPPacketCheck  (u_char             *buf,
                           int                cc,
                           struct sockaddr_in *from,
                           int                seq);

void   IPPacketHeaderPrint(u_char             *buf,
                           int                cc,
                           struct sockaddr_in *from);

PSZ    IPInterNetName     (struct in_addr in);

APIRET initialize         (void);

int    main               (int,
                           char **);


/*****************************************************************************
 * Name      : void help
 * Funktion  : Anzeigen des Hilfetextes.
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 *****************************************************************************/

void help (void)
{
  TOOLVERSION("TraceRoute",                             /* application name */
              0x00010001,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


#define  MAXPACKET                       65535                        /* max ip packet size */


/*****************************************************************************
 * Name      : APIRET IPTraceRoute
 * Funktion  : This was the former main() of the original TraceRoute.C
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 *****************************************************************************/

APIRET IPTraceRoute(void)
{
  struct hostent *hp;
  struct protoent *pe;
  struct sockaddr_in from, *to;
  int    ch,
         i,
         on,
         probe,
         seq,
         tos,
         ttl;

  on = 1;
  seq = tos = 0;
  to = (struct sockaddr_in *)&Globals.saTarget;

  memset (&Globals.saTarget,        /* zero out the target address structure */
    0,
    sizeof(Globals.saTarget));

  to->sin_family      = AF_INET;
  to->sin_addr.s_addr = inet_addr(Options.pszHost);

  if (to->sin_addr.s_addr != -1)
    Globals.pszHostname = Options.pszHost;
  else
  {
    hp = gethostbyname(Options.pszHost);
    if (hp)
    {
      to->sin_family = hp->h_addrtype;
      bcopy(hp->h_addr, (caddr_t)&to->sin_addr, hp->h_length);
      Globals.pszHostname = (char *)hp->h_name;
    }
    else
    {
      fprintf(stderr,
              "traceroute: unknown host %s\n",
              Options.pszHost);
      exit(1);
    }
  }

  if (Options.usSize >= MAXPACKET - sizeof(PACKETPROBE))
  {
    fprintf(stderr,
            "traceroute: packet size must be 0 <= s < %d.\n",
            MAXPACKET - sizeof(PACKETPROBE));
    exit(1);
  }

  Options.usSize += sizeof(PACKETPROBE);
  Globals.pPacketOutput = (PPACKETPROBE )malloc((unsigned)Options.usSize);
  if (! Globals.pPacketOutput)
  {
    perror("traceroute: malloc");
    exit(1);
  }

  (void) bzero((char *)Globals.pPacketOutput, Options.usSize);
  Globals.pPacketOutput->IP.ip_dst = to->sin_addr;
  Globals.pPacketOutput->IP.ip_tos = tos;
  Globals.pPacketOutput->IP.ip_v = IPVERSION;
  Globals.pPacketOutput->IP.ip_id = 0;

  Globals.pidIdentifier = (getpid() & 0xffff) | 0x8000;



   if ((pe = getprotobyname("icmp")) == NULL) {
      fprintf(stderr, "icmp: unknown protocol\n");
      exit(10);
   }
   if ((Globals.sockRecv = socket(AF_INET, SOCK_RAW, pe->p_proto)) < 0) {
      perror("traceroute: icmp socket");
      exit(5);
   }
   if (Options.fsDebug)
      (void) setsockopt(Globals.sockRecv, SOL_SOCKET, SO_DEBUG,
              (char *)&on, sizeof(on));
   if (Options.fsDontRoute)
      (void) setsockopt(Globals.sockRecv, SOL_SOCKET, SO_DONTROUTE,
              (char *)&on, sizeof(on));

   if ((Globals.sockSend = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
      perror("traceroute: raw socket");
      exit(5);
   }
#ifdef SO_SNDBUF
   if (Options.fsSize)
   {
     if (setsockopt(Globals.sockSend, SOL_SOCKET, SO_SNDBUF, (char *)&Options.usSize,
               sizeof(Options.usSize)) < 0) {
        perror("traceroute: SO_SNDBUF");
        exit(6);
     }
   }
#endif /* SO_SNDBUF */
#ifdef IP_HDRINCL
   if (setsockopt(Globals.sockSend, IPPROTO_IP, IP_HDRINCL, (char *)&on,
             sizeof(on)) < 0) {
      perror("traceroute: IP_HDRINCL");
      exit(6);
   }
#endif /* IP_HDRINCL */
   if (Options.fsDebug)
      (void) setsockopt(Globals.sockSend, SOL_SOCKET, SO_DEBUG,
              (char *)&on, sizeof(on));
   if (Options.fsDontRoute)
      (void) setsockopt(Globals.sockSend, SOL_SOCKET, SO_DONTROUTE,
              (char *)&on, sizeof(on));

   if (Options.pszSource) {
      (void) bzero((char *)&from, sizeof(struct sockaddr));
      from.sin_family = AF_INET;
      from.sin_addr.s_addr = inet_addr(Options.pszSource);
      if (from.sin_addr.s_addr == -1) {
         printf("traceroute: unknown host %s\n", Options.pszSource);
         exit(1);
      }
      Globals.pPacketOutput->IP.ip_src = from.sin_addr;
#ifndef IP_HDRINCL
      if (bind(Globals.sockSend, (struct sockaddr *)&from, sizeof(from)) < 0) {
         perror ("traceroute: bind:");
         exit (1);
      }
#endif /* IP_HDRINCL */
   }

  fprintf(stdout,
          "traceroute to %s (%s)",
          Globals.pszHostname,
           inet_ntoa(to->sin_addr));

  if (Options.pszSource)
    fprintf(stdout,
            " from %s",
            Options.pszSource);

  fprintf(stdout,
          ", %d hops max, %d byte packets\n",
          Options.ucTTL,
          Options.usSize);
  fflush(stderr);

  for (ttl = 1;
       ttl <= Options.ucTTL;
       ++ttl)
  {
    u_long lastaddr    = 0;
    int    got_there   = 0;
    int    unreachable = 0;

    printf("%2d",                                       /* print hop counter */
           ttl);

    for (probe = 0;
         probe < Options.ulQueries;
         ++probe)
    {
      PERFSTRUCT psTime1;
      PERFSTRUCT psTime2;

      int cc,
          reset_timer;

      struct ip *ip;

      ToolsPerfQuery(&psTime1);

      IPSendProbe(++seq,
                  ttl);

      reset_timer = 1;

      while ((cc = IPWaitForReply(Globals.sockRecv,
                                  &from,
                                  reset_timer)) != 0)
      {
        ToolsPerfQuery(&psTime2);

        i = IPICMPPacketCheck(Globals.PacketBuffer,
                                   cc,
                                   &from,
                                   seq);
        if (i != 0)
        {
          reset_timer = 1;
          if (from.sin_addr.s_addr != lastaddr)
          {
            IPPacketHeaderPrint(Globals.PacketBuffer,
                                cc,
                                &from);
            lastaddr = from.sin_addr.s_addr;
          }

          printf(" %7.2fms",
                 (psTime2.fSeconds - psTime1.fSeconds) * 1000);

          switch(i - 1)
          {
            case ICMP_UNREACH_PORT:
  #ifndef ARCHAIC
              ip = (struct ip *)Globals.PacketBuffer;
              if (ip->ip_ttl <= 1)
                printf(" !");
  #endif /* ARCHAIC */
              ++got_there;
              break;

            case ICMP_UNREACH_NET:
              ++unreachable;
              printf(" !N");
              break;

            case ICMP_UNREACH_HOST:
              ++unreachable;
              printf(" !H");
              break;

            case ICMP_UNREACH_PROTOCOL:
              ++got_there;
              printf(" !P");
              break;

            case ICMP_UNREACH_NEEDFRAG:
              ++unreachable;
              printf(" !F");
              break;

            case ICMP_UNREACH_SRCFAIL:
              ++unreachable;
              printf(" !S");
              break;
          }
          break;
        }
        else
          reset_timer = 0;
      }

      if (cc == 0)
        printf(" *");

      fflush(stdout);
    }

    putchar('\n');
    if (got_there || unreachable >= Options.ulQueries-1)
       exit(0);
  }

  return 0;
}


/*****************************************************************************
 * Name      : APIRET IPWaitForReply
 * Funktion  : wait for a reply packet
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 *****************************************************************************/

APIRET IPWaitForReply(SOCKET             sock,
                      struct sockaddr_in *from,
                      int                reset_timer)
{
  fd_set                fds;
  static struct timeval wait;
  int                   cc = 0;
  int                   fromlen = sizeof (*from);

  FD_ZERO(&fds);
  FD_SET (sock,
          &fds);

  if (reset_timer)
  {
    /*
     * traceroute could hang if someone else has a ping
     * running and our ICMP reply gets dropped but we don't
     * realize it because we keep waking up to handle those
     * other ICMP packets that keep coming in.  To fix this,
     * "reset_timer" will only be true if the last packet that
     * came in was for us or if this is the first time we're
     * waiting for a reply since sending out a probe.  Note
     * that this takes advantage of the select() feature on
     * Linux where the remaining timeout is written to the
     * struct timeval area.
     */
    wait.tv_sec  = Options.ulWait;
    wait.tv_usec = 0;
  }

  if (select(sock+1,
             &fds,
             (fd_set *)0,
             (fd_set *)0,
             &wait) > 0)
    cc=recvfrom(Globals.sockRecv,
                (char *)Globals.PacketBuffer,
                sizeof(Globals.PacketBuffer),
                0,
                (struct sockaddr *)from,
                &fromlen);

  return(cc);
}


/*****************************************************************************
 * Name      : APIRET IPSendProbe
 * Funktion  : Send a probe packet to the client
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 *****************************************************************************/

APIRET IPSendProbe(int seq,
                   int ttl)
{
  PPACKETPROBE   op = Globals.pPacketOutput;
  struct ip     *ip = &op->IP;
  struct udphdr *up = &op->UDP;
  int            i;

  ip->ip_off = 0;
  ip->ip_hl  = sizeof(*ip) >> 2;
  ip->ip_p   = IPPROTO_UDP;
  ip->ip_len = Options.usSize;
  ip->ip_ttl = ttl;
  ip->ip_v   = IPVERSION;
  ip->ip_id  = htons(Globals.pidIdentifier+seq);

  up->uh_sport = htons(Globals.pidIdentifier);
  up->uh_dport = htons(Options.usPort+seq);
  up->uh_ulen  = htons((u_short)(Options.usSize - sizeof(struct ip)));
  up->uh_sum   = 0;

  op->ulSequence   = seq;
  op->ucTimeToLive = ttl;

  ToolsPerfQuery(&op->psSent);                /* put timestamp in the packet */

  i = sendto(Globals.sockSend,
             (char *)Globals.pPacketOutput,
             Options.usSize,
             0,
             &Globals.saTarget,
             sizeof(struct sockaddr));

  if (i < 0 || i != Options.usSize)
  {
    if (i<0)
      perror("sendto");

    printf("traceroute: wrote %s %d chars, ret=%d\n",
           Globals.pszHostname,
           Options.usSize,
           i);

    fflush(stdout);
  }

  return (NO_ERROR);                                                   /* OK */
}


/*****************************************************************************
 * Name      : PSZ IPICMPPacketType
 * Funktion  : return a descriptive string of the ICMP packet type
 * Parameter : UCHAR ucICMPType
 * Variablen :
 * Ergebnis  : PSZ pszICMPString
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 *****************************************************************************/

PSZ IPICMPPacketType(UCHAR ucICMPType)
{
  static char *ttab[] =
  {
    "Echo Reply",
    "ICMP 1",
    "ICMP 2",
    "Dest Unreachable",
    "Source Quench",
    "Redirect",
    "ICMP 6",
    "ICMP 7",
    "Echo",
    "ICMP 9",
    "ICMP 10",
    "Time Exceeded",
    "Param Problem",
    "Timestamp",
    "Timestamp Reply",
    "Info Request",
    "Info Reply"
  };

  if(ucICMPType > 16)
    return("OUT-OF-RANGE");

  return(ttab[ucICMPType]);
}


/*****************************************************************************
 * Name      : PSZ IPICMPPacketType
 * Funktion  : return a descriptive string of the ICMP packet type
 * Parameter : UCHAR ucICMPType
 * Variablen :
 * Ergebnis  : PSZ pszICMPString
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 *****************************************************************************/

int IPICMPPacketCheck(u_char             *buf,
                      int                cc,
                      struct sockaddr_in *from,
                      int                seq)
{
   struct icmp *icp;
   u_char type, code;
   int hlen;
#ifndef ARCHAIC
   struct ip *ip;

   ip = (struct ip *) buf;
   hlen = ip->ip_hl << 2;
   if (cc < hlen + ICMP_MINLEN) {
      if (Options.fsVerbose)
         printf("packet too short (%d bytes) from %s\n", cc,
            inet_ntoa(from->sin_addr));
      return (0);
   }
   cc -= hlen;
   icp = (struct icmp *)(buf + hlen);
#else
   icp = (struct icmp *)buf;
#endif /* ARCHAIC */
   type = icp->icmp_type; code = icp->icmp_code;
   if ((type == ICMP_TIMXCEED && code == ICMP_TIMXCEED_INTRANS) ||
       type == ICMP_UNREACH) {
      struct ip *hip;
      struct udphdr *up;

      hip = &icp->icmp_ip;
      hlen = hip->ip_hl << 2;
      up = (struct udphdr *)((u_char *)hip + hlen);
      if (hlen + 12 <= cc && hip->ip_p == IPPROTO_UDP &&
          up->uh_sport == htons(Globals.pidIdentifier) &&
          up->uh_dport == htons(Options.usPort+seq))
         return (type == ICMP_TIMXCEED? -1 : code+1);
   }
#ifndef ARCHAIC
   if (Options.fsVerbose) {
      int i;
      u_long *lp = (u_long *)&icp->icmp_ip;

      printf("\n%d bytes from %s to %s", cc,
         inet_ntoa(from->sin_addr), inet_ntoa(ip->ip_dst));
      printf(": icmp type %d (%s) code %d\n", type, IPICMPPacketType(type),
             icp->icmp_code);
      for (i = 4; i < cc ; i += sizeof(long))
         printf("%2d: x%8.8lx\n", i, *lp++);
   }
#endif /* ARCHAIC */
   return(0);
}


/*****************************************************************************
 * Name      : PSZ IPICMPPacketType
 * Funktion  : return a descriptive string of the ICMP packet type
 * Parameter : UCHAR ucICMPType
 * Variablen :
 * Ergebnis  : PSZ pszICMPString
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 *****************************************************************************/

void IPPacketHeaderPrint(u_char             *buf,
                         int                cc,
                         struct sockaddr_in *from)
{
  struct ip *ip;
  int       hlen;

  ip = (struct ip *) buf;
  hlen = ip->ip_hl << 2;
  cc -= hlen;

  if (Options.fsNumeric)
    printf(" %-15s",
           inet_ntoa(from->sin_addr));
  else
    printf(" %-39s(%-15s)",
           IPInterNetName(from->sin_addr),
           inet_ntoa(from->sin_addr));

  if (Options.fsVerbose)
    printf (" %d bytes to %s",
            cc,
            inet_ntoa (ip->ip_dst));
}


/*****************************************************************************
 * Name      : PSZ IPICMPPacketType
 * Funktion  : return a descriptive string of the ICMP packet type
 * Parameter : UCHAR ucICMPType
 * Variablen :
 * Ergebnis  : PSZ pszICMPString
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 *****************************************************************************/


/*
 * Construct an Internet address representation.
 * If the nflag has been supplied, give
 * numeric value, otherwise try for symbolic name.
 */

PSZ IPInterNetName(struct in_addr in)
{
  char           *cp;
  static char    line[50];
  struct hostent *hp;
  static char    domain[MAXHOSTNAMELEN + 1];
  static int     first = 1;

  if (first && !Options.fsNumeric)
  {
    first = 0;
    cp = strchr(domain, '.');
    if (gethostname(domain,
                    MAXHOSTNAMELEN) == 0 && cp)
      strcpy(domain, cp + 1);
    else
      domain[0] = 0;
  }

  cp = 0;
  if (!Options.fsNumeric && in.s_addr != INADDR_ANY)
  {
    hp = gethostbyaddr((char *)&in,
                       sizeof (in),
                       AF_INET);
    if (hp)
    {
      cp = strchr(hp->h_name, '.');

      if (cp && !strcmp(cp + 1, domain))
        *cp = 0;

      cp = (char *)hp->h_name;
    }
  }

  if (cp)
    strcpy(line,
           cp);
  else
  {
    in.s_addr = ntohl(in.s_addr);
#define C(x)   ((x) & 0xff)
    sprintf(line,
            "%lu.%lu.%lu.%lu",
            C(in.s_addr >> 24),
            C(in.s_addr >> 16),
            C(in.s_addr >> 8),
            C(in.s_addr));
  }

  return (line);
}




/*****************************************************************************
 * Name      : void initialize
 * Funktion  : Initialisierung einiger Variablen
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET initialize (void)
{
  memset(&Options,
         0L,
         sizeof(Options));

  memset(&Globals,
         0L,
         sizeof(Globals));

  sock_init();                                    /* initialize TCP/IP stack */

  Options.ulQueries = 2;                                   /* default values */
  Options.ucTTL     = 30;
  Options.usPort    = 32768+666;  /* start udp dest port # for probe packets */
  Options.ulWait    = 4;           /* time to wait for response (in seconds) */

  return (NO_ERROR);                                                   /* OK */
}


/*****************************************************************************
 * Name      : int main
 * Funktion  : Hauptroutine
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

int main (int argc, char *argv[])
{
  int rc;                                                    /* RÅckgabewert */

  rc = initialize ();                                     /* Initialisierung */
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  rc = ArgStandard (argc,                          /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if ( Options.fsHelp )                                /* user requests help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }


  /* @@@PH .. */
  rc = IPTraceRoute();
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
