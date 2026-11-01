/* F_tcp.hpp */
/* TCP backend for FreePM: connection object.

   Cross-platform: same source builds for OS/2 and Win32.
   Platform-specific bits are hidden behind PMC_* macros below.
   OS/2-compatible types (APIRET, ULONG, LONG, PVOID, ...) are
   defined here on Win32 so the rest of the backend compiles
   unchanged. */

#ifndef FREEPM_TCP_HPP
#define FREEPM_TCP_HPP

#include <string.h>

#if defined(__OS2__) || defined(__OS2V2__)

  /* --- OS/2 target --- */
  #define INCL_DOS
  #define INCL_DOSERRORS
  #include <os2.h>
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
  #define PMC_SLEEP(ms)       DosSleep(ms)
  #define PMC_ERR_OPEN        ERROR_OPEN_FAILED
  #define PMC_ERR_BROKEN      ERROR_BROKEN_PIPE

#elif defined(__WIN32__) || defined(__NT__) || defined(_WIN32)

  /* --- Win32 target --- */
  #include <windows.h>
  #include <winsock.h>

  /* OS/2-compatible types, needed because the whole backend API
     was designed for OS/2. */
  #ifndef APIRET
    typedef unsigned long APIRET;
  #endif
  #ifndef ULONG
    typedef unsigned long ULONG;
  #endif
  #ifndef PULONG
    typedef ULONG *PULONG;
  #endif
  #ifndef LONG
    typedef long LONG;
  #endif
  #ifndef PLONG
    typedef LONG *PLONG;
  #endif
  #ifndef PVOID
    typedef void *PVOID;
  #endif
  #ifndef PCSZ
    typedef const char *PCSZ;
  #endif
  #ifndef PSZ
    typedef char *PSZ;
  #endif

  #ifdef APIENTRY
    #undef APIENTRY
  #endif
  #ifndef APIENTRY
    #define APIENTRY __stdcall
    
    /* Custom calling convention to prevent name decoration (_Name@N) 
       while keeping the __stdcall stack cleanup. */
    #pragma aux __estdcall "*" \
               parm routine [] \
               value struct struct caller [] \
               modify [eax ecx edx]
    #undef EXPENTRY
    #define EXPENTRY __declspec(__pragma("__estdcall"))
  #endif

  extern "C" int  PMC_WsaStartup(void);
  extern "C" void PMC_WsaCleanup(void);

  #define PMC_CLOSESOCKET(s)  closesocket(s)
  typedef SOCKET PMC_SOCK;
  #define PMC_INVALID_SOCK    INVALID_SOCKET
  #define PMC_SOCK_ERROR      SOCKET_ERROR
  #define PMC_WSA_STARTUP()   PMC_WsaStartup()
  #define PMC_WSA_CLEANUP()   PMC_WsaCleanup()
  #define PMC_SLEEP(ms)       Sleep(ms)
  #define PMC_ERR_OPEN        WSAECONNREFUSED
  #define PMC_ERR_BROKEN      WSAECONNABORTED

#else
  #error Unsupported platform for PMTCP
#endif

#define FREEPM_TCP_DEFAULT_HOST  "127.0.0.1"
#define FREEPM_TCP_DEFAULT_PORT  9876
#define FREEPM_TCP_BACKLOG       5

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
    PMC_SOCK sock;        /*!< Socket handle, or PMC_INVALID_SOCK. */
    int      mode;        /*!< TCP_CLIENT_MODE or TCP_SERVER_MODE. */
    int      connected;   /*!< Non-zero once connected. */
    char     host[256];   /*!< Peer host name or address. */
    int      port;        /*!< Peer port. */

    TCPipe();
    TCPipe(const char *pszHost, int iPort, int iMode);
    ~TCPipe();

    int  Create(void);            /*!< Server: socket+bind+listen. */
    int  Accept(TCPipe *pClient); /*!< Server: accept a connection. */
    int  Connect(void);           /*!< Client: socket+connect. */
    int  Send(const void *pvData, int cbData);
    int  Recv(void *pvData, int *pcbReceived, int cbBuffer);
    int  Close(void);
    int  HandShake(void);         /*!< Same protocol as NPipe. */
};

#endif /* FREEPM_TCP_HPP */
