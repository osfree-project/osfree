/*! @file pmtest.cpp
    @brief Self-contained protocol test for the FreePM transport.

    Run once. Starts nothing — PMCLIENT.DLL's DllMain starts
    pmserver.exe and does the initial CONNECT. The test then:

      Phase 1  Channel A (through PMCLIENT): CONNECT idempotency,
               three CORE commands, channel A's own session id.
      Phase 2  Raw channel (direct TCP + handshake):
                 - separate CONNECT creates a separate session
                 - garbage CORE commands are echoed without crash
                 - RECONNECT against ACTIVE sessions is rejected
                 - RECONNECT with idClient=0 is rejected
      Phase 3  Disconnect callback + DETACHED->ACTIVE:
                 - close the raw socket, wait for the disconnect
                   callback to move the session to DETACHED
                 - RECONNECT from a fresh raw socket resumes the
                   same idClient
      Phase 4  Channel A is unaffected by all raw traffic.
      Phase 5  Raw socket closed before reading its reply: server
               survives, channel A still works.
      Phase 6  Idle > ulInactive (65 s): PING keeps channel A's
               session alive (idClient unchanged).
      Phase 7  EXIT frees the session: next CONNECT allocates a new
               idClient.

    Exit code: 0 if all checks passed, 1 otherwise. */

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
  #define RawClose(s)      soclose(s)
  #define RawInvalid       (-1)
#else
  typedef SOCKET RawSock;
  #define RawClose(s)      closesocket(s)
  #define RawInvalid       INVALID_SOCKET
#endif

/*! @brief Accumulated failures; determines verdict and exit code. */
static int g_failures = 0;

/*! @brief Print PASS/FAIL for one check.
    @param [in] ok    Non-zero if the check passed.
    @param [in] what  Description. */
static void check(int ok, const char *what)
{
    printf("PMTEST: [%s] %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok) g_failures++;
}

/* ======================================================================== */
/*  Channel A: through PMCLIENT                                            */
/* ======================================================================== */

/*! @brief Send CONNECT and read F_CLIENTCONFIG.
    @param [out] cfg  Receives the config.
    @return 0 on success, non-zero on transport error. */
static APIRET DoConnect(F_CLIENTCONFIG *cfg)
{
    LONG len = 0;
    if (PmcPostCommand(F_CMD_CLIENT_CONNECT, 0) != 0) return (APIRET)-1;
    if (PmcGetData(cfg, &len, (LONG)sizeof(*cfg)) != 0) return (APIRET)-1;
    if (len != (LONG)sizeof(*cfg)) return (APIRET)-1;
    return 0;
}

/* ======================================================================== */
/*  Channel B: raw TCP socket                                              */
/* ======================================================================== */

/*! @brief Open a raw TCP socket and perform the FreePM handshake
    ("pIpEtEsT" out, "PiPeTeSt" in).
    @return Connected socket, or RawInvalid. */
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
        RawClose(s);
        return RawInvalid;
    }
    if (send(s, hello, (int)strlen(hello) + 1, 0) <= 0) {
        RawClose(s);
        return RawInvalid;
    }
    memset(buf, 0, sizeof(buf));
    if (recv(s, buf, sizeof(buf), 0) <= 0) {
        RawClose(s);
        return RawInvalid;
    }
    if (strcmp(buf, "PiPeTeSt") != 0) {
        RawClose(s);
        return RawInvalid;
    }
    return s;
}

/*! @brief Send an 8-byte (cmd, data) header.
    @param [in] s     Connected raw socket.
    @param [in] cmd   Command code.
    @param [in] data  32-bit argument.
    @return 0 on success. */
static int RawSendHdr(RawSock s, ULONG cmd, ULONG data)
{
    unsigned char hdr[8];
    memcpy(hdr,     &cmd,  4);
    memcpy(hdr + 4, &data, 4);
    return (send(s, (const char *)hdr, 8, 0) == 8) ? 0 : -1;
}

/*! @brief Read exactly n bytes.
    @param [in]  s   Connected raw socket.
    @param [out] pv  Destination buffer.
    @param [in]  n   Number of bytes.
    @return 0 on success, -1 on error/short read. */
static int RawRecvN(RawSock s, void *pv, int n)
{
    char *p = (char *)pv;
    int   got = 0;
    while (got < n) {
        int r = recv(s, p + got, n - got, 0);
        if (r <= 0) return -1;
        got += r;
    }
    return 0;
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

/*! @brief Raw RECONNECT: send RECONNECT with idClient, read F_CLIENTCONFIG.
    @param [in]  s         Connected raw socket.
    @param [in]  idClient  Session to resume.
    @param [out] cfg       Receives the config.
    @return 0 on success, -1 on failure. */
static int RawReconnect(RawSock s, ULONG idClient, F_CLIENTCONFIG *cfg)
{
    if (RawSendHdr(s, F_CMD_CLIENT_RECONNECT, idClient) != 0) return -1;
    return RawRecvN(s, cfg, (int)sizeof(*cfg));
}

/* ======================================================================== */
/*  Test body                                                              */
/* ======================================================================== */

/*! @brief Run the protocol test.
    @return 0 on success, 1 on failure. */
int main(void)
{
    APIRET         rc;
    F_CLIENTCONFIG cfg;
    ULONG          idA;
    RawSock        raw1, raw2, raw3, raw4;
    F_CLIENTCONFIG rcfg;
    ULONG          idRaw;
    unsigned long  reply;

    printf("PMTEST: starting\n");
    fflush(stdout);

    /* ---------- Phase 1: channel A ---------- */

    if (DoConnect(&cfg) != 0) {
        printf("PMTEST: RESULT FAIL (initial CONNECT)\n");
        return 1;
    }
    idA = cfg.idClient;
    check(idA > 0, "channel A CONNECT returned idClient");
    printf("PMTEST: channel A idClient = %lu\n", (unsigned long)idA);
    fflush(stdout);

    if (DoConnect(&cfg) == 0) {
        check(cfg.idClient == idA, "repeated CONNECT returns same idClient");
    } else {
        check(0, "repeated CONNECT");
    }

    rc = PmcSendCommand(0x100, 0x42);
    check(rc == 0x100, "channel A reply for 0x100");
    rc = PmcSendCommand(0x200, 0x07);
    check(rc == 0x200, "channel A reply for 0x200");
    rc = PmcSendCommand(0x300, 0x00);
    check(rc == 0x300, "channel A reply for 0x300");

    /* ---------- Phase 2: raw garbage + hijack ACTIVE ---------- */

    raw1 = RawOpen();
    if (raw1 == RawInvalid) {
        check(0, "raw1 handshake");
        printf("PMTEST: RESULT FAIL\n");
        return 1;
    }
    check(1, "raw1 handshake");

    if (RawConnect(raw1, &rcfg) != 0) {
        check(0, "raw1 CONNECT");
        printf("PMTEST: RESULT FAIL\n");
        return 1;
    }
    idRaw = rcfg.idClient;
    check(idRaw > 0 && idRaw != idA,
          "raw1 CONNECT creates a separate session");
    printf("PMTEST: raw1 idClient = %lu\n", (unsigned long)idRaw);
    fflush(stdout);

    if (RawSendHdr(raw1, 0xDEADBEEFUL, 0x11223344UL) == 0 &&
        RawRecvN(raw1, &reply, (int)sizeof(reply)) == 0 &&
        reply == 0xDEADBEEFUL) {
        check(1, "garbage 0xDEADBEEF echoed without crash");
    } else {
        check(0, "garbage 0xDEADBEEF");
    }

    if (RawSendHdr(raw1, 0xFFFFFFFFUL, 0) == 0 &&
        RawRecvN(raw1, &reply, (int)sizeof(reply)) == 0 &&
        reply == 0xFFFFFFFFUL) {
        check(1, "garbage 0xFFFFFFFF echoed without crash");
    } else {
        check(0, "garbage 0xFFFFFFFF");
    }

    raw2 = RawOpen();
    if (raw2 != RawInvalid) {
        if (RawReconnect(raw2, idA, &rcfg) == 0) {
            check(rcfg.idClient == 0,
                  "RECONNECT to channel A (ACTIVE) rejected");
        } else {
            check(0, "RECONNECT to channel A (ACTIVE) transport error");
        }

        if (RawReconnect(raw2, idRaw, &rcfg) == 0) {
            check(rcfg.idClient == 0,
                  "RECONNECT to raw1 (ACTIVE) rejected");
        } else {
            check(0, "RECONNECT to raw1 (ACTIVE) transport error");
        }

        if (RawReconnect(raw2, 0, &rcfg) == 0) {
            check(rcfg.idClient == 0, "RECONNECT with idClient=0 rejected");
        } else {
            check(0, "RECONNECT with idClient=0");
        }

        RawClose(raw2);
    } else {
        check(0, "raw2 handshake");
    }

    /* ---------- Phase 3: disconnect callback + DETACHED->ACTIVE ---------- */

    RawClose(raw1);
    DosSleep(500);   /* let PMTCP's thread notice recv() EOF and call
                        the disconnect callback */

    raw3 = RawOpen();
    if (raw3 != RawInvalid) {
        if (RawReconnect(raw3, idRaw, &rcfg) == 0) {
            check(rcfg.idClient == idRaw,
                  "RECONNECT after disconnect resumes DETACHED session");
        } else {
            check(0, "RECONNECT after disconnect");
        }
        RawClose(raw3);
    } else {
        check(0, "raw3 handshake");
    }

    /* ---------- Phase 4: channel A still works ---------- */

    rc = PmcSendCommand(0x100, 0x11);
    check(rc == 0x100, "channel A still works after raw traffic");

    if (DoConnect(&cfg) == 0) {
        check(cfg.idClient == idA, "channel A session unchanged");
    } else {
        check(0, "channel A CONNECT after raw traffic");
    }

    /* ---------- Phase 5: raw socket closed before reading reply ---------- */

    raw4 = RawOpen();
    if (raw4 != RawInvalid) {
        RawSendHdr(raw4, 0xCAFEBABEUL, 0);
        RawClose(raw4);

        rc = PmcSendCommand(0x200, 0x22);
        check(rc == 0x200, "channel A survives raw socket closed mid-reply");
    } else {
        check(0, "raw4 handshake");
    }

    /* ---------- Phase 6: idle > ulInactive ---------- */

    printf("PMTEST: sleeping 65 s (ulInactive=60 s, PING must keep session)\n");
    fflush(stdout);
    DosSleep(65 * 1000);

    if (DoConnect(&cfg) == 0) {
        check(cfg.idClient == idA,
              "channel A survived 65 s idle (PING worked)");
    } else {
        check(0, "channel A CONNECT after idle");
    }

    /* ---------- Phase 7: EXIT frees the session ---------- */

    if (PmcPostCommand(F_CMD_CLIENT_EXIT, idA) == 0) {
        DosSleep(300);
        if (DoConnect(&cfg) == 0) {
            check(cfg.idClient != idA,
                  "CONNECT after EXIT allocates a new session");
        } else {
            check(0, "CONNECT after EXIT");
        }
    } else {
        check(0, "EXIT sent");
    }

    if (g_failures == 0) {
        printf("PMTEST: RESULT PASS\n");
        return 0;
    }
    printf("PMTEST: RESULT FAIL (%d)\n", g_failures);
    return 1;
}
