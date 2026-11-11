/*!
 * @file test_winopenwindowdc.cpp
 * @brief Test for WinOpenWindowDC 1:1 association.
 *
 * Verifies:
 *   - Valid HWND returns a non-NULL HDC.
 *   - Second call with the same HWND returns the exact same HDC.
 *   - Invalid HWND returns NULLHANDLE.
 */
#define INCL_WINWINDOWMGR
#define INCL_DOSPROCESS
#include <F_platform.h>

#include <stdio.h>

static int g_fail = 0;

static void check(int ok, const char *msg) {
    printf("[%s] %s\n", ok ? "PASS" : "FAIL", msg);
    if (!ok) g_fail++;
}

int main(void)
{
    HAB hab;
    HDC hdc1;
    HDC hdc2;
    HDC hdc_invalid;

    hab = WinInitialize(0);
    check(hab != 0, "WinInitialize");
    if (!hab) return 1;

    /* First call: creates a new DC for HWND_DESKTOP. */
    hdc1 = WinOpenWindowDC(HWND_DESKTOP);
    check(hdc1 != NULLHANDLE, "WinOpenWindowDC(HWND_DESKTOP) - first call");

    /* Second call: must return the existing HDC (1:1 association). */
    hdc2 = WinOpenWindowDC(HWND_DESKTOP);
    check(hdc2 != NULLHANDLE, "WinOpenWindowDC(HWND_DESKTOP) - second call");
    check(hdc1 == hdc2, "WinOpenWindowDC returns the same HDC (no duplicate)");

    /* Invalid HWND: must return NULLHANDLE. */
    hdc_invalid = WinOpenWindowDC((HWND)999);
    check(hdc_invalid == NULLHANDLE, "WinOpenWindowDC(invalid HWND) -> NULLHANDLE");

    printf("  waiting 2 s...\n");
    DosSleep(2000);

    WinTerminate(hab);

    printf("\n%s (%d)\n", g_fail ? "FAIL" : "PASS", g_fail);
    return g_fail ? 1 : 0;
}
