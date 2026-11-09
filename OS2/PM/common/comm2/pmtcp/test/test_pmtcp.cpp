/*! @file test_pmtcp.cpp
    @brief Transport-level tests for PMTCP.DLL (IPv4).

    Tests exercise the exported Fc_Trn* / Fs_Trn* API only. Peer-close
    is tested via the wire-protocol "close" command (cmd=0); partial
    frames via raw sockets connected to the listener on TEST_PORT.

    Constraint: port 9876 is hardcoded inside PMTCP for both
    Fc_TrnConnect (default) and Fs_TrnRunListener. The test binds it as
    a listener, so PMSERVER.EXE must NOT be running.

    Run under wall-clock timeout: timeout 60 ./test_pmtcp
    A hang indicates a transport bug (Recv blocked on closed peer).
*/

#ifndef INCL_DOSERRORS
  #define INCL_DOSERRORS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

/* F_tcp.hpp brings in OS/2 socket headers on OS/2, Winsock on Win32,
   plus F_wire.hpp (11 exports) and F_link.hpp. */
#include "F_tcp.hpp"
#include "F_debug.h"

/* ======================================================================== */
/*  Constants and globals                                                  */
/* ======================================================================== */

/*! @def TEST_PORT
    @brief Server port for Fs_TrnRunListener (matches FREEPM_TCP_DEFAULT_PORT). */
#define TEST_PORT 9876

/*! @brief Global failure counter. */
static int g_fail = 0;
/*! @brief Global test counter. */
static int g_total = 0;

/*! @brief Record test outcome.
    @param [in] ok    Non-zero if passed.
    @param [in] what  Human-readable description. */
static void check(int ok, const char *what)
{
    g_total++;
    printf("PMTCP_TEST: [%s] %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok) g_fail++;
}

/* ======================================================================== */
/*  Threading and calling-convention shims                                 */
/* ======================================================================== */

#if defined(__OS2__) || defined(__OS2V2__)
  /*! @def TBEGIN
      @brief Start a thread (OS/2 4-arg _beginthread).
      @param [in] fn   Thread entry point.
      @param [in] arg  Thread argument. */
  #define TBEGIN(fn, arg) _beginthread((void(*)(void*))(fn), NULL, 65536, (void*)(arg))
#else
  /*! @def TBEGIN
      @brief Start a thread (Win32 3-arg _beginthread).
      @param [in] fn   Thread entry point.
      @param [in] arg  Thread argument. */
  #define TBEGIN(fn, arg) _beginthread((void(*)(void*))(fn), 65536, (void*)(arg))
#endif

#if defined(__OS2__) || defined(__OS2V2__)
  /*! @def TEST_CB
      @brief Calling convention for callbacks passed into PMTCP (OS/2). */
  #define TEST_CB APIENTRY
#else
  /*! @def TEST_CB
      @brief Calling convention for callbacks passed into PMTCP (Win32). */
  #define TEST_CB __stdcall
#endif

/* ======================================================================== */
/*  Raw socket helpers                                                     */
/* ======================================================================== */

#if defined(__OS2__) || defined(__OS2V2__)
  /*! @brief Raw socket type (OS/2). */
  typedef int RawSock;
  /*! @def RawClose
      @brief Close raw socket (OS/2).
      @param [in] s  Socket. */
  #define RawClose(s)  soclose(s)
  /*! @def RawInvalid
      @brief Invalid raw socket value (OS/2). */
  #define RawInvalid   (-1)
#else
  /*! @brief Raw socket type (Win32). */
  typedef SOCKET RawSock;
  /*! @def RawClose
      @brief Close raw socket (Win32).
      @param [in] s  Socket. */
  #define RawClose(s)  closesocket(s)
  /*! @def RawInvalid
      @brief Invalid raw socket value (Win32). */
  #define RawInvalid   INVALID_SOCKET
#endif

/*! @brief Send exactly N bytes over a raw socket.
    @param [in] s   Socket.
    @param [in] pv  Buffer.
    @param [in] n   Bytes to send.
    @return 0 on success, -1 on failure.
    @retval 0   All bytes sent.
    @retval -1  Send error or peer close. */
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
    @param [in]  s   Socket.
    @param [out] pv  Destination.
    @param [in]  n   Bytes to receive.
    @return 0 on success, -1 on failure.
    @retval 0   All bytes received.
    @retval -1  Recv error or peer close. */
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

/*! @brief Raw handshake on a raw socket (client side).
    @param [in] s  Socket.
    @return 0 on success, -1 on failure.
    @retval 0   Handshake completed.
    @retval -1  Transport error or wrong magic. */
static int RawHandshake(RawSock s)
{
    char hs[9];
    if (RawSendAll(s, "pIpEtEsT\0", 9) != 0) return -1;
    memset(hs, 0, sizeof(hs));
    if (RawRecvAll(s, hs, 9) != 0) return -1;
    if (memcmp(hs, "PiPeTeSt\0", 9) != 0) return -1;
    return 0;
}

/* ======================================================================== */
/*  Unified handler (used by Fs_TrnRunListener on TEST_PORT)               */
/* ======================================================================== */

/*! @brief Static 64 KB buffer to avoid 64 KB stack in server thread. */
static char g_bigbuf[65536];

/*! @brief Unified handler for all round-trip test scenarios.

    Dispatch by cmd:
    - 0x0001: echo par back as 4-byte retcode
    - 0x0002: read par bytes, echo back
    - 0x0003: send fixed 8-byte payload
    - 0x0004: read par bytes, reply 4-byte retcode
    - 0xDEAD: expect Fs_TrnGetData to fail on partial payload

    @param [in] obj        Connection object.
    @param [in] cmd        Command code.
    @param [in] par        32-bit parameter.
    @param [in] threadNum  Per-connection thread index (unused). */
static void TEST_CB unified_handler(void *obj, int cmd, int par, int threadNum)
{
    (void)threadNum;

    switch (cmd) {
    case 0x0001: {
        int retcode = par;
        Fs_TrnPostData(obj, (void *)&retcode, 4);
        break;
    }
    case 0x0002: {
        ULONG len = 0;
        if (par > 0 && (ULONG)par <= (ULONG)sizeof(g_bigbuf) &&
            Fs_TrnGetData(obj, g_bigbuf, &len, (ULONG)par) == 0) {
            Fs_TrnPostData(obj, g_bigbuf, len);
        }
        break;
    }
    case 0x0003: {
        char payload[9] = "REPLYyy";
        Fs_TrnPostData(obj, payload, 8);
        break;
    }
    case 0x0004: {
        ULONG len = 0;
        int retcode = 0xCAFEBABE;
        if (par > 0 && (ULONG)par <= (ULONG)sizeof(g_bigbuf) &&
            Fs_TrnGetData(obj, g_bigbuf, &len, (ULONG)par) == 0) {
            Fs_TrnPostData(obj, (void *)&retcode, 4);
        }
        break;
    }
    case 0xDEAD: {
        ULONG len = 0;
        char small[100];
        Fs_TrnGetData(obj, small, &len, 100);
        break;
    }
    default:
        break;
    }
}

/*! @brief Listener thread body. Does not return on success.
    @param [in] p  Unused. */
static void listener_thread(void *p)
{
    (void)p;
    Fs_TrnRunListener((void *)unified_handler, NULL);
}

/* ======================================================================== */
/*  Peer close via cmd=0 (wire-protocol "close")                           */
/* ======================================================================== */

/*! @brief Peer close: client sends cmd=0; server exits recv loop and
    closes; client's next Recv returns -1.

    Must be called AFTER the listener thread is up. */
static void test_peer_close(void)
{
    ULONG  obj = 0;
    APIRET rc;
    char   buf[4];
    ULONG  l = 0;

    if (Fc_TrnConnect("127.0.0.1", &obj) != 0) {
        check(0, "peer_close: connect"); return;
    }

    Fc_TrnPostCommand((void *)obj, 0, 0);

    rc = Fc_TrnGetData((void *)obj, buf, &l, 4);
    check(rc == (APIRET)-1, "peer close (cmd=0): GetData -> -1");

    Fc_TrnDisconnect((void *)obj);
}

/* ======================================================================== */
/*  Group A: Fc_TrnConnect — address parsing + error paths                 */
/* ======================================================================== */

/*! @brief Fc_TrnConnect(NULL obj) -> PMC_ERR_OPEN, no crash. */
static void test_connect_null_obj(void)
{
    APIRET rc = Fc_TrnConnect("127.0.0.1", NULL);
    check(rc == PMC_ERR_OPEN, "Fc_TrnConnect(NULL obj) -> PMC_ERR_OPEN");
}

/*! @brief Bad address formats rejected; obj set to 0. */
static void test_connect_bad_address(void)
{
    ULONG  obj = 0xDEADBEEF;
    APIRET rc;

    rc = Fc_TrnConnect((char *)"::1", &obj);
    check(rc != 0 && obj == 0, "IPv6 bare rejected (::1)");

    obj = 0xDEADBEEF;
    rc = Fc_TrnConnect((char *)"host:port:extra", &obj);
    check(rc != 0 && obj == 0, "Triple-colon rejected");

    obj = 0xDEADBEEF;
    rc = Fc_TrnConnect((char *)"127.0.0.1:99999", &obj);
    check(rc != 0 && obj == 0, "Port > 65535 rejected");

    obj = 0xDEADBEEF;
    rc = Fc_TrnConnect((char *)"127.0.0.1:0", &obj);
    check(rc != 0 && obj == 0, "Port 0 rejected");

    obj = 0xDEADBEEF;
    rc = Fc_TrnConnect((char *)"127.0.0.1:abc", &obj);
    check(rc != 0 && obj == 0, "Non-numeric port rejected");
}

/*! @brief Fc_TrnConnect to unused port -> PMC_ERR_OPEN, obj=0. */
static void test_connect_refused(void)
{
    ULONG  obj = 0xDEADBEEF;
    APIRET rc  = Fc_TrnConnect((char *)"127.0.0.1:65534", &obj);
    check(rc != 0 && obj == 0, "Fc_TrnConnect to closed port fails");
}

/* ======================================================================== */
/*  Group B: connect / disconnect lifecycle                                */
/* ======================================================================== */

/*! @brief Happy-path connect + disconnect. */
static void test_connect_disconnect(void)
{
    ULONG  obj = 0;
    APIRET rc  = Fc_TrnConnect("127.0.0.1", &obj);

    check(rc == 0 && obj != 0, "Fc_TrnConnect OK");
    check(Fc_TrnDisconnect((void *)obj) == 0, "Fc_TrnDisconnect OK");
}

/*! @brief Fc_TrnConnect with explicit port matches default. */
static void test_connect_explicit_port(void)
{
    ULONG  obj = 0;
    APIRET rc  = Fc_TrnConnect((char *)"127.0.0.1:9876", &obj);

    check(rc == 0 && obj != 0, "Fc_TrnConnect(host:9876) OK");
    Fc_TrnDisconnect((void *)obj);
}

/*! @brief Fc_TrnDisconnect(NULL) is a no-op returning 0. */
static void test_disconnect_null(void)
{
    check(Fc_TrnDisconnect(NULL) == 0, "Fc_TrnDisconnect(NULL) -> 0");
}

/* ======================================================================== */
/*  Group C: Fc_Trn* NULL obj / NULL buf                                   */
/* ======================================================================== */

/*! @brief All Fc_Trn* reject NULL obj. */
static void test_fc_null_obj(void)
{
    char  buf[8] = {0};
    ULONG len = 0;

    check(Fc_TrnPostCommand(NULL, 1, 2) != 0,
          "Fc_TrnPostCommand(NULL) fails");
    check(Fc_TrnPostData(NULL, buf, 8) != 0,
          "Fc_TrnPostData(NULL) fails");
    check(Fc_TrnGetData(NULL, buf, &len, 8) == (APIRET)-1,
          "Fc_TrnGetData(NULL) -> -1");
    check(Fc_TrnSendCommand(NULL, 1, 2) == (APIRET)-1,
          "Fc_TrnSendCommand(NULL) -> -1");
    check(Fc_TrnSendCommandData(NULL, 1, 2, buf, 8) == (APIRET)-1,
          "Fc_TrnSendCommandData(NULL) -> -1");
}

/*! @brief NULL buf with len > 0 is rejected; connection intact. */
static void test_fc_null_buf(void)
{
    ULONG obj = 0;
    ULONG len = 0;

    if (Fc_TrnConnect("127.0.0.1", &obj) != 0) {
        check(0, "null_buf: connect"); return;
    }
    check(Fc_TrnPostData((void *)obj, NULL, 8) != 0,
          "Fc_TrnPostData(NULL buf, len=8) fails");
    check(Fc_TrnGetData((void *)obj, NULL, &len, 8) == (APIRET)-1,
          "Fc_TrnGetData(NULL buf, size=8) -> -1");
    check(Fc_TrnSendCommand((void *)obj, 0x0001, 0x55) == 0x55,
          "connection usable after NULL buf reject");
    Fc_TrnDisconnect((void *)obj);
}

/* ======================================================================== */
/*  Group D: round-trip through listener on TEST_PORT                      */
/* ======================================================================== */

/*! @brief PostCommand + GetData: handler echoes par. */
static void test_fc_post_command(void)
{
    ULONG obj = 0;
    int   ret = 0;
    ULONG l = 0;

    if (Fc_TrnConnect("127.0.0.1", &obj) != 0) {
        check(0, "post_command: connect"); return;
    }
    check(Fc_TrnPostCommand((void *)obj, 0x0001, 0x1234) == 0,
          "Fc_TrnPostCommand -> 0");
    check(Fc_TrnGetData((void *)obj, &ret, &l, 4) == 0,
          "Fc_TrnGetData reply");
    check(ret == 0x1234, "post_command: par echoed");
    Fc_TrnDisconnect((void *)obj);
}

/*! @brief SendCommand: PostCommand + GetData in one call. */
static void test_fc_send_command(void)
{
    ULONG  obj = 0;
    APIRET ret;

    if (Fc_TrnConnect("127.0.0.1", &obj) != 0) {
        check(0, "send_command: connect"); return;
    }
    ret = Fc_TrnSendCommand((void *)obj, 0x0001, 0xABCD);
    check(ret == 0xABCD, "Fc_TrnSendCommand retcode = par");
    Fc_TrnDisconnect((void *)obj);
}

/*! @brief PostCommand + PostData + GetData (payload echo). */
static void test_fc_post_data(void)
{
    ULONG obj = 0;
    char  payload[16] = "test-payload-42";
    char  got[16] = {0};
    ULONG l = 0;

    if (Fc_TrnConnect("127.0.0.1", &obj) != 0) {
        check(0, "post_data: connect"); return;
    }
    check(Fc_TrnPostCommand((void *)obj, 0x0002, 16) == 0,
          "post_data: PostCommand");
    check(Fc_TrnPostData((void *)obj, payload, 16) == 0,
          "post_data: PostData");
    check(Fc_TrnGetData((void *)obj, got, &l, 16) == 0,
          "post_data: GetData");
    check(l == 16 && memcmp(got, payload, 16) == 0,
          "post_data: payload echoed");
    Fc_TrnDisconnect((void *)obj);
}

/*! @brief SendCommandData: PostCommand + PostData + GetData(4). */
static void test_fc_send_command_data(void)
{
    ULONG  obj = 0;
    char   payload[9] = "PAYLOAD!";
    APIRET ret;

    if (Fc_TrnConnect("127.0.0.1", &obj) != 0) {
        check(0, "send_command_data: connect"); return;
    }
    ret = Fc_TrnSendCommandData((void *)obj, 0x0004, 8, payload, 8);
    check(ret == 0xCAFEBABE, "Fc_TrnSendCommandData retcode match");
    Fc_TrnDisconnect((void *)obj);
}

/*! @brief Server sends 8-byte payload; client reads. */
static void test_fc_get_data(void)
{
    ULONG obj = 0;
    char  got[8] = {0};
    ULONG l = 0;

    if (Fc_TrnConnect("127.0.0.1", &obj) != 0) {
        check(0, "get_data: connect"); return;
    }
    check(Fc_TrnPostCommand((void *)obj, 0x0003, 0) == 0,
          "get_data: PostCommand");
    check(Fc_TrnGetData((void *)obj, got, &l, 8) == 0,
          "get_data: GetData");
    check(l == 8 && memcmp(got, "REPLYyy", 8) == 0,
          "get_data: server payload matches");
    Fc_TrnDisconnect((void *)obj);
}

/* ======================================================================== */
/*  Group E: partial frames (raw socket -> listener)                       */
/* ======================================================================== */

/*! @brief Partial header (4 of 8 bytes) then close; listener survives. */
static void test_partial_header(void)
{
    RawSock rs = RawConnect(TEST_PORT);
    ULONG   probe = 0;
    APIRET  rc;

    if (rs == RawInvalid) { check(0, "partial header: connect"); return; }
    if (RawHandshake(rs) != 0) {
        RawClose(rs);
        check(0, "partial header: raw handshake");
        return;
    }
    RawSendAll(rs, "HALF", 4);
    RawClose(rs);

    DosSleep(300);

    rc = Fc_TrnConnect("127.0.0.1", &probe);
    check(rc == 0 && probe != 0, "partial header: listener survives");
    if (rc == 0) Fc_TrnDisconnect((void *)probe);
}

/*! @brief Partial data (50 of 100) then close; listener survives. */
static void test_partial_data(void)
{
    RawSock rs = RawConnect(TEST_PORT);
    ULONG   probe = 0;
    APIRET  rc;
    int     hdr[2];
    char    payload[50];

    if (rs == RawInvalid) { check(0, "partial data: connect"); return; }
    if (RawHandshake(rs) != 0) {
        RawClose(rs);
        check(0, "partial data: raw handshake");
        return;
    }

    hdr[0] = 0xDEAD;
    hdr[1] = 0;
    memset(payload, 'X', sizeof(payload));
    RawSendAll(rs, hdr, 8);
    RawSendAll(rs, payload, sizeof(payload));
    RawClose(rs);

    DosSleep(500);

    rc = Fc_TrnConnect("127.0.0.1", &probe);
    check(rc == 0 && probe != 0, "partial data: listener survives");
    if (rc == 0) Fc_TrnDisconnect((void *)probe);
}

/* ======================================================================== */
/*  Group F: multi-connection stress                                       */
/* ======================================================================== */

/*! @brief Sequential multi-connection: 5 connect/disconnect. */
static void test_multi_conn(void)
{
    int i, ok = 1;

    for (i = 0; i < 5; i++) {
        ULONG  obj = 0;
        APIRET rc  = Fc_TrnConnect("127.0.0.1", &obj);
        if (rc != 0 || obj == 0) { ok = 0; break; }
        if (Fc_TrnSendCommand((void *)obj, 0x0001, i) != (APIRET)i) {
            ok = 0;
            Fc_TrnDisconnect((void *)obj);
            break;
        }
        Fc_TrnDisconnect((void *)obj);
    }
    check(ok, "multi-conn: 5 sequential pairs OK");
}

/* ======================================================================== */
/*  main                                                                   */
/* ======================================================================== */

/*! @brief Run all test groups; print PASS/FAIL; return exit code.
    @return 0 if all tests pass, 1 otherwise.
    @retval 0  All tests passed.
    @retval 1  At least one test failed or setup failed. */
int main(void)
{
    ULONG  probe = 0;
    APIRET rc;

    printf("PMTCP_TEST: starting\n"); fflush(stdout);

    rc = Fc_TrnConnect("127.0.0.1", &probe);
    if (rc == 0 && probe != 0) {
        Fc_TrnDisconnect((void *)probe);
        printf("PMTCP_TEST: RESULT SKIP (port 9876 in use; "
               "stop PMSERVER first)\n");
        return 1;
    }

    TBEGIN(listener_thread, NULL);
    DosSleep(1000);

    rc = Fc_TrnConnect("127.0.0.1", &probe);
    if (rc != 0 || probe == 0) {
        printf("PMTCP_TEST: RESULT FAIL (listener did not start)\n");
        return 1;
    }
    Fc_TrnDisconnect((void *)probe);
    printf("PMTCP_TEST: listener baseline OK\n"); fflush(stdout);

    test_connect_null_obj();
    test_connect_bad_address();
    test_connect_refused();

    test_connect_disconnect();
    test_connect_explicit_port();
    test_disconnect_null();

    test_fc_null_obj();
    test_fc_null_buf();

    test_fc_post_command();
    test_fc_send_command();
    test_fc_post_data();
    test_fc_send_command_data();
    test_fc_get_data();

    test_peer_close();

    test_partial_header();
    test_partial_data();

    test_multi_conn();

    if (g_fail == 0) {
        printf("PMTCP_TEST: RESULT PASS (%d/%d)\n", g_total, g_total);
        return 0;
    }
    printf("PMTCP_TEST: RESULT FAIL (%d/%d)\n", g_fail, g_total);
    return 1;
}
