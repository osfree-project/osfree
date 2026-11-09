/*!
 * @file test_pmhab_reconnect.cpp
 * @brief RECONNECT protocol test.
 *
 * Transport-error-triggered reconnect cannot be exercised through
 * PMCLIENT's public API: the only way Fc_Ses* returns a transport
 * error is when the underlying socket breaks, and PMCLIENT has no
 * API to break it from the outside. So this test uses raw sockets
 * (like test_pmtcp/test_pmpipe do for transport edge cases) and
 * drives the RECONNECT wire protocol directly:
 *
 *     CONNECT(0)               -> idClient=N
 *     NWinInitialize(tid)      -> iHab=H
 *     close socket             -> server sees DISCONNECT -> DETACHED
 *     RECONNECT(N) + tid       -> cfg.idClient == N (resumed)
 *     NWinGetLastError(H)      -> 0            (HAB survived)
 *     NWinTerminate(H)         -> 1
 *     CLIENT_EXIT
 *
 * Negative cases:
 *     RECONNECT(N) with wrong tid            -> rejected (cfg.idClient=0)
 *     RECONNECT(0)                           -> rejected
 *     RECONNECT(N) on ACTIVE thread          -> rejected
 *
 * Run under a wall-clock timeout: timeout 30 ./test_pmhab_reconnect
 */
#include <stdio.h>
#include <string.h>

#include "F_platform.h"
#include "F_cmd.h"
#include "win_wire.h"

#ifndef PMERR_INVALID_HAB
#define PMERR_INVALID_HAB 0x1019
#endif

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

/*! @brief Accumulated failures; drives verdict. */
static int g_failures = 0;

/*! @brief Print PASS/FAIL for one check.
    @param [in] ok    Non-zero if the check passed.
    @param [in] what  Description. */
static void check(int ok, const char *what)
{
    printf("PMHAB_RECONNECT: [%s] %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok) g_failures++;
}

/*! @brief Reply header, exactly as PMSERVER sends it. */
struct RespHeader {
    ULONG ulStatus;
    ULONG ulResult;
    ULONG cbPayload;
    ULONG ulLastError;
};

/* ======================================================================== */
/*  Raw socket helpers                                                     */
/* ======================================================================== */

/*! @brief Send exactly n bytes.
    @param [in] s   Connected raw socket.
    @param [in] pv  Source buffer.
    @param [in] n   Bytes to send.
    @return 0 on success, -1 on error. */
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

/*! @brief Read exactly n bytes.
    @param [in]  s   Connected raw socket.
    @param [out] pv  Destination buffer.
    @param [in]  n   Number of bytes.
    @return 0 on success, -1 on error/short read. */
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

/*! @brief Open a raw TCP socket and perform the FreePM handshake.
    @return Connected socket, or RawInvalid on any failure. */
static RawSock RawOpen(void)
{
    struct sockaddr_in sa;
    RawSock s;
    char    buf[16];
    const char *hello = "pIpEtEsT";
    int     rc, w;

    printf("  RawOpen: socket()\n"); fflush(stdout);
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == RawInvalid) {
        printf("  RawOpen: socket() failed\n"); fflush(stdout);
        return RawInvalid;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family      = AF_INET;
    sa.sin_port        = htons(9876);
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("  RawOpen: connect(127.0.0.1:9876)\n"); fflush(stdout);
    rc = connect(s, (struct sockaddr *)&sa, sizeof(sa));
    if (rc != 0) {
        printf("  RawOpen: connect() failed: rc=%d\n", rc); fflush(stdout);
        RawClose(s); return RawInvalid;
    }

    printf("  RawOpen: send handshake\n"); fflush(stdout);
    w = send(s, hello, (int)strlen(hello) + 1, 0);
    if (w <= 0) {
        printf("  RawOpen: send() returned %d\n", w); fflush(stdout);
        RawClose(s); return RawInvalid;
    }

    printf("  RawOpen: recv handshake reply\n"); fflush(stdout);
    memset(buf, 0, sizeof(buf));
    rc = recv(s, buf, 9, 0);
    if (rc <= 0) {
        printf("  RawOpen: recv() returned %d\n", rc); fflush(stdout);
        RawClose(s); return RawInvalid;
    }

    printf("  RawOpen: got %d bytes\n", rc); fflush(stdout);
    if (memcmp(buf, "PiPeTeSt", 9) != 0) {
        printf("  RawOpen: magic mismatch\n"); fflush(stdout);
        RawClose(s); return RawInvalid;
    }
    return s;
}

/* ======================================================================== */
/*  Wire helpers                                                           */
/* ======================================================================== */

/*! @brief Send CONNECT(idClient) and read F_CLIENTCONFIG.
    @return 0 on success, -1 on failure. */
static int RawConnect(RawSock s, ULONG idClient, F_CLIENTCONFIG *cfg)
{
    ULONG cmd = F_CMD_CLIENT_CONNECT, data = idClient;
    unsigned char hdr[8];
    memcpy(hdr,     &cmd,  4);
    memcpy(hdr + 4, &data, 4);
    if (RawSendN(s, hdr, 8) != 0) return -1;
    return RawRecvN(s, cfg, (int)sizeof(*cfg));
}

/*! @brief Send RECONNECT(idClient) + {tid}, read F_CLIENTCONFIG.
    @return 0 on success, -1 on failure. */
static int RawReconnect(RawSock s, ULONG idClient, ULONG tid, F_CLIENTCONFIG *cfg)
{
    ULONG cmd = F_CMD_CLIENT_RECONNECT, data = idClient;
    unsigned char hdr[8];
    memcpy(hdr,     &cmd,  4);
    memcpy(hdr + 4, &data, 4);
    if (RawSendN(s, hdr, 8) != 0) return -1;
    if (RawSendN(s, &tid, 4) != 0) return -1;
    return RawRecvN(s, cfg, (int)sizeof(*cfg));
}

/*! @brief Send WIN command with cbArgs=0 and read reply header.
    @return 0 on success, -1 on failure. */
static int RawWin(RawSock s, ULONG fnCode, ULONG data, RespHeader *out)
{
    ULONG cmd = F_CMD_WIN(fnCode);
    ULONG cb  = 0;
    unsigned char hdr[8];
    memcpy(hdr,     &cmd,  4);
    memcpy(hdr + 4, &data, 4);
    if (RawSendN(s, hdr, 8) != 0) return -1;
    if (RawSendN(s, &cb,  4) != 0) return -1;
    return RawRecvN(s, out, (int)sizeof(*out));
}

/*! @brief Send CLIENT_EXIT (no reply).
    @return 0 on success, -1 on failure. */
static int RawExit(RawSock s)
{
    ULONG cmd = F_CMD_CLIENT_EXIT, data = 0;
    unsigned char hdr[8];
    memcpy(hdr,     &cmd,  4);
    memcpy(hdr + 4, &data, 4);
    return RawSendN(s, hdr, 8);
}

/* ======================================================================== */

int main(void)
{
    RawSock        s;
    F_CLIENTCONFIG cfg;
    RespHeader     rh;
    ULONG          idClient, hab;
    ULONG          tid = 100;
    int            attempt, resumed;

#if defined(__WIN32__) || defined(_WIN32)
    {
        WSADATA wsaData;
        int     wsaErr;
        printf("PMHAB_RECONNECT: WSAStartup...\n"); fflush(stdout);
        wsaErr = WSAStartup(MAKEWORD(1, 1), &wsaData);
        if (wsaErr != 0) {
            printf("PMHAB_RECONNECT: WSAStartup failed: %d\n", wsaErr);
            return 1;
        }
    }
#endif

    printf("PMHAB_RECONNECT: starting\n"); fflush(stdout);

    /* ---- Setup: CONNECT + NWinInitialize over socket A ---- */
    s = RawOpen();
    if (s == RawInvalid) {
        printf("PMHAB_RECONNECT: RESULT FAIL (raw open)\n");
        return 1;
    }

    if (RawConnect(s, 0, &cfg) != 0 || cfg.idClient == 0) {
        RawClose(s);
        printf("PMHAB_RECONNECT: RESULT FAIL (CONNECT)\n");
        return 1;
    }
    idClient = cfg.idClient;
    printf("PMHAB_RECONNECT: idClient=%lu\n", (unsigned long)idClient);
    fflush(stdout);

    if (RawWin(s, NWinInitialize, tid, &rh) != 0 ||
        rh.ulStatus != 0 || rh.ulResult == 0) {
        RawClose(s);
        printf("PMHAB_RECONNECT: RESULT FAIL (NWinInitialize)\n");
        return 1;
    }
    hab = rh.ulResult;
    printf("PMHAB_RECONNECT: iHab=%lu (tid=%lu)\n",
           (unsigned long)hab, (unsigned long)tid);
    fflush(stdout);

    /* ---- Drop the socket; server must transition to DETACHED ---- */
    RawClose(s);
    DosSleep(500);

    /* ---- RECONNECT. Retry in case the DISCONNECT callback has not
       landed yet on the server side. Each attempt uses a fresh socket;
       a rejected RECONNECT (cfg.idClient=0) leaves the server thread
       in DETACHED and we retry. ---- */
    resumed = 0;
    for (attempt = 0; attempt < 10; attempt++) {
        s = RawOpen();
        if (s == RawInvalid) { DosSleep(100); continue; }
        if (RawReconnect(s, idClient, tid, &cfg) == 0 &&
            cfg.idClient == idClient) {
            resumed = 1;
            break;
        }
        RawClose(s);
        DosSleep(200);
    }
    check(resumed, "RECONNECT(idClient, tid) resumes DETACHED session");
    if (!resumed) {
        printf("PMHAB_RECONNECT: RESULT FAIL (RECONNECT rejected)\n");
        return 1;
    }

    /* ---- HAB survived: NWinGetLastError still returns 0 ---- */
    if (RawWin(s, NWinGetLastError, hab, &rh) != 0) {
        check(0, "NWinGetLastError transport"); goto done;
    }
    check(rh.ulStatus == 0 && rh.ulResult == 0,
          "HAB survived reconnect (NWinGetLastError = 0)");

    /* ---- Negative: RECONNECT for a thread that is now ACTIVE ---- */
    {
        RawSock s2 = RawOpen();
        int     r;
        if (s2 == RawInvalid) {
            check(0, "negative: raw open");
        } else {
            r = RawReconnect(s2, idClient, tid, &cfg);
            check(r == 0 && cfg.idClient == 0,
                  "RECONNECT on ACTIVE thread rejected");
            RawClose(s2);
        }
    }

    /* ---- Negative: RECONNECT with wrong tid ---- */
    {
        RawSock s3 = RawOpen();
        int     r;
        if (s3 == RawInvalid) {
            check(0, "negative wrong tid: raw open");
        } else {
            r = RawReconnect(s3, idClient, tid + 9999, &cfg);
            check(r == 0 && cfg.idClient == 0,
                  "RECONNECT with unknown tid rejected");
            RawClose(s3);
        }
    }

    /* ---- Negative: RECONNECT with idClient=0 ---- */
    {
        RawSock s4 = RawOpen();
        int     r;
        if (s4 == RawInvalid) {
            check(0, "negative idClient=0: raw open");
        } else {
            r = RawReconnect(s4, 0, tid, &cfg);
            check(r == 0 && cfg.idClient == 0,
                  "RECONNECT with idClient=0 rejected");
            RawClose(s4);
        }
    }

    /* ---- Cleanup ---- */
    if (RawWin(s, NWinTerminate, hab, &rh) != 0 ||
        rh.ulStatus != 0 || rh.ulResult != 1) {
        check(0, "NWinTerminate after reconnect");
    } else {
        check(1, "NWinTerminate after reconnect = 1");
    }
    RawExit(s);

done:
    RawClose(s);

    if (g_failures == 0) {
        printf("PMHAB_RECONNECT: RESULT PASS\n");
        return 0;
    }
    printf("PMHAB_RECONNECT: RESULT FAIL (%d)\n", g_failures);
    return 1;
}
