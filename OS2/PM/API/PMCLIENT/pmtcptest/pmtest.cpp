/*! @file pmtcp_test.cpp
    @brief Targeted regression tests for TCP transport fixes 1, 2, 3, 5.

    Each test exercises a bug that was present before the fixes.
    All "channel A alive" checks go through PMCLIENT (PmcSendCommand),
    which only works if pmserver.exe is still running and responsive —
    so a crash in the fixed path is detected immediately.

    Run under a wall-clock timeout:   timeout 60 ./pmtcp_test

    If the process hangs, that IS the bug being tested (pre-fix
    behaviour was a deadlock or an unbounded block); the timeout
    turns that into a FAIL instead of a stuck terminal.
*/

#include <stdio.h>
#include <string.h>

#include "F_platform.h"
#include "pmclient.h"
#include "F_cmd.h"

#if defined(__OS2__) || defined(__OS2V2__)
  #include <types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  extern "C" int _System soclose(int);
  typedef int RawSock;
  #define RawClose(s)  soclose(s)
  #define RawInvalid   (-1)
#else
  typedef SOCKET RawSock;
  #define RawClose(s)  closesocket(s)
  #define RawInvalid   INVALID_SOCKET
#endif

/*! @brief Failure counter; drives verdict. */
static int g_failures = 0;

static void check(int ok, const char *what)
{
    printf("PMTCP_TEST: [%s] %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok) g_failures++;
}

/* ---------- raw socket helpers (same as pmtest.cpp) ---------- */

static RawSock RawOpen(void)
{
    struct sockaddr_in sa;
    RawSock s;
    char    buf[32];
    const char *hello = "pIpEtEsT";

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == RawInvalid) return RawInvalid;

    memset(&sa, 0, sizeof(sa));
    sa.sin_family      = AF_INET;
    sa.sin_port        = htons(9876);
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) != 0) {
        RawClose(s); return RawInvalid;
    }
    if (send(s, hello, (int)strlen(hello) + 1, 0) <= 0) {
        RawClose(s); return RawInvalid;
    }
    memset(buf, 0, sizeof(buf));
    if (recv(s, buf, sizeof(buf), 0) <= 0) {
        RawClose(s); return RawInvalid;
    }
    if (strcmp(buf, "PiPeTeSt") != 0) {
        RawClose(s); return RawInvalid;
    }
    return s;
}

static int RawSendN(RawSock s, const void *pv, int n)
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

static int RawRecvN(RawSock s, void *pv, int n)
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

static int RawSendHdr(RawSock s, ULONG cmd, ULONG data)
{
    unsigned char hdr[8];
    memcpy(hdr,     &cmd,  4);
    memcpy(hdr + 4, &data, 4);
    return RawSendN(s, hdr, 8);
}

/*! @brief Raw CONNECT: send CONNECT, read F_CLIENTCONFIG.
    @param [in]  s    Connected raw socket.
    @param [out] cfg  Receives the config.
    @return 0 on success, -1 on failure. */
static int RawConnect(RawSock s, F_CLIENTCONFIG *cfg)
{
    if (RawSendHdr(s, F_CMD_CLIENT_CONNECT, 0) != 0) return -1;
    return RawRecvN(s, cfg, (int)sizeof(*cfg));
}

/* ======================================================================== */
/*  Test 5: handshake over-read                                            */
/* ======================================================================== */

/*! @brief Old code: HandShake's recv(buf, 32) can consume handshake AND
    the first command; strcmp passes (NUL at 8), but the remaining
    command bytes are gone, so both sides hang.
    New code: reads exactly 9 bytes; the following command is read
    separately and echoed normally.

    Wire, one send():  "pIpEtEsT\0" + F_CMD_CLIENT_CONNECT(0) + 8 bytes.
    Server must: (1) read exactly 9 bytes for handshake, (2) return
    "PiPeTeSt\0", (3) read the 8-byte CONNECT as a separate command,
    (4) create a session and return F_CLIENTCONFIG.
    Pre-fix: (3) reads from mid-handshake garbage; test hangs. */
static void test_handshake_overread(void)
{
    RawSock s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sa;
    char hs_and_connect[9 + 8];
    char reply_hs[9];
    F_CLIENTCONFIG cfg;
    ULONG cmd = F_CMD_CLIENT_CONNECT, data = 0;

    if (s == RawInvalid) { check(0, "handshake_overread: socket"); return; }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family      = AF_INET;
    sa.sin_port        = htons(9876);
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) != 0) {
        RawClose(s); check(0, "handshake_overread: connect"); return;
    }

    memcpy(hs_and_connect, "pIpEtEsT\0", 9);
    memcpy(hs_and_connect + 9,     &cmd,  4);
    memcpy(hs_and_connect + 9 + 4, &data, 4);

    if (RawSendN(s, hs_and_connect, sizeof(hs_and_connect)) != 0) {
        RawClose(s); check(0, "handshake_overread: send"); return;
    }

    /* Expect: "PiPeTeSt\0" then 16-byte F_CLIENTCONFIG (CONNECT reply).
       Pre-fix: reads buf[32] on server, eats first 23 bytes;
       CONNECT is never seen, both sides hang. */
    if (RawRecvN(s, reply_hs, 9) != 0) {
        RawClose(s);
        check(0, "handshake_overread: no handshake reply (HANG - fix 5 broken)");
        return;
    }
    if (memcmp(reply_hs, "PiPeTeSt", 9) != 0) {
        RawClose(s); check(0, "handshake_overread: wrong handshake reply"); return;
    }
    if (RawRecvN(s, &cfg, (int)sizeof(cfg)) != 0) {
        RawClose(s);
        check(0, "handshake_overread: no CONNECT reply (fix 5 broken)");
        return;
    }
    check(cfg.idClient > 0,
          "handshake over-read: CONNECT after handshake created session");
    RawClose(s);
}

/* ======================================================================== */
/*  Test 1+3: partial GPI payload, then client closes                      */
/* ======================================================================== */

/*! @brief Reproduces the exact log pattern:
        _F_RecvDataFromClient(..., size=80004) -> l=<partial>
        DispatchGpi(FnCode=0x70, ArgsSize=80004)
        NGpiPolyLine(..., cbArgs=80004)   <- OOB read
    Old code: Recv returns l=<partial> with rc=0; dispatcher trusts
    80004 and reads 10000 POINTLs from a short buffer -> crash /
    corruption.
    New code: Recv loops; close -> EOF -> rc=-1 with *pcbReceived set
    to partial count; _F_RecvDataFromClient sees *l != size and returns
    -1; session goes to DETACHED, server survives.

    CONNECT is issued first: without a session the server answers
    "no session" and never calls _F_RecvDataFromClient, so neither
    fix 1 nor fix 3 is exercised. */
static void test_partial_data_disconnect(void)
{
    RawSock        s = RawOpen();
    F_CLIENTCONFIG cfg;
    ULONG          cmd, hps, argsSize;

    if (s == RawInvalid) { check(0, "partial_data: open"); return; }

    /* Session required so the GPI dispatch path is reached. */
    if (RawConnect(s, &cfg) != 0 || cfg.idClient == 0) {
        RawClose(s); check(0, "partial_data: CONNECT"); return;
    }

    /* FnCode 0x70 = NGpiPolyLine, the one from the original crash log.
       argsSize=80004 = 4-byte lCount + 10000 * sizeof(POINTL). */
    cmd      = F_CMD_GPI(0x70);
    hps      = 1;
    argsSize = 80004;

    if (RawSendHdr(s, cmd, hps) != 0 ||
        RawSendN(s, &argsSize, 4) != 0 ||
        RawSendN(s, "PARTIAL", 7) != 0)
    {
        RawClose(s); check(0, "partial_data: send"); return;
    }

    /* Close after 7 of 80004 bytes. Fix 1: Recv returns -1 with
       *pcbReceived=7 (EOF). Fix 3: _F_RecvDataFromClient sees
       *l != size and returns -1 before DispatchGpi is called. */
    RawClose(s);

    DosSleep(500);
    {
        APIRET rc = PmcSendCommand(0x100, 0x42);
        check(rc == 0x100,
              "partial-data disconnect: server alive (fix 1+3)");
    }
}

/* ======================================================================== */
/*  Test 3: partial CORE header, then close                                */
/* ======================================================================== */

/*! @brief Regression: server must handle a truncated 8-byte CORE header
    cleanly. Both old and new code handle it, but this catches any future
    regression that makes Recv block forever on a closed socket. */
static void test_partial_header(void)
{
    RawSock s = RawOpen();
    ULONG   half = 0x100;

    if (s == RawInvalid) { check(0, "partial_header: open"); return; }

    RawSendN(s, &half, 4);
    RawClose(s);
    DosSleep(300);

    check(PmcSendCommand(0x200, 0x01) == 0x200,
          "partial-header close: server alive");
}

/* ======================================================================== */
/*  Regression: full valid GPI payload                                     */
/* ======================================================================== */

/*! @brief The fix must not reject valid large payloads. Send the full
    80004-byte GpiPolyLine args and verify the server processes it and
    the session stays in sync afterwards. */
static void test_large_valid_payload(void)
{
    RawSock s = RawOpen();
    F_CLIENTCONFIG cfg;
    ULONG cmd, hps, argsSize, lCount;
    char  points[80000];
    ULONG resp[4];

    if (s == RawInvalid) { check(0, "large_valid: open"); return; }

    /* Session required: otherwise the server answers "no session" and
       never calls _F_RecvDataFromClient / NGpiPolyLine. */
    if (RawConnect(s, &cfg) != 0 || cfg.idClient == 0) {
        RawClose(s); check(0, "large_valid: CONNECT"); return;
    }

    cmd      = F_CMD_GPI(0x70);
    hps      = 1;
    argsSize = 80004;
    lCount   = 10000;

    if (RawSendHdr(s, cmd, hps)   != 0 ||
        RawSendN(s, &argsSize, 4) != 0 ||
        RawSendN(s, &lCount, 4)   != 0)
    {
        RawClose(s); check(0, "large_valid: send header"); return;
    }

    memset(points, 0, sizeof(points));
    if (RawSendN(s, points, sizeof(points)) != 0) {
        RawClose(s); check(0, "large_valid: send data"); return;
    }

    if (RawRecvN(s, resp, 16) != 0) {
        RawClose(s);
        check(0, "large_valid: no response (HANG - fix 1 broken)");
        return;
    }
    RawClose(s);

    check(PmcSendCommand(0x300, 0x99) == 0x300,
          "large valid GPI payload: server processed it, session synced");
}

/* ======================================================================== */
/*  Stress: repeated partial-data disconnects                              */
/* ======================================================================== */

/*! @brief Old code: each iteration corrupts server heap by OOB read.
    One iteration might not crash, but 20 probably will. New code: all
    iterations clean.

    Each iteration now does CONNECT first, so the GPI path is reached
    and _F_RecvDataFromClient really sees short reads. A failed
    CONNECT aborts the test — without a session we would only be
    testing the "no session" path, not the fixes. */
static void test_repeated_partial(void)
{
    int i;

    for (i = 0; i < 20; i++) {
        RawSock        s = RawOpen();
        F_CLIENTCONFIG cfg;
        ULONG          cmd, hps, argsSize;

        if (s == RawInvalid) {
            check(0, "repeated_partial: open");
            return;
        }
        if (RawConnect(s, &cfg) != 0 || cfg.idClient == 0) {
            RawClose(s);
            check(0, "repeated_partial: CONNECT");
            return;
        }
        cmd = F_CMD_GPI(0x70); hps = 1; argsSize = 80004;
        RawSendHdr(s, cmd, hps);
        RawSendN(s, &argsSize, 4);
        RawSendN(s, "x", 1);
        RawClose(s);
    }
    DosSleep(1000);
    check(PmcSendCommand(0x400, 0x55) == 0x400,
          "20x partial-data disconnect: server alive");
}

/* ======================================================================== */
/*  main                                                                   */
/* ======================================================================== */

int main(void)
{
    printf("PMTCP_TEST: starting\n"); fflush(stdout);

    /* Baseline: channel A responds. PMCLIENT already did CONNECT in DllMain. */
    if (PmcSendCommand(0x100, 0x00) != 0x100) {
        printf("PMTCP_TEST: RESULT FAIL (channel A not responding)\n");
        return 1;
    }
    printf("PMTCP_TEST: channel A baseline OK\n"); fflush(stdout);

    test_handshake_overread();
    test_partial_data_disconnect();
    test_partial_header();
    test_large_valid_payload();
    test_repeated_partial();

    if (g_failures == 0) {
        printf("PMTCP_TEST: RESULT PASS\n");
        return 0;
    }
    printf("PMTCP_TEST: RESULT FAIL (%d)\n", g_failures);
    return 1;
}
