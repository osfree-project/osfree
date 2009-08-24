/*  WSA.h	1.7
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

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/

/********************************************************************
*                                                                   *
*  WSA.h Contains the API definitions for the winsock interface     *
*  as specified in the winsock 1.1 design specifications. Also note *
*  that the protype definitions are shown for the regular BSD Socket*
*  Interface also which is required for this implementation of      *
*  WINSOCK under TWIN                                               *
*                                                                   *
*********************************************************************/

#ifndef __WSA__h
#define __WSA__h

#include "WinSocktypes.h"

#define WSADESCRIPTION_LEN      256
#define WSASYS_STATUS_LEN       128
#define SOCKET_ERROR            (-1)
#define INVALID_SOCKET          (SOCKET)(~0)

#define FD_READ         0x01
#define FD_WRITE        0x02
#define FD_OOB          0x04
#define FD_ACCEPT       0x08
#define FD_CONNECT      0x10
#define FD_CLOSE        0x20

typedef struct WSAData {
        WORD                    wVersion;
        WORD                    wHighVersion;
        char                    szDescription[WSADESCRIPTION_LEN+1];
        char                    szSystemStatus[WSASYS_STATUS_LEN+1];
        unsigned short          iMaxSockets;
        unsigned short          iMaxUdpDg;
        char FAR *              lpVendorInfo;
} WSADATA;

typedef WSADATA FAR *LPWSADATA;

#define MAXGETHOSTSTRUCT   1024

#ifdef __cplusplus
extern "C" {
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

/* Prototype definitions for Socket functions */

#ifndef NOAPIPROTO
SOCKET PASCAL FAR Accept (SOCKET , struct sockaddr FAR *, int FAR *);

int PASCAL FAR Bind (SOCKET , const struct sockaddr FAR *, int );

int PASCAL FAR Closesocket (SOCKET );

int PASCAL FAR Connect (SOCKET, const struct sockaddr FAR *, int );

int PASCAL FAR Ioctlsocket (SOCKET , long , DWORD  FAR *);

int PASCAL FAR Getpeername (SOCKET , struct sockaddr FAR *,
                            int FAR * );

int PASCAL FAR Getsockname (SOCKET , struct sockaddr FAR *,
                            int FAR * );

int PASCAL FAR Getsockopt (SOCKET , int , int ,
                           char FAR * , int FAR *);

DWORD  PASCAL FAR Htonl (DWORD  );

WORD    PASCAL FAR Htons (WORD  );

unsigned long PASCAL FAR Inet_addr (const char FAR * );

/* RD for now. char FAR * PASCAL FAR Inet_ntoa (struct in_addr );
*/
int PASCAL FAR Listen (SOCKET , int );

DWORD  PASCAL FAR Ntohl (DWORD  );

WORD    PASCAL FAR Ntohs (WORD  );

int PASCAL FAR Recv (SOCKET , char FAR * , int , int );

int PASCAL FAR Recvfrom (SOCKET , char FAR * , int , int ,
                         struct sockaddr FAR *, int FAR * );
/*
int PASCAL FAR TWINselect (int , fd_set FAR *,fd_set FAR  *, 
                     fd_set FAR *, const struct timeval *);
*/
int PASCAL FAR Send (SOCKET , const char FAR *, int , int );

int PASCAL FAR Sendto (SOCKET , const char FAR * , int , int ,
                       const struct sockaddr FAR *, int );

int PASCAL FAR Setsockopt (SOCKET , int , int ,
                           const char FAR * , int );

int PASCAL FAR Shutdown (SOCKET , int );

SOCKET PASCAL FAR Socket (int , int , int );

/* Database function prototypes */

struct hostent  FAR * PASCAL FAR Gethostbyaddr(const char FAR * ,
                                              int , int );

struct hostent  FAR * PASCAL FAR Gethostbyname(const char FAR * );

int PASCAL FAR Gethostname (char FAR * , int );

struct servent FAR * PASCAL FAR Getservbyport(int , const char FAR * );

struct servent FAR * PASCAL FAR Getservbyname(const char FAR * ,
                                              const char FAR * );

struct protoent FAR * PASCAL FAR Getprotobynumber(int );

struct protoent FAR * PASCAL FAR Getprotobyname(const char FAR * );

/* Microsoft Windows Extension function prototypes */
int PASCAL FAR WSAStartup(WORD , LPWSADATA );

int PASCAL FAR WSACleanup(void);

void PASCAL FAR WSASetLastError(int );

int PASCAL FAR WSAGetLastError(void);

BOOL PASCAL FAR WSAIsBlocking(void);

int PASCAL FAR WSAUnhookBlockingHook(void);

FARPROC PASCAL FAR WSASetBlockingHook(FARPROC );

int PASCAL FAR WSACancelBlockingCall(void);

HANDLE PASCAL FAR WSAAsyncGetServByName(HWND , UINT  ,
                                        const char FAR *, 
                                        const char FAR *,
                                        char FAR *, int );

HANDLE PASCAL FAR WSAAsyncGetServByPort(HWND , UINT  , int ,
                                        const char FAR *, char FAR *,
                                        int );

HANDLE PASCAL FAR WSAAsyncGetProtoByName(HWND , UINT  ,
                                         const char FAR *, char FAR *,
                                         int );

HANDLE PASCAL FAR WSAAsyncGetProtoByNumber(HWND , UINT  ,
                                           int , char FAR *,
                                           int );

HANDLE PASCAL FAR WSAAsyncGetHostByName(HWND , UINT ,
                                        const char FAR *, char FAR *,
                                        int );

HANDLE PASCAL FAR WSAAsyncGetHostByAddr(HWND , UINT ,
                                        const char FAR *, int , int ,
                                        char FAR *, int );

int PASCAL FAR WSACancelAsyncRequest(HANDLE );

int PASCAL FAR WSAAsyncSelect(SOCKET , HWND , UINT , long );

HANDLE PASCAL FAR WSAAsyncGetHostName ( HWND, UINT, char *, int);

#endif
#ifdef __cplusplus
}
#endif


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

/*
 * All Windows Sockets error constants are biased by WSABASEERR from
 * the "normal"
 */
#define WSABASEERR              10000

/* Authoritative Answer: Host not found */
#define WSAHOST_NOT_FOUND       (WSABASEERR+1001)

/* Non-Authoritative: Host not found, or SERVERFAIL */
#define WSATRY_AGAIN            (WSABASEERR+1002)

/* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define WSANO_RECOVERY          (WSABASEERR+1003)

/* Valid name, no data record of requested type */
#define WSANO_DATA              (WSABASEERR+1004)

/* no address, look for MX record */
#define WSANO_ADDRESS           WSANO_DATA

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

#if 0
	/* DO NOT replace select() with winsock version because
	 * x11 driver calls select() expecting to get the working UNIX
	 * version.
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

#define accept(a,b,c) (Accept(a,b,c))
#define bind(a,b,c) (Bind(a,b,c))
#define closesocket(a) (Closesocket(a))
#define connect(a,b,c) (Connect(a,b,c))
#define ioctlsocket(a,b,c) (Ioctlsocket(a,b,c))
#define getpeername(a,b,c) (Getpeername(a,b,c))
#define getsockname(a,b,c) (Getsockname(a,b,c))
#define getsockopt(a,b,c,d,e)  (Getsockopt(a,b,c,d,e))
#define htonl(a) (Htonl(a))
#define htons(a) (Htons(a))
#define inet_addr(a) (Inet_addr(a))
#define inet_ntoa(a) (Inet_ntoa(a))
#define listen(a,b)  (Listen(a,b))
#define ntohl(a) (Ntohl(a))
#define ntohs(a) (Ntohs(a))
#define recv(a,b,c,d) (Recv(a,b,c,d))
#define recvfrom(a,b,c,d,e,f) (Recvfrom(a,b,c,d,e,f))
#define select(a,b,c,d,e) (Select(a,b,c,d,e))
#define send(a,b,c,d) (Send(a,b,c,d))
#define sendto(a,b,c,d,e,f) (Sendto(a,b,c,d,e,f))
#define setsockopt(a,b,c,d,e) (Setsockopt(a,b,c,d,e))
#define shutdown(a,b) (Shutdown(a,b))
#define socket(a,b,c) (Socket(a,b,c))
#define gethostbyaddr(a,b,c) (Gethostbyaddr(a,b,c))
#define gethostbyname(a) (Gethostbyname(a))
#define gethostname(a,b) (Gethostname(a,b))
#define getservbyport(a,b) (Getservbyport(a,b))
#define getservbyname(a,b) (Getservbyname(a,b))
#define getprotobynumber(a) (Getprotobynumber(a))
#define getprotobyname(a) (Getprotobyname(a))

#endif

#endif	/* __WSA__h */
