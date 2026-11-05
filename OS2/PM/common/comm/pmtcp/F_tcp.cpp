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
#include <errno.h>
#include "F_tcp.hpp"

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
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TCPipe::Create ERROR: socket() failed. WSAError=%d",
                   PMC_LAST_SOCK_ERR());
        return -1;
    }

    /* Allow immediate reuse of the port after a previous server instance exits */
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(optval));

    memset(&sa, 0, sizeof(sa));
    sa.sin_family      = AF_INET;
    sa.sin_port        = htons((unsigned short)port);
    sa.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) == PMC_SOCK_ERROR) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TCPipe::Create ERROR: bind() failed. WSAError=%d",
                   PMC_LAST_SOCK_ERR());
        return -1;
    }
    if (listen(sock, FREEPM_TCP_BACKLOG) == PMC_SOCK_ERROR) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TCPipe::Create ERROR: listen() failed. WSAError=%d",
                   PMC_LAST_SOCK_ERR());
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
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TCPipe::Accept ERROR: accept() failed. WSAError=%d",
                   PMC_LAST_SOCK_ERR());
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
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TCPipe::Connect ERROR: socket() failed. WSAError=%d",
                   PMC_LAST_SOCK_ERR());
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
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TCPipe::Connect ERROR: connect() failed. WSAError=%d",
                   PMC_LAST_SOCK_ERR());
        return -1;
    }
    connected = 1;
    return 0;
}

/*! @brief Send data over the connection.

    Loops until all bytes are sent. Retries on EINTR / WSAEINTR.

    @param [in] pvData  Pointer to the payload.
    @param [in] cbData  Payload size in bytes.
    @return 0 on success, -1 on failure.
    @retval 0   All bytes were sent.
    @retval -1  Send failed or the connection is closed. */
int TCPipe::Send(const void *pvData, int cbData)
{
    const char *pc = (const char *)pvData;
    int total = 0;

    if (!connected) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TCPipe::Send ERROR: not connected");
        return -1;
    }
    while (total < cbData) {
        int iSent = send(sock, pc + total, cbData - total, 0);
        if (iSent == PMC_SOCK_ERROR) {
            int err = PMC_LAST_SOCK_ERR();
#if defined(__WIN32__) || defined(__NT__) || defined(_WIN32)
            if (err == WSAEINTR) continue;
#else
            if (err == EINTR) continue;
#endif
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TCPipe::Send ERROR: send() failed. WSAError=%d",
                       err);
            return -1;
        }
        total += iSent;
    }
    return 0;
}

/*! @brief Receive data from the connection.

    Loops until exactly cbBuffer bytes are read, or the peer closes
    (recv returns 0) / errors out. Retries on EINTR / WSAEINTR.

    @param [out] pvData       Destination buffer.
    @param [out] pcbReceived  Receives the number of bytes read.
                              On error, contains the partial count.
    @param [in]  cbBuffer     Number of bytes to read.
    @return 0 on success, -1 on failure.
    @retval 0   Exactly cbBuffer bytes were read.
    @retval -1  Receive failed, peer closed, or partial read. */
int TCPipe::Recv(void *pvData, int *pcbReceived, int cbBuffer)
{
    char *pc = (char *)pvData;
    int total = 0;

    if (!connected) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TCPipe::Recv ERROR: not connected");
        return -1;
    }
    while (total < cbBuffer) {
        int n = recv(sock, pc + total, cbBuffer - total, 0);
        if (n == 0) {                    /* orderly shutdown */
            *pcbReceived = total;
            return -1;
        }
        if (n < 0) {
            int err = PMC_LAST_SOCK_ERR();
#if defined(__WIN32__) || defined(__NT__) || defined(_WIN32)
            if (err == WSAEINTR) continue;
#else
            if (err == EINTR) continue;
#endif
            *pcbReceived = total;
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TCPipe::Recv ERROR: recv() failed. WSAError=%d",
                       err);
            return -1;
        }
        total += n;
    }
    *pcbReceived = total;
    return 0;
}

/*! @brief Close the underlying socket.

    @return 0.
    @retval 0  Always. */
int TCPipe::Close(void)
{
    if (sock != PMC_INVALID_SOCK)
    {
        shutdown(sock, 1);
        PMC_CLOSESOCKET(sock);
        sock = PMC_INVALID_SOCK;
    }
    connected = 0;
    return 0;
}

/*! @brief Perform the FreePM handshake over the connection.

    Server side: read exactly strlen(HAND_SHAKE_INP)+1 bytes,
    reply with HAND_SHAKE_OUT.
    Client side: send HAND_SHAKE_INP, expect HAND_SHAKE_OUT.

    @return 0 on success, HAND_SHAKE_ERROR on protocol mismatch or
            transport failure.
    @retval 0                 Handshake completed.
    @retval HAND_SHAKE_ERROR  Handshake mismatch or transport failure. */
int TCPipe::HandShake(void)
{
    char buf[32];
    int  cb;
    int  inpLen = (int)strlen(HAND_SHAKE_INP) + 1;   /* incl. NUL */
    int  outLen = (int)strlen(HAND_SHAKE_OUT) + 1;   /* incl. NUL */

    if (mode == TCP_SERVER_MODE)
    {
        if (Recv(buf, &cb, inpLen) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TCPipe::HandShake ERROR: Recv failed (server)");
            return HAND_SHAKE_ERROR;
        }
        if (cb != inpLen || memcmp(buf, HAND_SHAKE_INP, inpLen) != 0) {
            buf[cb < inpLen ? cb : inpLen - 1] = 0;   /* safe %s for log */
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TCPipe::HandShake ERROR: Invalid magic (server). "
                       "Expected '%s', got '%s'", HAND_SHAKE_INP, buf);
            return HAND_SHAKE_ERROR;
        }
        if (Send(HAND_SHAKE_OUT, outLen) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TCPipe::HandShake ERROR: Send failed (server)");
            return HAND_SHAKE_ERROR;
        }
        return 0;
    }

    if (Send(HAND_SHAKE_INP, inpLen) != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TCPipe::HandShake ERROR: Send failed (client)");
        return HAND_SHAKE_ERROR;
    }
    if (Recv(buf, &cb, outLen) != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TCPipe::HandShake ERROR: Recv failed (client)");
        return HAND_SHAKE_ERROR;
    }
    if (cb != outLen || memcmp(buf, HAND_SHAKE_OUT, outLen) != 0) {
        buf[cb < outLen ? cb : outLen - 1] = 0;   /* safe %s for log */
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TCPipe::HandShake ERROR: Invalid magic (client). "
                   "Expected '%s', got '%s'", HAND_SHAKE_OUT, buf);
        return HAND_SHAKE_ERROR;
    }
    return 0;
}
