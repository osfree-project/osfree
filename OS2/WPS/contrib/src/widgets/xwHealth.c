
/*
 *@@sourcefile xwHealth.c:
 *      widget for interfacing STHEALTH.DLL.
 *
 *      This is all new with V0.9.9.
 *
 *@@added V0.9.9 (2001-02-01) [umoeller]
 *@@header "shared\center.h"
 */

/*
 *      Copyright (C) 2001 Stefan Milcke.
 *      Copyright (C) 2000-2007 Ulrich M”ller.
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
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINDIALOGS
#define INCL_WININPUT
#define INCL_WINPOINTERS
#define INCL_WINPROGRAMLIST
#define INCL_WINSWITCHLIST
#define INCL_WINSYS
#define INCL_WINTIMER
#define INCL_WINMENUS
#define INCL_WINWORKPLACE
#define INCL_WINENTRYFIELDS

#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#define INCL_GPIREGIONS
#define NO_STHEALTH_PRAGMA_LIBRARY
#include "StHealth.h"
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

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
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\stringh.h"            // string helper routines
#include "helpers\timer.h"              // replacement PM timers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\center.h"              // public XCenter interfaces
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs

#include "config\drivdlgs.h"            // driver configuration dialogs

#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Private definitions
 *
 ********************************************************************/

#define MWGT_HEALTHMONITOR       1

VOID EXPENTRY xwhShowSettingsDlg(PWIDGETSETTINGSDLGDATA pData);
MRESULT EXPENTRY fnwpSettingsDlg(HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);

/* ******************************************************************
 *
 *   XCenter widget class definition
 *
 ********************************************************************/

/*
 *      This contains the name of the PM window class and
 *      the XCENTERWIDGETCLASS definition(s) for the widget
 *      class(es) in this DLL.
 */

#define WNDCLASS_WIDGET_XWHEALTH    "XWPHealthMonitorWidget"

static const XCENTERWIDGETCLASS G_WidgetClasses[] =
    {
        {
            WNDCLASS_WIDGET_XWHEALTH,
            MWGT_HEALTHMONITOR,
            "Health Monitor",
            (PCSZ)(XCENTER_STRING_RESOURCE | ID_CRSI_WIDGET_HEALTH),
                                        // widget class name displayed to user
                                        // (NLS DLL) V0.9.19 (2002-05-07) [umoeller]
            WGTF_UNIQUEPERXCENTER,
            xwhShowSettingsDlg
        },
    };

/* ******************************************************************
 *
 *   Function imports from XFLDR.DLL
 *
 ********************************************************************/

/*
 *      To reduce the size of the widget DLL, it is
 *      compiled with the VAC subsystem libraries.
 *      In addition, instead of linking frequently
 *      used helpers against the DLL again, we import
 *      them from XFLDR.DLL, whose module handle is
 *      given to us in the INITMODULE export.
 *
 *      Note that importing functions from XFLDR.DLL
 *      is _not_ a requirement. We only do this to
 *      avoid duplicate code.
 *
 *      For each funtion that you need, add a global
 *      function pointer variable and an entry to
 *      the G_aImports array. These better match.
 */

// resolved function pointers from XFLDR.DLL
PCMNQUERYDEFAULTFONT pcmnQueryDefaultFont = NULL;
PCMNQUERYHELPLIBRARY pcmnQueryHelpLibrary = NULL;
PCMNQUERYMAINRESMODULEHANDLE pcmnQueryMainResModuleHandle = NULL;
PCMNQUERYNLSMODULEHANDLE pcmnQueryNLSModuleHandle = NULL;
PCMNSETCONTROLSFONT pcmnSetControlsFont = NULL;

PCTRDISPLAYHELP pctrDisplayHelp = NULL;
PCTRFREESETUPVALUE pctrFreeSetupValue = NULL;
PCTRPARSECOLORSTRING pctrParseColorString = NULL;
PCTRSCANSETUPSTRING pctrScanSetupString = NULL;

PDRV_SPRINTF pdrv_sprintf = NULL;

PGPIHDRAW3DFRAME pgpihDraw3DFrame = NULL;
PGPIHSWITCHTORGB pgpihSwitchToRGB = NULL;

PNLSTHOUSANDSULONG pnlsThousandsULong = NULL;

PTMRSTARTXTIMER ptmrStartXTimer = NULL;
PTMRSTOPXTIMER ptmrStopXTimer = NULL;

PWINHCENTERWINDOW pwinhCenterWindow = NULL;
PWINHFREE pwinhFree = NULL;
PWINHQUERYPRESCOLOR pwinhQueryPresColor = NULL;
PWINHQUERYWINDOWFONT pwinhQueryWindowFont = NULL;
PWINHSETWINDOWFONT pwinhSetWindowFont = NULL;

PXSTRCAT pxstrcat = NULL;
PXSTRCLEAR pxstrClear = NULL;
PXSTRINIT pxstrInit = NULL;

static const RESOLVEFUNCTION G_aImports[] =
{
        "cmnQueryDefaultFont", (PFN*)&pcmnQueryDefaultFont,
        "cmnQueryHelpLibrary", (PFN*)&pcmnQueryHelpLibrary,
        "cmnQueryMainResModuleHandle", (PFN*)&pcmnQueryMainResModuleHandle,
        "cmnQueryNLSModuleHandle", (PFN*)&pcmnQueryNLSModuleHandle,
        "cmnSetControlsFont", (PFN*)&pcmnSetControlsFont,
        "ctrDisplayHelp", (PFN*)&pctrDisplayHelp,
        "ctrFreeSetupValue", (PFN*)&pctrFreeSetupValue,
        "ctrParseColorString", (PFN*)&pctrParseColorString,
        "ctrScanSetupString", (PFN*)&pctrScanSetupString,
        "drv_sprintf", (PFN*)&pdrv_sprintf,
        "gpihDraw3DFrame", (PFN*)&pgpihDraw3DFrame,
        "gpihSwitchToRGB", (PFN*)&pgpihSwitchToRGB,
        "nlsThousandsULong", (PFN*)&pnlsThousandsULong,
        "tmrStartXTimer", (PFN*)&ptmrStartXTimer,
        "tmrStopXTimer", (PFN*)&ptmrStopXTimer,
        "winhFree", (PFN*)&pwinhFree,
        "winhCenterWindow", (PFN*)&pwinhCenterWindow,
        "winhQueryPresColor", (PFN*)&pwinhQueryPresColor,
        "winhQueryWindowFont", (PFN*)&pwinhQueryWindowFont,
        "winhSetWindowFont", (PFN*)&pwinhSetWindowFont,
        "xstrcat", (PFN*)&pxstrcat,
        "xstrClear", (PFN*)&pxstrClear,
        "xstrInit", (PFN*)&pxstrInit
};

BOOL(*_System sthRegisterDaemon) (BOOL) = NULL;
BOOL(*_System sthUnregisterDaemon) (void) = NULL;
BOOL(*_System sthDetectChip) (void) = NULL;
long (*_System sthFan) (int, BOOL) = NULL;
double (*_System sthTemp) (int, BOOL) = NULL;
double (*_System sthVoltage) (int, BOOL) = NULL;
int (*_System sthFilterEvents) (PULONG, ULONG, PULONG, ULONG, PVOID) = NULL;

static const RESOLVEFUNCTION G_StHealthImports[] =
{
    "StHealthRegisterDaemon", (PFN*)&sthRegisterDaemon,
    "StHealthUnregisterDaemon", (PFN*)&sthUnregisterDaemon,
    "StHealthDetectChip", (PFN*)&sthDetectChip,
    "StHealthFan", (PFN*)&sthFan,
    "StHealthTemp", (PFN*)&sthTemp,
    "StHealthVoltage", (PFN*)&sthVoltage,
    "StHealthFilterEvents", (PFN*)&sthFilterEvents
};

HMODULE hmStHealth = 0;
// HMODULE hmXwHealth = 0;

/* ******************************************************************
 *
 *   Private widget instance data
 *
 ********************************************************************/

/*
 *@@ MONITORSETUP:
 *      instance data to which setup strings correspond.
 *      This is also a member of HEALTHPRIVATE.
 *
 *      Putting these settings into a separate structure
 *      is no requirement technically. However, once the
 *      widget uses a settings dialog, the dialog must
 *      support changing the widget settings even if the
 *      widget doesn't currently exist as a window, so
 *      separating the setup data from the widget window
 *      data will come in handy for managing the setup
 *      strings.
 */

typedef struct _MONITORSETUP
{
    LONG        lcolBackground,         // background color
                lcolForeground;         // foreground color (for text)

    PSZ         pszFont;
            // if != NULL, non-default font (in "8.Helv" format);
            // this has been allocated using local malloc()!

    PSZ         pszViewString;
            // health view string for painting

} MONITORSETUP, *PMONITORSETUP;

/*
 *@@ HEALTHPRIVATE:
 *      more window data for the various monitor widgets.
 *
 *      An instance of this is created on WM_CREATE in
 *      fnwpHealthWidget and stored in XCENTERWIDGET.pUser.
 */

typedef struct _HEALTHPRIVATE
{
    PXCENTERWIDGET pWidget;
            // reverse ptr to general widget data ptr; we need
            // that all the time and don't want to pass it on
            // the stack with each function call

    ULONG           ulType;
                // this is copied from the widget class on WM_CREATE

    ULONG           cxCurrent,
                    cyCurrent;

    MONITORSETUP    Setup;
            // widget settings that correspond to a setup string

    ULONG           ulTimerID;              // if != NULLHANDLE, update timer is running

} HEALTHPRIVATE, *PHEALTHPRIVATE;

/* ******************************************************************
 *
 *   Widget settings dialog
 *
 ********************************************************************/

/*
 *      This section contains shared code to manage the
 *      widget's settings. This can translate a widget
 *      setup string into the fields of a binary setup
 *      structure and vice versa. This code is used by
 *      both an open widget window and a settings dialog.
 */

/*
 *@@ MwgtFreeSetup:
 *      cleans up the data in the specified setup
 *      structure, but does not free the structure
 *      itself.
 */

VOID MwgtFreeSetup(PMONITORSETUP pSetup)
{
    if (pSetup)
    {
        if (pSetup->pszFont)
        {
            free(pSetup->pszFont);
            pSetup->pszFont = NULL;
        }

        if (pSetup->pszViewString) // V0.9.9 (2001-03-07) [umoeller]
        {
            free(pSetup->pszViewString);
            pSetup->pszViewString = NULL;
        }
    }
}

/*
 *@@ MwgtScanSetup:
 *      scans the given setup string and translates
 *      its data into the specified binary setup
 *      structure.
 *
 *      NOTE: It is assumed that pSetup is zeroed
 *      out. We do not clean up previous data here.
 *
 *@@changed V0.9.9 (2001-02-06) [smilcke]: Added setup strings for health monitoring
 */

VOID MwgtScanSetup(PCSZ pcszSetupString,
                   PMONITORSETUP pSetup)
{
    PSZ p;
    // background color
    if (p = pctrScanSetupString(pcszSetupString,
                                "BGNDCOL"))
    {
        pSetup->lcolBackground = pctrParseColorString(p);
        pctrFreeSetupValue(p);
    }
    else
        // default color:
        pSetup->lcolBackground = WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0);

    // text color:
    if (p = pctrScanSetupString(pcszSetupString,
                                "TEXTCOL"))
    {
        pSetup->lcolForeground = pctrParseColorString(p);
        pctrFreeSetupValue(p);
    }
    else
        pSetup->lcolForeground = WinQuerySysColor(HWND_DESKTOP, SYSCLR_WINDOWTEXT, 0); // V1.0.8 (2007-08-05) [pr]

    // font:
    // we set the font presparam, which automatically
    // affects the cached presentation spaces
    if (p = pctrScanSetupString(pcszSetupString,
                                "FONT"))
    {
        pSetup->pszFont = strdup(p);
        pctrFreeSetupValue(p);
    }
    // else: leave this field null

    // Health view string
    if (p = pctrScanSetupString(pcszSetupString,
                                "HEALTHVSTR"))
    {
        pSetup->pszViewString = strdup(p);
        pctrFreeSetupValue(p);
    }
    else
    {
        pSetup->pszViewString = strdup("Temp:%T0/%T1/%T2 øC - Fan:%F0/%F1/%F2 RPM - Volt:%V0/%V1/%V2/%V3/%V4/%V5/%V6");
                // V0.9.9 (2001-03-07) [umoeller]
                // if you use malloc() for the one case, we better
                // use malloc() for the second case also... otherwise
                // we can't use free() on pszViewString
    }
}

/*
 *@@ MwgtSaveSetup:
 *      composes a new setup string.
 *      The caller must invoke xstrClear on the
 *      string after use.
 *
 *@@added V0.9.7 (2000-12-04) [umoeller]
 *@@changed V0.9.9 (2001-02-06) [smilcke]: Added setup strings for health monitoring
 */

VOID MwgtSaveSetup(PXSTRING pstrSetup,  // out: setup string (is cleared first)
                   PMONITORSETUP pSetup)
{
    CHAR szTemp[400];
    // PSZ psz = 0;

    pxstrInit(pstrSetup, 400);
    pdrv_sprintf(szTemp, "BGNDCOL=%06lX;", pSetup->lcolBackground);
    pxstrcat(pstrSetup, szTemp, 0);
    pdrv_sprintf(szTemp, "TEXTCOL=%06lX;", pSetup->lcolForeground);
    pxstrcat(pstrSetup, szTemp, 0);
    if (pSetup->pszFont)
    {
        // non-default font:
        pdrv_sprintf(szTemp, "FONT=%s;", pSetup->pszFont);
        pxstrcat(pstrSetup, szTemp, 0);
    }

    if (pSetup->pszViewString)
    {
        pdrv_sprintf(szTemp,"HEALTHVSTR=%s;", pSetup->pszViewString);
        pxstrcat(pstrSetup, szTemp, 0);
    }
}

/*
 *@@ buildHealthString:
 *      composes a new string with health values.
 *
 *@@added V0.9.9 (2001-02-06) [smilcke]:
 */

void buildHealthString(PSZ szPaint,PSZ szViewString)
{
    if(szPaint && szViewString)
    {
        // CHAR identifier;
        CHAR stringValue[500];
        unsigned int number;
        double t[10];
        int f[10];
        double v[10];
        int i, j;
        szPaint[0]=(char)0;
        for(i=0;i<10;i++)
        {
            t[i]=sthTemp(i,FALSE);
            f[i]=sthFan(i,FALSE);
            v[i]=sthVoltage(i,FALSE);
        }

        j=0;

        for(i = 0;
            i < strlen(szViewString);
            i++)
        {
            if(szViewString[i]=='%')
            {
                number=szViewString[i+2]-'0';
                if(strncmp(&(szViewString[i]),"%TF",3)==0)
                {
                    number=szViewString[i+3]-'0';
                    if(t[number]==STHEALTH_NOT_PRESENT_ERROR)
                        strcat(szPaint,"[ERR]");
                    else
                    {
                        pdrv_sprintf(stringValue,"%.2f",((t[number]*9)/5)+32);
                        strcat(szPaint,stringValue);
                    }
                    i+=3;
                }
                else if(strncmp(&(szViewString[i]),"%T",2)==0)
                {
                    if(t[number]==STHEALTH_NOT_PRESENT_ERROR)
                        strcat(szPaint,"[ERR]");
                    else
                    {
                        pdrv_sprintf(stringValue,"%.2f",t[number]);
                        strcat(szPaint,stringValue);
                    }
                    i+=2;
                }
                else if(strncmp(&(szViewString[i]),"%V",2)==0)
                {
                    if(v[number]==STHEALTH_NOT_PRESENT_ERROR)
                        strcat(szPaint,"[ERR]");
                    else
                    {
                        pdrv_sprintf(stringValue,"%.2f",v[number]);
                        strcat(szPaint,stringValue);
                    }
                    i+=2;
                }
                else if(strncmp(&(szViewString[i]),"%F",2)==0)
                {
                    if(f[number]==STHEALTH_NOT_PRESENT_ERROR)
                        strcat(szPaint,"[ERR]");
                    else
                    {
                        pdrv_sprintf(stringValue,"%.2d",f[number]);
                        strcat(szPaint,stringValue);
                    }
                    i+=2;
                }
            }
            else
            {
                stringValue[0]=szViewString[i];
                stringValue[1]=(char)0;
                strcat(szPaint,stringValue);
            }
        }
    }
}

/* ******************************************************************
 *
 *   Widget settings dialog
 *
 ********************************************************************/

/*
 *@@ xwhShowSettingsDlg:
 *
 */

VOID EXPENTRY xwhShowSettingsDlg(PWIDGETSETTINGSDLGDATA pData)
{
    HWND hwnd=WinLoadDlg(HWND_DESKTOP,
                         pData->hwndOwner,
                         fnwpSettingsDlg,
                         pcmnQueryNLSModuleHandle(FALSE),
                         ID_CRD_HEALTHWGT_SETTINGS,
                         (PVOID)pData);
    if (!hwnd)
        winhDebugBox(pData->hwndOwner,
                     "xwhealth",
                     "Cannot load setup dialog.");
    else
    {
        ULONG reply=0;
        pcmnSetControlsFont(hwnd,1,10000);
        pwinhCenterWindow(hwnd);
        reply = WinProcessDlg(hwnd);
        WinDestroyWindow(hwnd);
        if(reply == DID_OK)
        {
            // Reread setup string
            // resize control
        }
    }
}

/*
 *@@ fnwpSettingsDlg:
 *
 */

MRESULT EXPENTRY fnwpSettingsDlg(HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
    MRESULT mrc=0;
    switch(msg)
    {
        case WM_INITDLG:
        {
            PWIDGETSETTINGSDLGDATA pData=(PWIDGETSETTINGSDLGDATA)mp2;
            PMONITORSETUP pSetup = malloc(sizeof(MONITORSETUP));
            WinSetWindowPtr(hwnd, QWL_USER, pData);
            if(pSetup)
            {
                HWND hwndEf=WinWindowFromID(hwnd,ID_CRDI_SETUP_STRING);
                memset(pSetup,0,sizeof(*pSetup));
                pData->pUser = pSetup;
                // decode setup string into binary data;
                // this sets pSetup->pszViewString to a malloc() buffer
                MwgtScanSetup(pData->pcszSetupString, pSetup);
                winhSetEntryFieldLimit(hwndEf, 550);
                WinSetWindowText(hwndEf, pSetup->pszViewString);
            }

            mrc=WinDefDlgProc(hwnd,msg,mp1,mp2);
        break; }

        case WM_COMMAND:
        {
            PWIDGETSETTINGSDLGDATA pData=(PWIDGETSETTINGSDLGDATA)WinQueryWindowPtr(hwnd,QWL_USER);
            if(pData)
            {
                PMONITORSETUP pSetup = (PMONITORSETUP)pData->pUser;
                if(pSetup)
                {
                    switch((SHORT)mp1)
                    {
                        case DID_OK:
                        {
                            XSTRING strSetup;

                            CHAR sz[600];
                            HWND hwndEf=WinWindowFromID(hwnd,ID_CRDI_SETUP_STRING);

                            // V0.9.9 (2001-03-07) [umoeller]
                            if (pSetup->pszViewString)
                            {
                                free(pSetup->pszViewString);
                                pSetup->pszViewString = NULL;
                            }

                            WinQueryWindowText(hwndEf,600,sz);
                            if (sz[0])
                            {
                                pSetup->pszViewString = strdup(sz);
                            }

                            MwgtSaveSetup(&strSetup, pSetup);
                            pData->pctrSetSetupString(pData->hSettings,
                                                      strSetup.psz);
                            pxstrClear(&strSetup);
                            WinDismissDlg(hwnd,DID_OK);
                        break; }

                        case DID_CANCEL:
                            WinDismissDlg(hwnd,DID_CANCEL);
                        break;

                        case DID_HELP:
                            pctrDisplayHelp(pData->pGlobals,
                                            pcmnQueryHelpLibrary(),
                                            ID_XSH_WIDGET_HEALTH_SETTINGS);
                        break;
                    }
                }
            }
        break; }

        case WM_DESTROY:
        {
            // V0.9.9 (2001-03-07) [umoeller]
            // Stefan, you must clean up your allocation here...
            PWIDGETSETTINGSDLGDATA pData=(PWIDGETSETTINGSDLGDATA)WinQueryWindowPtr(hwnd,QWL_USER);
            if(pData)
            {
                PMONITORSETUP pSetup = (PMONITORSETUP)pData->pUser;
                if(pSetup)
                {
                    MwgtFreeSetup(pSetup);
                    free(pSetup);
                    pData->pUser = NULL;
                }
            }
        break; }

        default:
            mrc=WinDefDlgProc(hwnd,msg,mp1,mp2);
        break;
    }
    return mrc;
}

/* ******************************************************************
 *
 *   PM window class implementation
 *
 ********************************************************************/

/*
 *      This code has the actual PM window class.
 *
 */

/*
 *@@ MwgtCreate:
 *      implementation for WM_CREATE.
 */

MRESULT MwgtCreate(HWND hwnd,
                   PXCENTERWIDGET pWidget)
{
    MRESULT mrc = 0;            // continue window creation

    // PSZ p;
    PHEALTHPRIVATE pPrivate = malloc(sizeof(HEALTHPRIVATE));

    memset(pPrivate, 0, sizeof(HEALTHPRIVATE));
    // link the two together
    pWidget->pUser = pPrivate;
    pPrivate->pWidget = pWidget;
    // get widget type (clock, memory, ...) from class setting;
    // this is lost after WM_CREATE
    if (pWidget->pWidgetClass)
        pPrivate->ulType = pWidget->pWidgetClass->ulExtra;

    pPrivate->cxCurrent = 10;   // we'll resize ourselves later
    pPrivate->cyCurrent = 10;
    // initialize binary setup structure from setup string
    MwgtScanSetup(pWidget->pcszSetupString, &pPrivate->Setup);
    // set window font (this affects all the cached presentation
    // spaces we use)
    pwinhSetWindowFont(hwnd, (pPrivate->Setup.pszFont) ? pPrivate->Setup.pszFont
    // default font: use the same as in the rest of XWorkplace:
                       : pcmnQueryDefaultFont());

    // enable context menu help
    pWidget->pcszHelpLibrary = pcmnQueryHelpLibrary();
    pWidget->ulHelpPanelID = ID_XSH_WIDGET_HEALTH_MAIN;

    // start update timer
    pPrivate->ulTimerID = ptmrStartXTimer(pWidget->pGlobals->pvXTimerSet,
                                         hwnd,
                                         1,
                                         1500);
    return mrc;
}

/*
 *@@ MwgtControl:
 *      implementation for WM_CONTROL.
 *
 *@@added V0.9.7 (2000-12-14) [umoeller]
 */

BOOL MwgtControl(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    BOOL brc = FALSE;
    PXCENTERWIDGET pWidget = (PXCENTERWIDGET) WinQueryWindowPtr(hwnd, QWL_USER);

    if (pWidget)
    {
        PHEALTHPRIVATE pPrivate = (PHEALTHPRIVATE) pWidget->pUser;

        if (pPrivate)
        {
            USHORT usID = SHORT1FROMMP(mp1);
            USHORT usNotifyCode = SHORT2FROMMP(mp1);

            if (usID == ID_XCENTER_CLIENT)
            {
                switch (usNotifyCode)
                {
                    /*
                     * XN_QUERYSIZE:
                     *      XCenter wants to know our size.
                     */

                    case XN_QUERYSIZE:
                    {
                        PSIZEL pszl = (PSIZEL) mp2;

                        pszl->cx = pPrivate->cxCurrent;
                        pszl->cy = pPrivate->cyCurrent;
                        brc = TRUE;
                        break;
                    }
                    case XN_SETUPCHANGED:
                    {
                     PCSZ pcszNewSetupString=(const char*)mp2;
                     // reinitialize the setup data
                     MwgtFreeSetup(&pPrivate->Setup);
                     MwgtScanSetup(pcszNewSetupString,&pPrivate->Setup);
                     WinInvalidateRect(pWidget->hwndWidget,NULL,FALSE);
                    }
                }

            }
        }
    } // end if (pWidget)

    return brc;
}

/*
 * MwgtPaint:
 *      implementation for WM_PAINT.
 *
 *      The specified HPS is switched to RGB mode before
 *      painting.
 */

VOID MwgtPaint(HWND hwnd, PHEALTHPRIVATE pPrivate, HPS hps, BOOL fDrawFrame)
{
    RECTL rclWin;
    ULONG ulBorder = 1;
    CHAR szPaint[900] = "";
    // CHAR szValue[200];
    ULONG ulPaintLen = 0;
    POINTL aptlText[TXTBOX_COUNT];
    INT i;
    // double dvalue;
    // INT ivalue;
    LONG lcol = pPrivate->Setup.lcolBackground;

    // country settings from XCenter globals
    // (what a pointer)
    // PCOUNTRYSETTINGS pCountrySettings
       //  = (PCOUNTRYSETTINGS) pPrivate->pWidget->pGlobals->pCountrySettings;

    // now paint button frame
    WinQueryWindowRect(hwnd, &rclWin);
    pgpihSwitchToRGB(hps);
    if (pPrivate->pWidget->pGlobals->flDisplayStyle & XCS_SUNKBORDERS)
    {
        if (fDrawFrame)
        {
            RECTL rcl2;

            rcl2.xLeft = rclWin.xLeft;
            rcl2.yBottom = rclWin.yBottom;
            rcl2.xRight = rclWin.xRight - 1;
            rcl2.yTop = rclWin.yTop - 1;
            pgpihDraw3DFrame(hps, &rcl2, ulBorder
                             ,pPrivate->pWidget->pGlobals->lcol3DDark
                             ,pPrivate->pWidget->pGlobals->lcol3DLight);
        }

        // now paint middle
        rclWin.xLeft += ulBorder;
        rclWin.yBottom += ulBorder;
        rclWin.xRight -= ulBorder;
        rclWin.yTop -= ulBorder;
    }
    // Because frame color can change frequently (wether there is an
    // active event or not) we have to paint the background every time
    // if(fDrawFrame)
    {
        // Get color from setup
        // Check, if there is an event for this source active
        for (i = 0; i < 10; i++)
        {
            STHEALTH_EVT_FILTER_102 fltr;
            ULONG hEvt[10];
            ULONG numEvents = 0;

            fltr.cbLen = sizeof(STHEALTH_EVT_FILTER_102);
            fltr.source = 1;
            fltr.sourceNumber = i;
            fltr.actionState = STHEALTH_EVT_STATE_ISACTIVE;
            sthFilterEvents(hEvt, 10, &numEvents, STHEALTH_EVT_FILTER_LEVEL_102, &fltr);
            if (numEvents)
            {
                // Active event found. Set color to red
                lcol = 0x00ff0000;
                break;
            }
            fltr.source = 2;
            sthFilterEvents(hEvt, 10, &numEvents, STHEALTH_EVT_FILTER_LEVEL_102, &fltr);
            if (numEvents)
            {
                // Active event found. Set color to red
                lcol = 0x00ff0000;
                break;
            }
            fltr.source = 3;
            sthFilterEvents(hEvt, 10, &numEvents, STHEALTH_EVT_FILTER_LEVEL_102, &fltr);
            if (numEvents)
            {
                // Active event found. Set color to red
                lcol = 0x00ff0000;
                break;
            }
        }
        WinFillRect(hps, &rclWin, lcol);
    }
    switch (pPrivate->ulType)
    {
        case MWGT_HEALTHMONITOR:
            buildHealthString(szPaint,pPrivate->Setup.pszViewString);
            break;
    }
    ulPaintLen = strlen(szPaint);
    GpiQueryTextBox(hps, ulPaintLen, szPaint, TXTBOX_COUNT, aptlText);
    if (abs(aptlText[TXTBOX_TOPRIGHT].x + 4 - rclWin.xRight) > 4)
    {
        // we need more space: tell XCenter client
        pPrivate->cxCurrent = (aptlText[TXTBOX_TOPRIGHT].x + 2 * ulBorder + 4);
        WinPostMsg(WinQueryWindow(hwnd, QW_PARENT), XCM_SETWIDGETSIZE
                   ,(MPARAM) hwnd, (MPARAM) pPrivate->cxCurrent);
    }
    else
    {
        // sufficient space:
        GpiSetBackMix(hps, BM_OVERPAINT);
        WinDrawText(hps, ulPaintLen, szPaint, &rclWin,
                    pPrivate->Setup.lcolForeground,
                    lcol,
                    DT_CENTER | DT_VCENTER);
    }
}

/*
 *@@ MwgtPresParamChanged:
 *      implementation for WM_PRESPARAMCHANGED.
 *
 *@@changed V0.9.13 (2001-06-21) [umoeller]: changed XCM_SAVESETUP call for tray support
 *@@changed V1.0.8 (2007-08-05) [pr]: rewrote this mess @@fixes 994
 */

VOID MwgtPresParamChanged(HWND hwnd,
                          ULONG ulAttrChanged,
                          PXCENTERWIDGET pWidget)
{
    PHEALTHPRIVATE pPrivate = (PHEALTHPRIVATE) pWidget->pUser;

    if (pPrivate)
    {
        BOOL fInvalidate = TRUE;

        switch (ulAttrChanged)  // V1.0.8 (2007-08-05) [pr]
        {
            case 0:         // layout palette thing dropped
                pPrivate->Setup.lcolBackground
                    = pwinhQueryPresColor(hwnd,
                                          PP_BACKGROUNDCOLOR,
                                          FALSE,
                                          SYSCLR_DIALOGBACKGROUND);
                pPrivate->Setup.lcolForeground
                    = pwinhQueryPresColor(hwnd,
                                          PP_FOREGROUNDCOLOR,
                                          FALSE,
                                          SYSCLR_WINDOWTEXT);
                break;

            case PP_BACKGROUNDCOLOR:
                pPrivate->Setup.lcolBackground
                    = pwinhQueryPresColor(hwnd,
                                          PP_BACKGROUNDCOLOR,
                                          FALSE,
                                          SYSCLR_DIALOGBACKGROUND);
                break;

            case PP_FOREGROUNDCOLOR:
                pPrivate->Setup.lcolForeground
                    = pwinhQueryPresColor(hwnd,
                                          PP_FOREGROUNDCOLOR,
                                          FALSE,
                                          SYSCLR_WINDOWTEXT);
                break;

            case PP_FONTNAMESIZE:
                {
                    PSZ pszFont;

                    if (pPrivate->Setup.pszFont)
                    {
                        free(pPrivate->Setup.pszFont);
                        pPrivate->Setup.pszFont = NULL;
                    }

                    if (pszFont = pwinhQueryWindowFont(hwnd))
                    {
                        // we must use local malloc() for the font
                        pPrivate->Setup.pszFont = strdup(pszFont);
                        pwinhFree(pszFont);
                    }
                    break;
                }

            default:
                fInvalidate = FALSE;
        }

        if (fInvalidate)
        {
            XSTRING strSetup;

            WinInvalidateRect(hwnd, NULL, FALSE);
            MwgtSaveSetup(&strSetup, &pPrivate->Setup);
            if (strSetup.ulLength)
                // changed V0.9.13 (2001-06-21) [umoeller]:
                // post it to parent instead of fixed XCenter client
                // to make this trayable
                WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), // pPrivate->pWidget->pGlobals->hwndClient,
                           XCM_SAVESETUP,
                           (MPARAM) hwnd,
                           (MPARAM) strSetup.psz);

            pxstrClear(&strSetup);
        }
    }                           // end if (pPrivate)

}

/*
 *@@ fnwpHealthWidget:
 *      window procedure for the various "Monitor" widget classes.
 *
 *      This window proc is shared among the "Monitor" widgets,
 *      which all have in common that they are text-only display
 *      widgets with a little bit of extra functionality.
 *
 *      Presently, the following widgets are implemented this way:
 *      -- clock;
 *      -- swapper monitor;
 *      -- available-memory monitor.
 *      Supported setup strings:
 *      -- "TYPE={CLOCK|SWAPPER|MEMORY}"
 */

MRESULT EXPENTRY fnwpHealthWidget(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;
    PXCENTERWIDGET pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER);
                    // this ptr is valid after WM_CREATE

    switch (msg)
    {
        /*
         * WM_CREATE:
         *      as with all widgets, we receive a pointer to the
         *      XCENTERWIDGET in mp1, which was created for us.
         *
         *      The first thing the widget MUST do on WM_CREATE
         *      is to store the XCENTERWIDGET pointer (from mp1)
         *      in the QWL_USER window word by calling:
         *
         *          WinSetWindowPtr(hwnd, QWL_USER, mp1);
         *
         *      We use XCENTERWIDGET.pUser for allocating
         *      HEALTHPRIVATE for our own stuff.
         */

        case WM_CREATE:
            WinSetWindowPtr(hwnd, QWL_USER, mp1);
            pWidget = (PXCENTERWIDGET)mp1;
            if ((pWidget) && (pWidget->pfnwpDefWidgetProc))
                mrc = MwgtCreate(hwnd, pWidget);
            else
                // stop window creation!!
                mrc = (MPARAM)TRUE;
        break;

        /*
         * WM_CONTROL:
         *      process notifications/queries from the XCenter.
         */

        case WM_CONTROL:
            mrc = (MPARAM)MwgtControl(hwnd, mp1, mp2);
        break;

        /*
         * WM_PAINT:
         *
         */

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);

            if (hps)
            {
                // get widget data and its button data from QWL_USER
                PHEALTHPRIVATE pPrivate = (PHEALTHPRIVATE)pWidget->pUser;
                if (pPrivate)
                {
                    MwgtPaint(hwnd,
                              pPrivate,
                              hps,
                              TRUE);        // draw everything
                } // end if (pPrivate)

                WinEndPaint(hps);
            }
        break; }

        /*
         * WM_TIMER:
         *      clock timer --> repaint.
         */

        case WM_TIMER:
        {
            HPS hps = WinGetPS(hwnd);
            if (hps)
            {
                // get widget data and its button data from QWL_USER
                PHEALTHPRIVATE pPrivate = (PHEALTHPRIVATE)pWidget->pUser;
                if (pPrivate)
                {
                    MwgtPaint(hwnd,
                              pPrivate,
                              hps,
                              FALSE);   // text only
                } // end if (pPrivate)

                WinReleasePS(hps);
            }
        break; }

        /*
         * WM_PRESPARAMCHANGED:
         *
         */

        case WM_PRESPARAMCHANGED:
            if (pWidget)
                // this gets sent before this is set!
                MwgtPresParamChanged(hwnd, (ULONG)mp1, pWidget);
        break;

        /*
         * WM_DESTROY:
         *      clean up. This _must_ be passed on to
         *      ctrDefWidgetProc.
         */

        case WM_DESTROY:
        {
            PHEALTHPRIVATE pPrivate = (PHEALTHPRIVATE)pWidget->pUser;
            if (pPrivate)
            {
                if (pPrivate->ulTimerID)
                    ptmrStopXTimer(pPrivate->pWidget->pGlobals->pvXTimerSet,
                                  hwnd,
                                  pPrivate->ulTimerID);
                free(pPrivate);
            } // end if (pPrivate)
            mrc = pWidget->pfnwpDefWidgetProc(hwnd, msg, mp1, mp2);
        break; }

        default:
            if (pWidget)
                mrc = pWidget->pfnwpDefWidgetProc(hwnd, msg, mp1, mp2);
            else
                mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
    } // end switch(msg)

    return mrc;
}

/*
 *@@loadStHealth:
 *      load StHealth library and query function pointers.
 *
 *@@added V0.9.9 (2001-02-06) [smilcke]
 *@@changed V0.9.13 (2001-06-19) [umoeller]: added pszFailMod
 */

ULONG loadStHealth(PSZ pszFailMod,      // out: failing module
                   ULONG cbFailMod)
{
    ULONG rc = 0;

    if (!hmStHealth)
    {
        ULONG ul = 0;

        if (!(rc = DosLoadModule(pszFailMod,
                                 cbFailMod,
                                 "StHealth",
                                 &hmStHealth)))
        {
            for (ul = 0;
                 ul < sizeof(G_StHealthImports) / sizeof(G_StHealthImports[0]);
                 ul++)
            {
                if (rc = DosQueryProcAddr(hmStHealth,
                                          0,
                                          (PSZ)G_StHealthImports[ul].pcszFunctionName,
                                          G_StHealthImports[ul].ppFuncAddress))
                {
                    DosFreeModule(hmStHealth);
                    hmStHealth=0;
                    break;
                }
            }
        }
    }

    return rc;
}

/*
 *@@unloadStHealth:
 *      unload StHealth library.
 *
 *@@added V0.9.9 (2001-02-06) [smilcke]
 */
ULONG unloadStHealth(void)
{
    ULONG rc = 0;
    if(hmStHealth)
    {
        rc = DosFreeModule(hmStHealth);
        rc = 0;
    }
    return rc;
}

/* ******************************************************************
 *
 *   Exported procedures
 *
 ********************************************************************/

/*
 *@@ MwgtInitModule:
 *      required export with ordinal 1, which must tell
 *      the XCenter how many widgets this DLL provides,
 *      and give the XCenter an array of XCENTERWIDGETCLASS
 *      structures describing the widgets.
 *
 *      With this call, you are given the module handle of
 *      XFLDR.DLL. For convenience, you may resolve imports
 *      for some useful functions which are exported thru
 *      src\shared\xwp.def. See the code below.
 *
 *      This function must also register the PM window classes
 *      which are specified in the XCENTERWIDGETCLASS array
 *      entries. For this, you are given a HAB which you
 *      should pass to WinRegisterClass. For the window
 *      class style (4th param to WinRegisterClass),
 *      you should specify
 *
 +          CS_PARENTCLIP | CS_SIZEREDRAW | CS_SYNCPAINT
 *
 *      Your widget window _will_ be resized, even if you're
 *      not planning it to be.
 *
 *      This function only gets called _once_ when the widget
 *      DLL has been successfully loaded by the XCenter. If
 *      there are several instances of a widget running (in
 *      the same or in several XCenters), this function does
 *      not get called again. However, since the XCenter unloads
 *      the widget DLLs again if they are no longer referenced
 *      by any XCenter, this might get called again when the
 *      DLL is re-loaded.
 *
 *      There will ever be only one load occurence of the DLL.
 *      The XCenter manages sharing the DLL between several
 *      XCenters. As a result, it doesn't matter if the DLL
 *      has INITINSTANCE etc. set or not.
 *
 *      If this returns 0, this is considered an error, and the
 *      DLL will be unloaded again immediately.
 *
 *      If this returns any value > 0, *ppaClasses must be
 *      set to a static array (best placed in the DLL's
 *      global data) of XCENTERWIDGETCLASS structures,
 *      which must have as many entries as the return value.
 *
 *@@changed V0.9.13 (2001-06-19) [umoeller]: made error msg more meaningful
 */

ULONG EXPENTRY MwgtInitModule(HAB hab,  // XCenter's anchor block
                              HMODULE hmodPlugin, // module handle of the widget DLL
                              HMODULE hmodXFLDR,   // XFLDR.DLL module handle
                              PCXCENTERWIDGETCLASS *ppaClasses,
                              PSZ pszErrorMsg)  // if 0 is returned, 500 bytes of error msg
{
    ULONG ulrc = 0;
    ULONG ul = 0;
    BOOL fImportsFailed = FALSE;

    // resolve imports from XFLDR.DLL (this is basically
    // a copy of the doshResolveImports code, but we can't
    // use that before resolving...)
    for (ul = 0;
         ul < sizeof(G_aImports) / sizeof(G_aImports[0]); // array item count
         ul++)
    {
        if (DosQueryProcAddr(hmodXFLDR,
                             0, // ordinal, ignored
                             (PSZ)G_aImports[ul].pcszFunctionName,
                             G_aImports[ul].ppFuncAddress)
            != NO_ERROR)
        {
            strcpy(pszErrorMsg, "Import ");
            strcat(pszErrorMsg, G_aImports[ul].pcszFunctionName);
            strcat(pszErrorMsg, " failed.");
            fImportsFailed = TRUE;
            break;
        }
    }

    if (!fImportsFailed)
    {
        CHAR szFail[300] = "unknown";
        if (!loadStHealth(szFail, sizeof(szFail)))
        {
            sthRegisterDaemon(FALSE);
            if (!sthDetectChip())
            {
                if (!WinRegisterClass(hab
                                      ,WNDCLASS_WIDGET_XWHEALTH
                                      ,fnwpHealthWidget
                                      ,CS_PARENTCLIP | CS_SIZEREDRAW | CS_SYNCPAINT
                                      ,sizeof(PHEALTHPRIVATE)))
                    strcpy(pszErrorMsg, "WinRegisterClass failed.");
                else
                {
                    // no error:
                    // return classes
                    *ppaClasses = G_WidgetClasses;
                    // no. of classes in this DLL:
                    ulrc = sizeof(G_WidgetClasses) / sizeof(G_WidgetClasses[0]);
                }
            }
            else
                strcpy(pszErrorMsg, "No compatible chip detected.");
        }
        else
        {
            strcpy(pszErrorMsg, "Unable to load StHealth.DLL, failing module: \"");
            strcat(pszErrorMsg, szFail);
            strcat(pszErrorMsg, "\"");
        }
    }

    return ulrc;
}

/*
 *@@ MwgtUnInitModule:
 *      optional export with ordinal 2, which can clean
 *      up global widget class data.
 *
 *      This gets called by the XCenter right before
 *      a widget DLL gets unloaded. Note that this
 *      gets called even if the "init module" export
 *      returned 0 (meaning an error) and the DLL
 *      gets unloaded right away.
 *
 *@@added V0.9.7 (2000-12-07) [umoeller]
 */

VOID EXPENTRY MwgtUnInitModule(VOID)
{
    if (sthUnregisterDaemon)
        sthUnregisterDaemon();
    unloadStHealth();
}

/*
 *@@ MwgtQueryVersion:
 *      this new export with ordinal 3 can return the
 *      XWorkplace version number which is required
 *      for this widget to run. For example, if this
 *      returns 0.9.10, this widget will not run on
 *      earlier XWorkplace versions.
 *
 *      NOTE: This export was mainly added because the
 *      prototype for the "Init" export was changed
 *      with V0.9.9. If this returns 0.9.9, it is
 *      assumed that the INIT export understands
 *      the new FNWGTINITMODULE_099 format (see center.h).
 *
 *@@added V0.9.9 (2000-02-06) [umoeller]
 */

VOID EXPENTRY MwgtQueryVersion(PULONG pulMajor,
                               PULONG pulMinor,
                               PULONG pulRevision)
{
    *pulMajor = XFOLDER_MAJOR;              // dlgids.h
    *pulMinor = XFOLDER_MINOR;
    *pulRevision = XFOLDER_REVISION;
}

