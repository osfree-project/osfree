/*! @file F_tcp.cpp
    @brief TCP backend for FreePM: socket-level implementation.

    Implements the TCPipe connection object used by PMTCP.DLL: client
    connect, server listen/accept, send/recv, close, and the standard
    FreePM handshake.
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "F_tcp.hpp"

/* --- ÍÀ×ÀËÎ ÏÐÀÂÎÊ: Ëîêàëüíàÿ ôóíêöèÿ îòëàäêè --- */
/*! @brief Print a debug message to stderr and OutputDebugString.
    @param [in] fmt  printf-style format string.
    @param [in] ...  Arguments for the format string. */
static void PMTCP_Dbg(const char *fmt, ...)
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
#if defined(__WIN32__) || defined(_WIN32)
    OutputDebugStringA(buf);
#endif
    fprintf(stderr, "%s", buf);
    fflush(stderr);
}
/* --- ÊÎÍÅÖ ÏÐÀÂÎÊ --- */

/*! @brief Construct a client-mode TCPipe with default host and port.

    @return Nothing. */
TCPipe::TCPipe()
{
    sock = PMC_INVALID_SOCK;
    connected = 0;
    strcpy(host, FREEPM_TCP_DEFAULT_HOST);
    port = FREEPM_TCP_DEFAULT_PORT;
    mode = TCP_CLIENT_MODE;
}

/*! @brief Construct a TCPipe with the given host, port and mode.

    @param [in] pszHost  Peer host name or address.
    @param [in] iPort    Peer TCP port.
    @param [in] iMode    TCP_CLIENT_MODE or TCP_SERVER_MODE.
    @return Nothing. */
TCPipe::TCPipe(const char *pszHost, int iPort, int iMode)
{
    sock = PMC_INVALID_SOCK;
    connected = 0;
    strncpy(host, pszHost, sizeof(host) - 1);
    host[sizeof(host) - 1] = 0;
    port = iPort;
    mode = iMode;
}

/*! @brief Destroy the TCPipe and close the underlying socket.

    @return Nothing. */
TCPipe::~TCPipe()
{
    Close();
}

/*! @brief Set up a listening socket (server mode).

    Creates a TCP socket, binds it to INADDR_ANY:port, and starts
    listening with a backlog of FREEPM_TCP_BACKLOG.

    @return 0 on success, -1 on failure.
    @retval 0   Socket is listening.
    @retval -1  Setup failed. */
int TCPipe::Create(void)
{
    struct sockaddr_in sa;
    int optval = 1;

    if (mode != TCP_SERVER_MODE) return -1;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == PMC_INVALID_SOCK) {
        PMTCP_Dbg("PMTCP: TCPipe::Create ERROR: socket() failed. WSAError=%d\n", WSAGetLastError());
        return -1;
    }

    /* Allow immediate reuse of the port after a previous server instance exits */
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(optval));

    memset(&sa, 0, sizeof(sa));
    sa.sin_family      = AF_INET;
    sa.sin_port        = htons((unsigned short)port);
    sa.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) == PMC_SOCK_ERROR) {
        PMTCP_Dbg("PMTCP: TCPipe::Create ERROR: bind() failed. WSAError=%d\n", WSAGetLastError());
        return -1;
    }
    if (listen(sock, FREEPM_TCP_BACKLOG) == PMC_SOCK_ERROR) {
        PMTCP_Dbg("PMTCP: TCPipe::Create ERROR: listen() failed. WSAError=%d\n", WSAGetLastError());
        return -1;
    }
    return 0;
}

/*! @brief Accept one incoming client connection (server mode).

    Blocks until a client connects, then stores the accepted socket in
    pClient and marks it connected.

    @param [out] pClient  Receives the accepted connection.
    @return 0 on success, -1 on failure.
    @retval 0   A connection was accepted.
    @retval -1  Accept failed. */
int TCPipe::Accept(TCPipe *pClient)
{
    struct sockaddr_in sa;
    int iLen = sizeof(sa);

    if (sock == PMC_INVALID_SOCK) return -1;

    pClient->sock = accept(sock, (struct sockaddr *)&sa, &iLen);
    if (pClient->sock == PMC_INVALID_SOCK) {
        PMTCP_Dbg("PMTCP: TCPipe::Accept ERROR: accept() failed. WSAError=%d\n", WSAGetLastError());
        return -1;
    }

    pClient->connected = 1;
    pClient->mode = TCP_SERVER_MODE;
    return 0;
}

/*! @brief Connect to a server (client mode).

    Creates a TCP socket, resolves the host name (falling back to
    inet_addr for numeric addresses), and connects to host:port.

    @return 0 on success, -1 on failure.
    @retval 0   Connected.
    @retval -1  Connect failed. */
int TCPipe::Connect(void)
{
    struct sockaddr_in sa;
    struct hostent *pHe;

    if (mode != TCP_CLIENT_MODE) return -1;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == PMC_INVALID_SOCK) {
        PMTCP_Dbg("PMTCP: TCPipe::Connect ERROR: socket() failed. WSAError=%d\n", WSAGetLastError());
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port   = htons((unsigned short)port);

    pHe = gethostbyname(host);
    if (pHe != NULL)
        memcpy(&sa.sin_addr, pHe->h_addr, pHe->h_length);
    else
        sa.sin_addr.s_addr = inet_addr(host);

    if (connect(sock, (struct sockaddr *)&sa, sizeof(sa)) == PMC_SOCK_ERROR) {
        PMTCP_Dbg("PMTCP: TCPipe::Connect ERROR: connect() failed. WSAError=%d\n", WSAGetLastError());
        return -1;
    }
    connected = 1;
    return 0;
}

/*! @brief Send data over the connection.

    @param [in] pvData  Pointer to the payload.
    @param [in] cbData  Payload size in bytes.
    @return 0 on success, -1 on failure.
    @retval 0   All bytes were sent.
    @retval -1  Send failed or the connection is closed. */
int TCPipe::Send(const void *pvData, int cbData)
{
    int iSent;

    if (!connected) {
        PMTCP_Dbg("PMTCP: TCPipe::Send ERROR: not connected\n");
        return -1;
    }
    iSent = send(sock, (const char *)pvData, cbData, 0);
    if (iSent == PMC_SOCK_ERROR) {
        PMTCP_Dbg("PMTCP: TCPipe::Send ERROR: send() failed. WSAError=%d\n", WSAGetLastError());
        return -1;
    }
    return (iSent == cbData) ? 0 : -1;
}

/*! @brief Receive data from the connection.

    @param [out] pvData       Destination buffer.
    @param [out] pcbReceived  Receives the number of bytes read.
    @param [in]  cbBuffer     Size of the destination buffer.
    @return 0 on success, -1 on failure.
    @retval 0   At least one byte was read.
    @retval -1  Receive failed or the connection is closed. */
int TCPipe::Recv(void *pvData, int *pcbReceived, int cbBuffer)
{
    int iRead;

    if (!connected) {
        PMTCP_Dbg("PMTCP: TCPipe::Recv ERROR: not connected\n");
        return -1;
    }
    iRead = recv(sock, (char *)pvData, cbBuffer, 0);
    if (iRead < 0) {
        PMTCP_Dbg("PMTCP: TCPipe::Recv ERROR: recv() failed. WSAError=%d\n", WSAGetLastError());
        return -1;
    }
    *pcbReceived = iRead;
    return 0;
}

/*! @brief Close the underlying socket.

    @return 0.
    @retval 0  Always. */
int TCPipe::Close(void)
{
    if (sock != PMC_INVALID_SOCK)
    {
        shutdown(sock, 2);
        PMC_CLOSESOCKET(sock);
        sock = PMC_INVALID_SOCK;
    }
    connected = 0;
    return 0;
}

/*! @brief Perform the FreePM handshake over the connection.

    Server side: read HAND_SHAKE_INP, reply HAND_SHAKE_OUT.
    Client side: send HAND_SHAKE_INP, expect HAND_SHAKE_OUT.

    @return 0 on success, HAND_SHAKE_ERROR on protocol mismatch or
            transport failure.
    @retval 0                 Handshake completed.
    @retval HAND_SHAKE_ERROR  Handshake mismatch or transport failure. */
int TCPipe::HandShake(void)
{
    char buf[32];
    int  cb;

    if (mode == TCP_SERVER_MODE)
    {
        if (Recv(buf, &cb, sizeof(buf)) != 0) {
            PMTCP_Dbg("PMTCP: TCPipe::HandShake ERROR: Recv failed (server)\n");
            return HAND_SHAKE_ERROR;
        }
        if (strcmp(buf, HAND_SHAKE_INP) != 0) {
            PMTCP_Dbg("PMTCP: TCPipe::HandShake ERROR: Invalid magic (server). Expected '%s', got '%s'\n", HAND_SHAKE_INP, buf);
            return HAND_SHAKE_ERROR;
        }
        if (Send(HAND_SHAKE_OUT, (int)strlen(HAND_SHAKE_OUT) + 1) != 0) {
            PMTCP_Dbg("PMTCP: TCPipe::HandShake ERROR: Send failed (server)\n");
            return HAND_SHAKE_ERROR;
        }
        return 0;
    }

    if (Send(HAND_SHAKE_INP, (int)strlen(HAND_SHAKE_INP) + 1) != 0) {
        PMTCP_Dbg("PMTCP: TCPipe::HandShake ERROR: Send failed (client)\n");
        return HAND_SHAKE_ERROR;
    }
    if (Recv(buf, &cb, sizeof(buf)) != 0) {
        PMTCP_Dbg("PMTCP: TCPipe::HandShake ERROR: Recv failed (client)\n");
        return HAND_SHAKE_ERROR;
    }
    if (strcmp(buf, HAND_SHAKE_OUT) != 0) {
        PMTCP_Dbg("PMTCP: TCPipe::HandShake ERROR: Invalid magic (client). Expected '%s', got '%s'\n", HAND_SHAKE_OUT, buf);
        return HAND_SHAKE_ERROR;
    }
    return 0;
}
