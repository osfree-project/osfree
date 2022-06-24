
/*
 *@@sourcefile drivdlgs.c:
 *      this file has the CONFIG.SYS driver configuration dialogs
 *      which get called from the "Drivers" page in the "OS/2 Kernel"
 *      object (whose implementation is in cfgsys.c).
 *
 *      The driver dialogs have been exported to this file so that
 *      new driver dialogs can be added more easily without having
 *      to mess with the SOM interface. SO TO WRITE A DRIVER DIALOG,
 *      YOU NEED NOT KNOW NOTHING ABOUT NO WPS PROGRAMMING. ;-)
 *
 *      Each driver dialog function can presently handle one CONFIG.SYS
 *      line with all the driver parameters. Presently, we have:
 *
 *      -- drv_fnwpConfigHPFS: HPFS.IFS configuration.
 *
 *      -- drv_fnwpConfigCDFS: CDFS.IFS configuration.
 *
 *      -- drv_fnwpConfigIBM1S506: IBM1S506.ADD and DANIS506.ADD
 *         configuration.
 *
 *      In order to add a new driver configuration dialog, do the
 *      following:
 *
 *      1)  Add a check for your driver to drvConfigSupported.
 *          This will enable the "Configure" button on the "Drivers"
 *          page.
 *
 *      2)  Create a dialog template and a window procedure for your
 *          driver. When the "Configure" button is pressed on the
 *          "Drivers" page, your dialog function will get a
 *          DRIVERDLGDATA structure with WM_INITDLG in mp2
 *          (pCreateParam), which has all the necessary data for
 *          interacting with the main settings page.
 *
 *          In WM_INITDLG, you should store that pointer in the
 *          QWL_USER window word.
 *
 *          It is the responsibility of your dialog func to
 *          set its dialog items according to the CONFIG.SYS
 *          parameters passed in DRIVERDLGDATA and modify
 *          that data according to user changes. Only if the
 *          dialog is dismissed with DID_OK, cfgDriversItemChanged
 *          will update the "Parameters" field on the "Dialogs"
 *          page with the new parameters from DRIVERDLGDATA, as
 *          set up by the dialog func.
 *
 *      So in short, each driver dialog func needs to do three things:
 *
 *      1)  parse DRIVERDLGDATA.szParams and set up the dialog
 *          controls accordingly;
 *
 *      2)  while the dialog is running, react to user changes
 *          (just the normal PM dialog programming);
 *
 *      3)  on "OK", write a new params string into DRIVERDLGDATA.szParams
 *          and return DID_OK, or return DID_CANCEL to discard the changes.
 *
 *      Notes:
 *
 *      1)  Your dialog template should not have the
 *          WS_VISIBLE flag set, because we will center the
 *          dialog on the screen (causes flicker otherwise).
 *
 *      2)  The dialog items will automatically be given
 *          font presparams by cfgDriversItemChanged, according
 *          to what cmnQueryDefaultFont returns.
 *
 *      This file (and its functionality) is completely new with V0.9.0.
 *
 *@@added V0.9.0 [umoeller]
 *@@header "config\drivdlgs.h"
 */

/*
 *      Copyright (C) 1999-2010 Ulrich M”ller.
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

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSMODULEMGR
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINSYS
#define INCL_WINDIALOGS
#define INCL_WINENTRYFIELDS
#define INCL_WINBUTTONS
#define INCL_WINMLE
#define INCL_WINSTDSPIN
#define INCL_WINSTDSLIDER
#define INCL_WINSTDCNR
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\except.h"             // exception handling
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
#include "xfsys.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs

#include "config\drivdlgs.h"            // driver configuration dialogs

#pragma hdrstop                         // VAC++ keeps crashing otherwise
// other SOM headers
#include <wpfsys.h>                     // WPFileSystem

/* ******************************************************************
 *
 *   Definitions
 *
 ********************************************************************/

/*
 *@@ DRIVERPLUGIN:
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

typedef struct _DRIVERPLUGIN
{
    HMODULE     hmodPlugin;         // NULLHANDLE if built-in
    PFNCHECKDRIVERNAME  pfnCheckDriverName;
} DRIVERPLUGIN, *PDRIVERPLUGIN;

/* ******************************************************************
 *
 *   Private prototypes
 *
 ********************************************************************/

BOOL APIENTRY CheckHPFSDriverName(HMODULE hmodPlugin, HMODULE hmodXFLDR, PDRIVERSPEC pSpec, PSZ psz);
BOOL APIENTRY CheckHPFS386DriverName(HMODULE hmodPlugin, HMODULE hmodXFLDR, PDRIVERSPEC pSpec, PSZ psz);
BOOL APIENTRY CheckIBM1S506DriverName(HMODULE hmodPlugin, HMODULE hmodXFLDR, PDRIVERSPEC pSpec, PSZ psz);

// built-in driver configuration dialogs
MRESULT EXPENTRY drv_fnwpConfigHPFS(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY drv_fnwpConfigHPFS386(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY drv_fnwpConfigIBM1S506(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2);

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

static HMTX         G_hmxPlugins = NULLHANDLE;

static LINKLIST     G_llCheckDrivers,       // linked list of DRIVERPLUGIN structs, auto-free
                    G_llModules;            // linked list of HMODULE's, no auto-free
static BOOL         G_fModulesInitialized = FALSE,
                    G_fDriverProcsLoaded = FALSE;

static ULONG        G_ulDriverLoadsCount = 0;

static PFNCHECKDRIVERNAME G_aBuiltInDriverProcs[] =
    {
            CheckHPFSDriverName,
            CheckHPFS386DriverName,
            CheckIBM1S506DriverName
    };

/* ******************************************************************
 *
 *   Plugin DLL Management
 *
 ********************************************************************/

/*
 *@@ LockPlugins:
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

STATIC BOOL LockPlugins(VOID)
{
    if (G_hmxPlugins)
        return !DosRequestMutexSem(G_hmxPlugins, SEM_INDEFINITE_WAIT);

    // first call:
    return !DosCreateMutexSem(NULL,
                              &G_hmxPlugins,
                              0,
                              TRUE);      // request!
}

/*
 *@@ UnlockPlugins:
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

STATIC VOID UnlockPlugins(VOID)
{
    DosReleaseMutexSem(G_hmxPlugins);
}

/* ******************************************************************
 *
 *   APIs
 *
 ********************************************************************/

/*
 *@@ drvLoadPlugins:
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

BOOL drvLoadPlugins(HAB hab)
{
    BOOL rc = FALSE;
    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = LockPlugins())
        {
            if (!G_fModulesInitialized)
            {
                // very first call:
                lstInit(&G_llCheckDrivers, TRUE);
                lstInit(&G_llModules, FALSE);
                G_fModulesInitialized = TRUE;
            }

            if (!G_fDriverProcsLoaded)
            {
                // widget classes not loaded yet (or have been released again):

                // HMODULE         hmodXFLDR = cmnQueryMainCodeModuleHandle();

                // built-in widget classes:
                APIRET          arc = NO_ERROR;
                CHAR            szPluginsDir[2*CCHMAXPATH],
                                szSearchMask[2*CCHMAXPATH];
                HDIR            hdirFindHandle = HDIR_CREATE;
                FILEFINDBUF3    ffb3 = {0};      // returned from FindFirst/Next
                ULONG           cbFFB3 = sizeof(FILEFINDBUF3);
                ULONG           ulFindCount = 1;  // look for 1 file at a time
                ULONG           ul;

                // step 1: append built-in widgets to list
                for (ul = 0;
                     ul < ARRAYITEMCOUNT(G_aBuiltInDriverProcs);
                     ul++)
                {
                    PDRIVERPLUGIN pDef = NEW(DRIVERPLUGIN);
                    if (pDef)
                    {
                        pDef->hmodPlugin = NULLHANDLE;      // built-in
                        pDef->pfnCheckDriverName = G_aBuiltInDriverProcs[ul];
                        lstAppendItem(&G_llCheckDrivers,
                                      (PVOID)pDef);
                    }
                }

                // step 2: append plugin DLLs to list
                // compose path for widget plugin DLLs
                cmnQueryXWPBasePath(szPluginsDir);
                strcat(szPluginsDir, "\\plugins\\drvdlgs");
                sprintf(szSearchMask, "%s\\%s", szPluginsDir, "*.dll");

                // _PmpfF(("searching for '%s'", szSearchMask));

                arc = DosFindFirst(szSearchMask,
                                   &hdirFindHandle,
                                   // find everything except directories
                                   FILE_ARCHIVED | FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
                                   &ffb3,
                                   cbFFB3,
                                   &ulFindCount,
                                   FIL_STANDARD);
                // and start looping...
                while (arc == NO_ERROR)
                {
                    // alright... we got the file's name in ffb3.achName
                    CHAR            szDLL[2*CCHMAXPATH],
                                    szError[CCHMAXPATH] = "";
                    HMODULE         hmod = NULLHANDLE;
                    APIRET          arc2 = NO_ERROR;

                    sprintf(szDLL, "%s\\%s", szPluginsDir, ffb3.achName);

                    if (arc2 = DosLoadModule(szError,
                                             sizeof(szError),
                                             szDLL,
                                             &hmod))
                    {
                        // error loading module:
                        // log this, but we'd rather not have a message box here
                        cmnLog(__FILE__, __LINE__, __FUNCTION__,
                               "Unable to load plugin DLL \"%s\"."
                               "\n    DosLoadModule returned code %d and string: \"%s\"",
                               szDLL,
                               arc2,
                               szError);
                    }
                    else
                    {
                        // Check if the widget has the "CheckDriverName" export.
                        PFNCHECKDRIVERNAME pfnCheckDriverName = NULL;
                        if (!(arc2 = DosQueryProcAddr(hmod,
                                                      0,
                                                      "CheckDriverName",
                                                      (PFN*)(&pfnCheckDriverName))))
                        {
                            // plugin DLL seems OK:
                            // create an entry for this
                            PDRIVERPLUGIN pDef = NEW(DRIVERPLUGIN);
                            if (pDef)
                            {
                                pDef->hmodPlugin = hmod;
                                pDef->pfnCheckDriverName = pfnCheckDriverName;
                                lstAppendItem(&G_llCheckDrivers,
                                              (PVOID)pDef);

                                _PmpfF(("got CheckDriverName 0x%lX for module 0x%lX",
                                            pDef->pfnCheckDriverName,
                                            pDef->hmodPlugin));

                                // and store the module too
                                lstAppendItem(&G_llModules,
                                              (PVOID)hmod);
                            }
                        }
                        else
                            DosFreeModule(hmod);
                    } // end if DosLoadModule

                    // find next DLL
                    ulFindCount = 1;
                    arc = DosFindNext(hdirFindHandle,
                                      &ffb3,
                                      cbFFB3,
                                      &ulFindCount);
                } // while (arc == NO_ERROR)

                DosFindClose(hdirFindHandle);

                rc = (lstCountItems(&G_llCheckDrivers) > 0);

                G_fDriverProcsLoaded = TRUE;
            }

            G_ulDriverLoadsCount++;
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (fLocked)
        UnlockPlugins();

    return rc;
}

/*
 *@@ drvUnloadPlugins:
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

VOID drvUnloadPlugins(VOID)
{
    BOOL fLocked = FALSE;
    TRY_LOUD(excpt2)
    {
        if (fLocked = LockPlugins())
        {
            if (G_ulDriverLoadsCount == 0)
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                               "G_ulDriverLoadsCount is already 0!");
            else
            {
                G_ulDriverLoadsCount--;
                if (G_ulDriverLoadsCount == 0)
                {
                    // no more references to the data:

                    // unload modules
                    PLISTNODE pNode = lstQueryFirstNode(&G_llModules);
                    while (pNode)
                    {
                        HMODULE hmod = (HMODULE)pNode->pItemData;

                        _PmpfF(("unloading hmod 0x%lX",
                                    hmod));

                        DosFreeModule(hmod);

                        pNode = pNode->pNext;
                    }

                    lstClear(&G_llModules);
                    lstClear(&G_llCheckDrivers);

                    G_fDriverProcsLoaded = FALSE;
                }
            }
        }
    }
    CATCH(excpt2) {} END_CATCH();

    if (fLocked)
        UnlockPlugins();
}

/*
 *@@ drvConfigSupported:
 *      this function gets called from cfgDriversInitPage
 *      for each driver which was found. This must check
 *      the driver name for whether a driver configuration
 *      dialog exists for that driver and, if so, set the
 *      fields in the DRIVERSPEC structure accordingly.
 *
 *      This was rewritten with V0.9.13. Instead of
 *      explicitly checking for driver names, this now
 *      runs through the list of driver configuration
 *      dialogs (either built-in or from the plugins dir)
 *      and calls the "check driver name" func for each.
 *
 *      Returns FALSE if a fatal error occurred.
 *
 *@@added V0.9.1 (99-11-29) [umoeller]
 *@@changed V0.9.3 (2000-04-10) [umoeller]: added IBM1S506.ADD support
 *@@changed V0.9.5 (2000-08-10) [umoeller]: added HPFS386 support
 *@@changed V0.9.13 (2001-06-27) [umoeller]: rewritten for plugin support
 */

BOOL drvConfigSupported(PDRIVERSPEC pSpec)
{
    BOOL    brc = TRUE,
            fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        HMODULE hmodXFLDR = cmnQueryMainCodeModuleHandle();
        PLISTNODE pNode;

        /* pSpec->hmodConfigDlg = 0;
        pSpec->idConfigDlg = 0;
        pSpec->pfnwpConfigure = 0; */

        pSpec->pfnShowDriverDlg = NULL;

        // go thru the driver dlg defs that were loaded
        // and call each callback for whether it can handle
        // this driver
        if (fLocked = LockPlugins())
        {
            CHAR szErrorMsg[500];
            pNode = lstQueryFirstNode(&G_llCheckDrivers);
            while (pNode)
            {
                PDRIVERPLUGIN pDef = (PDRIVERPLUGIN)pNode->pItemData;

                ULONG ul = pDef->pfnCheckDriverName(pDef->hmodPlugin,
                                                    hmodXFLDR,
                                                    pSpec,
                                                    szErrorMsg);
                if (ul == 1)
                    // this func returned TRUE:
                    // stop searching
                    break;
                else
                    if (ul == -1)
                    {
                        CHAR szModule[CCHMAXPATH];

                        if (DosQueryModuleName(pDef->hmodPlugin,
                                               sizeof(szModule),
                                               szModule))
                            strcpy(szModule, "?");

                        cmnLog(__FILE__, __LINE__, __FUNCTION__,
                               "CheckDriverName call failed for plugin DLL"
                               "\n        \"%s\"."
                               "\n    DLL returned error msg:"
                               "\n        %s",
                               szModule,
                               szErrorMsg);
                    }

                pNode = pNode->pNext;
            }
        }
    }
    CATCH(excpt1)
    {
        brc = FALSE;
    } END_CATCH();

    if (fLocked)
        UnlockPlugins();

    return brc;
}

/* ******************************************************************
 *
 *   Built-in dialogs
 *
 ********************************************************************/

/*
 *@@ ShowDriverDlg:
 *      shared helper to show one of the built-in dialogs.
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

STATIC BOOL ShowDriverDlg(HWND hwndOwner,
                          ULONG ulID,
                          PFNWP pfnwp,
                          PDRIVERDLGDATA pDlgData)
{
    BOOL brc = FALSE;
    HWND hwndDlg;
    if (hwndDlg = cmnLoadDlg(hwndOwner,
                             pfnwp,
                                // dlg proc as in DRIVERSPEC
                             ulID,
                                // resource ID as in DRIVERSPEC
                             pDlgData))
                                // pass DRIVERDLGDATA as create param
    {
        // successfully loaded:
        // set dialog title to driver name
        CHAR szTitle[500];
        sprintf(szTitle, "%s: %s",
                pDlgData->pcszKernelTitle,
                pDlgData->pDriverSpec->pszFilename);
        WinSetWindowText(hwndDlg, szTitle);
        winhCenterWindow(hwndDlg);
        cmnSetControlsFont(hwndDlg, 0, 5000);
        // go!!
        if (WinProcessDlg(hwndDlg) == DID_OK)
            brc = TRUE;

        WinDestroyWindow(hwndDlg);
    }

    return brc;
}

/*
 *@@ ShowHPFSDlg:
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

STATIC BOOL EXPENTRY ShowHPFSDlg(HWND hwndOwner,
                                 PDRIVERDLGDATA pDlgData)
{
    return ShowDriverDlg(hwndOwner,
                         ID_OSD_DRIVER_HPFS,
                         drv_fnwpConfigHPFS,
                         pDlgData);
}

/*
 *@@ ShowHPFS386Dlg:
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

STATIC BOOL EXPENTRY ShowHPFS386Dlg(HWND hwndOwner,
                                    PDRIVERDLGDATA pDlgData)
{
    return ShowDriverDlg(hwndOwner,
                         ID_OSD_DRIVER_HPFS386,
                         drv_fnwpConfigHPFS386,
                         pDlgData);
}

/*
 *@@ ShowIBM1S506Dlg:
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

STATIC BOOL EXPENTRY ShowIBM1S506Dlg(HWND hwndOwner,
                                     PDRIVERDLGDATA pDlgData)
{
    return ShowDriverDlg(hwndOwner,
                         ID_OSD_DRIVER_IBM1S506,
                         drv_fnwpConfigIBM1S506,
                         pDlgData);
}

/*
 *@@ CheckHPFSDriverName:
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

STATIC BOOL EXPENTRY CheckHPFSDriverName(HMODULE hmodPlugin, HMODULE hmodXFLDR, PDRIVERSPEC pSpec, PSZ psz)
{
    if (!stricmp(pSpec->pszFilename, "HPFS.IFS"))
    {
        pSpec->pfnShowDriverDlg = ShowHPFSDlg;
        return TRUE;
    }

    return FALSE;
}

/*
 *@@ CheckHPFS386DriverName:
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

STATIC BOOL EXPENTRY CheckHPFS386DriverName(HMODULE hmodPlugin, HMODULE hmodXFLDR, PDRIVERSPEC pSpec, PSZ psz)
{
    if (!stricmp(pSpec->pszFilename, "HPFS386.IFS"))
    {
        pSpec->pfnShowDriverDlg = ShowHPFS386Dlg;
        return TRUE;
    }

    return FALSE;
}

/*
 *@@ CheckIBM1S506DriverName:
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

STATIC BOOL EXPENTRY CheckIBM1S506DriverName(HMODULE hmodPlugin, HMODULE hmodXFLDR, PDRIVERSPEC pSpec, PSZ psz)
{
    if (    (!stricmp(pSpec->pszFilename, "IBM1S506.ADD"))
         || (!stricmp(pSpec->pszFilename, "DANIS506.ADD"))
       )
    {
        pSpec->pfnShowDriverDlg = ShowIBM1S506Dlg;
        return TRUE;
    }

    return FALSE;
}

/*
 *@@ drv_fnwpConfigHPFS:
 *      dialog procedure for the "Configure HPFS.IFS" dialog.
 *
 *      This gets called automatically from cfgDriversItemChanged
 *      (xfsys.c) when the "Configure" button is pressed.
 *
 *      As with all driver dialogs, this gets a DRIVERDLGDATA
 *      structure with mp2 in WM_INITDLG.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.12 (2001-05-03) [umoeller]: fixed strtok, thanks Lars Erdmann
 */

MRESULT EXPENTRY drv_fnwpConfigHPFS(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PDRIVERDLGDATA pddd = WinQueryWindowPtr(hwndDlg, QWL_USER);
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_INITDLG:
        {
            PSZ         pszParamsCopy = 0,
                        pszToken = 0;
            ULONG   ulCache = 0,                // 10% of mem
                    ulCrecl = 4;

            HWND    hwndCacheSlider = WinWindowFromID(hwndDlg,
                                                      ID_OSDI_CACHESIZE),
                    hwndThresholdSlider = WinWindowFromID(hwndDlg,
                                                          ID_OSDI_CACHE_THRESHOLD);
            // store DRIVERDLGDATA in window words
            pddd = (PDRIVERDLGDATA)mp2;
            WinSetWindowPtr(hwndDlg, QWL_USER, pddd);
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);

            // now parse the parameters
            if (pszParamsCopy = strdup(pddd->szParams))
            {
                pszToken = strtok(pszParamsCopy, " ");
                while (pszToken)        // V0.9.12 (2001-05-03) [umoeller]
                {
                    if (!memicmp(pszToken, "/C:", 3))
                        ulCache = strtoul(pszToken+3, NULL, 0);
                    else if (!memicmp(pszToken, "/CACHE:", 7))
                        ulCache = strtoul(pszToken+7, NULL, 0);
                    else if (!memicmp(pszToken, "/AUTOCHECK:", 11))
                        WinSetDlgItemText(hwndDlg, ID_OSDI_AUTOCHECK, pszToken+11);
                    else if (!memicmp(pszToken, "/CRECL:", 7))
                        ulCrecl = strtoul(pszToken+7, NULL, 0);

                    pszToken = strtok(NULL, " ");       // V0.9.12 (2001-05-03) [umoeller]
                };

                free(pszParamsCopy);
            }

            // set dialog items
            winhSetDlgItemChecked(hwndDlg, ID_OSDI_CACHESIZE_AUTO,
                                  (ulCache == 0));

            // sliders
            winhSetSliderTicks(hwndCacheSlider,
                               (MPARAM)0, 4,
                               (MPARAM)-1, -1);
            winhSetSliderArmPosition(hwndCacheSlider, SMA_INCREMENTVALUE, ulCache / 64);
            WinEnableWindow(hwndCacheSlider, (ulCache != 0));

            winhSetSliderTicks(hwndThresholdSlider,
                               (MPARAM)0, 4,
                               (MPARAM)-1, -1);
            winhSetSliderArmPosition(hwndThresholdSlider, SMA_INCREMENTVALUE, (ulCrecl / 4) - 1);

        }
        break;

        case WM_CONTROL:
        {
            USHORT  usItemID = SHORT1FROMMP(mp1),
                    usNotifyCode = SHORT2FROMMP(mp1);
            switch (usItemID)
            {
                // sliders changed:
                case ID_OSDI_CACHESIZE:
                case ID_OSDI_CACHE_THRESHOLD:
                    if (    (usNotifyCode == SLN_CHANGE)
                         || (usNotifyCode == SLN_SLIDERTRACK)
                       )
                    {
                        HWND hwndSlider = WinWindowFromID(hwndDlg,
                                                          usItemID);
                        LONG lSliderIndex
                            = winhQuerySliderArmPosition(hwndSlider,
                                                         SMA_INCREMENTVALUE);
                        if (usItemID == ID_OSDI_CACHESIZE)
                        {
                            // cache slider: update text on the right
                            CHAR szTemp[100];
                            sprintf(szTemp, "%d KB", lSliderIndex * 64);
                            WinSetDlgItemText(hwndDlg, ID_OSDI_CACHESIZE_TXT, szTemp);
                        }
                        else
                        {
                            // crecl slider: update text on the right
                            CHAR szTemp[100];
                            sprintf(szTemp, "%d KB", (lSliderIndex+1) * 4);
                            WinSetDlgItemText(hwndDlg, ID_OSDI_CACHE_THRESHOLD_TXT, szTemp);
                        }
                    }
                break;

                case ID_OSDI_CACHESIZE_AUTO:
                {
                    BOOL fChecked = winhIsDlgItemChecked(hwndDlg, usItemID);
                    WinEnableControl(hwndDlg, ID_OSDI_CACHESIZE, !fChecked);
                    WinEnableControl(hwndDlg, ID_OSDI_CACHESIZE_TXT, !fChecked);
                }
                break;
            }
        }
        break;

        case WM_COMMAND:
            switch ((USHORT)mp1)
            {
                case DID_OK:
                {
                    // recompose params string
                    LONG    lSliderIndex;
                    CHAR    szTemp[100];
                    PSZ     pszTemp = NULL;
                    pddd->szParams[0] = 0;

                    if (!winhIsDlgItemChecked(hwndDlg, ID_OSDI_CACHESIZE_AUTO))
                    {
                        lSliderIndex
                            = winhQuerySliderArmPosition(WinWindowFromID(hwndDlg,
                                                                         ID_OSDI_CACHESIZE),
                                                         SMA_INCREMENTVALUE);
                        // cache slider: update text on the right
                        sprintf(szTemp, "/CACHE:%d ", lSliderIndex * 64);
                        strcat(pddd->szParams, szTemp);
                    }

                    lSliderIndex
                            = winhQuerySliderArmPosition(WinWindowFromID(hwndDlg,
                                                                         ID_OSDI_CACHE_THRESHOLD),
                                                         SMA_INCREMENTVALUE);
                    if (lSliderIndex != 0)
                    {
                        sprintf(szTemp, "/CRECL:%d ", (lSliderIndex+1) * 4);
                        strcat(pddd->szParams, szTemp);
                    }

                    pszTemp = winhQueryDlgItemText(hwndDlg, ID_OSDI_AUTOCHECK);
                    if (pszTemp)
                    {
                        strcat(pddd->szParams, "/AUTOCHECK:");
                        strcat(pddd->szParams, pszTemp);
                        strcat(pddd->szParams, " ");
                        free(pszTemp);
                    }

                    // dismiss
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                }
                break;

                case DID_DEFAULT:
                    winhSetDlgItemChecked(hwndDlg, ID_OSDI_CACHESIZE_AUTO, TRUE);
                    winhSetSliderArmPosition(WinWindowFromID(hwndDlg,
                                                             ID_OSDI_CACHE_THRESHOLD),
                                             SMA_INCREMENTVALUE, 0);
                    WinEnableControl(hwndDlg, ID_OSDI_CACHESIZE, FALSE);
                    WinEnableControl(hwndDlg, ID_OSDI_CACHESIZE_TXT, FALSE);
                    WinSetDlgItemText(hwndDlg, ID_OSDI_AUTOCHECK, "");
                break;

                case ID_OSDI_AUTOCHECK_PROPOSE:
                {
                    // "Propose" button for auto-chkdsk (HPFS/FAT pages):
                    // enumerate all HPFS or FAT drives on the system
                    CHAR szHPFSDrives[30];
                    doshEnumDrives(szHPFSDrives,
                                   "HPFS",
                                   TRUE); // skip removeable drives
                    WinSetDlgItemText(hwndDlg, ID_OSDI_AUTOCHECK, szHPFSDrives);
                }
                break;

                default:
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            }
        break;

        case WM_HELP:
            cmnDisplayHelp(NULL,        // active Desktop
                           ID_XSH_DRIVER_HPFS);
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ drv_fnwpConfigHPFS386:
 *      dialog procedure for the "Configure HPFS386.IFS" dialog.
 *
 *      This gets called automatically from cfgDriversItemChanged
 *      (xfsys.c) when the "Configure" button is pressed.
 *
 *      As with all driver dialogs, this gets a DRIVERDLGDATA
 *      structure with mp2 in WM_INITDLG.
 *
 *@@added V0.9.5 (2000-08-10) [umoeller]
 *@@changed V0.9.12 (2001-05-03) [umoeller]: fixed strtok, thanks Lars Erdmann
 *@@changed V0.9.12 (2001-05-24) [umoeller]: fixed missing INI default
 */

MRESULT EXPENTRY drv_fnwpConfigHPFS386(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PDRIVERDLGDATA pddd = WinQueryWindowPtr(hwndDlg, QWL_USER);
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_INITDLG:
        {
            PSZ         pszParamsCopy = 0,
                        pszToken = 0;
            ULONG       cParams = 0;
            HWND        hwndCnr = WinWindowFromID(hwndDlg, ID_OSDI_HPFS386INI_CNR);

            CHAR        szIniFilename[CCHMAXPATH] = "";
            WPFileSystem *pIniFile = 0;

            // store DRIVERDLGDATA in window words
            pddd = (PDRIVERDLGDATA)mp2;
            WinSetWindowPtr(hwndDlg, QWL_USER, pddd);
            pddd->pvUser = 0;

            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);

            // set up cnr
            BEGIN_CNRINFO()
            {
                cnrhSetView(CV_NAME);
            } END_CNRINFO(hwndCnr);

            // now parse the parameters
            if (pszParamsCopy = strdup(pddd->szParams))
            {
                pszToken = strtok(pszParamsCopy, " ");
                while (pszToken)    // V0.9.12 (2001-05-03) [umoeller]
                {
                    if (!memicmp(pszToken, "/AUTOCHECK:", 11))
                        WinSetDlgItemText(hwndDlg, ID_OSDI_AUTOCHECK, pszToken+11);
                    else if (cParams == 0)
                        // probably HPFS386.INI location then
                        strlcpy(szIniFilename,
                                pszToken,
                                sizeof(szIniFilename));

                    cParams++;

                    pszToken = strtok(NULL, " ");       // V0.9.12 (2001-05-03) [umoeller]
                };

                free(pszParamsCopy);
            }

            // if we didn't have an INI filename given,
            // use default V0.9.12 (2001-05-24) [umoeller]
            if (szIniFilename[0] == '\0' )
                sprintf(szIniFilename,
                        "%c:\\IBM386FS\\HPFS386.INI",
                        doshQueryBootDrive());

            // set group title
            WinSetDlgItemText(hwndDlg, ID_OSDI_HPFS386INI_GROUP, szIniFilename);

            if (pIniFile = _wpclsQueryObjectFromPath(_WPFileSystem, szIniFilename))
            {
                POINTL ptlIcon = {0, 0};
                // now insert that file into the container
                if (_wpCnrInsertObject(pIniFile,
                                       hwndCnr,
                                       &ptlIcon,
                                       NULL,    // parent
                                       NULL))    // next available position
                        // returns a PMINIRECORDCORE
                {
                    // success: store object ptr in dlg data
                    pddd->pvUser = pIniFile;
                    // set object flags...
                    _wpModifyStyle(pIniFile,
                                   OBJSTYLE_NODELETE
                                    | OBJSTYLE_NORENAME
                                    | OBJSTYLE_NOMOVE,
                                   OBJSTYLE_NODELETE
                                    | OBJSTYLE_NORENAME
                                    | OBJSTYLE_NOMOVE);
                }
            }
        }
        break;

        case WM_COMMAND:
            switch ((USHORT)mp1)
            {
                case DID_OK:
                {
                    // recompose params string
                    // CHAR    szTemp[100];
                    PSZ     pszTemp = NULL;
                    pddd->szParams[0] = 0;

                    // location of HPFS386.INI
                    WinQueryWindowText(WinWindowFromID(hwndDlg, ID_OSDI_HPFS386INI_GROUP),
                                       sizeof(pddd->szParams),
                                       pddd->szParams);
                    strcat(pddd->szParams, " ");

                    // autocheck next
                    pszTemp = winhQueryDlgItemText(hwndDlg, ID_OSDI_AUTOCHECK);
                    if (pszTemp)
                    {
                        strcat(pddd->szParams, "/AUTOCHECK:");
                        strcat(pddd->szParams, pszTemp);
                        strcat(pddd->szParams, " ");
                        free(pszTemp);
                    }

                    // dismiss
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                }
                break;

                case ID_OSDI_AUTOCHECK_PROPOSE:
                {
                    // "Propose" button for auto-chkdsk (HPFS/FAT pages):
                    // enumerate all HPFS or FAT drives on the system
                    CHAR szHPFSDrives[30];
                    doshEnumDrives(szHPFSDrives,
                                   "HPFS",
                                   TRUE); // skip removeable drives
                    WinSetDlgItemText(hwndDlg, ID_OSDI_AUTOCHECK, szHPFSDrives);
                }
                break;

                default:
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            }
        break;

        case WM_HELP:
            cmnDisplayHelp(NULL,        // active Desktop
                           ID_XSH_DRIVER_HPFS386);
        break;

        case WM_DESTROY:
            // remove HPFS386.INI WPFileSystem object from cnr
            if (pddd->pvUser)
            {
                WPFileSystem *pIniFile = (WPFileSystem*)pddd->pvUser;
                _wpCnrRemoveObject(pIniFile,
                                   WinWindowFromID(hwndDlg,
                                                   ID_OSDI_HPFS386INI_CNR));
            }

            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ S506ADAPTER:
 *      structure for storing S506 driver
 *      adapter settings.
 *
 *      Two fields of this struct are stored
 *      in S506ALL.
 */

typedef struct _S506ADAPTER
{
    BOOL    fParsed;            // TRUE if adapter has been parsed

    BOOL    fIgnore;            // /I
    ULONG   b3AllowReset;       // /!R; tri-state
    BOOL    fBaseAddress;       // /P:hex
    ULONG   ulBaseAddress;
    BOOL    fIRQ;               // /IRQ:dec
    ULONG   ulIRQ;
    BOOL    fDMAChannel;        // /DC:hex
    ULONG   ulDMAChannel;
    BOOL    fDMAScatterGatherAddr; // /DSGP:xxx
    ULONG   ulDMAScatterGatherAddr;
    ULONG   b3BusMaster;        // /!BM; tri-state (for adapter)
} S506ADAPTER, *PS506ADAPTER;

/*
 *@@ S506UNIT:
 *      structure for storing S506 driver
 *      adapter settings.
 *
 *      Four fields of this struct are stored
 *      in S506ALL.
 */

typedef struct _S506UNIT
{
    BOOL    fParsed;            // TRUE if unit has been parsed

    BOOL    fRecoveryTime;      // /T:dec
    ULONG   ulRecoveryTime;
    BOOL    fGeometry;          // /GEO: either /GEO:<biostype> (0-47) or /GEO:<c>,<h>,<s>
    CHAR    szGeometry[30];
    ULONG   b3SMS;              // /!SMS; tri-state
    BOOL    fLBA;               // /LBA
    ULONG   b3DASD;             // /!DM; tri-state
    BOOL    fForce;             // /FORCE or /F
    ULONG   b3ATAPI;            // /!ATAPI; tri-state
    ULONG   b3BusMaster;        // /!BM; tri-state (for unit)

    // DANIS506.ADD extensions
    BOOL    fTimeout;           // /TI:dec
    ULONG   ulTimeout;
    BOOL    fLimitRate;         // /MR:uda
    BYTE    bUDMA,
            bMWDMA,
            bPIO;
    BYTE    b3Removeable;       // /[!]RMV
} S506UNIT, *PS506UNIT;

#define DRVT_IBM1S506            1
#define DRVT_DANIS506            2

/*
 *@@ S506ALL:
 *      structure for global IBM1S506 settings.
 *      This also holds the other two adapter
 *      and unit structures (S506ADAPTER, S506UNIT).
 *
 *      This is stored in pvUser of DRIVERDLGDATA
 *      of drv_fnwpConfigIBM1S506, so we always have
 *      access to this (because DRIVERDLGDATA got
 *      stored in QWL_USER).
 */

typedef struct _S506ALL
{
    ULONG       ulDriverType;
                    // specifies the driver type we're dealing with:
                    // -- DRVT_IBM1S506
                    // -- DRVT_DANIS506
    ULONG       ulVersionMajor,     // for IBM1S506: major version (9 or 10)
                ulVersionMinor;     // for IBM1S506: minor version

    ULONG       bInit;              // 0: quiet, 1: verbose (/V), 2: wait (/W)
    ULONG       b3ScatterGather;    // /!DSG; tri-state

    BOOL        fPCIClock;          // /PCLK:n (DANIS506.ADD)
    ULONG       ulPCIClock;         // 0 = 25MHz, 1 = 33MHz, 2 = 37MHz, 3 = 41MHz
    BOOL        fGBM;               // /GBM (DANIS506.ADD)
    BOOL        fForceGBM;          // /FORCEGBM (DANIS506.ADD)
    BOOL        fMGAFix;            // /MGAFIX   (DANIS506.ADD)

    ULONG       bCurrentAdapter;    // currently selected adapter, 0 or 1
    ULONG       bCurrentUnit;       // currently selected unit, 0 thru 3

    S506ADAPTER Adapters[2];        // two adapter settings structs
    S506UNIT    Units[4];           // four unit settings structs

    HWND        hwndTooltip;        // tooltip control (comctl.c)
    BOOL        fShowTooltips;      // tooltip control currently enabled
    XSTRING     strTooltipString;   // tooltip string for TTN_NEEDTEXT
} S506ALL, *PS506ALL;

#define DMACHANNELS_COUNT 4
// 4 PSZ's for DMA spinbutton string array
PSZ     apszDMAChannels[DMACHANNELS_COUNT];

// array of tools to be subclassed for tooltips
static USHORT usS506ToolIDs[] =
    {
        ID_OSDI_S506_INITQUIET,
        ID_OSDI_S506_INITVERBOSE,
        ID_OSDI_S506_INITWAIT,
        ID_OSDI_S506_DSG,
        ID_OSDI_S506_ADAPTER0,
        ID_OSDI_S506_ADAPTER1,
        ID_OSDI_S506_A_IGNORE,
        ID_OSDI_S506_A_RESET,
        ID_OSDI_S506_A_BASEADDR_CHECK,
        ID_OSDI_S506_A_BASEADDR_ENTRY,
        ID_OSDI_S506_A_IRQ_CHECK,
        ID_OSDI_S506_A_IRQ_SLIDER      ,
        ID_OSDI_S506_A_DMA_CHECK,
        ID_OSDI_S506_A_DMA_SPIN,
        ID_OSDI_S506_A_DSGADDR_CHECK,
        ID_OSDI_S506_A_DSGADDR_ENTRY,
        ID_OSDI_S506_A_BUSMASTER,
        ID_OSDI_S506_UNIT0,
        ID_OSDI_S506_UNIT1,
        ID_OSDI_S506_UNIT2,
        ID_OSDI_S506_UNIT3,
        ID_OSDI_S506_U_BUSMASTER,
        ID_OSDI_S506_U_RECOVERY_CHECK,
        ID_OSDI_S506_U_RECOVERY_SLIDER ,
        ID_OSDI_S506_U_GEO_CHECK,
        ID_OSDI_S506_U_GEO_ENTRY,
        ID_OSDI_S506_U_SMS,
        ID_OSDI_S506_U_LBA,
        ID_OSDI_S506_U_DASD            ,
        ID_OSDI_S506_U_FORCE,
        ID_OSDI_S506_U_ATAPI,
        ID_OSDI_S506_NEWPARAMS,

        ID_OSDI_DANIS506_CLOCK_CHECK   ,
        ID_OSDI_DANIS506_CLOCK_SLIDER  ,
        ID_OSDI_DANIS506_GBM           ,
        ID_OSDI_DANIS506_FORCEGBM      ,
        ID_OSDI_DANIS506_MGAFIX        ,
        ID_OSDI_DANIS506_U_TIMEOUT_CHECK   ,
        ID_OSDI_DANIS506_U_TIMEOUT_SPIN    ,
        ID_OSDI_DANIS506_U_RATE_CHECK      ,
        ID_OSDI_DANIS506_U_RATE_UDMA_SPIN  ,
        ID_OSDI_DANIS506_U_RATE_MWDMA_SPIN ,
        ID_OSDI_DANIS506_U_RATE_PIO_SPIN   ,
        ID_OSDI_DANIS506_U_REMOVEABLE,

        DID_OK,
        DID_CANCEL,
        DID_DEFAULT
    };

/*
 *@@ SetS506Defaults:
 *      this gets called from drv_fnwpConfigIBM1S506 upon
 *      initialization and when the "Defaults" button
 *      is pressed to (re)set the data in S506ALL to
 *      default values.
 */

STATIC VOID SetS506Defaults(PS506ALL pS506All)
{
    ULONG   ul = 0;
    // defaults for global parameters
    pS506All->bInit = 0;
    pS506All->b3ScatterGather = 2;      // indeterminate
    pS506All->fPCIClock = FALSE;
    pS506All->ulPCIClock = 1;   // 33 MHz
    pS506All->fGBM = FALSE;
    pS506All->fForceGBM = FALSE;
    pS506All->fMGAFix = FALSE;

    // defaults for adapter parameters
    for (ul = 0;
         ul < 2;
         ul++)
    {
        pS506All->Adapters[ul].fParsed = FALSE;

        pS506All->Adapters[ul].fIgnore = FALSE;
        pS506All->Adapters[ul].b3AllowReset = 2; // indeterminate
        pS506All->Adapters[ul].fBaseAddress = FALSE;
        pS506All->Adapters[ul].fIRQ = FALSE;
        pS506All->Adapters[ul].fDMAChannel = FALSE;
        pS506All->Adapters[ul].ulDMAChannel = 3;
        pS506All->Adapters[ul].fDMAScatterGatherAddr = FALSE;
        pS506All->Adapters[ul].ulDMAScatterGatherAddr = 0x400;
        pS506All->Adapters[ul].b3BusMaster = 2;
    }
    pS506All->Adapters[0].ulIRQ = 14;
    pS506All->Adapters[1].ulIRQ = 15;
    pS506All->Adapters[0].ulBaseAddress = 0x1f0;
    pS506All->Adapters[1].ulBaseAddress = 0x170;

    // defaults for unit parameters
    for (ul = 0;
         ul < 4;
         ul++)
    {
        pS506All->Units[ul].fParsed = FALSE;
        pS506All->Units[ul].fRecoveryTime = FALSE;
        pS506All->Units[ul].ulRecoveryTime = 30;
        pS506All->Units[ul].fGeometry = FALSE;
        pS506All->Units[ul].szGeometry[0] = 0;
        pS506All->Units[ul].b3SMS = 2;
        pS506All->Units[ul].fLBA = FALSE;
        pS506All->Units[ul].b3DASD = 2;
        pS506All->Units[ul].fForce = FALSE;
        pS506All->Units[ul].b3ATAPI = 2;
        pS506All->Units[ul].b3BusMaster = 2;

        pS506All->Units[ul].fTimeout = FALSE;
        pS506All->Units[ul].ulTimeout = 0;
        pS506All->Units[ul].fLimitRate = FALSE;
        pS506All->Units[ul].bUDMA = 0;
        pS506All->Units[ul].bMWDMA = 0;
        pS506All->Units[ul].bPIO = 4;
        pS506All->Units[ul].b3Removeable = 2;
    }
}

/*
 *@@ S506ParseParamsString:
 *      This was extracted from drv_fnwpConfigIBM1S506
 *      with V0.9.3.
 *
 *@@added V0.9.3 (2000-04-10) [umoeller]
 */

STATIC VOID S506ParseParamsString(PDRIVERDLGDATA pddd)
{
    PSZ         pszParamsCopy = 0,
                pszToken = 0;
    PS506ALL    pS506All = (PS506ALL)pddd->pvUser;

    // now parse the parameters
    // and update the settings
    if (pszParamsCopy = strdup(pddd->szParams))
    {
        LONG            lAdapter = -1,      // current adapter = none
                        lUnit = -1;         // current unit = none
        PS506ADAPTER    pS506AdapterThis = NULL; // current adapter = none
        PS506UNIT       pS506UnitThis = NULL;    // current unit = none
        PSZ             pszError = NULL;    // error message = none
        XSTRING         strUnrecognized;    // unrecognized params = none

        xstrInit(&strUnrecognized, 100);

        // strtok loop
        pszToken = strtok(pszParamsCopy, " ");
        if (pszToken) do
        {
            // adapter spec?
            if (!memicmp(pszToken, "/A:", 3))
            {
                LONG lNew = strtoul(pszToken + 3, 0, 0);
                if ((lNew < 0) || (lNew > 1))
                    pszError = "Invalid adapter number specified (must be 0 or 1).";
                else
                {
                    lAdapter = lNew;
                    pS506AdapterThis = &pS506All->Adapters[lAdapter];
                    if (pS506AdapterThis->fParsed)
                        pszError = "Adapter specified twice.";
                    else
                    {
                        pS506AdapterThis->fParsed = TRUE;
                        // invalidate current unit
                        pS506UnitThis = NULL;
                    }
                }
            }

            // unit spec?
            else if (!memicmp(pszToken, "/U:", 3))
            {
                if (lAdapter == -1)
                    pszError = "Unit without previous adapter specified.";
                else
                {
                    LONG lNew = strtoul(pszToken + 3, 0, 0);
                    if ((lNew < 0) || (lNew > 1))
                        pszError = "Invalid unit number specified (must be 0 or 1).";
                    else
                    {
                        lUnit = lNew;
                        if (lAdapter == 1)
                            lUnit += 2;        // now we have a range of 0 thru 3
                        pS506UnitThis = &pS506All->Units[lUnit];
                        if (pS506UnitThis->fParsed)
                            pszError = "Unit specified twice.";
                        else
                        {
                            pS506UnitThis->fParsed = TRUE;
                            // invalidate current adapter
                            pS506AdapterThis = NULL;
                        }
                    }
                }
            }

            // global settings
            else if (!stricmp(pszToken, "/V"))
                pS506All->bInit = 1;
            else if (!stricmp(pszToken, "/!V"))
                pS506All->bInit = 0;
            else if (!stricmp(pszToken, "/W"))
                pS506All->bInit = 2;
            else if (!stricmp(pszToken, "/DSG"))
                pS506All->b3ScatterGather = 1;
            else if (!stricmp(pszToken, "/!DSG"))
                pS506All->b3ScatterGather = 0;
            else if (!memicmp(pszToken, "/PCLK:", 6))
            {
                pS506All->fPCIClock = TRUE;
                if (!strcmp(pszToken+6, "25"))
                    pS506All->ulPCIClock = 0;
                else if (!strcmp(pszToken+6, "33"))
                    pS506All->ulPCIClock = 1;
                else if (!strcmp(pszToken+6, "37"))
                    pS506All->ulPCIClock = 2;
                else
                    pS506All->ulPCIClock = 3;
            }
            else if (!stricmp(pszToken, "/GBM"))
                pS506All->fGBM = TRUE;
            else if (!stricmp(pszToken, "/FORCEGBM"))
                pS506All->fForceGBM = TRUE;
            else if (!stricmp(pszToken, "/MGAFIX"))
                pS506All->fMGAFix = TRUE;

            // adapter settings
            else if (!stricmp(pszToken, "/I"))
                if (pS506AdapterThis == NULL)
                    pszError = "Parameter out of adapter context.";
                else
                    pS506AdapterThis->fIgnore = TRUE;
            else if (!memicmp(pszToken, "/R", 2))
                if (pS506AdapterThis == NULL)
                    pszError = "Parameter out of adapter context.";
                else
                    pS506AdapterThis->b3AllowReset = 1;
            else if (!memicmp(pszToken, "/!R", 3))
                if (pS506AdapterThis == NULL)
                    pszError = "Parameter out of adapter context.";
                else
                    pS506AdapterThis->b3AllowReset = 0;
            else if (!memicmp(pszToken, "/P:", 3))
                if (pS506AdapterThis == NULL)
                    pszError = "Parameter out of adapter context.";
                else
                {
                    pS506AdapterThis->fBaseAddress = TRUE;
                    pS506AdapterThis->ulBaseAddress = strtoul(pszToken + 3, 0,
                                                      16); // hex
                }
            else if (!memicmp(pszToken, "/IRQ:", 5))
                if (pS506AdapterThis == NULL)
                    pszError = "Parameter out of adapter context.";
                else
                {
                    pS506AdapterThis->fIRQ = TRUE;
                    pS506AdapterThis->ulIRQ = strtoul(pszToken + 5, 0, 0);  // dec
                }
            else if (!memicmp(pszToken, "/DC:", 4))
                if (pS506AdapterThis == NULL)
                    pszError = "Parameter out of adapter context.";
                else
                {
                    pS506AdapterThis->fDMAChannel = TRUE;
                    pS506AdapterThis->ulDMAChannel = strtoul(pszToken + 4, 0, 0);  // dec
                }
            else if (!memicmp(pszToken, "/DSGP:", 6))
                if (pS506AdapterThis == NULL)
                    pszError = "Parameter out of adapter context.";
                else
                {
                    pS506AdapterThis->fDMAScatterGatherAddr = TRUE;
                    pS506AdapterThis->ulDMAScatterGatherAddr = strtoul(pszToken + 6, 0,
                                                               16);  // hex
                }
            else if (!memicmp(pszToken, "/BM", 3))
                // busmaster: can be specified both
                // as an adapter and user parameter
                if (pS506AdapterThis == NULL)
                    if (pS506UnitThis == NULL)
                        pszError = "/[!]BM must be specified either with an adapter or unit.";
                    else
                        // unit:
                        pS506UnitThis->b3BusMaster = 1;
                else
                    // adapter:
                    pS506AdapterThis->b3BusMaster = 1;
            else if (!memicmp(pszToken, "/!BM", 3))
                // busmaster: can be specified both
                // as an adapter and user parameter
                if (pS506AdapterThis == NULL)
                    if (pS506UnitThis == NULL)
                        pszError = "/[!]BM must be specified either with an adapter or unit.";
                    else
                        // unit:
                        pS506UnitThis->b3BusMaster = 0;
                else
                    // adapter:
                    pS506AdapterThis->b3BusMaster = 0;

            // unit settings
            else if (!memicmp(pszToken, "/T:", 3))
                if (pS506UnitThis == NULL)
                    pszError = "Parameter out of unit context.";
                else
                {
                    pS506UnitThis->fRecoveryTime = TRUE;
                    pS506UnitThis->ulRecoveryTime = strtoul(pszToken + 3, 0, 0); // dec
                }
            else if (!memicmp(pszToken, "/GEO:", 5))
                if (pS506UnitThis == NULL)
                    pszError = "Parameter out of unit context.";
                else
                {
                    pS506UnitThis->fGeometry = TRUE;
                    strlcpy(pS506UnitThis->szGeometry,
                            pszToken + 5,
                            sizeof(pS506UnitThis->szGeometry));
                }
            else if (!memicmp(pszToken, "/SMS", 4))
                if (pS506UnitThis == NULL)
                    pszError = "Parameter out of unit context.";
                else
                    pS506UnitThis->b3SMS = 1;
            else if (!memicmp(pszToken, "/!SMS", 5))
                if (pS506UnitThis == NULL)
                    pszError = "Parameter out of unit context.";
                else
                    pS506UnitThis->b3SMS = 0;
            else if (!memicmp(pszToken, "/LBA", 4))
                if (pS506UnitThis == NULL)
                    pszError = "Parameter out of unit context.";
                else
                    pS506UnitThis->fLBA = TRUE;
            else if (!memicmp(pszToken, "/DM", 3))
                if (pS506UnitThis == NULL)
                    pszError = "Parameter out of unit context.";
                else
                    pS506UnitThis->b3DASD = 1;
            else if (!memicmp(pszToken, "/!DM", 4))
                if (pS506UnitThis == NULL)
                    pszError = "Parameter out of unit context.";
                else
                    pS506UnitThis->b3DASD = 0;
            else if (!memicmp(pszToken, "/F", 2))
                if (pS506UnitThis == NULL)
                    pszError = "Parameter out of unit context.";
                else
                    pS506UnitThis->fForce = TRUE;
            else if (!memicmp(pszToken, "/ATAPI", 6))
                if (pS506UnitThis == NULL)
                    pszError = "Parameter out of unit context.";
                else
                    pS506UnitThis->b3ATAPI = 1;
            else if (!memicmp(pszToken, "/!ATAPI", 6))
                if (pS506UnitThis == NULL)
                    pszError = "Parameter out of unit context.";
                else
                    pS506UnitThis->b3ATAPI = 0;
            else if (!memicmp(pszToken, "/IT:", 4))
                if (pS506UnitThis == NULL)
                    pszError = "Parameter out of unit context.";
                else
                {
                    pS506UnitThis->fTimeout = TRUE;
                    pS506UnitThis->ulTimeout = strtoul(pszToken + 4, 0, 0);
                }
            else if (!memicmp(pszToken, "/MR:", 4))
                if (pS506UnitThis == NULL)
                    pszError = "Parameter out of unit context.";
                else
                {
                    pS506UnitThis->fLimitRate = TRUE;
                    pS506UnitThis->bUDMA = *(pszToken + 4) - '0';
                    pS506UnitThis->bMWDMA = *(pszToken + 5) - '0';
                    pS506UnitThis->bPIO = *(pszToken + 6) - '0';
                }
            else if (!memicmp(pszToken, "/RMV", 4))
                if (pS506UnitThis == NULL)
                    pszError = "Parameter out of unit context.";
                else
                    pS506UnitThis->b3Removeable = 1;
            else if (!memicmp(pszToken, "/!RMV", 4))
                if (pS506UnitThis == NULL)
                    pszError = "Parameter out of unit context.";
                else
                    pS506UnitThis->b3Removeable = 0;

            else
            {
                // unrecognized param:
                // append to string, which will be
                // displayed later with a warning
                xstrcat(&strUnrecognized, pszToken, 0);
                xstrcatc(&strUnrecognized, ' ');
            }

            if (pszError)
            {
                CHAR szMsg[500];
                sprintf(szMsg, "Syntax error in parameters: %s\n\nCurrent token: %s"
                               "\n\nWarning: All subsequent parameters have been discarded.\n",
                        pszError, pszToken);
                winhDebugBox(NULLHANDLE, "IBM1S506.ADD", szMsg);
                break;
            }
        } while (pszToken = strtok(NULL, " "));

        free(pszParamsCopy);

        if (strUnrecognized.ulLength)
        {
            // anything unrecognized:
            CHAR szMsg[500];
            sprintf(szMsg, "Warning: The following parameters were not recognized and will be ignored:"
                           "\n\n%s\n\n",
                    strUnrecognized.psz);
            winhDebugBox(NULLHANDLE, "IBM1S506.ADD", szMsg);
        }

        xstrClear(&strUnrecognized);
    } // end if (pszParamsCopy)
}

/*
 *@@ RecomposeParamsString:
 *      returns a new complete S506 driver parameters
 *      string in a new buffer, based on the current
 *      dialog settings. The caller must free() the
 *      buffer.
 *
 *      Returns NULL if no params have been defined.
 */

STATIC PSZ RecomposeParamsString(HWND hwndDlg,         // in: driver dialog
                                 PS506ALL pS506All)    // in: driver settings
{
    XSTRING     strParams,
                strAdapterParams[2],
                strUnitParams[4];
    // PS506ADAPTER pS506AdapterThis = 0;
    // PS506UNIT   pS506UnitThis = 0;
    ULONG       ul = 0;
    CHAR        szTemp[40];

    xstrInit(&strParams, 0);
    xstrInit(&strAdapterParams[0], 0);
    xstrInit(&strAdapterParams[1], 0);
    xstrInit(&strUnitParams[0], 0);
    xstrInit(&strUnitParams[1], 0);
    xstrInit(&strUnitParams[2], 0);
    xstrInit(&strUnitParams[3], 0);

    // go thru units and compose unit strings array
    for (ul = 0;
         ul < 4;
         ul++)
    {
        if (pS506All->Units[ul].fRecoveryTime)
        {
            sprintf(szTemp, "/T:%d ", pS506All->Units[ul].ulRecoveryTime);
            xstrcpy(&strUnitParams[ul], szTemp, 0);
        }

        if (pS506All->Units[ul].fGeometry == TRUE)
        {
            sprintf(szTemp, "/GEO:%s ", pS506All->Units[ul].szGeometry);
            xstrcat(&strUnitParams[ul], szTemp, 0);
        }

        if (pS506All->Units[ul].b3BusMaster == 0)
            xstrcat(&strUnitParams[ul], "/!BM ", 0);
        else if (pS506All->Units[ul].b3BusMaster == 1)
            xstrcat(&strUnitParams[ul], "/BM ", 0);

        if (pS506All->Units[ul].b3SMS == 0)
            xstrcat(&strUnitParams[ul], "/!SMS ", 0);
        else if (pS506All->Units[ul].b3SMS == 1)
            xstrcat(&strUnitParams[ul], "/SMS ", 0);

        if (pS506All->Units[ul].fLBA == TRUE)
            xstrcat(&strUnitParams[ul], "/LBA ", 0);

        if (pS506All->Units[ul].b3DASD == 0)
            xstrcat(&strUnitParams[ul], "/!DM ", 0);
        else if (pS506All->Units[ul].b3DASD == 1)
            xstrcat(&strUnitParams[ul], "/DM ", 0);

        if (pS506All->Units[ul].b3ATAPI == 0)
            xstrcat(&strUnitParams[ul], "/!ATAPI ", 0);
        else if (pS506All->Units[ul].b3ATAPI == 1)
            xstrcat(&strUnitParams[ul], "/ATAPI ", 0);

        if (pS506All->Units[ul].fForce == TRUE)
            xstrcat(&strUnitParams[ul], "/FORCE ", 0);

        if (pS506All->Units[ul].fTimeout == TRUE)
        {
            sprintf(szTemp, "/IT:%d ", pS506All->Units[ul].ulTimeout);
            xstrcat(&strUnitParams[ul], szTemp, 0);
        }

        if (pS506All->Units[ul].fLimitRate == TRUE)
        {
            sprintf(szTemp, "/MR:%d%d%d ",
                    pS506All->Units[ul].bUDMA,
                    pS506All->Units[ul].bMWDMA,
                    pS506All->Units[ul].bPIO);
            xstrcat(&strUnitParams[ul], szTemp, 0);
        }

        if (pS506All->Units[ul].b3Removeable == 0)
            xstrcat(&strUnitParams[ul], "/!RMV ", 0);
        else if (pS506All->Units[ul].b3Removeable == 1)
            xstrcat(&strUnitParams[ul], "/RMV ", 0);
    }

    // go thru adapters and compose adapter strings array
    for (ul = 0;
         ul < 2;
         ul++)
    {
        if (pS506All->Adapters[ul].fIgnore == TRUE)
            xstrcat(&strAdapterParams[ul], "/I ", 0);
        else
        {
            if (pS506All->Adapters[ul].b3AllowReset == 0)
                xstrcat(&strAdapterParams[ul], "/!R ", 0);
            else if (pS506All->Adapters[ul].b3AllowReset == 1)
                xstrcat(&strAdapterParams[ul], "/R ", 0);

            if (pS506All->Adapters[ul].b3BusMaster == 0)
                xstrcat(&strAdapterParams[ul], "/!BM ", 0);
            else if (pS506All->Adapters[ul].b3BusMaster == 1)
                xstrcat(&strAdapterParams[ul], "/BM ", 0);

            if (pS506All->Adapters[ul].fBaseAddress)
            {
                sprintf(szTemp, "/P:%lX ", pS506All->Adapters[ul].ulBaseAddress);
                xstrcat(&strAdapterParams[ul], szTemp, 0);
            }

            if (pS506All->Adapters[ul].fIRQ)
            {
                sprintf(szTemp, "/IRQ:%d ", pS506All->Adapters[ul].ulIRQ);
                xstrcat(&strAdapterParams[ul], szTemp, 0);
            }

            if (pS506All->Adapters[ul].fDMAChannel)
            {
                sprintf(szTemp, "/DC:%d ", pS506All->Adapters[ul].ulDMAChannel);
                xstrcat(&strAdapterParams[ul], szTemp, 0);
            }

            if (pS506All->Adapters[ul].fDMAScatterGatherAddr)
            {
                sprintf(szTemp, "/DSGP:%lX ", pS506All->Adapters[ul].ulDMAScatterGatherAddr);
                xstrcat(&strAdapterParams[ul], szTemp, 0);
            }
        }
    }

    // now start composing the complete return string;
    // we'll start with the global parameters
    if (pS506All->bInit == 1)
        xstrcpy(&strParams, "/V ", 0);
    else if (pS506All->bInit == 2)
        xstrcpy(&strParams, "/W ", 0);

    if (pS506All->b3ScatterGather == 0)
        xstrcat(&strParams, "/!DSG ", 0);
    else if (pS506All->b3ScatterGather == 1)
        xstrcat(&strParams, "/DSG ", 0);

    if (pS506All->fPCIClock == TRUE)
    {
        sprintf(szTemp,
                "/PCLK:%s ",
                // 0 = 25MHz, 1 = 33MHz, 2 = 37MHz, 3 = 41MHz
                (pS506All->ulPCIClock == 0) ? "25"
                : (pS506All->ulPCIClock == 1) ? "33"
                : (pS506All->ulPCIClock == 2) ? "37"
                : "41");
        xstrcat(&strParams, szTemp, 0);
    }

    if (pS506All->fGBM == TRUE)
        xstrcat(&strParams, "/GBM ", 0);

    if (pS506All->fForceGBM == TRUE)
        xstrcat(&strParams, "/FORCEGBM ", 0);

    if (pS506All->fMGAFix == TRUE)
        xstrcat(&strParams, "/MGAFIX ", 0);

    // go over the adapters:
    // adapter 0
    if (    (strAdapterParams[0].ulLength)
         || (strUnitParams[0].ulLength)
         || (strUnitParams[1].ulLength)
       )
    {
        xstrcat(&strParams, "/A:0 ", 0);
        if (strAdapterParams[0].ulLength)
        {
            // string for adapter 0:
            xstrcat(&strParams, strAdapterParams[0].psz, 0);
        }

        if (pS506All->Adapters[0].fIgnore == FALSE)
        {
            if (strUnitParams[0].ulLength)
            {
                // string for adapter 0, unit 0:
                xstrcat(&strParams, "/U:0 ", 0);
                xstrcat(&strParams, strUnitParams[0].psz, 0);
            }
            if (strUnitParams[1].ulLength)
            {
                // string for adapter 0, unit 1:
                xstrcat(&strParams, "/U:1 ", 0);
                xstrcat(&strParams, strUnitParams[1].psz, 0);
            }
        }
    }

    // adapter 1
    if (    (strAdapterParams[1].ulLength)
         || (strUnitParams[2].ulLength)
         || (strUnitParams[3].ulLength)
       )
    {
        xstrcat(&strParams, "/A:1 ", 0);
        if (strAdapterParams[1].ulLength)
        {
            // string for adapter 1:
            xstrcat(&strParams,
                    strAdapterParams[1].psz,
                    strAdapterParams[1].ulLength);
        }

        if (pS506All->Adapters[1].fIgnore == FALSE)
        {
            if (strUnitParams[2].ulLength)
            {
                // string for adapter 1, unit 0:
                xstrcat(&strParams, "/U:0 ", 0);      // /A:1 /U:0 == unit 2
                xstrcat(&strParams,
                        strUnitParams[2].psz,
                        strUnitParams[2].ulLength);
            }
            if (strUnitParams[3].ulLength)
            {
                // string for adapter 1, unit 1:
                xstrcat(&strParams, "/U:1 ", 0);      // /A:1 /U:1 == unit 3
                xstrcat(&strParams,
                        strUnitParams[3].psz,
                        strUnitParams[3].ulLength);

            }
        }
    }

    xstrClear(&strAdapterParams[0]);
    xstrClear(&strAdapterParams[1]);
    xstrClear(&strUnitParams[0]);
    xstrClear(&strUnitParams[1]);
    xstrClear(&strUnitParams[2]);
    xstrClear(&strUnitParams[3]);

    return strParams.psz;
}

/*
 *@@ S506Settings2Dlg:
 *      sets the controls' data in the dialog according
 *      to the current settings in S506ALL.
 *      This was extracted from drv_fnwpConfigIBM1S506
 *      with V0.9.3.
 *
 *@@added V0.9.3 (2000-04-10) [umoeller]
 */

STATIC VOID S506Settings2Dlg(HWND hwndDlg,
                             PDRIVERDLGDATA pddd)
{
    CHAR            szTemp[30];
    ULONG           ul = 0;
    PS506ALL        pS506All = (PS506ALL)pddd->pvUser;

    // find current adapter pointer
    PS506ADAPTER    pS506AdapterThis = &pS506All->Adapters[pS506All->bCurrentAdapter];
    // find current unit pS506All->bCurrentUnit
    PS506UNIT       pS506UnitThis = &pS506All->Units[pS506All->bCurrentUnit];

    /*
     *  global settings
     */

    switch (pS506All->bInit)
    {
        case 1:
            winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_INITVERBOSE, TRUE); break;
        case 2:
            winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_INITWAIT, TRUE); break;
        default:
            winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_INITQUIET, TRUE); break;
    }

    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_DSG,
                          pS506All->b3ScatterGather);

    winhSetDlgItemChecked(hwndDlg, ID_OSDI_DANIS506_CLOCK_CHECK,
                          pS506All->fPCIClock);
    winhSetSliderArmPosition(WinWindowFromID(hwndDlg,
                                             ID_OSDI_DANIS506_CLOCK_SLIDER),
                             SMA_INCREMENTVALUE, pS506All->ulPCIClock);

    winhSetDlgItemChecked(hwndDlg, ID_OSDI_DANIS506_GBM,
                          pS506All->fGBM);
    winhSetDlgItemChecked(hwndDlg, ID_OSDI_DANIS506_FORCEGBM,
                          pS506All->fForceGBM);
    winhSetDlgItemChecked(hwndDlg, ID_OSDI_DANIS506_MGAFIX,
                          pS506All->fMGAFix);

    /*
     *  adapter settings
     */

    // /I
    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_A_IGNORE,
                          pS506AdapterThis->fIgnore);

    // /!R; tri-state
    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_A_RESET,
                          pS506AdapterThis->b3AllowReset);

    // /P
    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_A_BASEADDR_CHECK,
                          pS506AdapterThis->fBaseAddress);
    sprintf(szTemp, "%lX",
            pS506AdapterThis->ulBaseAddress);
    WinSetDlgItemText(hwndDlg, ID_OSDI_S506_A_BASEADDR_ENTRY, szTemp);

    // /IRQ
    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_A_IRQ_CHECK,
                          pS506AdapterThis->fIRQ);
    winhSetSliderArmPosition(WinWindowFromID(hwndDlg,
                                             ID_OSDI_S506_A_IRQ_SLIDER),
                             SMA_INCREMENTVALUE, pS506AdapterThis->ulIRQ);

    // /DC
    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_A_DMA_CHECK,
                          pS506AdapterThis->fDMAChannel);
    // spin button: ID_OSDI_S506_A_DMA_SPIN 3, 5, 6, 7
    for (ul = 0;
         ul < DMACHANNELS_COUNT;
         ul++)
        if (strtoul(apszDMAChannels[ul], 0, 0) == pS506AdapterThis->ulDMAChannel)
        {
            WinSendDlgItemMsg(hwndDlg, ID_OSDI_S506_A_DMA_SPIN,
                              SPBM_SETCURRENTVALUE,
                              (MPARAM)ul,
                              (MPARAM)0);
            break;
        }

    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_A_DSGADDR_CHECK,
                          pS506AdapterThis->fDMAScatterGatherAddr);
    sprintf(szTemp, "%lX",
            pS506AdapterThis->ulDMAScatterGatherAddr);
    WinSetDlgItemText(hwndDlg, ID_OSDI_S506_A_DSGADDR_ENTRY, szTemp);

    // /!BM; tri-state (adapter)
    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_A_BUSMASTER,
                          pS506AdapterThis->b3BusMaster);

    /*
     *  unit settings
     */

    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_U_RECOVERY_CHECK,
                          pS506UnitThis->fRecoveryTime);

    winhSetSliderArmPosition(WinWindowFromID(hwndDlg,
                                             ID_OSDI_S506_U_RECOVERY_SLIDER),
                             SMA_INCREMENTVALUE,
                             pS506UnitThis->ulRecoveryTime - 5);

    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_U_GEO_CHECK,
                          pS506UnitThis->fGeometry);
    WinSetDlgItemText(hwndDlg, ID_OSDI_S506_U_GEO_ENTRY,
                      pS506UnitThis->szGeometry);

    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_U_SMS,
                          pS506UnitThis->b3SMS);

    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_U_LBA,
                          pS506UnitThis->fLBA);

    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_U_DASD,
                          pS506UnitThis->b3DASD);

    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_U_FORCE,
                          pS506UnitThis->fForce);

    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_U_ATAPI,
                          pS506UnitThis->b3ATAPI);

    // /!BM; tri-state (unit)
    winhSetDlgItemChecked(hwndDlg, ID_OSDI_S506_U_BUSMASTER,
                          pS506UnitThis->b3BusMaster);

    winhSetDlgItemChecked(hwndDlg, ID_OSDI_DANIS506_U_TIMEOUT_CHECK,
                          pS506UnitThis->fTimeout);
    winhSetDlgItemSpinData(hwndDlg, ID_OSDI_DANIS506_U_TIMEOUT_SPIN,
                           0, 60, pS506UnitThis->ulTimeout);

    winhSetDlgItemChecked(hwndDlg, ID_OSDI_DANIS506_U_RATE_CHECK,
                          pS506UnitThis->fLimitRate);
    winhSetDlgItemSpinData(hwndDlg, ID_OSDI_DANIS506_U_RATE_UDMA_SPIN,
                           0, 5, pS506UnitThis->bUDMA);
    winhSetDlgItemSpinData(hwndDlg, ID_OSDI_DANIS506_U_RATE_MWDMA_SPIN,
                           0, 3, pS506UnitThis->bMWDMA);
    winhSetDlgItemSpinData(hwndDlg, ID_OSDI_DANIS506_U_RATE_PIO_SPIN,
                           0, 4, pS506UnitThis->bPIO);

    winhSetDlgItemChecked(hwndDlg, ID_OSDI_DANIS506_U_REMOVEABLE,
                          pS506UnitThis->b3Removeable);

    WinPostMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);
}

/*
 * ausNotSupportedByIBM:
 *      array of dlg IDs which must be disabled when
 *      this dialog is used for the IBM1S506.ADD
 *      driver.
 */

static USHORT ausNotSupportedByIBM[] =
            {
                ID_OSDI_DANIS506_CLOCK_CHECK,
                ID_OSDI_DANIS506_CLOCK_SLIDER,
                ID_OSDI_DANIS506_CLOCK_TXT,
                ID_OSDI_DANIS506_GBM,
                ID_OSDI_DANIS506_FORCEGBM,
                ID_OSDI_DANIS506_MGAFIX,

                ID_OSDI_DANIS506_U_TIMEOUT_CHECK,
                ID_OSDI_DANIS506_U_TIMEOUT_SPIN,
                ID_OSDI_DANIS506_U_RATE_CHECK,
                ID_OSDI_DANIS506_U_RATE_UDMA_TXT,
                ID_OSDI_DANIS506_U_RATE_UDMA_SPIN,
                ID_OSDI_DANIS506_U_RATE_MWDMA_TXT,
                ID_OSDI_DANIS506_U_RATE_MWDMA_SPIN,
                ID_OSDI_DANIS506_U_RATE_PIO_TXT,
                ID_OSDI_DANIS506_U_RATE_PIO_SPIN,
                ID_OSDI_DANIS506_U_REMOVEABLE
            };

/*
 * ausNotSupportedByOldIBM:
 *      more dlg IDs which must be disabled for IBM
 *      drivers with versions < 10.xxx.
 */

static USHORT ausNotSupportedByOldIBM[] =
            {
                ID_OSDI_S506_INITWAIT,
                ID_OSDI_S506_U_BUSMASTER
            };

/*
 *@@ S506EnableItems:
 *      enables/disables items in the dialog according
 *      to the settings and the driver versions.
 *      This was extracted from drv_fnwpConfigIBM1S506
 *      with V0.9.3.
 *
 *@@added V0.9.3 (2000-04-10) [umoeller]
 */

STATIC VOID S506EnableItems(HWND hwndDlg,
                            PDRIVERDLGDATA pddd)
{
    PS506ALL        pS506All = (PS506ALL)pddd->pvUser;
    // find current adapter pointer
    PS506ADAPTER    pS506AdapterThis = &pS506All->Adapters[pS506All->bCurrentAdapter];
    // find current unit pS506All->bCurrentUnit
    PS506UNIT       pS506UnitThis = &pS506All->Units[pS506All->bCurrentUnit];
    BOOL            fUnitDisabled = FALSE;

    // global items
    WinEnableControl(hwndDlg, ID_OSDI_DANIS506_CLOCK_SLIDER,
                      pS506All->fPCIClock);
    WinEnableControl(hwndDlg, ID_OSDI_DANIS506_CLOCK_TXT,
                      pS506All->fPCIClock);

    // adapter items:
    // disable all if adapter is disabled
    winhEnableControls(hwndDlg,
                       ID_OSDI_S506_A_RESET,            // first
                       ID_OSDI_S506_A_DSGADDR_ENTRY,    // last
                       !pS506AdapterThis->fIgnore);

    if (!pS506AdapterThis->fIgnore)
    {
        WinEnableControl(hwndDlg, ID_OSDI_S506_A_BASEADDR_ENTRY,
                          pS506AdapterThis->fBaseAddress);
        WinEnableControl(hwndDlg, ID_OSDI_S506_A_IRQ_SLIDER,
                          pS506AdapterThis->fIRQ);
        WinEnableControl(hwndDlg, ID_OSDI_S506_A_IRQ_TXT,
                          pS506AdapterThis->fIRQ);
        WinEnableControl(hwndDlg, ID_OSDI_S506_A_DMA_SPIN,
                          pS506AdapterThis->fDMAChannel);
        WinEnableControl(hwndDlg, ID_OSDI_S506_A_DSGADDR_ENTRY,
                          pS506AdapterThis->fDMAScatterGatherAddr);
    }

    // unit radio buttons
    WinEnableControl(hwndDlg, ID_OSDI_S506_UNIT0,
                      !pS506All->Adapters[0].fIgnore);
    WinEnableControl(hwndDlg, ID_OSDI_S506_UNIT1,
                      !pS506All->Adapters[0].fIgnore);
    WinEnableControl(hwndDlg, ID_OSDI_S506_UNIT2,
                      !pS506All->Adapters[1].fIgnore);
    WinEnableControl(hwndDlg, ID_OSDI_S506_UNIT3,
                      !pS506All->Adapters[1].fIgnore);

    // unit items:
    // disable all if corresponding adapter is disabled
    fUnitDisabled = (   (   (pS506All->Adapters[0].fIgnore)
                         && (   (pS506All->bCurrentUnit == 0)
                             || (pS506All->bCurrentUnit == 1)
                            )
                        )
                     || (   (pS506All->Adapters[1].fIgnore)
                         && (   (pS506All->bCurrentUnit == 2)
                             || (pS506All->bCurrentUnit == 3)
                            )
                        )
                    );
    winhEnableControls(hwndDlg,
                       ID_OSDI_S506_U_BUSMASTER,        // first
                       ID_OSDI_DANIS506_U_REMOVEABLE,   // last
                       !fUnitDisabled);
    if (!fUnitDisabled)
    {
        WinEnableControl(hwndDlg, ID_OSDI_S506_U_RECOVERY_SLIDER,
                          pS506UnitThis->fRecoveryTime);
        WinEnableControl(hwndDlg, ID_OSDI_S506_U_RECOVERY_TXT,
                          pS506UnitThis->fRecoveryTime);
        WinEnableControl(hwndDlg, ID_OSDI_S506_U_GEO_ENTRY,
                          pS506UnitThis->fGeometry);
        WinEnableControl(hwndDlg, ID_OSDI_DANIS506_U_TIMEOUT_SPIN,
                          pS506UnitThis->fTimeout);

        WinEnableControl(hwndDlg, ID_OSDI_DANIS506_U_RATE_UDMA_TXT,
                          pS506UnitThis->fLimitRate);
        WinEnableControl(hwndDlg, ID_OSDI_DANIS506_U_RATE_UDMA_SPIN,
                          pS506UnitThis->fLimitRate);
        WinEnableControl(hwndDlg, ID_OSDI_DANIS506_U_RATE_MWDMA_TXT,
                          pS506UnitThis->fLimitRate);
        WinEnableControl(hwndDlg, ID_OSDI_DANIS506_U_RATE_MWDMA_SPIN,
                          pS506UnitThis->fLimitRate);
        WinEnableControl(hwndDlg, ID_OSDI_DANIS506_U_RATE_PIO_TXT,
                          pS506UnitThis->fLimitRate);
        WinEnableControl(hwndDlg, ID_OSDI_DANIS506_U_RATE_PIO_SPIN,
                          pS506UnitThis->fLimitRate);
    }

    if (pS506All->ulDriverType == DRVT_IBM1S506)
    {
        // IBM driver: disable DANIS506.ADD features
        ULONG ul;
        for (ul = 0;
             ul < sizeof(ausNotSupportedByIBM) / sizeof(ausNotSupportedByIBM[0]);
             ul++)
        {
            WinEnableControl(hwndDlg,
                             ausNotSupportedByIBM[ul],
                             FALSE);
        }

        if (pS506All->ulVersionMajor < 10)
        {
            // for old IBM1S506 drivers, disable newer
            // features
            for (ul = 0;
                 ul < sizeof(ausNotSupportedByOldIBM) / sizeof(ausNotSupportedByOldIBM[0]);
                 ul++)
            {
                WinEnableControl(hwndDlg,
                                 ausNotSupportedByOldIBM[ul],
                                 FALSE);
            }
        }
    }
}

/*
 *@@ drv_fnwpConfigIBM1S506:
 *      monster dialog procedure for the "Configure IBM1S506.ADD"
 *      dialog.
 *
 *      This gets called automatically from cfgDriversItemChanged
 *      (xfsys.c) when the "Configure" button is pressed.
 *
 *      As with all driver dialogs, this gets a DRIVERDLGDATA
 *      structure with mp2 in WM_INITDLG.
 *
 *      This dialog maintains a S506ALL in DRIVERDLGDATA.pvUser,
 *      in which we store the various adapter/unit settings
 *      even if they are currently invisible.
 *
 *      In the S506ALL structure, we have two S506ADAPTER
 *      and four S506UNIT structures to allow for all the
 *      different settings. This function parses the parameters
 *      string into all those structures, sets up the dialog
 *      controls accordingly, and recomposes the parameters
 *      string according to the controls' changes.
 *
 *      This dialog uses the new tooltip control (ctl_fnwpTooltip,
 *      comctl.c) to display fly-over help for the various
 *      dlg items. This works by setting TOOLINFO.lpszText to
 *      LPSTR_TEXTCALLBACK, which will cause a WM_CONTROL with
 *      TTN_NEEDTEXT when the tooltip needs a tool text, upon
 *      which we will query the XWorkplace message file
 *      (XFLDRxxx.TMF, tmfGetMessage) for a proper message.
 *
 *      Whoa. I don't think I've ever written a dialog func
 *      which was longer than this one.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.3 (2000-04-10) [umoeller]: crashed on OK if no params were set; fixed
 *@@changed V0.9.3 (2000-04-10) [umoeller]: added IBM1S506.ADD support
 *@@changed V0.9.6 (2000-11-12) [umoeller]: fixed free(0) call
 */

MRESULT EXPENTRY drv_fnwpConfigIBM1S506(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PDRIVERDLGDATA pddd = WinQueryWindowPtr(hwndDlg, QWL_USER);
    MRESULT mrc = 0;

    switch (msg)
    {
        /*
         * WM_INITDLG:
         *      initialize controls, set up defaults,
         *      parse DRIVERDLGDATA.szParams, set
         *      controls' data accordingly...
         */

        case WM_INITDLG:
        {
            PS506ALL    pS506All = malloc(sizeof(S506ALL));
            ULONG       ul = 0;
            // LONG        lColor;
            // CHAR        szFont[] = "8.Helv";

            memset(pS506All, 0, sizeof(S506ALL));

            // store DRIVERDLGDATA in window words
            pddd = (PDRIVERDLGDATA)mp2;
            WinSetWindowPtr(hwndDlg, QWL_USER, pddd);

            // store S506ALL in DRIVERDLGDATA
            memset(pS506All, 0, sizeof(S506ALL));
            pddd->pvUser = pS506All;

            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);

            // initialize dialog data
            if (!stricmp(pddd->pDriverSpec->pszFilename, "DANIS506.ADD"))
                pS506All->ulDriverType = DRVT_DANIS506;
            else
            {
                pS506All->ulDriverType = DRVT_IBM1S506;

                if (pddd->pDriverSpec->pszVersion)
                {
                    sscanf(pddd->pDriverSpec->pszVersion,
                           "%d.%d",
                           &pS506All->ulVersionMajor,
                           &pS506All->ulVersionMinor);
                }
            }

            pS506All->bCurrentAdapter = 0;
            pS506All->bCurrentUnit = 0;

            // set driver, adapter, unit defaults
            SetS506Defaults(pS506All);

            S506ParseParamsString(pddd);

            // entry fields
            winhSetEntryFieldLimit(WinWindowFromID(hwndDlg, ID_OSDI_S506_A_BASEADDR_ENTRY),
                                   3);
            winhSetEntryFieldLimit(WinWindowFromID(hwndDlg, ID_OSDI_S506_A_DSGADDR_ENTRY),
                                   3);
            winhSetEntryFieldLimit(WinWindowFromID(hwndDlg, ID_OSDI_S506_U_GEO_ENTRY),
                                   30);
            winhSetEntryFieldLimit(WinWindowFromID(hwndDlg, ID_OSDI_S506_NEWPARAMS),
                                   500);

            // sliders
            winhSetSliderTicks(WinWindowFromID(hwndDlg,
                                               ID_OSDI_S506_A_IRQ_SLIDER),
                               0, 4,
                               (MPARAM)-1, -1);

            winhSetSliderTicks(WinWindowFromID(hwndDlg,
                                               ID_OSDI_S506_U_RECOVERY_SLIDER),
                               0, 2,
                               MPFROM2SHORT(5, 10), 4);

            // spin buttons
            apszDMAChannels[0] = "3";
            apszDMAChannels[1] = "5";
            apszDMAChannels[2] = "6";
            apszDMAChannels[3] = "7";
            WinSendDlgItemMsg(hwndDlg, ID_OSDI_S506_A_DMA_SPIN,
                              SPBM_SETARRAY,
                              (MPARAM)&apszDMAChannels,
                              (MPARAM)DMACHANNELS_COUNT);       // array size

            // set adapters/units
            winhSetDlgItemChecked(hwndDlg,
                                  ID_OSDI_S506_ADAPTER0,
                                  TRUE);
            winhSetDlgItemChecked(hwndDlg,
                                  ID_OSDI_S506_UNIT0,
                                  TRUE);

            // "show tooltips" checkbox
            pS506All->fShowTooltips = TRUE;
            winhSetDlgItemChecked(hwndDlg,
                                  DID_SHOWTOOLTIPS,
                                  TRUE);

            // register tooltip class
            ctlRegisterTooltip(WinQueryAnchorBlock(hwndDlg));
            // create tooltip
            pS506All->hwndTooltip = WinCreateWindow(HWND_DESKTOP,  // parent
                                                    WC_CCTL_TOOLTIP, // wnd class
                                                    "",            // window text
                                                    XWP_TOOLTIP_STYLE,
                                                         // tooltip window style (common.h)
                                                    10, 10, 10, 10,    // window pos and size, ignored
                                                    hwndDlg,       // owner window -- important!
                                                    HWND_TOP,      // hwndInsertBehind, ignored
                                                    DID_TOOLTIP, // window ID, optional
                                                    NULL,          // control data
                                                    NULL);         // presparams

            if (pS506All->hwndTooltip)
            {
                // tooltip successfully created:
                // add tools (i.e. controls of the dialog)
                // according to the usToolIDs array
                TOOLINFO    ti = {0};
                HWND        hwndCtl;
                ti.ulFlags = TTF_CENTER_X_ON_TOOL | TTF_POS_Y_BELOW_TOOL | TTF_SUBCLASS;
                ti.hwndToolOwner = hwndDlg;
                ti.pszText = PSZ_TEXTCALLBACK;  // send TTN_NEEDTEXT
                for (ul = 0;
                     ul < (sizeof(usS506ToolIDs) / sizeof(usS506ToolIDs[0]));
                     ul++)
                {
                    hwndCtl = WinWindowFromID(hwndDlg, usS506ToolIDs[ul]);
                    if (hwndCtl)
                    {
                        // add tool to tooltip control
                        ti.hwndTool = hwndCtl;
                        WinSendMsg(pS506All->hwndTooltip,
                                   TTM_ADDTOOL,
                                   (MPARAM)0,
                                   &ti);
                    }
                }

                // set timers
                WinSendMsg(pS506All->hwndTooltip,
                           TTM_SETDELAYTIME,
                           (MPARAM)TTDT_AUTOPOP,
                           (MPARAM)(40*1000));        // 40 secs for autopop (hide)
            }

            // update dialog
            WinPostMsg(hwndDlg, XM_SETTINGS2DLG, 0, 0);
        }
        break;

        /*
         * XM_SETTINGS2DLG:
         *      this user msg (common.h) gets posted when
         *      the dialog controls need to be set according
         *      to the current settings.
         */

        case XM_SETTINGS2DLG:
            S506Settings2Dlg(hwndDlg, pddd);
        break;

        /*
         * XM_ENABLEITEMS:
         *      this user msg (common.h) gets posted when
         *      the dialog controls need to be enabled/disabled
         *      according to the current settings.
         */

        case XM_ENABLEITEMS:
            S506EnableItems(hwndDlg, pddd);
        break;

        /*
         * XM_DLG2SETTINGS:
         *      this user msg (common.h) gets posted when
         *      the settings need to be re-read from the
         *      dialog controls.
         */

        case XM_DLG2SETTINGS:
        {
            PSZ pszNewParams = RecomposeParamsString(hwndDlg,
                                                     (PS506ALL)pddd->pvUser);
            WinSetDlgItemText(hwndDlg, ID_OSDI_S506_NEWPARAMS, pszNewParams);
            if (pszNewParams)       // V0.9.6 (2000-11-12) [umoeller]
                free(pszNewParams);
        }
        break;

        /*
         * WM_CONTROL:
         *
         */

        case WM_CONTROL:
        {
            USHORT  usItemID = SHORT1FROMMP(mp1),
                    usNotifyCode = SHORT2FROMMP(mp1);
            PS506ALL        pS506All = (PS506ALL)pddd->pvUser;
            // find current adapter pointer
            PS506ADAPTER    pS506AdapterThis = &pS506All->Adapters[pS506All->bCurrentAdapter];
            // find current unit pS506All->bCurrentUnit
            PS506UNIT       pS506UnitThis = &pS506All->Units[pS506All->bCurrentUnit];

            switch (usItemID)
            {
                /*
                 * ID_OSDI_S506_TOOLINFO:
                 *      "toolinfo" control (comctl.c)
                 */

                case DID_TOOLTIP:

                    /*
                     * TTN_NEEDTEXT:
                     *      this gets sent from the tooltip
                     *      control just before a tool help
                     *      is to be displayed. We retrieve
                     *      a help text for the control from
                     *      the TMF file and pass it to the
                     *      tooltip for display.
                     */

                    if (usNotifyCode == TTN_NEEDTEXT)
                    {
                        PTOOLTIPTEXT pttt = (PTOOLTIPTEXT)mp2;
                        CHAR    szMessageID[200];
                        XSTRING strHelpString;

                        // get control ID; we need that for the TMF msg ID
                        ULONG   ulID = WinQueryWindowUShort(pttt->hwndTool,
                                                            QWS_ID);
                        ULONG   ulWritten = 0;
                        APIRET  arc = 0;

                        // if not first query: free old text
                        xstrClear(&pS506All->strTooltipString);

                        // compose TMF msg ID
                        sprintf(szMessageID,
                                "S506DLG_%04d",
                                ulID);
                        xstrInit(&strHelpString, 0);
                        cmnGetMessageExt(NULL,                   // pTable
                                         0,                      // cTable
                                         &strHelpString,
                                         szMessageID);

                        // put dlg item text before that string
                        xstrset(&pS506All->strTooltipString,
                                winhQueryDlgItemText(hwndDlg, ulID));
                        if (pS506All->strTooltipString.ulLength)
                        {
                            ULONG ulLength = pS506All->strTooltipString.ulLength;
                            if (pS506All->strTooltipString.psz[ulLength - 1]
                                        != ':')
                                xstrcat(&pS506All->strTooltipString, ":\n", 0);
                            else
                                // last char is already ":"
                                xstrcat(&pS506All->strTooltipString, "\n", 0);
                        }
                        xstrcats(&pS506All->strTooltipString, &strHelpString);

                        pttt->ulFormat = TTFMT_PSZ;
                        pttt->pszText = pS506All->strTooltipString.psz;

                        xstrClear(&strHelpString);
                    }
                break;

                // adapter radio buttons
                case ID_OSDI_S506_ADAPTER0:
                case ID_OSDI_S506_ADAPTER1:
                    if (    (usNotifyCode == BN_CLICKED)
                         || (usNotifyCode == BN_DBLCLICKED)
                       )
                    {
                        // adapter selected:
                        if (usItemID == ID_OSDI_S506_ADAPTER0)
                            pS506All->bCurrentAdapter = 0;
                        else
                            pS506All->bCurrentAdapter = 1;

                        // update controls for that adapter
                        WinPostMsg(hwndDlg, XM_SETTINGS2DLG, 0, 0);
                    }
                break;

                // unit radio buttons
                case ID_OSDI_S506_UNIT0:
                case ID_OSDI_S506_UNIT1:
                case ID_OSDI_S506_UNIT2:
                case ID_OSDI_S506_UNIT3:
                    if (    (usNotifyCode == BN_CLICKED)
                         || (usNotifyCode == BN_DBLCLICKED)
                       )
                    {
                        // unit selected:
                        switch (usItemID)
                        {
                            case ID_OSDI_S506_UNIT0:
                                pS506All->bCurrentUnit = 0; break;
                            case ID_OSDI_S506_UNIT1:
                                pS506All->bCurrentUnit = 1; break;
                            case ID_OSDI_S506_UNIT2:
                                pS506All->bCurrentUnit = 2; break;
                            case ID_OSDI_S506_UNIT3:
                                pS506All->bCurrentUnit = 3; break;
                        }

                        WinPostMsg(hwndDlg, XM_SETTINGS2DLG, 0, 0);
                    }
                break;

                // adapter/unit spin buttons
                case ID_OSDI_S506_A_DMA_SPIN:
                case ID_OSDI_DANIS506_U_TIMEOUT_SPIN:
                case ID_OSDI_DANIS506_U_RATE_UDMA_SPIN:
                case ID_OSDI_DANIS506_U_RATE_MWDMA_SPIN:
                case ID_OSDI_DANIS506_U_RATE_PIO_SPIN:
                    if (    (usNotifyCode == SPBN_UPARROW)
                         || (usNotifyCode == SPBN_DOWNARROW)
                         || (usNotifyCode == SPBN_CHANGE)   // manual input
                       )
                    {
                        LONG lIndex = 0;
                        WinSendDlgItemMsg(hwndDlg, usItemID,
                                          SPBM_QUERYVALUE,
                                          (MPARAM)&lIndex,
                                          MPFROM2SHORT(0,  // return index
                                                       SPBQ_ALWAYSUPDATE));

                        switch (usItemID)
                        {
                            case ID_OSDI_S506_A_DMA_SPIN:
                                // DMA spin button: has values array
                                pS506AdapterThis->ulDMAChannel = strtoul(apszDMAChannels[lIndex],
                                                                         0, 0);
                            break;

                            case ID_OSDI_DANIS506_U_TIMEOUT_SPIN:
                                pS506UnitThis->ulTimeout = lIndex;
                            break;

                            case ID_OSDI_DANIS506_U_RATE_UDMA_SPIN:
                                pS506UnitThis->bUDMA = (BYTE)lIndex;
                            break;

                            case ID_OSDI_DANIS506_U_RATE_MWDMA_SPIN:
                                pS506UnitThis->bMWDMA = (BYTE)lIndex;
                            break;

                            case ID_OSDI_DANIS506_U_RATE_PIO_SPIN:
                                pS506UnitThis->bPIO = (BYTE)lIndex;
                            break;
                        }
                        // update params string
                        WinPostMsg(hwndDlg, XM_DLG2SETTINGS, 0, 0);
                    }
                break;

                // global checkboxes/radio buttons
                case ID_OSDI_S506_INITQUIET:
                case ID_OSDI_S506_INITVERBOSE:
                case ID_OSDI_S506_INITWAIT:
                case ID_OSDI_S506_DSG:
                case ID_OSDI_DANIS506_CLOCK_CHECK:
                case ID_OSDI_DANIS506_GBM:
                case ID_OSDI_DANIS506_FORCEGBM:
                case ID_OSDI_DANIS506_MGAFIX:

                // adapter checkboxes
                case ID_OSDI_S506_A_IGNORE:
                case ID_OSDI_S506_A_RESET:
                case ID_OSDI_S506_A_BASEADDR_CHECK:
                case ID_OSDI_S506_A_IRQ_CHECK:
                case ID_OSDI_S506_A_DMA_CHECK:
                case ID_OSDI_S506_A_DSGADDR_CHECK:
                case ID_OSDI_S506_A_BUSMASTER:

                // unit checkboxes
                case ID_OSDI_S506_U_RECOVERY_CHECK:
                case ID_OSDI_S506_U_GEO_CHECK:
                case ID_OSDI_S506_U_SMS:
                case ID_OSDI_S506_U_LBA:
                case ID_OSDI_S506_U_DASD:
                case ID_OSDI_S506_U_FORCE:
                case ID_OSDI_S506_U_ATAPI:
                case ID_OSDI_S506_U_BUSMASTER:
                case ID_OSDI_DANIS506_U_TIMEOUT_CHECK:
                case ID_OSDI_DANIS506_U_RATE_CHECK:
                case ID_OSDI_DANIS506_U_REMOVEABLE:
                    if (    (usNotifyCode == BN_CLICKED)
                         || (usNotifyCode == BN_DBLCLICKED)
                       )
                    {
                        ULONG ulValue = winhIsDlgItemChecked(hwndDlg,
                                                             usItemID);
                        switch (usItemID)
                        {
                            case ID_OSDI_S506_INITQUIET:
                                pS506All->bInit = 0;
                            break;

                            case ID_OSDI_S506_INITVERBOSE:
                                pS506All->bInit = 1;
                            break;

                            case ID_OSDI_S506_INITWAIT:
                                pS506All->bInit = 2;
                            break;

                            case ID_OSDI_S506_DSG:
                                pS506All->b3ScatterGather = ulValue;
                            break;

                            case ID_OSDI_DANIS506_CLOCK_CHECK:
                                pS506All->fPCIClock = ulValue;
                            break;

                            case ID_OSDI_DANIS506_GBM:
                                pS506All->fGBM = ulValue;
                            break;

                            case ID_OSDI_DANIS506_FORCEGBM:
                                pS506All->fForceGBM = ulValue;
                            break;

                            case ID_OSDI_DANIS506_MGAFIX:
                                pS506All->fMGAFix = ulValue;
                            break;

                            case ID_OSDI_S506_A_IGNORE:
                                pS506AdapterThis->fIgnore = ulValue;
                            break;

                            case ID_OSDI_S506_A_RESET:
                                pS506AdapterThis->b3AllowReset = ulValue;
                            break;

                            case ID_OSDI_S506_A_BASEADDR_CHECK:
                                pS506AdapterThis->fBaseAddress = ulValue;
                            break;

                            case ID_OSDI_S506_A_IRQ_CHECK:
                                pS506AdapterThis->fIRQ = ulValue;
                            break;

                            case ID_OSDI_S506_A_DMA_CHECK:
                                pS506AdapterThis->fDMAChannel = ulValue;
                            break;

                            case ID_OSDI_S506_A_DSGADDR_CHECK:
                                pS506AdapterThis->fDMAScatterGatherAddr = ulValue;
                            break;

                            case ID_OSDI_S506_A_BUSMASTER:
                                pS506AdapterThis->b3BusMaster = ulValue;
                            break;

                            case ID_OSDI_S506_U_RECOVERY_CHECK:
                                pS506UnitThis->fRecoveryTime = ulValue;
                            break;

                            case ID_OSDI_S506_U_GEO_CHECK:
                                pS506UnitThis->fGeometry = ulValue;
                            break;

                            case ID_OSDI_S506_U_SMS:
                                pS506UnitThis->b3SMS = ulValue;
                            break;

                            case ID_OSDI_S506_U_LBA:
                                pS506UnitThis->fLBA = ulValue;
                            break;

                            case ID_OSDI_S506_U_DASD:
                                pS506UnitThis->b3DASD = ulValue;
                            break;

                            case ID_OSDI_S506_U_FORCE:
                                pS506UnitThis->fForce = ulValue;
                            break;

                            case ID_OSDI_S506_U_ATAPI:
                                pS506UnitThis->b3ATAPI = ulValue;
                            break;

                            case ID_OSDI_S506_U_BUSMASTER:
                                pS506UnitThis->b3BusMaster = ulValue;
                            break;

                            case ID_OSDI_DANIS506_U_TIMEOUT_CHECK:
                                pS506UnitThis->fTimeout = ulValue;
                            break;

                            case ID_OSDI_DANIS506_U_RATE_CHECK:
                                pS506UnitThis->fLimitRate = ulValue;
                            break;

                            case ID_OSDI_DANIS506_U_REMOVEABLE:
                                pS506UnitThis->b3Removeable = ulValue;
                            break;
                        }

                        WinPostMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);
                        // update params string
                        WinPostMsg(hwndDlg, XM_DLG2SETTINGS, 0, 0);
                    }
                break;

                // adapter/unit entry fields
                case ID_OSDI_S506_A_BASEADDR_ENTRY:
                case ID_OSDI_S506_A_DSGADDR_ENTRY:
                case ID_OSDI_S506_U_GEO_ENTRY:
                    if (usNotifyCode == EN_CHANGE)
                    {
                        CHAR szText[1000];
                        WinQueryDlgItemText(hwndDlg, usItemID, sizeof(szText), szText);
                        if (usItemID == ID_OSDI_S506_U_GEO_ENTRY)
                            strlcpy(pS506UnitThis->szGeometry,
                                    szText,
                                    sizeof(pS506UnitThis->szGeometry));
                        else
                        {
                            ULONG ulAddr = strtoul(szText, NULL,
                                                   16);  // hex
                            if (usItemID == ID_OSDI_S506_A_BASEADDR_ENTRY)
                                pS506AdapterThis->ulBaseAddress = ulAddr;
                            else
                                pS506AdapterThis->ulDMAScatterGatherAddr = ulAddr;
                        }

                        // update params string
                        WinPostMsg(hwndDlg, XM_DLG2SETTINGS, 0, 0);
                    }
                break;

                // global/adapter/unit sliders: update text next to them
                case ID_OSDI_DANIS506_CLOCK_SLIDER:
                case ID_OSDI_S506_A_IRQ_SLIDER:
                case ID_OSDI_S506_U_RECOVERY_SLIDER:
                    if (    (usNotifyCode == SLN_CHANGE)
                         || (usNotifyCode == SLN_SLIDERTRACK)
                       )
                    {
                        CHAR szTemp[20];
                        PCSZ pcszTemp;
                        LONG lIndex = winhQuerySliderArmPosition(WinWindowFromID(hwndDlg,
                                                                                 usItemID),
                                                                 SMA_INCREMENTVALUE);
                        switch (usItemID)
                        {
                            case ID_OSDI_DANIS506_CLOCK_SLIDER:
                                switch (lIndex)
                                {
                                    case 0: pcszTemp = "25 MHz"; break;
                                    case 1: pcszTemp = "33 MHz"; break;
                                    case 2: pcszTemp = "37 MHz"; break;
                                    default: pcszTemp = "41 MHz"; break;
                                }
                                WinSetDlgItemText(hwndDlg,
                                                  ID_OSDI_DANIS506_CLOCK_TXT,
                                                  (PSZ)pcszTemp);
                                pS506All->ulPCIClock = lIndex;
                            break;

                            case ID_OSDI_S506_A_IRQ_SLIDER:
                                sprintf(szTemp, "%d", lIndex);
                                WinSetDlgItemText(hwndDlg,
                                                  ID_OSDI_S506_A_IRQ_TXT,
                                                  szTemp);
                                pS506AdapterThis->ulIRQ = lIndex;
                            break;

                            case ID_OSDI_S506_U_RECOVERY_SLIDER:
                                sprintf(szTemp, "%d", lIndex + 5);
                                WinSetDlgItemText(hwndDlg,
                                                  ID_OSDI_S506_U_RECOVERY_TXT,
                                                  szTemp);
                                pS506UnitThis->ulRecoveryTime = lIndex + 5;
                            break;
                        }

                        // update params string
                        WinPostMsg(hwndDlg, XM_DLG2SETTINGS, 0, 0);
                    }
                break;

                // "show tooltips" checkbox
                case DID_SHOWTOOLTIPS:
                    pS506All->fShowTooltips = winhIsDlgItemChecked(hwndDlg, usItemID);
                    WinSendMsg(pS506All->hwndTooltip,
                               TTM_ACTIVATE,
                               (MPARAM)pS506All->fShowTooltips,
                               (MPARAM)0);
                break;
            }
        }
        break;

        case WM_COMMAND:
            switch ((USHORT)mp1)
            {
                case DID_OK:
                {
                    // update caller's parameters string buffer
                    PSZ pszNewParams = RecomposeParamsString(hwndDlg,
                                                             (PS506ALL)pddd->pvUser);
                    WinSetDlgItemText(hwndDlg, ID_OSDI_S506_NEWPARAMS, pszNewParams);
                    if (pszNewParams)       // can be NULL V0.9.3 (2000-04-10) [umoeller]
                    {
                        strlcpy(pddd->szParams,
                                pszNewParams,
                                sizeof(pddd->szParams));
                        free(pszNewParams);
                    }
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                }
                break;

                case DID_DEFAULT:
                {
                    // reset all values
                    PS506ALL        pS506All = (PS506ALL)pddd->pvUser;
                    SetS506Defaults(pS506All);
                    WinPostMsg(hwndDlg, XM_SETTINGS2DLG, 0, 0);
                }
                break;

                default:
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            }
        break;

        case WM_HELP:
            cmnDisplayHelp(NULL,        // active Desktop
                           ID_XSH_DRIVER_S506);
        break;

        case WM_DESTROY:
        {
            PS506ALL        pS506All = (PS506ALL)pddd->pvUser;
            xstrClear(&pS506All->strTooltipString);
            WinDestroyWindow(pS506All->hwndTooltip);
            free(pS506All);
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        }
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return mrc;
}
