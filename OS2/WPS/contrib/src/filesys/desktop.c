
/*
 *@@sourcefile desktop.c:
 *      XFldDesktop implementation code. Note that more
 *      Desktop-related code resides in config\archives.c
 *      and startshut\shutdown.c.
 *
 *      This file is ALL new with V0.9.0.
 *
 *      Function prefix for this file:
 *      --  dtp*
 *
 *@@added V0.9.0 [umoeller]
 *@@header "filesys\desktop.h"
 */

/*
 *      Copyright (C) 1997-2014 Ulrich M”ller.
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
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSERRORS
#define INCL_DOSMISC

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINPOINTERS
#define INCL_WINMENUS
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINLISTBOXES
#define INCL_WINSTDCNR
#define INCL_WINSTDSPIN
#define INCL_WINSHELLDATA       // Prf* functions
#define INCL_WINSTDFILE
#define INCL_WINSYS

#define INCL_GPIBITMAPS
#include <os2.h>

// C library headers
#include <stdio.h>
#include <io.h>
#include <math.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\animate.h"            // icon and other animations
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\shapewin.h"           // shaped windows helper functions
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\syssound.h"           // system sound helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\wphandle.h"           // file-system object handles

// SOM headers which don't crash with prec. header files
#include "xfdesk.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\notebook.h"            // generic XWorkplace notebook handling

#include "filesys\desktop.h"            // XFldDesktop implementation
#include "filesys\fdrmenus.h"           // shared folder menu logic
#include "filesys\xthreads.h"           // extra XWorkplace threads

#include "startshut\shutdown.h"         // XWorkplace eXtended Shutdown

// other SOM headers
#pragma hdrstop

/* ******************************************************************
 *
 *   Query setup strings
 *
 ********************************************************************/

/*
 *@@ dtpSetup:
 *      implementation of XFldDesktop::wpSetup.
 *
 *      This parses the XSHUTDOWNNOW setup string to
 *      start XShutdown now, if needed.
 *
 *@@added V0.9.7 (2001-01-25) [umoeller]
 *@@changed V0.9.14 (2001-07-28) [umoeller]: added SHOWRUNDLG
 *@@changed V0.9.20 (2002-07-03) [umoeller]: added POSTSHUTDOWN
 *@@changed V0.9.20 (2002-07-12) [umoeller]: added SHOWHELPPANEL
 */

BOOL dtpSetup(WPDesktop *somSelf,
              PCSZ pcszSetupString)
{
    BOOL brc = TRUE;

    CHAR szValue[500];
    ULONG cbValue = sizeof(szValue);
    if (_wpScanSetupString(somSelf,
                           (PSZ)pcszSetupString,
                           "XSHUTDOWNNOW",
                           szValue,
                           &cbValue))
    {
        // XSHUTDOWNNOW setup string present:
        // well, start shutdown now.
        // This is a bit tricky, because we want to support
        // overriding the default shutdown settings for this
        // one time... as a result, we fill a SHUTDOWNPARAMS
        // structure with the current settings and override
        // them when parsing szValue now.

        SHUTDOWNPARAMS xsd;
        PSZ pszToken;
        xsdQueryShutdownSettings(&xsd);

        // convert params to upper case
        nlsUpper(szValue);

        pszToken = strtok(szValue, ", ");
        if (pszToken)
            do
            {
                if (!strcmp(pszToken, "HALT"))
                {
                    // xsd.ulRestartWPS = 0;           // shutdown
                    xsd.ulCloseMode = SHUT_SHUTDOWN;
                    xsd.optReboot = FALSE;
                    xsd.optPowerOff = FALSE;  // V1.0.5 (2006-06-26) [pr]
                }
                else if (!strcmp(pszToken, "REBOOT"))
                {
                    // xsd.ulRestartWPS = 0;           // shutdown
                    xsd.ulCloseMode = SHUT_SHUTDOWN;
                    xsd.optReboot = TRUE;
                    xsd.optPowerOff = FALSE;  // V1.0.5 (2006-06-26) [pr]
                }
                else if (!strncmp(pszToken, "USERREBOOT(", 11))
                {
                    PSZ p = strchr(pszToken, ')');
                    if (p)
                    {
                        PSZ pszCmd = strhSubstr(pszToken + 11,
                                                p);
                        if (pszCmd)
                        {
                            strhncpy0(xsd.szRebootCommand,
                                      pszCmd,
                                      sizeof(xsd.szRebootCommand));
                            free(pszCmd);
                        }
                    }
                    // xsd.ulRestartWPS = 0;           // shutdown
                    xsd.ulCloseMode = SHUT_SHUTDOWN;
                    xsd.optReboot = TRUE;
                    xsd.optPowerOff = FALSE;  // V1.0.5 (2006-06-26) [pr]
                }
                else if (!strcmp(pszToken, "POWEROFF"))
                {
                    // xsd.ulRestartWPS = 0;           // shutdown
                    xsd.ulCloseMode = SHUT_SHUTDOWN;
                    xsd.optReboot = FALSE;
                    xsd.optPowerOff = TRUE;  // V1.0.5 (2006-06-26) [pr]
                }
                else if (!strcmp(pszToken, "RESTARTWPS"))
                {
                    // xsd.ulRestartWPS = 1;           // restart Desktop
                    xsd.ulCloseMode = SHUT_RESTARTWPS;
                    xsd.optWPSCloseWindows = FALSE;
                    xsd.optWPSReuseStartupFolder = FALSE;
                }
                else if (!strcmp(pszToken, "FULLRESTARTWPS"))
                {
                    // xsd.ulRestartWPS = 1;           // restart Desktop
                    xsd.ulCloseMode = SHUT_RESTARTWPS;
                    xsd.optWPSCloseWindows = TRUE;
                    xsd.optWPSReuseStartupFolder = TRUE;
                }
                else if (!strcmp(pszToken, "NOAUTOCLOSEVIO"))
                    xsd.optAutoCloseVIO = FALSE;
                else if (!strcmp(pszToken, "AUTOCLOSEVIO"))
                    xsd.optAutoCloseVIO = TRUE;
                else if (!strcmp(pszToken, "NOLOG"))
                    xsd.optLog = FALSE;
                else if (!strcmp(pszToken, "LOG"))
                    xsd.optLog = TRUE;
                /* else if (!strcmp(pszToken, "NOANIMATE"))
                    xsd.optAnimate = FALSE;
                else if (!strcmp(pszToken, "ANIMATE"))
                    xsd.optAnimate = TRUE; */
                else if (!strcmp(pszToken, "NOCONFIRM"))
                    xsd.optConfirm = FALSE;
                else if (!strcmp(pszToken, "CONFIRM"))
                    xsd.optConfirm = TRUE;

            } while (pszToken = strtok(NULL, ", "));

        brc = xsdInitiateShutdownExt(&xsd);
    }
    // added POSTSHUTDOWN string to support eStylerLite
    // shutdown; if we use MENUITEMSELECTED=704, it cannot
    // intercept the shutdown, so we have to post a WM_COMMAND
    // instead
    // V0.9.20 (2002-07-03) [umoeller]
    else if (_wpScanSetupString(somSelf,
                                (PSZ)pcszSetupString,
                                "POSTSHUTDOWN",
                                szValue,
                                &cbValue))
    {
        WinPostMsg(cmnQueryActiveDesktopHWND(),
                   WM_COMMAND,
                   MPFROMSHORT(WPMENUID_SHUTDOWN),
                   MPFROM2SHORT(CMDSRC_MENU,
                                FALSE));
    }

    // V0.9.14 (2001-07-28) [umoeller]
    cbValue = sizeof(szValue);
    if (_wpScanSetupString(somSelf,
                           (PSZ)pcszSetupString,
                           "SHOWRUNDLG",
                           szValue,
                           &cbValue))
    {
        PSZ pszStartup = NULL;
        if (strcmp(szValue, "DEFAULT"))         // boot drive
            pszStartup = szValue;
        brc = (cmnRunCommandLine(NULLHANDLE,           // active desktop
                                 pszStartup)
                    != NULLHANDLE);
    }

    if (_wpScanSetupString(somSelf,
                           (PSZ)pcszSetupString,
                           "TESTFILEDLG",
                           szValue,
                           &cbValue))
    {
        CHAR    szFullFile[CCHMAXPATH] = "";
        strlcpy(szFullFile, szValue, sizeof(szFullFile));
        if (cmnFileDlg2(cmnQueryActiveDesktopHWND(),
                        szFullFile,
                        0,
                        NULLHANDLE,
                        NULL,
                        NULL,
                        TRUE))      // force use of new file dlg
            winhDebugBox(NULLHANDLE,
                         "Test file dlg",
                         szFullFile);
    }

    // SHOWHELPPANEL=[XWP|filename,]panelid
    // V0.9.20 (2002-07-12) [umoeller]
    if (_wpScanSetupString(somSelf,
                           (PSZ)pcszSetupString,
                           "SHOWHELPPANEL",
                           szValue,
                           &cbValue))
    {
        PSZ     p;
        PCSZ    pcszHelpLibrary = NULL,      // null means WPHELP.HLP
                pcszPanelId = szValue;
        ULONG   ulPanelId;
        if (p = strchr(szValue, ','))
        {
            *p = '\0';
            if (!strcmp(szValue, "XWP"))
                pcszHelpLibrary = cmnQueryHelpLibrary();
            else
                pcszHelpLibrary = szValue;
            pcszPanelId = p + 1;
        }

        if (ulPanelId = atoi(pcszPanelId))
        {
            brc = _wpDisplayHelp(somSelf,
                                 ulPanelId,
                                 (PSZ)pcszHelpLibrary);  // can be NULL for WPHELP.HLP
        }
    }

    return brc;
}

/*
 *@@ dtpQuerySetup:
 *      implementation of XFldDesktop::xwpQuerySetup2.
 *      See remarks there.
 *
 *      This returns the length of the XFldDesktop
 *      setup string part only.
 *
 *@@added V0.9.1 (2000-01-20) [umoeller]
 *@@todo warp4 setup strings
 */

BOOL dtpQuerySetup(WPDesktop *somSelf,
                   PVOID pstrSetup)
{
    // PSZ     pszTemp = NULL;
    // ULONG   ulValue = 0;
            // ulDefaultValue = 0;

    /*

      @@todo This is the complete list of all WPDesktop setup
      strings, as documented by WPSREF. However, method
      implementations only exist for Warp 4.

      We'd need to manually decode what all the settings
      in PM_Lockup in OS2.INI are good for.

    */


    // AUTOLOCKUP=YES/NO
    /* if (_wpQueryAutoLockup(somSelf))
        xstrcat(&pszTemp, "AUTOLOCKUP=YES");

    // LOCKUPAUTODIM=YES/NO
    if (_wpQueryLockupAutoDim(somSelf) == FALSE)
        xstrcat(&pszTemp, "LOCKUPAUTODIM=NO");

    // LOCKUPBACKGROUND

    // LOCKUPFULLSCREEN
    if (_wpQueryLockupFullScreen(somSelf) == FALSE)
        xstrcat(&pszTemp, "LOCKUPFULLSCREEN=NO");

    // LOCKUPONSTARTUP
    if (_wpQueryLockupOnStart(somSelf))
        xstrcat(&pszTemp, "LOCKUPONSTARTUP=YES");

    _wpQueryLockupBackground();

    // LOCKUPTIMEOUT
    ulValue = _wpQueryLockupTimeout(somSelf);
    if (ulValue != 3)
    {
        CHAR szTemp[300];
        sprintf(szTemp, "LOCKUPTIMEOUT=%d", ulValue);
        xstrcat(&pszTemp, szTemp);
    } */

    /*
     * append string
     *
     */

    /* if (pszTemp)
    {
        // return string if buffer is given
        if ( (pszSetupString) && (cbSetupString) )
            strhncpy0(pszSetupString,   // target
                      pszTemp,          // source
                      cbSetupString);   // buffer size

        // always return length of string
        ulReturn = strlen(pszTemp);
        free(pszTemp);
    } */

    return TRUE;
}

/* ******************************************************************
 *
 *   Desktop menus
 *
 ********************************************************************/

/*
 *@@ dtpModifyPopupMenu:
 *      implementation for XFldDesktop::wpModifyPopupMenu.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.3 (2000-04-26) [umoeller]: changed shutdown menu IDs for launchpad
 *@@changed V0.9.7 (2000-12-13) [umoeller]: changed shutdown menu items
 *@@changed V0.9.7 (2000-12-13) [umoeller]: added "logoff network now"
 *@@changed V0.9.9 (2001-03-09) [umoeller]: "shutdown" wasn't always disabled if running
 */

VOID dtpModifyPopupMenu(WPDesktop *somSelf,
                        HWND hwndMenu)
{
    HWND            hwndMenuInsert = hwndMenu;
    // PCKERNELGLOBALS  pKernelGlobals = krnQueryGlobals();

    // position of original "Shutdown" menu item in context menu
    SHORT   sOrigShutdownPos = (SHORT)WinSendMsg(hwndMenu,
                                                 MM_ITEMPOSITIONFROMID,
                                                 MPFROM2SHORT(WPMENUID_SHUTDOWN, FALSE),
                                                 MPNULL);

    BOOL fShutdownRunning = xsdQueryShutdownState() != XSD_IDLE;
    ULONG ulShutdownAttr = 0;

    ULONG fl = cmnQuerySetting(mnuQueryMenuXWPSetting(somSelf));  // V1.0.0 (2002-10-11) [pr]

    if (fShutdownRunning)
        // disable all those menu items if XShutdown is currently running
        ulShutdownAttr = MIA_DISABLED;

#ifndef __NOXSHUTDOWN__
    if (    (cmnQuerySetting(sfXShutdown))    // XShutdown enabled?
         // && (!cmnQuerySetting(sNoWorkerThread))  // Worker thread enabled?
            // removed this setting V0.9.16 (2002-01-04) [umoeller]
       )
    {
/* shutdown menu no longer supported V0.9.19 (2002-04-17) [umoeller]
        if (cmnQuerySetting(sfDTMShutdownMenu))
        {
            CHAR szShutdown[50];

            // remove original shutdown item
            winhDeleteMenuItem(hwndMenu, WPMENUID_SHUTDOWN);

            strcpy(szShutdown, cmnGetString(ID_XSSI_XSHUTDOWN));
            if (!(cmnQuerySetting(sflXShutdown) & XSD_NOCONFIRM))
                strcat(szShutdown, "...");

            // create "Shutdown" submenu and use this for
            // subsequent menu items
            hwndMenuInsert
                = winhInsertSubmenu(hwndMenu,
                                    // submenu position: after existing "Shutdown" item
                                    sOrigShutdownPos + 1,
                                    ulOfs + ID_XFM_OFS_SHUTDOWNMENU,
                                    cmnGetString(ID_SDSI_SHUTDOWN),  // pszShutdown
                                    MIS_TEXT,
                                    // add "shutdown" menu item with original WPMENUID_SHUTDOWN;
                                    // this is intercepted in dtpMenuItemSelected to initiate
                                    // XShutdown:
                                    WPMENUID_SHUTDOWN,
                                    szShutdown,
                                    MIS_TEXT,
                                    // disable if Shutdown is currently running
                                    ulShutdownAttr);

            // default shutdown menu item enabled?
            if (cmnQuerySetting(sfDTMShutdown))
                // yes: insert "default shutdown" before that
                winhInsertMenuItem(hwndMenuInsert,
                                   0,       // index
                                   ulOfs + ID_XFMI_OFS_OS2_SHUTDOWN,
                                               // WPMENUID_SHUTDOWN,
                                               // changed V0.9.3 (2000-04-26) [umoeller]
                                   cmnGetString(ID_XSSI_DEFAULTSHUTDOWN),  // "Default OS/2 shutdown...", // pszDefaultShutdown
                                   MIS_TEXT,
                                   // disable if Shutdown is currently running
                                   ulShutdownAttr);

            // append "restart Desktop" to the end
            sOrigShutdownPos = MIT_END;
        } // end if (cmnQuerySetting(sDTMShutdownMenu))
        else
*/

        // V1.0.10 (2014-04-17) [pr]
#ifndef __EASYSHUTDOWN__
        if ((cmnQuerySetting(sflXShutdown) & XSD_NOCONFIRM))
            // if XShutdown confirmations have been disabled,
            // remove "..." from "Shut down" entry
            winhMenuRemoveEllipse(hwndMenu,
                                  WPMENUID_SHUTDOWN);
#endif
        // disable "shutdown" if shutdown is running
        // V0.9.9 (2001-03-07) [umoeller]
        if (fShutdownRunning)
            WinEnableMenuItem(hwndMenu,
                              WPMENUID_SHUTDOWN,
                              FALSE);

    } // end if (cmnQuerySetting(sfXShutdown)) ...

    if (   (!(fl & XWPCTXT_RESTARTWPS))  // V1.0.0 (2002-10-11) [pr]
        && cmnQuerySetting(sfRestartDesktop))
    {
        // insert "Restart Desktop"
        winhInsertMenuItem(hwndMenuInsert,  // either main menu or "Shutdown" submenu
                           sOrigShutdownPos,  // either MIT_END or position of "Shutdown" item
                           *G_pulVarMenuOfs + ID_XFMI_OFS_RESTARTWPS,
                           cmnGetString(ID_SDSI_RESTARTWPS),  // pszRestartWPS
                           MIS_TEXT,
                           // disable if Shutdown is currently running
                           ulShutdownAttr);

#ifndef __EASYSHUTDOWN__
        if ((cmnQuerySetting(sflXShutdown) & XSD_NOCONFIRM))
            // if XShutdown confirmations have been disabled,
            // remove "..." from "Restart Desktop" entry
            winhMenuRemoveEllipse(hwndMenuInsert,
                                  *G_pulVarMenuOfs + ID_XFMI_OFS_RESTARTWPS);
#endif
    }

    if (krnMultiUser())
    {
        // XWPShell running:
        // insert "logoff"
        winhInsertMenuItem(hwndMenuInsert,  // either main menu or "Shutdown" submenu
                           sOrigShutdownPos,  // either MIT_END or position of "Shutdown" item
                           *G_pulVarMenuOfs + ID_XFMI_OFS_LOGOFF,
                           cmnGetString(ID_XSSI_XSD_LOGOFF),  // pszXSDLogoff
                           MIS_TEXT,
                           // disable if Shutdown is currently running
                           ulShutdownAttr);

#ifndef __EASYSHUTDOWN__
        if ((cmnQuerySetting(sflXShutdown) & XSD_NOCONFIRM))
            // if XShutdown confirmations have been disabled,
            // remove "..." from "Logoff" entry
            winhMenuRemoveEllipse(hwndMenuInsert,
                                  *G_pulVarMenuOfs + ID_XFMI_OFS_LOGOFF);
#endif
    }
#endif

    // remove other default menu items?
    {
        static const ULONG aSuppressFlags[] =
            {
                XWPCTXT_SYSTEMSETUP,
                XWPCTXT_LOGOFF,
            };
        mnuRemoveMenuItems(somSelf,
                           hwndMenu,
                           aSuppressFlags,
                           ARRAYITEMCOUNT(aSuppressFlags));
    }

    #ifdef __XWPMEMDEBUG__ // setup.h, helpers\memdebug.c
        // if XWorkplace is compiled with
        // VAC++ debug memory funcs,
        // add a menu item for listing all memory objects

        cmnInsertSeparator(hwndMenu, MIT_END);

        winhInsertMenuItem(hwndMenu,
                           MIT_END,
                           DEBUG_MENUID_LISTHEAP,
                           "List VAC++ debug heap",
                           MIS_TEXT, 0);
        winhInsertMenuItem(hwndMenu,
                           MIT_END,
                           DEBUG_MENUID_RELEASEFREED,
                           "Discard logs for freed memory",
                           MIS_TEXT, 0);
    #endif

    #ifdef __DEBUG__
        // if we have a debug compile,
        // add "crash" items
        cmnInsertSeparator(hwndMenu, MIT_END);

        hwndMenuInsert = winhInsertSubmenu(hwndMenu,
                                           MIT_END,
                                           DEBUG_MENUID_CRASH_MENU,
                                           "Crash WPS",
                                           MIS_TEXT,
                                           // first item ID in "Shutdown" menu:
                                           // crash thread 1
                                           DEBUG_MENUID_CRASH_THR1,
                                           "Thread 1",
                                           MIS_TEXT, 0);
        winhInsertMenuItem(hwndMenuInsert,
                           MIT_END,
                           DEBUG_MENUID_CRASH_WORKER,
                           "Worker thread",
                           MIS_TEXT, 0);
        winhInsertMenuItem(hwndMenuInsert,
                           MIT_END,
                           DEBUG_MENUID_CRASH_QUICK,
                           "Speedy thread",
                           MIS_TEXT, 0);
        winhInsertMenuItem(hwndMenuInsert,
                           MIT_END,
                           DEBUG_MENUID_CRASH_FILE,
                           "File thread",
                           MIS_TEXT, 0);

        // add "Dump window list"
        winhInsertMenuItem(hwndMenu,
                           MIT_END,
                           DEBUG_MENUID_DUMPWINLIST,
                           "Dump window list",
                           MIS_TEXT, 0);
    #endif

    // krnUnlockGlobals();
}

/*
 *@@ dtpMenuItemSelected:
 *      implementation for XFldDesktop::wpMenuItemSelected.
 *
 *      This returns TRUE if one of the new items was processed
 *      or FALSE if the parent method should be called. We may
 *      change *pulMenuId and return FALSE.
 *
 *@@added V0.9.1 (99-12-04) [umoeller]
 *@@changed V0.9.3 (2000-04-26) [umoeller]: changed shutdown menu item IDs; changed prototype
 *@@changed V0.9.5 (2000-08-10) [umoeller]: added logoff support
 */

BOOL dtpMenuItemSelected(XFldDesktop *somSelf,
                         HWND hwndFrame,
                         PULONG pulMenuId) // in/out: menu item ID (can be changed)
{
    if (xsdQueryShutdownState() == XSD_IDLE)
    {
        ULONG ulMenuId2 = (*pulMenuId - *G_pulVarMenuOfs);

        if (ulMenuId2 == ID_XFMI_OFS_RESTARTWPS)
        {
            xsdInitiateRestartWPS(FALSE);   // restart Desktop, no logoff
            return TRUE;
        }
        else if (ulMenuId2 == ID_XFMI_OFS_LOGOFF)
        {
            xsdInitiateRestartWPS(TRUE);    // logoff
            return TRUE;
        }
#ifndef __NOXSHUTDOWN__
        else if (    (cmnQuerySetting(sfXShutdown))
                 // &&  (cmnQuerySetting(sNoWorkerThread) == 0)
                    // removed this setting V0.9.16 (2002-01-04) [umoeller]
                )
        {
            // shutdown enabled:
            if (*pulMenuId == WPMENUID_SHUTDOWN)
            {
                xsdInitiateShutdown();
                return TRUE;
            }
            else if (ulMenuId2 == ID_XFMI_OFS_OS2_SHUTDOWN)
            {
                // default OS/2 shutdown (in submenu):
                // have parent method called with default shutdown menu item ID
                // to start OS/2 shutdown...
                *pulMenuId = WPMENUID_SHUTDOWN;
                return FALSE;
            }
        }
#endif
    }

#ifdef __XWPLITE__
    if (*pulMenuId == 0x25D)          // product info
    {
        cmnShowProductInfo(NULLHANDLE,      // owner
                           MMSOUND_SYSTEMSTARTUP);
        return TRUE;
    }
#endif

    #ifdef __XWPMEMDEBUG__ // setup.h, helpers\memdebug.c
        // if XWorkplace is compiled with
        // VAC++ debug memory funcs,
        // check the menu item for listing all memory objects
        if (*pulMenuId == DEBUG_MENUID_LISTHEAP)
        {
            memdCreateMemDebugWindow();
            return TRUE;
        }
        else if (*pulMenuId == DEBUG_MENUID_RELEASEFREED)
        {
            HPOINTER hptrOld = winhSetWaitPointer();
            memdReleaseFreed();
            WinSetPointer(HWND_DESKTOP, hptrOld);
            return TRUE;
        }
    #endif

    #ifdef __DEBUG__
        switch (*pulMenuId)
        {
            case DEBUG_MENUID_CRASH_THR1:
                krnPostThread1ObjectMsg(XM_CRASH, 0, 0);
            break;

            case DEBUG_MENUID_CRASH_WORKER:
                xthrPostWorkerMsg(XM_CRASH, 0, 0);
            break;

            case DEBUG_MENUID_CRASH_QUICK:
                xthrPostBushMsg(XM_CRASH, 0, 0);
            break;

            case DEBUG_MENUID_CRASH_FILE:
                xthrPostFileMsg(XM_CRASH, 0, 0);
            break;

            case DEBUG_MENUID_DUMPWINLIST:
                winlCreateWinListWindow();
            break;
        }
    #endif

    return FALSE;
}

/* ******************************************************************
 *
 *   XFldDesktop notebook settings pages callbacks (notebook.c)
 *
 ********************************************************************/

static const CONTROLDEF
#ifndef __NOBOOTLOGO__
    BootLogoGroup = LOADDEF_GROUP(ID_XSDI_DTP_LOGOGROUP, SZL_AUTOSIZE),
    BootLogoCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_DTP_BOOTLOGO),
    LogoStyleGroup = LOADDEF_GROUP(ID_XSDI_DTP_LOGOSTYLEGROUP, SZL_AUTOSIZE),
    LogoTransparentRadio = LOADDEF_FIRST_AUTORADIO(ID_XSDI_DTP_LOGO_TRANSPARENT),
    LogoBlowUpRadio = LOADDEF_NEXT_AUTORADIO(ID_XSDI_DTP_LOGO_BLOWUP),
    LogoFrameGroup = CONTROLDEF_GROUP(
                            NULL,
                            ID_XSDI_DTP_LOGOFRAME,
                            -1,
                            -1),
    LogoBitmap =
        {
            WC_STATIC,
            "",
            SS_FGNDFRAME | WS_VISIBLE,
            ID_XSDI_DTP_LOGOBITMAP,
            CTL_COMMON_FONT,
            {66, 40},
            COMMON_SPACING
        },
    LogoFileText = LOADDEF_TEXT(ID_XSDI_DTP_LOGOFILETXT),
    LogoFileEF = CONTROLDEF_ENTRYFIELD(
                            NULL,
                            ID_XSDI_DTP_LOGOFILE,
                            100,
                            -1),
    LogoFileBrowseButton = CONTROLDEF_PUSHBUTTON(
                            LOAD_STRING, // "Bro~wse..."
                            DID_BROWSE,
                            -1,
                            STD_BUTTON_HEIGHT),
    LogoFileTestButton = CONTROLDEF_PUSHBUTTON(
                            LOAD_STRING, // "T~est logo",
                            ID_XSDI_DTP_TESTLOGO,
                            -1,
                            STD_BUTTON_HEIGHT),
#endif
    WriteXWPStartLogCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_DTP_WRITEXWPSTARTLOG),
#ifndef __NOBOOTUPSTATUS__
    BootupStatusCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_DTP_BOOTUPSTATUS),
#endif
#ifndef __NOXWPSTARTUP__
    CreateStartupFolderButton = CONTROLDEF_PUSHBUTTON(
                            LOAD_STRING, // "Create ~XWorkplace Startup folder",
                            ID_XSDI_DTP_CREATESTARTUPFLDR,
                            -1,
                            STD_BUTTON_HEIGHT),
#endif
    NumLockOnCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_DTP_NUMLOCKON);

static const DLGHITEM dlgDesktopStartup[] =
    {
        START_TABLE,            // root table, required
#ifndef __NOBOOTLOGO__
            START_ROW(0),       // boot logo group
                START_GROUP_TABLE(&BootLogoGroup),
                    START_ROW(0),
                        CONTROL_DEF(&BootLogoCB),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&LogoFileText),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&LogoFileEF),
                    // START_ROW(0),
                        CONTROL_DEF(&LogoFileBrowseButton),
                    START_ROW(0),
                        START_GROUP_TABLE(&LogoFrameGroup),
                            START_ROW(0),
                                CONTROL_DEF(&LogoBitmap),
                        END_TABLE,      // logo frame group
                        START_TABLE,
                            START_ROW(0),
                                START_GROUP_TABLE(&LogoStyleGroup),
                                    START_ROW(0),
                                        CONTROL_DEF(&LogoTransparentRadio),
                                    START_ROW(0),
                                        CONTROL_DEF(&LogoBlowUpRadio),
                                END_TABLE,      // logo style group
                            START_ROW(0),
                                CONTROL_DEF(&LogoFileTestButton),
                        END_TABLE,
                END_TABLE,      // end of boot logo group
#endif
            START_ROW(0),
                CONTROL_DEF(&WriteXWPStartLogCB),
#ifndef __NOBOOTUPSTATUS__
            START_ROW(0),
                CONTROL_DEF(&BootupStatusCB),
#endif
            START_ROW(0),
                CONTROL_DEF(&NumLockOnCB),
#ifndef __NOXWPSTARTUP__
            START_ROW(0),
                CONTROL_DEF(&CreateStartupFolderButton),
#endif
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };

static const XWPSETTING G_DtpStartupBackup[] =
    {
        sfWriteXWPStartupLog,
#ifndef __NOBOOTUPSTATUS__
        sfShowBootupStatus,
#endif
#ifndef __NOBOOTLOGO__
        sfBootLogo,
        sulBootLogoStyle,
#endif
        sfNumLockStartup
    };

/*
 * dtpStartupInitPage:
 *      notebook callback function (notebook.c) for the
 *      "Startup" page in the Desktop's settings
 *      notebook.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.1 (2000-02-09) [umoeller]: added NumLock support to this page
 *@@changed V0.9.13 (2001-06-14) [umoeller]: fixed Undo for boot logo file
 *@@changed V0.9.14 (2001-08-21) [umoeller]: added "write startuplog" setting
 *@@changecd V0.9.16 (2001-09-29) [umoeller]: now using dialog formatter
 */

VOID dtpStartupInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                        ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        // first call: backup Global Settings for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = cmnBackupSettings(G_DtpStartupBackup,
                                         ARRAYITEMCOUNT(G_DtpStartupBackup));

        // insert the controls using the dialog formatter
        // V0.9.16 (2001-09-29) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      dlgDesktopStartup,
                      ARRAYITEMCOUNT(dlgDesktopStartup));

#ifndef __NOBOOTLOGO__
        // backup old boot logo file
        pnbp->pUser2 = cmnQueryBootLogoFile();     // malloc'ed
                // fixed V0.9.13 (2001-06-14) [umoeller]

        // prepare the control to properly display
        // stretched bitmaps
        ctlPrepareStretchedBitmap(WinWindowFromID(pnbp->hwndDlgPage,
                                                  ID_XSDI_DTP_LOGOBITMAP),
                                  TRUE);    // preserve proportions

        // set entry field limit
        winhSetEntryFieldLimit(WinWindowFromID(pnbp->hwndDlgPage,
                                               ID_XSDI_DTP_LOGOFILE),
                               CCHMAXPATH);
#endif
    }

    if (flFlags & CBI_SET)
    {
#ifndef __NOBOOTLOGO__
        USHORT      usRadioID;
        HDC         hdcMem;
        HPS         hpsMem;
        HBITMAP     hbmBootLogo;

        HPOINTER hptrOld = winhSetWaitPointer();

        SIZEL       szlPage = {0, 0};
        PSZ         pszBootLogoFile = cmnQueryBootLogoFile();

        // "boot logo enabled"
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_DTP_BOOTLOGO,
                              cmnQuerySetting(sfBootLogo));

        // "boot logo style"
        if (cmnQuerySetting(sulBootLogoStyle) == 0)
            usRadioID = ID_XSDI_DTP_LOGO_TRANSPARENT;
        else
            usRadioID = ID_XSDI_DTP_LOGO_BLOWUP;
        winhSetDlgItemChecked(pnbp->hwndDlgPage, usRadioID,
                              TRUE);  // V1.0.6 (2006-08-20) [pr]

        // set boot logo file entry field
        WinSetDlgItemText(pnbp->hwndDlgPage,
                          ID_XSDI_DTP_LOGOFILE,
                          pszBootLogoFile);

        // attempt to display the boot logo
        if (gpihCreateMemPS(WinQueryAnchorBlock(pnbp->hwndDlgPage),
                            &szlPage,
                            &hdcMem,
                            &hpsMem))
        {
            APIRET arc;
            if (!(arc = gpihLoadBitmapFile(&hbmBootLogo,
                                           hpsMem,
                                           pszBootLogoFile)))
            {
                // and have the subclassed static control display the thing
                WinSendDlgItemMsg(pnbp->hwndDlgPage, ID_XSDI_DTP_LOGOBITMAP,
                                  SM_SETHANDLE,
                                  (MPARAM)hbmBootLogo,
                                  MPNULL);

                // delete the bitmap again
                // (the static control has made a private copy
                // of the bitmap, so this is safe)
                GpiDeleteBitmap(hbmBootLogo);
            }
            GpiDestroyPS(hpsMem);
            DevCloseDC(hdcMem);
        }
        free(pszBootLogoFile);

        WinSetPointer(HWND_DESKTOP, hptrOld);
#endif
        // startup log file
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_DTP_WRITEXWPSTARTLOG,
                              cmnQuerySetting(sfWriteXWPStartupLog));

#ifndef __NOBOOTUPSTATUS__
        // bootup status
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_DTP_BOOTUPSTATUS,
                              cmnQuerySetting(sfShowBootupStatus));
#endif

        // numlock on
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_DTP_NUMLOCKON,
                              cmnQuerySetting(sfNumLockStartup));
    }

    if (flFlags & CBI_ENABLE)
    {
#ifndef __NOBOOTLOGO__
        PSZ     pszBootLogoFile = cmnQueryBootLogoFile();
        BOOL    fBootLogoFileExists = (access(pszBootLogoFile, 0) == 0);
        free(pszBootLogoFile);

        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_DTP_LOGOBITMAP,
                         cmnQuerySetting(sfBootLogo));
        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_DTP_TESTLOGO, fBootLogoFileExists);
#endif

#ifndef __NOXWPSTARTUP__
        if (WinQueryObject((PSZ)XFOLDER_STARTUPID))
            WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_DTP_CREATESTARTUPFLDR, FALSE);
#endif
    }
}

#ifndef __NOBOOTLOGO__

/*
 *@@ SetBootLogoFile:
 *      changes the boot logo file. Shared between the
 *      entry field handler and "Undo".
 *
 *@@added V0.9.13 (2001-06-14) [umoeller]
 */

STATIC VOID SetBootLogoFile(PNOTEBOOKPAGE pnbp,
                            PCSZ pcszNewBootLogoFile,
                            BOOL fWrite)                   // in: if TRUE, write back to OS2.INI
{
    WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_DTP_TESTLOGO,
                     (access(pcszNewBootLogoFile, 0) == 0));

    if (fWrite)
    {
        // query new file name from entry field
        PrfWriteProfileString(HINI_USER,
                              (PSZ)INIAPP_XWORKPLACE,
                              (PSZ)INIKEY_BOOTLOGOFILE,
                              (PSZ)pcszNewBootLogoFile);
        // update the display by calling the INIT callback
        pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
    }
}

#endif

/*
 * dtpStartupItemChanged:
 *      notebook callback function (notebook.c) for the
 *      "Startup" page in the Desktop's settings
 *      notebook.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.1 (2000-02-09) [umoeller]: added NumLock support to this page
 *@@changed V0.9.3 (2000-04-11) [umoeller]: fixed major resource leak; the bootlogo bitmap was never freed
 *@@changed V0.9.9 (2001-04-07) [pr]: fixed Undo
 *@@changed V0.9.13 (2001-06-14) [umoeller]: fixed Undo for boot logo file
 *@@changed V0.9.14 (2001-08-21) [umoeller]: added "write startuplog" setting
 */

MRESULT dtpStartupItemChanged(PNOTEBOOKPAGE pnbp,
                              ULONG ulItemID,
                              USHORT usNotifyCode,
                              ULONG ulExtra)      // for checkboxes: contains new state
{
    BOOL fProcessed = TRUE;

    ULONG ulChange = 1;

    {
        switch (ulItemID)
        {
#ifndef __NOBOOTLOGO__
            case ID_XSDI_DTP_BOOTLOGO:
                cmnSetSetting(sfBootLogo, ulExtra);
                ulChange = 2;       // re-enable items
            break;

            case ID_XSDI_DTP_LOGO_TRANSPARENT:
                cmnSetSetting(sulBootLogoStyle, 0);
            break;

            case ID_XSDI_DTP_LOGO_BLOWUP:
                cmnSetSetting(sulBootLogoStyle, 1);
            break;
#endif

            case ID_XSDI_DTP_WRITEXWPSTARTLOG:
                cmnSetSetting(sfWriteXWPStartupLog, ulExtra);
            break;

#ifndef __NOBOOTUPSTATUS__
            case ID_XSDI_DTP_BOOTUPSTATUS:
                cmnSetSetting(sfShowBootupStatus, ulExtra);
            break;
#endif

            case ID_XSDI_DTP_NUMLOCKON:
                cmnSetSetting(sfNumLockStartup, ulExtra);
                winhSetNumLock(ulExtra);
            break;

            case DID_UNDO:
                // "Undo" button: get pointer to backed-up Global Settings
                cmnRestoreSettings(pnbp->pUser,
                                   ARRAYITEMCOUNT(G_DtpStartupBackup));

#ifndef __NOBOOTLOGO__
                SetBootLogoFile(pnbp,
                                (PCSZ)pnbp->pUser2,
                                TRUE);      // write
#endif

                // update the display by calling the INIT callback
                pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            break;

            case DID_DEFAULT:
                // set the default settings for this settings page
                // (this is in common.c because it's also used at
                // Desktop startup)
                cmnSetDefaultSettings(pnbp->inbp.ulPageID);
                // update the display by calling the INIT callback
                pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            break;

            default:
                ulChange = 0;
                fProcessed = FALSE;
        }
    }

    if (ulChange == 2)
        // enable/disable items
        pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);

    if (!fProcessed)
    {
        // not processed above:
        // second switch-case with non-global settings stuff

        switch (ulItemID)
        {
#ifndef __NOBOOTLOGO__
            /*
             * ID_XSDI_DTP_LOGOFILE:
             *      focus leaves "file" entry field:
             *      update OS2.INI
             */

            case ID_XSDI_DTP_LOGOFILE:
            {
                PSZ pszNewBootLogoFile = winhQueryWindowText(pnbp->hwndControl);
                SetBootLogoFile(pnbp,
                                pszNewBootLogoFile,
                                (usNotifyCode == EN_KILLFOCUS)); // write?
                if (pszNewBootLogoFile)
                    free(pszNewBootLogoFile);
            }
            break;

            /*
             * DID_BROWSE:
             *      "Browse" button: open file dialog
             */

            case DID_BROWSE:
            {
                // FILEDLG fd;
                CHAR szFile[CCHMAXPATH] = "*.BMP";
                PSZ pszNewBootLogoFile;
                if (pszNewBootLogoFile = winhQueryDlgItemText(pnbp->hwndDlgPage,
                                                              ID_XSDI_DTP_LOGOFILE))
                {
                    // get last directory used
                    PSZ p;
                    if (p = strrchr(pszNewBootLogoFile, '\\'))
                    {
                        // contains directory:
                        PSZ pszDir = strhSubstr(pszNewBootLogoFile, p + 1);
                        strlcpy(szFile, pszDir, sizeof(szFile));
                        free(pszDir);
                    }
                    free(pszNewBootLogoFile);
                }

                strlcat(szFile, "*.bmp", sizeof(szFile));

                if (cmnFileDlg(pnbp->hwndFrame,
                               szFile,
                               0, // WINH_FOD_INILOADDIR | WINH_FOD_INISAVEDIR,
                               0,
                               0,
                               0))
                {
                    // copy file from FOD to page
                    WinSetDlgItemText(pnbp->hwndDlgPage,
                                      ID_XSDI_DTP_LOGOFILE,
                                      szFile);
                    PrfWriteProfileString(HINI_USER,
                                          (PSZ)INIAPP_XWORKPLACE,
                                          (PSZ)INIKEY_BOOTLOGOFILE,
                                          szFile);
                    // update the display by calling the INIT callback
                    pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
                }
            }
            break;

            /*
             * ID_XSDI_DTP_TESTLOGO:
             *
             */

            case ID_XSDI_DTP_TESTLOGO:
            {
                HDC         hdcMem;
                HPS         hpsMem;
                HBITMAP     hbmBootLogo;
                ULONG       ulError;
                SIZEL       szlPage = {0, 0};
                HPOINTER    hptrOld = winhSetWaitPointer();

                PSZ         pszBootLogoFile = cmnQueryBootLogoFile();

                // attempt to load the boot logo
                if (gpihCreateMemPS(WinQueryAnchorBlock(pnbp->hwndDlgPage),
                                    &szlPage,
                                    &hdcMem,
                                    &hpsMem))
                {
                    if (!gpihLoadBitmapFile(&hbmBootLogo,
                                            hpsMem,
                                            pszBootLogoFile))
                    {
                        if (cmnQuerySetting(sulBootLogoStyle) == 1)
                        {
                            // blow-up mode:
                            HPS     hpsScreen = WinGetScreenPS(HWND_DESKTOP);
                            anmBlowUpBitmap(hpsScreen,
                                            hbmBootLogo,
                                            1000);  // total animation time

                            DosSleep(2000);

                            WinReleasePS(hpsScreen);

                            // repaint all windows
                            winhRepaintWindows(HWND_DESKTOP);
                        }
                        else
                        {
                            // transparent mode:
                            SHAPEFRAME sf;
                            SWP     swpScreen;

                            sf.hab = WinQueryAnchorBlock(pnbp->hwndDlgPage);
                            sf.hps = hpsMem;
                            sf.hbm = hbmBootLogo;
                            sf.bmi.cbFix = sizeof(sf.bmi);
                            GpiQueryBitmapInfoHeader(sf.hbm, &sf.bmi);

                            // set ptlLowerLeft so that the bitmap
                            // is centered on the screen
                            WinQueryWindowPos(HWND_DESKTOP, &swpScreen);
                            sf.ptlLowerLeft.x = (swpScreen.cx - sf.bmi.cx) / 2;
                            sf.ptlLowerLeft.y = (swpScreen.cy - sf.bmi.cy) / 2;

                            if (shpCreateWindows(&sf)) // this selects the bitmap into the HPS
                            {
                                DosSleep(2000);

                                GpiSetBitmap(sf.hps, NULLHANDLE); // V0.9.3 (2000-04-11) [umoeller]

                                winhDestroyWindow(&sf.hwndShapeFrame) ;
                                winhDestroyWindow(&sf.hwndShape);
                            }
                        }
                        // delete the bitmap again
                        if (!GpiDeleteBitmap(hbmBootLogo))
                            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                                "Unable to free bootlogo bitmap.");
                    }
                    GpiDestroyPS(hpsMem);
                    DevCloseDC(hdcMem);
                }
                free(pszBootLogoFile);

                WinSetPointer(HWND_DESKTOP, hptrOld);
            }
            break;
#endif

#ifndef __NOXWPSTARTUP__
            /*
             *@@ ID_XSDI_DTP_CREATESTARTUPFLDR:
             *      "Create startup folder"
             */

            case ID_XSDI_DTP_CREATESTARTUPFLDR:
            {
                CHAR        szSetup[200];
                HOBJECT     hObj;
                sprintf(szSetup,
                    "DEFAULTVIEW=ICON;ICONVIEW=NONFLOWED,MINI;"
                    "OBJECTID=%s;",
                    XFOLDER_STARTUPID);
                if (hObj = WinCreateObject((PSZ)G_pcszXFldStartup,
                                           (PSZ)cmnGetString(ID_XFSI_XWPSTARTUPFDR), // "XWorkplace Startup",
                                                // finally localized V1.0.0 (2002-08-31) [umoeller]
                                           szSetup,
                                           (PSZ)WPOBJID_DESKTOP, // "<WP_DESKTOP>",
                                           CO_UPDATEIFEXISTS))
                    WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_DTP_CREATESTARTUPFLDR, FALSE);
                else
                    cmnMessageBoxExt(pnbp->hwndFrame,
                                     104,
                                     NULL, 0,
                                     105,
                                     MB_OK);
            }
            break;
#endif

            default:
                ;
        }
    } // end if (!fProcessed)

    return (MPARAM)0;
}


