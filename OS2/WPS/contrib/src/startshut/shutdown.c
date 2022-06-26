
/*
 *@@sourcefile shutdwn.c:
 *      this file contains all the XShutdown code, which
 *      was in xfdesk.c before V0.84.
 *
 *      XShutdown is a (hopefully) complete rewrite of what
 *      WinShutdownSystem does. This code is also used for
 *      "Restart Desktop", since it does in part the same thing.
 *
 *      This file implements (in this order):
 *
 *      --  Shutdown dialogs. See xsdConfirmShutdown and
 *          xsdConfirmRestartWPS.
 *
 *      --  Shutdown interface. See xsdInitiateShutdown,
 *          xsdInitiateRestartWPS, and xsdInitiateShutdownExt.
 *
 *      --  Shutdown settings notebook pages. See xsdShutdownInitPage
 *          and below.
 *
 *      --  The Shutdown thread itself, which does the grunt
 *          work of shutting down the system, together with
 *          the Update thread, which monitors the window list
 *          while shutting down. See fntShutdownThread and
 *          fntUpdateThread.
 *
 *      All the functions in this file have the xsd* prefix.
 *
 *@@header "startshut\shutdown.h"
 */

/*
 *      Copyright (C) 1997-2015 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSPROCESS
#define INCL_DOSSESMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINFRAMEMGR
#define INCL_WINDIALOGS
#define INCL_WINPOINTERS
#define INCL_WINSHELLDATA
#define INCL_WINPROGRAMLIST
#define INCL_WINSWITCHLIST
#define INCL_WINCOUNTRY
#define INCL_WINSYS
#define INCL_WINMENUS
#define INCL_WINSTATICS
#define INCL_WINENTRYFIELDS
#define INCL_WINBUTTONS
#define INCL_WINLISTBOXES
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <stdarg.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\animate.h"            // icon and other animations
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\exeh.h"               // executable helpers
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\procstat.h"           // DosQProcStat handling
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\wphandle.h"           // file-system object handles
#include "helpers\xprf.h"               // replacement profile (INI) functions
#include "helpers\xstring.h"            // extended string helpers
#include "helpers\xwpsecty.h"           // XWorkplace Security base

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"                     // needed for shutdown folder

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "xwpapi.h"                     // public XWorkplace definitions
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\notebook.h"            // generic XWorkplace notebook handling

// headers in /hook
#include "hook\xwphook.h"

#include "filesys\object.h"             // XFldObject implementation
#include "filesys\xthreads.h"           // extra XWorkplace threads

#include "media\media.h"                // XWorkplace multimedia support

#include "startshut\apm.h"              // APM/ACPI power-off for XShutdown
#include "startshut\archives.h"         // archiving declarations
#include "startshut\shutdown.h"         // XWorkplace eXtended Shutdown

// other SOM headers
#pragma hdrstop
#include <wpdesk.h>                     // WPDesktop; includes WPFolder also
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

static THREADINFO       G_tiShutdownThread = {0},
                        G_tiUpdateThread = {0};

static ULONG            G_ulShutdownState = XSD_IDLE;
                // V0.9.19 (2002-04-24) [umoeller]
                // -- XSD_* flag signalling status

// forward declarations
MRESULT EXPENTRY fnwpShutdownThread(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2);
void _Optlink fntUpdateThread(PTHREADINFO pti);

/* ******************************************************************
 *
 *   Shutdown interface
 *
 ********************************************************************/

/*
 *@@ xsdQueryShutdownSettings:
 *      this fills the specified SHUTDOWNPARAMS array with
 *      the current shutdown settings, as specified by the
 *      user in the desktop's settings notebooks.
 *
 *      Notes:
 *
 *      -- psdp->optAnimate is set to the animation setting
 *         which applies according to whether reboot is enabled
 *         or not.
 *
 *      -- There is no "current setting" for the user reboot
 *         command. As a result, psdp->szRebootCommand is
 *         zeroed.
 *
 *      -- Neither is there a "current setting" for whether
 *         to use "restart Desktop" or "logoff" instead. To later use
 *         "restart Desktop", set psdp->ulRestartWPS and maybe
 *         psdp->optWPSReuseStartupFolder also.
 *
 *@@added V0.9.7 (2001-01-25) [umoeller]
 *@@changed V1.0.5 (2006-06-26) [pr]: added ACPI support
 */

VOID xsdQueryShutdownSettings(PSHUTDOWNPARAMS psdp)
{
    ULONG flShutdown = 0;
#ifndef __NOXSHUTDOWN__
    flShutdown = cmnQuerySetting(sflXShutdown);
#endif

    memset(psdp, 0, sizeof(SHUTDOWNPARAMS));
    psdp->optReboot = ((flShutdown & XSD_REBOOT) != 0);
#ifndef __EASYSHUTDOWN__
    psdp->optConfirm = (!(flShutdown & XSD_NOCONFIRM));
#else
    psdp->optConfirm = TRUE;
#endif
    psdp->optDebug = FALSE;

    // psdp->ulRestartWPS = 0;         // no, do shutdown
    psdp->ulCloseMode = SHUT_SHUTDOWN;

    psdp->optWPSProcessShutdown = TRUE;
    psdp->optWPSCloseWindows = TRUE;
    psdp->optAutoCloseVIO = ((flShutdown & XSD_AUTOCLOSEVIO) != 0);
    psdp->optLog = ((flShutdown & XSD_LOG) != 0);

    /* if (psdp->optReboot)
        // animate on reboot? V0.9.3 (2000-05-22) [umoeller]
        psdp->optAnimate = ((flShutdown & XSD_ANIMATE_REBOOT) != 0);
    else
        psdp->optAnimate = ((flShutdown & XSD_ANIMATE_SHUTDOWN) != 0);
       */

    psdp->optPowerOff = (  ((flShutdown & XSD_POWEROFF) != 0)  // V1.0.5 (2006-06-26) [pr]
                         && (apmPowerOffSupported() || acpiPowerOffSupported())
                        );
    psdp->optDelay = ((flShutdown & XSD_DELAY) != 0);  // V1.0.5 (2006-06-26) [pr]

    psdp->optACPIOff = ((flShutdown & XSD_ACPIPOWEROFF) != 0);  // V1.0.5 (2006-06-26) [pr]

    psdp->optWPSReuseStartupFolder = psdp->optWPSCloseWindows;

    psdp->optEmptyTrashCan = ((flShutdown & XSD_EMPTY_TRASH) != 0);

#ifndef __EASYSHUTDOWN__
    psdp->optWarpCenterFirst = ((flShutdown & XSD_WARPCENTERFIRST) != 0);
#endif

    psdp->szRebootCommand[0] = 0;
}

/*
 *@@ xsdIsShutdownRunning:
 *      returns the XSD_* value that represents
 *      the current shutdown state:
 *
 *      --  XSD_IDLE                0
 *      --  XSD_CONFIRMING          1
 *      --  XSD_CLOSING             2
 *      --  XSD_CANCELLED           3
 *      --  XSD_ALLCLOSED_SAVING    4
 *      --  XSD_SAVEDONE_FLUSHING   5
 *
 *@@added V0.9.19 (2002-04-24) [umoeller]
 */

ULONG xsdQueryShutdownState(VOID)
{
    return G_ulShutdownState;
}

/*
 *@@ StartShutdownThread:
 *      starts the Shutdown thread with the specified
 *      parameters.
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 */

STATIC VOID StartShutdownThread(BOOL fStartShutdown,
                                BOOL fPlayRestartDesktopSound,     // in: else: play shutdown sound
                                PSHUTDOWNPARAMS psdp)
{
    if (psdp)
    {
        if (fStartShutdown)
        {
            // everything OK: create shutdown thread,
            // which will handle the rest
            thrCreate(&G_tiShutdownThread,
                      fntShutdownThread,
                      NULL, // running flag
                      "Shutdown",
                      THRF_PMMSGQUEUE,    // changed V0.9.12 (2001-05-29) [umoeller]
                      (ULONG)psdp);           // pass SHUTDOWNPARAMS to thread
#ifndef __NOXSYSTEMSOUNDS__
            cmnPlaySystemSound(fPlayRestartDesktopSound
                                    ? MMSOUND_XFLD_RESTARTWPS
                                    : MMSOUND_XFLD_SHUTDOWN);
#endif

            return;
        }
        else
            free(psdp);     // fixed V0.9.1 (99-12-12)
    }

    G_ulShutdownState = XSD_IDLE; // V0.9.19 (2002-04-24) [umoeller]
}

/*
 *@@ xsdInitiateShutdown:
 *      common shutdown entry point; checks sflXShutdown
 *      for all the XSD_* flags (shutdown options).
 *      If compiled with XFLDR_DEBUG defined (in common.h),
 *      debug mode will also be turned on if the SHIFT key is
 *      pressed at call time (that is, when the menu item is
 *      selected).
 *
 *      This routine will display a confirmation box,
 *      if the settings want it, and then start the
 *      main shutdown thread (xsd_fntShutdownThread),
 *      which will keep running even after shutdown
 *      is complete, unless the user presses the
 *      "Cancel shutdown" button.
 *
 *      Although this method does return almost
 *      immediately (after the confirmation dlg is dismissed),
 *      the shutdown will proceed in the separate thread
 *      after this function returns.
 *
 *@@changed V0.9.0 [umoeller]: global SHUTDOWNPARAMS removed
 *@@changed V0.9.0 [umoeller]: this used to be an XFldDesktop instance method
 *@@changed V0.9.1 (99-12-10) [umoeller]: fixed KERNELGLOBALS locks
 *@@changed V0.9.1 (99-12-12) [umoeller]: fixed memory leak when shutdown was cancelled
 *@@changed V0.9.3 (2000-05-22) [umoeller]: added animate on reboot
 *@@changed V0.9.4 (2000-08-03) [umoeller]: added "empty trash can"
 *@@changed V0.9.11 (2001-04-25) [umoeller]: changed pending spool jobs msg to always abort now
 *@@changed V1.0.5 (2006-06-26) [pr]: added ACPI shutdown support
 *@@changed V1.0.8 (2007-04-15) [pr]: removed print job test @@fixes 920
 */

BOOL xsdInitiateShutdown(VOID)
{
    BOOL                fStartShutdown = TRUE;
    PSHUTDOWNPARAMS     psdp = (PSHUTDOWNPARAMS)malloc(sizeof(SHUTDOWNPARAMS));

    if (G_ulShutdownState != XSD_IDLE)
        // shutdown thread already running: return!
        fStartShutdown = FALSE;

    // lock shutdown menu items
    G_ulShutdownState = XSD_CONFIRMING;

    if (fStartShutdown)
    {
        ULONG flShutdown = 0;
#ifndef __NOXSHUTDOWN__
        flShutdown = cmnQuerySetting(sflXShutdown);
#endif

        memset(psdp, 0, sizeof(SHUTDOWNPARAMS));
        psdp->optReboot = ((flShutdown & XSD_REBOOT) != 0);
        // psdp->ulRestartWPS = 0;
        psdp->ulCloseMode = SHUT_SHUTDOWN;
        psdp->optWPSProcessShutdown = TRUE;
        psdp->optWPSCloseWindows = TRUE;
        psdp->optWPSReuseStartupFolder = psdp->optWPSCloseWindows;
#ifndef __EASYSHUTDOWN__
        psdp->optConfirm = (!(flShutdown & XSD_NOCONFIRM));
#else
        psdp->optConfirm = TRUE;
#endif
        psdp->optAutoCloseVIO = ((flShutdown & XSD_AUTOCLOSEVIO) != 0);
#ifndef __EASYSHUTDOWN__
        psdp->optWarpCenterFirst = ((flShutdown & XSD_WARPCENTERFIRST) != 0);
#endif
        psdp->optLog = ((flShutdown & XSD_LOG) != 0);
        /* if (psdp->optReboot)
            // animate on reboot? V0.9.3 (2000-05-22) [umoeller]
            psdp->optAnimate = ((flShutdown & XSD_ANIMATE_REBOOT) != 0);
        else
            psdp->optAnimate = ((flShutdown & XSD_ANIMATE_SHUTDOWN) != 0);
           */

        psdp->optPowerOff = (  ((flShutdown & XSD_POWEROFF) != 0)  // V1.0.5 (2006-06-26) [pr]
                             && (apmPowerOffSupported() || acpiPowerOffSupported())
                            );
        psdp->optDelay = ((flShutdown & XSD_DELAY) != 0);

        psdp->optACPIOff = ((flShutdown & XSD_ACPIPOWEROFF) != 0);  // V1.0.5 (2006-06-26) [pr]

        #ifdef __DEBUG__
            psdp->optDebug = doshQueryShiftState();
        #else
            psdp->optDebug = FALSE;
        #endif

        psdp->optEmptyTrashCan = ((flShutdown & XSD_EMPTY_TRASH) != 0);

        psdp->szRebootCommand[0] = 0;

        if (psdp->optConfirm)
        {
            ULONG ulReturn = xsdConfirmShutdown(psdp);
            if (ulReturn != DID_OK)
                fStartShutdown = FALSE;
        }

        /* V1.0.8 (2007-04-15) [pr]: seems an unnecessary restriction @@fixes 920
        if (fStartShutdown)
        {
            // check for pending spool jobs
            ULONG ulSpooled;
            if (ulSpooled = winhQueryPendingSpoolJobs())
            {
                // if we have any, issue a warning message and
                // tell the user to remove print jobs
                CHAR szTemp[20];
                PCSZ pTable[1];
                sprintf(szTemp, "%d", ulSpooled);
                pTable[0] = szTemp;
                cmnMessageBoxExt(HWND_DESKTOP,
                                    114,
                                    pTable,
                                    1,
                                    115,            // tmf file updated V0.9.11 (2001-04-25) [umoeller]
                                    MB_CANCEL);
                // changed this V0.9.11 (2001-04-25) [umoeller]:
                // never allow the user to continue here... we used
                // to have a yesno box here, but apparently continuing
                // here hangs the system, so I changed the message to
                // "please remove print jobs from the spooler".
                fStartShutdown = FALSE;
            }
        }
        */
    }

    StartShutdownThread(fStartShutdown,
                        FALSE,  // fPlayRestartDesktopSound
                        psdp);

    return fStartShutdown;
}

/*
 *@@ xsdInitiateRestartWPS:
 *      pretty similar to xsdInitiateShutdown, i.e. this
 *      will also show a confirmation box and start the Shutdown
 *      thread, except that flags are set differently so that
 *      after closing all windows, no shutdown is performed, but
 *      only the WPS is restarted.
 *
 *@@changed V0.9.0 [umoeller]: global SHUTDOWNPARAMS removed
 *@@changed V0.9.0 [umoeller]: this used to be an XFldDesktop instance method
 *@@changed V0.9.1 (99-12-10) [umoeller]: fixed KERNELGLOBALS locks
 *@@changed V0.9.1 (99-12-12) [umoeller]: fixed memory leak when shutdown was cancelled
 *@@changed V0.9.5 (2000-08-10) [umoeller]: added logoff support
 *@@changed V0.9.7 (2001-01-25) [umoeller]: this played the wrong sound, fixed
 */

BOOL xsdInitiateRestartWPS(BOOL fLogoff)        // in: if TRUE, perform logoff also
{
    BOOL                fStartShutdown = TRUE;
    PSHUTDOWNPARAMS     psdp = (PSHUTDOWNPARAMS)malloc(sizeof(SHUTDOWNPARAMS));

    if (G_ulShutdownState != XSD_IDLE)
        // shutdown thread already running: return!
        fStartShutdown = FALSE;

    // lock shutdown menu items
    G_ulShutdownState = XSD_CONFIRMING;

    if (fStartShutdown)
    {
        ULONG flShutdown = 0;
#ifndef __NOXSHUTDOWN__
        flShutdown = cmnQuerySetting(sflXShutdown);
#endif

        memset(psdp, 0, sizeof(SHUTDOWNPARAMS));
        psdp->optReboot =  FALSE;
        // psdp->ulRestartWPS = (fLogoff) ? 2 : 1; // V0.9.5 (2000-08-10) [umoeller]
        psdp->ulCloseMode = (fLogoff) ? SHUT_LOGOFF : SHUT_RESTARTWPS;
        psdp->optWPSCloseWindows = ((flShutdown & XSD_WPS_CLOSEWINDOWS) != 0);
        psdp->optWPSProcessShutdown
        = psdp->optWPSReuseStartupFolder
        = psdp->optWPSCloseWindows;
#ifndef __EASYSHUTDOWN__
        psdp->optConfirm = (!(flShutdown & XSD_NOCONFIRM));
#else
        psdp->optConfirm = TRUE;
#endif
        psdp->optAutoCloseVIO = ((flShutdown & XSD_AUTOCLOSEVIO) != 0);
#ifndef __EASYSHUTDOWN__
        psdp->optWarpCenterFirst = ((flShutdown & XSD_WARPCENTERFIRST) != 0);
#endif
        psdp->optLog =  ((flShutdown & XSD_LOG) != 0);
        #ifdef __DEBUG__
            psdp->optDebug = doshQueryShiftState();
        #else
            psdp->optDebug = FALSE;
        #endif

        if (psdp->optConfirm)
        {
            ULONG ulReturn = xsdConfirmRestartWPS(psdp);
            if (ulReturn != DID_OK)
                fStartShutdown = FALSE;
        }
    }

    StartShutdownThread(fStartShutdown,
                        TRUE,  // fPlayRestartDesktopSound
                        psdp);

    return fStartShutdown;
}

/*
 *@@ xsdInitiateShutdownExt:
 *      just like the XFldDesktop method, but this one
 *      allows setting all the shutdown parameters by
 *      using the SHUTDOWNPARAMS structure. This is used
 *      for calling XShutdown externally, which is done
 *      by sending T1M_EXTERNALSHUTDOWN to the thread-1
 *      object window (see kernel.c).
 *
 *      NOTE: The memory block pointed to by psdp is
 *      not released by this function.
 *
 *@@changed V0.9.2 (2000-02-28) [umoeller]: fixed KERNELGLOBALS locks
 *@@changed V0.9.7 (2001-01-25) [umoeller]: rearranged for setup strings
 */

BOOL xsdInitiateShutdownExt(PSHUTDOWNPARAMS psdpShared)
{
    BOOL                fStartShutdown = TRUE;
    PSHUTDOWNPARAMS     psdpNew = NULL;

    if (G_ulShutdownState != XSD_IDLE)
        // shutdown thread already running: return!
        fStartShutdown = FALSE;

    // lock shutdown menu items
    G_ulShutdownState = XSD_CONFIRMING;

    if (psdpShared == NULL)
        fStartShutdown = FALSE;

    if (fStartShutdown)
    {
        psdpNew = (PSHUTDOWNPARAMS)malloc(sizeof(SHUTDOWNPARAMS));
        if (!psdpNew)
            fStartShutdown = FALSE;
        else
        {
            memcpy(psdpNew, psdpShared, sizeof(SHUTDOWNPARAMS));

            if (psdpNew->optConfirm)
            {
                // confirmations are on: display proper
                // confirmation dialog
                ULONG ulReturn;
                if (psdpNew->ulCloseMode != SHUT_SHUTDOWN)
                    ulReturn = xsdConfirmRestartWPS(psdpNew);
                else
                    ulReturn = xsdConfirmShutdown(psdpNew);

                if (ulReturn != DID_OK)
                    fStartShutdown = FALSE;
            }
        }
    }

    StartShutdownThread(fStartShutdown,
                        FALSE,  // fPlayRestartDesktopSound
                        psdpNew);

    return fStartShutdown;
}

/* ******************************************************************
 *
 *   Shutdown helper functions
 *
 ********************************************************************/

/*
 *@@ xsdLoadAnimation:
 *      this loads the shutdown (traffic light) animation
 *      as an array of icons from the XFLDR.DLL module.
 *
 *@@changed V1.0.0 (2002-09-13) [umoeller]: optimized
 */

VOID xsdLoadAnimation(PSHUTDOWNANIM psda)
{
    HMODULE hmod = cmnQueryMainResModuleHandle();

    static const USHORT ausPtrIDs[XSD_ANIM_COUNT] =
        {
            ID_ICONSDANIM1,
            ID_ICONSDANIM2,
            ID_ICONSDANIM3,
            ID_ICONSDANIM4,
            ID_ICONSDANIM5,
            ID_ICONSDANIM4,
            ID_ICONSDANIM3,
            ID_ICONSDANIM2
        };

    USHORT us;
    // made this a loop V1.0.0 (2002-09-13) [umoeller]
    for (us = 0;
         us < XSD_ANIM_COUNT;
         us++)
    {
        psda->ahptr[us] = WinLoadPointer(HWND_DESKTOP, hmod, ausPtrIDs[us]);
    }
}

/*
 *@@ xsdFreeAnimation:
 *      this frees the animation loaded by xsdLoadAnimation.
 */

VOID xsdFreeAnimation(PSHUTDOWNANIM psda)
{
    USHORT us;
    for (us = 0;
         us < XSD_ANIM_COUNT;
         us++)
    {
        WinDestroyPointer((psda->ahptr)[us]);
        psda->ahptr[us] = NULLHANDLE;
    }
}

/*
 *@@ xsdRestartWPS:
 *      terminated the WPS process, which will lead
 *      to a Desktop restart.
 *
 *      If (fLogoff == TRUE), this will perform a logoff
 *      as well. If XWPShell is not running, this flag
 *      has no effect.
 *
 *      Runs on the Shutdown thread.
 *
 *@@changed V0.9.5 (2000-08-10) [umoeller]: added XWPSHELL.EXE interface
 */

VOID xsdRestartWPS(HAB hab,
                   BOOL fLogoff)    // in: if TRUE, perform logoff as well.
{
    ULONG ul;

    PCKERNELGLOBALS pcKernelGlobals = krnQueryGlobals();

    // wait a maximum of 2 seconds while there's still
    // a system sound playing
    for (ul = 0; ul < 20; ul++)
        if (xmmIsBusy())
            DosSleep(100);
        else
            break;

    // close leftover open devices
    xmmCleanup();

    if (G_pXWPShellShared)
    {
        // XWPSHELL.EXE running:
        // set flag in shared memory; XWPSHELL
        // will check this once the WPS has terminated
        G_pXWPShellShared->fNoLogonButRestart = !fLogoff;
    }

    // terminate the current process,
    // which is PMSHELL.EXE. We shouldn't use DosExit()
    // directly, because this might mess up the
    // C runtime library... even though this doesn't
    // help much with the rest of the WPS.
    exit(0);        // 0 == no error
}

/*
 *@@ xsdFlushWPS2INI:
 *      this forces the WPS to flush its internal buffers
 *      into OS2.INI/OS2SYS.INI. We call this function
 *      after we have closed all the WPS windows, before
 *      we actually save the INI files.
 *
 *      This undocumented semaphore was published in some
 *      newsgroup ages ago, I don't remember.
 *
 *      Returns APIRETs of event semaphore calls.
 *
 *@@added V0.9.0 (99-10-22) [umoeller]
 */

APIRET xsdFlushWPS2INI(VOID)
{
    APIRET arc  = 0;
    HEV hev = NULLHANDLE;

    if (!(arc = DosOpenEventSem("\\SEM32\\WORKPLAC\\LAZYWRIT.SEM", &hev)))
    {
        arc = DosPostEventSem(hev);
        DosCloseEventSem(hev);
    }

    return arc;
}

/* ******************************************************************
 *
 *   Additional declarations for Shutdown thread
 *
 ********************************************************************/

/*
 *@@ SHUTDOWNDATA:
 *      shutdown instance data allocated from the heap
 *      while the shutdown thread (fntShutdown) is
 *      running.
 *
 *      This replaces the sick set of global variables
 *      which used to be all over the place before V0.9.9
 *      and fixes a number of serialization problems on
 *      the way.
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 */

typedef struct _SHUTDOWNDATA
{
    // shutdown parameters
    SHUTDOWNPARAMS  sdParams;

    ULONG           ulMaxItemCount,
                    ulLastItemCount;

    PXFILE          ShutdownLogFile;        // changed V0.9.16 (2001-11-22) [umoeller]

    HAB             habShutdownThread;

    HMODULE         hmodResource;

    HWND            hwndProgressBar;        // progress bar in status window

    // flags for whether we're currently owning semaphores
    BOOL            fShutdownSemOwned,
                    fSkippedSemOwned;

    ULONG           hPOC;

    // this is the global list of items to be closed (SHUTLISTITEMs)
    LINKLIST        llShutdown,
    // and the list of items that are to be skipped
                    llSkipped;

    HMTX            hmtxShutdown,
                    hmtxSkipped;
    HEV             hevUpdated;

    // temporary storage for closing VIOs
    CHAR            szVioTitle[1000];
    SHUTLISTITEM    VioItem;

    // global linked list of auto-close VIO windows (AUTOCLOSELISTITEM)
    LINKLIST        llAutoClose;

    ULONG           sidWPS,
                    sidPM;

    SHUTDOWNCONSTS  SDConsts;

} SHUTDOWNDATA, *PSHUTDOWNDATA;

VOID xsdFinishShutdown(PSHUTDOWNDATA pShutdownData);
VOID xsdFinishStandardMessage(PSHUTDOWNDATA pShutdownData);
VOID xsdFinishStandardReboot(PSHUTDOWNDATA pShutdownData);
VOID xsdFinishUserReboot(PSHUTDOWNDATA pShutdownData);
VOID xsdFinishPowerOff(PSHUTDOWNDATA pShutdownData);

/* ******************************************************************
 *
 *   XShutdown data maintenance
 *
 ********************************************************************/

/*
 *@@ xsdGetShutdownConsts:
 *      prepares a number of constants in the specified
 *      SHUTDOWNCONSTS structure which are used throughout
 *      XShutdown.
 *
 *      SHUTDOWNCONSTS is part of SHUTDOWNDATA, so this
 *      func gets called once when fntShutdownThread starts
 *      up. However, since this is also used externally,
 *      we have put these fields into a separate structure.
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 */

VOID xsdGetShutdownConsts(PSHUTDOWNCONSTS pConsts)
{
    PXWPUSERDBENTRY puiLocal;

    pConsts->pKernelGlobals = krnQueryGlobals();
    pConsts->pWPDesktop = _WPDesktop;
    pConsts->pActiveDesktop = _wpclsQueryActiveDesktop(pConsts->pWPDesktop);
    pConsts->hwndActiveDesktop = _wpclsQueryActiveDesktopHWND(pConsts->pWPDesktop);
    pConsts->hwndOpenWarpCenter = NULLHANDLE;

    WinQueryWindowProcess(pConsts->hwndActiveDesktop,
                          &pConsts->pidWPS,
                          NULL);
    WinQueryWindowProcess(HWND_DESKTOP,
                          &pConsts->pidPM,
                          NULL);

    if (G_pAwakeWarpCenter)     // global variable (xfobj.c, kernel.h) V0.9.20 (2002-07-25) [umoeller]
    {
        // WarpCenter is awake: check if it's open
        PUSEITEM pUseItem;
        for (pUseItem = _wpFindUseItem(G_pAwakeWarpCenter,
                                       USAGE_OPENVIEW,
                                       NULL);
             pUseItem;
             pUseItem = _wpFindUseItem(G_pAwakeWarpCenter,
                                       USAGE_OPENVIEW,
                                       pUseItem))
        {
            PVIEWITEM pViewItem = (PVIEWITEM)(pUseItem+1);
            if (pViewItem->view == OPEN_RUNNING)
            {
                pConsts->hwndOpenWarpCenter = pViewItem->handle;
                break;
            }
        }
    }

    // get uid and gid of currently logged on user, if xwpshell
    // is running V0.9.19 (2002-04-02) [umoeller]
    if (!xsecQueryLocalUser(&puiLocal))
    {
        // running:
        // store uid
        pConsts->uid = puiLocal->User.uid;
        free(puiLocal);
    }
    else
        pConsts->uid = -1;
}

/*
 *@@ xsdItemFromPID:
 *      searches a given LINKLIST of SHUTLISTITEMs
 *      for a process ID.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist.c functions
 */

PSHUTLISTITEM xsdItemFromPID(PLINKLIST pList,
                             PID pid,
                             HMTX hmtx)
{
    PSHUTLISTITEM   pItem = NULL;
    BOOL            fAccess = FALSE,
                    fLocked = FALSE;

    TRY_QUIET(excpt1)
    {
        if (hmtx)
        {
            fLocked = !DosRequestMutexSem(hmtx, SEM_INDEFINITE_WAIT);
            fAccess = fLocked;
        }
        else
            fAccess = TRUE;

        if (fAccess)
        {
            PLISTNODE pNode = lstQueryFirstNode(pList);
            while (pNode)
            {
                pItem = pNode->pItemData;
                if (pItem->swctl.idProcess == pid)
                    break;

                pNode = pNode->pNext;
                pItem = 0;
            }
        }
    }
    CATCH(excpt1) { } END_CATCH();

    if (fLocked)
        DosReleaseMutexSem(hmtx);

    return pItem;
}

/*
 *@@ xsdItemFromSID:
 *      searches a given LINKLIST of SHUTLISTITEMs
 *      for a session ID.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist.c functions
 */

PSHUTLISTITEM xsdItemFromSID(PLINKLIST pList,
                             ULONG sid,
                             HMTX hmtx,
                             ULONG ulTimeout)
{
    PSHUTLISTITEM pItem = NULL;
    BOOL          fAccess = FALSE,
                  fLocked = FALSE;

    TRY_QUIET(excpt1)
    {
        if (hmtx)
        {
            fLocked = !DosRequestMutexSem(hmtx, ulTimeout);
            fAccess = fLocked;
        }
        else
            fAccess = TRUE;

        if (fAccess)
        {
            PLISTNODE pNode = lstQueryFirstNode(pList);
            while (pNode)
            {
                pItem = pNode->pItemData;
                if (pItem->swctl.idSession == sid)
                    break;

                pNode = pNode->pNext;
                pItem = 0;
            }
        }
    }
    CATCH(excpt1) { } END_CATCH();

    if (fLocked)
        DosReleaseMutexSem(hmtx);

    return pItem;
}

/*
 *@@ xsdCountRemainingItems:
 *      counts the items left to be closed by counting
 *      the window list items and subtracting the items
 *      which were skipped by the user.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist.c functions
 *@@changed V0.9.20 (2002-07-22) [umoeller]: fixed mutex release order
 */

ULONG xsdCountRemainingItems(PSHUTDOWNDATA pData)
{
    ULONG   ulrc = 0;
    BOOL    fShutdownSemOwned = FALSE,
            fSkippedSemOwned = FALSE;

    TRY_QUIET(excpt1)
    {
        if (    (fShutdownSemOwned = !DosRequestMutexSem(pData->hmtxShutdown, 4000))
             && (fSkippedSemOwned = !DosRequestMutexSem(pData->hmtxSkipped, 4000))
           )
            ulrc = (
                        lstCountItems(&pData->llShutdown)
                      - lstCountItems(&pData->llSkipped)
                   );
    }
    CATCH(excpt1) { } END_CATCH();

    // fixed release order V0.9.20 (2002-07-22) [umoeller]
    if (fSkippedSemOwned)
        DosReleaseMutexSem(pData->hmtxSkipped);

    if (fShutdownSemOwned)
        DosReleaseMutexSem(pData->hmtxShutdown);

    return ulrc;
}

/*
 *@@ xsdLongTitle:
 *      creates a descriptive string in pszTitle from pItem
 *      (for the main (debug) window listbox).
 */

void xsdLongTitle(PSZ pszTitle,
                  PSHUTLISTITEM pItem)
{
    sprintf(pszTitle, "%s%s",
            pItem->swctl.szSwtitle,
            (pItem->swctl.uchVisibility == SWL_VISIBLE)
                ? (", visible")
                : ("")
        );

    strcat(pszTitle, ", ");
    switch (pItem->swctl.bProgType)
    {
        case PROG_DEFAULT: strcat(pszTitle, "default"); break;
        case PROG_FULLSCREEN: strcat(pszTitle, "OS/2 FS"); break;
        case PROG_WINDOWABLEVIO: strcat(pszTitle, "OS/2 win"); break;
        case PROG_PM:
            strcat(pszTitle, "PM, class: ");
            strcat(pszTitle, pItem->szClass);
        break;
        case PROG_VDM: strcat(pszTitle, "VDM"); break;
        case PROG_WINDOWEDVDM: strcat(pszTitle, "VDM win"); break;
        default:
            sprintf(pszTitle+strlen(pszTitle), "? (%lX)");
        break;
    }
    sprintf(pszTitle+strlen(pszTitle),
            ", hwnd: 0x%lX, pid: 0x%lX, sid: 0x%lX, pObj: 0x%lX",
            (ULONG)pItem->swctl.hwnd,
            (ULONG)pItem->swctl.idProcess,
            (ULONG)pItem->swctl.idSession,
            (ULONG)pItem->pObject
        );
}

/*
 *@@ xsdQueryCurrentItem:
 *      returns the next PSHUTLISTITEM to be
 *      closed (skipping the items that were
 *      marked to be skipped).
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist.c functions
 *@@changed V0.9.20 (2002-07-22) [umoeller]: fixed mutex release order
 */

PSHUTLISTITEM xsdQueryCurrentItem(PSHUTDOWNDATA pData)
{
    CHAR            szShutItem[1000],
                    szSkipItem[1000];
    BOOL            fShutdownSemOwned = FALSE,
                    fSkippedSemOwned = FALSE;
    PSHUTLISTITEM   pliShutItem = 0;

    TRY_QUIET(excpt1)
    {
        if (    (fShutdownSemOwned = !DosRequestMutexSem(pData->hmtxShutdown, 4000))
             && (fSkippedSemOwned = !DosRequestMutexSem(pData->hmtxSkipped, 4000))
           )
        {
            PLISTNODE pShutNode = lstQueryFirstNode(&pData->llShutdown);
            // pliShutItem = pliShutdownFirst;
            while (pShutNode)
            {
                PLISTNODE pSkipNode = lstQueryFirstNode(&pData->llSkipped);
                pliShutItem = pShutNode->pItemData;

                while (pSkipNode)
                {
                    PSHUTLISTITEM pliSkipItem = pSkipNode->pItemData;
                    xsdLongTitle(szShutItem, pliShutItem);
                    xsdLongTitle(szSkipItem, pliSkipItem);
                    if (!strcmp(szShutItem, szSkipItem))
                        /* current shut item is on skip list:
                           break (==> take next shut item */
                        break;

                    pSkipNode = pSkipNode->pNext;
                    pliSkipItem = 0;
                }

                if (pSkipNode == NULL)
                    // current item is not on the skip list:
                    // return this item
                    break;

                // current item was skipped: take next one
                pShutNode = pShutNode->pNext;
                pliShutItem = 0;
            }
        }
    }
    CATCH(excpt1) { } END_CATCH();

    // fixed release order V0.9.20 (2002-07-22) [umoeller]
    if (fSkippedSemOwned)
        DosReleaseMutexSem(pData->hmtxSkipped);

    if (fShutdownSemOwned)
        DosReleaseMutexSem(pData->hmtxShutdown);

    return pliShutItem;
}

/*
 *@@ xsdAppendShutListItem:
 *      this appends a new PSHUTLISTITEM to the given list
 *      and returns the address of the new item; the list
 *      to append to must be specified in *ppFirst / *ppLast.
 *
 *      NOTE: It is entirely the job of the caller to serialize
 *      access to the list, using mutex semaphores.
 *      The item to add is to be specified by swctl and possibly
 *      *pObject (if swctl describes an open Desktop object).
 *
 *      Since this gets called from xsdBuildShutList, this runs
 *      on both the Shutdown and Update threads.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist.c functions
 *@@changed V0.9.4 (2000-07-11) [umoeller]: fixed bug in window class detection
 *@@changed V0.9.6 (2000-10-27) [umoeller]: fixed WarpCenter detection
 *@@changed V1.0.1 (2002-12-15) [umoeller]: now closing workareas first @@fixes 273
 */

PSHUTLISTITEM xsdAppendShutListItem(PSHUTDOWNDATA pShutdownData,
                                    PLINKLIST pList,    // in/out: linked list to work on
                                    SWCNTRL* pswctl,    // in: tasklist entry to add
                                    WPObject *pObject,  // in: !=NULL: Desktop object
                                    LONG lSpecial)
{
    PSHUTLISTITEM   pNewItem;

    if (pNewItem = malloc(sizeof(SHUTLISTITEM)))
    {
        BOOL    fToFront = FALSE;

        pNewItem->pObject = pObject;
        memcpy(&pNewItem->swctl, pswctl, sizeof(SWCNTRL));

        strcpy(pNewItem->szClass, "unknown");

        pNewItem->lSpecial = lSpecial;

        if (pObject)
        {
            // for Desktop objects, store additional data
            if (wpshCheckObject(pObject))
            {
                PUSEITEM    pui;

                strncpy(pNewItem->szClass,
                        (PSZ)_somGetClassName(pObject),
                        sizeof(pNewItem->szClass)-1);

                pNewItem->swctl.szSwtitle[0] = '\0';
                strncpy(pNewItem->swctl.szSwtitle,
                        _wpQueryTitle(pObject),
                        sizeof(pNewItem->swctl.szSwtitle)-1);

                // always set PID and SID to that of the WPS,
                // because the tasklist returns garbage for
                // Desktop objects
                pNewItem->swctl.idProcess = pShutdownData->SDConsts.pidWPS;
                pNewItem->swctl.idSession = pShutdownData->sidWPS;

                // set HWND to object-in-use-list data,
                // because the tasklist also returns garbage
                // for that
                if (pui = _wpFindUseItem(pObject, USAGE_OPENVIEW, NULL))
                {
                    PVIEWITEM   pvi = (PVIEWITEM)(pui + 1);
                    pNewItem->swctl.hwnd = pvi->handle;
                }

                // close workareas first
                // V1.0.1 (2002-12-15) [umoeller]
                if (    (objIsAFolder(pObject))
                     && (_wpQueryFldrFlags(pObject) & FOI_WORKAREA)
                   )
                    fToFront = TRUE;
            }
            else
            {
                // invalid object:
                pNewItem->pObject = NULL;
                strcpy(pNewItem->szClass, "wpshCheckObject failed");
            }
        }
        else
        {
            // no Desktop object: get window class name
            WinQueryClassName(pswctl->hwnd,               // old V0.9.3 code
                              sizeof(pNewItem->szClass)-1,
                              pNewItem->szClass);
        }

        // append to list
        if (fToFront)
            // to front     V1.0.1 (2002-12-15) [umoeller]
            lstInsertItemBefore(pList,
                                pNewItem,
                                0);
        else
            // to tail
            lstAppendItem(pList, pNewItem);
    }

    return pNewItem;
}

/*
 *@@ xsdIsClosable:
 *      examines the given switch list entry and returns
 *      an XSD_* constant telling the caller what that
 *      item represents.
 *
 *      While we're at it, we also change some of the
 *      data in the switch list entry if needed.
 *
 *      If the switch list entry represents a Desktop object,
 *      *ppObject is set to that object's SOM pointer.
 *      Otherwise *ppObject receives NULL.
 *
 *      This returns:
 *      --  XSD_SYSTEM (-1)
 *
 *      --  XSD_INVISIBLE (-2)
 *
 *      --  XSD_DEBUGNEED (-3)
 *
 *      --  XSD_DESKTOP (-4) for the WPS desktop, which
 *          needs special handling anyway since it must
 *          be closed last;
 *
 *      --  XSD_WARPCENTER (-5)
 *
 *      --  XSD_WPSOBJECT_CLOSE (0) for any WPS object to
 *          be closed; for those, *ppObject is set to
 *          the SOM object pointer. These objects must
 *          be closed with all shutdown modes.
 *
 *      --  XSD_OTHER_OWNED (1): the program is owned by
 *          the current user, but is not a WPS object.
 *          Close this with shutdown and logoff, but
 *          with "Restart WPS", close it only if the
 *          user wants to have all sessions closed.
 *
 *      --  XSD_OTHER_FOREIGN (2): the program is not a
 *          WPS object, and it's not even owned by the
 *          current user. Close this with shutdown only;
 *          never close this with "restart desktop" or
 *          "logoff".
 *
 *      In other words, if this returns something >= 0,
 *      the object might have to be closed depending on
 *      the mode.
 *
 *@@added V0.9.4 (2000-07-15) [umoeller]
 *@@changed V0.9.6 (2000-10-27) [umoeller]: fixed WarpCenter detection
 *@@changed V0.9.19 (2002-04-02) [umoeller]: refined detection for the different modes
 */

LONG xsdIsClosable(HAB hab,                 // in: caller's anchor block
                   PSHUTDOWNCONSTS pConsts,
                   SWENTRY *pSwEntry,       // in/out: switch entry
                   WPObject **ppObject,     // out: the WPObject*, really, or NULL if the window is no object
                   XWPSECID *puidOwner)     // out: user ID on whose behalf the user is running or sth negative for errors
{
    LONG           lrc = 0;
    CHAR           szSwUpperTitle[100];

    *ppObject = NULL;

    strcpy(szSwUpperTitle,
           pSwEntry->swctl.szSwtitle);
    WinUpper(hab, 0, 0, szSwUpperTitle);

    if (    // skip if PID == 0:
            (pSwEntry->swctl.idProcess == 0)
            // skip the Shutdown windows:
         || (pSwEntry->swctl.hwnd == pConsts->hwndMain)
         || (pSwEntry->swctl.hwnd == pConsts->hwndVioDlg)
         || (pSwEntry->swctl.hwnd == pConsts->hwndShutdownStatus)
       )
        return XSD_SYSTEM;
    // skip invisible tasklist entries; this
    // includes a PMWORKPLACE cmd.exe:
    else if (pSwEntry->swctl.uchVisibility != SWL_VISIBLE)
        return XSD_INVISIBLE;
    // open WarpCenter (WarpCenter bar only):
    else if (   (pSwEntry->swctl.hwnd == pConsts->hwndOpenWarpCenter)
             && (pConsts->pKernelGlobals)
            )
    {
        *ppObject = G_pAwakeWarpCenter;     // global variable (xfobj.c, kernel.h) V0.9.20 (2002-07-25) [umoeller]
        return XSD_WARPCENTER;
    }
#ifdef __DEBUG__
    // if we're in debug mode, skip the PMPRINTF window
    // because we want to see debug output
    else if (!strncmp(szSwUpperTitle, "PMPRINTF", 8))
        return XSD_DEBUGNEED;
    // skip VAC debugger, which is probably debugging
    // PMSHELL.EXE
    else if (!strcmp(szSwUpperTitle, "ICSDEBUG.EXE"))
        return XSD_DEBUGNEED;
#endif

    // now fix the data in the switch list entries,
    // if necessary
    if (pSwEntry->swctl.bProgType == PROG_DEFAULT)
    {
        // in this case, we need to find out what
        // type the program really has
        PQPROCSTAT16 pps;
        if (!prc16GetInfo(&pps))
        {
            PRCPROCESS prcp;
            // default for errors
            pSwEntry->swctl.bProgType = PROG_WINDOWABLEVIO;
            if (prc16QueryProcessInfo(pps, pSwEntry->swctl.idProcess, &prcp))
                // according to bsedos.h, the PROG_* types are identical
                // to the SSF_TYPE_* types, so we can use the data from
                // DosQProcStat
                pSwEntry->swctl.bProgType = prcp.ulSessionType;
            prc16FreeInfo(pps);
        }
    }

    if (pSwEntry->swctl.bProgType == PROG_WINDOWEDVDM)
        // DOS/Win-OS/2 window: get real PID/SID, because
        // the tasklist contains false data
        WinQueryWindowProcess(pSwEntry->swctl.hwnd,
                              &pSwEntry->swctl.idProcess,
                              &pSwEntry->swctl.idSession);

    if (pSwEntry->swctl.idProcess == pConsts->pidWPS)
    {
        // is Desktop window?
        if (pSwEntry->swctl.hwnd == pConsts->hwndActiveDesktop)
        {
            *ppObject = pConsts->pActiveDesktop;
            lrc = XSD_DESKTOP;
        }
        else
        {
            // PID == Workplace Shell PID: get SOM pointer from hwnd
            *ppObject = _wpclsQueryObjectFromFrame(pConsts->pWPDesktop, // _WPDesktop
                                                   pSwEntry->swctl.hwnd);

            if (*ppObject == pConsts->pActiveDesktop)
                lrc = XSD_DESKTOP;
        }
    }

    // if this is not a WPS object, mark it so caller can decide whether to close it
    if (!*ppObject)
        lrc = XSD_OTHER_OWNED;

    if (pConsts->uid == -1)
        // XWPShell not running:
        *puidOwner = -2;
    else
    {
        APIRET arc;
        // XWPShell running:
        if (*ppObject)
            // WPS object: use WPS pid (save time)
            *puidOwner = pConsts->uid;
        else
            if (arc = xsecQueryProcessOwner(pSwEntry->swctl.idProcess,
                                            puidOwner))
            {
                // error:
                *puidOwner = -(LONG)arc;
                lrc = XSD_OTHER_FOREIGN;
            }
            else
                if (*puidOwner != pConsts->uid)
                    lrc = XSD_OTHER_FOREIGN;
    }

    return lrc;
}

/*
 *@@ xsdBuildShutList:
 *      this routine builds a new ShutList by evaluating the
 *      system task list; this list is built in pList.
 *      NOTE: It is entirely the job of the caller to serialize
 *      access to the list, using mutex semaphores.
 *      We call xsdAppendShutListItem for each task list entry,
 *      if that entry is to be closed, so we're doing a few checks.
 *
 *      This gets called from both xsdUpdateListBox (on the
 *      Shutdown thread) and the Update thread (fntUpdateThread)
 *      directly.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist.c functions
 *@@changed V0.9.0 [umoeller]: PSHUTDOWNPARAMS added to prototype
 *@@changed V0.9.4 (2000-07-15) [umoeller]: PSHUTDOWNCONSTS added to prototype
 *@@changed V0.9.4 (2000-07-15) [umoeller]: extracted xsdIsClosable; fixed WarpCenter detection
 *@@changed V1.0.1 (2003-01-05) [umoeller]: now respecting "close all windows" with logoff too
 */

void xsdBuildShutList(HAB hab,
                      PSHUTDOWNDATA pShutdownData,
                      PLINKLIST pList)
{
    PSWBLOCK        pSwBlock   = NULL;         // Pointer to information returned
    ULONG           ul;
    WPObject        *pObj;
    BOOL            Append;

    // get all the tasklist entries into a buffer
    pSwBlock = winhQuerySwitchList(hab);

    // loop through all the tasklist entries
    for (ul = 0;
         ul < pSwBlock->cswentry;
         ul++)
    {
        // now we check which windows we add to the shutdown list
        XWPSECID uid;
        LONG lrc = xsdIsClosable(hab,
                                 &pShutdownData->SDConsts,
                                 &pSwBlock->aswentry[ul],
                                 &pObj,
                                 &uid);
        if (lrc >= 0)
        {
            // generally closeable:
            BOOL fSkip = FALSE;

            if (lrc == XSD_OTHER_OWNED)
                // the program is owned by the current user, but is not a WPS object.
                // Close this with shutdown and logoff, but
                // with "Restart WPS", close it only if the
                // user wants to have all sessions closed.

                // Hmm, changed that. Added "close all windows" checkbox
                // back to logoff confirmation window, so we should rather
                // respect that setting. V1.0.1 (2003-01-05) [umoeller]
                if (    (    (pShutdownData->sdParams.ulCloseMode == SHUT_RESTARTWPS)
                          || (pShutdownData->sdParams.ulCloseMode == SHUT_LOGOFF)
                        )
                     && (!pShutdownData->sdParams.optWPSCloseWindows)
                   )
                    fSkip = TRUE;

            if (lrc == XSD_OTHER_FOREIGN)
                // the program is not a
                // WPS object, and it's not even owned by the
                // current user. Close this with shutdown only;
                // never close this with "restart desktop" or
                // "logoff".
                if (pShutdownData->sdParams.ulCloseMode != SHUT_SHUTDOWN)
                    fSkip = TRUE;

            if (!fSkip)
                xsdAppendShutListItem(pShutdownData,
                                      pList,
                                      &pSwBlock->aswentry[ul].swctl,
                                      pObj,
                                      lrc);
        }
    }

    free(pSwBlock);
}

/*
 *@@ xsdUpdateListBox:
 *      this routine builds a new PSHUTITEM list from the
 *      pointer to the pointer of the first item (*ppliShutdownFirst)
 *      by setting its value to xsdBuildShutList's return value;
 *      it also fills the listbox in the "main" window, which
 *      is only visible in Debug mode.
 *      But even if it's invisible, the listbox is used for closing
 *      windows. Ugly, but nobody can see it. ;-)
 *      If *ppliShutdownFirst is != NULL the old shutlist is cleared
 *      also.
 *
 *      Runs on the Shutdown thread.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist.c functions
 *@@changed V0.9.0 [umoeller]: PSHUTDOWNPARAMS added to prototype
 *@@changed V0.9.4 (2000-07-15) [umoeller]: PSHUTDOWNCONSTS added to prototype
 */

void xsdUpdateListBox(HAB hab,
                      PSHUTDOWNDATA pShutdownData,
                      HWND hwndListbox)
{
    PSHUTLISTITEM   pItem;
    CHAR            szTitle[1024];

    BOOL            fLocked = FALSE;

    TRY_QUIET(excpt1)
    {
        if (fLocked = !DosRequestMutexSem(pShutdownData->hmtxShutdown, 4000))
        {
            PLISTNODE pNode = 0;
            lstClear(&pShutdownData->llShutdown);
            xsdBuildShutList(hab,
                             pShutdownData,
                             &pShutdownData->llShutdown);

            // clear list box
            WinEnableWindowUpdate(hwndListbox, FALSE);
            WinSendMsg(hwndListbox, LM_DELETEALL, MPNULL, MPNULL);

            // and insert all list items as strings
            pNode = lstQueryFirstNode(&pShutdownData->llShutdown);
            while (pNode)
            {
                pItem = pNode->pItemData;
                xsdLongTitle(szTitle, pItem);
                WinInsertLboxItem(hwndListbox, 0, szTitle);
                pNode = pNode->pNext;
            }
            WinEnableWindowUpdate(hwndListbox, TRUE);
        }
    }
    CATCH(excpt1) { } END_CATCH();

    if (fLocked)
        DosReleaseMutexSem(pShutdownData->hmtxShutdown);
}

/*
 *@@ xsdUpdateClosingStatus:
 *      this gets called from fnwpShutdownThread to
 *      set the Shutdown status wnd text to "Closing xxx".
 *
 *      Runs on the Shutdown thread.
 *
 *@@changed V1.0.1 (2003-01-30) [umoeller]: optimized, fixed buf overflows
 */

VOID xsdUpdateClosingStatus(HWND hwndShutdownStatus,
                            PCSZ pcszProgTitle)   // in: window title from SHUTLISTITEM
{
    XSTRING str;

    xstrInitCopy(&str, cmnGetString(ID_SDSI_CLOSING), 100);
    xstrcat(&str, " \"", 2);
    xstrcat(&str, pcszProgTitle, 0);
    xstrcat(&str, "\"...", 4);
    WinSetDlgItemText(hwndShutdownStatus, ID_SDDI_STATUS,
                      str.psz);
    xstrClear(&str);

    WinSetActiveWindow(HWND_DESKTOP, hwndShutdownStatus);
}

/*
 *@@ xsdWaitForExceptions:
 *      checks for whether helpers/except.c is currently
 *      busy processing an exception and, if so, waits
 *      until that thread is done.
 *
 *      Gets called several times during fntShutdownThread
 *      because we don't want to lose the trap logs.
 *
 *@@added V0.9.13 (2001-06-19) [umoeller]
 *@@changed V1.0.9 (2011-05-08) [pr]: added timeout
 */

VOID xsdWaitForExceptions(PSHUTDOWNDATA pShutdownData)
{
    // check the global variable exported from except.h,
    // which is > 0 if some exception is currently running

    if (G_ulExplainExceptionRunning)
    {
        ULONG ulSlept = 1;

        while (G_ulExplainExceptionRunning)
        {
            CHAR szTemp[500];
            sprintf(szTemp,
                    "Urgh, %d exception(s) running, waiting... (%d)",
                    G_ulExplainExceptionRunning,
                    ulSlept++);
            WinSetDlgItemText(pShutdownData->SDConsts.hwndShutdownStatus,
                              ID_SDDI_STATUS,
                              szTemp);

            // wait half a second
            winhSleep(500);
            if (ulSlept > 10)  // V1.0.9
                break;
        }

        WinSetDlgItemText(pShutdownData->SDConsts.hwndShutdownStatus,
                          ID_SDDI_STATUS,
                          "OK");
    }
}

/*
 *@@ fncbSaveImmediate:
 *      callback for objForAllDirtyObjects to save
 *      the WPS.
 *
 *@@added V0.9.9 (2001-04-04) [umoeller]
 *@@changed V0.9.16 (2001-12-06) [umoeller]: now skipping saving objects from foreign desktops
 */

BOOL _Optlink fncbSaveImmediate(WPObject *pobjThis,
                                ULONG ulIndex,
                                ULONG cObjects,
                                PVOID pvUser)
{
    BOOL    brc = FALSE;
    PSHUTDOWNDATA pShutdownData = (PSHUTDOWNDATA)pvUser;

    // update progress bar
    WinSendMsg(pShutdownData->hwndProgressBar,
               WM_UPDATEPROGRESSBAR,
               (MPARAM)ulIndex,
               (MPARAM)cObjects);

    TRY_QUIET(excpt1)
    {
        if (pobjThis == pShutdownData->SDConsts.pActiveDesktop)
                        // we already saved the desktop, so skip this
                        // V0.9.16 (2001-10-25) [umoeller]
            brc = TRUE;
        else if (cmnIsObjectFromForeignDesktop(pobjThis))
                        // never save objects which belong to
                        // a foreign desktop
                        // V0.9.16 (2001-12-06) [umoeller]
        {
            /* CHAR szFolderPath[CCHMAXPATH];
            _wpQueryFilename(_wpQueryFolder(pobjThis),
                             szFolderPath,
                             TRUE);
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "skipping save of object %s (class: %s) in folder %s",
                   _wpQueryTitle(pobjThis),
                   _somGetClassName(pobjThis),
                   szFolderPath);
            */
            brc = TRUE;
        }
        else
        {
            brc = _wpSaveImmediate(pobjThis);

            PMPF_SHUTDOWN(("saved obj 0x%lX (%s, class %s)",
                                pobjThis,
                                _wpQueryTitle(pobjThis),
                                _somGetClassName(pobjThis)));
        }
    }
    CATCH(excpt1)
    {
        brc = FALSE;
    } END_CATCH();

    return brc;
}

/* ******************************************************************
 *
 *   Shutdown thread
 *
 ********************************************************************/

/*
 *@@ fntShutdownThread:
 *      this is the main shutdown thread which is created by
 *      xsdInitiateShutdown / xsdInitiateRestartWPS when shutdown is about
 *      to begin.
 *
 *      Parameters: this thread must be created using thrCreate
 *      (src/helpers/threads.c), so it is passed a pointer
 *      to a THREADINFO structure. In that structure you
 *      must set ulData to point to a SHUTDOWNPARAMS structure.
 *
 *      Note: if you're trying to understand what's going on here,
 *      I recommend rebuilding XWorkplace in debug mode. That will
 *      allow you to switch XShutdown into "Debug" mode by holding down
 *      the "Shift" key while selecting "Shutdown" from the
 *      Desktop's context menu.
 *
 *      Shutdown / Restart Desktop runs in the following phases:
 *
 *      1)  First, all necessary preparations are done, i.e. two
 *          windows are created (the status window with the progress
 *          bar and the "main" window, which is only visible in debug
 *          mode, but processes all the messages). These two windows
 *          daringly share the same msg proc (fnwpShutdownThread below),
 *          but receive different messages, so this shan't hurt.
 *
 *          After these windows have been created, fntShutdown will also
 *          create the Update thread (fntUpdateThread below).
 *          This Update thread is responsible for monitoring the
 *          task list; every time an item is closed (or even opened!),
 *          it will post a ID_SDMI_UPDATESHUTLIST command to fnwpShutdownThread,
 *          which will then start working again.
 *
 *      2)  fntShutdownThread then remains in a standard PM message
 *          loop until shutdown is cancelled by the user or all
 *          windows have been closed.
 *          In both cases, fnwpShutdownThread posts a WM_QUIT then.
 *
 *          The order of msg processing in fntShutdownThread / fnwpShutdownThread
 *          is the following:
 *
 *          a)  ID_SDMI_UPDATESHUTLIST will update the list of currently
 *              open windows (which is not touched by any other thread)
 *              by calling xsdUpdateListBox.
 *
 *              Unless we're in debug mode (where shutdown has to be
 *              started manually), the first time ID_SDMI_UPDATESHUTLIST
 *              is received, we will post ID_SDDI_BEGINSHUTDOWN (go to c)).
 *              Otherwise (subsequent calls), we post ID_SDMI_CLOSEITEM
 *              (go to d)).
 *
 *          b)  ID_SDDI_BEGINSHUTDOWN will begin processing the contents
 *              of the Shutdown folder and empty the trash can. After this
 *              is done, ID_SDMI_BEGINCLOSINGITEMS is posted.
 *
 *          c)  ID_SDMI_BEGINCLOSINGITEMS will prepare closing all windows
 *              by setting flagClosingItems to TRUE and then post the
 *              first ID_SDMI_CLOSEITEM.
 *
 *          d)  ID_SDMI_CLOSEITEM will now undertake the necessary
 *              actions for closing the first / next item on the list
 *              of items to close, that is, post WM_CLOSE to the window
 *              or kill the process or whatever.
 *              If no more items are left to close, we post
 *              ID_SDMI_PREPARESAVEWPS (go to g)).
 *              Otherwise, after this, the Shutdown thread is idle.
 *
 *          e)  When the window has actually closed, the Update thread
 *              realizes this because the task list will have changed.
 *              The next ID_SDMI_UPDATESHUTLIST will be posted by the
 *              Update thread then. Go back to b).
 *
 *          f)  ID_SDMI_PREPARESAVEWPS will save the state of all currently
 *              awake Desktop objects by using the list which was maintained
 *              by the Worker thread all the while during the whole WPS session.
 *
 *          g)  After this, ID_SDMI_FLUSHBUFFERS is posted, which will
 *              set fAllWindowsClosed to TRUE and post WM_QUIT, so that
 *              the PM message loop in fntShutdownThread will exit.
 *
 *      3)  Depending on whether fnwpShutdownThread set fAllWindowsClosed to
 *          TRUE, we will then actually restart the WPS or shut down the system
 *          or exit this thread (= shutdown cancelled), and the user may continue work.
 *
 *          Shutting down the system is done by calling xsdFinishShutdown,
 *          which will differentiate what needs to be done depending on
 *          what the user wants (new with V0.84).
 *          We will then either reboot the machine or run in an endless
 *          loop, if no reboot was desired, or call the functions for an
 *          APM 1.2 or ACPI power-off in apm.c (V0.82).
 *
 *          When shutdown was cancelled by pressing the respective button,
 *          the Update thread is killed, all shutdown windows are closed,
 *          and then this thread also terminates.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist.c functions
 *@@changed V0.9.0 [umoeller]: changed shutdown logging to stdio functions (fprintf etc.)
 *@@changed V0.9.0 [umoeller]: code has been re-ordered for semaphore safety.
 *@@changed V0.9.1 (99-12-10) [umoeller]: extracted auto-close list code to xsdLoadAutoCloseItems
 *@@changed V0.9.9 (2001-04-04) [umoeller]: moved all post-close stuff from fnwpShutdownThread here
 *@@changed V0.9.9 (2001-04-04) [umoeller]: rewrote "save Desktop objects" to use dirty list from object.c
 *@@changed V0.9.11 (2001-04-18) [umoeller]: fixed logoff
 *@@changed V0.9.12 (2001-04-29) [umoeller]: deferred update thread startup to fnwpShutdownThread; this fixes shutdown folder
 *@@changed V0.9.12 (2001-05-15) [umoeller]: now telling XPager to recover windows first
 *@@changed V0.9.12 (2001-05-29) [umoeller]: now broadcasting WM_SAVEAPPLICATION here
 *@@changed V0.9.12 (2001-05-29) [umoeller]: StartShutdownThread now uses THRF_PMMSGQUEUE so Wininitialize etc. has been removed here
 *@@changed V0.9.13 (2001-06-17) [umoeller]: no longer broadcasting WM_SAVEAPPLICATION, going back to old code
 *@@changed V0.9.13 (2001-06-19) [umoeller]: now pausing while exception handler is still running somewhere
 *@@changed V0.9.16 (2001-10-25) [umoeller]: couple of extra hacks for saving desktop
 *@@changed V0.9.19 (2002-04-24) [umoeller]: adjustments for new global G_ulShutdownState flag
 *@@changed V0.9.19 (2002-06-18) [umoeller]: misc optimizations
 *@@changed V0.9.19 (2002-06-18) [umoeller]: no longer recovering windows to current desktop for restart wps
 *@@changed V1.0.2 (2003-12-07) [pr]: retry closing WarpCenter @@fixes 302
 *@@changed V1.0.6 (2006-10-30) [pr]: re-added Window List position save @@fixes 458
 */

STATIC void _Optlink fntShutdownThread(PTHREADINFO ptiMyself)
{
    /*************************************************
     *
     *      data setup:
     *
     *************************************************/

    PSZ             pszErrMsg = NULL;
    QMSG            qmsg;
    APIRET          arc = NO_ERROR;
    HAB             hab = ptiMyself->hab;
    PXFILE          LogFile = NULL;

    // allocate shutdown data V0.9.9 (2001-03-07) [umoeller]
    PSHUTDOWNDATA   pShutdownData;
    if (!(pShutdownData = (PSHUTDOWNDATA)malloc(sizeof(SHUTDOWNDATA))))
    {
        G_ulShutdownState = XSD_IDLE;
        return;
    }

    // set shutdown state to "running"
    G_ulShutdownState = XSD_INITIALIZING;

    // CLEAR ALL FIELDS -- this is essential!
    memset(pShutdownData, 0, sizeof(SHUTDOWNDATA));

    // get shutdown params from thread info
    memcpy(&pShutdownData->sdParams,
           (PSHUTDOWNPARAMS)ptiMyself->ulData,
           sizeof(SHUTDOWNPARAMS));

    xsdGetShutdownConsts(&pShutdownData->SDConsts);

    // copy anchor block so subfuncs can use it
    // V0.9.12 (2001-05-29) [umoeller]
    pShutdownData->habShutdownThread = hab;

    // set some global data for all the following
    pShutdownData->hmodResource = cmnQueryNLSModuleHandle(FALSE);

    WinCancelShutdown(ptiMyself->hmq, TRUE);

    // open shutdown log file for writing, if enabled
    if (pShutdownData->sdParams.optLog)
    {
        CHAR    szLogFileName[CCHMAXPATH];
        ULONG   cbFile = 0;
        if (doshCreateLogFilename(szLogFileName,
                                  XFOLDER_SHUTDOWNLOG,
                                  F_ALLOW_BOOTROOT_LOGFILE))
            if (arc = doshOpen(szLogFileName,
                               XOPEN_READWRITE_APPEND | XOPEN_WRITETHRU,        // not XOPEN_BINARY
                                // added XOPEN_WRITETHRU V1.0.1 (2003-01-25) [umoeller]
                               &cbFile,
                               &LogFile))
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "Cannot create log file %s", szLogFileName);

        pShutdownData->ShutdownLogFile = LogFile;
    }

    if (LogFile)
    {
        // write log header
        doshWriteLogEntry(LogFile, "-----------------------------------------------------------");
        doshWriteLogEntry(LogFile, "XWorkplace version: %s", XFOLDER_VERSION);
        doshWriteLogEntry(LogFile, "Shutdown thread started, TID: 0x%lX",
                thrQueryID(ptiMyself));
        doshWriteLogEntry(LogFile, "Settings: CloseMode %d, Confirm %s, Reboot %s, WPSCloseWnds %s, CloseVIOs %s, WarpCenterFirst %s, PowerOff %s",
                pShutdownData->sdParams.ulCloseMode,
                (pShutdownData->sdParams.optConfirm) ? "ON" : "OFF",
                (pShutdownData->sdParams.optReboot) ? "ON" : "OFF",
                (pShutdownData->sdParams.optWPSCloseWindows) ? "ON" : "OFF",
                (pShutdownData->sdParams.optAutoCloseVIO) ? "ON" : "OFF",
                (pShutdownData->sdParams.optWarpCenterFirst) ? "ON" : "OFF",
                (pShutdownData->sdParams.optPowerOff)  // V1.0.5 (2006-06-26) [pr]
                    ? ((pShutdownData->sdParams.optACPIOff) ? "ACPI" : "APM")
                    : "OFF");
    }

    // raise our own priority; we will
    // still use the REGULAR class, but
    // with the maximum delta, so we can
    // get above nasty (DOS?) sessions
    DosSetPriority(PRTYS_THREAD,
                   PRTYC_REGULAR,
                   PRTYD_MAXIMUM, // priority delta
                   0);

    TRY_LOUD(excpt1)
    {
        SWCNTRL     swctl;
        HSWITCH     hswitch;
        ULONG       // ulKeyLength = 0,
                    ulAutoCloseItemsFound = 0;
        HPOINTER    hptrShutdown = WinLoadPointer(HWND_DESKTOP, pShutdownData->hmodResource,
                                                  ID_SDICON);

        // create an event semaphore which signals to the Update thread
        // that the Shutlist has been updated by fnwpShutdownThread
        DosCreateEventSem(NULL,         // unnamed
                          &pShutdownData->hevUpdated,
                          0,            // unshared
                          FALSE);       // not posted

        // create mutex semaphores for linked lists
        if (pShutdownData->hmtxShutdown == NULLHANDLE)
        {
            DosCreateMutexSem("\\sem32\\ShutdownList",
                              &pShutdownData->hmtxShutdown, 0, FALSE);     // unnamed, unowned
            DosCreateMutexSem("\\sem32\\SkippedList",
                              &pShutdownData->hmtxSkipped, 0, FALSE);      // unnamed, unowned
        }

        lstInit(&pShutdownData->llShutdown, TRUE);      // auto-free items
        lstInit(&pShutdownData->llSkipped, TRUE);       // auto-free items
        lstInit(&pShutdownData->llAutoClose, TRUE);     // auto-free items

        // check for auto-close items in OS2.INI
        // and build llAutoClose list accordingly
        doshWriteLogEntry(LogFile,
               __FUNCTION__ ": Getting auto-close items from OS2.INI...");

        ulAutoCloseItemsFound = xsdLoadAutoCloseItems(&pShutdownData->llAutoClose,
                                                      NULLHANDLE); // no list box

        doshWriteLogEntry(LogFile,
               "  Found %d auto-close items.", ulAutoCloseItemsFound);

        /*************************************************
         *
         *      shutdown windows setup:
         *
         *************************************************/

        doshWriteLogEntry(LogFile,
               "  Creating shutdown windows...");

        // setup main (debug) window; this is hidden
        // unless we're in debug mode
        pShutdownData->SDConsts.hwndMain
                = cmnLoadDlg(NULLHANDLE,
                             fnwpShutdownThread,
                             ID_SDD_MAIN,
                             NULL);
        WinSetWindowPtr(pShutdownData->SDConsts.hwndMain,
                        QWL_USER,
                        pShutdownData); // V0.9.9 (2001-03-07) [umoeller]
        WinSendMsg(pShutdownData->SDConsts.hwndMain,
                   WM_SETICON,
                   (MPARAM)hptrShutdown,
                    NULL);

        doshWriteLogEntry(LogFile,
               "  Created main window (hwnd: 0x%lX)",
               pShutdownData->SDConsts.hwndMain);
        doshWriteLogEntry(LogFile,
               "  HAB: 0x%lX, HMQ: 0x%lX, pidWPS: 0x%lX, pidPM: 0x%lX",
               hab,
               ptiMyself->hmq,
               pShutdownData->SDConsts.pidWPS,
               pShutdownData->SDConsts.pidPM);

        pShutdownData->ulMaxItemCount = 0;
        pShutdownData->ulLastItemCount = -1;

        pShutdownData->hPOC = 0;

        pShutdownData->sidPM = 1;  // should always be this, I hope

        // add ourselves to the tasklist
        swctl.hwnd = pShutdownData->SDConsts.hwndMain;                  // window handle
        swctl.hwndIcon = hptrShutdown;               // icon handle
        swctl.hprog = NULLHANDLE;               // program handle
        swctl.idProcess = pShutdownData->SDConsts.pidWPS;               // PID
        swctl.idSession = 0;                    // SID
        swctl.uchVisibility = SWL_VISIBLE;      // visibility
        swctl.fbJump = SWL_JUMPABLE;            // jump indicator
        WinQueryWindowText(pShutdownData->SDConsts.hwndMain, sizeof(swctl.szSwtitle), (PSZ)&swctl.szSwtitle);
        swctl.bProgType = PROG_DEFAULT;         // program type

        hswitch = WinAddSwitchEntry(&swctl);
        WinQuerySwitchEntry(hswitch, &swctl);
        pShutdownData->sidWPS = swctl.idSession;   // get the "real" WPS SID

        // setup status window (always visible)
        pShutdownData->SDConsts.hwndShutdownStatus
                = cmnLoadDlg(NULLHANDLE,
                             fnwpShutdownThread,
                             ID_SDD_STATUS,
                             NULL);
        WinSetWindowPtr(pShutdownData->SDConsts.hwndShutdownStatus,
                        QWL_USER,
                        pShutdownData); // V0.9.9 (2001-03-07) [umoeller]
        WinSendMsg(pShutdownData->SDConsts.hwndShutdownStatus,
                   WM_SETICON,
                   (MPARAM)hptrShutdown,
                   NULL);

        doshWriteLogEntry(LogFile,
               "  Created status window (hwnd: 0x%lX)",
               pShutdownData->SDConsts.hwndShutdownStatus);

        // subclass the static rectangle control in the dialog to make
        // it a progress bar
        pShutdownData->hwndProgressBar
            = WinWindowFromID(pShutdownData->SDConsts.hwndShutdownStatus,
                              ID_SDDI_PROGRESSBAR);
        ctlProgressBarFromStatic(pShutdownData->hwndProgressBar,
                                 PBA_ALIGNCENTER | PBA_BUTTONSTYLE);

        // set status window to top
        WinSetWindowPos(pShutdownData->SDConsts.hwndShutdownStatus,
                        HWND_TOP,
                        0, 0, 0, 0,
                        SWP_SHOW | SWP_ZORDER | SWP_ACTIVATE);

        // animate the traffic light
        xsdLoadAnimation(&G_sdAnim);
        ctlPrepareAnimation(WinWindowFromID(pShutdownData->SDConsts.hwndShutdownStatus,
                                            ID_SDDI_ICON),
                            XSD_ANIM_COUNT,
                            G_sdAnim.ahptr,
                            150,    // delay
                            TRUE);  // start now

        if (pShutdownData->sdParams.optDebug)
        {
            // debug mode: show "main" window, which
            // is invisible otherwise
            winhCenterWindow(pShutdownData->SDConsts.hwndMain);
            WinShowWindow(pShutdownData->SDConsts.hwndMain, TRUE);
        }

        doshWriteLogEntry(LogFile,
               __FUNCTION__ ": Now entering shutdown message loop...");

        // tell XPager to recover all windows to the current screen
        // V0.9.12 (2001-05-15) [umoeller]
        if (    (G_pXwpGlobalShared)
             && (G_pXwpGlobalShared->hwndDaemonObject)
           )
        {
            BOOL fWPSOnly = FALSE;
            // now, if we are doing a restart wps and not all
            // sessions should be closed, we should still recover
            // windows, but only those of the WPS process... so
            // a flag has been added for that to XDM_RECOVERWINDOWS
            // V0.9.20 (2002-08-10) [umoeller]
            if (    (pShutdownData->sdParams.ulCloseMode != SHUT_SHUTDOWN)
                            // not Desktop (1), not logoff (2)
                 && (!pShutdownData->sdParams.optWPSCloseWindows)
               )
                fWPSOnly = TRUE;

            doshWriteLogEntry(LogFile,
                   __FUNCTION__ ": Recovering all XPager windows...");

            WinSendMsg(G_pXwpGlobalShared->hwndDaemonObject,
                       XDM_RECOVERWINDOWS,
                       (MPARAM)fWPSOnly,
                       0);
        }

        WinShowWindow(pShutdownData->SDConsts.hwndShutdownStatus, TRUE);

        // empty trash can?
        if (    (pShutdownData->sdParams.optEmptyTrashCan)
             && (cmnTrashCanReady())
           )
        {
            WinSetDlgItemText(pShutdownData->SDConsts.hwndShutdownStatus, ID_SDDI_STATUS,
                              cmnGetString(ID_XSSI_FOPS_EMPTYINGTRASHCAN)) ; // pszFopsEmptyingTrashCan
            doshWriteLogEntry(pShutdownData->ShutdownLogFile, "    Emptying trash can...");

            if (!(arc = cmnEmptyDefTrashCan(pShutdownData->habShutdownThread,
                                                      // synchronously
                                            NULL,
                                            NULLHANDLE)))   // no confirm
                // success:
                doshWriteLogEntry(pShutdownData->ShutdownLogFile, "    Done emptying trash can.");
            else
            {
                doshWriteLogEntry(pShutdownData->ShutdownLogFile, "    Emptying trash can failed, rc: %d.",
                        arc);

                if (cmnMessageBoxExt(pShutdownData->SDConsts.hwndShutdownStatus,
                                     104, // "error"
                                     NULL, 0,
                                     189, // "empty failed"
                                     MB_YESNO)
                        == MBID_YES)
                    // continue anyway:
                    arc = NO_ERROR;
            }
        }

        if (!arc)
        {
            // now run items in Shutdown folder
            // but only if we're shutting down, not on restart wps
            // V0.9.19 (2002-06-18) [umoeller]
            // wrong, let user decide V1.0.1 (2003-01-29) [umoeller]
            XFolder         *pShutdownFolder;
            if (    (pShutdownData->sdParams.optWPSProcessShutdown)        // V1.0.1 (2003-01-29) [umoeller]
                 && (pShutdownFolder = _wpclsQueryFolder(_WPFolder,
                                                         (PSZ)XFOLDER_SHUTDOWNID,
                                                         TRUE))
               )
            {
                doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                                  "    Processing shutdown folder...");

                // using new implementation V0.9.12 (2001-04-29) [umoeller]
                arc = _xwpStartFolderContents(pShutdownFolder,
                                              0);         // wait mode
                        // added error code V1.0.1 (2003-01-29) [umoeller]

                doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                                  "    Shutdown folder rc = %d",
                                  arc);
            }
        }

        if (arc)
            G_ulShutdownState = XSD_CANCELLED;
        else
        {
            if (!pShutdownData->sdParams.optDebug)
            {
                // if we're not in debug mode, begin shutdown
                // automatically; ID_SDDI_BEGINSHUTDOWN will
                // first empty the trash can, process the
                // shutdown folder, and finally start closing
                // windows
                doshWriteLogEntry(LogFile, __FUNCTION__ ": Posting ID_SDDI_BEGINSHUTDOWN");
                WinPostMsg(pShutdownData->SDConsts.hwndMain,
                           WM_COMMAND,
                           MPFROM2SHORT(ID_SDDI_BEGINSHUTDOWN, 0),
                           MPNULL);
            }

            // pShutdownData->ulStatus is still XSD_IDLE at this point

            /*************************************************
             *
             *      standard PM message loop:
             *          here we are closing the windows
             *
             *************************************************/

            // now enter the common message loop for the main (debug) and
            // status windows (fnwpShutdownThread); this will keep running
            // until closing all windows is complete or cancelled, upon
            // both of which fnwpShutdownThread will post WM_QUIT
            while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
                WinDispatchMsg(hab, &qmsg);

            doshWriteLogEntry(LogFile,
                   __FUNCTION__ ": Done with message loop.");

            /*************************************************
             *
             *      done closing windows:
             *
             *************************************************/

            // in any case,
            // close the Update thread to prevent it from interfering
            // with what we're doing now
            if (thrQueryID(&G_tiUpdateThread))
            {
                doshWriteLogEntry(LogFile,
                       __FUNCTION__ ": Closing Update thread, tid: 0x%lX...",
                       thrQueryID(&G_tiUpdateThread));

                thrFree(&G_tiUpdateThread);  // close and wait
                doshWriteLogEntry(LogFile,
                       __FUNCTION__ ": Update thread closed.");
            }
        }

        // check if shutdown was cancelled (XSD_CANCELLED)
        // or if we should proceed (XSD_ALLCLOSED_SAVING)
        if (G_ulShutdownState == XSD_ALLCLOSED_SAVING)
        {
            ULONG       cObjectsToSave = 0,
                        cObjectsSaved = 0;
            CHAR        szTitle[400];
            PSWBLOCK    psw;
            WPObject    *pWarpCenter;

            /*************************************************
             *
             *      close desktop and WarpCenter
             *
             *************************************************/

            // save the window list position and fonts
            // V0.9.16 (2002-01-13) [umoeller]
            // this doesn't work... apparently we must
            // be in the shell process for WinStoreWindowPos
            // to work correctly
            // V1.0.6 (2006-10-31) [pr]: added this helper instead @@fixes 458
            if (!winhStoreWindowPos(winhQueryTasklistWindow(),
                                    HINI_USER,
                                    "PM_Workplace:WindowListPos",
                                    "SavePos"))
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "winhStoreWindowPos for tasklist failed");

            WinSetActiveWindow(HWND_DESKTOP,
                               pShutdownData->SDConsts.hwndShutdownStatus);

            // disable buttons in status window... we can't stop now!
            WinEnableControl(pShutdownData->SDConsts.hwndShutdownStatus,
                             ID_SDDI_CANCELSHUTDOWN,
                             FALSE);
            WinEnableControl(pShutdownData->SDConsts.hwndShutdownStatus,
                             ID_SDDI_SKIPAPP,
                             FALSE);

            // close Desktop window (which we excluded from
            // the regular SHUTLISTITEM list)
            doshWriteLogEntry(LogFile,
                              __FUNCTION__ ": Closing Desktop window");
            xsdUpdateClosingStatus(pShutdownData->SDConsts.hwndShutdownStatus,
                                   _wpQueryTitle(pShutdownData->SDConsts.pActiveDesktop));

            // sleep a little while... XCenter might have resized
            // the desktop, and this will hang otherwise
            // V0.9.9 (2001-04-04) [umoeller]
            winhSleep(300);

            if (pShutdownData->SDConsts.pActiveDesktop)
            {
                CHAR szDesktop[CCHMAXPATH];
                // set <WP_DESKTOP> ID on desktop; sometimes this gets
                // lost during shutdown
                // V0.9.16 (2001-10-25) [umoeller]
                if (_wpQueryFilename(pShutdownData->SDConsts.pActiveDesktop,
                                     szDesktop,
                                     TRUE))
                {
                    // save last active desktop in OS2.INI in case
                    // <WP_DESKTOP> gets broken so we can get the
                    // path in the new panic dialog; see
                    // initRepairDesktopIfBroken
                    // V0.9.16 (2001-10-25) [umoeller]
                    PrfWriteProfileString(HINI_USER,
                                          (PSZ)INIAPP_XWORKPLACE,
                                          (PSZ)INIKEY_LASTDESKTOPPATH,
                                          szDesktop);
                }

                _wpSetObjectID(pShutdownData->SDConsts.pActiveDesktop,
                               (PSZ)WPOBJID_DESKTOP); // "<WP_DESKTOP>",

                _wpSaveImmediate(pShutdownData->SDConsts.pActiveDesktop);
                _wpClose(pShutdownData->SDConsts.pActiveDesktop);
                _wpWaitForClose(pShutdownData->SDConsts.pActiveDesktop,
                                NULLHANDLE,     // all views
                                0xFFFFFFFF,
                                5*1000,     // timeout value
                                TRUE);      // force close for new views
                            // added V0.9.4 (2000-07-11) [umoeller]

                // give the desktop time to save icon positions
                winhSleep(300); // V0.9.12 (2001-04-29) [umoeller]
            }

            // close WarpCenter next (V0.9.5, from V0.9.3)
            if (    (pWarpCenter = G_pAwakeWarpCenter)      // global variable (xfobj.c, kernel.h) V0.9.20 (2002-07-25) [umoeller]
                 && (somIsObj(pWarpCenter))
               )
            {
                // WarpCenter still open?
                // V1.0.2 (2003-12-07) [pr]: Retry closing WarpCenter @@fixes 302
                while (_wpFindUseItem(pWarpCenter, USAGE_OPENVIEW, NULL))
                {
                    // if open: close it
                    xsdUpdateClosingStatus(pShutdownData->SDConsts.hwndShutdownStatus,
                                           _wpQueryTitle(pWarpCenter));
                    doshWriteLogEntry(LogFile,
                           __FUNCTION__ ": Found open WarpCenter USEITEM, closing...");

                    _wpSaveImmediate(pWarpCenter);
                    // _wpClose(pWarpCenter);
                    WinPostMsg(pShutdownData->SDConsts.hwndOpenWarpCenter,
                               WM_COMMAND,
                               MPFROMSHORT(0x66F7),
                                    // "Close" menu item in WarpCenter context menu...
                                    // nothing else works right!
                               MPFROM2SHORT(CMDSRC_OTHER,
                                            FALSE));     // keyboard?!?
                    WinPostMsg(pShutdownData->SDConsts.hwndOpenWarpCenter,
                               WM_CLOSE,
                               WM_NULL,
                               WM_NULL);
                    _wpWaitForClose(pWarpCenter,
                                    pShutdownData->SDConsts.hwndOpenWarpCenter,
                                    VIEW_ANY,
                                    /*SEM_INDEFINITE_WAIT*/ 1000,
                                    TRUE);
                }

                pShutdownData->SDConsts.hwndOpenWarpCenter = NULLHANDLE;
            }

            // if some thread is currently in an exception handler,
            // wait until the handler is done; otherwise the trap
            // log won't be written and we can't find out what
            // happened V0.9.13 (2001-06-19) [umoeller]
            xsdWaitForExceptions(pShutdownData);

            // set progress bar to the max
            WinSendMsg(pShutdownData->hwndProgressBar,
                       WM_UPDATEPROGRESSBAR,
                       (MPARAM)1,
                       (MPARAM)1);

            winhSleep(300);

            // now we need a blank screen so that it looks
            // as if we had closed all windows, even if we
            // haven't; we do this by creating a "fake
            // desktop", which is just an empty window w/out
            // title bar which takes up the whole screen and
            // has the color of the PM desktop
            if (    (pShutdownData->sdParams.ulCloseMode == SHUT_SHUTDOWN)
                 && (!(pShutdownData->sdParams.optDebug))
               )
                winhCreateFakeDesktop(pShutdownData->SDConsts.hwndShutdownStatus);

            /*************************************************
             *
             *      save Desktop objects
             *
             *************************************************/

            cObjectsToSave = objQueryDirtyObjectsCount();

            sprintf(szTitle,
                    cmnGetString(ID_SDSI_SAVINGDESKTOP), // cmnQueryNLSStrings()->pszSDSavingDesktop,
                        // "Saving xxx awake Desktop objects..."
                    cObjectsToSave);
            WinSetDlgItemText(pShutdownData->SDConsts.hwndShutdownStatus,
                              ID_SDDI_STATUS,
                              szTitle);

            doshWriteLogEntry(LogFile,
                   __FUNCTION__ ": Saving %d awake Desktop objects...",
                   cObjectsToSave);

            // reset progress bar
            WinSendMsg(pShutdownData->hwndProgressBar,
                       WM_UPDATEPROGRESSBAR,
                       (MPARAM)0,
                       (MPARAM)cObjectsToSave);

            // if some thread is currently in an exception handler,
            // wait until the handler is done; otherwise the trap
            // log won't be written and we can't find out what
            // happened V0.9.13 (2001-06-19) [umoeller]
            xsdWaitForExceptions(pShutdownData);

            // finally, save WPS!!

            // now using proper "dirty" list V0.9.9 (2001-04-04) [umoeller]
            cObjectsSaved = objForAllDirtyObjects(fncbSaveImmediate,
                                                  pShutdownData);  // user param

            // have the WPS flush its buffers
            xsdFlushWPS2INI();  // added V0.9.0 (UM 99-10-22)

            // and wait a while
            winhSleep(500);

            // if some thread is currently in an exception handler,
            // wait until the handler is done; otherwise the trap
            // log won't be written and we can't find out what
            // happened V0.9.13 (2001-06-19) [umoeller]
            xsdWaitForExceptions(pShutdownData);

            // set progress bar to max
            WinSendMsg(pShutdownData->hwndProgressBar,
                       WM_UPDATEPROGRESSBAR,
                       (MPARAM)1,
                       (MPARAM)1);

            doshWriteLogEntry(LogFile,
                   __FUNCTION__ ": Done saving WPS, %d objects saved.",
                   cObjectsSaved);

            winhSleep(200);

            if (pShutdownData->sdParams.ulCloseMode != SHUT_SHUTDOWN)
            {
                // "Restart Desktop" mode, or "logoff":
                WinSetDlgItemText(pShutdownData->SDConsts.hwndShutdownStatus,
                                  ID_SDDI_STATUS,
                                  cmnGetString(ID_SDSI_RESTARTINGWPS)); // (cmnQueryNLSStrings())->pszSDRestartingWPS);

                // reuse startup folder?
                krnSetProcessStartupFolder(pShutdownData->sdParams.optWPSReuseStartupFolder);
            }

        } // end if (pShutdownData->ulStatus == XSD_ALLDONEOK)

// end moved code with V0.9.9 (2001-04-04) [umoeller]

    } // end TRY_LOUD(excpt1
    CATCH(excpt1)
    {
        // exception occurred:
        krnUnlockGlobals();     // just to make sure
        // fExceptionOccurred = TRUE;

        // only report the first error, or otherwise we will
        // jam the system with msg boxes @@todo get rid of this shit

        if (!pszErrMsg)
        {
            if (pszErrMsg = strdup("An error occurred in the XFolder Shutdown thread. "
                        "In the root directory of your boot drive, you will find a "
                        "file named XFLDTRAP.LOG, which contains debugging information. "
                        "If you had shutdown logging enabled, you will also find the "
                        "file XSHUTDWN.LOG there. If not, please enable shutdown "
                        "logging in the Desktop's settings notebook. "
                        "\n\nThe XShutdown procedure will be terminated now. We can "
                        "now also restart the Workplace Shell. This is recommended if "
                        "your Desktop has already been closed or if "
                        "the error occurred during the saving of the INI files. In these "
                        "cases, please disable XShutdown and perform a regular OS/2 "
                        "shutdown to prevent loss of your WPS data."
                        "\n\nRestart the Workplace Shell now?"))
            {
                krnPostThread1ObjectMsg(T1M_EXCEPTIONCAUGHT, (MPARAM)pszErrMsg,
                                        (MPARAM)1); // enforce Desktop restart

                doshWriteLogEntry(LogFile,
                       "\n*** CRASH\n%s\n", pszErrMsg);
            }
        }
    } END_CATCH();

    /*
     * Cleanup:
     *
     */

    // we arrive here if
    //      a) fnwpShutdownThread successfully closed all windows;
    //         only in that case, fAllWindowsClosed is TRUE;
    //      b) shutdown was cancelled by the user;
    //      c) an exception occurred.
    // In any of these cases, we need to clean up big time now.

    // close "main" window, but keep the status window for now
    winhDestroyWindow(&pShutdownData->SDConsts.hwndMain);

    doshWriteLogEntry(LogFile,
           __FUNCTION__ ": Entering cleanup...");

    // check for whether we're owning semaphores;
    // if we do (e.g. after an exception), release them now
    if (pShutdownData->fShutdownSemOwned)
    {
        doshWriteLogEntry(LogFile, "  Releasing shutdown mutex");
        DosReleaseMutexSem(pShutdownData->hmtxShutdown);
        pShutdownData->fShutdownSemOwned = FALSE;
    }
    if (pShutdownData->fSkippedSemOwned)
    {
        doshWriteLogEntry(LogFile, "  Releasing skipped mutex");
        DosReleaseMutexSem(pShutdownData->hmtxSkipped);
        pShutdownData->fSkippedSemOwned = FALSE;
    }

    doshWriteLogEntry(LogFile, "  Done releasing semaphores.");

    // get rid of the Update thread;
    // this got closed by fnwpShutdownThread normally,
    // but with exceptions, this might not have happened
    if (thrQueryID(&G_tiUpdateThread))
    {
        doshWriteLogEntry(LogFile, "  Closing Update thread...");
        thrFree(&G_tiUpdateThread); // fixed V0.9.0
        doshWriteLogEntry(LogFile, "  Update thread closed.");
    }

    doshWriteLogEntry(LogFile, "  Closing semaphores...");
    DosCloseEventSem(pShutdownData->hevUpdated);
    if (pShutdownData->hmtxShutdown != NULLHANDLE)
    {
        if (arc = DosCloseMutexSem(pShutdownData->hmtxShutdown))
        {
            DosBeep(100, 1000);
            doshWriteLogEntry(LogFile, "    Error %d closing hmtxShutdown!",
                              arc);
        }
        pShutdownData->hmtxShutdown = NULLHANDLE;
    }
    if (pShutdownData->hmtxSkipped != NULLHANDLE)
    {
        if (arc = DosCloseMutexSem(pShutdownData->hmtxSkipped))
        {
            DosBeep(100, 1000);
            doshWriteLogEntry(LogFile, "    Error %d closing hmtxSkipped!",
                              arc);
        }
        pShutdownData->hmtxSkipped = NULLHANDLE;
    }
    doshWriteLogEntry(LogFile, "  Done closing semaphores.");

    doshWriteLogEntry(LogFile, "  Freeing lists...");
    TRY_LOUD(excpt1)
    {
        // destroy all global lists; this time, we need
        // no mutex semaphores, because the Update thread
        // is already down
        lstClear(&pShutdownData->llShutdown);
        lstClear(&pShutdownData->llSkipped);
    }
    CATCH(excpt1) {} END_CATCH();
    doshWriteLogEntry(LogFile, "  Done freeing lists.");

    /*
     * Restart Desktop or shutdown:
     *
     */

    if (G_ulShutdownState == XSD_ALLCLOSED_SAVING)
    {
        // happens only if shutdown was not cancelled;
        // this means that all windows have been successfully
        // closed, the WPS is saved, and we can actually shut
        // down the system

        G_ulShutdownState = XSD_SAVEDONE_FLUSHING;
            // V0.9.19 (2002-04-24) [umoeller]

        if (pShutdownData->sdParams.ulCloseMode != SHUT_SHUTDOWN) // restart Desktop (1) or logoff (2)
        {
            // here we will actually restart the WPS
            doshWriteLogEntry(LogFile, "Preparing Desktop restart...");

            ctlStopAnimation(WinWindowFromID(pShutdownData->SDConsts.hwndShutdownStatus, ID_SDDI_ICON));
            winhDestroyWindow(&pShutdownData->SDConsts.hwndShutdownStatus);
            xsdFreeAnimation(&G_sdAnim);

            doshWriteLogEntry(LogFile, "Restarting WPS: Calling DosExit(), closing log.");
            doshClose(&LogFile);
            LogFile = NULL;

            xsdRestartWPS(hab,
                          (pShutdownData->sdParams.ulCloseMode == SHUT_LOGOFF));
                            // V0.9.11 (2001-04-18) [umoeller]
                // this will not return, I think
        }
        else
        {
            // *** no restart Desktop:
            // call the termination routine, which
            // will do the rest

            xsdFinishShutdown(pShutdownData);
            // this will not return, except in debug mode

            if (pShutdownData->sdParams.optDebug)
                // in debug mode, restart Desktop
                pShutdownData->sdParams.ulCloseMode = SHUT_RESTARTWPS;
        }
    } // end if (fAllWindowsClosed)

    // the following code is only reached if
    // shutdown was cancelled...

    // close logfile
    if (LogFile)
    {
        doshWriteLogEntry(LogFile, "Reached cleanup, closing log.");
        doshClose(&LogFile);
        LogFile = NULL;
    }

    // moved this down, because we need a msg queue for restart Desktop
    // V0.9.3 (2000-04-26) [umoeller]

    winhDestroyWindow(&pShutdownData->SDConsts.hwndShutdownStatus);

    free(pShutdownData);        // V0.9.9 (2001-03-07) [umoeller]

    // set the global flag for whether shutdown is
    // running to FALSE; this will re-enable the
    // items in the Desktop's context menu
    G_ulShutdownState = XSD_IDLE;

    // end of Shutdown thread
    // thread exits!
}

/*
 *@@ xsdCloseVIO:
 *      this gets called upon ID_SDMI_CLOSEVIO in
 *      fnwpShutdownThread when a VIO window is encountered.
 *      (To be precise, this gets called for all non-PM
 *      sessions, not just VIO windows.)
 *
 *      This function queries the list of auto-close
 *      items and closes the VIO window accordingly.
 *      If no corresponding item was found, we display
 *      a dialog, querying the user what to do with this.
 *
 *      Runs on the Shutdown thread.
 *
 *@@added V0.9.1 (99-12-10) [umoeller]
 *@@changed V0.9.19 (2002-05-23) [umoeller]: removed Ctrl+C option, which never worked
 *@@changed V1.0.10 (20140-12-07) [pr]: DosKillProcess replaced with prc32KillProcessTree @@fixes 1216
 */

VOID xsdCloseVIO(PSHUTDOWNDATA pShutdownData,
                 HWND hwndFrame)
{
    PSHUTLISTITEM   pItem;
    ULONG           ulReply;
    PAUTOCLOSELISTITEM pliAutoCloseFound = NULL; // fixed V0.9.0
    ULONG           ulSessionAction = 0;
                        // this will become one of the ACL_* flags
                        // if something is to be done with this session

    doshWriteLogEntry(pShutdownData->ShutdownLogFile,
           "  ID_SDMI_CLOSEVIO, hwnd: 0x%lX; entering xsdCloseVIO",
           hwndFrame);

    // get VIO item to close
    if (pItem = xsdQueryCurrentItem(pShutdownData))
    {
        // valid item: go thru list of auto-close items
        // if this item is on there
        PLISTNODE   pAutoCloseNode = 0;
        xsdLongTitle(pShutdownData->szVioTitle, pItem);
        pShutdownData->VioItem = *pItem;
        doshWriteLogEntry(pShutdownData->ShutdownLogFile, "    xsdCloseVIO: VIO item: %s", pShutdownData->szVioTitle);

        // activate VIO window
        WinSetActiveWindow(HWND_DESKTOP, pShutdownData->VioItem.swctl.hwnd);

        // check if VIO window is on auto-close list
        pAutoCloseNode = lstQueryFirstNode(&pShutdownData->llAutoClose);
        while (pAutoCloseNode)
        {
            PAUTOCLOSELISTITEM pliThis = pAutoCloseNode->pItemData;
            doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      Checking %s", pliThis->szItemName);
            // compare first characters
            if (!strnicmp(pShutdownData->VioItem.swctl.szSwtitle,
                          pliThis->szItemName,
                          strlen(pliThis->szItemName)))
            {
                // item found:
                doshWriteLogEntry(pShutdownData->ShutdownLogFile, "        Matching item found, auto-closing item");
                pliAutoCloseFound = pliThis;
                break;
            }

            pAutoCloseNode = pAutoCloseNode->pNext;
        }

        if (pliAutoCloseFound)
        {
            // item was found on auto-close list:
            doshWriteLogEntry(pShutdownData->ShutdownLogFile, "    Found on auto-close list");

            // store this item's action for later
            ulSessionAction = pliAutoCloseFound->usAction;
        } // end if (pliAutoCloseFound)
        else
        {
            // not on auto-close list:
            if (pShutdownData->sdParams.optAutoCloseVIO)
            {
                // auto-close enabled globally though:
                doshWriteLogEntry(pShutdownData->ShutdownLogFile, "    Not found on auto-close list, auto-close is on:");

                // "auto close VIOs" is on
                if (pShutdownData->VioItem.swctl.idSession == 1)
                {
                    // for some reason, DOS/Windows sessions always
                    // run in the Shell process, whose SID == 1
                    WinPostMsg((WinWindowFromID(pShutdownData->VioItem.swctl.hwnd, FID_SYSMENU)),
                               WM_SYSCOMMAND,
                               (MPARAM)SC_CLOSE, MPNULL);
                    doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      Posted SC_CLOSE to hwnd 0x%lX",
                           WinWindowFromID(pShutdownData->VioItem.swctl.hwnd, FID_SYSMENU));
                }
                else
                {
                    // OS/2 windows: kill
                    prc32KillProcessTree(pShutdownData->VioItem.swctl.idProcess);  // V1.0.10
                    doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      Killed pid 0x%lX",
                           pShutdownData->VioItem.swctl.idProcess);
                }
            } // end if (psdParams->optAutoCloseVIO)
            else
            {
                CHAR    szText[500];
                ULONG   len;

                // no auto-close: confirmation wnd
                doshWriteLogEntry(pShutdownData->ShutdownLogFile, "    Not found on auto-close list, auto-close is off, query-action dlg:");

                cmnSetDlgHelpPanel(ID_XFH_CLOSEVIO);
                pShutdownData->SDConsts.hwndVioDlg
                    = cmnLoadDlg(pShutdownData->SDConsts.hwndShutdownStatus,
                                 cmn_fnwpDlgWithHelp,
                                 ID_SDD_CLOSEVIO,
                                 NULL);

                // ID_SDDI_VDMAPPTEXT has "\"cannot be closed automatically";
                // prefix session title
                szText[0] = '\"';
                len = strlcpy(szText + 1,
                              pShutdownData->VioItem.swctl.szSwtitle,
                              sizeof(szText) - 1);
                ++len;
                WinQueryDlgItemText(pShutdownData->SDConsts.hwndVioDlg,
                                    ID_SDDI_VDMAPPTEXT,
                                    sizeof(szText) - len,
                                    &szText[len]);
                WinSetDlgItemText(pShutdownData->SDConsts.hwndVioDlg, ID_SDDI_VDMAPPTEXT,
                                  szText);

                cmnSetControlsFont(pShutdownData->SDConsts.hwndVioDlg, 1, 5000);
                winhCenterWindow(pShutdownData->SDConsts.hwndVioDlg);
                ulReply = WinProcessDlg(pShutdownData->SDConsts.hwndVioDlg);

                if (ulReply == DID_OK)
                {
                    doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      'OK' pressed");

                    // "OK" button pressed: check the radio buttons
                    // for what to do with this session
                    if (winhIsDlgItemChecked(pShutdownData->SDConsts.hwndVioDlg, ID_XSDI_ACL_SKIP))
                    {
                        ulSessionAction = ACL_SKIP;
                        doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      xsdCloseVIO: 'Skip' selected");
                    }
                    else if (winhIsDlgItemChecked(pShutdownData->SDConsts.hwndVioDlg, ID_XSDI_ACL_WMCLOSE))
                    {
                        ulSessionAction = ACL_WMCLOSE;
                        doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      xsdCloseVIO: 'WM_CLOSE' selected");
                    }
                    else if (winhIsDlgItemChecked(pShutdownData->SDConsts.hwndVioDlg, ID_XSDI_ACL_KILLSESSION))
                    {
                        ulSessionAction = ACL_KILLSESSION;
                        doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      xsdCloseVIO: 'Kill' selected");
                    }

                    // "store item" checked?
                    if (ulSessionAction)
                        if (winhIsDlgItemChecked(pShutdownData->SDConsts.hwndVioDlg, ID_XSDI_ACL_STORE))
                        {
                            ULONG ulInvalid = 0;
                            // "store" checked:
                            // add item to list
                            PAUTOCLOSELISTITEM pliNew = malloc(sizeof(AUTOCLOSELISTITEM));
                            strncpy(pliNew->szItemName,
                                    pShutdownData->VioItem.swctl.szSwtitle,
                                    sizeof(pliNew->szItemName)-1);
                            pliNew->szItemName[99] = 0;
                            pliNew->usAction = ulSessionAction;
                            lstAppendItem(&pShutdownData->llAutoClose,
                                          pliNew);

                            // write list back to OS2.INI
                            ulInvalid = xsdWriteAutoCloseItems(&pShutdownData->llAutoClose);
                            doshWriteLogEntry(pShutdownData->ShutdownLogFile, "         Updated auto-close list in OS2.INI, rc: %d",
                                        ulInvalid);
                        }
                }
                else if (ulReply == ID_SDDI_CANCELSHUTDOWN)
                {
                    // "Cancel shutdown" pressed:
                    // pass to main window
                    WinPostMsg(pShutdownData->SDConsts.hwndMain, WM_COMMAND,
                               MPFROM2SHORT(ID_SDDI_CANCELSHUTDOWN, 0),
                               MPNULL);
                    doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      'Cancel shutdown' pressed");
                }
                // else: we could also get DID_CANCEL; this means
                // that the dialog was closed because the Update
                // thread determined that a session was closed
                // manually, so we just do nothing...

                winhDestroyWindow(&pShutdownData->SDConsts.hwndVioDlg);
            } // end else (optAutoCloseVIO)
        }

        doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                          "      xsdCloseVIO: ulSessionAction is %d", ulSessionAction);

        // OK, let's see what to do with this session
        switch (ulSessionAction)
                    // this is 0 if nothing is to be done
        {
            case ACL_WMCLOSE:
                WinPostMsg((WinWindowFromID(pShutdownData->VioItem.swctl.hwnd, FID_SYSMENU)),
                           WM_SYSCOMMAND,
                           (MPARAM)SC_CLOSE, MPNULL);
                doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      xsdCloseVIO: Posted SC_CLOSE to sysmenu, hwnd: 0x%lX",
                       WinWindowFromID(pShutdownData->VioItem.swctl.hwnd, FID_SYSMENU));
            break;

            /* case ACL_CTRL_C:     removed V0.9.19 (2002-05-23) [umoeller]
                DosSendSignalException(pShutdownData->VioItem.swctl.idProcess,
                                       XCPT_SIGNAL_INTR);
                doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      xsdCloseVIO: Sent INTR signal to pid 0x%lX",
                       pShutdownData->VioItem.swctl.idProcess);
            break; */

            case ACL_KILLSESSION:
                prc32KillProcessTree(pShutdownData->VioItem.swctl.idProcess);  // V1.0.10
                doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      xsdCloseVIO: Killed pid 0x%lX",
                       pShutdownData->VioItem.swctl.idProcess);
            break;

            case ACL_SKIP:
                WinPostMsg(pShutdownData->SDConsts.hwndMain, WM_COMMAND,
                           MPFROM2SHORT(ID_SDDI_SKIPAPP, 0),
                           MPNULL);
                doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      xsdCloseVIO: Posted ID_SDDI_SKIPAPP");
            break;
        }
    }

    doshWriteLogEntry(pShutdownData->ShutdownLogFile, "  Done with xsdCloseVIO");
}

/*
 *@@ CloseOneItem:
 *      implementation for ID_SDMI_CLOSEITEM in
 *      ID_SDMI_CLOSEITEM. Closes one item on
 *      the shutdown list, depending on the
 *      item's type.
 *
 *@@added V0.9.9 (2001-04-04) [umoeller]
 */

STATIC VOID CloseOneItem(PSHUTDOWNDATA pShutdownData,
                         HWND hwndListbox,
                         PSHUTLISTITEM pItem)
{
    CHAR        szTitle[1024];
    USHORT      usItem;

    // compose string from item
    xsdLongTitle(szTitle, pItem);

    doshWriteLogEntry(pShutdownData->ShutdownLogFile, "    Item: %s", szTitle);

    // find string in the invisible list box
    usItem = (USHORT)WinSendMsg(hwndListbox,
                                LM_SEARCHSTRING,
                                MPFROM2SHORT(0, LIT_FIRST),
                                (MPARAM)szTitle);
    // and select it
    if ((usItem != (USHORT)LIT_NONE))
        WinPostMsg(hwndListbox,
                   LM_SELECTITEM,
                   (MPARAM)usItem,
                   (MPARAM)TRUE);

    // update status window: "Closing xxx"
    xsdUpdateClosingStatus(pShutdownData->SDConsts.hwndShutdownStatus,
                           pItem->swctl.szSwtitle);

    // now check what kind of action needs to be done
    if (pItem->pObject)
    {
        // we have a WPS window
        // (cannot be WarpCenter, cannot be Desktop):
        _wpClose(pItem->pObject);
                    // WPObject::goes thru the list of USAGE_OPENVIEW
                    // useitems and sends WM_CLOSE to each of them

        doshWriteLogEntry(pShutdownData->ShutdownLogFile,
               "      Open Desktop object, called wpClose(pObject)");
    }
    else if (pItem->swctl.hwnd)
    {
        // no WPS window: differentiate further
        doshWriteLogEntry(pShutdownData->ShutdownLogFile,
               "      swctl.hwnd found: 0x%lX",
               pItem->swctl.hwnd);

        if (    (pItem->swctl.bProgType == PROG_VDM)
             || (pItem->swctl.bProgType == PROG_WINDOWEDVDM)
             || (pItem->swctl.bProgType == PROG_FULLSCREEN)
             || (pItem->swctl.bProgType == PROG_WINDOWABLEVIO)
             // || (pItem->swctl.bProgType == PROG_DEFAULT)
           )
        {
            // not a PM session: ask what to do (handled below)
            doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      Seems to be VIO, swctl.bProgType: 0x%lX", pItem->swctl.bProgType);
            if (    (pShutdownData->SDConsts.hwndVioDlg == NULLHANDLE)
                 || (strcmp(szTitle, pShutdownData->szVioTitle) != 0)
               )
            {
                // "Close VIO window" not currently open:
                doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      Posting ID_SDMI_CLOSEVIO");
                WinPostMsg(pShutdownData->SDConsts.hwndMain, WM_COMMAND,
                           MPFROM2SHORT(ID_SDMI_CLOSEVIO, 0),
                           MPNULL);
            }
            /// else do nothing
        }
        else
        {
            // close PM application:
            // WM_SAVEAPPLICATION and WM_QUIT is what WinShutdown
            // does too for every message queue per process;
            // re-enabled this here per window V0.9.13 (2001-06-17) [umoeller]
            doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                   "      Posting WM_SAVEAPPLICATION to hwnd 0x%lX",
                   pItem->swctl.hwnd);

            WinPostMsg(pItem->swctl.hwnd,
                       WM_SAVEAPPLICATION,
                       MPNULL, MPNULL);

            doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                   "      Posting WM_QUIT to hwnd 0x%lX",
                   pItem->swctl.hwnd);

            WinPostMsg(pItem->swctl.hwnd,
                       WM_QUIT,
                       MPNULL,      // V1.0.3 (2004-5-9) [umoeller]: fixed compile stopper
                       (MPARAM)pItem->swctl.hwnd); // V1.0.3 (2004-5-9) [pr]:Required for WarpCenter
        }
    }
    else
    {
        doshWriteLogEntry(pShutdownData->ShutdownLogFile,
               "      Helpless... leaving item alone, pid: 0x%lX",
               pItem->swctl.idProcess);
        // DosKillProcess(DKP_PROCESS, pItem->swctl.idProcess);
    }
}

/*
 *@@ fnwpShutdownThread:
 *      window procedure for both the main (debug) window and
 *      the status window; it receives messages from the Update
 *      threads, so that it can update the windows' contents
 *      accordingly; it also controls this thread by suspending
 *      or killing it, if necessary, and setting semaphores.
 *      Note that the main (debug) window with the listbox is only
 *      visible in debug mode (signalled to xfInitiateShutdown).
 *
 *      Runs on the Shutdown thread.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist.c functions
 *@@changed V0.9.0 [umoeller]: fixed some inconsistensies in ID_SDMI_CLOSEVIO
 *@@changed V0.9.0 [umoeller]: changed shutdown logging to stdio functions (fprintf etc.)
 *@@changed V0.9.0 [umoeller]: changed "reuse Desktop startup folder" to use XWPGLOBALSHARED
 *@@changed V0.9.0 [umoeller]: added xsdFlushWPS2INI call
 *@@changed V0.9.1 (99-12-10) [umoeller]: extracted VIO code to xsdCloseVIO
 *@@changed V0.9.4 (2000-07-11) [umoeller]: added wpWaitForClose for Desktop
 *@@changed V0.9.4 (2000-07-15) [umoeller]: added special treatment for WarpCenter
 *@@changed V0.9.6 (2000-10-27) [umoeller]: fixed special treatment for WarpCenter
 *@@changed V0.9.7 (2000-12-08) [umoeller]: now taking WarpCenterFirst setting into account
 *@@changed V0.9.9 (2001-03-07) [umoeller]: now using all settings in SHUTDOWNDATA
 *@@changed V0.9.9 (2001-03-07) [umoeller]: fixed race condition on closing XCenter
 *@@changed V0.9.9 (2001-04-04) [umoeller]: extracted CloseOneItem
 *@@changed V0.9.9 (2001-04-04) [umoeller]: moved all post-close stuff to fntShutdownThread
 *@@changed V0.9.12 (2001-04-29) [umoeller]: now starting update thread after shutdown folder processing
 *@@changed V0.9.12 (2001-04-29) [umoeller]: now using new shutdown folder implementation
 *@@changed V0.9.16 (2001-10-22) [pr]: fixed bug trying to close the first switch list item twice
 *@@changed V0.9.19 (2002-06-18) [umoeller]: now running shutdown folder only for shutdown, not restart wps
 */

STATIC MRESULT EXPENTRY fnwpShutdownThread(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT         mrc = MRFALSE;

    switch(msg)
    {
        // case WM_INITDLG:     // both removed V0.9.9 (2001-03-07) [umoeller]
        // case WM_CREATE:

        case WM_COMMAND:
        {
            PSHUTDOWNDATA   pShutdownData;
            HWND            hwndListbox;

            if (!(pShutdownData = (PSHUTDOWNDATA)WinQueryWindowPtr(hwndFrame,
                                                                   QWL_USER)))
                break;

            hwndListbox = WinWindowFromID(pShutdownData->SDConsts.hwndMain,
                                          ID_SDDI_LISTBOX);

            switch (SHORT1FROMMP(mp1))
            {
                /*
                 * ID_SDDI_BEGINSHUTDOWN:
                 *
                 */

                case ID_SDDI_BEGINSHUTDOWN:
                {
                    // this is either posted by the "Begin shutdown"
                    // button (in debug mode) or otherwise automatically
                    // after the first update initiated by the Update
                    // thread

                    // pShutdownData->ulStatus is still XSD_IDLE at this point

                    PMPF_SHUTDOWN((" ---> ID_SDDI_BEGINSHUTDOWN"));

                    doshWriteLogEntry(pShutdownData->ShutdownLogFile, "  ID_SDDI_BEGINSHUTDOWN, hwnd: 0x%lX", hwndFrame);

                    // now build our shutlist V0.9.12 (2001-04-29) [umoeller]
                    xsdUpdateListBox(pShutdownData->habShutdownThread,
                                     pShutdownData,
                                     hwndListbox);

                    // create the Update thread now
                    // V0.9.12 (2001-04-28) [umoeller]
                    // moved this down here, we shouldn't start this
                    // before the shutdown folder has finished
                    // processing, or we'll close the windows we've
                    // started ourselves
                    if (thrQueryID(&G_tiUpdateThread) == NULLHANDLE)
                    {
                        thrCreate(&G_tiUpdateThread,
                                  fntUpdateThread,
                                  NULL, // running flag
                                  "ShutdownUpdate",
                                  THRF_PMMSGQUEUE | THRF_WAIT_EXPLICIT,
                                        // but wait explicit V0.9.12 (2001-04-29) [umoeller]
                                        // added msgq V0.9.16 (2002-01-13) [umoeller]
                                  (ULONG)pShutdownData);  // V0.9.9 (2001-03-07) [umoeller]

                        doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                               __FUNCTION__ ": Update thread started, tid: 0x%lX",
                               thrQueryID(&G_tiUpdateThread));
                    }

                    // set progress bar to the left
                    WinSendMsg(pShutdownData->hwndProgressBar,
                               WM_UPDATEPROGRESSBAR,
                               (MPARAM)0,
                               (MPARAM)1);

                    // close open WarpCenter first, if desired
                    // V0.9.7 (2000-12-08) [umoeller]
                    doshWriteLogEntry(pShutdownData->ShutdownLogFile, "  WarpCenter treatment:");
                    if (somIsObj(G_pAwakeWarpCenter))       // global variable (xfobj.c, kernel.h) V0.9.20 (2002-07-25) [umoeller]
                    {
                        if (pShutdownData->SDConsts.hwndOpenWarpCenter)
                        {
                            doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      WarpCenter found, has HWND 0x%lX",
                                   pShutdownData->SDConsts.hwndOpenWarpCenter);
                            if (pShutdownData->sdParams.optWarpCenterFirst)
                            {
                                doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      WarpCenterFirst is ON, posting WM_COMMAND 0x66F7");
                                xsdUpdateClosingStatus(pShutdownData->SDConsts.hwndShutdownStatus,
                                                       "WarpCenter");
                                WinPostMsg(pShutdownData->SDConsts.hwndOpenWarpCenter,
                                           WM_COMMAND,
                                           MPFROMSHORT(0x66F7),
                                                // "Close" menu item in WarpCenter context menu...
                                                // nothing else works right!
                                           MPFROM2SHORT(CMDSRC_OTHER,
                                                        FALSE));     // keyboard?!?
                                winhSleep(400);
                                pShutdownData->SDConsts.hwndOpenWarpCenter = NULLHANDLE;
                            }
                            else
                                doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      WarpCenterFirst is OFF, skipping...");
                        }
                        else
                            doshWriteLogEntry(pShutdownData->ShutdownLogFile, "      WarpCenter not found.");
                    }

                    // mark status as "closing windows now"
                    G_ulShutdownState = XSD_CLOSINGWINDOWS;
                    WinEnableControl(pShutdownData->SDConsts.hwndMain,
                                      ID_SDDI_BEGINSHUTDOWN,
                                      FALSE);

                    // V0.9.16 (2001-10-22) [pr]: ID_SDMI_UPDATESHUTLIST kicks this off
                    /* WinPostMsg(pShutdownData->SDConsts.hwndMain,
                               WM_COMMAND,
                               MPFROM2SHORT(ID_SDMI_CLOSEITEM, 0),
                               MPNULL); */
                }
                break;

                /*
                 * ID_SDMI_CLOSEITEM:
                 *     this msg is posted first upon receiving
                 *     ID_SDDI_BEGINSHUTDOWN and subsequently for every
                 *     window that is to be closed; we only INITIATE
                 *     closing the window here by posting messages
                 *     or killing the window; we then rely on the
                 *     update thread to realize that the window has
                 *     actually been removed from the Tasklist. The
                 *     Update thread then posts ID_SDMI_UPDATESHUTLIST,
                 *     which will then in turn post another ID_SDMI_CLOSEITEM.
                 */

                case ID_SDMI_CLOSEITEM:
                {
                    PSHUTLISTITEM pItem;

                    doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                           "  ID_SDMI_CLOSEITEM");

                    // get task list item to close from linked list
                    if (pItem = xsdQueryCurrentItem(pShutdownData))
                    {
                        CloseOneItem(pShutdownData,
                                     hwndListbox,
                                     pItem);
                    } // end if (pItem)
                    else
                    {
                        // no more items left: enter phase 2 (save WPS)
                        doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                               "    All items closed. Posting WM_QUIT...");

                        G_ulShutdownState = XSD_ALLCLOSED_SAVING;

                        WinPostMsg(pShutdownData->SDConsts.hwndMain,
                                   WM_QUIT,     // V0.9.9 (2001-04-04) [umoeller]
                                   0,
                                   0);
                    }
                }
                break;

                /*
                 * ID_SDDI_SKIPAPP:
                 *     comes from the "Skip" button
                 */

                case ID_SDDI_SKIPAPP:
                {
                    PSHUTLISTITEM   pItem,
                                    pSkipItem;
                    doshWriteLogEntry(pShutdownData->ShutdownLogFile, "  ID_SDDI_SKIPAPP, hwnd: 0x%lX", hwndFrame);

                    pItem = xsdQueryCurrentItem(pShutdownData);
                    if (pItem)
                    {
                        doshWriteLogEntry(pShutdownData->ShutdownLogFile, "    Adding %s to the list of skipped items",
                               pItem->swctl.szSwtitle);

                        if (pShutdownData->fSkippedSemOwned = !DosRequestMutexSem(pShutdownData->hmtxSkipped, 4000))
                        {
                            pSkipItem = malloc(sizeof(SHUTLISTITEM));
                            memcpy(pSkipItem, pItem, sizeof(SHUTLISTITEM));

                            lstAppendItem(&pShutdownData->llSkipped,
                                          pSkipItem);
                            DosReleaseMutexSem(pShutdownData->hmtxSkipped);
                            pShutdownData->fSkippedSemOwned = FALSE;
                        }
                    }

                    // shutdown still running
                    // (started and not all done and not cancelled)?
                    if (G_ulShutdownState == XSD_CLOSINGWINDOWS)
                        WinPostMsg(pShutdownData->SDConsts.hwndMain,
                                   WM_COMMAND,
                                   MPFROM2SHORT(ID_SDMI_UPDATEPROGRESSBAR, 0),
                                   MPNULL);
                }
                break;

                /*
                 * ID_SDMI_CLOSEVIO:
                 *     this is posted by ID_SDMI_CLOSEITEM when
                 *     a non-PM session is encountered; we will now
                 *     either close this session automatically or
                 *     open up a confirmation dlg
                 */

                case ID_SDMI_CLOSEVIO:
                    xsdCloseVIO(pShutdownData,
                                hwndFrame);
                break;

                /*
                 * ID_SDMI_UPDATESHUTLIST:
                 *    this cmd comes from the Update thread when
                 *    the task list has changed. This happens when
                 *    1)    something was closed by this function
                 *    2)    the user has closed something
                 *    3)    and even if the user has OPENED something
                 *          new.
                 *    We will then rebuilt the pliShutdownFirst list and
                 *    continue closing items, if Shutdown is currently in progress
                 */

                case ID_SDMI_UPDATESHUTLIST:
                {
                    doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                           "  ID_SDMI_UPDATESHUTLIST, hwnd: 0x%lX",
                           hwndFrame);

                    xsdUpdateListBox(pShutdownData->habShutdownThread,
                                     pShutdownData,
                                     hwndListbox);
                        // this updates the Shutdown linked list
                    DosPostEventSem(pShutdownData->hevUpdated);
                        // signal update to Update thread

                    doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                           "    Rebuilt shut list, %d items remaining",
                           xsdCountRemainingItems(pShutdownData));

                    if (pShutdownData->SDConsts.hwndVioDlg)
                    {
                        USHORT usItem;
                        // "Close VIO" confirmation window is open:
                        // check if the item that was to be closed
                        // is still in the listbox; if so, exit,
                        // if not, close confirmation window and
                        // continue
                        usItem = (USHORT)WinSendMsg(hwndListbox,
                                                    LM_SEARCHSTRING,
                                                    MPFROM2SHORT(0, LIT_FIRST),
                                                    (MPARAM)pShutdownData->szVioTitle);

                        if ((usItem != (USHORT)LIT_NONE))
                            break;
                        else
                        {
                            WinPostMsg(pShutdownData->SDConsts.hwndVioDlg,
                                       WM_CLOSE,
                                       MPNULL,
                                       MPNULL);
                            // this will result in a DID_CANCEL return code
                            // for WinProcessDlg in ID_SDMI_CLOSEVIO above
                            doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                                   "    Closed open VIO confirm dlg' dlg");
                        }
                    }
                    goto updateprogressbar;
                    // continue with update progress bar
                }

                /*
                 * ID_SDMI_UPDATEPROGRESSBAR:
                 *     well, update the progress bar in the
                 *     status window
                 */

                case ID_SDMI_UPDATEPROGRESSBAR:
                updateprogressbar:
                {
                    ULONG           ulItemCount, ulMax, ulNow;

                    ulItemCount = xsdCountRemainingItems(pShutdownData);
                    if (ulItemCount > pShutdownData->ulMaxItemCount)
                    {
                        pShutdownData->ulMaxItemCount = ulItemCount;
                        pShutdownData->ulLastItemCount = -1; // enforce update
                    }

                    doshWriteLogEntry(pShutdownData->ShutdownLogFile, "  ID_SDMI_UPDATEPROGRESSBAR, hwnd: 0x%lX, remaining: %d, total: %d",
                           hwndFrame, ulItemCount, pShutdownData->ulMaxItemCount);

                    if ((ulItemCount) != pShutdownData->ulLastItemCount)
                    {
                        ulMax = pShutdownData->ulMaxItemCount;
                        ulNow = (ulMax - ulItemCount);

                        WinSendMsg(pShutdownData->hwndProgressBar,
                                   WM_UPDATEPROGRESSBAR,
                                   (MPARAM)ulNow,
                                   (MPARAM)(ulMax + 1));        // add one extra for desktop
                        pShutdownData->ulLastItemCount = (ulItemCount);
                    }

                    if (G_ulShutdownState == XSD_CLOSINGWINDOWS)
                        // if we're already in the process of shutting down, we will
                        // initiate closing the next item
                        WinPostMsg(pShutdownData->SDConsts.hwndMain,
                                   WM_COMMAND,
                                   MPFROM2SHORT(ID_SDMI_CLOSEITEM, 0),
                                   MPNULL);
                }
                break;

                /*
                 * DID_CANCEL:
                 * ID_SDDI_CANCELSHUTDOWN:
                 *
                 */

                case DID_CANCEL:
                case ID_SDDI_CANCELSHUTDOWN:
                    // results from the "Cancel shutdown" buttons in both the
                    // main (debug) and the status window; we set a semaphore
                    // upon which the Update thread will terminate itself,
                    // and WM_QUIT is posted to the main (debug) window, so that
                    // the message loop in the main Shutdown thread function ends
                    if (winhIsDlgItemEnabled(pShutdownData->SDConsts.hwndShutdownStatus,
                                             ID_SDDI_CANCELSHUTDOWN))
                    {
                        // mark shutdown status as cancelled
                        G_ulShutdownState = XSD_CANCELLED;

                        doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                                "  DID_CANCEL/ID_SDDI_CANCELSHUTDOWN, hwnd: 0x%lX",
                                hwndFrame);

                        /* if (pShutdownData->hPOC)
                            ((PPROCESSCONTENTINFO)pShutdownData->hPOC)->fCancelled = TRUE; */
                        WinEnableControl(pShutdownData->SDConsts.hwndShutdownStatus,
                                         ID_SDDI_CANCELSHUTDOWN,
                                         FALSE);
                        WinEnableControl(pShutdownData->SDConsts.hwndShutdownStatus,
                                         ID_SDDI_SKIPAPP,
                                         FALSE);
                        WinEnableControl(pShutdownData->SDConsts.hwndMain,
                                         ID_SDDI_BEGINSHUTDOWN,
                                         TRUE);

                        WinPostMsg(pShutdownData->SDConsts.hwndMain,
                                   WM_QUIT,     // V0.9.9 (2001-04-04) [umoeller]
                                   0,
                                   MPNULL);
                    }
                break;

                default:
                    mrc = WinDefDlgProc(hwndFrame, msg, mp1, mp2);
            } // end switch;
        }
        break;  // end case WM_COMMAND

        // other msgs: have them handled by the usual WC_FRAME wnd proc
        default:
           mrc = WinDefDlgProc(hwndFrame, msg, mp1, mp2);
        break;
    }

    return mrc;
}

/* ******************************************************************
 *
 *   here comes the "Finish" routines
 *
 ********************************************************************/

/*
 *  The following routines are called to finish shutdown
 *  after all windows have been closed and the system is
 *  to be shut down or APM/ACPI power-off'ed or rebooted or
 *  whatever.
 *
 *  All of these routines run on the Shutdown thread.
 */

/*
 *@@ fncbUpdateINIStatus:
 *      this is a callback func for prfhSaveINIs for
 *      updating the progress bar.
 *
 *      Runs on the Shutdown thread.
 */

STATIC MRESULT EXPENTRY fncbUpdateINIStatus(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    if (hwnd)
        WinSendMsg(hwnd, msg, mp1, mp2);
    return (MPARAM)TRUE;
}

/*
 *@@ fncbINIError:
 *      callback func for prfhSaveINIs (/helpers/winh.c).
 *
 *      Runs on the Shutdown thread.
 */

STATIC MRESULT EXPENTRY fncbSaveINIError(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    return (MRESULT)cmnMessageBox(HWND_DESKTOP,
                                  "XShutdown: Error",
                                  (PSZ)mp1,     // error text
                                  NULLHANDLE, // no help
                                  (ULONG)mp2);   // MB_ABORTRETRYIGNORE or something
}

/*
 *@@ xsd_fnSaveINIsProgress:
 *
 *@@added V0.9.5 (2000-08-13) [umoeller]
 */

BOOL _Optlink xsd_fnSaveINIsProgress(ULONG ulUser,
                                           // in: user param specified with
                                           // xprfCopyProfile and xprfSaveINIs
                                           // (progress bar HWND)
                                     ULONG ulProgressNow,
                                           // in: current progress
                                     ULONG ulProgressMax)
                                           // in: maximum progress
{
    HWND hwndProgress = (HWND)ulUser;
    if (hwndProgress)
        WinSendMsg(hwndProgress,
                   WM_UPDATEPROGRESSBAR,
                   (MPARAM)ulProgressNow,
                   (MPARAM)ulProgressMax);
    return TRUE;
}

/*
 *@@ xsdFinishShutdown:
 *      this is the generic routine called by
 *      fntShutdownThread after closing all windows
 *      is complete and the system is to be shut
 *      down, depending on the user settings.
 *
 *      This evaluates the current shutdown settings and
 *      calls xsdFinishStandardReboot, xsdFinishUserReboot,
 *      xsdFinishPowerOff, or xsdFinishStandardMessage.
 *
 *      Note that this is only called for "real" shutdown.
 *      For "Restart Desktop", xsdRestartWPS is called instead.
 *
 *      Runs on the Shutdown thread.
 *
 *@@changed V0.9.3 (2000-05-22) [umoeller]: added reboot animation
 *@@changed V0.9.5 (2000-08-13) [umoeller]: now using new save-INI routines (xprfSaveINIs)
 *@@changed V0.9.12 (2001-05-12) [umoeller]: animations frequently didn't show up, fixed
 */

VOID xsdFinishShutdown(PSHUTDOWNDATA pShutdownData) // HAB hab)
{
    APIRET      arc = NO_ERROR;
    ULONG       ulSaveINIs = 0;

#ifndef __NOXSHUTDOWN__
    ulSaveINIs = cmnQuerySetting(sulSaveINIS);
#endif

    // change the mouse pointer to wait state
    winhSetWaitPointer();

    // enforce saving of INIs
    WinSetDlgItemText(pShutdownData->SDConsts.hwndShutdownStatus, ID_SDDI_STATUS,
                      cmnGetString(ID_SDSI_SAVINGPROFILES)) ; // pszSDSavingProfiles

#ifndef __NOXSHUTDOWN__
    switch (ulSaveINIs)
    {
        case 2:         // do nothing:
            doshWriteLogEntry(pShutdownData->ShutdownLogFile, "Saving INIs has been disabled, skipping.");
        break;

        case 1:     // old method:
            doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                              "Saving INIs, old method (prfhSaveINIs)...");
            arc = prfhSaveINIs(pShutdownData->habShutdownThread,
                               NULL, // pShutdownData->ShutdownLogFile, @@todo
                               (PFNWP)fncbUpdateINIStatus,
                                   // callback function for updating the progress bar
                               pShutdownData->hwndProgressBar,
                                   // status window handle passed to callback
                               WM_UPDATEPROGRESSBAR,
                                   // message passed to callback
                               (PFNWP)fncbSaveINIError);
                                   // callback for errors
            doshWriteLogEntry(pShutdownData->ShutdownLogFile, "Done with prfhSaveINIs.");
        break;

        default:    // includes 0, new method
#endif
            doshWriteLogEntry(pShutdownData->ShutdownLogFile, "Saving INIs, new method (xprfSaveINIs)...");

            arc = xprfSaveINIs(pShutdownData->habShutdownThread,
                               xsd_fnSaveINIsProgress,
                               pShutdownData->hwndProgressBar,
                               NULL,
                               NULL,
                               NULL);
            doshWriteLogEntry(pShutdownData->ShutdownLogFile, "Done with xprfSaveINIs.");
#ifndef __NOXSHUTDOWN__
        break;
    }
#endif

    if (arc != NO_ERROR)
    {
        CHAR    szErrorNo[30];
        XSTRING strErrDescr;
        PCSZ    apcsz[2];
        doshWriteLogEntry(pShutdownData->ShutdownLogFile, "--- Error %d was reported!", arc);

        // error occurred: ask whether to restart the WPS
        sprintf(szErrorNo, "%d", arc);
        apcsz[0] = szErrorNo;
        xstrInit(&strErrDescr, 0);
        cmnDescribeError(&strErrDescr,
                         arc,
                         NULL,
                         TRUE);
        apcsz[1] = strErrDescr.psz;
        if (cmnMessageBoxExt(pShutdownData->SDConsts.hwndShutdownStatus,
                             110,
                             apcsz, 2,      // V1.0.0 (2002-09-17) [umoeller]
                             111,
                             MB_YESNO)
                    == MBID_YES)
        {
            doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                              "User requested to restart Desktop.");
            xsdRestartWPS(pShutdownData->habShutdownThread,
                          FALSE);
                    // doesn't return
        }

        xstrClear(&strErrDescr);
    }

    doshWriteLogEntry(pShutdownData->ShutdownLogFile, "Now preparing shutdown...");

    // always say "releasing filesystems"
    WinShowPointer(HWND_DESKTOP, FALSE);
    WinSetDlgItemText(pShutdownData->SDConsts.hwndShutdownStatus,
                      ID_SDDI_STATUS,
                      cmnGetString(ID_SDSI_FLUSHING)) ; // pszSDFlushing

    // V1.0.4 (2005-06-22) [pr]: Turn off HPFS's Lazy Writer
    if (!pShutdownData->sdParams.optDebug)
    {
        ULONG cbdata, cbparam;

        cbdata = cbparam = 0;
        DosFSCtl (NULL, 0, &cbdata, NULL, 0, &cbparam, 0x9001, "HPFS", -1, FSCTL_FSDNAME);
    }

    // here comes the settings-dependent part:
    // depending on what we are to do after shutdown,
    // we switch to different routines which handle
    // the rest.
    // I guess this is a better solution than putting
    // all this stuff in the same routine because after
    // DosShutdown(), even the swapper file is blocked,
    // and if the following code is in the swapper file,
    // it cannot be executed. From user reports, I suspect
    // this has happened on some memory-constrained
    // systems with XFolder < V0.84. So we better put
    // all the code together which will be used together.

    /* if (pShutdownData->sdParams.optAnimate)
        // hps for animation later
        // moved this here V0.9.12 (2001-05-12) [umoeller]
        hpsScreen = WinGetScreenPS(HWND_DESKTOP); */

    G_ulShutdownState = XSD_SAVEDONE_FLUSHING;

    if (pShutdownData->sdParams.optReboot)
    {
        // reboot:
        if (strlen(pShutdownData->sdParams.szRebootCommand) == 0)
            // no user reboot action:
            xsdFinishStandardReboot(pShutdownData);
        else
            // user reboot action:
            xsdFinishUserReboot(pShutdownData);
    }
    else if (pShutdownData->sdParams.optDebug)
    {
        // debug mode: just sleep a while
        DosSleep(2000);
    }
    else if (pShutdownData->sdParams.optPowerOff)
    {
        // no reboot, but APM/ACPI power off?
        xsdFinishPowerOff(pShutdownData);
    }
    else
        // normal shutdown: show message
        xsdFinishStandardMessage(pShutdownData);

    // the xsdFinish* functions never return;
    // so we only get here in debug mode and
    // return
}

/*
 *@@ PowerOffAnim:
 *      calls anmPowerOff with the proper timings
 *      to display the cute power-off animation.
 *
 *@@added V0.9.7 (2000-12-13) [umoeller]
 */

STATIC VOID PowerOffAnim(HPS hpsScreen)
{
    anmPowerOff(hpsScreen,
                500, 800, 200, 300);
}

/*
 *@@ xsdFinishStandardMessage:
 *      this finishes the shutdown procedure,
 *      displaying the "Shutdown is complete..."
 *      window and halting the system.
 *
 *      Runs on the Shutdown thread.
 *
 *@@changed V0.9.12 (2001-05-12) [umoeller]: animations frequently didn't show up, fixed
 *@@changed V0.9.17 (2002-02-05) [pr]: fix text not displaying by making it a 2 stage message
 *@@changed V1.0.9 (2010-04-25) [ataylor]: fix DBCS shutdown dialog @@fixes 1149
 */

VOID xsdFinishStandardMessage(PSHUTDOWNDATA pShutdownData)
{
    HWND hwndMsg;
    ULONG flShutdown = 0;
    PCSZ pcszComplete = cmnGetString(ID_SDDI_COMPLETE);
    PCSZ pcszSwitchOff = cmnGetString(ID_SDDI_SWITCHOFF);

    HPS hpsScreen = WinGetScreenPS(HWND_DESKTOP);

    // setup Ctrl+Alt+Del message window; this needs to be done
    // before DosShutdown, because we won't be able to reach the
    // resource files after that
    HWND hwndCADMessage = WinLoadDlg(HWND_DESKTOP, NULLHANDLE,
                                     WinDefDlgProc,
                                     pShutdownData->hmodResource,
                                     ID_SDD_CAD,
                                     NULL);
    winhCenterWindow(hwndCADMessage);  // wnd is still invisible

#ifndef __NOXSHUTDOWN__
    flShutdown = cmnQuerySetting(sflXShutdown);
#endif

    if (pShutdownData->ShutdownLogFile)
    {
        doshWriteLogEntry(pShutdownData->ShutdownLogFile, "xsdFinishStandardMessage: Calling DosShutdown(0), closing log.");
        doshClose(&pShutdownData->ShutdownLogFile);
        pShutdownData->ShutdownLogFile = NULL;
    }

    if (flShutdown & XSD_ANIMATE_SHUTDOWN)
        // cute power-off animation
        PowerOffAnim(hpsScreen);
    else
        // only hide the status window if
        // animation is off, because otherwise
        // PM will repaint part of the animation
        WinShowWindow(pShutdownData->SDConsts.hwndShutdownStatus, FALSE);

    // now, this is fun:
    // here's a fine example of how to completely
    // block the system without ANY chance to escape.
    // -- show "Shutdown in progress..." window
    WinShowWindow(hwndCADMessage, TRUE);
    // -- make the CAD message system-modal
    WinSetSysModalWindow(HWND_DESKTOP, hwndCADMessage);
    // -- kill the tasklist (/helpers/winh.c)
    // so that Ctrl+Esc fails
    winhKillTasklist();
    // -- block all other WPS threads
    DosEnterCritSec();
    // -- block all file access
    DosShutdown(0); // V0.9.16 (2002-02-03) [pr]: moved this down
    // -- update the message
    WinSetDlgItemText(hwndCADMessage, ID_SDDI_PROGRESS1, (PSZ)pcszComplete);
    WinSetDlgItemText(hwndCADMessage, ID_SDDI_PROGRESS2, (PSZ)pcszSwitchOff);
    // -- display the shutdown message
    if (hwndMsg = WinWindowFromID(hwndCADMessage, ID_SDDI_PROGRESS0))
        WinShowWindow(hwndMsg, FALSE);
    // -- and now loop forever!
    while (TRUE)
        DosSleep(10000);
}

/*
 *@@ xsdFinishStandardReboot:
 *      this finishes the shutdown procedure,
 *      rebooting the computer using the standard
 *      reboot procedure (DOS.SYS).
 *      There's no shutdown animation here.
 *
 *      Runs on the Shutdown thread.
 *
 *@@changed V0.9.3 (2000-05-22) [umoeller]: added reboot animation
 *@@changed V0.9.12 (2001-05-12) [umoeller]: animations frequently didn't show up, fixed
 *@@changed V0.9.16 (2002-01-05) [umoeller]: fixed hang on loading string resource
 *@@changed V1.0.10 (2015-01-03) [at]: fix hang on DBCS systems
 */

VOID xsdFinishStandardReboot(PSHUTDOWNDATA pShutdownData)
{
    ULONG       flShutdown = 0;
    HFILE       hIOCTL;
    ULONG       ulAction;
    BOOL        fShowRebooting = TRUE;
    BOOL        fIsDBCS = FALSE;
    // load string resource before shutting down
    // V0.9.16 (2002-01-05) [umoeller]
    PCSZ        pcszRebooting = cmnGetString(ID_SDSI_REBOOTING);
    HPS         hpsScreen = WinGetScreenPS(HWND_DESKTOP);
    PCSZ        pcszLang;

#ifndef __NOXSHUTDOWN__
    flShutdown = cmnQuerySetting(sflXShutdown);
#endif

    // set the DBCS flag if using a CJK NLV - this is used below to prevent
    // trying to update the reboot dialog text after DosShutdown() returns
    // (as loading DBCS glyphs from file would fail, possibly causing a hang)
    pcszLang = cmnQueryLanguageCode();
    if (   !strncmp(pcszLang, "081", 3)     // Japanese
        || !strncmp(pcszLang, "082", 3)     // Korean (maybe not necessary?)
        || !strncmp(pcszLang, "086", 3)     // S.Chinese
        || !strncmp(pcszLang, "088", 3)    // T.Chinese
       )
    {
        fIsDBCS = TRUE;
    }

    // if (optReboot), open DOS.SYS; this
    // needs to be done before DosShutdown() also
    if (DosOpen("\\DEV\\DOS$", &hIOCTL, &ulAction, 0L,
               FILE_NORMAL,
               OPEN_ACTION_OPEN_IF_EXISTS,
               OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE,
               0L)
        != NO_ERROR)
    {
        winhDebugBox(HWND_DESKTOP, "XShutdown", "The DOS.SYS device driver could not be opened. "
                 "XShutdown will be unable to reboot your computer. "
                 "Please consult the XFolder Online Reference for a remedy of this problem.");
    }

    if (pShutdownData->ShutdownLogFile)
    {
        doshWriteLogEntry(pShutdownData->ShutdownLogFile, "xsdFinishStandardReboot: Opened DOS.SYS, hFile: 0x%lX", hIOCTL);
        doshWriteLogEntry(pShutdownData->ShutdownLogFile, "xsdFinishStandardReboot: Calling DosShutdown(0), closing log.");
        doshClose(&pShutdownData->ShutdownLogFile);
        pShutdownData->ShutdownLogFile = NULL;
    }

    if (flShutdown & XSD_ANIMATE_REBOOT)  // V0.9.3 (2000-05-22) [umoeller]
    {
        // cute power-off animation
        PowerOffAnim(hpsScreen);
        fShowRebooting = FALSE;
    }

    DosShutdown(0);
        // @@todo what to do if this fails?

    // say "Rebooting..." if we had no animation
    if (fShowRebooting && !fIsDBCS)
    {
        WinSetDlgItemText(pShutdownData->SDConsts.hwndShutdownStatus,
                          ID_SDDI_STATUS,
                          (PSZ)pcszRebooting);
        DosSleep(500);
    }

    // restart the machine via DOS.SYS (opened above)
    DosDevIOCtl(hIOCTL, 0xd5, 0xab, NULL, 0, NULL, NULL, 0, NULL);

    // don't know if this function returns, but
    // we better make sure we don't return from this
    // function
    while (TRUE)
        DosSleep(10000);
}

/*
 *@@ xsdFinishUserReboot:
 *      this finishes the shutdown procedure,
 *      rebooting the computer by starting a
 *      user reboot command.
 *      There's no shutdown animation here.
 *
 *      Runs on the Shutdown thread.
 *
 *@@changed V0.9.3 (2000-05-22) [umoeller]: added reboot animation
 *@@changed V0.9.12 (2001-05-12) [umoeller]: animations frequently didn't show up, fixed
 *@@changed V1.0.5 (2006-04-01) [pr]: added title parameter to doshQuickStartSession
 */

VOID xsdFinishUserReboot(PSHUTDOWNDATA pShutdownData)
{
    // user reboot item: in this case, we don't call
    // DosShutdown(), which is supposed to be done by
    // the user reboot command
    ULONG   flShutdown = 0;
    CHAR    szTemp[CCHMAXPATH];
    PID     pid;
    ULONG   sid;
    HPS hpsScreen = WinGetScreenPS(HWND_DESKTOP);

#ifndef __NOXSHUTDOWN__
    flShutdown = cmnQuerySetting(sflXShutdown);
#endif

    if (flShutdown & XSD_ANIMATE_REBOOT)        // V0.9.3 (2000-05-22) [umoeller]
        // cute power-off animation
        PowerOffAnim(hpsScreen);
    else
    {
        sprintf(szTemp,
                cmnGetString(ID_SDSI_STARTING),  // pszStarting
                pShutdownData->sdParams.szRebootCommand);
        WinSetDlgItemText(pShutdownData->SDConsts.hwndShutdownStatus, ID_SDDI_STATUS,
                          szTemp);
    }

    if (pShutdownData->ShutdownLogFile)
    {
        doshWriteLogEntry(pShutdownData->ShutdownLogFile, "Trying to start user reboot cmd: %s, closing log.",
               pShutdownData->sdParams.szRebootCommand);
        doshClose(&pShutdownData->ShutdownLogFile);
        pShutdownData->ShutdownLogFile = NULL;
    }

    sprintf(szTemp, "/c %s", pShutdownData->sdParams.szRebootCommand);
    if (doshQuickStartSession("cmd.exe",
                              szTemp,
                              szTemp + 3,            // title
                              SSF_TYPE_DEFAULT,      // session type
                              FALSE,                 // background
                              SSF_CONTROL_INVISIBLE, // but auto-close
                              TRUE,                  // wait flag
                              &sid,
                              &pid,
                              NULL)
               != NO_ERROR)
    {
        winhDebugBox(HWND_DESKTOP,
                     "XShutdown",
                     "The user-defined restart command failed. We will now restart the WPS.");
        xsdRestartWPS(pShutdownData->habShutdownThread,
                      FALSE);
    }
    else
        // no error:
        // we'll always get here, since the user reboot
        // is now taking place in a different process, so
        // we just stop here
        while (TRUE)
            DosSleep(10000);
}

/*
 *@@ xsdFinishPowerOff:
 *      this finishes the shutdown procedure,
 *      using the functions in apm.c.
 *
 *      Runs on the Shutdown thread.
 *
 *@@changed V0.9.12 (2001-05-12) [umoeller]: animations frequently didn't show up, fixed
 *@@changed V1.0.5 (2006-06-26) [pr]: moved code from apmDoPowerOff()
 *@@changed V1.0.8 (2007-04-13) [pr]: reinstated APM_DOSSHUTDOWN_0 @@fixes 726
 *@@changed V1.0.9 (2008-12-19) [pr]: moved animation above DosShutdown
 */

VOID xsdFinishPowerOff(PSHUTDOWNDATA pShutdownData)
{
    CHAR        szError[500];
    ULONG       ulrc = 0;
    ULONG       flShutdown = 0;
    HPS         hpsScreen = WinGetScreenPS(HWND_DESKTOP);

#ifndef __NOXSHUTDOWN__
    flShutdown = cmnQuerySetting(sflXShutdown);
#endif

    // prepare power off
    if (pShutdownData->sdParams.optACPIOff)
    {
        ulrc = acpiPreparePowerOff(szError);
        doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                          "xsdFinishPowerOff: acpiPreparePowerOff returned 0x%lX",
                          ulrc);
    }
    else
    {
        ulrc = apmPreparePowerOff(szError);
        doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                          "xsdFinishPowerOff: apmPreparePowerOff returned 0x%lX",
                          ulrc);
    }

    if (ulrc & APM_IGNORE)
    {
        // if this returns APM_IGNORE, we continue
        // with the regular shutdown w/out APM/ACPI
        doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                          "APM_IGNORE, continuing with normal shutdown",
                          ulrc);

        xsdFinishStandardMessage(pShutdownData);
        // this does not return
    }
    else if (ulrc & APM_CANCEL)
    {
        // APM_CANCEL means cancel shutdown
        WinShowPointer(HWND_DESKTOP, TRUE);
        doshWriteLogEntry(pShutdownData->ShutdownLogFile,
                          "  APM/ACPI error message: %s",
                          szError);

        cmnMessageBox(HWND_DESKTOP,
                      "APM/ACPI Error",
                      szError,
                      NULLHANDLE, // no help
                      MB_CANCEL | MB_SYSTEMMODAL);
        // restart Desktop
        xsdRestartWPS(pShutdownData->habShutdownThread,
                      FALSE);
        // this does not return
    }
    // else: APM_OK means preparing went alright

    // V1.0.9 (2008-12-19) [pr]: moved this above DosShutdown
    if (flShutdown & XSD_ANIMATE_SHUTDOWN)
        // cute power-off animation
        PowerOffAnim(hpsScreen);
    else
        // only hide the status window if
        // animation is off, because otherwise
        // we get screen display errors
        // (hpsScreen...)
        WinShowWindow(pShutdownData->SDConsts.hwndShutdownStatus, FALSE);

    if (ulrc & APM_DOSSHUTDOWN_0)
    {
        // shutdown request by apm.c:
        if (pShutdownData->ShutdownLogFile)
        {
            doshWriteLogEntry(pShutdownData->ShutdownLogFile, "xsdFinishPowerOff: Calling DosShutdown(0), closing log.");
            doshClose(&pShutdownData->ShutdownLogFile);
            pShutdownData->ShutdownLogFile = NULL;
        }

        DosShutdown(0);
    }
    // if apmPreparePowerOff/acpiPreparePowerOff requested this,
    // do DosShutdown(1)
    if (ulrc & APM_DOSSHUTDOWN_1)
    {
        if (pShutdownData->ShutdownLogFile)
        {
            doshWriteLogEntry(pShutdownData->ShutdownLogFile, "xsdFinishPowerOff: Calling DosShutdown(1), closing log.");
            doshClose(&pShutdownData->ShutdownLogFile);
            pShutdownData->ShutdownLogFile = NULL;
        }

        DosShutdown(1);
    }
    // or if apmPreparePowerOff/acpiPreparePowerOff requested this,
    // do no DosShutdown()

    if (pShutdownData->ShutdownLogFile)
    {
        doshClose(&pShutdownData->ShutdownLogFile);
        pShutdownData->ShutdownLogFile = NULL;
    }

    // V1.0.5 (2006-06-26) [pr]
    if (pShutdownData->sdParams.optDelay)
    {
        ULONG   ul;

        // try another pause of 3 seconds; maybe DosShutdown is
        // still running V0.9.2 (2000-02-29) [umoeller]
        for (ul = 0;
             ul < 3;
             ul++)
        {
            DosBeep(4000, 10);
            DosSleep(1000);
        }
        DosBeep(8000, 10);
    }

    // if power-off was properly prepared above,
    // we will now call the function which
    // actually turns the power off
    // V1.0.5 (2006-06-26) [pr]
    if (pShutdownData->sdParams.optACPIOff)
        acpiDoPowerOff();
    else
        apmDoPowerOff();

    // OS/2 _does_ return from the above functions, but in the
    // background APM/ACPI power-off is now being executed.
    // So we must now loop forever until power-off has
    // completed, when the computer will just stop
    // executing anything.
    // We must _not_ return to the XFolder shutdown routines,
    // because XFolder would then enter a critical section,
    // which would keep power-off from working.
    while (TRUE)
        DosSleep(10000);
}

/* ******************************************************************
 *
 *   Update thread
 *
 ********************************************************************/

/*
 *@@ fntUpdateThread:
 *          this thread is responsible for monitoring the window list
 *          while XShutdown is running and windows are being closed.
 *
 *          It is created with a PM message queue from fntShutdown
 *          when shutdown is about to begin.
 *
 *          It builds an internal second PSHUTLISTITEM linked list
 *          from the PM window list every 100 ms and then compares
 *          it to the global pliShutdownFirst.
 *
 *          If they are different, this means that windows have been
 *          closed (or even maybe opened), so fnwpShutdownThread is posted
 *          ID_SDMI_UPDATESHUTLIST so that it can rebuild pliShutdownFirst
 *          and react accordingly, in most cases, close the next window.
 *
 *          This thread monitors its THREADINFO.fExit flag and exits if
 *          it is set to TRUE. See thrClose (threads.c) for how this works.
 *
 *          Global resources used by this thread:
 *          -- HEV hevUpdated: event semaphore posted by Shutdown thread
 *                             when it's done updating its shutitem list.
 *                             We wait for this semaphore after having
 *                             posted ID_SDMI_UPDATESHUTLIST.
 *          -- PLINKLIST pllShutdown: the global list of SHUTLISTITEMs
 *                                    which are left to be closed. This is
 *                                    also used by the Shutdown thread.
 *          -- HMTX hmtxShutdown: mutex semaphore for protecting pllShutdown.
 *
 *@@changed V0.9.0 [umoeller]: code has been re-ordered for semaphore safety.
 */

STATIC void _Optlink fntUpdateThread(PTHREADINFO ptiMyself)
{
    PSZ             pszErrMsg = NULL;

    PSHUTDOWNDATA   pShutdownData = (PSHUTDOWNDATA)ptiMyself->ulData;

    BOOL            fLocked = FALSE;
    LINKLIST        llTestList;

    DosSetPriority(PRTYS_THREAD,
                   PRTYC_REGULAR,
                   +31,          // priority delta
                   0);           // this thread


    lstInit(&llTestList, TRUE);     // auto-free items

    TRY_LOUD(excpt1)
    {
        ULONG           ulShutItemCount = 0,
                        ulTestItemCount = 0;
        BOOL            fUpdated = FALSE;

        WinCancelShutdown(ptiMyself->hmq, TRUE);

        while (!G_tiUpdateThread.fExit)
        {
            ULONG ulDummy;

            // this is the first loop: we arrive here every time
            // the task list has changed */
            PMPF_SHUTDOWN(("UT: Waiting for update..." ));

            DosResetEventSem(pShutdownData->hevUpdated, &ulDummy);
                        // V0.9.9 (2001-04-04) [umoeller]

            // have Shutdown thread update its list of items then
            WinPostMsg(pShutdownData->SDConsts.hwndMain, WM_COMMAND,
                       MPFROM2SHORT(ID_SDMI_UPDATESHUTLIST, 0),
                       MPNULL);
            fUpdated = FALSE;

            // shutdown thread is waiting for us to post the
            // "ready" semaphore, so do this now
            DosPostEventSem(ptiMyself->hevRunning);

            // now wait until Shutdown thread is done updating its
            // list; it then posts an event semaphore
            while (     (!fUpdated)
                    &&  (!G_tiUpdateThread.fExit)
                  )
            {
                if (G_tiUpdateThread.fExit)
                {
                    // we're supposed to exit:
                    fUpdated = TRUE;
                    PMPF_SHUTDOWN(("UT: Exit recognized" ));
                }
                else
                {
                    ULONG   ulUpdate;
                    // query event semaphore post count
                    DosQueryEventSem(pShutdownData->hevUpdated, &ulUpdate);
                    fUpdated = (ulUpdate > 0);
                    PMPF_SHUTDOWN(("UT: update recognized" ));

                    DosSleep(100);
                }
            } // while (!fUpdated);

            ulTestItemCount = 0;
            ulShutItemCount = 0;

            PMPF_SHUTDOWN(("UT: Waiting for task list change, loop 2..." ));

            while (     (ulTestItemCount == ulShutItemCount)
                     && (!G_tiUpdateThread.fExit)
                  )
            {
                // this is the second loop: we stay in here until the
                // task list has changed; for monitoring this, we create
                // a second task item list similar to the pliShutdownFirst
                // list and compare the two
                lstClear(&llTestList);

                // create a test list for comparing the task list;
                // this is our private list, so we need no mutex
                // semaphore
                xsdBuildShutList(ptiMyself->hab,
                                 pShutdownData,
                                 &llTestList);

                // count items in the test list
                ulTestItemCount = lstCountItems(&llTestList);

                // count items in the list of the Shutdown thread;
                // here we need a mutex semaphore, because the
                // Shutdown thread might be working on this too
                TRY_LOUD(excpt2)
                {
                    if (fLocked = !DosRequestMutexSem(pShutdownData->hmtxShutdown, 4000))
                    {
                        ulShutItemCount = lstCountItems(&pShutdownData->llShutdown);
                        DosReleaseMutexSem(pShutdownData->hmtxShutdown);
                        fLocked = FALSE;
                    }
                }
                CATCH(excpt2) {} END_CATCH();

                if (!G_tiUpdateThread.fExit)
                    DosSleep(100);
            } // end while; loop until either the Shutdown thread has set the
              // Exit flag or the list has changed

            PMPF_SHUTDOWN(("UT: Change or exit recognized" ));

        }  // end while; loop until exit flag set
    } // end TRY_LOUD(excpt1)
    CATCH(excpt1)
    {
        // exception occurred:
        // complain to the user
        if (pszErrMsg == NULL)
        {
            if (fLocked)
            {
                DosReleaseMutexSem(pShutdownData->hmtxShutdown);
                fLocked = FALSE;
            }

            // only report the first error, or otherwise we will
            // jam the system with msg boxes @@todo get rid of this
            if (pszErrMsg = strdup("An error occurred in the XFolder Update thread. "
                        "In the root directory of your boot drive, you will find a "
                        "file named XFLDTRAP.LOG, which contains debugging information. "
                        "If you had shutdown logging enabled, you will also find the "
                        "file XSHUTDWN.LOG there. If not, please enable shutdown "
                        "logging in the Desktop's settings notebook. "))
            {
                krnPostThread1ObjectMsg(T1M_EXCEPTIONCAUGHT, (MPARAM)pszErrMsg,
                        (MPARAM)0); // don't enforce Desktop restart

                doshWriteLogEntry(pShutdownData->ShutdownLogFile, "\n*** CRASH\n%s\n", pszErrMsg);
            }
        }
    } END_CATCH();

    // clean up
    PMPF_SHUTDOWN(("UT: Exiting..." ));

    if (fLocked)
    {
        // release our mutex semaphore
        DosReleaseMutexSem(pShutdownData->hmtxShutdown);
        fLocked = FALSE;
    }

    lstClear(&llTestList);
}


