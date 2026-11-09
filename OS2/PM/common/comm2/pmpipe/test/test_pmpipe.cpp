/*! @file test_pmpipe.cpp
    @brief Transport-level tests for PMPIPE.DLL.
*/
#ifndef INCL_DOSERRORS
  #define INCL_DOSERRORS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#include "F_pipe.hpp"
#include "F_debug.h"

static int g_fail = 0;
static int g_total = 0;

static void check(int ok, const char *what)
{
    g_total++;
    printf("PMPIPE_TEST: [%s] %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok) g_fail++;
}

#if defined(__OS2__) || defined(__OS2V2__)
  #define TBEGIN(fn, arg) _beginthread((void(*)(void*))(fn), NULL, 65536, (void*)(arg))
#else
  #define TBEGIN(fn, arg) _beginthread((void(*)(void*))(fn), 65536, (void*)(arg))
#endif

#if defined(__OS2__) || defined(__OS2V2__)
  #define TEST_CB APIENTRY
#else
  #define TEST_CB __stdcall
#endif

static char g_bigbuf[65536];

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
    default:
        break;
    }
}

static void listener_thread(void *p)
{
    (void)p;
    Fs_TrnRunListener((void *)unified_handler, NULL);
}

static void test_connect_null_obj(void)
{
    APIRET rc = Fc_TrnConnect(NULL, NULL);
    check(rc == PMC_ERR_OPEN, "Fc_TrnConnect(NULL obj) -> PMC_ERR_OPEN");
}

static void test_disconnect_null(void)
{
    check(Fc_TrnDisconnect(NULL) == 0, "Fc_TrnDisconnect(NULL) -> 0");
}

static void test_connect_disconnect(void)
{
    ULONG  obj = 0;
    APIRET rc  = Fc_TrnConnect(NULL, &obj);

    check(rc == 0 && obj != 0, "Fc_TrnConnect default name OK");
    check(Fc_TrnDisconnect((void *)obj) == 0, "Fc_TrnDisconnect OK");
}

static void test_fc_null_obj(void)
{
    char  buf[8] = {0};
    ULONG len = 0;

    check(Fc_TrnPostCommand(NULL, 1, 2) != 0, "Fc_TrnPostCommand(NULL) fails");
    check(Fc_TrnPostData(NULL, buf, 8) != 0, "Fc_TrnPostData(NULL) fails");
    check(Fc_TrnGetData(NULL, buf, &len, 8) == (APIRET)-1, "Fc_TrnGetData(NULL) -> -1");
    check(Fc_TrnSendCommand(NULL, 1, 2) == (APIRET)-1, "Fc_TrnSendCommand(NULL) -> -1");
    check(Fc_TrnSendCommandData(NULL, 1, 2, buf, 8) == (APIRET)-1, "Fc_TrnSendCommandData(NULL) -> -1");
}

static void test_fc_null_buf(void)
{
    ULONG obj = 0;
    ULONG len = 0;

    if (Fc_TrnConnect(NULL, &obj) != 0) {
        check(0, "null_buf: connect"); return;
    }
    check(Fc_TrnPostData((void *)obj, NULL, 8) != 0, "Fc_TrnPostData(NULL buf, len=8) fails");
    check(Fc_TrnGetData((void *)obj, NULL, &len, 8) == (APIRET)-1, "Fc_TrnGetData(NULL buf, size=8) -> -1");
    check(Fc_TrnSendCommand((void *)obj, 0x0001, 0x55) == 0x55, "connection usable after NULL buf reject");
    Fc_TrnDisconnect((void *)obj);
}

static void test_fc_post_command(void)
{
    ULONG obj = 0;
    int   ret = 0;
    ULONG l = 0;

    if (Fc_TrnConnect(NULL, &obj) != 0) { check(0, "post_command: connect"); return; }
    check(Fc_TrnPostCommand((void *)obj, 0x0001, 0x1234) == 0, "Fc_TrnPostCommand -> 0");
    check(Fc_TrnGetData((void *)obj, &ret, &l, 4) == 0, "Fc_TrnGetData reply");
    check(ret == 0x1234, "post_command: par echoed");
    Fc_TrnDisconnect((void *)obj);
}

static void test_fc_send_command(void)
{
    ULONG  obj = 0;
    APIRET ret;

    if (Fc_TrnConnect(NULL, &obj) != 0) { check(0, "send_command: connect"); return; }
    ret = Fc_TrnSendCommand((void *)obj, 0x0001, 0xABCD);
    check(ret == 0xABCD, "Fc_TrnSendCommand retcode = par");
    Fc_TrnDisconnect((void *)obj);
}

static void test_fc_post_data(void)
{
    ULONG obj = 0;
    char  payload[16] = "test-payload-42";
    char  got[16] = {0};
    ULONG l = 0;

    if (Fc_TrnConnect(NULL, &obj) != 0) { check(0, "post_data: connect"); return; }
    check(Fc_TrnPostCommand((void *)obj, 0x0002, 16) == 0, "post_data: PostCommand");
    check(Fc_TrnPostData((void *)obj, payload, 16) == 0, "post_data: PostData");
    check(Fc_TrnGetData((void *)obj, got, &l, 16) == 0, "post_data: GetData");
    check(l == 16 && memcmp(got, payload, 16) == 0, "post_data: payload echoed");
    Fc_TrnDisconnect((void *)obj);
}

static void test_fc_send_command_data(void)
{
    ULONG  obj = 0;
    char   payload[9] = "PAYLOAD!";
    APIRET ret;

    if (Fc_TrnConnect(NULL, &obj) != 0) { check(0, "send_command_data: connect"); return; }
    ret = Fc_TrnSendCommandData((void *)obj, 0x0004, 8, payload, 8);
    check(ret == 0xCAFEBABE, "Fc_TrnSendCommandData retcode match");
    Fc_TrnDisconnect((void *)obj);
}

static void test_fc_get_data(void)
{
    ULONG obj = 0;
    char  got[8] = {0};
    ULONG l = 0;

    if (Fc_TrnConnect(NULL, &obj) != 0) { check(0, "get_data: connect"); return; }
    check(Fc_TrnPostCommand((void *)obj, 0x0003, 0) == 0, "get_data: PostCommand");
    check(Fc_TrnGetData((void *)obj, got, &l, 8) == 0, "get_data: GetData");
    check(l == 8 && memcmp(got, "REPLYyy", 8) == 0, "get_data: server payload matches");
    Fc_TrnDisconnect((void *)obj);
}

static void test_large_payload(void)
{
    ULONG obj = 0;
    static char payload[65536];
    static char got[65536];
    ULONG l = 0;
    int   i, ok;

    if (Fc_TrnConnect(NULL, &obj) != 0) { check(0, "large_payload: connect"); return; }
    for (i = 0; i < (int)sizeof(payload); i++) payload[i] = (char)(i & 0xFF);

    if (Fc_TrnPostCommand((void *)obj, 0x0002, sizeof(payload)) != 0) {
        check(0, "large_payload: PostCommand"); Fc_TrnDisconnect((void *)obj); return;
    }
    if (Fc_TrnPostData((void *)obj, payload, sizeof(payload)) != 0) {
        check(0, "large_payload: PostData"); Fc_TrnDisconnect((void *)obj); return;
    }
    if (Fc_TrnGetData((void *)obj, got, &l, sizeof(got)) != 0) {
        check(0, "large_payload: GetData"); Fc_TrnDisconnect((void *)obj); return;
    }
    ok = (l == sizeof(payload)) && (memcmp(got, payload, sizeof(payload)) == 0);
    check(ok, "large_payload: 64 KB round-trip");
    Fc_TrnDisconnect((void *)obj);
}

static void test_multi_conn(void)
{
    int i, ok = 1;

    for (i = 0; i < 5; i++) {
        ULONG  obj = 0;
        APIRET rc  = Fc_TrnConnect(NULL, &obj);
        if (rc != 0 || obj == 0) { ok = 0; break; }
        if (Fc_TrnSendCommand((void *)obj, 0x0001, i) != (APIRET)i) {
            ok = 0; Fc_TrnDisconnect((void *)obj); break;
        }
        Fc_TrnDisconnect((void *)obj);
    }
    check(ok, "multi-conn: 5 sequential pairs OK");
}

int main(void)
{
    ULONG  probe = 0;
    APIRET rc;

    printf("PMPIPE_TEST: starting\n"); fflush(stdout);

    rc = Fc_TrnConnect(NULL, &probe);
    if (rc == 0 && probe != 0) {
        Fc_TrnDisconnect((void *)probe);
        printf("PMPIPE_TEST: RESULT SKIP (backend already running)\n");
        return 1;
    }

    TBEGIN(listener_thread, NULL);
    DosSleep(1000);

    rc = Fc_TrnConnect(NULL, &probe);
    if (rc != 0 || probe == 0) {
        printf("PMPIPE_TEST: RESULT FAIL (listener did not start)\n");
        return 1;
    }
    Fc_TrnDisconnect((void *)probe);
    printf("PMPIPE_TEST: listener baseline OK\n"); fflush(stdout);

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
    test_multi_conn();

    if (g_fail == 0) {
        printf("PMPIPE_TEST: RESULT PASS (%d/%d)\n", g_total, g_total);
        return 0;
    }
    printf("PMPIPE_TEST: RESULT FAIL (%d/%d)\n", g_fail, g_total);
    return 1;
}
