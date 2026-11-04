/*! @file F_tcp.cpp
    @brief TCP transport for FreePM.

    TcpLink + the exports that this backend must provide:
      _InitServerConnection, _CloseServerConnection, _startServerThreads,
      and the DLL entry point.
*/
#include <string.h>
#include <process.h>

#include "F_platform.h"
#include "F_tcp.hpp"
#include "F_wire.hpp"
#include "F_debug.hpp"

#if defined(__WIN32__) || defined(__NT__) || defined(_WIN32)

static int TcpWsaStartup(void)
{
    WSADATA wsa;
    return WSAStartup(MAKEWORD(1, 1), &wsa);
}

static void TcpWsaCleanup(void)
{
    WSACleanup();
}

#else

static int  TcpWsaStartup(void)  { return 0; }
static void TcpWsaCleanup(void)  {}

#endif

/* ======================================================================== */
/*  TcpLink                                                                 */
/* ======================================================================== */

TcpLink::TcpLink()
    : sock(PMC_INVALID_SOCK), mode(TCP_CLIENT_MODE), connected(0),
      port(FREEPM_TCP_DEFAULT_PORT)
{
    strcpy(host, FREEPM_TCP_DEFAULT_HOST);
}

TcpLink::TcpLink(const char *pszHost, int iPort, int iMode)
    : sock(PMC_INVALID_SOCK), mode(iMode), connected(0), port(iPort)
{
    strncpy(host, pszHost, sizeof(host) - 1);
    host[sizeof(host) - 1] = 0;
}

TcpLink::~TcpLink()
{
    Close();
}

int TcpLink::Open(void)
{
    if (mode == TCP_SERVER_MODE)
    {
        struct sockaddr_in sa;
        int optval = 1;

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == PMC_INVALID_SOCK) {
            F_Dbg("TCP: socket() failed, err=%d\n", PMC_LAST_SOCK_ERR());
            return -1;
        }
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                   (const char *)&optval, sizeof(optval));

        memset(&sa, 0, sizeof(sa));
        sa.sin_family      = AF_INET;
        sa.sin_port        = htons((unsigned short)port);
        sa.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) == PMC_SOCK_ERROR) {
            F_Dbg("TCP: bind() failed, err=%d\n", PMC_LAST_SOCK_ERR());
            return -1;
        }
        if (listen(sock, FREEPM_TCP_BACKLOG) == PMC_SOCK_ERROR) {
            F_Dbg("TCP: listen() failed, err=%d\n", PMC_LAST_SOCK_ERR());
            return -1;
        }
        return 0;
    }
    else
    {
        struct sockaddr_in sa;
        struct hostent *pHe;

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == PMC_INVALID_SOCK) {
            F_Dbg("TCP: socket() failed, err=%d\n", PMC_LAST_SOCK_ERR());
            return -1;
        }

        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port   = htons((unsigned short)port);

        pHe = gethostbyname(host);
        if (pHe != NULL) memcpy(&sa.sin_addr, pHe->h_addr, pHe->h_length);
        else             sa.sin_addr.s_addr = inet_addr(host);

        if (connect(sock, (struct sockaddr *)&sa, sizeof(sa)) == PMC_SOCK_ERROR) {
            F_Dbg("TCP: connect() failed, err=%d\n", PMC_LAST_SOCK_ERR());
            return -1;
        }
        connected = 1;
        return 0;
    }
}

int TcpLink::Accept(F_Link **ppNew)
{
    struct sockaddr_in sa;
    int iLen = sizeof(sa);
    TcpLink *pClient;

    if (sock == PMC_INVALID_SOCK) return -1;

    pClient = new TcpLink();
    if (!pClient) return -1;

    pClient->sock = accept(sock, (struct sockaddr *)&sa, &iLen);
    if (pClient->sock == PMC_INVALID_SOCK) {
        F_Dbg("TCP: accept() failed, err=%d\n", PMC_LAST_SOCK_ERR());
        delete pClient;
        return -1;
    }
    pClient->connected = 1;
    pClient->mode      = TCP_SERVER_MODE;
    *ppNew = pClient;
    return 0;
}

int TcpLink::Handshake(void)
{
    char buf[32];
    int  cb;

    if (mode == TCP_SERVER_MODE) {
        if (Recv(buf, &cb, (int)strlen(HAND_SHAKE_INP) + 1) != 0) return HAND_SHAKE_ERROR;
        if (strcmp(buf, HAND_SHAKE_INP) != 0) return HAND_SHAKE_ERROR;
        if (Send(HAND_SHAKE_OUT, (int)strlen(HAND_SHAKE_OUT) + 1) != 0) return HAND_SHAKE_ERROR;
    } else {
        if (Send(HAND_SHAKE_INP, (int)strlen(HAND_SHAKE_INP) + 1) != 0) return HAND_SHAKE_ERROR;
        if (Recv(buf, &cb, (int)strlen(HAND_SHAKE_OUT) + 1) != 0) return HAND_SHAKE_ERROR;
        if (strcmp(buf, HAND_SHAKE_OUT) != 0) return HAND_SHAKE_ERROR;
    }
    return 0;
}

int TcpLink::Send(const void *pvData, int cbData)
{
    const char *p = (const char *)pvData;
    int sent = 0;

    if (!connected) return -1;
    while (sent < cbData) {
        int r = send(sock, p + sent, cbData - sent, 0);
        if (r <= 0) {
            F_Dbg("TCP: send() failed, err=%d\n", PMC_LAST_SOCK_ERR());
            return -1;
        }
        sent += r;
    }
    return 0;
}

int TcpLink::Recv(void *pvData, int *pcbReceived, int cbBuffer)
{
    char *p = (char *)pvData;
    int   got = 0;

    if (!connected) return -1;
    while (got < cbBuffer) {
        int r = recv(sock, p + got, cbBuffer - got, 0);
        if (r <= 0) {
            F_Dbg("TCP: recv() failed, err=%d\n", PMC_LAST_SOCK_ERR());
            return -1;
        }
        got += r;
    }
    *pcbReceived = got;
    return 0;
}

int TcpLink::Close(void)
{
    if (sock != PMC_INVALID_SOCK) {
        shutdown(sock, 1);
        PMC_CLOSESOCKET(sock);
        sock = PMC_INVALID_SOCK;
    }
    connected = 0;
    return 0;
}

/* ======================================================================== */
/*  Exports                                                                 */
/* ======================================================================== */

extern "C" APIRET EXPENTRY _InitServerConnection(char *ExternMachine, ULONG *obj,
                                                 void *cb)
{
    TcpLink *pLink;

    F_Dbg("FPMTCP: _InitServerConnection IN -> ExternMachine=%s, cb=%p\n",
          ExternMachine ? ExternMachine : "NULL", cb);

    if (cb) g_shutdownCb = (PMC_SHUTDOWN_CB)cb;

    if (TcpWsaStartup() != 0) return PMC_ERR_OPEN;

    if (g_pClientLink != NULL) {
        delete g_pClientLink;
        g_pClientLink = NULL;
    }

    pLink = new TcpLink(ExternMachine ? ExternMachine : FREEPM_TCP_DEFAULT_HOST,
                        FREEPM_TCP_DEFAULT_PORT, TCP_CLIENT_MODE);
    if (!pLink) { TcpWsaCleanup(); return PMC_ERR_OPEN; }

    if (pLink->Open() != 0) { delete pLink; TcpWsaCleanup(); return PMC_ERR_OPEN; }
    if (pLink->Handshake() != 0) {
        pLink->Close();
        delete pLink;
        TcpWsaCleanup();
        return HAND_SHAKE_ERROR;
    }

    g_pClientLink = pLink;
    F_WireRegisterFatalSink();
    *obj = (ULONG)pLink;

    F_Dbg("FPMTCP: _InitServerConnection OUT -> Success (obj=%p)\n", pLink);
    return 0;
}

extern "C" APIRET EXPENTRY _CloseServerConnection(void)
{
    F_Dbg("FPMTCP: _CloseServerConnection IN\n");
    F_DbgSetWireSink(NULL);
    if (g_pClientLink != NULL) {
        delete g_pClientLink;
        g_pClientLink = NULL;
    }
    TcpWsaCleanup();
    return 0;
}

extern "C" APIRET EXPENTRY _startServerThreads(void *handl, void *disc_cb)
{
    PMC_HANDLER       h    = (PMC_HANDLER)handl;
    PMC_DISCONNECT_CB disc = (PMC_DISCONNECT_CB)disc_cb;
    TcpLink          *pListener;

    if (TcpWsaStartup() != 0) return PMC_ERR_OPEN;

    pListener = new TcpLink(FREEPM_TCP_DEFAULT_HOST, FREEPM_TCP_DEFAULT_PORT,
                            TCP_SERVER_MODE);
    if (!pListener) { TcpWsaCleanup(); return PMC_ERR_OPEN; }

    if (pListener->Open() != 0) {
        F_Dbg("FPMTCP: listener Open failed\n");
        delete pListener;
        TcpWsaCleanup();
        return PMC_ERR_OPEN;
    }

    F_Dbg("FPMTCP: listening on port %d\n", FREEPM_TCP_DEFAULT_PORT);

    F_RunServer(pListener, h, disc);

    delete pListener;
    TcpWsaCleanup();
    return 0;
}

/* ======================================================================== */
/*  DLL entry point                                                         */
/* ======================================================================== */

#if defined(__OS2__) || defined(__OS2V2__)

unsigned long _System _DLL_InitTerm(unsigned long hModule, unsigned long flag)
{
    (void)hModule;
    if (flag == 0) return 1;

    if (g_shutdownCb) {
        g_shutdownCb();
        g_shutdownCb = NULL;
    }
    F_DbgSetWireSink(NULL);
    if (g_pClientLink) {
        delete g_pClientLink;
        g_pClientLink = NULL;
    }
    TcpWsaCleanup();
    return 1;
}

#else /* Win32 */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    (void)hinstDLL; (void)lpReserved;

    if (fdwReason == DLL_PROCESS_DETACH)
    {
        /* Re-init WSA: ws2_32.dll was detached before us (LIFO), so
           its Winsock state is reset. Re-init so the shutdown callback
           can call send() one last time. */
        WSADATA wsa;
        int wsaOk = (WSAStartup(MAKEWORD(1, 1), &wsa) == 0);

        if (g_shutdownCb) {
            g_shutdownCb();
            g_shutdownCb = NULL;
        }
        F_DbgSetWireSink(NULL);
        if (g_pClientLink) {
            delete g_pClientLink;
            g_pClientLink = NULL;
        }
        if (wsaOk) WSACleanup();
    }
    return TRUE;
}

#endif
