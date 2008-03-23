/*************************************************************************
	@(#)MSWinSock.h	1.2
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

**************************************************************************/
 
/********************************************************************
**
**   Clone of MS-Windows Winsock.h file.  The only difference is
**   that in this include file the winsock functions get mapped to 
**   other names.  For example, gethostbyaddr() is mapped to
**   Gethostbyaddr() and WSAAsyncSelect() is mapped to 
**   TWIN_WSAAsyncSelect().
**
**   At some point when we move all winsock calls to the driver layer,
**   this could become winsock.h and the functions getted mapped to
**   driver calls.
**
*********************************************************************/

#ifndef MSWinSock_h
#define MSWinSock_h

#include <windows.h>

typedef unsigned char  u_char;
typedef unsigned short u_short;
typedef unsigned int   u_int;
typedef unsigned long  u_long;

typedef u_int SOCKET;

#ifndef FD_SETSIZE
#define FD_SETSIZE      64
#endif 

typedef struct fd_set 
{
  u_int   fd_count;               /* How many SET? */
  SOCKET  fd_array[FD_SETSIZE];   /* Array of SOCKETs. */
} 
fd_set;

#ifdef __cplusplus
extern "C" {
#endif

extern int __WSAFDIsSet(SOCKET, fd_set FAR *);

#ifdef __cplusplus
}
#endif

#define FD_CLR(fd, set) do { \
    u_int __i; \
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count ; __i++) { \
        if (((fd_set FAR *)(set))->fd_array[__i] == fd) { \
            while (__i < ((fd_set FAR *)(set))->fd_count-1) { \
                ((fd_set FAR *)(set))->fd_array[__i] = \
                    ((fd_set FAR *)(set))->fd_array[__i+1]; \
                __i++; \
            } \
            ((fd_set FAR *)(set))->fd_count--; \
            break; \
        } \
    } \
} while(0)

#define FD_SET(fd, set) do { \
    if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) \
        ((fd_set FAR *)(set))->fd_array[((fd_set FAR *)(set))->fd_count++]=fd;\
} while(0)

#define FD_ZERO(set) (((fd_set FAR *)(set))->fd_count=0)

#define FD_ISSET(fd, set) __WSAFDIsSet((SOCKET)fd, (fd_set FAR *)set)

/* Used in select() call. */
struct timeval 
{
  long tv_sec;  /* Seconds. */
  long tv_usec; /* Microseconds. */
};

/* Operations on timevals. */
#define timerisset(tvp) ((tvp)->tv_sec || (tvp)->tv_usec)

#define timercmp(tvp, uvp, cmp) \
        ((tvp)->tv_sec cmp (uvp)->tv_sec || \
         (tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp (uvp)->tv_usec)

#define timerclear(tvp) (tvp)->tv_sec = (tvp)->tv_usec = 0

/* Commands for ioctlsocket(). */
#define IOCPARM_MASK 0x7f              /* Parameters must be < 128 bytes. */
#define IOC_VOID     0x20000000        /* No parameters. */
#define IOC_OUT      0x40000000        /* Copy out parameters. */
#define IOC_IN       0x80000000        /* Copy in parameters. */

#define IOC_INOUT    (IOC_IN|IOC_OUT)  /* 0x20000000 distinguishes new &
                                          old ioctl's. */
#define _IO(x,y)     (IOC_VOID|(x<<8)|y)

#define _IOR(x,y,t)  (IOC_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|(x<<8)|y)

#define _IOW(x,y,t)  (IOC_IN|(((long)sizeof(t)&IOCPARM_MASK)<<16)|(x<<8)|y)

#define FIONREAD _IOR('f', 127, u_long) /* Get # bytes to read. */
#define FIONBIO  _IOW('f', 126, u_long) /* Set/clear non-blocking i/o. */
#define FIOASYNC _IOW('f', 125, u_long) /* Set/clear async i/o. */

/* Socket I/O Controls */
#define SIOCSHIWAT  _IOW('s',  0, u_long)  /* Set high watermark. */
#define SIOCGHIWAT  _IOR('s',  1, u_long)  /* Get high watermark. */
#define SIOCSLOWAT  _IOW('s',  2, u_long)  /* Set low watermark. */
#define SIOCGLOWAT  _IOR('s',  3, u_long)  /* Get low watermark. */
#define SIOCATMARK  _IOR('s',  7, u_long)  /* At oob mark? */

struct hostent 
{
  char  FAR * h_name;            /* Official name of host. */
  char  FAR * FAR * h_aliases;   /* Alias list. */
  short h_addrtype;              /* Host address type. */
  short h_length;                /* Length of address. */
  char  FAR * FAR * h_addr_list; /* List of addresses. */
#define h_addr h_addr_list[0]    /* Address, for backward compat. */
};

/* Assumed that a network number fits in 32 bits. */
struct netent 
{
  char   FAR * n_name;           /* Official name of net. */
  char   FAR * FAR * n_aliases;  /* Alias list. */
  short  n_addrtype;             /* Net address type. */
  u_long n_net;                  /* Network #. */
};

struct servent 
{
  char  FAR * s_name;           /* Official service name. */
  char  FAR * FAR * s_aliases;  /* Alias list. */
  short s_port;                 /* Port #. */
  char  FAR * s_proto;          /* Protocol to use. */
};

struct Protoent 
{
  char  FAR * p_name;           /* Official protocol name. */
  char  FAR * FAR * p_aliases;  /* Alias list. */
  short p_proto;                /* Protocol #. */
};

#define IPPROTO_IP   0    /* Dummy for IP. */
#define IPPROTO_ICMP 1    /* Control message protocol. */
#define IPPROTO_GGP  2    /* Gateway^2 (deprecated). */
#define IPPROTO_TCP  6    /* tcp. */
#define IPPROTO_PUP  12   /* pup. */
#define IPPROTO_UDP  17   /* User datagram protocol. */
#define IPPROTO_IDP  22   /* xns idp. */
#define IPPROTO_ND   77   /* UNOFFICIAL net disk proto. */

#define IPPROTO_RAW  255  /* Raw IP packet. */
#define IPPROTO_MAX  256

#define IPPORT_ECHO        7
#define IPPORT_DISCARD     9
#define IPPORT_SYSTAT      11
#define IPPORT_DAYTIME     13
#define IPPORT_NETSTAT     15
#define IPPORT_FTP         21
#define IPPORT_TELNET      23
#define IPPORT_SMTP        25
#define IPPORT_TIMESERVER  37
#define IPPORT_NAMESERVER  42
#define IPPORT_WHOIS       43
#define IPPORT_MTP         57

/* Port/socket numbers: host specific functions. */
#define IPPORT_TFTP             69
#define IPPORT_RJE              77
#define IPPORT_FINGER           79
#define IPPORT_TTYLINK          87
#define IPPORT_SUPDUP           95

/* UNIX TCP sockets. */
#define IPPORT_EXECSERVER       512
#define IPPORT_LOGINSERVER      513
#define IPPORT_CMDSERVER        514
#define IPPORT_EFSSERVER        520

/* UNIX UDP sockets. */
#define IPPORT_BIFFUDP          512
#define IPPORT_WHOSERVER        513
#define IPPORT_ROUTESERVER      520
/* 520+1 also used */

/*
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).
 */
#define IPPORT_RESERVED         1024

/* Link numbers. */
#define IMPLINK_IP              155
#define IMPLINK_LOWEXPER        156
#define IMPLINK_HIGHEXPER       158

/* Internet address (old style... should be updated). */
struct in_addr 
{
  union 
  {
    struct { u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
    struct { u_short s_w1,s_w2; } S_un_w;
    u_long S_addr;
  } S_un;
#define s_addr  S_un.S_addr
                                /* can be used for most tcp & ip code */
#define s_host  S_un.S_un_b.s_b2
                                /* host on imp */
#define s_net   S_un.S_un_b.s_b1
                                /* network */
#define s_imp   S_un.S_un_w.s_w2
                                /* imp */
#define s_impno S_un.S_un_b.s_b4
                                /* imp # */
#define s_lh    S_un.S_un_b.s_b3
                                /* logical host */
};

/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 */
#define IN_CLASSA(i)            (((long)(i) & 0x80000000) == 0)
#define IN_CLASSA_NET           0xff000000
#define IN_CLASSA_NSHIFT        24
#define IN_CLASSA_HOST          0x00ffffff
#define IN_CLASSA_MAX           128

#define IN_CLASSB(i)            (((long)(i) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET           0xffff0000
#define IN_CLASSB_NSHIFT        16
#define IN_CLASSB_HOST          0x0000ffff
#define IN_CLASSB_MAX           65536

#define IN_CLASSC(i)            (((long)(i) & 0xe0000000) == 0xc0000000)
#define IN_CLASSC_NET           0xffffff00
#define IN_CLASSC_NSHIFT        8
#define IN_CLASSC_HOST          0x000000ff

#define INADDR_ANY              (u_long)0x00000000
#define INADDR_LOOPBACK         0x7f000001
#define INADDR_BROADCAST        (u_long)0xffffffff
#define INADDR_NONE             0xffffffff

/* Socket address, internet style. */
struct sockaddr_in 
{
  short   sin_family;
  u_short sin_port;
  struct  in_addr sin_addr;
  char    sin_zero[8];
};

#define WSADESCRIPTION_LEN      256
#define WSASYS_STATUS_LEN       128

typedef struct WSAData 
{
  WORD              wVersion;
  WORD              wHighVersion;
  char              szDescription[WSADESCRIPTION_LEN+1];
  char              szSystemStatus[WSASYS_STATUS_LEN+1];
  unsigned short    iMaxSockets;
  unsigned short    iMaxUdpDg;
  char FAR *        lpVendorInfo;
} 
WSADATA;
typedef WSADATA FAR *LPWSADATA;

/* Options for use with [gs]etsockopt at the IP level. */
#define IP_OPTIONS      1   /* set/get IP per-packet options */

/*
 * Definitions related to sockets: types, address families, options,
 * taken from the BSD file sys/socket.h.
 */

/*
 * This is used instead of -1, since the
 * SOCKET type is unsigned.
 */
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

/*
 * Types
 */
#define SOCK_STREAM     1               /* stream socket */
#define SOCK_DGRAM      2               /* datagram socket */
#define SOCK_RAW        3               /* raw-protocol interface */
#define SOCK_RDM        4               /* reliably-delivered message */
#define SOCK_SEQPACKET  5               /* sequenced packet stream */

/*
 * Option flags per-socket.
 */
#define SO_DEBUG        0x0001          /* turn on debugging info recording */
#define SO_ACCEPTCONN   0x0002          /* socket has had listen() */
#define SO_REUSEADDR    0x0004          /* allow local address reuse */
#define SO_KEEPALIVE    0x0008          /* keep connections alive */
#define SO_DONTROUTE    0x0010          /* just use interface addresses */
#define SO_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define SO_USELOOPBACK  0x0040          /* bypass hardware when possible */
#define SO_LINGER       0x0080          /* linger on close if data present */
#define SO_OOBINLINE    0x0100          /* leave received OOB data in line */

#define SO_DONTLINGER   (u_int)(~SO_LINGER)

/*
 * Additional options.
 */
#define SO_SNDBUF       0x1001          /* send buffer size */
#define SO_RCVBUF       0x1002          /* receive buffer size */
#define SO_SNDLOWAT     0x1003          /* send low-water mark */
#define SO_RCVLOWAT     0x1004          /* receive low-water mark */
#define SO_SNDTIMEO     0x1005          /* send timeout */
#define SO_RCVTIMEO     0x1006          /* receive timeout */
#define SO_ERROR        0x1007          /* get error status and clear */
#define SO_TYPE         0x1008          /* get socket type */

/*
 * TCP options.
 */
#define TCP_NODELAY     0x0001

/*
 * IP options, to use when specifying IPPROTO_IP as the level in (g/s)etsockopt
 */
#define IP_TTL		4		/* IP time to live */

/*
 * Address families.
 */
#define AF_UNSPEC       0               /* unspecified */
#define AF_UNIX         1               /* local to host (pipes, portals) */
#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
#define AF_IMPLINK      3               /* arpanet imp addresses */
#define AF_PUP          4               /* pup protocols: e.g. BSP */
#define AF_CHAOS        5               /* mit CHAOS protocols */
#define AF_NS           6               /* XEROX NS protocols */
#define AF_IPX          6               /* IPX and SPX */
#define AF_ISO          7               /* ISO protocols */
#define AF_OSI          AF_ISO          /* OSI is ISO */
#define AF_ECMA         8               /* european computer manufacturers */
#define AF_DATAKIT      9               /* datakit protocols */
#define AF_CCITT        10              /* CCITT protocols, X.25 etc */
#define AF_SNA          11              /* IBM SNA */
#define AF_DECnet       12              /* DECnet */
#define AF_DLI          13              /* Direct data link interface */
#define AF_LAT          14              /* LAT */
#define AF_HYLINK       15              /* NSC Hyperchannel */
#define AF_APPLETALK    16              /* AppleTalk */
#define AF_NETBIOS      17              /* NetBios-style addresses */

#define AF_MAX          18

/*
 * Structure used by kernel to store most
 * addresses.
 */
struct sockaddr {
        u_short sa_family;              /* address family */
        char    sa_data[14];            /* up to 14 bytes of direct address */
};

/*
 * Structure used by kernel to pass protocol
 * information in raw sockets.
 */
struct sockproto {
        u_short sp_family;              /* address family */
        u_short sp_protocol;            /* protocol */
};

/*
 * Protocol families, same as address families for now.
 */
#define PF_UNSPEC       AF_UNSPEC
#define PF_UNIX         AF_UNIX
#define PF_INET         AF_INET
#define PF_IMPLINK      AF_IMPLINK
#define PF_PUP          AF_PUP
#define PF_CHAOS        AF_CHAOS
#define PF_NS           AF_NS
#define PF_IPX          AF_IPX
#define PF_ISO          AF_ISO
#define PF_OSI          AF_OSI
#define PF_ECMA         AF_ECMA
#define PF_DATAKIT      AF_DATAKIT
#define PF_CCITT        AF_CCITT
#define PF_SNA          AF_SNA
#define PF_DECnet       AF_DECnet
#define PF_DLI          AF_DLI
#define PF_LAT          AF_LAT
#define PF_HYLINK       AF_HYLINK
#define PF_APPLETALK    AF_APPLETALK

#define PF_MAX          AF_MAX

/*
 * Structure used for manipulating linger option.
 */
struct  linger {
        u_short l_onoff;                /* option on/off */
        u_short l_linger;               /* linger time */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define SOL_SOCKET      0xffff          /* options for socket level */

/*
 * Maximum queue length specifiable by listen.
 */
#define SOMAXCONN       5

#define MSG_OOB         0x1             /* process out-of-band data */
#define MSG_PEEK        0x2             /* peek at incoming message */
#define MSG_DONTROUTE   0x4             /* send without using routing tables */

#define MSG_MAXIOVLEN   16

/*
 * Define constant based on rfc883, used by gethostbyxxxx() calls.
 */
#define MAXGETHOSTSTRUCT        1024

/*
 * Define flags to be used with the WSAAsyncSelect() call.
 */
#define FD_READ         0x01
#define FD_WRITE        0x02
#define FD_OOB          0x04
#define FD_ACCEPT       0x08
#define FD_CONNECT      0x10
#define FD_CLOSE        0x20

/*
 * All Windows Sockets error constants are biased by WSABASEERR from
 * the "normal"
 */
#define WSABASEERR              10000
/*
 * Windows Sockets definitions of regular Microsoft C error constants
 */
#define WSAEINTR                (WSABASEERR+4)
#define WSAEBADF                (WSABASEERR+9)
#define WSAEACCES               (WSABASEERR+13)
#define WSAEFAULT               (WSABASEERR+14)
#define WSAEINVAL               (WSABASEERR+22)
#define WSAEMFILE               (WSABASEERR+24)

/*
 * Windows Sockets definitions of regular Berkeley error constants
 */
#define WSAEWOULDBLOCK          (WSABASEERR+35)
#define WSAEINPROGRESS          (WSABASEERR+36)
#define WSAEALREADY             (WSABASEERR+37)
#define WSAENOTSOCK             (WSABASEERR+38)
#define WSAEDESTADDRREQ         (WSABASEERR+39)
#define WSAEMSGSIZE             (WSABASEERR+40)
#define WSAEPROTOTYPE           (WSABASEERR+41)
#define WSAENOPROTOOPT          (WSABASEERR+42)
#define WSAEPROTONOSUPPORT      (WSABASEERR+43)
#define WSAESOCKTNOSUPPORT      (WSABASEERR+44)
#define WSAEOPNOTSUPP           (WSABASEERR+45)
#define WSAEPFNOSUPPORT         (WSABASEERR+46)
#define WSAEAFNOSUPPORT         (WSABASEERR+47)
#define WSAEADDRINUSE           (WSABASEERR+48)
#define WSAEADDRNOTAVAIL        (WSABASEERR+49)
#define WSAENETDOWN             (WSABASEERR+50)
#define WSAENETUNREACH          (WSABASEERR+51)
#define WSAENETRESET            (WSABASEERR+52)
#define WSAECONNABORTED         (WSABASEERR+53)
#define WSAECONNRESET           (WSABASEERR+54)
#define WSAENOBUFS              (WSABASEERR+55)
#define WSAEISCONN              (WSABASEERR+56)
#define WSAENOTCONN             (WSABASEERR+57)
#define WSAESHUTDOWN            (WSABASEERR+58)
#define WSAETOOMANYREFS         (WSABASEERR+59)
#define WSAETIMEDOUT            (WSABASEERR+60)
#define WSAECONNREFUSED         (WSABASEERR+61)
#define WSAELOOP                (WSABASEERR+62)
#define WSAENAMETOOLONG         (WSABASEERR+63)
#define WSAEHOSTDOWN            (WSABASEERR+64)
#define WSAEHOSTUNREACH         (WSABASEERR+65)
#define WSAENOTEMPTY            (WSABASEERR+66)
#define WSAEPROCLIM             (WSABASEERR+67)
#define WSAEUSERS               (WSABASEERR+68)
#define WSAEDQUOT               (WSABASEERR+69)
#define WSAESTALE               (WSABASEERR+70)
#define WSAEREMOTE              (WSABASEERR+71)

/*
 * Extended Windows Sockets error constant definitions
 */
#define WSASYSNOTREADY          (WSABASEERR+91)
#define WSAVERNOTSUPPORTED      (WSABASEERR+92)
#define WSANOTINITIALISED       (WSABASEERR+93)

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (when using the resolver). Note that these errors are
 * retrieved via WSAGetLastError() and must therefore follow
 * the rules for avoiding clashes with error numbers from
 * specific implementations or language run-time systems.
 * For this reason the codes are based at WSABASEERR+1001.
 * Note also that [WSA]NO_ADDRESS is defined only for
 * compatibility purposes.
 */

#define h_errno         WSAGetLastError()

/* Authoritative Answer: Host not found */
#define WSAHOST_NOT_FOUND       (WSABASEERR+1001)
#define HOST_NOT_FOUND          WSAHOST_NOT_FOUND

/* Non-Authoritative: Host not found, or SERVERFAIL */
#define WSATRY_AGAIN            (WSABASEERR+1002)
#define TRY_AGAIN               WSATRY_AGAIN

/* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define WSANO_RECOVERY          (WSABASEERR+1003)
#define NO_RECOVERY             WSANO_RECOVERY

/* Valid name, no data record of requested type */
#define WSANO_DATA              (WSABASEERR+1004)
#define NO_DATA                 WSANO_DATA

/* no address, look for MX record */
#define WSANO_ADDRESS           WSANO_DATA
#define NO_ADDRESS              WSANO_ADDRESS

/*
 * Windows Sockets errors redefined as regular Berkeley error constants
 * Commented out to avoid conflicts with errno.h.  Use the WSA constants.
 */
#if 0
#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS
#define EALREADY                WSAEALREADY
#define ENOTSOCK                WSAENOTSOCK
#define EDESTADDRREQ            WSAEDESTADDRREQ
#define EMSGSIZE                WSAEMSGSIZE
#define EPROTOTYPE              WSAEPROTOTYPE
#define ENOPROTOOPT             WSAENOPROTOOPT
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#define EOPNOTSUPP              WSAEOPNOTSUPP
#define EPFNOSUPPORT            WSAEPFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#define EADDRINUSE              WSAEADDRINUSE
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define ENETDOWN                WSAENETDOWN
#define ENETUNREACH             WSAENETUNREACH
#define ENETRESET               WSAENETRESET
#define ECONNABORTED            WSAECONNABORTED
#define ECONNRESET              WSAECONNRESET
#define ENOBUFS                 WSAENOBUFS
#define EISCONN                 WSAEISCONN
#define ENOTCONN                WSAENOTCONN
#define ESHUTDOWN               WSAESHUTDOWN
#define ETOOMANYREFS            WSAETOOMANYREFS
#define ETIMEDOUT               WSAETIMEDOUT
#define ECONNREFUSED            WSAECONNREFUSED
#define ELOOP                   WSAELOOP
#define ENAMETOOLONG            WSAENAMETOOLONG
#define EHOSTDOWN               WSAEHOSTDOWN
#define EHOSTUNREACH            WSAEHOSTUNREACH
#define ENOTEMPTY               WSAENOTEMPTY
#define EPROCLIM                WSAEPROCLIM
#define EUSERS                  WSAEUSERS
#define EDQUOT                  WSAEDQUOT
#define ESTALE                  WSAESTALE
#define EREMOTE                 WSAEREMOTE
#endif


/* 
** Convert name of Winsock functions to our own
** functions in the ETSSock.c file.
*/
/* Macros for the WinSock Components */
#undef accept
#undef bind
#undef closesocket
#undef connect
#undef ioctlsocket
#undef getpeername
#undef getsockname
#undef getsockopt
#undef htonl
#undef htons
#undef inet_addr
#undef inet_ntoa
#undef listen
#undef ntohl
#undef ntohs
#undef recv
#undef recvfrom
#undef select
#undef send
#undef sendto
#undef setsockopt
#undef shutdown
#undef socket
#undef gethostbyaddr 
#undef gethostbyname
#undef gethostname
#undef getservbyport
#undef getservbyname
#undef getprotobyname
#undef getprotobynumber
#undef WSAStartup
#undef WSACleanup
#undef WSASetLastError
#undef WSAGetLastError
#undef WSAIsBlocking
#undef WSAUnhookBlockingHook
#undef WSASetBlockingHook
#undef WSACancelBlockingCall
#undef WSAAsyncGetServByName
#undef WSAAsyncGetServByPort
#undef WSAAsyncGetProtoByName
#undef WSAAsyncGetProtoByNumber
#undef WSAAsyncGetHostByName
#undef WSAAsyncGetHostByAddr
#undef WSACancelAsyncRequest
#undef WSAAsyncSelect

#define accept              Accept
#define bind                Bind
#define closesocket         Closesocket
#define connect             Connect
#define ioctlsocket         Ioctlsocket
#define getpeername         Getpeername
#define getsockname         Getsockname
#define getsockopt          Getsockopt
#define htonl               Htonl
#define htons               Htons    
#define inet_addr           Inet_addr
#define inet_ntoa           Inet_ntoa
#define listen              Listen
#define ntohl               Ntohl
#define ntohs               Ntohs
#define recv                Recv
#define recvfrom            Recvfrom
#define select              Select
#define send                Send
#define sendto              Sendto
#define setsockopt          Setsockopt
#define shutdown            Shutdown
#define socket              Socket
#define gethostbyaddr       Gethostbyaddr
#define gethostbyname       Gethostbyname
#define gethostname         Gethostname
#define getservbyport       Getservbyport
#define getservbyname       Getservbyname
#define getprotobynumber    Getprotobynumber
#define getprotobyname      Getprotobyname
#define WSAStartup          TWIN_WSAStartup
#define WSACleanup          TWIN_WSACleanup
#define WSASetLastError     TWIN_WSASetLastError
#define WSAGetLastError     TWIN_WSAGetLastError
#define WSAIsBlocking       TWIN_WSAIsBlocking
#define WSAUnhookBlockingHook     TWIN_WSAUnhookBlockingHook
#define WSASetBlockingHook        TWIN_WSASetBlockingHook
#define WSACancelBlockingCall     TWIN_WSACancelBlockingCall
#define WSAAsyncGetServByName     TWIN_WSAAsyncGetServByName
#define WSAAsyncGetServByPort     TWIN_WSAAsyncGetServByPort
#define WSAAsyncGetProtoByName    TWIN_WSAAsyncGetProtoByName
#define WSAAsyncGetProtoByNumber  TWIN_WSAAsyncGetProtoByNumber
#define WSAAsyncGetHostByName     TWIN_WSAAsyncGetHostByName
#define WSAAsyncGetHostByAddr     TWIN_WSAAsyncGetHostByAddr
#define WSACancelAsyncRequest     TWIN_WSACancelAsyncRequest
#define WSAAsyncSelect            TWIN_WSAAsyncSelect


/* Socket function prototypes */
#ifdef __cplusplus
extern "C" {
#endif

 
SOCKET Accept
(
  SOCKET s, 
  struct sockaddr FAR *addr,
  int FAR *addrlen
);

int Bind
(
  SOCKET s, 
  const struct sockaddr FAR *addr, 
  int namelen
);

int Closesocket( SOCKET s );

int Connect
(
  SOCKET s, 
  const struct sockaddr FAR *name, 
  int namelen
);

int Ioctlsocket
(
  SOCKET s, 
  long cmd, 
  u_long FAR *argp
);

int Getpeername
(
  SOCKET s, 
  struct sockaddr FAR *name,
  int FAR * namelen
);

int Getsockname
(
  SOCKET s, 
  struct sockaddr FAR *name,
  int FAR * namelen
);

int Getsockopt
(
  SOCKET s, 
  int level, 
  int optname,
  char FAR * optval, 
  int FAR *optlen
);

u_long Htonl( u_long hostlong );

u_short Htons( u_short hostshort );

unsigned long Inet_addr( const char FAR * cp );

char FAR * Inet_ntoa( struct in_addr in );

int Listen( SOCKET s, int backlog );

u_long Ntohl( u_long netlong );

u_short Ntohs( u_short netshort );

int Recv
(
  SOCKET s, 
  char FAR * buf, 
  int len, 
  int flags
);

int Recvfrom
(
  SOCKET s, 
  char FAR * buf, 
  int len, 
  int flags,
  struct sockaddr FAR *from, 
  int FAR * fromlen
);

int Select
(
  int nfds, 
  fd_set FAR *readfds, 
  fd_set FAR *writefds,
  fd_set FAR *exceptfds, 
  const struct timeval FAR *timeout
);

int Send
(
  SOCKET s, 
  const char FAR * buf, 
  int len, 
  int flags
);

int Sendto
(
  SOCKET s, 
  const char FAR * buf, 
  int len, 
  int flags,
  const struct sockaddr FAR *to, 
  int tolen
);

int Setsockopt
(
  SOCKET s, 
  int level, 
  int optname,
  const char FAR * optval, 
  int optlen
);

int Shutdown( SOCKET s, int how );

SOCKET Socket
( 
  int af, 
  int type, 
  int protocol
);


/* Database function prototypes */
struct hostent FAR * Gethostbyaddr(const char FAR * addr,
                                              int len, int type);

struct hostent FAR * Gethostbyname(const char FAR * name);

int Gethostname (char FAR * name, int namelen);

struct servent FAR * Getservbyport(int port, const char FAR * proto);

struct servent FAR * Getservbyname(const char FAR * name,
                                              const char FAR * proto);

struct protoent FAR * Getprotobynumber(int proto);

struct protoent FAR * Getprotobyname(const char FAR * name);

/* Microsoft Windows Extension function prototypes */

int TWIN_WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData);

int TWIN_WSACleanup(void);

void TWIN_WSASetLastError(int iError);

int TWIN_WSAGetLastError(void);

BOOL TWIN_WSAIsBlocking(void);

int TWIN_WSAUnhookBlockingHook(void);

FARPROC TWIN_WSASetBlockingHook(FARPROC lpBlockFunc);

int TWIN_WSACancelBlockingCall(void);

HANDLE TWIN_WSAAsyncGetServByName(HWND hWnd, u_int wMsg,
                                        const char FAR * name,
                                        const char FAR * proto,
                                        char FAR * buf, int buflen);

HANDLE TWIN_WSAAsyncGetServByPort(HWND hWnd, u_int wMsg, int port,
                                        const char FAR * proto, char FAR * buf,
                                        int buflen);

HANDLE TWIN_WSAAsyncGetProtoByName(HWND hWnd, u_int wMsg,
                                         const char FAR * name, char FAR * buf,
                                         int buflen);

HANDLE TWIN_WSAAsyncGetProtoByNumber(HWND hWnd, u_int wMsg,
                                           int number, char FAR * buf,
                                           int buflen);

HANDLE TWIN_WSAAsyncGetHostByName(HWND hWnd, u_int wMsg,
                                        const char FAR * name, char FAR * buf,
                                        int buflen);

HANDLE TWIN_WSAAsyncGetHostByAddr(HWND hWnd, u_int wMsg,
                                        const char FAR * addr, int len, int type,
                                        char FAR * buf, int buflen);

int PASCAL FAR TWIN_WSACancelAsyncRequest(HANDLE hAsyncTaskHandle);

int TWIN_WSAAsyncSelect(SOCKET s, HWND hWnd, u_int wMsg,
                               long lEvent);

#ifdef __cplusplus
}
#endif

/* Microsoft Windows Extended data types */
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr *PSOCKADDR;
typedef struct sockaddr FAR *LPSOCKADDR;

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in *PSOCKADDR_IN;
typedef struct sockaddr_in FAR *LPSOCKADDR_IN;

typedef struct linger LINGER;
typedef struct linger *PLINGER;
typedef struct linger FAR *LPLINGER;

typedef struct in_addr IN_ADDR;
typedef struct in_addr *PIN_ADDR;
typedef struct in_addr FAR *LPIN_ADDR;

typedef struct fd_set FD_SET;
typedef struct fd_set *PFD_SET;
typedef struct fd_set FAR *LPFD_SET;

typedef struct hostent HOSTENT;
typedef struct hostent *PHOSTENT;
typedef struct hostent FAR *LPHOSTENT;

typedef struct servent SERVENT;
typedef struct servent *PSERVENT;
typedef struct servent FAR *LPSERVENT;

typedef struct protoent PROTOENT;
typedef struct protoent *PPROTOENT;
typedef struct protoent FAR *LPPROTOENT;

typedef struct timeval TIMEVAL;
typedef struct timeval *PTIMEVAL;
typedef struct timeval FAR *LPTIMEVAL;

/*
 * Windows message parameter composition and decomposition
 * macros.
 *
 * WSAMAKEASYNCREPLY is intended for use by the Windows Sockets implementation
 * when constructing the response to a WSAAsyncGetXByY() routine.
 */
#define WSAMAKEASYNCREPLY(buflen,error)     MAKELONG(buflen,error)
/*
 * WSAMAKESELECTREPLY is intended for use by the Windows Sockets implementation
 * when constructing the response to WSAAsyncSelect().
 */
#define WSAMAKESELECTREPLY(event,error)     MAKELONG(event,error)
/*
 * WSAGETASYNCBUFLEN is intended for use by the Windows Sockets application
 * to extract the buffer length from the lParam in the response
 * to a WSAGetXByY().
 */
#define WSAGETASYNCBUFLEN(lParam)           LOWORD(lParam)
/*
 * WSAGETASYNCERROR is intended for use by the Windows Sockets application
 * to extract the error code from the lParam in the response
 * to a WSAGetXByY().
 */
#define WSAGETASYNCERROR(lParam)            HIWORD(lParam)
/*
 * WSAGETSELECTEVENT is intended for use by the Windows Sockets application
 * to extract the event code from the lParam in the response
 * to a WSAAsyncSelect().
 */
#define WSAGETSELECTEVENT(lParam)           LOWORD(lParam)
/*
 * WSAGETSELECTERROR is intended for use by the Windows Sockets application
 * to extract the error code from the lParam in the response
 * to a WSAAsyncSelect().
 */
#define WSAGETSELECTERROR(lParam)           HIWORD(lParam)

#endif /* MSWinSock_h */

