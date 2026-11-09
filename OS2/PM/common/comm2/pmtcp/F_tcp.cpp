/*! @file F_tcp.cpp
    @brief TCP transport for FreePM: TcpLink + 2 backend exports.

    Implements F_Link over BSD sockets. OS/2 socket API on both platforms
    (Win32 via F_platform.h emulation). WSA refcount in TcpLink ctor/dtor.
*/
#include <string.h>
#include <ctype.h>
#include <process.h>

#include "F_tcp.hpp"
#include "F_debug.h"

/* ======================================================================== */
/*  WSA refcount (Win32 only; no-op on OS/2)                               */
/* ======================================================================== */

#if defined(__WIN32__) || defined(__NT__) || defined(_WIN32)

/*! @brief Global WSA mutex protecting g_wsaRefCount. */
static HMTX g_wsaMtx = NULLHANDLE;
/*! @brief Number of live TcpLink objects. */
static int  g_wsaRefCount = 0;
/*! @brief One-shot guard for g_wsaMtx creation. */
static volatile LONG g_wsaMtxInit = 0;

/*! @brief Ensure g_wsaMtx exists, exactly once (atomic winner). */
static void TcpWsaLockInit(void)
{
    if (InterlockedCompareExchange(&g_wsaMtxInit, 1, 0) == 0) {
        DosCreateMutexSem(NULL, &g_wsaMtx, 0, FALSE);
    } else {
        while (g_wsaMtx == NULLHANDLE) DosSleep(0);
    }
}

/*! @brief Increment WSA refcount; WSAStartup on 0->1. */
static void TcpWsaRef(void)
{
    TcpWsaLockInit();
    DosRequestMutexSem(g_wsaMtx, SEM_INDEFINITE_WAIT);
    if (g_wsaRefCount++ == 0) {
        WSADATA wsa;
        WSAStartup(MAKEWORD(1, 1), &wsa);
    }
    DosReleaseMutexSem(g_wsaMtx);
}

/*! @brief Decrement WSA refcount; WSACleanup on 1->0. */
static void TcpWsaUnref(void)
{
    if (g_wsaMtx == NULLHANDLE) return;
    DosRequestMutexSem(g_wsaMtx, SEM_INDEFINITE_WAIT);
    if (--g_wsaRefCount == 0) {
        WSACleanup();
    }
    DosReleaseMutexSem(g_wsaMtx);
}

#else
/*! @brief No-op on OS/2. */
static void TcpWsaRef(void)   {}
/*! @brief No-op on OS/2. */
static void TcpWsaUnref(void) {}
#endif

/* ======================================================================== */
/*  Socket helpers                                                         */
/* ======================================================================== */

/*! @def PMC_CONNECT_TIMEOUT_MS
    @brief Non-blocking connect timeout, milliseconds. */
#define PMC_CONNECT_TIMEOUT_MS    5000

/*! @def PMC_HANDSHAKE_TIMEOUT_MS
    @brief Per-select() timeout during handshake, milliseconds. */
#define PMC_HANDSHAKE_TIMEOUT_MS  3000

/*! @def PMC_MAX_SOCK_CHUNK
    @brief Max bytes per single send()/recv() (bounded by INT_MAX). */
#define PMC_MAX_SOCK_CHUNK        0x40000000UL

/*! @brief Put a socket into non-blocking mode.
    @param [in] s  Socket.
    @return 0 on success, -1 on ioctl failure.
    @retval 0   Non-blocking mode set.
    @retval -1  ioctl/ioctlsocket failed. */
static int SetNonBlocking(PMC_SOCK s)
{
#if defined(__OS2__) || defined(__OS2V2__)
    int arg = 1;
    return ioctl(s, FIONBIO, (char *)&arg, sizeof(arg));
#else
    unsigned long arg = 1;
    return ioctlsocket(s, FIONBIO, &arg);
#endif
}

/*! @brief Put a socket into blocking mode.
    @param [in] s  Socket.
    @return 0 on success, -1 on ioctl failure.
    @retval 0   Blocking mode set.
    @retval -1  ioctl/ioctlsocket failed. */
static int SetBlocking(PMC_SOCK s)
{
#if defined(__OS2__) || defined(__OS2V2__)
    int arg = 0;
    return ioctl(s, FIONBIO, (char *)&arg, sizeof(arg));
#else
    unsigned long arg = 0;
    return ioctlsocket(s, FIONBIO, &arg);
#endif
}

/*! @brief Test if a connect() error means "in progress".
    @param [in] err  Socket error code.
    @return Non-zero if non-blocking connect is still running.
    @retval 0  No, error is terminal.
    @retval 1  Yes, connect is in progress. */
static int IsConnectInProgress(int err)
{
#if defined(__OS2__) || defined(__OS2V2__)
    return (err == EINPROGRESS || err == EWOULDBLOCK);
#else
    return (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS);
#endif
}

/*! @brief Test if a socket error is a signal interruption.
    @param [in] err  Socket error code.
    @return Non-zero if the call was interrupted and may be retried.
    @retval 0  No, error is terminal.
    @retval 1  Yes, EINTR / WSAEINTR. */
static int IsInterrupted(int err)
{
#if defined(__OS2__) || defined(__OS2V2__)
    return (err == EINTR);
#else
    return (err == WSAEINTR);
#endif
}

/*! @brief select() on one socket with EINTR-retry.
    @param [in] s           Socket.
    @param [in] for_write   Non-zero: wait for writability. Zero: readability.
    @param [in] timeout_ms  Timeout in milliseconds.
    @return 1 ready, 0 timeout, -1 error.
    @retval 1   Socket ready.
    @retval 0   Timeout.
    @retval -1  select() failed (non-EINTR). */
static int SelectOne(PMC_SOCK s, int for_write, int timeout_ms)
{
    fd_set fds;
    struct timeval tv;
    int n;

    for (;;) {
        FD_ZERO(&fds);
        FD_SET(s, &fds);
        tv.tv_sec  = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        n = select(PMC_NFDS(s),
                   for_write ? NULL : &fds,
                   for_write ? &fds : NULL,
                   NULL, &tv);
        if (n < 0) {
            if (IsInterrupted(PMC_LAST_SOCK_ERR())) continue;
            return -1;
        }
        return n;
    }
}

/*! @brief Wait for a non-blocking connect to complete.
    @param [in]  s           Socket with connect() pending.
    @param [in]  timeout_ms  Timeout, milliseconds.
    @param [out] err_out     -1 on timeout, otherwise socket error code
                             (0 on success).
    @return 0 on success, -1 on timeout or failure.
    @retval 0   Connect completed successfully.
    @retval -1  Timeout or connect failed. */
static int WaitConnect(PMC_SOCK s, int timeout_ms, int *err_out)
{
    int n, err, errlen;

    *err_out = 0;

    n = SelectOne(s, 1, timeout_ms);
    if (n == 0) { *err_out = -1; return -1; }
    if (n < 0)  { *err_out = PMC_LAST_SOCK_ERR(); return -1; }

    err = 0;
    errlen = sizeof(err);
    if (getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&err, &errlen) != 0) {
        *err_out = PMC_LAST_SOCK_ERR();
        return -1;
    }
    if (err != 0) { *err_out = err; return -1; }
    return 0;
}

/*! @brief Receive exactly len bytes with a per-chunk timeout.
    @param [in]  s           Connected socket.
    @param [out] buf         Destination.
    @param [out] pcb         Bytes received (only on success).
    @param [in]  len         Bytes required.
    @param [in]  timeout_ms  Timeout per select(), milliseconds.
    @return 0 on success, -1 on timeout or error.
    @retval 0   Exactly len bytes received.
    @retval -1  Timeout or transport error. */
static int RecvWithTimeout(PMC_SOCK s, void *buf, ULONG *pcb, ULONG len,
                           int timeout_ms)
{
    char *p = (char *)buf;
    ULONG total = 0;

    if (pcb) *pcb = 0;
    if (len == 0) return 0;

    while (total < len) {
        ULONG rem = len - total;
        int   chunk = (rem > PMC_MAX_SOCK_CHUNK) ? (int)PMC_MAX_SOCK_CHUNK : (int)rem;
        int   n;

        n = SelectOne(s, 0, timeout_ms);
        if (n <= 0) return -1;
        n = recv(s, p + total, chunk, 0);
        if (n <= 0) return -1;
        total += (ULONG)n;
    }
    if (pcb) *pcb = total;
    return 0;
}

/*! @brief Send exactly len bytes with a per-chunk timeout.
    @param [in] s           Connected socket.
    @param [in] buf         Source.
    @param [in] len         Bytes to send.
    @param [in] timeout_ms  Timeout per select(), milliseconds.
    @return 0 on success, -1 on timeout or error.
    @retval 0   Exactly len bytes sent.
    @retval -1  Timeout or transport error. */
static int SendWithTimeout(PMC_SOCK s, const void *buf, ULONG len, int timeout_ms)
{
    const char *p = (const char *)buf;
    ULONG total = 0;

    if (len == 0) return 0;

    while (total < len) {
        ULONG rem = len - total;
        int   chunk = (rem > PMC_MAX_SOCK_CHUNK) ? (int)PMC_MAX_SOCK_CHUNK : (int)rem;
        int   n;

        n = SelectOne(s, 1, timeout_ms);
        if (n <= 0) return -1;
        n = send(s, p + total, chunk, 0);
        if (n <= 0) return -1;
        total += (ULONG)n;
    }
    return 0;
}

/* ======================================================================== */
/*  TcpLink                                                                */
/* ======================================================================== */

/*! @brief Construct a client TcpLink with default host and port.
    Socket is not opened. WSA refcount incremented (Win32).
    @return Nothing. */
TcpLink::TcpLink()
    : sock(PMC_INVALID_SOCK), mode(TCP_CLIENT_MODE), connected(0),
      port(FREEPM_TCP_DEFAULT_PORT)
{
    strcpy(host, FREEPM_TCP_DEFAULT_HOST);
    TcpWsaRef();
}

/*! @brief Construct a TcpLink with explicit host, port and mode.
    Socket is not opened. WSA refcount incremented (Win32).
    @param [in] pszHost  Peer host (or listen host).
    @param [in] iPort    Port.
    @param [in] iMode    TCP_CLIENT_MODE or TCP_SERVER_MODE.
    @return Nothing. */
TcpLink::TcpLink(const char *pszHost, int iPort, int iMode)
    : sock(PMC_INVALID_SOCK), mode(iMode), connected(0), port(iPort)
{
    strncpy(host, pszHost, sizeof(host) - 1);
    host[sizeof(host) - 1] = 0;
    TcpWsaRef();
}

/*! @brief Destroy the TcpLink. Closes socket; WSA refcount decremented.
    @return Nothing. */
TcpLink::~TcpLink()
{
    Close();
    TcpWsaUnref();
}

/*! @brief Open the link: bind+listen (server) or connect (client).
    @return 0 on success, -1 on failure.
    @retval 0   Listening socket ready (server) or peer connected (client).
    @retval -1  Socket/bind/listen/connect/ioctl/resolve failure. */
int TcpLink::Open(void)
{
    struct sockaddr_in sa;
    int optval;
    PMC_SOCK s;

    if (sock != PMC_INVALID_SOCK) return -1;

    if (mode == TCP_SERVER_MODE) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == PMC_INVALID_SOCK) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TcpLink::Open ERROR: socket() failed. WSAError=%d",
                       PMC_LAST_SOCK_ERR());
            return -1;
        }

        optval = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                   (const char *)&optval, sizeof(optval));

        memset(&sa, 0, sizeof(sa));
        sa.sin_family      = AF_INET;
        sa.sin_port        = htons((unsigned short)port);
        sa.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TcpLink::Open ERROR: bind() failed. WSAError=%d",
                       PMC_LAST_SOCK_ERR());
            PMC_CLOSESOCKET(sock);
            sock = PMC_INVALID_SOCK;
            return -1;
        }
        if (listen(sock, FREEPM_TCP_BACKLOG) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TcpLink::Open ERROR: listen() failed. WSAError=%d",
                       PMC_LAST_SOCK_ERR());
            PMC_CLOSESOCKET(sock);
            sock = PMC_INVALID_SOCK;
            return -1;
        }
        return 0;
    }
    else {
        struct hostent *pHe;
        ULONG   inaddr;
        int     rc, err;

        inaddr = inet_addr(host);
        if (inaddr == INADDR_NONE) {
            pHe = gethostbyname(host);
            if (pHe == NULL || pHe->h_addrtype != AF_INET ||
                pHe->h_length != (int)sizeof(inaddr)) {
                F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                           "PMTCP: TcpLink::Open ERROR: cannot resolve '%s'",
                           host);
                return -1;
            }
            memcpy(&inaddr, pHe->h_addr, sizeof(inaddr));
        }

        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == PMC_INVALID_SOCK) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TcpLink::Open ERROR: socket() failed. WSAError=%d",
                       PMC_LAST_SOCK_ERR());
            return -1;
        }

        if (SetNonBlocking(s) != 0) {
            err = PMC_LAST_SOCK_ERR();
            PMC_CLOSESOCKET(s);
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TcpLink::Open ERROR: SetNonBlocking failed. "
                       "WSAError=%d", err);
            return -1;
        }

        memset(&sa, 0, sizeof(sa));
        sa.sin_family      = AF_INET;
        sa.sin_port        = htons((unsigned short)port);
        sa.sin_addr.s_addr = inaddr;

        rc = connect(s, (struct sockaddr *)&sa, sizeof(sa));
        if (rc != 0) {
            err = PMC_LAST_SOCK_ERR();
            if (!IsConnectInProgress(err)) {
                PMC_CLOSESOCKET(s);
                F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                           "PMTCP: TcpLink::Open ERROR: connect() failed. "
                           "WSAError=%d", err);
                return -1;
            }
            if (WaitConnect(s, PMC_CONNECT_TIMEOUT_MS, &err) != 0) {
                PMC_CLOSESOCKET(s);
                if (err == -1) {
                    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                               "PMTCP: TcpLink::Open ERROR: connect timeout "
                               "after %d ms", PMC_CONNECT_TIMEOUT_MS);
                } else {
                    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                               "PMTCP: TcpLink::Open ERROR: connect failed, "
                               "WSAError=%d", err);
                }
                return -1;
            }
        }

        if (SetBlocking(s) != 0) {
            err = PMC_LAST_SOCK_ERR();
            PMC_CLOSESOCKET(s);
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TcpLink::Open ERROR: SetBlocking failed. "
                       "WSAError=%d", err);
            return -1;
        }

        optval = 1;
        setsockopt(s, SOL_SOCKET, SO_KEEPALIVE,
                   (const char *)&optval, sizeof(optval));

        sock = s;
        connected = 1;
        return 0;
    }
}

/*! @brief Accept one incoming client (server mode).
    Sets TCP_NODELAY and SO_KEEPALIVE on the accepted socket.
    @param [out] ppNew  Receives a new TcpLink for the accepted connection.
    @return 0 on success, -1 on failure.
    @retval 0   Connection accepted.
    @retval -1  accept() failed or bad args. */
int TcpLink::Accept(F_Link **ppNew)
{
    struct sockaddr_in sa;
    int iLen = sizeof(sa);
    int optval;
    TcpLink *pClient;

    if (sock == PMC_INVALID_SOCK) return -1;
    if (ppNew == NULL) return -1;

    pClient = new TcpLink();
    if (!pClient) return -1;

    pClient->sock = accept(sock, (struct sockaddr *)&sa, &iLen);
    if (pClient->sock == PMC_INVALID_SOCK) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TcpLink::Accept ERROR: accept() failed. WSAError=%d",
                   PMC_LAST_SOCK_ERR());
        delete pClient;
        return -1;
    }

    optval = 1;
    setsockopt(pClient->sock, IPPROTO_TCP, TCP_NODELAY,
               (const char *)&optval, sizeof(optval));
    setsockopt(pClient->sock, SOL_SOCKET, SO_KEEPALIVE,
               (const char *)&optval, sizeof(optval));

    pClient->connected = 1;
    pClient->mode      = TCP_SERVER_MODE;
    *ppNew = pClient;
    return 0;
}

/*! @brief Perform the FreePM handshake.
    Uses PMC_HANDSHAKE_TIMEOUT_MS per select() so a stalled peer cannot
    block the accept thread indefinitely.
    @return 0 on success, HAND_SHAKE_ERROR on protocol mismatch/transport error.
    @retval 0                 Handshake completed.
    @retval HAND_SHAKE_ERROR  Magic mismatch or transport failure. */
int TcpLink::Handshake(void)
{
    char  buf[32];
    ULONG cb;
    ULONG inpLen = (ULONG)strlen(HAND_SHAKE_INP) + 1;
    ULONG outLen = (ULONG)strlen(HAND_SHAKE_OUT) + 1;
    int   rc;

    if (inpLen > (ULONG)sizeof(buf) || outLen > (ULONG)sizeof(buf)) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TcpLink::Handshake ERROR: buffer too small");
        return HAND_SHAKE_ERROR;
    }

    if (mode == TCP_SERVER_MODE) {
        rc = RecvWithTimeout(sock, buf, &cb, inpLen, PMC_HANDSHAKE_TIMEOUT_MS);
        if (rc != 0 || cb != inpLen) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TcpLink::Handshake ERROR: Recv failed (server)");
            return HAND_SHAKE_ERROR;
        }
        if (memcmp(buf, HAND_SHAKE_INP, inpLen) != 0) {
            buf[inpLen - 1] = 0;
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TcpLink::Handshake ERROR: Invalid magic "
                       "(server). Expected '%s', got '%s'",
                       HAND_SHAKE_INP, buf);
            return HAND_SHAKE_ERROR;
        }
        rc = SendWithTimeout(sock, HAND_SHAKE_OUT, outLen,
                             PMC_HANDSHAKE_TIMEOUT_MS);
        if (rc != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TcpLink::Handshake ERROR: Send failed (server)");
            return HAND_SHAKE_ERROR;
        }
        return 0;
    }

    rc = SendWithTimeout(sock, HAND_SHAKE_INP, inpLen, PMC_HANDSHAKE_TIMEOUT_MS);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TcpLink::Handshake ERROR: Send failed (client)");
        return HAND_SHAKE_ERROR;
    }
    rc = RecvWithTimeout(sock, buf, &cb, outLen, PMC_HANDSHAKE_TIMEOUT_MS);
    if (rc != 0 || cb != outLen) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TcpLink::Handshake ERROR: Recv failed (client)");
        return HAND_SHAKE_ERROR;
    }
    if (memcmp(buf, HAND_SHAKE_OUT, outLen) != 0) {
        buf[outLen - 1] = 0;
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TcpLink::Handshake ERROR: Invalid magic (client). "
                   "Expected '%s', got '%s'", HAND_SHAKE_OUT, buf);
        return HAND_SHAKE_ERROR;
    }
    return 0;
}

/*! @brief Send exactly cbData bytes. Loops on partial send, EINTR-retry.
    @param [in] pvData  Source buffer.
    @param [in] cbData  Bytes to send (0 is a no-op).
    @return 0 on success, -1 on error.
    @retval 0   All bytes sent (or cbData == 0).
    @retval -1  Not connected or send() failed. */
int TcpLink::Send(const void *pvData, ULONG cbData)
{
    const char *pc = (const char *)pvData;
    ULONG total = 0;

    if (cbData == 0) return 0;
    if (!connected) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TcpLink::Send ERROR: not connected");
        return -1;
    }
    if (!pvData) return -1;

    while (total < cbData) {
        ULONG rem = cbData - total;
        int   chunk = (rem > PMC_MAX_SOCK_CHUNK) ? (int)PMC_MAX_SOCK_CHUNK : (int)rem;
        int   iSent = send(sock, pc + total, chunk, 0);

        if (iSent == PMC_SOCK_ERROR) {
            int err = PMC_LAST_SOCK_ERR();
            if (IsInterrupted(err)) continue;
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TcpLink::Send ERROR: send() failed. WSAError=%d",
                       err);
            return -1;
        }
        if (iSent == 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TcpLink::Send ERROR: send() returned 0");
            return -1;
        }
        total += (ULONG)iSent;
    }
    return 0;
}

/*! @brief Receive exactly cbBuffer bytes. Loops on partial recv, EINTR-retry.
    On peer-close sets *pcbReceived = bytes-read-so-far and returns -1.
    @param [out] pvData       Destination.
    @param [out] pcbReceived  Bytes received.
    @param [in]  cbBuffer     Bytes required.
    @return 0 on success, -1 on peer-close or error.
    @retval 0   Exactly cbBuffer bytes received.
    @retval -1  Not connected, peer close, or recv() error. */
int TcpLink::Recv(void *pvData, ULONG *pcbReceived, ULONG cbBuffer)
{
    char *pc = (char *)pvData;
    ULONG total = 0;

    if (pcbReceived) *pcbReceived = 0;
    if (cbBuffer == 0) return 0;
    if (!connected) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: TcpLink::Recv ERROR: not connected");
        return -1;
    }
    if (!pvData) return -1;

    while (total < cbBuffer) {
        ULONG rem = cbBuffer - total;
        int   chunk = (rem > PMC_MAX_SOCK_CHUNK) ? (int)PMC_MAX_SOCK_CHUNK : (int)rem;
        int   n = recv(sock, pc + total, chunk, 0);

        if (n == 0) {
            if (pcbReceived) *pcbReceived = total;
            return -1;
        }
        if (n < 0) {
            int err = PMC_LAST_SOCK_ERR();
            if (IsInterrupted(err)) continue;
            if (pcbReceived) *pcbReceived = total;
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMTCP: TcpLink::Recv ERROR: recv() failed. WSAError=%d",
                       err);
            return -1;
        }
        total += (ULONG)n;
    }
    if (pcbReceived) *pcbReceived = total;
    return 0;
}

/*! @brief Close the socket. Idempotent.
    @return 0 always.
    @retval 0  Always. */
int TcpLink::Close(void)
{
    if (sock != PMC_INVALID_SOCK) {
        shutdown(sock, SD_SEND);
        PMC_CLOSESOCKET(sock);
        sock = PMC_INVALID_SOCK;
    }
    connected = 0;
    return 0;
}

/* ======================================================================== */
/*  Address parsing                                                        */
/* ======================================================================== */

/*! @brief Parse "host" or "host:port" into host and port.

    Rejects whitespace, bare IPv6 (multiple colons), empty host, non-numeric
    or out-of-range port. NULL/empty input applies defaults.

    @param [in]  s         Address string (may be NULL).
    @param [out] host_out  Receives host (NUL-terminated).
    @param [in]  host_size Capacity of host_out.
    @param [out] port_out  Receives port.
    @return 0 on success, -1 on malformed input.
    @retval 0   Parsed (or defaults applied).
    @retval -1  Malformed address. */
static int ParseHostPort(const char *s, char *host_out, int host_size,
                         int *port_out)
{
    const char *colon;
    const char *p;
    int hlen, plen, i;

    if (host_size <= 0 || !host_out || !port_out) return -1;

    if (!s || !*s) {
        strncpy(host_out, FREEPM_TCP_DEFAULT_HOST, (size_t)host_size - 1);
        host_out[host_size - 1] = 0;
        *port_out = FREEPM_TCP_DEFAULT_PORT;
        return 0;
    }

    for (p = s; *p; p++) {
        if (isspace((unsigned char)*p)) return -1;
    }

    colon = strchr(s, ':');
    if (colon == NULL) {
        hlen = (int)strlen(s);
        if (hlen <= 0 || hlen >= host_size) return -1;
        strcpy(host_out, s);
        *port_out = FREEPM_TCP_DEFAULT_PORT;
        return 0;
    }

    if (strchr(colon + 1, ':') != NULL) return -1;

    hlen = (int)(colon - s);
    plen = (int)strlen(colon + 1);
    if (hlen <= 0 || hlen >= host_size || plen <= 0 || plen > 5) return -1;

    memcpy(host_out, s, (size_t)hlen);
    host_out[hlen] = 0;
    *port_out = 0;
    for (i = 0; i < plen; i++) {
        int d = colon[1 + i] - '0';
        if (d < 0 || d > 9) return -1;
        *port_out = *port_out * 10 + d;
    }
    if (*port_out < 1 || *port_out > 65535) return -1;
    return 0;
}

/* ======================================================================== */
/*  Backend exports                                                        */
/* ======================================================================== */

/*! @brief Open a client TCP connection and perform the handshake.

    Address format: "host" or "host:port". *obj is always written:
    0 on any error, non-zero on success.

    @param [in]  address  Server address, or NULL for defaults.
    @param [out] obj      Receives the connection object; set to 0 on error.
    @return 0 on success, non-zero on failure.
    @retval 0                  Connected.
    @retval PMC_ERR_OPEN       Bad address, connect failed, or new failed.
    @retval HAND_SHAKE_ERROR   Handshake mismatch. */
extern "C" APIRET EXPENTRY Fc_TrnConnect(char *address, ULONG *obj)
{
    char     host[256];
    int      port = FREEPM_TCP_DEFAULT_PORT;
    TcpLink *pNew;

    if (!obj) return PMC_ERR_OPEN;
    *obj = 0;

    if (ParseHostPort(address, host, sizeof(host), &port) != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fc_TrnConnect ERROR: bad address '%s'",
                   address ? address : "NULL");
        return PMC_ERR_OPEN;
    }

    pNew = new TcpLink(host, port, TCP_CLIENT_MODE);
    if (pNew == NULL) return PMC_ERR_OPEN;

    if (pNew->Open() != 0) {
        delete pNew;
        return PMC_ERR_OPEN;
    }

    if (pNew->Handshake() == HAND_SHAKE_ERROR) {
        delete pNew;
        return HAND_SHAKE_ERROR;
    }

    *obj = (ULONG)pNew;
    return 0;
}

/*! @brief Server-side listener loop: bind, listen, accept, spawn thread.

    Does not return on success.

    @param [in] handler  Command handler callback (PMC_HANDLER).
    @param [in] disc_cb  Disconnect callback (PMC_DISCONNECT_CB), may be NULL.
    @return PMC_ERR_OPEN on setup failure; does not return otherwise.
    @retval PMC_ERR_OPEN  new/listen/Open failed.
    @retval 0             Unreachable under normal operation. */
extern "C" APIRET EXPENTRY Fs_TrnRunListener(void *handler, void *disc_cb)
{
    PMC_HANDLER       h    = (PMC_HANDLER)handler;
    PMC_DISCONNECT_CB disc = (PMC_DISCONNECT_CB)disc_cb;
    TcpLink          *pServer;

    pServer = new TcpLink(FREEPM_TCP_DEFAULT_HOST, FREEPM_TCP_DEFAULT_PORT,
                          TCP_SERVER_MODE);
    if (!pServer) return PMC_ERR_OPEN;

    if (pServer->Open() != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fs_TrnRunListener: Open failed (port %d)",
                   FREEPM_TCP_DEFAULT_PORT);
        delete pServer;
        return PMC_ERR_OPEN;
    }

    Fs_RunServer(pServer, h, disc);

    delete pServer;
    return 0;
}
