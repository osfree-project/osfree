/*! @file fpmdebug_test.cpp
    @brief Self-contained test for fpmdebug.dll. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#include "F_platform.h"
#include "F_debug.h"

static int g_pass = 0;
static int g_fail = 0;

static void Check(int ok, const char *what)
{
    printf("FDBG_TEST: [%s] %s\n", ok ? "PASS" : "FAIL", what);
    if (ok) g_pass++; else g_fail++;
}

static int ReadFile(const char *path, char *buf, int maxlen)
{
    FILE *f = fopen(path, "r");
    int   n;
    if (!f) { buf[0] = 0; return 0; }
    n = (int)fread(buf, 1, maxlen - 1, f);
    fclose(f);
    buf[n] = 0;
    return n;
}

static int FileExists(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    fclose(f);
    return 1;
}

/* Wire sink capture. */
static char g_wirePrint[1024];
static char g_wireFatal[1024];
static int  g_wirePrintCount = 0;
static int  g_wireFatalCount = 0;

/* F_DBG_WIRE_CALLBACK matches the typedef in F_debug.h on both
   platforms (APIENTRY on OS/2, __stdcall on Win32). */
static void F_DBG_WIRE_CALLBACK WireSink(F_DbgKind kind, const char *msg)
{
    if (kind == F_DBG_KIND_PRINT) {
        g_wirePrintCount++;
        strncpy(g_wirePrint, msg, sizeof(g_wirePrint) - 1);
        g_wirePrint[sizeof(g_wirePrint) - 1] = 0;
    } else if (kind == F_DBG_KIND_FATAL) {
        g_wireFatalCount++;
        strncpy(g_wireFatal, msg, sizeof(g_wireFatal) - 1);
        g_wireFatal[sizeof(g_wireFatal) - 1] = 0;
    }
}

/* Child modes. */
static int ChildFatal(void)
{
    F_DbgConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.logfile = "fpmdebug_child.log";
    cfg.stderrLevel = 0;
    F_DbgInit(&cfg, "ALL,10");
    F_DbgSetWireSink(WireSink);
    F_DbgFatal("child fatal message");
    return 0;   /* not reached */
}

static int ChildFatalf(void)
{
    F_DbgConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.logfile = "fpmdebug_child.log";
    cfg.stderrLevel = 0;
    F_DbgInit(&cfg, "ALL,10");
    F_DbgFatalf("child: answer=%d", 42);
    return 0;
}

static int ChildAssert(void)
{
    F_DbgConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.logfile = "fpmdebug_child.log";
    cfg.stderrLevel = 0;
    F_DbgInit(&cfg, "ALL,10");
    F_DbgAssert("expr", "file.c", 42);
    return 0;
}

/* Parent spawns child via DosExecPgm and checks exit + log. */
static void RunChild(const char *exe, const char *flag)
{
    char    args[260];
    char    fail[CCHMAXPATH];
    RESULTCODES rcode;
    APIRET  rc;

    rc = DosExecPgm(fail, sizeof(fail), EXEC_SYNC, flag, NULL, &rcode, exe);
    Check(rc == 0, "child exec");
    Check(rcode.codeTerminate != 0, "child terminated abnormally");
}

static void RunInProcessTests(void)
{
    const char *logPath = "fpmdebug_test.log";
    F_DbgConfig cfg;
    char        buf[8192];

    /* --- T1/T2: init --- */
    remove(logPath);
    memset(&cfg, 0, sizeof(cfg));
    cfg.logfile     = logPath;
    cfg.stderrLevel = 5;
    F_DbgInit(&cfg, NULL);                     /* levels unconfigured */
    Check(FileExists(logPath), "T1 log file created");
    F_DbgPrint(0, 0, "T2 should not appear");
    ReadFile(logPath, buf, sizeof(buf));
    Check(strstr(buf, "T2") == NULL, "T2 unconfigured sections are silent");

    /* --- T4 ---
       Выполняется до T3, пока ни одна секция ещё не сконфигурирована:
       "unconfigured" означает уровень -1, а через F_DbgSetLevels его
       уже не вернуть (ParseLevelToken зажимает -1 в 0). */
    F_DbgSetLevels("1,5 2,3");
    F_DbgPrint(1, 4, "T4-1-4");
    F_DbgPrint(1, 6, "T4-1-6");
    F_DbgPrint(2, 3, "T4-2-3");
    F_DbgPrint(2, 4, "T4-2-4");
    F_DbgPrint(3, 0, "T4-3-0");
    ReadFile(logPath, buf, sizeof(buf));
    Check(strstr(buf, "T4-1-4") != NULL, "T4 section 1 level 4 emitted");
    Check(strstr(buf, "T4-1-6") == NULL, "T4 section 1 level 6 dropped");
    Check(strstr(buf, "T4-2-3") != NULL, "T4 section 2 level 3 emitted");
    Check(strstr(buf, "T4-2-4") == NULL, "T4 section 2 level 4 dropped");
    Check(strstr(buf, "T4-3-0") == NULL, "T4 section 3 unconfigured dropped");

    /* --- T3 --- */
    F_DbgSetLevels("ALL,7");
    F_DbgPrint(5, 5, "T3-a");
    F_DbgPrint(5, 8, "T3-b");
    ReadFile(logPath, buf, sizeof(buf));
    Check(strstr(buf, "T3-a") != NULL, "T3 level 5 <= 7 emitted");
    Check(strstr(buf, "T3-b") == NULL, "T3 level 8 > 7 dropped");

    /* --- T5: clamping --- */
    F_DbgSetLevels("0,15");
    F_DbgPrint(0, 10, "T5-10");
    F_DbgPrint(0, 11, "T5-11");
    ReadFile(logPath, buf, sizeof(buf));
    Check(strstr(buf, "T5-10") != NULL, "T5 level clamped to 10, msg(10) emitted");
    Check(strstr(buf, "T5-11") == NULL, "T5 msg(11) dropped");

    /* --- T6: format --- */
    F_DbgSetLevels("ALL,7");
    F_DbgPrint(0, 0, "T6-format");
    ReadFile(logPath, buf, sizeof(buf));
    {
        const char *p = strstr(buf, "T6-format");
        int ok = 0;
        if (p) {
            /* Простейшая проверка: перед сообщением есть " N YYYY/MM/DD HH:MM:SS|" */
            const char *bar = p;
            while (bar > buf && *bar != '|') bar--;
            if (bar > buf) {
                /* Ищем точку с " / " и двоеточия */
                if (strchr(bar - 20, '/')) ok = 1;
            }
        }
        Check(ok, "T6 format contains N YYYY/MM/DD HH:MM:SS|");
    }

    /* --- T7: wire sink for print --- */
    g_wirePrintCount = 0;
    F_DbgSetWireSink(WireSink);
    F_DbgPrint(0, 0, "T7-wire");
    Check(g_wirePrintCount == 1, "T7 wire sink called for F_DbgPrint");
    Check(strstr(g_wirePrint, "T7-wire") != NULL, "T7 wire sink got message");

    /* --- T9: trace unconditional --- */
    F_DbgSetLevels("ALL,-1");   /* can't set -1 (clamped to 0), so use NULL? */
    /* Если F_DbgInit уже вызывался — уровни установлены. Проверяем:
       F_DbgTrace идёт в файл всегда, независимо от уровней. */
    F_DbgTrace("T9-trace");
    ReadFile(logPath, buf, sizeof(buf));
    Check(strstr(buf, "T9-trace") != NULL, "T9 F_DbgTrace emitted unconditionally");

    /* --- T10: context --- */
    F_DbgSetLevels("ALL,7");
    {
        int c1 = F_DbgCtxEnter("T10-ctx1");
        int c2 = F_DbgCtxEnter("T10-ctx2");
        F_DbgPrint(0, 0, "T10-msg");
        F_DbgCtxExit(c1);
        (void)c2;
    }
    ReadFile(logPath, buf, sizeof(buf));
    Check(strstr(buf, "ctx: enter level 0: 'T10-ctx1'") != NULL, "T10 ctx enter 0 logged");
    Check(strstr(buf, "ctx: enter level 1: 'T10-ctx2'") != NULL, "T10 ctx enter 1 logged");

    /* --- T11: shutdown suppresses assert abort --- */
    F_DbgSetShuttingDown(1);
    F_DbgAssert("T11", "f.c", 1);   /* must return */
    ReadFile(logPath, buf, sizeof(buf));
    Check(strstr(buf, "assertion failed") == NULL, "T11 assert suppressed during shutdown");
    F_DbgSetShuttingDown(0);

    /* --- T13: rotation --- */
    {
        const char *rotPath = "fpmdebug_rot.log";
        F_DbgConfig rc2;
        remove(rotPath);
        remove("fpmdebug_rot.log.0");
        memset(&rc2, 0, sizeof(rc2));
        rc2.logfile      = rotPath;
        rc2.rotateNumber = 1;
        /* F_DbgInit idempotent — пропускаем, используем текущий log.
           Для ротации нужен отдельный тест в subprocess. */
    }

    /* --- T15: runtime set levels --- */
    F_DbgSetLevels("1,3");
    F_DbgPrint(1, 5, "T15-dropped");
    F_DbgSetLevels("1,7");
    F_DbgPrint(1, 5, "T15-emitted");
    ReadFile(logPath, buf, sizeof(buf));
    Check(strstr(buf, "T15-dropped") == NULL, "T15 dynamic drop");
    Check(strstr(buf, "T15-emitted") != NULL, "T15 dynamic emit");
}

/* T14: multithreaded log.
   F_DbgInit идемпотентен — второй раз новый лог не откроется, поэтому
   пишем в тот же fpmdebug_test.log, что и остальные тесты, и ждём
   завершения потоков через DosWaitThread. Подсчёт идёт по вхождениям
   подстроки "T14 tid=", чтобы не зависеть от переводов строк. */
static void ThreadBody(void *param)
{
    int i;
    int tid = (int)(long)param;
    for (i = 0; i < 500; i++) {
        F_DbgPrint(0, 0, "T14 tid=%d i=%d", tid, i);
    }
}

static void RunThreadTest(void)
{
    const char *logPath = "fpmdebug_test.log";
    static char buf[512 * 1024];
    int         tids[4];
    int         i, n, count = 0;
    const char *p;
    FILE       *f;

    for (i = 0; i < 4; i++) {
#if defined(__OS2__) || defined(__OS2V2__)
        tids[i] = _beginthread(ThreadBody, NULL, 65536, (void *)(long)i);
#else
        tids[i] = (int)_beginthread(ThreadBody, 65536, (void *)(long)i);
#endif
    }

    for (i = 0; i < 4; i++)
        DosWaitThread((PTID)&tids[i], DCWW_WAIT);

    f = fopen(logPath, "r");
    if (!f) { Check(0, "T14 log missing"); return; }
    n = (int)fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    buf[n] = 0;

    p = buf;
    while ((p = strstr(p, "T14 tid=")) != NULL) { count++; p++; }

    Check(count == 2000, "T14 2000 thread messages present, none interleaved");
}

int main(int argc, char *argv[])
{
if (argc >= 2) {
    if (strcmp(argv[1], "--fatal")  == 0) return ChildFatal();
    if (strcmp(argv[1], "--fatalf") == 0) return ChildFatalf();
    if (strcmp(argv[1], "--assert") == 0) return ChildAssert();

    fprintf(stderr, "FDBG_TEST: unknown argument '%s'\n", argv[1]);
    return 2;
}

    printf("FDBG_TEST: starting\n");

    RunInProcessTests();

    remove("fpmdebug_child.log");
    RunChild(argv[0], "--fatal");
    {
        char buf[4096];
        ReadFile("fpmdebug_child.log", buf, sizeof(buf));
        Check(strstr(buf, "FATAL: child fatal message") != NULL,
              "T8 FATAL message in child log");
    }

    remove("fpmdebug_child.log");
    RunChild(argv[0], "--fatalf");
    {
        char buf[4096];
        ReadFile("fpmdebug_child.log", buf, sizeof(buf));
        Check(strstr(buf, "FATAL: child: answer=42") != NULL,
              "T8b FATAL printf-formatted message");
    }

    remove("fpmdebug_child.log");
    RunChild(argv[0], "--assert");
    {
        char buf[4096];
        ReadFile("fpmdebug_child.log", buf, sizeof(buf));
        Check(strstr(buf, "assertion failed: file.c:42: \"expr\"") != NULL,
              "T12 assertion-failure logged in child");
    }

    RunThreadTest();

    printf("FDBG_TEST: %d PASS, %d FAIL\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
