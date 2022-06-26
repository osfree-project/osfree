
/*
 *@@sourcefile d_cdfs.c:
 *      XWorkplace driver plugin for the CDFS.IFS
 *      driver configuration dialog.
 *
 *      See CheckDriverName() for how driver plugin
 *      DLLs work.
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

/*
 *      Copyright (C) 1999-2003 Ulrich M”ller.
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
#define INCL_DOSMODULEMGR
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINDIALOGS
#define INCL_WININPUT
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINLISTBOXES
#define INCL_WINENTRYFIELDS
#define INCL_WINSTDSLIDER
#define INCL_WINSTDSPIN

#include <os2.h>

// C library headers
#include <stdio.h>

// generic headers
#define DONT_REPLACE_MALLOC             // in case mem debug is enabled
#define DONT_REPLACE_FOR_DBCS           // do not replace strchr with DBCS version
#include "setup.h"                      // code generation and debugging options

// disable wrappers, because we're not linking statically
#ifdef DOSH_STANDARDWRAPPERS
    #undef DOSH_STANDARDWRAPPERS
#endif
#ifdef WINH_STANDARDWRAPPERS
    #undef WINH_STANDARDWRAPPERS
#endif

// headers in /helpers
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\winh.h"               // PM helper routines
#include "helpers\standards.h"          // some standard macros

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs

#include "config\drivdlgs.h"            // driver configuration dialogs

#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Function imports from XFLDR.DLL
 *
 ********************************************************************/

PCMNGETSTRING pcmnGetString = NULL;
PCMNSETCONTROLSFONT pcmnSetControlsFont = NULL;
PCMNQUERYDEFAULTFONT pcmnQueryDefaultFont = NULL;
PCMNQUERYNLSMODULEHANDLE pcmnQueryNLSModuleHandle = NULL;

PDLGHCREATEDLG pdlghCreateDlg = NULL;

PDRVDISPLAYHELP pdrvDisplayHelp = NULL;
PDRV_MEMICMP pdrv_memicmp = NULL;
PDRV_SPRINTF pdrv_sprintf = NULL;
PDRV_STRTOK pdrv_strtok = NULL;

PWINHADJUSTDLGITEMSPINDATA pwinhAdjustDlgItemSpinData = NULL;
PWINHCENTERWINDOW pwinhCenterWindow = NULL;
PWINHSETDLGITEMSPINDATA pwinhSetDlgItemSpinData = NULL;
PWINHSETWINDOWFONT pwinhSetWindowFont = NULL;
PWINHSETSLIDERTICKS pwinhSetSliderTicks = NULL;

static const RESOLVEFUNCTION G_aImports[] =
    {
        "cmnGetString", (PFN*)&pcmnGetString,
        "cmnSetControlsFont", (PFN*)&pcmnSetControlsFont,
        "cmnQueryDefaultFont", (PFN*)&pcmnQueryDefaultFont,
        "cmnQueryNLSModuleHandle", (PFN*)&pcmnQueryNLSModuleHandle,
        "dlghCreateDlg", (PFN*)&pdlghCreateDlg,
        "drvDisplayHelp", (PFN*)&pdrvDisplayHelp,
        "drv_memicmp", (PFN*)&pdrv_memicmp,
        "drv_sprintf", (PFN*)&pdrv_sprintf,
        "drv_strtok", (PFN*)&pdrv_strtok,
        "winhAdjustDlgItemSpinData", (PFN*)&pwinhAdjustDlgItemSpinData,
        "winhCenterWindow", (PFN*)&pwinhCenterWindow,
        "winhSetDlgItemSpinData", (PFN*)&pwinhSetDlgItemSpinData,
        "winhSetWindowFont", (PFN*)&pwinhSetWindowFont,
        "winhSetSliderTicks", (PFN*)&pwinhSetSliderTicks,
    };

/* ******************************************************************
 *
 *   CDFS.IFS driver dialog
 *
 ********************************************************************/

/*
 *@@ GetDefaultJFSCacheSize:
 *      returns the default JFS cache size in KB,
 *      which is 12.5 % of physical RAM.
 */

LONG GetDefaultJFSCacheSize(VOID)
{
    ULONG   ulTotPhysMem;
    DosQuerySysInfo(QSV_TOTPHYSMEM,     // changed V0.9.7 (2001-01-17) [umoeller]
                    QSV_TOTPHYSMEM,
                    &ulTotPhysMem,              // in bytes
                    sizeof(ulTotPhysMem));
    return (ulTotPhysMem / 1024 * 125 / 1000);      // 12.5 % in KB
}

/*
 *@@ SetLazyWrites:
 *
 */

VOID SetLazyWrites(HWND hwndDlg,
                   LONG lSynctime,
                   LONG lMaxage,
                   LONG lBufferidle)
{
    pwinhSetDlgItemSpinData(hwndDlg,       // in: dlg window
                            ID_OSDI_CACHE_SYNCHTIME,
                            1,
                            1000,
                            lSynctime);
    pwinhSetDlgItemSpinData(hwndDlg,       // in: dlg window
                            ID_OSDI_CACHE_MAXAGE,
                            1,
                            1000,
                            lMaxage);
    pwinhSetDlgItemSpinData(hwndDlg,       // in: dlg window
                            ID_OSDI_CACHE_BUFFERIDLE,
                            1,
                            1000,
                            lBufferidle);
}

/*
 *@@ drv_fnwpConfigJFS:
 *      dialog procedure for the "Configure JFS.IFS" dialog.
 *
 *      This gets called automatically from the "Drivers"
 *      page in the "OS/2 Kernel" settings notebook when the
 *      "Configure" button is pressed for this driver.
 *
 *      As with all driver dialogs, this gets a DRIVERDLGDATA
 *      structure with mp2 in WM_INITDLG.
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

MRESULT EXPENTRY fnwpConfigJFS(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PDRIVERDLGDATA pddd = WinQueryWindowPtr(hwndDlg, QWL_USER);
    MRESULT mrc = 0;

    LONG    lSynctime = 64,
                    // "inside wseb" says default is 32, but
                    // wseb fp2 readme says 64 and openjfs sources
                    // have 64 also, so we assume 64
            lMaxage = lSynctime * 4,
            lBufferidle = lSynctime / 8;

    switch (msg)
    {
        case WM_INITDLG:
        {
            PSZ     pszParamsCopy = 0,
                    pszToken = 0,
                    p;
            // defaults
            BOOL    fLazyWrite = TRUE;
            LONG    lCacheSizeKB;       // default: 12.5% of physical memory

            HWND    hwndCacheSlider = WinWindowFromID(hwndDlg, ID_OSDI_CDFS_CACHESLIDER);

            // store DRIVERDLGDATA in window words
            pddd = (PDRIVERDLGDATA)mp2;
            WinSetWindowPtr(hwndDlg, QWL_USER, pddd);
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);

            lCacheSizeKB = GetDefaultJFSCacheSize();

            // now parse the parameters
            if (pszParamsCopy = strdup(pddd->szParams))
            {
                pszToken = pdrv_strtok(pszParamsCopy, " ");
                if (pszToken) // V0.9.4 (2000-06-11) [umoeller]
                    do
                    {
                        // lazywrite
                        if (!pdrv_memicmp(pszToken, "/L", 2))
                        {
                            if (p = strchr(pszToken, ':'))
                            {
                                if (!pdrv_memicmp(p + 1, "OFF", 3))
                                    fLazyWrite = FALSE;
                                else
                                {
                                    // /L:sync,maxage,bufferidle
                                    // we could have used sscanf, but that
                                    // blows up the DLL by 10 KB, which we
                                    // can save here...
                                    PSZ p2,
                                        p3;
                                    if (p2 = strchr(p + 1, ','))
                                    {
                                        CHAR c = *p2;
                                        *p2 = 0;
                                        lSynctime = atoi(p + 1);
                                        *p2 = c;

                                        if (p3 = strchr(p2 + 1, ','))
                                        {
                                            c = *p3;
                                            *p3 = 0;
                                            lMaxage = atoi(p2 + 1);
                                            *p3 = c;

                                            lBufferidle = atoi(p3 + 1);
                                        }
                                    }
                                }
                            }
                        }

                        // autocheck
                        else if (!pdrv_memicmp(pszToken, "/A", 2))
                        {
                            if (p = strchr(pszToken, ':'))
                            {
                                WinSetDlgItemText(hwndDlg, ID_OSDI_AUTOCHECK,
                                                  p + 1);
                            }
                        }

                        // cache
                        else if (!pdrv_memicmp(pszToken, "/C", 2))
                        {
                            if (p = strchr(pszToken, ':'))
                            {
                                lCacheSizeKB = atoi(p + 1);
                            }
                        }
                    } while (pszToken = pdrv_strtok(NULL, " "));

                // now go set controls (except autocheck, that's been done above)

                winhSetSliderArmPosition(hwndCacheSlider,
                                         SMA_INCREMENTVALUE,
                                         // we have one snap every 512 KB,
                                         // so round this
                                         lCacheSizeKB / 512);

                winhSetDlgItemChecked(hwndDlg, ID_OSDI_CACHE_LAZYWRITE, fLazyWrite);

                SetLazyWrites(hwndDlg,
                              lSynctime,
                              lMaxage,
                              lBufferidle);

                WinPostMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);

                free(pszParamsCopy);
            }
        }
        break;

        case XM_ENABLEITEMS:
        {
            BOOL fEnable = winhIsDlgItemChecked(hwndDlg, ID_OSDI_CACHE_LAZYWRITE);

            WinEnableControl(hwndDlg, ID_OSDI_CACHE_SYNCHTIME, fEnable);
            WinEnableControl(hwndDlg, ID_OSDI_CACHE_MAXAGE, fEnable);
            WinEnableControl(hwndDlg, ID_OSDI_CACHE_BUFFERIDLE, fEnable);
        }
        break;

        case WM_CONTROL:
        {
            USHORT  usItemID = SHORT1FROMMP(mp1),
                    usNotifyCode = SHORT2FROMMP(mp1);
            switch (usItemID)
            {
                case ID_OSDI_CDFS_CACHESLIDER:
                    if (    (usNotifyCode == SLN_CHANGE)
                         || (usNotifyCode == SLN_SLIDERTRACK)
                       )
                    {
                        HWND hwndSlider = WinWindowFromID(hwndDlg,
                                                          usItemID);
                        LONG lSliderIndex
                            = winhQuerySliderArmPosition(hwndSlider,
                                                         SMA_INCREMENTVALUE);
                        // cache slider: update text on the right
                        CHAR szTemp[100];
                        pdrv_sprintf(szTemp, "%d KB", lSliderIndex * 512);

                        WinSetDlgItemText(hwndDlg, ID_OSDI_CDFS_CACHETXT, szTemp);
                    }
                break;

                case ID_OSDI_CACHE_LAZYWRITE:
                    WinPostMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);
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
                    CHAR szAutocheck[100];
                    LONG lSliderIndex;
                    pddd->szParams[0] = 0;

                    if (WinQueryDlgItemText(hwndDlg,
                                            ID_OSDI_AUTOCHECK,
                                            sizeof(szAutocheck),
                                            szAutocheck))
                    {
                        pdrv_sprintf(pddd->szParams,
                                     "/AUTOCHECK:%s ",
                                     szAutocheck);
                    }

                    lSliderIndex = winhQuerySliderArmPosition(WinWindowFromID(hwndDlg,
                                                                              ID_OSDI_CDFS_CACHESLIDER),
                                                              SMA_INCREMENTVALUE);

                    if (lSliderIndex != GetDefaultJFSCacheSize() / 512)
                    {
                        pdrv_sprintf(pddd->szParams + strlen(pddd->szParams),
                                     "/CACHE:%d ",
                                     lSliderIndex * 512);
                    }

                    if (winhIsDlgItemChecked(hwndDlg, ID_OSDI_CACHE_LAZYWRITE))
                    {
                        lSynctime = pwinhAdjustDlgItemSpinData(hwndDlg,
                                                               ID_OSDI_CACHE_SYNCHTIME,
                                                               0,
                                                               0);
                        lMaxage = pwinhAdjustDlgItemSpinData(hwndDlg,
                                                             ID_OSDI_CACHE_MAXAGE,
                                                             0,
                                                             0);
                        lBufferidle = pwinhAdjustDlgItemSpinData(hwndDlg,
                                                                 ID_OSDI_CACHE_BUFFERIDLE,
                                                                 0,
                                                                 0);

                        if (    (lSynctime != 64)
                             || (lMaxage != 256)
                             || (lBufferidle != 8)
                           )
                        {
                            // something non-default:
                            pdrv_sprintf(pddd->szParams + strlen(pddd->szParams),
                                         "/L:%u,%u,%u ",
                                         lSynctime,
                                         lMaxage,
                                         lBufferidle);
                        }
                    }
                    else
                        strcat(pddd->szParams, "/L:OFF ");

                    // dismiss
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                }
                break;

                case DID_DEFAULT:
                {
                    WinSetDlgItemText(hwndDlg, ID_OSDI_AUTOCHECK, "*");
                    winhSetSliderArmPosition(WinWindowFromID(hwndDlg,
                                                             ID_OSDI_CDFS_CACHESLIDER),
                                             SMA_INCREMENTVALUE,
                                             // we have one snap every 512 KB,
                                             // so round this
                                             GetDefaultJFSCacheSize() / 512);
                    winhSetDlgItemChecked(hwndDlg, ID_OSDI_CACHE_LAZYWRITE, TRUE);

                    SetLazyWrites(hwndDlg,
                                  lSynctime,
                                  lMaxage,
                                  lBufferidle);
                }
                break;

                default:
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            }
        break;

        case WM_HELP:
            pdrvDisplayHelp(pddd->pvKernel,
                            NULL,                   // main help file
                            ID_XSH_DRIVER_JFS);
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return mrc;
}

SLDCDATA    SliderCData = {
                        sizeof(SLDCDATA),
        // usScale1Increments:
                        33,          // scale 1 increments
                        0,         // scale 1 spacing
                        1,          // scale 2 increments
                        0           // scale 2 spacing
                };

static CONTROLDEF
            Spacer = CONTROLDEF_SPACING(
                        15,
                        5),
            SecondsTxt = CONTROLDEF_TEXT(
                        "s",
                        -1,
                        SZL_AUTOSIZE,
                        SZL_AUTOSIZE),
    // cache size
            CachesizeTxt = CONTROLDEF_TEXT(
                        "Cach~e size:",
                        -1,
                        60,
                        SZL_AUTOSIZE),
            CachesizeSlider = CONTROLDEF_SLIDER(
                        ID_OSDI_CDFS_CACHESLIDER,
                        150,
                        STD_BUTTON_HEIGHT,
                        &SliderCData),
            CachesizeTxt2 = CONTROLDEF_TEXT_RIGHT(
                        "x KB",
                        ID_OSDI_CDFS_CACHETXT,
                        40,
                        SZL_AUTOSIZE),
    // autocheck
            AutocheckTxt = CONTROLDEF_TEXT(
                        "~Autocheck:",
                        -1,
                        60,
                        SZL_AUTOSIZE),
            AutocheckEntry = CONTROLDEF_ENTRYFIELD(
                        NULL,
                        ID_OSDI_AUTOCHECK,
                        SZL_REMAINDER,
                        SZL_AUTOSIZE),
    // lazywrite
            LazywriteCheckbox = CONTROLDEF_AUTOCHECKBOX(
                        "~Lazy write",
                        ID_OSDI_CACHE_LAZYWRITE,
                        SZL_AUTOSIZE,
                        SZL_AUTOSIZE),
            LWSyncText = CONTROLDEF_TEXT(
                        "S~ynch time:",
                        -1,
                        SZL_AUTOSIZE,
                        SZL_AUTOSIZE),
            LWSyncSpin = {
                        WC_SPINBUTTON,
                        NULL,
                        WS_VISIBLE | WS_TABSTOP
                            | SPBS_MASTER | SPBS_NUMERICONLY | SPBS_JUSTCENTER,
                        ID_OSDI_CACHE_SYNCHTIME,
                        CTL_COMMON_FONT,
                        { 50, STD_SPIN_HEIGHT },     // size
                        COMMON_SPACING,
                     },
            LWMaxageText = CONTROLDEF_TEXT(
                        "Ma~x age:",
                        -1,
                        SZL_AUTOSIZE,
                        SZL_AUTOSIZE),
            LWMaxageSpin = {
                        WC_SPINBUTTON,
                        NULL,
                        WS_VISIBLE | WS_TABSTOP
                            | SPBS_MASTER | SPBS_NUMERICONLY | SPBS_JUSTCENTER,
                        ID_OSDI_CACHE_MAXAGE,
                        CTL_COMMON_FONT,
                        { 50, STD_SPIN_HEIGHT },     // size
                        COMMON_SPACING,
                     },
            LWBufferidleText = CONTROLDEF_TEXT(
                        "~Buffer idle:",
                        -1,
                        SZL_AUTOSIZE,
                        SZL_AUTOSIZE),
            LWBufferidleSpin = {
                        WC_SPINBUTTON,
                        NULL,
                        WS_VISIBLE | WS_TABSTOP
                            | SPBS_MASTER | SPBS_NUMERICONLY | SPBS_JUSTCENTER,
                        ID_OSDI_CACHE_BUFFERIDLE,
                        CTL_COMMON_FONT,
                        { 50, STD_SPIN_HEIGHT },     // size
                        COMMON_SPACING,
                     },
    // buttons
            OKButton = CONTROLDEF_DEFPUSHBUTTON(
                        NULL,
                        DID_OK,
                        STD_BUTTON_WIDTH,
                        STD_BUTTON_HEIGHT),
            CancelButton = CONTROLDEF_PUSHBUTTON(
                        NULL,
                        DID_CANCEL,
                        STD_BUTTON_WIDTH,
                        STD_BUTTON_HEIGHT),
            DefaultButton = CONTROLDEF_PUSHBUTTON(
                        NULL,
                        DID_DEFAULT,
                        STD_BUTTON_WIDTH,
                        STD_BUTTON_HEIGHT),
            HelpButton = CONTROLDEF_HELPPUSHBUTTON(
                        NULL,
                        DID_HELP,
                        STD_BUTTON_WIDTH, STD_BUTTON_HEIGHT);

static const DLGHITEM dlgCDFS[] =
    {
        START_TABLE,
            START_ROW(ROW_VALIGN_CENTER),
                CONTROL_DEF(&CachesizeTxt),
                CONTROL_DEF(&CachesizeSlider),
                CONTROL_DEF(&CachesizeTxt2),
            START_ROW(ROW_VALIGN_CENTER),
                CONTROL_DEF(&AutocheckTxt),
                CONTROL_DEF(&AutocheckEntry),
            START_ROW(0),
                CONTROL_DEF(&LazywriteCheckbox),
            START_ROW(0),
                START_TABLE_ALIGN,
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&Spacer),
                        CONTROL_DEF(&LWSyncText),
                        CONTROL_DEF(&LWSyncSpin),
                        CONTROL_DEF(&SecondsTxt),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&Spacer),
                        CONTROL_DEF(&LWMaxageText),
                        CONTROL_DEF(&LWMaxageSpin),
                        CONTROL_DEF(&SecondsTxt),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&Spacer),
                        CONTROL_DEF(&LWBufferidleText),
                        CONTROL_DEF(&LWBufferidleSpin),
                        CONTROL_DEF(&SecondsTxt),
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&OKButton),
                CONTROL_DEF(&CancelButton),
                CONTROL_DEF(&DefaultButton),
                CONTROL_DEF(&HelpButton),
        END_TABLE
    };

/*
 *@@ ShowJFSDlg:
 *      callback specified in DRIVERSPEC to show
 *      the JFS dialog for the JFS.IFS driver.
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

BOOL EXPENTRY ShowJFSDlg(HWND hwndOwner,
                         PDRIVERDLGDATA pDlgData)
{
    BOOL brc = FALSE;

    HWND    hwndDlg = NULLHANDLE;
    ULONG   ulTotPhysMem = 0;

    CHAR szTitle[500];
    pdrv_sprintf(szTitle, "%s: %s",
                 pDlgData->pcszKernelTitle,
                 pDlgData->pDriverSpec->pszFilename);

    OKButton.pcszText = pcmnGetString(DID_OK);
    CancelButton.pcszText = pcmnGetString(DID_CANCEL);
    DefaultButton.pcszText = pcmnGetString(DID_DEFAULT);
    HelpButton.pcszText = pcmnGetString(DID_HELP);

    // set the cache slider's scale: one chunk every 512 KB,
    // and set the max to system's available memory / 4 / 512
    DosQuerySysInfo(QSV_TOTPHYSMEM,     // changed V0.9.7 (2001-01-17) [umoeller]
                    QSV_TOTPHYSMEM,
                    &ulTotPhysMem,              // in bytes
                    sizeof(ulTotPhysMem));
    SliderCData.usScale1Increments = ulTotPhysMem / 4 / (512 * 1024);

    if (NO_ERROR == pdlghCreateDlg(&hwndDlg,
                                   hwndOwner,
                                   FCF_FIXED_DLG,
                                   fnwpConfigJFS,
                                   szTitle,
                                   dlgCDFS,      // DLGHITEM array
                                   ARRAYITEMCOUNT(dlgCDFS),
                                   pDlgData,
                                   pcmnQueryDefaultFont()))
    {
        HWND    hwndCacheSlider = WinWindowFromID(hwndDlg, ID_OSDI_CDFS_CACHESLIDER);
        pwinhCenterWindow(hwndDlg);
        pcmnSetControlsFont(hwndDlg, 0, 5000);

        pwinhSetSliderTicks(hwndCacheSlider,
                            (MPARAM)0, 4,
                            (MPARAM)-1, -1);

        if (DID_OK == WinProcessDlg(hwndDlg))
        {
            brc = TRUE;
        }

        WinDestroyWindow(hwndDlg);
    }

    return brc;
}

/* ******************************************************************
 *
 *   CDFS.IFS driver dialog
 *
 ********************************************************************/

/*
 *@@ drv_fnwpConfigCDFS:
 *      dialog procedure for the "Configure CDFS.IFS" dialog.
 *
 *      This gets called automatically from the "Drivers"
 *      page in the "OS/2 Kernel" settings notebook when the
 *      "Configure" button is pressed for this driver.
 *
 *      As with all driver dialogs, this gets a DRIVERDLGDATA
 *      structure with mp2 in WM_INITDLG.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.4 (2000-06-05) [umoeller]: "Optimize" and "Default" didn't work
 *@@changed V0.9.4 (2000-06-05) [umoeller]: crashed if params line was empty
 */

MRESULT EXPENTRY fnwpConfigCDFS(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PDRIVERDLGDATA pddd = WinQueryWindowPtr(hwndDlg, QWL_USER);
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_INITDLG:
        {
            PSZ     pszParamsCopy = 0,
                    pszToken = 0;
            // defaults
            BOOL    fJoliet = FALSE,
                    fKanji = FALSE;
            ULONG   ulInit = 0,             // default
                    ulCacheX64 = 2,
                    ulSectors = 8;

            HWND    hwndCacheSlider = WinWindowFromID(hwndDlg, ID_OSDI_CDFS_CACHESLIDER),
                    hwndSectorsSlider = WinWindowFromID(hwndDlg, ID_OSDI_CDFS_SECTORSSLIDER);

            // store DRIVERDLGDATA in window words
            pddd = (PDRIVERDLGDATA)mp2;
            WinSetWindowPtr(hwndDlg, QWL_USER, pddd);
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);

            // now parse the parameters
            if (pszParamsCopy = strdup(pddd->szParams))
            {
                pszToken = pdrv_strtok(pszParamsCopy, " ");
                if (pszToken) // V0.9.4 (2000-06-11) [umoeller]
                    do {
                        if (pdrv_memicmp(pszToken, "/W", 2) == 0)
                            fJoliet = TRUE;
                        else if (pdrv_memicmp(pszToken, "/K", 2) == 0)
                            fKanji = TRUE;
                        else if (pdrv_memicmp(pszToken, "/Q", 2) == 0)
                            ulInit = 1;         // quiet
                        else if (pdrv_memicmp(pszToken, "/V", 2) == 0)
                            ulInit = 2;         // verbose
                        else if (pdrv_memicmp(pszToken, "/C:", 3) == 0)
                            ulCacheX64 = strtoul(pszToken + 3, NULL, 10);
                        else if (pdrv_memicmp(pszToken, "/M:", 3) == 0)
                            ulSectors = strtoul(pszToken + 3, NULL, 10);
                    } while (pszToken = pdrv_strtok(NULL, " "));

                free(pszParamsCopy);
            }

            // set dialog items
            winhSetDlgItemChecked(hwndDlg, ID_OSDI_CDFS_JOLIET, fJoliet);
            winhSetDlgItemChecked(hwndDlg, ID_OSDI_CDFS_KANJI, fKanji);

            // sliders
            pwinhSetSliderTicks(hwndCacheSlider,
                                (MPARAM)0, 4,
                                (MPARAM)-1, -1);
            winhSetSliderArmPosition(hwndCacheSlider, SMA_INCREMENTVALUE, ulCacheX64);
            pwinhSetSliderTicks(hwndSectorsSlider,
                                (MPARAM)0, 4,
                                (MPARAM)-1, -1);
            winhSetSliderArmPosition(hwndSectorsSlider, SMA_INCREMENTVALUE, ulSectors);

            // initialization
            switch(ulInit)
            {
                case 1:
                    winhSetDlgItemChecked(hwndDlg, ID_OSDI_CDFS_INITQUIET, TRUE); break;
                case 2:
                    winhSetDlgItemChecked(hwndDlg, ID_OSDI_CDFS_INITVERBOSE, TRUE); break;
                default:
                    winhSetDlgItemChecked(hwndDlg, ID_OSDI_CDFS_INITDEFAULT, TRUE); break;
            }
        }
        break;

        case WM_CONTROL:
        {
            USHORT  usItemID = SHORT1FROMMP(mp1),
                    usNotifyCode = SHORT2FROMMP(mp1);
            switch (usItemID)
            {
                // sliders changed:
                case ID_OSDI_CDFS_CACHESLIDER:
                case ID_OSDI_CDFS_SECTORSSLIDER:
                    if (    (usNotifyCode == SLN_CHANGE)
                         || (usNotifyCode == SLN_SLIDERTRACK)
                       )
                    {
                        HWND hwndSlider = WinWindowFromID(hwndDlg,
                                                          usItemID);
                        LONG lSliderIndex
                            = winhQuerySliderArmPosition(hwndSlider,
                                                         SMA_INCREMENTVALUE);
                        if (usItemID == ID_OSDI_CDFS_CACHESLIDER)
                        {
                            // cache slider: update text on the right
                            CHAR szTemp[100];
                            pdrv_sprintf(szTemp, "%d KB", lSliderIndex * 64);
                            WinSetDlgItemText(hwndDlg, ID_OSDI_CDFS_CACHETXT, szTemp);
                        }
                        else
                        {
                            // sectors slider: update text on the right
                            CHAR szTemp[100];
                            pdrv_sprintf(szTemp, "%d", lSliderIndex);
                            WinSetDlgItemText(hwndDlg, ID_OSDI_CDFS_SECTORSTXT, szTemp);
                        }
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
                    LONG lSliderIndex;
                    pddd->szParams[0] = 0;
                    if (winhIsDlgItemChecked(hwndDlg, ID_OSDI_CDFS_JOLIET))
                        strcpy(pddd->szParams, "/W ");
                    if (winhIsDlgItemChecked(hwndDlg, ID_OSDI_CDFS_KANJI))
                        strcat(pddd->szParams, "/K ");
                    lSliderIndex = winhQuerySliderArmPosition(WinWindowFromID(hwndDlg,
                                                                              ID_OSDI_CDFS_CACHESLIDER),
                                                              SMA_INCREMENTVALUE);
                    if (lSliderIndex != 2)
                        // non-default cache:
                        pdrv_sprintf(pddd->szParams + strlen(pddd->szParams),
                                     "/C:%d ", lSliderIndex);
                    lSliderIndex = winhQuerySliderArmPosition(WinWindowFromID(hwndDlg,
                                                                              ID_OSDI_CDFS_SECTORSSLIDER),
                                                              SMA_INCREMENTVALUE);
                    if (lSliderIndex != 8)
                        // non-default sectors:
                        pdrv_sprintf(pddd->szParams + strlen(pddd->szParams),
                                     "/M:%d ", lSliderIndex);
                    if (winhIsDlgItemChecked(hwndDlg, ID_OSDI_CDFS_INITQUIET))
                        strcat(pddd->szParams, "/Q ");
                    else if (winhIsDlgItemChecked(hwndDlg, ID_OSDI_CDFS_INITVERBOSE))
                        strcat(pddd->szParams, "/V ");

                    // dismiss
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                }
                break;

                case DID_OPTIMIZE:
                {
                    HWND    hwndCacheSlider = WinWindowFromID(hwndDlg, ID_OSDI_CDFS_CACHESLIDER),
                            hwndSectorsSlider = WinWindowFromID(hwndDlg, ID_OSDI_CDFS_SECTORSSLIDER);
                    winhSetDlgItemChecked(hwndDlg, ID_OSDI_CDFS_JOLIET, TRUE);
                    winhSetDlgItemChecked(hwndDlg, ID_OSDI_CDFS_KANJI, FALSE);
                    winhSetSliderArmPosition(hwndCacheSlider, SMA_INCREMENTVALUE, 8); // 512 KB
                    winhSetSliderArmPosition(hwndSectorsSlider, SMA_INCREMENTVALUE, 32); // sectors
                    winhSetDlgItemChecked(hwndDlg, ID_OSDI_CDFS_INITDEFAULT, TRUE);
                }
                break;

                case DID_DEFAULT:
                {
                    HWND    hwndCacheSlider = WinWindowFromID(hwndDlg, ID_OSDI_CDFS_CACHESLIDER),
                            hwndSectorsSlider = WinWindowFromID(hwndDlg, ID_OSDI_CDFS_SECTORSSLIDER);
                    winhSetDlgItemChecked(hwndDlg, ID_OSDI_CDFS_JOLIET, FALSE);
                    winhSetDlgItemChecked(hwndDlg, ID_OSDI_CDFS_KANJI, FALSE);
                    winhSetSliderArmPosition(hwndCacheSlider, SMA_INCREMENTVALUE, 2); // 128 KB
                    winhSetSliderArmPosition(hwndSectorsSlider, SMA_INCREMENTVALUE, 8); // sectors
                    winhSetDlgItemChecked(hwndDlg, ID_OSDI_CDFS_INITDEFAULT, TRUE);
                }
                break;

                default:
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            }
        break;

        case WM_HELP:
            pdrvDisplayHelp(pddd->pvKernel,
                            NULL,                   // main help file
                            ID_XSH_DRIVER_CDFS);
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ ShowCDFSDlg:
 *      callback specified in DRIVERSPEC to show
 *      the cdfs dialog for the CDFS.IFS driver.
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

BOOL EXPENTRY ShowCDFSDlg(HWND hwndOwner,
                          PDRIVERDLGDATA pDlgData)
{
    BOOL brc = FALSE;
    HWND hwndDlg;
    if (hwndDlg = WinLoadDlg(HWND_DESKTOP,     // parent
                             hwndOwner,
                             fnwpConfigCDFS,
                                // dlg proc as in DRIVERSPEC
                             pcmnQueryNLSModuleHandle(FALSE),
                                // dlg module as in DRIVERSPEC
                             ID_OSD_DRIVER_CDFS,
                                // resource ID as in DRIVERSPEC
                             pDlgData))
                                // pass DRIVERDLGDATA as create param
    {
        // successfully loaded:
        // set dialog title to driver name
        CHAR szTitle[500];
        pdrv_sprintf(szTitle,
                     "%s: %s",
                     pDlgData->pcszKernelTitle,
                     pDlgData->pDriverSpec->pszFilename);
        WinSetWindowText(hwndDlg, szTitle);
        pwinhCenterWindow(hwndDlg);
        pcmnSetControlsFont(hwndDlg, 0, 5000);

        // go!!
        if (DID_OK == WinProcessDlg(hwndDlg))
            brc = TRUE;

        WinDestroyWindow(hwndDlg);
    }

    return brc;
}

/* ******************************************************************
 *
 *   Exported procedures
 *
 ********************************************************************/

/*
 *@@ CheckDriverName:
 *      the one and only exported function from this
 *      driver plugin DLL.
 *
 *      The way driver plugins work is the following:
 *
 *      1)  When the "Drivers" page in "OS/2 Kernel"
 *          is opened, XWorkplace loads the DLLs
 *          in plugins\drvdlgs.
 *
 *      2)  XWorkplace then attempts to import the
 *          "CheckDriverName" function from each
 *          DLL by name. If this succeeds, the DLL
 *          is stored internally. Note that at
 *          this point, the DLL is not yet called.
 *
 *      3)  XWorkplace then starts a second thread
 *          to populate the drivers container on
 *          the page. For each driver that was
 *          found in CONFIG.SYS, XWorkplace will
 *          then call this CheckDriverName function
 *          of all plugin DLLs until one reports
 *          that it can handle the driver. This will
 *          then enable the "Configure" button.
 *
 *      4)  If the user then presses the "Configure"
 *          button, XWorkplace will display a dialog
 *          from the resources reported from this
 *          function.
 *
 *      <B>The CheckDriverName function</B>
 *
 *      This function receives a DRIVERSPEC structure with
 *      a driver definition. See drivdlgs.h for the
 *      specification. This function must simply check the
 *      pszFilename field for whether it supports this
 *      driver. For example, if pszFilename == "CDFS.IFS",
 *      we assume we can handle this.
 *
 *      ONLY if the driver is supported, this function
 *      must return 1 and set DRIVERSPEC.pfnShowDriverDlg
 *      to a "show dlg" function, which will later be
 *      called (from XWorkplace-internal code) to display
 *      the dialog.
 *
 *      As a result, there are at least two functions
 *      in your plugin DLL: the "CheckDriverName"
 *      function (which is exported) and your
 *      "show dlg" procedure (which might be returned
 *      in DRIVERSPEC).
 *
 *      As said above, when this function gets called,
 *      it is _not_ running on the PM interface
 *      thread, but in a temporary background thread
 *      of XWorkplace. So do not display any silly
 *      dialogs in _this_ function.
 *
 *      This function must return:
 *
 *      --  0: driver not supported. XWorkplace will
 *             then call the next plugin. If no plugin
 *             supports the driver, the "Configure"
 *             button is disabled for that driver.
 *
 *      --  1: driver _is_ supported here. See above.
 *
 *      --  -1: a fatal error occurred, and the plugin
 *             should be unloaded again.
 *
 *      <B>Importing from XFLDR.DLL</B>
 *
 *      As with XCenter plugins, you get the module handle
 *      of XFLDR.DLL with this function. You can use this
 *      for importing helper functions from XFLDR.DLL, if
 *      you want. For speed though, you should _only_ do
 *      this if you are about to return 1 from this function.
 *
 *      <B>The dialog procedure</B>
 *
 *      When the user presses the "Configure" button for
 *      the driver that you have set the DRIVERSPEC for
 *      here, your function will get called. XWorkplace
 *      doesn't care what that function does. It should
 *      however display a dialog.
 *
 *      Your function will receive a DRIVERDLGDATA, which
 *      it should pass to WinLoadDlg so that the dialog
 *      will receive it with mp2 of WM_INITDLG.
 *
 *      The DRIVERDLGDATA contains the parameters string
 *      of the driver from CONFIG.SYS. Your dialog proc
 *      should parse that and set up the dialog's controls
 *      accordingly.
 *
 *      Reversely, if the user presses "OK" later, your
 *      dialog proc should recompose a complete parameters
 *      string and put it back into the DRIVERDLGDATA.
 *      This will then be copied onto the "Drivers" page.
 */

ULONG EXPENTRY CheckDriverName(HMODULE hmodPlugin, // in: module handle of the widget DLL
                               HMODULE hmodXFLDR,  // in: XFLDR.DLL module handle
                               PDRIVERSPEC pSpec,  // in/out: driver specification
                               PSZ pszErrorMsg)    // out: 500 bytes for error msg
{
    BOOL    fOurDriver = FALSE;

    if (!strcmp(pSpec->pszFilename,         // has been uppered by "OS/2" kernel object
                "CDFS.IFS"))
    {
        // tell XWorkplace what we can do
        pSpec->pfnShowDriverDlg = ShowCDFSDlg;
        fOurDriver = TRUE;
    }
    else if (!strcmp(pSpec->pszFilename,         // has been uppered by "OS/2" kernel object
                     "JFS.IFS"))
    {
        // tell XWorkplace what we can do
        pSpec->pfnShowDriverDlg = ShowJFSDlg;
        fOurDriver = TRUE;
    }

    if (fOurDriver)
    {
        // this is our driver:
        ULONG ul;

        // resolve imports from XFLDR.DLL (this is basically
        // a copy of the doshResolveImports code, but we can't
        // use that before resolving...)
        for (ul = 0;
             ul < sizeof(G_aImports) / sizeof(G_aImports[0]); // array item count
             ul++)
        {
            if (DosQueryProcAddr(hmodXFLDR,
                                 0,               // ordinal, ignored
                                 (PSZ)G_aImports[ul].pcszFunctionName,
                                 G_aImports[ul].ppFuncAddress)
                    != NO_ERROR)
            {
                strcpy(pszErrorMsg, "Import ");
                strcat(pszErrorMsg, G_aImports[ul].pcszFunctionName);
                strcat(pszErrorMsg, " failed.");
                // report "error"
                return -1;
            }
        }

        // report "yes, we can do this driver"
        return 1;
    }

    // report "no, we don't know this driver"
    return 0;
}


