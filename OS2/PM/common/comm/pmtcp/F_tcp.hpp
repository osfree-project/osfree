/* F_tcp.hpp */
/* TCP backend for FreePM: connection object.

   Cross-platform: same source builds for OS/2 and Win32.
   All platform types (APIRET, ULONG, LONG, PVOID, APIENTRY, EXPENTRY),
   including Winsock declarations on Win32, come from F_platform.h.
   Only the socket-type aliases and the connection class remain here. */

#ifndef FREEPM_TCP_HPP
#define FREEPM_TCP_HPP

#include <string.h>

#define INCL_DOSERRORS      
#include "F_platform.h"
#include "F_debug.h"

#if defined(__OS2__) || defined(__OS2V2__)

  /* OS/2 socket headers; F_platform.h does not pull these in. */
  #include <types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>

  extern "C" int _System soclose(int);

  #define PMC_CLOSESOCKET(s)  soclose(s)
  typedef int PMC_SOCK;
  #define PMC_INVALID_SOCK    (-1)
  #define PMC_SOCK_ERROR      (-1)
  #define PMC_WSA_STARTUP()   0
  #define PMC_WSA_CLEANUP()   ((void)0)
  #define PMC_ERR_OPEN        ERROR_OPEN_FAILED
  #define PMC_ERR_BROKEN      ERROR_BROKEN_PIPE
  #define PMC_LAST_SOCK_ERR() sock_errno()

#elif defined(__WIN32__) || defined(__NT__) || defined(_WIN32)

  /* Winsock declarations are in F_platform.h. */
  extern "C" int  PMC_WsaStartup(void);
  extern "C" void PMC_WsaCleanup(void);

  #define PMC_CLOSESOCKET(s)  closesocket(s)
  typedef SOCKET PMC_SOCK;
  #define PMC_INVALID_SOCK    INVALID_SOCKET
  #define PMC_SOCK_ERROR      SOCKET_ERROR
  #define PMC_WSA_STARTUP()   PMC_WsaStartup()
  #define PMC_WSA_CLEANUP()   PMC_WsaCleanup()
  #define PMC_ERR_OPEN        WSAECONNREFUSED
  #define PMC_ERR_BROKEN      WSAECONNABORTED
  #define PMC_LAST_SOCK_ERR() WSAGetLastError()

#else
  #error Unsupported platform for PMTCP
#endif

#define FREEPM_TCP_DEFAULT_HOST  "127.0.0.1"
#define FREEPM_TCP_DEFAULT_PORT  9876
#define FREEPM_TCP_BACKLOG       5
#define FREEPM_TCP_THREAD_STACK  65536

/* Backend connection modes (same values as F_pipe.hpp). */
#define TCP_SERVER_MODE   1
#define TCP_CLIENT_MODE   2

/* Handshake constants (same protocol as NPipe). */
#define HAND_SHAKE_INP    "pIpEtEsT"
#define HAND_SHAKE_OUT    "PiPeTeSt"
#define HAND_SHAKE_ERROR  101

/*! @brief TCP connection object. One instance per client connection
    or per accepted server connection. */
class TCPipe
{
public:
    PMC_SOCK sock;
    int      mode;
    int      connected;
    char     host[256];
    int      port;

    TCPipe();
    TCPipe(const char *pszHost, int iPort, int iMode);
    ~TCPipe();

    int  Create(void);
    int  Accept(TCPipe *pClient);
    int  Connect(void);
    int  Send(const void *pvData, int cbData);
    int  Recv(void *pvData, int *pcbReceived, int cbBuffer);
    int  Close(void);
    int  HandShake(void);
};

#endif /* FREEPM_TCP_HPP */
