/*! @file test_pmshm.cpp
    @brief Transport-level tests for PMSHM.DLL.

    Tests exercise the exported Fc_Trn* / Fs_Trn* API. ShmLink is
    internal to PMSHM and is not tested directly.

    Constraint: listener queue name is hardcoded inside PMSHM for both
    Fc_TrnConnect (NULL -> default) and Fs_TrnRunListener. The test
    binds it as a listener, so PMSERVER.EXE must NOT be running.

    Run under wall-clock timeout: timeout 60 ./test_pmshm
    A hang indicates a transport bug (Recv blocked on closed peer).
*/

#ifndef INCL_DOSERRORS
  #define INCL_DOSERRORS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#include "F_shm.hpp"
#include "F_debug.h"

/* ======================================================================== */
/*  Test bookkeeping                                                       */
/* ======================================================================== */

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
    printf("PMSHM_TEST: [%s] %s\n", ok ? "PASS" : "FAIL", what);
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
      @brief Calling convention for callbacks passed into PMSHM (OS/2). */
  #define TEST_CB APIENTRY
#else
  /*! @def TEST_CB
      @brief Calling convention for callbacks passed into PMSHM (Win32). */
  #define TEST_CB __stdcall
#endif

/* ======================================================================== */
/*  Unified handler                                                        */
/* ======================================================================== */

/*! @brief Static 64 KB buffer to avoid 64 KB stack in server thread. */
static char g_bigbuf[65536];

/*! @brief Unified handler for all test scenarios.

    Dispatch by cmd:
    - 0x0001: echo par back as 4-byte retcode
    - 0x0002: read par bytes, echo back
    - 0x0003: send fixed 8-byte payload
    - 0x0004: read par bytes, reply 4-byte retcode
    - 0x0005: read par bytes in 64 KB chunks, echo back (large payload)

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
    case 0x0005: {
        /* Large payload test: read "par" bytes via chunked Recv, echo back. */
        ULONG totalRead = 0;
        ULONG len;
        char *buf = (char *)malloc((size_t)par);
        if (!buf) break;
        while (totalRead < (ULONG)par) {
            ULONG want = (ULONG)par - totalRead;
            if (want > 65536) want = 65536;
            len = 0;
            if (Fs_TrnGetData(obj, buf + totalRead, &len, want) != 0) break;
            totalRead += len;
        }
        if (totalRead == (ULONG)par) {
            ULONG sent = 0;
            while (sent < (ULONG)par) {
                ULONG chunk = (ULONG)par - sent;
                if (chunk > 65536) chunk = 65536;
                if (Fs_TrnPostData(obj, buf + sent, chunk) != 0) break;
                sent += chunk;
            }
        }
        free(buf);
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
/*  Group A: Fc_TrnConnect — error paths                                   */
/* ======================================================================== */

/*! @brief Fc_TrnConnect(NULL obj) -> PMC_ERR_OPEN, no crash. */
static void test_connect_null_obj(void)
{
    APIRET rc = Fc_TrnConnect(NULL, NULL);
    check(rc == PMC_ERR_OPEN, "Fc_TrnConnect(NULL obj) -> PMC_ERR_OPEN");
}

/*! @brief Fc_TrnDisconnect(NULL) is a no-op returning 0. */
static void test_disconnect_null(void)
{
    check(Fc_TrnDisconnect(NULL) == 0, "Fc_TrnDisconnect(NULL) -> 0");
}

/* ======================================================================== */
/*  Group B: connect / disconnect lifecycle                                */
/* ======================================================================== */

/*! @brief Happy-path connect + disconnect. */
static void test_connect_disconnect(void)
{
    ULONG  obj = 0;
    APIRET rc  = Fc_TrnConnect(NULL, &obj);

    check(rc == 0 && obj != 0, "Fc_TrnConnect default name OK");
    check(Fc_TrnDisconnect((void *)obj) == 0, "Fc_TrnDisconnect OK");
}

/* ======================================================================== */
/*  Group C: NULL obj / NULL buf                                           */
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

    if (Fc_TrnConnect(NULL, &obj) != 0) {
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
/*  Group D: round-trip through listener                                   */
/* ======================================================================== */

/*! @brief PostCommand + GetData: handler echoes par. */
static void test_fc_post_command(void)
{
    ULONG obj = 0;
    int   ret = 0;
    ULONG l = 0;

    if (Fc_TrnConnect(NULL, &obj) != 0) {
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

    if (Fc_TrnConnect(NULL, &obj) != 0) {
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

    if (Fc_TrnConnect(NULL, &obj) != 0) {
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

    if (Fc_TrnConnect(NULL, &obj) != 0) {
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

    if (Fc_TrnConnect(NULL, &obj) != 0) {
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
/*  Group E: large payload / chunking                                      */
/* ======================================================================== */

/*! @brief 64 KB round-trip (one or two QMS_SLOT_SIZE chunks). */
static void test_large_payload(void)
{
    ULONG obj = 0;
    static char payload[65536];
    static char got[65536];
    ULONG l = 0;
    int   i, ok;

    if (Fc_TrnConnect(NULL, &obj) != 0) {
        check(0, "large_payload: connect"); return;
    }
    for (i = 0; i < (int)sizeof(payload); i++) payload[i] = (char)(i & 0xFF);

    if (Fc_TrnPostCommand((void *)obj, 0x0002, sizeof(payload)) != 0) {
        check(0, "large_payload: PostCommand");
        Fc_TrnDisconnect((void *)obj);
        return;
    }
    if (Fc_TrnPostData((void *)obj, payload, sizeof(payload)) != 0) {
        check(0, "large_payload: PostData");
        Fc_TrnDisconnect((void *)obj);
        return;
    }
    if (Fc_TrnGetData((void *)obj, got, &l, sizeof(got)) != 0) {
        check(0, "large_payload: GetData");
        Fc_TrnDisconnect((void *)obj);
        return;
    }
    ok = (l == sizeof(payload)) &&
         (memcmp(got, payload, sizeof(payload)) == 0);
    check(ok, "large_payload: 64 KB round-trip");
    Fc_TrnDisconnect((void *)obj);
}

/*! @brief 4 MB via chunking (QMS_SLOT_SIZE = 1 MB, so 4 chunks). */
static void test_multichunk_payload(void)
{
    ULONG obj = 0;
    char *payload = (char *)malloc(4 * 1024 * 1024);
    char *got     = (char *)malloc(4 * 1024 * 1024);
    ULONG l = 0;
    ULONG sent = 0, totalRead = 0;
    int   ok = 0;

    if (!payload || !got) {
        if (payload) free(payload);
        if (got) free(got);
        check(0, "multichunk: malloc"); return;
    }
    memset(payload, 'A', 4 * 1024 * 1024);

    if (Fc_TrnConnect(NULL, &obj) != 0) {
        check(0, "multichunk: connect");
        free(payload); free(got);
        return;
    }

    if (Fc_TrnPostCommand((void *)obj, 0x0005, 4 * 1024 * 1024) != 0) {
        check(0, "multichunk: PostCommand");
        Fc_TrnDisconnect((void *)obj);
        free(payload); free(got);
        return;
    }

    while (sent < 4 * 1024 * 1024) {
        ULONG chunk = 4 * 1024 * 1024 - sent;
        if (chunk > 65536) chunk = 65536;
        if (Fc_TrnPostData((void *)obj, payload + sent, chunk) != 0) break;
        sent += chunk;
    }

    if (sent == 4 * 1024 * 1024) {
        while (totalRead < 4 * 1024 * 1024) {
            ULONG want = 4 * 1024 * 1024 - totalRead;
            if (want > 65536) want = 65536;
            l = 0;
            if (Fc_TrnGetData((void *)obj, got + totalRead, &l, want) != 0) break;
            totalRead += l;
        }
        ok = (totalRead == 4 * 1024 * 1024) &&
             (memcmp(got, payload, 4 * 1024 * 1024) == 0);
    }

    check(ok, "multichunk: 4 MB round-trip through chunks");
    Fc_TrnDisconnect((void *)obj);
    free(payload);
    free(got);
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
        APIRET rc  = Fc_TrnConnect(NULL, &obj);
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

    printf("PMSHM_TEST: starting\n"); fflush(stdout);

    rc = Fc_TrnConnect(NULL, &probe);
    if (rc == 0 && probe != 0) {
        Fc_TrnDisconnect((void *)probe);
        printf("PMSHM_TEST: RESULT SKIP (backend already running)\n");
        return 1;
    }

    TBEGIN(listener_thread, NULL);
    DosSleep(1000);

    rc = Fc_TrnConnect(NULL, &probe);
    if (rc != 0 || probe == 0) {
        printf("PMSHM_TEST: RESULT FAIL (listener did not start)\n");
        return 1;
    }
    Fc_TrnDisconnect((void *)probe);
    printf("PMSHM_TEST: listener baseline OK\n"); fflush(stdout);

    test_connect_null_obj();
    test_disconnect_null();
    test_connect_disconnect();
    test_fc_null_obj();
    test_fc_null_buf();
    test_fc_post_command();
    test_fc_send_command();
    test_fc_post_data();
    test_fc_send_command_data();
    test_fc_get_data();
    test_large_payload();
    test_multichunk_payload();
    test_multi_conn();

    if (g_fail == 0) {
        printf("PMSHM_TEST: RESULT PASS (%d/%d)\n", g_total, g_total);
        return 0;
    }
    printf("PMSHM_TEST: RESULT FAIL (%d/%d)\n", g_fail, g_total);
    return 1;
}
