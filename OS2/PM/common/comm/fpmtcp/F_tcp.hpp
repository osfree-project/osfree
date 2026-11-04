/*! @file F_tcp.hpp
    @brief TCP transport for FreePM: TcpLink class.
*/
#ifndef F_TCP_HPP
#define F_TCP_HPP

#include "F_platform.h"
#include "F_link.hpp"

#if defined(__OS2__) || defined(__OS2V2__)

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
  #define PMC_ERR_OPEN        ERROR_OPEN_FAILED
  #define PMC_ERR_BROKEN      ERROR_BROKEN_PIPE
  #define PMC_LAST_SOCK_ERR() sock_errno()

#else /* Win32 */

  #define PMC_CLOSESOCKET(s)  closesocket(s)
  typedef SOCKET PMC_SOCK;
  #define PMC_INVALID_SOCK    INVALID_SOCKET
  #define PMC_SOCK_ERROR      SOCKET_ERROR
  #define PMC_ERR_OPEN        WSAECONNREFUSED
  #define PMC_ERR_BROKEN      WSAECONNABORTED
  #define PMC_LAST_SOCK_ERR() WSAGetLastError()

#endif

#define FREEPM_TCP_DEFAULT_HOST  "127.0.0.1"
#define FREEPM_TCP_DEFAULT_PORT  9876
#define FREEPM_TCP_BACKLOG       5

#define TCP_SERVER_MODE   1
#define TCP_CLIENT_MODE   2

class TcpLink : public F_Link {
public:
    PMC_SOCK sock;
    int      mode;
    int      connected;
    char     host[256];
    int      port;

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
