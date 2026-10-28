/* F_tcp.hpp */
/* TCP backend for FreePM: connection object. */

#ifndef FREEPM_TCP_HPP
#define FREEPM_TCP_HPP

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

/* OpenWatcom/OS/2: BSD types (long_int, u_char, u_short, caddr_t)
   are in <types.h>, not in <sys/types.h>. */
#include <types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

/* OpenWatcom's <unistd.h> does not declare soclose() for OS/2.
   soclose() is the only correct way to close a socket on OS/2:
   read(), write() and close() do not work on sockets. */
extern "C" int _System soclose(int s);

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
    int     sock;        /*!< Socket handle, or -1. */
    int     mode;        /*!< TCP_CLIENT_MODE or TCP_SERVER_MODE. */
    int     connected;   /*!< Non-zero once connected. */
    char    host[256];   /*!< Peer host name or address. */
    int     port;        /*!< Peer port. */

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
