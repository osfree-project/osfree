
/*
 *@@sourcefile cctl_tooltip.c:
 *      implementation for the "tooltip" common control.
 *      See comctl.c for an overview.
 *
 *      This has been extracted from comctl.c with V0.9.3 (2000-05-21) [umoeller].
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\comctl.h"
 *@@added V0.9.3 (2000-05-21) [umoeller].
 */

/*
 *      Copyright (C) 1997-2005 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WININPUT
#define INCL_WINPOINTERS
#define INCL_WINTRACKRECT
#define INCL_WINTIMER
#define INCL_WINSYS

#define INCL_WINRECTANGLES      /// xxx temporary

#define INCL_WINMENUS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINSTDCNR

#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#define INCL_GPILCIDS
#define INCL_GPIPATHS
#define INCL_GPIREGIONS
#define INCL_GPIBITMAPS             // added V0.9.1 (2000-01-04) [umoeller]: needed for EMX headers
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

#include "setup.h"                      // code generation and debugging options

#include "helpers\cnrh.h"
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"
#include "helpers\linklist.h"
#include "helpers\nls.h"
#include "helpers\winh.h"

#include "helpers\comctl.h"

#pragma hdrstop

/*
 *@@category: Helpers\PM helpers\Window classes\Tooltips
 *      See cctl_tooltip.c.
 */

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// linked list of all tools which were subclassed for tooltip
HMTX        G_hmtxSubclassedTools = NULLHANDLE;
LINKLIST    G_llSubclassedTools;        // linked list of SUBCLASSEDTOOL items

/* ******************************************************************
 *
 *   "Tooltip" control
 *
 ********************************************************************/

/*
 *@@ ctlRegisterTooltip:
 *      this registers the Tooltip window class (ctl_fnwpTooltip)
 *      for an application. This is required before the tooltip
 *      control can be used.
 *
 *@@added V0.9.0 [umoeller]
 */

BOOL ctlRegisterTooltip(HAB hab)
{
    return WinRegisterClass(hab,
                            WC_CCTL_TOOLTIP,
                            ctl_fnwpTooltip,
                            CS_HITTEST,     // class styles;
                                            // CS_FRAME not working,
                                            // CS_CLIPSIBLINGS not working
                            sizeof(PVOID) * 2);   // addt'l bytes to reserve:
                                    // one pointer for QWL_USER,
                                    // one more for instance data
}

/* ******************************************************************
 *
 *   Subclassing
 *
 ********************************************************************/

/*
 *@@ LockSubclassedTools:
 *      locks the global list of subclassed tools.
 *
 *@@added V0.9.12 (2001-04-28) [umoeller]
 */

STATIC BOOL LockSubclassedTools(VOID)
{
    if (!G_hmtxSubclassedTools)
    {
        // first call:

        // initialize the list
        lstInit(&G_llSubclassedTools,
                TRUE);      // auto-free

        // create mutex and request it right away
        return !DosCreateMutexSem(NULL,
                                  &G_hmtxSubclassedTools,
                                  0,
                                  TRUE);      // request!
    }

    return !DosRequestMutexSem(G_hmtxSubclassedTools, SEM_INDEFINITE_WAIT);
}

/*
 *@@ UnlockSubclassedTools:
 *      unlocks the global list of subclassed tools.
 *
 *@@added V0.9.12 (2001-04-28) [umoeller]
 *@@changed V0.9.12 (2001-05-03) [umoeller]: this did nothing... fixed
 */

STATIC VOID UnlockSubclassedTools(VOID)
{
    DosReleaseMutexSem(G_hmtxSubclassedTools);      // was missing V0.9.12 (2001-05-03) [umoeller]
}

/*
 *@@ SUBCLASSEDTOOL:
 *      structure created for each control which is
 *      subclassed by the tooltip control (ctl_fnwpTooltip).
 *
 *@@added V0.9.0 [umoeller]
 */

typedef struct _SUBCLASSEDTOOL
{
    HWND        hwndTool;
    PFNWP       pfnwpOrig;
    HWND        hwndTooltip;
    HAB         hab;
} SUBCLASSEDTOOL, *PSUBCLASSEDTOOL;

/*
 *@@ FindSubclassedTool:
 *      returns the SUBCLASSEDTOOL struct from the
 *      global list which matches hwndTool or NULL
 *      if not found.
 *
 *      Preconditions: Caller must hold the subclassed
 *      tools mutex.
 *
 *@@added V0.9.12 (2001-04-28) [umoeller]
 */

STATIC PSUBCLASSEDTOOL FindSubclassedTool(HWND hwndTool)
{
    PLISTNODE pNode = lstQueryFirstNode(&G_llSubclassedTools);
    while (pNode)
    {
        PSUBCLASSEDTOOL pstThis = (PSUBCLASSEDTOOL)pNode->pItemData;
        if (pstThis->hwndTool == hwndTool)
            return pstThis;

        pNode = pNode->pNext;
    }

    return NULL;
}

/*
 *@@ ctl_fnwpSubclassedTool:
 *      window procedure for tools which were subclassed
 *      to support tooltips.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.12 (2001-04-28) [umoeller]: added mutex protection
 *@@changed V1.0.2 (2003-07-26) [pr]: fixed tooltip moving with mouse from xcenters @@fixes 455
 */

MRESULT EXPENTRY ctl_fnwpSubclassedTool(HWND hwndTool, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    PFNWP   pfnwpOrig = NULL;

    if (LockSubclassedTools())
    {
        PSUBCLASSEDTOOL pst;

        if (pst = FindSubclassedTool(hwndTool))
        {
            pfnwpOrig = pst->pfnwpOrig;     // call default

            switch (msg)
            {
                case WM_MOUSEMOVE:
                case WM_BUTTON1DOWN:
                case WM_BUTTON1UP:
                case WM_BUTTON2DOWN:
                case WM_BUTTON2UP:
                case WM_BUTTON3DOWN:
                case WM_BUTTON3UP:
                case WM_MOUSELEAVE: // V1.0.2 (2003-07-26) [pr]: @@fixes 455
                {
                    QMSG qmsg;
                    qmsg.hwnd = hwndTool;
                    qmsg.msg = msg;
                    qmsg.mp1 = mp1;
                    qmsg.mp2 = mp2;
                    // _Pmpf((__FUNCTION__ ": sending TTM_RELAYEVENT"));
                    WinSendMsg(pst->hwndTooltip,
                               TTM_RELAYEVENT,
                               (MPARAM)0,
                               (MPARAM)&qmsg);
                }
                break;

                case WM_DESTROY:
                    lstRemoveItem(&G_llSubclassedTools, pst);
                            // this frees the item
                break;
            }
        }

        UnlockSubclassedTools();
    }

    if (pfnwpOrig)
        mrc = pfnwpOrig(hwndTool, msg, mp1, mp2);

    return mrc;
}

/*
 *@@ SubclassTool:
 *      this gets called from ctl_fnwpTooltip if a control
 *      is to be subclassed to support mouse messaging
 *      (TTF_SUBCLASS flag).
 *
 *      Preconditions: Caller must hold the subclassed
 *      tools mutex.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.12 (2001-04-28) [umoeller]: renamed from SubclassToolForToolInfo
 */

STATIC BOOL SubclassTool(HWND hwndTooltip,
                         HWND hwndTool)
{
    BOOL    brc = FALSE;

    // make sure the tool is not on the list yet
    // V0.9.12 (2001-04-28) [umoeller]
    if (!FindSubclassedTool(hwndTool))
    {
        PFNWP   pfnwpOrig;
        if (pfnwpOrig = WinSubclassWindow(hwndTool,
                                          ctl_fnwpSubclassedTool))
        {
            PSUBCLASSEDTOOL pst;
            if (pst = (PSUBCLASSEDTOOL)malloc(sizeof(SUBCLASSEDTOOL)))
            {
                pst->pfnwpOrig = pfnwpOrig;
                pst->hwndTooltip = hwndTooltip;
                pst->hwndTool = hwndTool;
                pst->hab = WinQueryAnchorBlock(hwndTool);

                brc = !!lstAppendItem(&G_llSubclassedTools, pst);
            }
        }
    }

    return brc;
}

/*
 *@@ UnSubclassTool:
 *      un-subclasses a tool previously subclassed by
 *      SubclassToolForToolinfo.
 *
 *      Preconditions: Caller must hold the subclassed
 *      tools mutex.
 *
 *@@added V0.9.12 (2001-04-28) [umoeller]
 */

STATIC BOOL UnSubclassTool(HWND hwndTool)
{
    PSUBCLASSEDTOOL pst;
    if (pst = FindSubclassedTool(hwndTool))
    {
        WinSubclassWindow(hwndTool,
                          pst->pfnwpOrig);
                            // orig winproc == un-subclass
        return lstRemoveItem(&G_llSubclassedTools, pst);
                    // this frees the item
    }

    return FALSE;
}

/* ******************************************************************
 *
 *   Implementation
 *
 ********************************************************************/

/*
 *@@ TOOLTIPDATA:
 *      private data structure stored in the window
 *      words of ctl_fnwpTooltip to hold information for
 *      a tooltip instance.
 *
 *@@added V0.9.0 [umoeller]
 */

typedef struct _TOOLTIPDATA
{
    HWND        hwndOwner;          // from WM_CREATE
    HAB         hab;                // from WM_CREATE
    USHORT      usTooltipID;        // from WM_CREATE

    BOOL        fIsActive;          // TRUE per default; changed by TTM_ACTIVATE

    ULONG       idTimerInitial,     // if != 0, "initial" timer is running
                idTimerAutopop;     // if != 0, "autopop" (hide) timer is running

    ULONG       ulTimeoutInitial,   // "initial" timer timeout (ms)
                ulTimeoutAutopop,   // "autopop" (hide) timer timeout (ms)
                ulTimeoutReshow;    // "reshow" timer timeout (ms)

    LINKLIST    llTools;            // linked list of TOOLINFO structures
                                    // containing the tools
    FONTMETRICS fontmetrics;        // current font
    LONG        lForeColor,         // current foreground color
                lBackColor,         // current background color
                l3DHiColor,         // 3D border light color
                l3DLoColor;         // 3D border dark color
    PSZ         pszPaintText;       // text to paint (new buffer)

    POINTL      ptlPointerLast;     // last mouse pointer position

    PTOOLINFO   ptiMouseOver;       // tool info over which the mouse resides

    BOOL        fIsVisible;         // TRUE if tooltip is visible

    // CHAR        szTextBuf[256];     // static buffer for copying/loading strings
} TOOLTIPDATA, *PTOOLTIPDATA;

// timer IDs
#define TOOLTIP_ID_TIMER_INITIAL    1
#define TOOLTIP_ID_TIMER_AUTOPOP    2

// tooltip window border (free spaces)
#define TOOLTIP_CX_BORDER           5
#define TOOLTIP_CY_BORDER           3

/*
 *@@ UpdateTooltipPresColors:
 *      this gets called during WM_CREATE and WM_PRESPARAMCHANGED
 *      in ctl_fnwpTooltip to set the colors in TOOLTIPDATA according
 *      to the control's presparams.
 */

STATIC VOID UpdateTooltipPresColors(HWND hwndTooltip)      // in: tooltip control
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);

    // tooltip background color:
    pttd->lBackColor = winhQueryPresColor(hwndTooltip,
                                          PP_MENUBACKGROUNDCOLOR,
                                          FALSE,             // no inherit
                                          SYSCLR_ENTRYFIELD);
    // tooltip text color:
    pttd->lForeColor = winhQueryPresColor(hwndTooltip,
                                          PP_MENUFOREGROUNDCOLOR,
                                          FALSE,             // no inherit
                                          SYSCLR_WINDOWTEXT);
    // 3D border light color:
    pttd->l3DHiColor = winhQueryPresColor(hwndTooltip,
                                          PP_MENUHILITEFGNDCOLOR,
                                          FALSE,             // no inherit
                                          SYSCLR_WINDOWTEXT);
    // 3D border dark color:
    pttd->l3DLoColor = winhQueryPresColor(hwndTooltip,
                                          PP_MENUHILITEBGNDCOLOR,
                                          FALSE,             // no inherit
                                          SYSCLR_WINDOWTEXT);
}

/*
 *@@ TtmCreate:
 *      implementation for WM_CREATE in ctl_fnwpTooltip.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 */

STATIC MRESULT TtmCreate(HWND hwndTooltip,
                         MPARAM mp2)
{
    PTOOLTIPDATA pttd;
    PCREATESTRUCT pcs = (PCREATESTRUCT)mp2;

    // allocate and initialize tooltip data
    if (pttd = (PTOOLTIPDATA)malloc(sizeof(TOOLTIPDATA)))
    {
        CHAR        szFont[256];
        memset(pttd, 0, sizeof(TOOLTIPDATA));
        WinSetWindowPtr(hwndTooltip, 1, pttd);

        pttd->hwndOwner = pcs->hwndOwner;
        pttd->hab = WinQueryAnchorBlock(hwndTooltip);
        pttd->usTooltipID = pcs->id;

        pttd->fIsActive = TRUE;

        // default timeouts
        pttd->ulTimeoutInitial = 1000;
        pttd->ulTimeoutAutopop = 5000;
        pttd->ulTimeoutReshow = 500;

        // get colors from presparams/syscolors
        UpdateTooltipPresColors(hwndTooltip);

        // check if font presparam set
        if (WinQueryPresParam(hwndTooltip,
                              PP_FONTNAMESIZE, 0,
                              NULL,
                              sizeof(szFont),
                              szFont,
                              QPF_NOINHERIT)
                == 0)
        {
            // no: set default font presparam
            // (we never want the System Proportional font)
            winhSetWindowFont(hwndTooltip,
                              NULL);      // default (WarpSans or 8.Helv)
        }

        pttd->pszPaintText = strdup("undefined");

        lstInit(&pttd->llTools, TRUE);      // auto-free items

        // override CREATESTRUCT
        WinSetWindowPos(hwndTooltip,
                        HWND_TOP,
                        50, 50,
                        100, 100,
                        SWP_MOVE | SWP_SIZE | SWP_ZORDER | SWP_HIDE);

        return (MPARAM)FALSE;
    }

    // malloc failed:
    return (MPARAM)TRUE;
}

/*
 *@@ TtmTimer:
 *      implementation for WM_TIMER in ctl_fnwpTooltip.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 */

STATIC BOOL TtmTimer(HWND hwndTooltip, MPARAM mp1)
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);
    USHORT  usTimer = SHORT1FROMMP(mp1);

    switch (usTimer)
    {
        case TOOLTIP_ID_TIMER_INITIAL:
            // _Pmpf(("WM_TIMER: Stopping initial timer: %d", usTimer));
            // _Pmpf((__FUNCTION__ ": TOOLTIP_ID_TIMER_INITIAL"));
            WinStopTimer(pttd->hab,
                         hwndTooltip,
                         usTimer);
            pttd->idTimerInitial = 0;

            if (pttd->fIsActive)
                // show tooltip
                WinPostMsg(hwndTooltip, TTM_SHOWTOOLTIPNOW, (MPARAM)TRUE, 0);
        break;

        case TOOLTIP_ID_TIMER_AUTOPOP:
            // _Pmpf(("WM_TIMER: Stopping autopop timer: %d", usTimer));
            WinStopTimer(pttd->hab,
                         hwndTooltip,
                         usTimer);
            pttd->idTimerAutopop = 0;
            WinPostMsg(hwndTooltip, TTM_SHOWTOOLTIPNOW, (MPARAM)FALSE, 0);
        break;

        default:
            return FALSE;
    } // end switch

    return TRUE;
}

/*
 *@@ TtmPaint:
 *      implementation for WM_PAINT in ctl_fnwpTooltip.
 *
 *@@added V0.9.1 (99-11-30) [umoeller]
 */

STATIC VOID TtmPaint(HWND hwndTooltip)
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);
    HPS         hps = WinBeginPaint(hwndTooltip, NULLHANDLE, NULL);
    POINTL      ptl = {0, 0};
    RECTL       rclWindow,
                rclWindowBak;
    ULONG       ulStyle = WinQueryWindowULong(hwndTooltip, QWL_STYLE);

    ULONG       ulRounding = 0;
    if (ulStyle & TTS_ROUNDED)
        ulRounding = TT_ROUNDING;


    gpihSwitchToRGB(hps);

    // get tooltip size; this has been properly calculated
    // by TTM_SHOWTOOLTIPNOW earlier
    WinQueryWindowRect(hwndTooltip, &rclWindow);
    memcpy(&rclWindowBak, &rclWindow, sizeof(RECTL));

    if (ulStyle & TTS_SHADOW)
    {
        // if shadows are on, the window is too large;
        // make rectl smaller for rest of paint
        rclWindow.xRight -= TT_SHADOWOFS;
        rclWindow.yBottom += TT_SHADOWOFS;
        // restore old pattern
        GpiSetPattern(hps, PATSYM_DEFAULT);
    }

    // draw shadow
    if (ulStyle & TTS_SHADOW)
    {
        GpiSetColor(hps, CLR_BLACK);
        GpiSetPattern(hps, PATSYM_HALFTONE);
        ptl.x = rclWindowBak.xLeft + TT_SHADOWOFS;
        ptl.y = rclWindowBak.yBottom;
        GpiMove(hps, &ptl);
        ptl.x = rclWindowBak.xRight - 1;
        ptl.y = rclWindowBak.yTop - TT_SHADOWOFS;
        GpiBox(hps,
               DRO_FILL,
               &ptl,
               ulRounding, ulRounding);
        // restore pattern
        GpiSetPattern(hps, PATSYM_DEFAULT);
    }

    // draw "real" rectangle
    ptl.x = rclWindow.xLeft;
    ptl.y = rclWindow.yBottom;
    GpiMove(hps, &ptl);
    ptl.x = rclWindow.xRight - 1;
    ptl.y = rclWindow.yTop - 1;
    GpiSetColor(hps, pttd->lBackColor);
    GpiBox(hps,
           DRO_FILL,
           &ptl,
           6, 6);

    GpiSetColor(hps, pttd->lForeColor);
    GpiBox(hps,
           DRO_OUTLINE,
           &ptl,
           ulRounding, ulRounding);

    if (pttd->pszPaintText)
    {
        RECTL rclText;
        GpiSetColor(hps, pttd->lForeColor);
        GpiSetBackColor(hps, pttd->lBackColor);
        rclText.xLeft = rclWindow.xLeft + TOOLTIP_CX_BORDER;
        rclText.xRight = rclWindow.xRight - TOOLTIP_CX_BORDER;
        rclText.yBottom = rclWindow.yBottom + TOOLTIP_CY_BORDER;
        rclText.yTop = rclWindow.yTop - TOOLTIP_CY_BORDER;
        winhDrawFormattedText(hps,
                              &rclText,
                              pttd->pszPaintText,
                              DT_LEFT | DT_TOP | DT_WORDBREAK);
    }

    WinEndPaint(hps);
}

/*
 *@@ TtmDestroy:
 *      implementation for WM_DESTROY in ctl_fnwpTooltip.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 */

STATIC VOID TtmDestroy(HWND hwndTooltip)
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);
    // stop timers
    if (pttd->idTimerInitial)
        WinStopTimer(pttd->hab,
                     hwndTooltip,
                     pttd->idTimerInitial);
    if (pttd->idTimerAutopop)
        WinStopTimer(pttd->hab,
                     hwndTooltip,
                     pttd->idTimerAutopop);
    if (pttd->pszPaintText)
        free(pttd->pszPaintText);

    // un-subclass all tools that we subclassed
    // V0.9.12 (2001-04-28) [umoeller]
    if (LockSubclassedTools())
    {
        PLISTNODE pNode;
        PSUBCLASSEDTOOL pst;
        for (pNode = lstQueryFirstNode(&pttd->llTools);
             pNode;
             pNode = pNode->pNext)
        {
            PTOOLINFO pti = (PTOOLINFO)pNode->pItemData;
            if (pst = FindSubclassedTool(pti->hwndTool))
                UnSubclassTool(pti->hwndTool);
        }

        UnlockSubclassedTools();
    }
    // end V0.9.12 (2001-04-28) [umoeller]

    lstClear(&pttd->llTools);

    free(pttd);
}

/*
 *@@ TtmAddTool:
 *      implementation for TTM_ADDTOOL in ctl_fnwpTooltip.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 */

STATIC MRESULT TtmAddTool(HWND hwndTooltip, MPARAM mp2)
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);
    if (mp2)
    {
        PTOOLINFO ptiPassed = (PTOOLINFO)mp2,
                  ptiNew = (PTOOLINFO)malloc(sizeof(TOOLINFO));
        if (ptiNew)
        {
            memcpy(ptiNew, ptiPassed, sizeof(TOOLINFO));
            lstAppendItem(&pttd->llTools,
                          ptiNew);

            if (    (ptiPassed->ulFlags & TTF_SUBCLASS)
                 && (LockSubclassedTools()) // V0.9.12 (2001-04-28) [umoeller]
               )
            {
                // caller wants this tool to be subclassed:
                // well, do it then
                SubclassTool(hwndTooltip,
                             ptiPassed->hwndTool);

                UnlockSubclassedTools();
            }

            return ((MPARAM)TRUE);
        }
    }

    return (MPARAM)FALSE;
}

/*
 *@@ TtmDelTool:
 *      implementation for TTM_DELTOOL in ctl_fnwpTooltip.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 *@@changed V0.9.13 (2001-06-21) [umoeller]: fixed missing unlock
 *@@changed V0.9.13 (2001-06-21) [umoeller]: fixed endless loop
 */

STATIC VOID TtmDelTool(HWND hwndTooltip, MPARAM mp2)
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);
    PTOOLINFO ptiSearch;
    if (ptiSearch = (PTOOLINFO)mp2)
    {
        PLISTNODE pToolNode = lstQueryFirstNode(&pttd->llTools);
        while (pToolNode)
        {
            PTOOLINFO ptiThis = (PTOOLINFO)pToolNode->pItemData;
            if (    (ptiThis->hwndToolOwner == ptiSearch->hwndToolOwner)
                 && (ptiThis->hwndTool == ptiSearch->hwndTool)
               )
            {
                // found:

                // V0.9.12 (2001-04-28) [umoeller]
                // unsubclass if this was subclassed
                if (ptiThis->ulFlags & TTF_SUBCLASS)
                {
                    if (LockSubclassedTools())
                    {
                        UnSubclassTool(ptiSearch->hwndTool);

                        UnlockSubclassedTools();
                                // was missing V0.9.13 (2001-06-21) [umoeller]
                    }
                }

                // remove the tool from the list
                lstRemoveNode(&pttd->llTools, pToolNode);

                break;
            }

            pToolNode = pToolNode->pNext;
                    // fixed endless loop V0.9.13 (2001-06-21) [umoeller]
        }
    }
}

/*
 *@@ TtmRelayEvent:
 *      implementation for TTM_RELAYEVENT in ctl_fnwpTooltip.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 *@@changed V0.9.19 (2002-05-14) [umoeller]: fixed bad stop timer, thanks yuri
 *@@changed V1.0.2 (2003-07-26) [pr]: fixed tooltip moving with mouse from xcenters @@fixes 455
 */

STATIC VOID TtmRelayEvent(HWND hwndTooltip, MPARAM mp2)
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);
    PQMSG pqmsg = (PQMSG)mp2;
    if (pqmsg)
    {
        POINTL      ptlPointer;
        PLISTNODE   pToolNode;

        // moved stop timer down V0.9.19 (2002-05-14) [umoeller]

        WinQueryPointerPos(HWND_DESKTOP, &ptlPointer);

        // find TOOLINFO from mouse position
        pttd->ptiMouseOver = NULL;
        pToolNode = lstQueryFirstNode(&pttd->llTools);
        while (pToolNode)
        {
            PTOOLINFO pti = (PTOOLINFO)pToolNode->pItemData;
            if (pti->hwndTool == pqmsg->hwnd)
            {
                // _Pmpf((__FUNCTION__ ": found tool"));
                pttd->ptiMouseOver = pti;
                break;
            }
            pToolNode = pToolNode->pNext;
        }

        if (    (ptlPointer.x != pttd->ptlPointerLast.x)
             || (ptlPointer.y != pttd->ptlPointerLast.y)
             || (pqmsg->msg == WM_BUTTON1DOWN)
             || (pqmsg->msg == WM_BUTTON2DOWN)
             || (pqmsg->msg == WM_BUTTON3DOWN)
             || (pqmsg->msg == WM_MOUSELEAVE) // V1.0.2 (2003-07-26) [pr]: @@fixes 455
           )
        {
            // mouse pos changed:
            // moved stop timer here V0.9.19 (2002-05-14) [umoeller]
            if (pttd->idTimerInitial)
            {
                // _Pmpf(("TTM_RELAYEVENT: Stopping timer: %d", pttd->idTimerInitial));
                WinStopTimer(pttd->hab,
                             hwndTooltip,
                             TOOLTIP_ID_TIMER_INITIAL);
                pttd->idTimerInitial = 0;
            }

            // hide tooltip
            WinPostMsg(hwndTooltip,
                       TTM_SHOWTOOLTIPNOW,
                       (MPARAM)FALSE,
                       0);
            memcpy(&pttd->ptlPointerLast, &ptlPointer, sizeof(POINTL));

            // _Pmpf((__FUNCTION__ ": pttd->ptiMouseOver: 0x%lX", pttd->ptiMouseOver));
            // _Pmpf((__FUNCTION__ ": pttd->fIsActive: 0x%lX", pttd->fIsActive));
            if (    (pttd->ptiMouseOver)
                 && (pttd->fIsActive)
                 && (pqmsg->msg != WM_MOUSELEAVE) // V1.0.2 (2003-07-26) [pr]: @@fixes 455
               )
            {
                // tool found and tooltip is activated:
                pttd->idTimerInitial = WinStartTimer(pttd->hab,
                                                     hwndTooltip,
                                                     TOOLTIP_ID_TIMER_INITIAL,
                                                     pttd->ulTimeoutInitial);
                // _Pmpf(("TTM_RELAYEVENT: Started timer: %d", pttd->idTimerInitial));
            }
        }
    } // end if (pqmsg)
}

/*
 *@@ TtmGetDelayTime:
 *      implementation for TTM_GETDELAYTIME in ctl_fnwpTooltip.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 */

STATIC MRESULT TtmGetDelayTime(HWND hwndTooltip, MPARAM mp1)
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);

    switch ((ULONG)mp1)
    {
        case TTDT_AUTOPOP:
            return (MRESULT)pttd->ulTimeoutAutopop;

        case TTDT_INITIAL:
            return (MRESULT)pttd->ulTimeoutInitial;

        case TTDT_RESHOW:
            return (MRESULT)pttd->ulTimeoutReshow;
    }

    return 0;
}

/*
 *@@ TtmSetDelayTime:
 *      implementation for TTM_SETDELAYTIME in ctl_fnwpTooltip.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 */

STATIC VOID TtmSetDelayTime(HWND hwndTooltip, MPARAM mp1, MPARAM mp2)
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);
    ULONG   iDelay = (ULONG)mp2;
    switch (SHORT1FROMMP(mp1))
    {
        case TTDT_AUTOMATIC:
            pttd->ulTimeoutInitial = iDelay;
            pttd->ulTimeoutAutopop = iDelay * 5;
            pttd->ulTimeoutReshow = iDelay / 2;
        break;

        case TTDT_AUTOPOP:
            pttd->ulTimeoutAutopop = iDelay;
        break;

        case TTDT_INITIAL:
            pttd->ulTimeoutInitial = iDelay;
        break;

        case TTDT_RESHOW:
            pttd->ulTimeoutReshow = iDelay;
        break;
    }
}

/*
 *@@ TtmGetText:
 *      implementation for TTM_GETTEXT in ctl_fnwpTooltip.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 */

STATIC VOID TtmGetText(HWND hwndTooltip, MPARAM mp2)
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);
    PTOOLINFO pti = (PTOOLINFO)mp2;

    if (pti->pszText == PSZ_TEXTCALLBACK)
    {
        // TTN_NEEDTEXT notification desired:
        // compose values for that msg
        TOOLTIPTEXT ttt = {0};
        ttt.hwndTooltip = hwndTooltip;
        ttt.hwndTool = pti->hwndTool;
        WinSendMsg(pti->hwndToolOwner,
                   WM_CONTROL,
                   MPFROM2SHORT(pttd->usTooltipID,  // tooltip control wnd ID
                                TTN_NEEDTEXT),
                   &ttt);

        // in case of error: set lpszText to NULL; this
        // is not specified in the docs however.
        pti->pszText = NULL;

        switch (ttt.ulFormat)
        {
            case TTFMT_PSZ:
                if (ttt.pszText)
                    pti->pszText = ttt.pszText;
            break;

            case TTFMT_STRINGRES:
                    // @@todo
            break;
        }
    }
}

/*
 *@@ TtmEnumTools:
 *      implementation for TTM_ENUMTOOLS in ctl_fnwpTooltip.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 */

STATIC MRESULT TtmEnumTools(HWND hwndTooltip, MPARAM mp1, MPARAM mp2)
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);
    PTOOLINFO ptiTarget;
    if (ptiTarget = (PTOOLINFO)mp2)
    {
        PTOOLINFO ptiFound = (PTOOLINFO)lstItemFromIndex(&pttd->llTools,
                                                         SHORT1FROMMP(mp1));
        if (ptiFound)
        {
            memcpy(ptiTarget, ptiFound, sizeof(TOOLINFO));
            return (MRESULT)TRUE;
        }
    }

    return (MRESULT)FALSE;
}

/*
 *@@ TtmGetCurrentTool:
 *      implementation for TTM_GETCURRENTTOOL in ctl_fnwpTooltip.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 */

STATIC MRESULT TtmGetCurrentTool(HWND hwndTooltip, MPARAM mp2)
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);
    PTOOLINFO ptiTarget;
    if (ptiTarget = (PTOOLINFO)mp2)
    {
        if (pttd->ptiMouseOver)
        {
            memcpy(ptiTarget, pttd->ptiMouseOver, sizeof(TOOLINFO));
            return (MRESULT)TRUE;
        }
    }

    return (MRESULT)FALSE;
}

/*
 *@@ TtmGetToolInfo:
 *      implementation for TTM_GETTOOLINFO in ctl_fnwpTooltip.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 */

STATIC MRESULT TtmGetToolInfo(HWND hwndTooltip, MPARAM mp2)
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);
    PTOOLINFO ptiSearch;
    if (ptiSearch = (PTOOLINFO)mp2)
    {
        PLISTNODE pToolNode = lstQueryFirstNode(&pttd->llTools);
        while (pToolNode)
        {
            PTOOLINFO ptiThis = (PTOOLINFO)pToolNode->pItemData;
            if (    (ptiThis->hwndToolOwner == ptiSearch->hwndToolOwner)
                 && (ptiThis->hwndTool == ptiSearch->hwndTool)
               )
            {
                // found:
                memcpy(ptiSearch, ptiThis, sizeof(TOOLINFO));
                return (MPARAM)TRUE;
            }
            pToolNode = pToolNode->pNext;
        }
    }

    return (MRESULT)FALSE;
}

/*
 *@@ FormatTooltip:
 *      formats the tooltip window according to
 *      its text (which is assumed to be in
 *      pttd->pszPaintText) and positions it
 *      on the screen.
 *
 *      This does not change the visibility
 *      of the tooltip; if it is not yet visible,
 *      you must show it afterwards.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 *@@changed V1.0.4 (2005-10-15) [bvl]: Add 1 pixel to width of DBCS tooltip @@fixes 676
 */

STATIC VOID FormatTooltip(HWND hwndTooltip,
                          PTOOLTIPDATA pttd,
                          PPOINTL pptlPointer)      // in: current pointer pos or NULL
{
    // find out how much space we need
    RECTL   rcl = { 0, 0, 300, 1000 };
    POINTL  ptlTooltip;
    LONG    cx, cy;
    ULONG   ulStyle = WinQueryWindowULong(hwndTooltip, QWL_STYLE);

    HPS hps = WinGetPS(hwndTooltip);

    winhDrawFormattedText(hps,
                          &rcl,
                          pttd->pszPaintText,
                          DT_LEFT | DT_TOP | DT_WORDBREAK | DT_QUERYEXTENT);
    WinReleasePS(hps);

    // calc width and height of tooltip
    // DBCS needs a extra pixel to show characters.
    cx = rcl.xRight + 2*TOOLTIP_CX_BORDER;
    if (nlsDBCS())
        cx++;

    cy = (rcl.yTop - rcl.yBottom) + 2*TOOLTIP_CY_BORDER;

    // calc x and y pos of tooltip:

    // per default, use pointer pos
    ptlTooltip.x = pptlPointer->x - cx/2;
    ptlTooltip.y = pptlPointer->y - cy;

    // do we need the tool's position?
    if (    pttd->ptiMouseOver->ulFlags
                & (TTF_CENTER_X_ON_TOOL | TTF_POS_Y_ABOVE_TOOL | TTF_POS_Y_BELOW_TOOL)
       )
    {
        // yes:
        SWP     swpTool;
        POINTL  ptlTool;
        WinQueryWindowPos(pttd->ptiMouseOver->hwndTool, &swpTool);
        ptlTool.x = swpTool.x;
        ptlTool.y = swpTool.y;
        // convert x, y to desktop points
        WinMapWindowPoints(WinQueryWindow(pttd->ptiMouseOver->hwndTool,
                                          QW_PARENT), // hwndFrom
                           HWND_DESKTOP,            // hwndTo
                           &ptlTool,
                           1);

        // X
        if (pttd->ptiMouseOver->ulFlags & TTF_CENTER_X_ON_TOOL)
            // center X on tool:
            ptlTooltip.x = ptlTool.x + ((swpTool.cx - cx) / 2L);

        // Y
        if (pttd->ptiMouseOver->ulFlags & TTF_POS_Y_ABOVE_TOOL)
            ptlTooltip.y = ptlTool.y + swpTool.cy;
        else if (pttd->ptiMouseOver->ulFlags & TTF_POS_Y_BELOW_TOOL)
            ptlTooltip.y = ptlTool.y - cy;
    }

    // if "shy mouse" is enabled, make
    // sure the tool tip is not under the
    // mouse pointer
    if (ulStyle & TTF_SHYMOUSE)
    {
        // we need to subtract the current mouse
        // pointer's hot spot from the current
        // pointer position
        HPOINTER    hptr = WinQueryPointer(HWND_DESKTOP);
        POINTERINFO pi;
        if (WinQueryPointerInfo(hptr, &pi))
        {
            // calc bottom edge of mouse pointer rect
            ULONG yBottomPointer = (pptlPointer->y - pi.yHotspot);
            // _Pmpf(("yHotspot: %d", pi.yHotspot));
            if (   (ptlTooltip.y + cy) // top edge of tool tip
                 > yBottomPointer
               )
            {
                ptlTooltip.y = pptlPointer->y - cy - pi.yHotspot;
            }
        }
    }

    // constrain to screen
    if (ptlTooltip.x < 0)
        ptlTooltip.x = 0;
    if (ptlTooltip.y < 0)
        ptlTooltip.y = 0;
    if (ptlTooltip.x + cx > G_cxScreen)
        ptlTooltip.x = G_cxScreen - cx;
    if (ptlTooltip.y + cy > G_cyScreen)
        ptlTooltip.y = G_cyScreen - cy;

    // if shadow is enabled,
    // enlarge; the shadow might by
    // off-screen now, but that's OK
    if (ulStyle & TTS_SHADOW)
    {
        cx += TT_SHADOWOFS;
        cy += TT_SHADOWOFS;
        ptlTooltip.y -= TT_SHADOWOFS;
    }

    // set tooltip position at the pos we calculated
    // and show tooltip
    WinSetWindowPos(hwndTooltip,
                    HWND_TOP,
                    ptlTooltip.x,
                    ptlTooltip.y,
                    cx,
                    cy,
                    SWP_MOVE | SWP_SIZE | SWP_ZORDER);
}

/*
 *@@ TtmUpdateTipText:
 *      implementation for TTM_UPDATETIPTEXT in ctl_fnwpTooltip.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 */

STATIC VOID TtmUpdateTipText(HWND hwndTooltip,
                             const char *pcszNewText)
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);

    // has text really changed?
    if (    (    (pttd->pszPaintText)
              && (pcszNewText)
              && (strcmp(pttd->pszPaintText, pcszNewText))
            )
         || (pttd->pszPaintText && !pcszNewText)
         || (!pttd->pszPaintText && pcszNewText)
       )
    {
        // yes:
        if (pttd->pszPaintText)
        {
            free(pttd->pszPaintText);
            pttd->pszPaintText = NULL;
        }

        if (pcszNewText)
            pttd->pszPaintText = strdup(pcszNewText);

        if (pttd->fIsVisible)
        {
            // currently showing:
            // reformat
            POINTL ptlPointer;
            WinQueryPointerPos(HWND_DESKTOP, &ptlPointer);
            FormatTooltip(hwndTooltip,
                          pttd,
                          &ptlPointer);
            WinInvalidateRect(hwndTooltip, NULL, FALSE);
        }
    }
}

/*
 *@@ TtmShowTooltip:
 *      implementation for TTM_SHOW_TOOLTIP.
 *
 *      Depending on fShow, this shows or hides the
 *      tool tip at the position specified by the
 *      current tool or the mouse pointer (specified
 *      by the tool's style flags).
 *
 *@@added V0.9.1 (2000-02-04) [umoeller]
 */

STATIC VOID TtmShowTooltip(HWND hwndTooltip,
                           BOOL fShow)  // if TRUE: show, else: HIDE
{
    PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);
    if (fShow)
    {
        /*
         * show tooltip::
         *
         */

        POINTL      ptlPointer;
        // HPS         hps;

        // free old text
        if (pttd->pszPaintText)
        {
            free(pttd->pszPaintText);
            pttd->pszPaintText = NULL;
        }

        WinQueryPointerPos(HWND_DESKTOP, &ptlPointer);

        if (    (ptlPointer.x == pttd->ptlPointerLast.x)
             && (ptlPointer.y == pttd->ptlPointerLast.y)
           )
        {
            // mouse not moved since timer was started:
            // find the current TOOLINFO
            if (pttd->ptiMouseOver)
            {
                TOOLINFO    tiTemp;
                memcpy(&tiTemp, pttd->ptiMouseOver, sizeof(TOOLINFO));
                // get the text for the TOOLINFO
                WinSendMsg(hwndTooltip,
                           TTM_GETTEXT,
                           (MPARAM)0,
                           (MPARAM)&tiTemp);
                if (tiTemp.pszText)
                    pttd->pszPaintText = strdup(tiTemp.pszText);
                else
                    pttd->pszPaintText = NULL;
            }

            if (pttd->pszPaintText)
            {
                FormatTooltip(hwndTooltip,
                              pttd,
                              &ptlPointer);

                // notify owner (TTN_SHOW)
                WinSendMsg(pttd->hwndOwner,
                           WM_CONTROL,
                           MPFROM2SHORT(pttd->usTooltipID,  // tooltip control wnd ID
                                        TTN_SHOW),
                           pttd->ptiMouseOver);

                WinShowWindow(hwndTooltip, TRUE);
                pttd->fIsVisible = TRUE;

                // start autopop timer
                pttd->idTimerAutopop = WinStartTimer(pttd->hab,
                                                     hwndTooltip,
                                                     TOOLTIP_ID_TIMER_AUTOPOP,
                                                     pttd->ulTimeoutAutopop);
            } // end if (pttd->pszPaintText)
        } // end if (    (ptlPointer.x == pttd->ptlPointerLast.x)...
    } // end if (mp1)
    else
    {
        /*
         * hide tooltip::
         *
         */

        if (pttd->fIsVisible)
        {
            // notify owner (TTN_POP)
            WinSendMsg(pttd->hwndOwner,
                       WM_CONTROL,
                       MPFROM2SHORT(pttd->usTooltipID,  // tooltip control wnd ID
                                    TTN_POP),
                       pttd->ptiMouseOver);
            WinShowWindow(hwndTooltip, FALSE);
        }

        // stop autopop timer
        if (pttd->idTimerAutopop)
        {
            WinStopTimer(pttd->hab,
                         hwndTooltip,
                         TOOLTIP_ID_TIMER_AUTOPOP);
            pttd->idTimerAutopop = 0;
        }
    }

    // store new visibility
    pttd->fIsVisible = fShow;
}

/*
 *@@ ctl_fnwpTooltip:
 *      window procedure for the "tooltip" control. This control is
 *      largely source-code compatible to the Win32 version and has
 *      been modelled according to the Win32 programmer's reference.
 *
 *      A tooltip control is a small pop-up window that displays a
 *      single line of descriptive text giving the purpose of "tools"
 *      in an application.
 *      A "tool" is either a window, such as a child window or control,
 *      or an application-defined rectangular area within a window.
 *      See the TTM_ADDTOOL message for details.
 *
 *      To clarify: There is usually one tooltip control, which is hidden
 *      most of the time, for many "tools" (parts of a visible window).
 *      When the user puts the cursor on a tool and leaves it there for
 *      approximately one-half second, the tooltip control is set up for
 *      that tool and made visible. The tooltip control appears near the
 *      pointer and disappears when the user clicks a mouse button or moves
 *      the pointer off of the tool.
 *
 *      The Win32 concept has been extended with this implementation to
 *      display even longer texts, including line breaks. The tooltip
 *      window is formatted accordingly.
 *
 *      All default window styles are ignored when you create the tooltip,
 *      but will rather be set by this window proc automatically. The
 *      only valid window styles are the TTS_* flags (see notes below).
 *
 *      Presentation parameters are fully supported.
 *
 *      To create a tooltip control using comctl.c, use
 +          ctlRegisterTooltip(hab);
 +          hwndTooltip = WinCreateWindow(HWND_DESKTOP, // parent
 +                           WC_CCTL_TOOLTIP, // wnd class (comctl.h)
 +                           NULL,          // window text
 +                           TTS_ALWAYSTIP, // window style, ignored except for TTS_* flags
 +                           0, 0, 0, 0,    // window pos and size, ignored
 +                           hwndOwner,     // owner window -- important!
 +                           NULLHANDLE,    // hwndInsertBehind, ignored
 +                           ulID,          // window ID, optional
 +                           NULL,          // control data
 +                           NULL);         // presparams
 +
 *      ctl_fnwpTooltip automatically sets the size, position, and visibility
 *      of the tooltip control. The size of the tooltip window will vary
 *      depending on the text to be displayed and on the font presentation
 *      parameters, which are supported by this control (OS/2 only).
 *
 *      Note: OS/2 normally does not destroy windows which are only owned by
 *      another window. As a result, when the tooltip's owner is destroyed,
 *      you must explicitly also destroy the tooltip window.
 *
 *      Under both Win32 and OS/2, a tooltip control has two class-specific styles:
 *
 *      -- TTS_ALWAYSTIP: the tooltip appears when the cursor is on a tool,
 *          regardless of whether the tooltip control's owner window is active
 *          or inactive. Without this style, the tooltip control appears when the
 *          tool's owner window is active, but not when it is inactive.
 *
 *      -- TTS_NOPREFIX: this prevents the system from stripping the ampersand (&)
 *          character from a string. If a tooltip control does not have the TTS_NOPREFIX
 *          style, the system automatically strips ampersand characters, allowing an
 *          application to use the same string as both a menu item and tooltip text.
 *
 *      The tooltip control can be (de)activated using the TTM_ACTIVATE message.
 *
 *      To register tools with the tooltip control (to make it do anything
 *      meaningful), send the TTM_ADDTOOL message to the tooltip control.
 *
 *      This control supports the following presentation parameters (if the
 *      PP_* value is not found, the SYSCLR_* system color is used):
 *      --  PP_MENUBACKGROUNDCOLOR / SYSCLR_ENTRYFIELD: tooltip background color.
 *      --  PP_MENUFOREGROUNDCOLOR / SYSCLR_WINDOWTEXT: tooltip text color.
 *      --  PP_MENUHILITEFGNDCOLOR / SYSCLR_WINDOWTEXT: 3D border light color.
 *      --  PP_MENUHILITEBGNDCOLOR / SYSCLR_WINDOWTEXT: 3D border dark color.
 *      --  PP_FONTNAMESIZE: font to use for the tooltip. The default is "8.Helv"
 *              on Warp 3 and "9.WarpSans" on Warp 4.
 *
 *      So per default, if no presentation parameters are set, the control
 *      paints the background like an entry field and the text and the border
 *      in the same window text color (usually black).
 *      The tooltip does _not_ inherit presentation parameters from the parent,
 *      so unless you explicitly set presparams, the tooltip looks pretty much
 *      like the Win32 counterpart (with the default system colors, black border
 *      and text on yellow background).
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.12 (2001-04-28) [umoeller]: various fixes WRT subclassing
 */

MRESULT EXPENTRY ctl_fnwpTooltip(HWND hwndTooltip, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    TRY_LOUD(excpt1)
    {
        switch (msg)
        {
            /*
             * WM_CREATE:
             *      this message is sent to the window procedure of
             *      the window being created, thus offering it an
             *      opportunity to initialize that window.
             *
             *      The window procedure receives this after the window
             *      is created but before the window becomes visible.
             */

            case WM_CREATE:
                mrc = TtmCreate(hwndTooltip, mp2);
            break;

            /*
             * WM_HITTEST:
             *      since we have the CS_HITTEST class style set,
             *      we get this message. We return HT_TRANSPARENT
             *      always so the tooltip does not block mouse clicks
             *      for the windows which lie below.
             */

            case WM_HITTEST:    // done
                mrc = (MPARAM)HT_TRANSPARENT;
            break;

            /*
             * WM_TIMER:
             *      posted when either the "initial" timer
             *      or the "autopop" timer times out.
             *      We'll show or hide the tooltip then.
             */

            case WM_TIMER:      // done
                if (!TtmTimer(hwndTooltip, mp1))
                    mrc = WinDefWindowProc(hwndTooltip, msg, mp1, mp2);
            break;

            /*
             * WM_PAINT:
             *
             */

            case WM_PAINT:      // done
                TtmPaint(hwndTooltip);
            break;

            /*
             * WM_PRESPARAMCHANGED:
             *      presentation parameter has changed.
             */

            case WM_PRESPARAMCHANGED:

                switch ((LONG)mp1)      // pp index
                {
                    case 0:     // layout palette thing dropped
                    case PP_MENUBACKGROUNDCOLOR:
                    case PP_MENUFOREGROUNDCOLOR:
                    case PP_MENUHILITEFGNDCOLOR:
                    case PP_MENUHILITEBGNDCOLOR:
                        // re-query our presparams
                        UpdateTooltipPresColors(hwndTooltip);
                }

            break;

            /*
             * WM_SYSCOLORCHANGE:
             *      system color has changed.
             */

            case WM_SYSCOLORCHANGE:
                UpdateTooltipPresColors(hwndTooltip);
            break;

            /*
             * WM_DESTROY:
             *      clean up upon destruction.
             */

            case WM_DESTROY:
                TtmDestroy(hwndTooltip);
            break;

           /*
            *@@ TTM_ACTIVATE:
            *      activates or deactives the tooltip control.
            *
            *      Parameters:
            *      -- BOOL mp1: if TRUE, activate, if FALSE, deactivate.
            *      -- mp2: always 0.
            *
            *      Return value: 0 always.
            */

            case TTM_ACTIVATE:      // done
            {
                PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);
                if (!(pttd->fIsActive = (BOOL)mp1))
                    // disable: hide tooltip
                    WinPostMsg(hwndTooltip, TTM_SHOWTOOLTIPNOW, (MPARAM)FALSE, 0);
            }
            break;

            /*
             *@@ TTM_ADDTOOL:
             *      registers a tool with a tooltip control.
             *
             *      Parameters:
             *      --  mp1: always 0.
             *      --  PTOOLINFO mp2: information about the tool.
             *
             *      Return value: TRUE if successful or FALSE otherwise.
             *
             *      A tooltip control can support any number of tools. To
             *      support a particular tool, you must register the tool
             *      with the tooltip control by sending the TTM_ADDTOOL
             *      message to the tooltip control. The message includes
             *      the address of a TOOLINFO structure, which provides
             *      information the tooltip control needs to display text
             *      for the tool.
             *
             *      A tooltip control supports tools implemented as windows
             *      (such as child windows or control windows) and as
             *      rectangular areas within a window's client area.
             *
             *      --  When you add a tool implemented as a rectangular
             *          area, the "hwndToolOwner" member of TOOLINFO must
             *          specify the handle of the window that contains the
             *          area, and the "rect" member must specify the client
             *          coordinates of the area's bounding rectangle.
             *
             *      --  When you add a tool implemented as a window, the
             *          "hwndTool" member of TOOLINFO must contain the
             *          window handle of the tool. hwndToolOwner should be
             *          the owner of the tool.
             *
             *      When you add a tool to a tooltip control, the "pszText"
             *      member of the TOOLINFO structure must specify the string
             *      to display for the tool. You can change the text any time
             *      after adding the tool by using the TTM_UPDATETIPTEXT message.
             *
             *      If you specify the PSZ_TEXTCALLBACK value in the pszText
             *      member, whenever the tooltip control needs the text for the
             *      tool, it sends a WM_CONTROL message to hwndToolOwner with
             *      the TTN_NEEDTEXT notification code.
             *
             *      The message includes the address of a TOOLTIPTEXT structure,
             *      which contains the window handle of the tool. You can then
             *      fill the TOOLTIPTEXT structure with the tool text.
             *
             *      To retrieve the text for a tool, use the TTM_GETTEXT message.
             *
             *@@todo: add tool rectangles
             */

            case TTM_ADDTOOL:       // done
                mrc = TtmAddTool(hwndTooltip, mp2);
            break;

            /*
             *@@ TTM_DELTOOL:
             *      removes a tool from a tooltip control.
             *
             *      Parameters:
             *      --  mp1: always 0.
             *      --  PTOOLINFO mp2: information about the tool;
             *              all fields except cbSize, hwndToolOwner,
             *              and hwndTool are ignored.
             *
             *      Return value: 0 always.
             */

            case TTM_DELTOOL:       // done
                TtmDelTool(hwndTooltip, mp2);
            break;

            /*
             *@@ TTM_NEWTOOLRECT:
             *      sets a new bounding rectangle for a tool
             *      already registered with a tooltip control.
             *
             *      Parameters:
             *      -- mp1: always 0.
             *      -- PTOOLINFO mp2: information about the tool;
             *              all fields except hwnd, uID, and rect
             *              are ignored.
             *
             *      Return value: 0 always.
             *
             *      If an application includes a tool implemented as a rectangular
             *      area and the size or position of the control changes, it can
             *      use the TTM_NEWTOOLRECT message to report the change to the
             *      tooltip control. An application does not need to report size
             *      and position changes for a tool implemented as a window.
             *      Reporting that is not necessary because the tooltip control
             *      uses the window handle of a tool to determine if the cursor
             *      is on the tool, not the tool's bounding rectangle.
             */

            case TTM_NEWTOOLRECT:

            break;

            /*
             *@@ TTM_RELAYEVENT:
             *      passes a mouse message to a tooltip control
             *      for further processing.
             *
             *      Parameters:
             *      -- mp1: always 0.
             *      -- PQMSG mp2: pointer to a QMSG struct (Win95: MSG struct)
             *          containing a mouse message. Only the above messages
             *          are processed.
             *
             *      Return value: 0 always.
             *
             *      A tooltip control needs to receive mouse messages to determine
             *      when to display the tooltip window. Because mouse messages are
             *      sent only to the window that contains the cursor, you must
             *      use the TTM_RELAYEVENT message to relay mouse messages to the
             *      tooltip control.
             *
             *      If a tool is implemented as a rectangular area in an
             *      application-defined window, the window procedure receives all
             *      mouse messages and can relay the ones listed below to the
             *      tooltip control, using this message.
             *
             *      However, if a tool is implemented as a system-defined window,
             *      the mouse messages are sent to that window and are not readily
             *      available to the application. There are two ways you can have
             *      the tooltip control notified of these mouse messages:
             *
             *      -- You can specify the TTF_SUBCLASS flag when adding the tool
             *         to the tooltip control. The tooltip control will then subclass
             *         the tool window to get mouse-related messages, and you're off.
             *
             *      -- You can implement a message hook for your process and check
             *         for your tool windows there and send TTM_RELAYEVENT to the
             *         tooltip control. In that case, you need to intercept the
             *         messages specified below.
             *
             *      When a tooltip control receives a relayed WM_MOUSEMOVE message,
             *      it determines whether the cursor is in the bounding rectangle of
             *      a tool. If the cursor is there, the tooltip control sets a timer.
             *      At the end of the time-out duration, the tooltip control checks
             *      the position of the cursor to see whether it has moved. If the
             *      cursor has not, the tooltip control retrieves the text for the tool,
             *      copies the text into the tooltip window, and shows the window.
             *      The tooltip control continues to show the window until it receives
             *      a relayed button-up or button-down message or until a relayed
             *      WM_MOUSEMOVE message indicates that the cursor has moved outside
             *      the bounding rectangle of the tool.
             *
             *      For the timer descriptions, see TTM_SETDELAYTIME.
             *
             *      When it is about to be displayed, a tootip control sends the
             *      TTN_SHOW notification to the owner window. A tooltip control
             *      sends the TTN_POP notification when it is about to be hidden.
             *      Each notification is sent in the context of a WM_NOTIFY message
             *      (OS/2: WM_CONTROL).
             *
             *      Relevant messages:
             *      --  WM_MOUSEMOVE
             *      --  WM_BUTTON1DOWN  (Win95: WM_LBUTTONDOWN)
             *      --  WM_BUTTON1UP    (Win95: WM_LBUTTONUP)
             *      --  WM_BUTTON2DOWN  (Win95: WM_RBUTTONDOWN)
             *      --  WM_BUTTON2UP    (Win95: WM_RBUTTONUP)
             *      --  WM_BUTTON3DOWN  (Win95: WM_MBUTTONDOWN)
             *      --  WM_BUTTON3UP    (Win95: WM_MBUTTONUP)
             */

            case TTM_RELAYEVENT:
                TtmRelayEvent(hwndTooltip, mp2);
            break;

            /*
             *@@ TTM_GETDELAYTIME:
             *      returns the current value of the specified
             *      timeout value. See TTM_SETDELAYTIME.
             *
             *      Parameters:
             *
             *      -- USHORT mp1: timer value to query. One of:
             *              -- TTDT_AUTOPOP
             *              -- TTDT_INITIAL
             *              -- TTDT_RESHOW
             *
             *      Returns: ULONG timeout value.
             *
             *@@added V0.9.12 (2001-04-28) [umoeller]
             */

            case TTM_GETDELAYTIME:
                mrc = TtmGetDelayTime(hwndTooltip, mp1);
            break;

            /*
             *@@ TTM_SETDELAYTIME:
             *      overrides a few default timeout values for the
             *      tooltip control.
             *
             *      A tooltip control actually has three time-out durations
             *      associated with it. The initial duration is the length
             *      of time that the cursor must remain stationary within
             *      the bounding rectangle of a tool before the tooltip window
             *      is displayed. The reshow duration is the length of the delay
             *      before subsequent tooltip windows are displayed when the
             *      cursor moves from one tool to another. The autopopup duration
             *      is the length of time that the tooltip window remains
             *      displayed before it is hidden. That is, if the cursor remains
             *      stationary within the bounding rectangle after the tooltip
             *      window is displayed, the tooltip window is automatically
             *      hidden at the end of the autopopup duration.
             *
             *      You can adjust all of the time-out durations by using this
             *      message.
             *
             *      Parameters:
             *      -- USHORT mp1: parameter selection. One of the following:
             *              -- TTDT_AUTOMATIC: automatically calculates the initial,
             *                 reshow, and autopopup durations based on the value of mp2.
             *              -- TTDT_AUTOPOP: sets the length of time before the tooltip
             *                  window is hidden if the cursor remains stationary
             *                  in the tool's bounding rectangle after the tooltip window
             *                  has appeared.
             *              -- TTDT_INITIAL: sets the length of time that the cursor must
             *                  remain stationary within the bounding rectangle of
             *                  a tool before the tooltip window is displayed.
             *              -- TTDT_RESHOW: sets the length of the delay before subsequent
             *                  tooltip windows are displayed when the cursor is moved
             *                  from one tool to another.
             *      -- ULONG mp2: new duration, in milliseconds.
             *
             *      Return value: 0 always.
             */

            case TTM_SETDELAYTIME:  // done
                TtmSetDelayTime(hwndTooltip, mp1, mp2);
            break;

            /*
             *@@ TTM_GETTEXT:
             *      retrieves the text for a tool in the tooltip control.
             *
             *      Parameters:
             *      -- mp1: always 0
             *      -- PTOOLINFO mp2: pointer to a TOOLINFO structure.
             *         hwndTool identifies a tool. If the tooltip control
             *         includes the tool, the pszText member receives
             *         the pointer to the string on output.
             *
             *      Return value: 0 always.
             *
             *      Additional note: On input, if TOOLINFO.lpszText == PSZ_TEXTCALLBACK,
             *      this sends the TTN_NEEDTEXT notification to TOOLINFO.hwnd.
             *
             */

            case TTM_GETTEXT:       // done, I think
                TtmGetText(hwndTooltip, mp2);
            break;

            /*
             *@@ TTM_UPDATETIPTEXT:
             *      sets the current tooltip text explicitly,
             *      no matter for what tool the tooltip is
             *      currently being displayed. This might be
             *      useful if you want to dynamically update
             *      the tooltip display.
             *
             *      If the tooltip is currently showing,
             *      it is reformatted with the new text.
             *
             *      Note that the change is not permanent;
             *      if the mouse moves over a different
             *      tool next, the change will be lost.
             *
             *      Parameters:
             *      -- PSZ mp1: new text to display.
             *
             *@@added V0.9.13 (2001-06-21) [umoeller]
             */

            case TTM_UPDATETIPTEXT:
                TtmUpdateTipText(hwndTooltip, (PSZ)mp1);
            break;

            /*
             *@@ TTM_HITTEST:
             *      tests a point to determine whether it is within the
             *      bounding rectangle of the specified tool and, if the
             *      point is within, retrieves information about the tool.
             *
             *      Parameters:
             *      -- mp1: always 0.
             *      -- PHITTESTINFO mp2: pointer to a TTHITTESTINFO structure.
             *          When sending the message, the "hwnd" member must specify
             *          the handle of a tool and the "pt" member must specify the
             *          coordinates of a point. If the return value is TRUE, the
             *          "ti" member receives information about the tool that
             *          occupies the point.
             *
             *      Return value: returns TRUE if the tool occupies the specified
             *      point or FALSE otherwise.
             */

            case TTM_HITTEST:
            break;

            /*
             *@@ TTM_WINDOWFROMPOINT:
             *      this message allows a subclass procedure to cause a tooltip
             *      to display text for a window other than the one beneath the
             *      mouse cursor.
             *
             *      Parameters:
             *      -- mp1: always 0.
             *      -- mp2: PPOINTL lppt (Win95: (POINT FAR *)lppt):
             *          pointer to a POINTL structure that defines the point to be checked.
             *
             *      Return value: the handle to the window that contains the point, or
             *      NULL if no window exists at the specified point.
             *
             *      This message is intended to be processed by an application that
             *      subclasses a tooltip. It is not intended to be sent by an
             *      application. A tooltip sends this message to itself before
             *      displaying the text for a window. By changing the coordinates
             *      of the point specified by lppt, the subclass procedure can cause
             *      the tooltip to display text for a window other than the one
             *      beneath the mouse cursor.
             */

            case TTM_WINDOWFROMPOINT:
            break;

            /*
             *@@ TTM_ENUMTOOLS:
             *      this message retrieves the information that a tooltip control
             *      maintains about a certain tool.
             *
             *      Parameters:
             *      -- USHORT mp1: zero-based index of the tool for which to
             *         retrieve information.
             *      -- PTOOLINFO mp2: pointer to a TOOLINFO structure that
             *         receives information about the tool. Before sending
             *         this message, the cbSize member must specify the size
             *         of the structure.
             *
             *      Return value: TRUE if any tools are enumerated or FALSE otherwise.
             */

            case TTM_ENUMTOOLS:         // done
                mrc = TtmEnumTools(hwndTooltip, mp1, mp2);
            break;

            /*
             *@@ TTM_GETCURRENTTOOL:
             *      this message retrieves the information that the
             *      tooltip control maintains for the _current_ tool;
             *      that is, the one for which the tooltip control
             *      is currently displaying text.
             *
             *      Parameters:
             *      -- mp1: always 0.
             *      -- PTOOLINFO mp2: pointer to a TOOLINFO structure that receives
             *          information about the current tool.
             *
             *      Return value: TRUE if successful or FALSE otherwise.
             */

            case TTM_GETCURRENTTOOL:        // done
                mrc = TtmGetCurrentTool(hwndTooltip, mp2);
            break;

            /*
             *@@ TTM_GETTOOLCOUNT:
             *      returns the number of tools in the tooltip control.
             *
             *      No parameters.
             */

            case TTM_GETTOOLCOUNT:          // done
            {
                PTOOLTIPDATA pttd = (PTOOLTIPDATA)WinQueryWindowPtr(hwndTooltip, 1);
                mrc = (MPARAM)lstCountItems(&pttd->llTools);
            }
            break;

            /*
             *@@ TTM_GETTOOLINFO:
             *      this message retrieves the information that a tooltip
             *      control maintains about a tool.
             *
             *      Parameters:
             *      -- mp1: always 0.
             *      -- PTOOLINFO mp2:
             *          pointer to a TOOLINFO structure. When sending the message,
             *          the hwnd and uId members identify a tool, and the cbSize
             *          member must specify the size of the structure. If the
             *          tooltip control includes the tool, the structure receives
             *          information about the tool.
             *
             *      Return value: TRUE if successful or FALSE otherwise.
             */

            case TTM_GETTOOLINFO:       // done
                mrc = TtmGetToolInfo(hwndTooltip, mp2);
            break;

            /*
             *@@ TTM_SETTOOLINFO:
             *      this message sets the information that a tooltip control
             *      maintains for a tool.
             *
             *      Parameters:
             *      -- mp1: always 0.
             *      -- PTOOLINFO mp2: pointer to a TOOLINFO structure that
             *          specifies the information to set.
             *
             *      Return value: 0 always.
             */

            case TTM_SETTOOLINFO:
            break;

            /*
             *@@ TTM_SHOWTOOLTIPNOW:
             *      depending on BOOL mp1, shows or hides the tooltip.
             *      This is required because we cannot show or hide
             *      the window during processing of WM_MOUSEMOVE etc,
             *      which will lead to strange PM hangs.
             *
             *      This is not part of the Win95 message set but used
             *      in the OS/2 implementation only. This calls
             *      TtmShowTooltip in turn.
             */

            case TTM_SHOWTOOLTIPNOW:
                TtmShowTooltip(hwndTooltip, (BOOL)mp1);
            break;

            default:
                mrc = WinDefWindowProc(hwndTooltip, msg, mp1, mp2);
        }
    }
    CATCH(excpt1)
    {
        mrc = 0;        // XWP V1.0.4 (2005-10-09) [pr]
    }
    END_CATCH();

    return mrc;
}

