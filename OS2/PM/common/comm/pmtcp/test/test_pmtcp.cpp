/*! @file test_pmtcp.cpp
    @brief Pure PMTCP transport tests.

    Tests only the transport layer:
    - TCPipe class (socket primitives)
    - Fc_Trn* client wrappers (Post/Get/Send/Disconnect)
    - Fs_Trn* server wrappers (GetCommand/GetData/PostData)
    - Wire framing (8-byte cmd header)
    - Handshake (pIpEtEsT / PiPeTeSt)

    Does NOT test:
    - PMCLIENT: no Pmc* calls, no F_CMD_CLIENT_*, no session state
    - PMSERVER: no F_CMD_GPI/WIN interpretation, no dispatch
    - Wire protocol semantics (which command does what)

    Approach:
    - TCPipe round-trips use a pair on TEST_PORT (9877, not 9876).
    - Fc_Trn* / Fs_Trn* take a TCPipe* as "obj" directly, so they
      can be exercised without Fc_TrnConnect / Fs_TrnRunListener
      (which bind 9876 and block).
    - Handshake edge cases use raw sockets on the non-TCPipe side.

    Run under wall-clock timeout:  timeout 30 ./test_pmtcp
    A hang indicates a transport bug (Recv blocked on closed peer).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#include "F_platform.h"
#include "F_tcp.hpp"
#include "F_debug.h"

/* ---- Externals from Ft_main.cpp (C-linkage) ---- */

/*! @brief Close one specific client connection.
    @param [in] obj  Connection handle from Fc_TrnConnect.
    @return 0. */
extern "C" APIRET EXPENTRY Fc_TrnDisconnect(void *obj);
/*! @brief Client-side async write of a command (no wait).
    @return 0 on success, non-zero on transport error. */
extern "C" APIRET EXPENTRY Fc_TrnPostCommand(void *obj, int cmd, int par);
/*! @brief Client-side async write of raw data (no wait).
    @return 0 on success, non-zero on transport error. */
extern "C" APIRET EXPENTRY Fc_TrnPostData(void *obj, void *buf, int len);
/*! @brief Client-side blocking read of raw data.
    @return 0 on success, -1 on failure. */
extern "C" APIRET EXPENTRY Fc_TrnGetData(void *obj, void *buf, int *len, int size);
/*! @brief Client-side send command, wait for reply.
    @return Server reply code, or (APIRET)-1 on transport error. */
extern "C" APIRET EXPENTRY Fc_TrnSendCommand(void *obj, int cmd, int par);
/*! @brief Client-side send command with payload, wait for reply.
    @return Server reply code, or (APIRET)-1 on transport error. */
extern "C" APIRET EXPENTRY Fc_TrnSendCommandData(void *obj, int cmd, int par,
                                                 void *buf, int len);
/*! @brief Server-side blocking read of (cmd, par).
    @return 0 on success, -1 on failure. */
extern "C" APIRET EXPENTRY Fs_TrnGetCommand(void *obj, int *cmd, int *par);
/*! @brief Server-side blocking read of raw data.
    @return 0 on success, -1 on failure. */
extern "C" APIRET EXPENTRY Fs_TrnGetData(void *obj, void *buf, int *len, int size);
/*! @brief Server-side async write of raw data (no wait).
    @return 0 on success, non-zero on transport error. */
extern "C" APIRET EXPENTRY Fs_TrnPostData(void *obj, void *buf, int len);

/* ---- Debug stubs (test is self-contained, no PMDEBUG link) ---- */

/*! @brief Stub: emit diagnostic message. Does nothing in this test. */
extern "C" void APIENTRY F_DbgPrint(int s, int l, const char *f, ...)
{ (void)s; (void)l; (void)f; }
/*! @brief Stub: enter debug context. Returns -1 (filtered). */
extern "C" int  APIENTRY F_DbgCtxEnterf(int s, int l, const char *f, ...)
{ (void)s; (void)l; (void)f; return -1; }
/*! @brief Stub: exit debug context. Does nothing in this test. */
extern "C" void APIENTRY F_DbgCtxExitf(int c, const char *f, ...)
{ (void)c; (void)f; }

/*! @def TEST_PORT
    @brief Port for loopback test pairs. Distinct from FREEPM_TCP_DEFAULT_PORT. */
#define TEST_PORT 9877

/*! @brief Global failure counter; drives verdict. */
static int g_fail  = 0;
/*! @brief Global test counter; drives progress line. */
static int g_total = 0;

/*! @brief Record a test outcome.
    @param [in] ok    TRUE if the test passed.
    @param [in] what  Human-readable description. */
static void check(int ok, const char *what)
{
    g_total++;
    printf("PMTCP_TEST: [%s] %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok) g_fail++;
}

/* ======================================================================== */
/*  Threading shim                                                         */
/* ======================================================================== */

#if defined(__OS2__) || defined(__OS2V2__)
  /*! @def TBEGIN
      @brief Start a thread (OS/2: start, stack, size, arg). */
  #define TBEGIN(fn, arg) _beginthread((void(*)(void*))(fn), NULL, 65536, (void*)(arg))
#else
  /*! @def TBEGIN
      @brief Start a thread (Win32: start, size, arg). */
  #define TBEGIN(fn, arg) _beginthread((void(*)(void*))(fn), 65536, (void*)(arg))
#endif

/* ======================================================================== */
/*  Pair fixture: two connected TCPipes on TEST_PORT (no handshake yet)    */
/* ======================================================================== */

/*! @brief Args for pair_client_thread. */
struct PairArgs {
    TCPipe *pClient;  /*!< Receives the connected client. */
    int     port;     /*!< Server port (TEST_PORT).       */
    int     fail;     /*!< 0 = ok, 1 = alloc fail, 2 = connect fail. */
};

#if defined(__OS2__) || defined(__OS2V2__)
/*! @brief OS/2 thread: connect one client to the pair server. */
static void pair_client_thread(void *p) {
    PairArgs *a = (PairArgs *)p;
    a->pClient = new TCPipe("127.0.0.1", a->port, TCP_CLIENT_MODE);
    if (!a->pClient) { a->fail = 1; return; }
    if (a->pClient->Connect() != 0) a->fail = 2;
}
#else
/*! @brief Win32 thread: connect one client to the pair server. */
static unsigned long __stdcall pair_client_thread(void *p) {
    PairArgs *a = (PairArgs *)p;
    a->pClient = new TCPipe("127.0.0.1", a->port, TCP_CLIENT_MODE);
    if (!a->pClient) { a->fail = 1; return 0; }
    if (a->pClient->Connect() != 0) a->fail = 2;
    return 0;
}
#endif

/*! @brief Create a connected TCPipe pair on TEST_PORT.
    @param [out] pServer    Receives the server-side TCPipe.
    @param [out] pClient    Receives the client-side TCPipe.
    @param [out] pListener  Receives the listener (must be closed last).
    @return 0 on success, -1 on failure. */
static int OpenPair(TCPipe **pServer, TCPipe **pClient, TCPipe **pListener)
{
    TCPipe   *listener = new TCPipe();
    PairArgs  a = { NULL, TEST_PORT, 0 };
    TCPipe   *server;
    int       waited = 0;

    if (!listener) return -1;
    listener->mode = TCP_SERVER_MODE;
    listener->port = TEST_PORT;
    if (listener->Create() != 0) { delete listener; return -1; }

    TBEGIN(pair_client_thread, &a);

    server = new TCPipe();
    if (!server || listener->Accept(server) != 0) {
        if (server) delete server;
        delete listener;
        return -1;
    }

    while (a.pClient == NULL && a.fail == 0 && waited < 5000) {
        DosSleep(10);
        waited += 10;
    }
    if (!a.pClient || a.fail) {
        delete server; delete listener;
        return -1;
    }

    *pServer   = server;
    *pClient   = a.pClient;
    *pListener = listener;
    return 0;
}

/*! @brief Tear down a connected TCPipe pair.
    @param [in] srv  Server-side TCPipe.
    @param [in] cli  Client-side TCPipe.
    @param [in] lst  Listener. */
static void ClosePair(TCPipe *srv, TCPipe *cli, TCPipe *lst)
{
    if (srv) { srv->Close(); delete srv; }
    if (cli) { cli->Close(); delete cli; }
    if (lst) { lst->Close(); delete lst; }
}

/* ======================================================================== */
/*  Handshake fixture: run HandShake on both sides simultaneously          */
/* ======================================================================== */

/*! @brief Args for hs_thread. */
struct HSArgs {
    TCPipe *p;  /*!< TCPipe whose HandShake() to call. */
    int     rc; /*!< HandShake return value. -1 = not yet run. */
};

#if defined(__OS2__) || defined(__OS2V2__)
/*! @brief OS/2 thread: run HandShake on one side. */
static void hs_thread(void *p) { HSArgs *a = (HSArgs*)p; a->rc = a->p->HandShake(); }
#else
/*! @brief Win32 thread: run HandShake on one side. */
static unsigned long __stdcall hs_thread(void *p)
{ HSArgs *a = (HSArgs*)p; a->rc = a->p->HandShake(); return 0; }
#endif

/* ======================================================================== */
/*  Raw socket helpers (for wire-level handshake tests)                    */
/* ======================================================================== */

#if defined(__OS2__) || defined(__OS2V2__)
  /*! @brief Raw socket type (OS/2). */
  typedef int RawSock;
  /*! @def RawClose
      @brief Close a raw socket (OS/2). */
  #define RawClose(s)  soclose(s)
  /*! @def RawInvalid
      @brief Invalid raw socket value (OS/2). */
  #define RawInvalid   (-1)
#else
  /*! @brief Raw socket type (Win32). */
  typedef SOCKET RawSock;
  /*! @def RawClose
      @brief Close a raw socket (Win32). */
  #define RawClose(s)  closesocket(s)
  /*! @def RawInvalid
      @brief Invalid raw socket value (Win32). */
  #define RawInvalid   INVALID_SOCKET
#endif

/*! @brief Connect a raw socket to 127.0.0.1:port.
    @param [in] port  Server port.
    @return Connected socket, or RawInvalid. */
static RawSock RawConnect(int port)
{
    struct sockaddr_in sa;
    RawSock s;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == RawInvalid) return RawInvalid;

    memset(&sa, 0, sizeof(sa));
    sa.sin_family      = AF_INET;
    sa.sin_port        = htons((unsigned short)port);
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) != 0) {
        RawClose(s); return RawInvalid;
    }
    return s;
}

/*! @brief Send exactly N bytes over a raw socket.
    @return 0 on success, -1 on failure. */
static int RawSendAll(RawSock s, const void *pv, int n)
{
    const char *p = (const char *)pv;
    int sent = 0;
    while (sent < n) {
        int w = send(s, p + sent, n - sent, 0);
        if (w <= 0) return -1;
        sent += w;
    }
    return 0;
}

/*! @brief Receive exactly N bytes from a raw socket.
    @return 0 on success, -1 on failure. */
static int RawRecvAll(RawSock s, void *pv, int n)
{
    char *p = (char *)pv;
    int got = 0;
    while (got < n) {
        int r = recv(s, p + got, n - got, 0);
        if (r <= 0) return -1;
        got += r;
    }
    return 0;
}

/*! @brief Args for raw_client_thread. */
struct RawClientArgs {
    int  port;         /*!< Server port.                  */
    char magic[16];    /*!< Bytes to send (NUL-padded).   */
    int  cbMagic;      /*!< Number of bytes to send.      */
    int  fail;         /*!< 0 = ok, 1 = connect fail, 2 = send fail. */
};

#if defined(__OS2__) || defined(__OS2V2__)
/*! @brief OS/2 thread: raw client that sends magic bytes and closes. */
static void raw_client_thread(void *p) {
    RawClientArgs *a = (RawClientArgs *)p;
    RawSock s = RawConnect(a->port);
    if (s == RawInvalid) { a->fail = 1; return; }
    if (RawSendAll(s, a->magic, a->cbMagic) != 0) { a->fail = 2; RawClose(s); return; }
    RawClose(s);
}
#else
/*! @brief Win32 thread: raw client that sends magic bytes and closes. */
static unsigned long __stdcall raw_client_thread(void *p) {
    RawClientArgs *a = (RawClientArgs *)p;
    RawSock s = RawConnect(a->port);
    if (s == RawInvalid) { a->fail = 1; return 0; }
    if (RawSendAll(s, a->magic, a->cbMagic) != 0) { a->fail = 2; RawClose(s); return 0; }
    RawClose(s);
    return 0;
}
#endif

/* ======================================================================== */
/*  Group A: TCPipe unit tests (no network)                                */
/* ======================================================================== */

/*! @brief TCPipe default ctor: sock/connected/mode/host/port. */
static void test_ctor_default(void)
{
    TCPipe p;
    check(p.sock == PMC_INVALID_SOCK, "TCPipe(): sock == INVALID");
    check(p.connected == 0,           "TCPipe(): connected == 0");
    check(p.mode == TCP_CLIENT_MODE,  "TCPipe(): mode == CLIENT");
    check(strcmp(p.host, FREEPM_TCP_DEFAULT_HOST) == 0, "TCPipe(): default host");
    check(p.port == FREEPM_TCP_DEFAULT_PORT,           "TCPipe(): default port");
}

/*! @brief TCPipe parametrized ctor: host/port/mode, sock still invalid. */
static void test_ctor_param(void)
{
    TCPipe p("192.0.2.1", 12345, TCP_SERVER_MODE);
    check(strcmp(p.host, "192.0.2.1") == 0, "TCPipe(host,port,mode): host");
    check(p.port == 12345,                  "TCPipe(host,port,mode): port");
    check(p.mode == TCP_SERVER_MODE,        "TCPipe(host,port,mode): mode");
    check(p.sock == PMC_INVALID_SOCK,       "TCPipe(host,port,mode): sock INVALID");
}

/*! @brief Send on unconnected TCPipe returns -1. */
static void test_send_unconnected(void)
{
    TCPipe p;
    char buf[16] = {0};
    check(p.Send(buf, 16) == -1, "Send on unconnected -> -1");
    check(p.Send(buf, 0) == -1,  "Send(0) on unconnected -> -1");
}

/*! @brief Recv on unconnected TCPipe returns -1. */
static void test_recv_unconnected(void)
{
    TCPipe p;
    char buf[16];
    int  len = 0;
    check(p.Recv(buf, &len, 16) == -1, "Recv on unconnected -> -1");
}

/*! @brief Close is idempotent: repeated calls all return 0. */
static void test_close_idempotent(void)
{
    TCPipe p;
    check(p.Close() == 0, "Close on unconnected -> 0");
    check(p.Close() == 0, "Close twice -> 0");
    check(p.Close() == 0, "Close thrice -> 0");
}

/*! @brief Create on client-mode TCPipe returns -1. */
static void test_create_wrong_mode(void)
{
    TCPipe c("127.0.0.1", TEST_PORT, TCP_CLIENT_MODE);
    check(c.Create() == -1, "Create on client mode -> -1");
}

/*! @brief Connect on server-mode TCPipe returns -1. */
static void test_connect_wrong_mode(void)
{
    TCPipe s("127.0.0.1", TEST_PORT, TCP_SERVER_MODE);
    check(s.Connect() == -1, "Connect on server mode -> -1");
}

/*! @brief Accept on unconnected TCPipe returns -1. */
static void test_accept_unconnected(void)
{
    TCPipe p, peer;
    check(p.Accept(&peer) == -1, "Accept on unconnected -> -1");
}

/* ======================================================================== */
/*  Group B: TCPipe round-trip (no handshake)                              */
/* ======================================================================== */

/*! @brief Small (16-byte) round-trip over loopback pair. */
static void test_pair_roundtrip_small(void)
{
    TCPipe *srv, *cli, *lst;
    char send_buf[16] = "hello, world";
    char recv_buf[16] = {0};
    int  len = 0;

    if (OpenPair(&srv, &cli, &lst) != 0) { check(0, "small roundtrip: open"); return; }

    check(cli->Send(send_buf, 16) == 0,              "client Send(16)");
    check(srv->Recv(recv_buf, &len, 16) == 0,        "server Recv(16)");
    check(len == 16,                                  "server Recv len == 16");
    check(memcmp(send_buf, recv_buf, 16) == 0,        "data match 16B");

    ClosePair(srv, cli, lst);
}

/*! @brief Large (64 KiB) round-trip over loopback pair. */
static void test_pair_roundtrip_large(void)
{
    TCPipe *srv, *cli, *lst;
    const int N = 65536;
    char *send_buf = (char *)malloc(N);
    char *recv_buf = (char *)malloc(N);
    int  len = 0, i, ok;

    if (!send_buf || !recv_buf) { free(send_buf); free(recv_buf);
        check(0, "large roundtrip: alloc"); return; }
    for (i = 0; i < N; i++) send_buf[i] = (char)(i & 0xFF);
    memset(recv_buf, 0, N);

    if (OpenPair(&srv, &cli, &lst) != 0) { free(send_buf); free(recv_buf);
        check(0, "large roundtrip: open"); return; }

    check(cli->Send(send_buf, N) == 0,        "client Send(64K)");
    check(srv->Recv(recv_buf, &len, N) == 0,  "server Recv(64K)");
    check(len == N,                           "server Recv len == 64K");
    ok = (memcmp(send_buf, recv_buf, N) == 0);
    check(ok,                                 "data match 64K");

    ClosePair(srv, cli, lst);
    free(send_buf); free(recv_buf);
}

/*! @brief Bidirectional exchange: A->B then B->A. */
static void test_pair_bidirectional(void)
{
    TCPipe *srv, *cli, *lst;
    char a[8] = "AAAAaaa", b[8] = "BBBBbbb";
    char ra[8] = {0}, rb[8] = {0};
    int  len = 0;

    if (OpenPair(&srv, &cli, &lst) != 0) { check(0, "bidir: open"); return; }

    check(cli->Send(a, 8) == 0,        "client Send A");
    check(srv->Recv(ra, &len, 8) == 0, "server Recv A");
    check(memcmp(a, ra, 8) == 0,       "data A->B");

    check(srv->Send(b, 8) == 0,        "server Send B");
    check(cli->Recv(rb, &len, 8) == 0, "client Recv B");
    check(memcmp(b, rb, 8) == 0,       "data B->A");

    ClosePair(srv, cli, lst);
}

/*! @brief Send(0) returns 0 and transfers nothing. */
static void test_pair_send_zero(void)
{
    TCPipe *srv, *cli, *lst;
    char buf[1] = {0};

    if (OpenPair(&srv, &cli, &lst) != 0) { check(0, "send 0: open"); return; }
    check(cli->Send(buf, 0) == 0, "Send(0) returns 0");
    ClosePair(srv, cli, lst);
}

/*! @brief Two Recv(4) calls split an 8-byte Send correctly. */
static void test_pair_split_recv(void)
{
    TCPipe *srv, *cli, *lst;
    char send_buf[9] = "ABCDEFGH";
    char recv_buf[8] = {0};
    int  len = 0;

    if (OpenPair(&srv, &cli, &lst) != 0) { check(0, "split recv: open"); return; }

    check(cli->Send(send_buf, 8) == 0,              "client Send(8)");
    check(srv->Recv(recv_buf, &len, 4) == 0,        "server Recv(4) first");
    check(len == 4,                                  "first recv len == 4");
    check(memcmp(recv_buf, "ABCD", 4) == 0,          "first 4 bytes match");
    check(srv->Recv(recv_buf + 4, &len, 4) == 0,    "server Recv(4) second");
    check(len == 4,                                  "second recv len == 4");
    check(memcmp(recv_buf, "ABCDEFGH", 8) == 0,      "combined match");

    ClosePair(srv, cli, lst);
}

/*! @brief Send after Close returns -1 and clears connected. */
static void test_pair_send_after_close(void)
{
    TCPipe *srv, *cli, *lst;
    char buf[16] = {0};

    if (OpenPair(&srv, &cli, &lst) != 0) { check(0, "send after close: open"); return; }
    cli->Close();
    check(cli->Send(buf, 16) == -1, "Send after Close -> -1");
    check(cli->connected == 0,      "connected == 0 after Close");
    ClosePair(srv, cli, lst);
}

/*! @brief Recv after peer Close returns -1 with len == 0. */
static void test_pair_recv_after_peer_close(void)
{
    TCPipe *srv, *cli, *lst;
    char recv_buf[16];
    int  len = 0;

    if (OpenPair(&srv, &cli, &lst) != 0) { check(0, "recv after close: open"); return; }
    cli->Close();
    DosSleep(200);
    check(srv->Recv(recv_buf, &len, 16) == -1, "Recv after peer Close -> -1");
    check(len == 0,                            "len == 0 after clean close");
    ClosePair(srv, cli, lst);
}

/*! @brief Недосыл: peer sends 4 of 16 then closes.
    Recv must return -1 with len == 4 and partial data intact. */
static void test_pair_partial_then_close(void)
{
    TCPipe *srv, *cli, *lst;
    char recv_buf[16] = {0};
    int  len = 0;

    if (OpenPair(&srv, &cli, &lst) != 0) { check(0, "partial+close: open"); return; }
    check(cli->Send("ABCD", 4) == 0, "client Send(4) partial");
    cli->Close();
    DosSleep(200);
    check(srv->Recv(recv_buf, &len, 16) == -1, "Recv after partial+close -> -1");
    check(len == 4,                             "len == 4 (partial delivered)");
    check(memcmp(recv_buf, "ABCD", 4) == 0,     "partial data intact");
    ClosePair(srv, cli, lst);
}

/* ======================================================================== */
/*  Group C: Handshake                                                     */
/* ======================================================================== */

/*! @brief Happy-path handshake on a loopback pair. */
static void test_handshake_ok(void)
{
    TCPipe *srv, *cli, *lst;
    HSArgs  ca;
    int     src, waited;

    if (OpenPair(&srv, &cli, &lst) != 0) { check(0, "hs ok: open"); return; }

    ca.p = cli; ca.rc = -1;
    TBEGIN(hs_thread, &ca);
    src = srv->HandShake();

    waited = 0;
    while (ca.rc == -1 && waited < 3000) { DosSleep(10); waited += 10; }

    check(src == 0,    "server handshake OK");
    check(ca.rc == 0,  "client handshake OK");

    ClosePair(srv, cli, lst);
}

/*! @brief Handshake rejects wrong magic from peer. */
static void test_handshake_server_wrong_magic(void)
{
    TCPipe         *listener = new TCPipe();
    TCPipe         *server;
    RawClientArgs   ra;
    int             rc;

    listener->mode = TCP_SERVER_MODE;
    listener->port = TEST_PORT;
    if (listener->Create() != 0) { delete listener; check(0, "hs wrong magic: create"); return; }

    memset(ra.magic, 0, sizeof(ra.magic));
    strncpy(ra.magic, "WRONGMAGIC", sizeof(ra.magic) - 1);
    ra.port    = TEST_PORT;
    ra.cbMagic = 9;
    ra.fail    = 0;
    TBEGIN(raw_client_thread, &ra);

    server = new TCPipe();
    if (listener->Accept(server) != 0) {
        delete server; listener->Close(); delete listener;
        check(0, "hs wrong magic: accept"); return;
    }
    rc = server->HandShake();
    check(rc == HAND_SHAKE_ERROR, "server rejects wrong magic");

    DosSleep(100);
    delete server; listener->Close(); delete listener;
}

/*! @brief Handshake consumes exactly 9 bytes; extra bytes remain readable. */
static void test_handshake_overread(void)
{
    TCPipe         *listener = new TCPipe();
    TCPipe         *server;
    RawSock         rs;
    char            combined[17];
    char            recv_extra[8];
    int             len;

    listener->mode = TCP_SERVER_MODE;
    listener->port = TEST_PORT;
    if (listener->Create() != 0) { delete listener; check(0, "hs overread: create"); return; }

    rs = RawConnect(TEST_PORT);
    if (rs == RawInvalid) { listener->Close(); delete listener;
        check(0, "hs overread: connect"); return; }

    memcpy(combined, "pIpEtEsT\0", 9);
    memcpy(combined + 9, "EXTRADAT", 8);
    if (RawSendAll(rs, combined, 17) != 0) {
        RawClose(rs); listener->Close(); delete listener;
        check(0, "hs overread: send"); return;
    }

    server = new TCPipe();
    if (listener->Accept(server) != 0) {
        RawClose(rs); delete server; listener->Close(); delete listener;
        check(0, "hs overread: accept"); return;
    }
    check(server->HandShake() == 0, "hs overread: server handshake OK");

    /* The 8 extra bytes must still be in the socket buffer. */
    len = 0;
    check(server->Recv(recv_extra, &len, 8) == 0,
          "hs overread: 8 extra bytes available");
    check(len == 8 && memcmp(recv_extra, "EXTRADAT", 8) == 0,
          "hs overread: extra bytes intact");

    RawClose(rs);
    delete server; listener->Close(); delete listener;
}

/* ======================================================================== */
/*  Group D: Fc_Trn* client wrappers                                       */
/* ======================================================================== */

/*! @brief Fc_TrnPostCommand -> Fs_TrnGetCommand preserves cmd/par. */
static void test_fc_post_command(void)
{
    TCPipe *srv, *cli, *lst;
    int     cmd = 0, par = 0;

    if (OpenPair(&srv, &cli, &lst) != 0) { check(0, "fc_postcommand: open"); return; }

    check(Fc_TrnPostCommand(cli, 0x1234, 0x5678) == 0, "Fc_TrnPostCommand -> 0");
    check(Fs_TrnGetCommand(srv, &cmd, &par) == 0,       "Fs_TrnGetCommand OK");
    check(cmd == 0x1234,                                 "cmd matches");
    check(par == 0x5678,                                 "par matches");

    ClosePair(srv, cli, lst);
}

/*! @brief Fc_TrnPostData -> Fs_TrnGetData preserves payload. */
static void test_fc_post_data(void)
{
    TCPipe *srv, *cli, *lst;
    char    payload[16] = "test-payload-42";
    char    got[16] = {0};
    int     len = 0;

    if (OpenPair(&srv, &cli, &lst) != 0) { check(0, "fc_postdata: open"); return; }

    check(Fc_TrnPostData(cli, payload, 16) == 0,     "Fc_TrnPostData -> 0");
    check(Fs_TrnGetData(srv, got, &len, 16) == 0,    "Fs_TrnGetData OK");
    check(len == 16 && memcmp(got, payload, 16) == 0, "payload matches");

    ClosePair(srv, cli, lst);
}

/*! @brief Fs_TrnPostData -> Fc_TrnGetData preserves payload. */
static void test_fc_get_data(void)
{
    TCPipe *srv, *cli, *lst;
    char    payload[8] = "REPLYyy";
    char    got[8] = {0};
    int     len = 0;

    if (OpenPair(&srv, &cli, &lst) != 0) { check(0, "fc_getdata: open"); return; }

    check(Fs_TrnPostData(srv, payload, 8) == 0,     "server Fs_TrnPostData");
    check(Fc_TrnGetData(cli, got, &len, 8) == 0,    "Fc_TrnGetData OK");
    check(len == 8 && memcmp(got, payload, 8) == 0, "reply matches");

    ClosePair(srv, cli, lst);
}

/*! @brief Compound PostCommand+GetCommand+PostData+GetData round-trip.
    Equivalent to Fc_TrnSendCommand but split into explicit steps. */
static void test_fc_send_command_roundtrip(void)
{
    TCPipe *srv, *cli, *lst;
    int     rc = 0, retcode = 0xCAFEBABE;
    int     cmd = 0, par = 0;
    int     rl = 0;

    if (OpenPair(&srv, &cli, &lst) != 0) { check(0, "fc_sendcommand: open"); return; }

    check(Fc_TrnPostCommand(cli, 0xABCD, 0xEF01) == 0, "compound: PostCommand");
    check(Fs_TrnGetCommand(srv, &cmd, &par) == 0,       "compound: GetCommand");
    check(cmd == 0xABCD && par == 0xEF01,                "compound: cmd/par");
    check(Fs_TrnPostData(srv, &retcode, 4) == 0,        "compound: PostData reply");
    check(Fc_TrnGetData(cli, &rc, &rl, 4) == 0,         "compound: GetData reply");
    check(rc == retcode,                                  "compound: retcode matches");

    ClosePair(srv, cli, lst);
}

/*! @brief All Fc_Trn* reject NULL obj. */
static void test_fc_null_obj(void)
{
    char buf[8] = {0};
    int  len = 0;

    check(Fc_TrnPostCommand(NULL, 1, 2) != 0,        "Fc_TrnPostCommand(NULL) fails");
    check(Fc_TrnPostData(NULL, buf, 8) != 0,         "Fc_TrnPostData(NULL) fails");
    check(Fc_TrnGetData(NULL, buf, &len, 8) == -1,   "Fc_TrnGetData(NULL) -> -1");
    check(Fc_TrnSendCommand(NULL, 1, 2) == (APIRET)-1, "Fc_TrnSendCommand(NULL) -> -1");
    check(Fc_TrnSendCommandData(NULL, 1, 2, buf, 8) == (APIRET)-1,
                                                      "Fc_TrnSendCommandData(NULL) -> -1");
}

/*! @brief Fc_TrnDisconnect(NULL) is a no-op returning 0. */
static void test_fc_disconnect_null(void)
{
    check(Fc_TrnDisconnect(NULL) == 0, "Fc_TrnDisconnect(NULL) -> 0 (no-op)");
}

/* ======================================================================== */
/*  Group E: Fs_Trn* server wrappers                                       */
/* ======================================================================== */

/*! @brief Raw client sends 8 bytes; Fs_TrnGetCommand decodes correctly. */
static void test_fs_get_command_from_raw(void)
{
    TCPipe         *listener = new TCPipe();
    TCPipe         *server;
    RawSock         rs;
    ULONG           hdr[2] = { 0x11111111, 0x22222222 };
    int             cmd = 0, par = 0;

    listener->mode = TCP_SERVER_MODE;
    listener->port = TEST_PORT;
    if (listener->Create() != 0) { delete listener; check(0, "fs_getcmd: create"); return; }

    rs = RawConnect(TEST_PORT);
    if (rs == RawInvalid) { listener->Close(); delete listener;
        check(0, "fs_getcmd: connect"); return; }
    RawSendAll(rs, hdr, 8);

    server = new TCPipe();
    listener->Accept(server);
    check(Fs_TrnGetCommand(server, &cmd, &par) == 0, "Fs_TrnGetCommand OK");
    check((ULONG)cmd == 0x11111111,                   "cmd matches raw");
    check((ULONG)par == 0x22222222,                   "par matches raw");

    RawClose(rs); delete server; listener->Close(); delete listener;
}

/*! @brief Fs_TrnPostData writes to a raw client socket. */
static void test_fs_post_data_to_raw(void)
{
    TCPipe         *listener = new TCPipe();
    TCPipe         *server;
    RawSock         rs;
    char            payload[9] = "SRVPOST0";
    char            got[8] = {0};

    listener->mode = TCP_SERVER_MODE;
    listener->port = TEST_PORT;
    if (listener->Create() != 0) { delete listener; check(0, "fs_postdata: create"); return; }

    rs = RawConnect(TEST_PORT);
    if (rs == RawInvalid) { listener->Close(); delete listener;
        check(0, "fs_postdata: connect"); return; }
    server = new TCPipe();
    listener->Accept(server);

    check(Fs_TrnPostData(server, payload, 8) == 0, "Fs_TrnPostData -> 0");
    check(RawRecvAll(rs, got, 8) == 0,             "raw recv 8");
    check(memcmp(got, payload, 8) == 0,            "payload matches");

    RawClose(rs); delete server; listener->Close(); delete listener;
}

/*! @brief All Fs_Trn* reject NULL obj. */
static void test_fs_null_obj(void)
{
    char buf[8] = {0};
    int  len = 0;
    int  cmd = 0, par = 0;

    check(Fs_TrnGetCommand(NULL, &cmd, &par) == -1,  "Fs_TrnGetCommand(NULL) -> -1");
    check(Fs_TrnGetData(NULL, buf, &len, 8) == -1,   "Fs_TrnGetData(NULL) -> -1");
    check(Fs_TrnPostData(NULL, buf, 8) != 0,         "Fs_TrnPostData(NULL) fails");
}

/* ======================================================================== */
/*  Group F: Partial-frame handling (недосыл / недоприем)                  */
/* ======================================================================== */

/*! @brief Недосыл: 4 of 8 header bytes then close.
    Fs_TrnGetCommand must return -1, not hang. */
static void test_partial_header_then_close(void)
{
    TCPipe         *listener = new TCPipe();
    TCPipe         *server;
    RawSock         rs;
    int             cmd = 0, par = 0;

    listener->mode = TCP_SERVER_MODE;
    listener->port = TEST_PORT;
    if (listener->Create() != 0) { delete listener; check(0, "partial hdr: create"); return; }

    rs = RawConnect(TEST_PORT);
    if (rs == RawInvalid) { listener->Close(); delete listener;
        check(0, "partial hdr: connect"); return; }
    RawSendAll(rs, "HALF", 4);
    RawClose(rs);

    server = new TCPipe();
    listener->Accept(server);
    DosSleep(200);
    check(Fs_TrnGetCommand(server, &cmd, &par) == -1,
          "Fs_TrnGetCommand after partial header + close -> -1");

    delete server; listener->Close(); delete listener;
}

/*! @brief Недоприем: 8-byte header + 50 of 100 data bytes, then close.
    Fs_TrnGetData(100) must return -1 with len == 50. */
static void test_partial_data_then_close(void)
{
    TCPipe         *listener = new TCPipe();
    TCPipe         *server;
    RawSock         rs;
    char            buf[100];
    int             len = 0;

    listener->mode = TCP_SERVER_MODE;
    listener->port = TEST_PORT;
    if (listener->Create() != 0) { delete listener; check(0, "partial data: create"); return; }

    rs = RawConnect(TEST_PORT);
    if (rs == RawInvalid) { listener->Close(); delete listener;
        check(0, "partial data: connect"); return; }

    {
        ULONG hdr[2] = { 0xDEADBEEF, 0x00000000 };
        RawSendAll(rs, hdr, 8);
        memset(buf, 'X', 50);
        RawSendAll(rs, buf, 50);
    }
    RawClose(rs);

    server = new TCPipe();
    listener->Accept(server);
    DosSleep(200);
    memset(buf, 0, sizeof(buf));
    check(Fs_TrnGetData(server, buf, &len, 100) == -1,
          "Fs_TrnGetData after partial data + close -> -1");
    check(len == 50, "len == 50 (partial delivered)");

    delete server; listener->Close(); delete listener;
}

/*! @brief Fc_TrnGetData after server-side close returns -1. */
static void test_fc_getdata_after_peer_close(void)
{
    TCPipe *srv, *cli, *lst;
    char    buf[8] = {0};
    int     len = 0;

    if (OpenPair(&srv, &cli, &lst) != 0) { check(0, "fc_getdata close: open"); return; }
    srv->Close();
    DosSleep(200);
    check(Fc_TrnGetData(cli, buf, &len, 8) == -1,
          "Fc_TrnGetData after server close -> -1");
    ClosePair(srv, cli, lst);
}

/* ======================================================================== */
/*  Group G: Multi-connection stress                                       */
/* ======================================================================== */

/*! @brief Args for stress_client_thread. */
struct StressArgs {
    int port;  /*!< Server port. */
    int ok;    /*!< 1 on success, 0 on failure. */
};

#if defined(__OS2__) || defined(__OS2V2__)
/*! @brief OS/2 thread: one-shot stress client (connect, send, close). */
static void stress_client_thread(void *p) {
    StressArgs *a = (StressArgs *)p;
    TCPipe *c = new TCPipe("127.0.0.1", a->port, TCP_CLIENT_MODE);
    if (!c) { a->ok = 0; return; }
    if (c->Connect() != 0) { delete c; a->ok = 0; return; }
    if (c->Send("PING", 4) != 0) { c->Close(); delete c; a->ok = 0; return; }
    c->Close();
    delete c;
    a->ok = 1;
}
#else
/*! @brief Win32 thread: one-shot stress client (connect, send, close). */
static unsigned long __stdcall stress_client_thread(void *p) {
    StressArgs *a = (StressArgs *)p;
    TCPipe *c = new TCPipe("127.0.0.1", a->port, TCP_CLIENT_MODE);
    if (!c) { a->ok = 0; return 0; }
    if (c->Connect() != 0) { delete c; a->ok = 0; return 0; }
    if (c->Send("PING", 4) != 0) { c->Close(); delete c; a->ok = 0; return 0; }
    c->Close();
    delete c;
    a->ok = 1;
    return 0;
}
#endif

/*! @brief 5 simultaneous connections: all accepted, all clients complete. */
static void test_multi_connection(void)
{
    TCPipe     *listener = new TCPipe();
    StressArgs  sa[5];
    int         i, accepted = 0;

    listener->mode = TCP_SERVER_MODE;
    listener->port = TEST_PORT;
    if (listener->Create() != 0) { delete listener; check(0, "multi: create"); return; }

    for (i = 0; i < 5; i++) {
        sa[i].port = TEST_PORT;
        sa[i].ok   = 0;
        TBEGIN(stress_client_thread, &sa[i]);
    }

    for (i = 0; i < 5; i++) {
        TCPipe *s = new TCPipe();
        if (listener->Accept(s) == 0) {
            accepted++;
            char buf[4];
            int  len = 0;
            s->Recv(buf, &len, 4);
            s->Close();
            delete s;
        } else {
            delete s;
        }
    }

    DosSleep(500);
    {
        int ok = 0;
        for (i = 0; i < 5; i++) ok += sa[i].ok;
        check(accepted == 5,    "multi: 5 connections accepted");
        check(ok == 5,          "multi: 5 clients completed");
    }

    listener->Close(); delete listener;
}

/* ======================================================================== */
/*  main                                                                   */
/* ======================================================================== */

/*! @brief Run all test groups, print PASS/FAIL, return exit code. */
int main(void)
{
    printf("PMTCP_TEST: starting\n"); fflush(stdout);

    /* Group A: TCPipe unit */
    test_ctor_default();
    test_ctor_param();
    test_send_unconnected();
    test_recv_unconnected();
    test_close_idempotent();
    test_create_wrong_mode();
    test_connect_wrong_mode();
    test_accept_unconnected();

    /* Group B: TCPipe round-trip */
    test_pair_roundtrip_small();
    test_pair_roundtrip_large();
    test_pair_bidirectional();
    test_pair_send_zero();
    test_pair_split_recv();
    test_pair_send_after_close();
    test_pair_recv_after_peer_close();
    test_pair_partial_then_close();

    /* Group C: Handshake */
    test_handshake_ok();
    test_handshake_server_wrong_magic();
    test_handshake_overread();

    /* Group D: Fc_Trn* */
    test_fc_post_command();
    test_fc_post_data();
    test_fc_get_data();
    test_fc_send_command_roundtrip();
    test_fc_null_obj();
    test_fc_disconnect_null();

    /* Group E: Fs_Trn* */
    test_fs_get_command_from_raw();
    test_fs_post_data_to_raw();
    test_fs_null_obj();

    /* Group F: Partial frames */
    test_partial_header_then_close();
    test_partial_data_then_close();
    test_fc_getdata_after_peer_close();

    /* Group G: Multi-conn */
    test_multi_connection();

    if (g_fail == 0) {
        printf("PMTCP_TEST: RESULT PASS (%d/%d)\n", g_total, g_total);
        return 0;
    }
    printf("PMTCP_TEST: RESULT FAIL (%d/%d)\n", g_fail, g_total);
    return 1;
}
