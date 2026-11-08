/*! @file F_tcp.hpp
    @brief TCP transport for FreePM: TcpLink class.

    Pulls in the platform abstraction (which on OS/2 includes <os2.h>)
    and, on OS/2 only, the BSD-style socket headers. All TCP code uses
    plain OS/2 + BSD socket API; Win32 sees those through F_platform.h
    emulation.
*/
#ifndef F_TCP_HPP
#define F_TCP_HPP

#define INCL_DOSERRORS
#include "F_platform.h"

#if defined(__OS2__) || defined(__OS2V2__)
  /* BSD_SELECT makes <sys/select.h> pull in the full struct timeval
     definition (otherwise tcpustd.h only forward-declares it). */
  #define BSD_SELECT
  #include <types.h>
  #include <sys/socket.h>
  #include <sys/select.h>
  #include <sys/ioctl.h>
  #include <sys/time.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <errno.h>
  #include <nerrno.h>

  /*! @def SD_SEND
      @brief shutdown() "stop sending" (== SHUT_WR; WinSock spelling). */
  #define SD_SEND 1

  extern "C" int _System soclose(int);

  /*! @def PMC_CLOSESOCKET
      @brief Close a socket (OS/2). */
  #define PMC_CLOSESOCKET(s)  soclose(s)
  /*! @brief OS/2 socket handle type. */
  typedef int PMC_SOCK;
  /*! @brief Invalid socket value (OS/2). */
  #define PMC_INVALID_SOCK    (-1)
  /*! @brief send/recv error return (OS/2). */
  #define PMC_SOCK_ERROR      (-1)
  /*! @brief Get last socket error (OS/2). */
  #define PMC_LAST_SOCK_ERR() sock_errno()
  /*! @brief select() first arg (OS/2: highest fd + 1). */
  #define PMC_NFDS(s) ((int)(s) + 1)
#else
  /*! @def PMC_CLOSESOCKET
      @brief Close a socket (Win32). */
  #define PMC_CLOSESOCKET(s)  closesocket(s)
  /*! @brief Win32 socket handle type. */
  typedef SOCKET PMC_SOCK;
  /*! @brief Invalid socket value (Win32). */
  #define PMC_INVALID_SOCK    INVALID_SOCKET
  /*! @brief send/recv error return (Win32). */
  #define PMC_SOCK_ERROR      SOCKET_ERROR
  /*! @brief Get last socket error (Win32). */
  #define PMC_LAST_SOCK_ERR() WSAGetLastError()
  /*! @brief select() first arg (WinSock: ignored). */
  #define PMC_NFDS(s) 0
#endif

#include "F_link.hpp"
#include "F_wire.hpp"

/*! @def FREEPM_TCP_DEFAULT_HOST
    @brief Default peer host (loopback). */
#define FREEPM_TCP_DEFAULT_HOST  "127.0.0.1"
/*! @def FREEPM_TCP_DEFAULT_PORT
    @brief Default TCP port (must match PMCLIENT expectations). */
#define FREEPM_TCP_DEFAULT_PORT  9876
/*! @def FREEPM_TCP_BACKLOG
    @brief listen() backlog. */
#define FREEPM_TCP_BACKLOG       5

/*! @def TCP_SERVER_MODE
    @brief TcpLink acts as a listener. */
#define TCP_SERVER_MODE          1
/*! @def TCP_CLIENT_MODE
    @brief TcpLink acts as a client. */
#define TCP_CLIENT_MODE          2

/*! @brief TCP transport: F_Link implementation over BSD sockets. */
class TcpLink : public F_Link {
public:
    PMC_SOCK sock;          /*!< Socket handle; PMC_INVALID_SOCK if closed. */
    int      mode;          /*!< TCP_SERVER_MODE or TCP_CLIENT_MODE. */
    int      connected;     /*!< Non-zero if handshake completed / accepted. */
    char     host[256];     /*!< Peer host (client mode). */
    int      port;          /*!< Peer or listen port. */

    TcpLink();
    TcpLink(const char *pszHost, int iPort, int iMode);
    virtual ~TcpLink();

    virtual int Open(void);
    virtual int Accept(F_Link **ppNew);
    virtual int Handshake(void);
    virtual int Send(const void *pvData, int cbData);
    virtual int Recv(void *pvData, int *pcbReceived, int cbBuffer);
    virtual int Close(void);
};

#endif /* F_TCP_HPP */
