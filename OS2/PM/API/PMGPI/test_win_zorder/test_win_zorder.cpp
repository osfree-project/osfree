/*!
 * @file test_win_zorder.cpp
 * @brief S2: parent / Z-order / DestroyTree / class + parent rejection.
 */
#define INCL_PM
#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_DOSPROCESS
#include <F_platform.h>

#include <stdio.h>
#include <string.h>

static int g_fail = 0;

static void check(int ok, const char *msg) {
    printf("[%s] %s\n", ok ? "PASS" : "FAIL", msg);
    if (!ok) g_fail++;
}

int main(void)
{
    HAB  hab;
    HMQ  hmq;
    HWND hwndTop, hwndBottom, hwndP, hwndC1, hwndC2;
    HWND hwndLongClass, hwndBad;
    char szLong[64];

    hab = WinInitialize(0);
    check(hab != 0, "WinInitialize");
    if (!hab) return 1;

    hmq = WinCreateMsgQueue(hab, 0);
    check(hmq != NULLHANDLE, "WinCreateMsgQueue");
    if (hmq == NULLHANDLE) goto terminate;

    /* T-S2-1: HWND_TOP, no siblings -> rank 1. */
    hwndTop = WinCreateWindow(HWND_DESKTOP, "TestTop", "Top",
                              WS_VISIBLE,
                              10, 10, 100, 100,
                              NULLHANDLE, HWND_TOP, 0, NULL, NULL);
    check(hwndTop != NULLHANDLE, "T-S2-1: CreateWindow(HWND_TOP)");

    /* T-S2-2: HWND_BOTTOM with one sibling -> goes below. */
    hwndBottom = WinCreateWindow(HWND_DESKTOP, "TestBottom", "Bottom",
                                 WS_VISIBLE,
                                 20, 20, 100, 100,
                                 NULLHANDLE, HWND_BOTTOM, 0, NULL, NULL);
    check(hwndBottom != NULLHANDLE, "T-S2-2: CreateWindow(HWND_BOTTOM)");
    check(hwndTop != hwndBottom, "T-S2-2: distinct hwnds");

    /* T-S2-3: parent + two children; DestroyTree(parent) cascades. */
    hwndP = WinCreateWindow(HWND_DESKTOP, "TestParent", "Parent",
                            WS_VISIBLE,
                            100, 100, 300, 300,
                            NULLHANDLE, HWND_TOP, 0, NULL, NULL);
    check(hwndP != NULLHANDLE, "T-S2-3: CreateWindow(parent)");

    hwndC1 = WinCreateWindow(hwndP, "TestChild1", "C1",
                             WS_VISIBLE,
                             10, 10, 50, 50,
                             NULLHANDLE, HWND_TOP, 1, NULL, NULL);
    check(hwndC1 != NULLHANDLE, "T-S2-3: CreateWindow(child1)");

    hwndC2 = WinCreateWindow(hwndP, "TestChild2", "C2",
                             WS_VISIBLE,
                             70, 70, 50, 50,
                             NULLHANDLE, HWND_TOP, 2, NULL, NULL);
    check(hwndC2 != NULLHANDLE, "T-S2-3: CreateWindow(child2)");

    check(WinDestroyWindow(hwndP), "T-S2-3: DestroyWindow(parent)");

    /* Children destroyed by DestroyTree -> second destroy must fail. */
    check(!WinDestroyWindow(hwndC1), "T-S2-3: child1 already gone");
    check(!WinDestroyWindow(hwndC2), "T-S2-3: child2 already gone");

    /* T-S2-4: class name longer than m_szClass-1 rejected. */
    memset(szLong, 'A', sizeof(szLong) - 1);
    szLong[sizeof(szLong) - 1] = 0;
    hwndLongClass = WinCreateWindow(HWND_DESKTOP, szLong, "Long",
                                    WS_VISIBLE,
                                    0, 0, 10, 10,
                                    NULLHANDLE, HWND_TOP, 0, NULL, NULL);
    check(hwndLongClass == NULLHANDLE, "T-S2-4: class > 31 rejected");

    /* T-S2-5: nonexistent parent rejected. */
    hwndBad = WinCreateWindow((HWND)0xDEAD, "TestBad", "Bad",
                              WS_VISIBLE,
                              0, 0, 10, 10,
                              NULLHANDLE, HWND_TOP, 0, NULL, NULL);
    check(hwndBad == NULLHANDLE, "T-S2-5: bad parent rejected");

    /* T-S2-6: NULLHANDLE as hwndInsertBehind is invalid per OS/2 PM
     * ("This parameter must be HWND_TOP, HWND_BOTTOM, or a child of
     * hwndParent"). Must be rejected with NULLHANDLE. */
    hwndBad = WinCreateWindow(HWND_DESKTOP, "TestNullBehind", "NullBehind",
                              WS_VISIBLE,
                              0, 0, 10, 10,
                              NULLHANDLE, NULLHANDLE, 0, NULL, NULL);
    check(hwndBad == NULLHANDLE, "T-S2-6: NULLHANDLE behind rejected");

    /* Cleanup. */
    WinDestroyWindow(hwndTop);
    WinDestroyWindow(hwndBottom);

    check(WinDestroyMsgQueue(hmq), "WinDestroyMsgQueue");

terminate:
    WinTerminate(hab);

    printf("\n%s (%d)\n", g_fail ? "FAIL" : "PASS", g_fail);
    return g_fail ? 1 : 0;
}
