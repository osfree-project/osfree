/*!
 * @file win_compat.cpp
 * @brief Win32 stubs for the OS/2 PM functions used by the PMGPI tests.
 *
 * Provides minimal implementations of WinInitialize, WinCreateMsgQueue,
 * WinGetScreenPS, WinReleasePS, WinTerminate, WinDestroyMsgQueue so that
 * testgpi.exe can link and run on Win32.
 *
 * On OS/2 all of these functions come from PMWIN.LIB (real SDK), so
 * this file compiles to nothing. The #if guard below makes it safe to
 * include in the OS/2 build without pulling in <os2win.h> (which
 * #errors on OS/2 by design).
 *
 * The Gpi* functions are NOT stubbed here: they come from PMGPI.DLL via
 * the IMPORT directives in the linker input.
 */

#if !defined(__OS2__) && !defined(__OS2V2__)

#include <os2win.h>

/*!
 * @brief Initialize PM anchor block (Win32 stub).
 * @param [in] flOptions Reserved; ignored.
 * @return Fake HAB (1).
 */
HAB APIENTRY WinInitialize(ULONG flOptions)
{
    (void)flOptions;
    return (HAB)1;
}

/*!
 * @brief Create message queue (Win32 stub).
 * @param [in] hab  Anchor block (ignored).
 * @param [in] cMsg Requested queue size (ignored).
 * @return Fake HMQ (1).
 */
HMQ APIENTRY WinCreateMsgQueue(HAB hab, LONG cMsg)
{
    (void)hab;
    (void)cMsg;
    return (HMQ)1;
}

/*!
 * @brief Destroy message queue (Win32 stub).
 * @param [in] hmq Message queue handle (ignored).
 * @return TRUE.
 */
BOOL APIENTRY WinDestroyMsgQueue(HMQ hmq)
{
    (void)hmq;
    return TRUE;
}

/*!
 * @brief Terminate PM anchor block (Win32 stub).
 * @param [in] hab Anchor block (ignored).
 * @return TRUE.
 */
BOOL APIENTRY WinTerminate(HAB hab)
{
    (void)hab;
    return TRUE;
}

/*!
 * @brief Get screen presentation space (Win32 stub).
 * @param [in] hwndDesktop Desktop window handle (ignored).
 * @return Fake HPS (1).
 */
HPS APIENTRY WinGetScreenPS(HWND hwndDesktop)
{
    (void)hwndDesktop;
    return (HPS)1;
}

/*!
 * @brief Release presentation space (Win32 stub).
 * @param [in] hps Presentation space handle (ignored).
 * @return TRUE.
 */
BOOL APIENTRY WinReleasePS(HPS hps)
{
    (void)hps;
    return TRUE;
}

#endif /* !__OS2__ && !__OS2V2__ */
