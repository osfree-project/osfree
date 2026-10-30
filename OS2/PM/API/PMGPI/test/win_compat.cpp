/*!
 * @file win_compat.c
 * @brief Win32 stubs for the OS/2 PM functions used by the PMGPI tests.
 *
 * Provides minimal implementations of WinInitialize, WinCreateMsgQueue,
 * WinGetScreenPS, WinReleasePS, WinTerminate, WinDestroyMsgQueue so that
 * testgpi.exe can link and run on Win32.
 *
 * The Gpi* functions are NOT stubbed here: they come from PMGPI.DLL via
 * the IMPORT directives in the linker input.
 */

#include <os2win.h>

HAB APIENTRY WinInitialize(ULONG flOptions)
{
    (void)flOptions;
    return (HAB)1;
}

HMQ APIENTRY WinCreateMsgQueue(HAB hab, LONG cMsg)
{
    (void)hab;
    (void)cMsg;
    return (HMQ)1;
}

BOOL APIENTRY WinDestroyMsgQueue(HMQ hmq)
{
    (void)hmq;
    return TRUE;
}

BOOL APIENTRY WinTerminate(HAB hab)
{
    (void)hab;
    return TRUE;
}

HPS APIENTRY WinGetScreenPS(HWND hwndDesktop)
{
    (void)hwndDesktop;
    return (HPS)1;
}

BOOL APIENTRY WinReleasePS(HPS hps)
{
    (void)hps;
    return TRUE;
}
