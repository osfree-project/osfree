
/*
 *@@sourcefile pager.c:
 *      daemon/hook interface for XPager.
 *
 *      Function prefix for this file:
 *      --  pfmi*: XPager interface.
 *
 *      This file is ALL new with V0.9.0.
 *
 *@@added V0.9.3 (2000-04-08) [umoeller]
 *@@header "config\pager.h"
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
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

#define INCL_DOSSEMAPHORES
#define INCL_WINSHELLDATA       // Prf* functions
#define INCL_WINWINDOWMGR
#define INCL_WININPUT
#define INCL_WINPOINTERS
#define INCL_WINSYS
#define INCL_WINMENUS
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINLISTBOXES
#define INCL_WINENTRYFIELDS
#define INCL_WINSTDCNR
#define INCL_WINSTDSLIDER
#define INCL_WINSTDSPIN
#define INCL_WINSTDVALSET
#define INCL_WINSWITCHLIST
#define INCL_GPILOGCOLORTABLE
#include <os2.h>

// C library headers
#include <stdio.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\regexp.h"             // extended regular expressions
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\winh.h"               // PM helper routines

// SOM headers which don't crash with prec. header files
#include "xwpscreen.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\notebook.h"            // generic XWorkplace notebook handling

// headers in /hook
#include "hook\xwphook.h"

#include "config\hookintf.h"            // daemon/hook interface
#include "config\pager.h"               // XPager interface

#pragma hdrstop

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

static PFNWP   G_pfnwpOrigStatic = NULL;

/* ******************************************************************
 *
 *   XPager (XWPScreen) helpers
 *
 ********************************************************************/

#ifndef __NOPAGER__

/*
 *@@ LoadPagerConfig:
 *
 *@@added V0.9.3 (2000-05-21) [umoeller]
 */

STATIC BOOL LoadPagerConfig(PAGERCONFIG* pPagerCfg)
{
    ULONG cb = sizeof(PAGERCONFIG);
    memset(pPagerCfg, 0, sizeof(PAGERCONFIG));
    // overwrite from INI, if found
    return PrfQueryProfileData(HINI_USER,
                               INIAPP_XWPHOOK,
                               INIKEY_HOOK_PAGERCONFIG,
                               pPagerCfg,
                               &cb);
}

/*
 *@@ SavePagerConfig:
 *      writes the given pager configuration struct
 *      back to OS2.INI and notifies the pager of
 *      the change, passing it the given PGRCFG_*
 *      flags to optimize repaint.
 *
 *      Here we post T1M_PAGERCONFIGDELAYED to
 *      the kernel thread-1 object, which will again
 *      send XDM_PAGERCONFIG to the daemon.
 *
 *@@added V0.9.3 (2000-04-09) [umoeller]
 */

STATIC VOID SavePagerConfig(PAGERCONFIG* pPagerCfg,
                            ULONG ulFlags)  // in: PGRCFG_* flags (xwphook.h)
{
    // settings changed:
    // 1) write back to OS2.INI
    if (PrfWriteProfileData(HINI_USER,
                            INIAPP_XWPHOOK,
                            INIKEY_HOOK_PAGERCONFIG,
                            pPagerCfg,
                            sizeof(PAGERCONFIG)))
    {
        // 2) notify daemon, but do this delayed, because
        //    page mage config changes may overload the
        //    system; the thread-1 object window starts
        //    a timer for this...
        //    after that, the daemon sends XDM_PAGERCONFIG
        //    to the daemon.
        krnPostThread1ObjectMsg(T1M_PAGERCONFIGDELAYED,
                                (MPARAM)ulFlags,
                                0);
    }
}

#endif

/*
 *@@ EnableNotebookButtons:
 *      enables or disables the "Default" and
 *      "Undo" notebook buttons. This is a bit
 *      tricky because on Warp 4, these are no
 *      longer children of the dialog page, but
 *      of the notebook control itself.
 *
 *      PMTREE shows me that all notebook buttons
 *      from all pages that have been loaded
 *      become children of the notebook control,
 *      so there may be several buttons with
 *      a style of DID_UNDO and DID_DEFAULT. So
 *      we must also check whether they're visible.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

STATIC VOID EnableNotebookButtons(PNOTEBOOKPAGE pnbp,
                                  BOOL fEnable)
{
    HWND    hwndThis,
            hwndNotebook = G_fIsWarp4
                            ? pnbp->inbp.hwndNotebook
                            : pnbp->hwndDlgPage;

    HENUM henum1 = WinBeginEnumWindows(hwndNotebook);
    while ((hwndThis = WinGetNextWindow(henum1)) != NULLHANDLE)
    {
        if (WinIsWindowVisible(hwndThis))
            switch (WinQueryWindowUShort(hwndThis, QWS_ID))
            {
                case DID_UNDO:
                case DID_DEFAULT:
                    WinEnableWindow(hwndThis, fEnable);
            }
    }
    WinEndEnumWindows(henum1);
}

/* ******************************************************************
 *
 *   XPager General page notebook functions (notebook.c)
 *
 ********************************************************************/

#ifndef __NOPAGER__

/*
 *@@ UpdateValueSet:
 *      called from PagerGeneralInitPage et al. to recreate
 *      the value set control with the proper virtual
 *      desktops dimensions.
 *
 *      The value set is initially created as a static by
 *      the dlg formatter, but then recreated immediately
 *      (and every time the dimensions change).
 *
 *@@added V0.9.9 (2001-03-15) [lafaix]
 *@@changed V0.9.19 (2002-05-07) [umoeller]: rewritten to display dimensions correctly
 */

STATIC VOID UpdateValueSet(HWND hwndPage,
                           PAGERCONFIG *pPagerCfg)
{
    // the stupid value set control only accepts the
    // rows and columns count on creation via the
    // control data, so we have no choice but to
    // destroy and recreate the value set whenever
    // the rows/columns change
    // V0.9.19 (2002-05-07) [umoeller]
    HWND hwndValueSet = WinWindowFromID(hwndPage,
                                        ID_SCDI_PGR1_VALUESET);

    SWP     swp;
    VSCDATA cd;
    WinQueryWindowPos(hwndValueSet, &swp);
    WinDestroyWindow(hwndValueSet);

    cd.cbSize = sizeof(cd);
    cd.usRowCount = pPagerCfg->cDesktopsY;
    cd.usColumnCount = pPagerCfg->cDesktopsX;

    if (hwndValueSet = WinCreateWindow(hwndPage,
                                       WC_VALUESET,
                                       "",
                                       WS_VISIBLE | VS_RGB | VS_BORDER,
                                       swp.x,
                                       swp.y,
                                       swp.cx,
                                       swp.cy,
                                       hwndPage,        // owner
                                       swp.hwndInsertBehind,
                                       ID_SCDI_PGR1_VALUESET,
                                       &cd,
                                       NULL))
   {
       int  row, col;

       for (row = 1;
            row <= pPagerCfg->cDesktopsY;
            ++row)
       {
           BOOL fCurrentRow = (pPagerCfg->cDesktopsY - row + 1) == pPagerCfg->bStartY;

           for (col = 1;
                col <= pPagerCfg->cDesktopsX;
                ++col)
           {
               BOOL fCurrent = (    (fCurrentRow)
                                 && (col == pPagerCfg->bStartX)
                               );
               WinSendMsg(hwndValueSet,
                          VM_SETITEM,
                          MPFROM2SHORT(row,
                                       col),
                          // highlight startup desktop
                          MPFROMLONG(   fCurrent
                                        ? pPagerCfg->lcolActiveDesktop
                                        : pPagerCfg->lcolDesktop1
                                     ));

               if (fCurrent)
                   WinSendMsg(hwndValueSet,
                              VM_SELECTITEM,
                              MPFROM2SHORT(row, col),
                              0);
           }
       }
    }
}

SLDCDATA
        Pgr1SliderCData =
             {
                     sizeof(SLDCDATA),
            // usScale1Increments:
                     10,        // scale 1 increments
                     0,         // scale 1 spacing
                     1,         // scale 2 increments
                     0          // scale 2 spacing
             };

#define SLIDER_CX           100
#define SLIDER_CY           50
#define SLIDER_WIDTH        14
#define VALUESET_WIDTH      (SLIDER_CX + SLIDER_WIDTH + 2 * COMMON_SPACING)
#define PAGE_WIDTH          (VALUESET_WIDTH + SLIDER_WIDTH + 4 * COMMON_SPACING)
            // used by second page

static const CONTROLDEF
    Pgr1Group = LOADDEF_GROUP(ID_SCDI_PGR1_GROUP, SZL_AUTOSIZE),
    Pgr1Enable = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_ENABLE),
    Pgr1Spacing = CONTROLDEF_TEXT("", -1, SLIDER_WIDTH, SLIDER_WIDTH),
    Pgr1XSlider = CONTROLDEF_SLIDER(ID_SCDI_PGR1_X_SLIDER,
                                    SLIDER_CX,
                                    SLIDER_WIDTH,
                                    &Pgr1SliderCData),
    Pgr1XSliderTxt = CONTROLDEF_TEXT_CENTER("",
                                    ID_SCDI_PGR1_X_TEXT2,
                                    SLIDER_WIDTH,
                                    SLIDER_WIDTH),
    Pgr1YSlider = CONTROLDEF_VSLIDER(
                                    ID_SCDI_PGR1_Y_SLIDER,
                                    SLIDER_WIDTH,
                                    SLIDER_CY,
                                    &Pgr1SliderCData),
    Pgr1YSliderTxt = CONTROLDEF_TEXT_CENTER("",
                                    ID_SCDI_PGR1_Y_TEXT2,
                                    SLIDER_WIDTH,
                                    SLIDER_WIDTH),
    // value set will be replaced
    Pgr1ValueSet = CONTROLDEF_TEXT("", ID_SCDI_PGR1_VALUESET,
                                   VALUESET_WIDTH,
                                   SLIDER_CY + SLIDER_WIDTH + 2 * COMMON_SPACING),
    Pgr1FollowFocusCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_FOLLOWFOCUS),
    Pgr1ArrowHotkeysCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_ARROWHOTKEYS),
    Pgr1HotkeysCtrlCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_HOTKEYS_CTRL),
    Pgr1HotkeysShiftCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_HOTKEYS_SHIFT),
    Pgr1HotkeysAltCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_HOTKEYS_ALT),
    Pgr1WindowsKeysCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_WINDOWS_KEYS),
    Pgr1WraparoundCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_WRAPAROUND);

static const DLGHITEM G_dlgPagerGeneral[] =
    {
        START_TABLE,
            START_ROW(0),
                CONTROL_DEF(&Pgr1Enable),
            START_ROW(0),
                START_GROUP_TABLE(&Pgr1Group),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&Pgr1Spacing),
                        CONTROL_DEF(&Pgr1XSliderTxt),
                        CONTROL_DEF(&Pgr1XSlider),
                    START_ROW(0),
                        START_TABLE,
                            START_ROW(0),
                                CONTROL_DEF(&Pgr1YSliderTxt),
                            START_ROW(0),
                                CONTROL_DEF(&Pgr1YSlider),
                        END_TABLE,
                        CONTROL_DEF(&Pgr1ValueSet),
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&Pgr1FollowFocusCB),
            START_ROW(0),
                CONTROL_DEF(&Pgr1ArrowHotkeysCB),
            START_ROW(0),
                CONTROL_DEF(&G_Spacing),
                CONTROL_DEF(&Pgr1HotkeysCtrlCB),
                CONTROL_DEF(&Pgr1HotkeysShiftCB),
                CONTROL_DEF(&Pgr1HotkeysAltCB),
            START_ROW(0),
                CONTROL_DEF(&Pgr1WindowsKeysCB),
            START_ROW(0),
                CONTROL_DEF(&Pgr1WraparoundCB),
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };

/*
 *@@ PAGERPAGEDATA:
 *      backup data for PagerGeneralInitPage.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

typedef struct _PAGERPAGEDATA
{
    BOOL            fEnableXPager;      // general page only
    PAGERCONFIG     PgrConfig;          // all pages
    HWND            hwndColorDlg;       // colors page only
} PAGERPAGEDATA, *PPAGERPAGEDATA;

/*
 *@@ PagerGeneralInitPage:
 *      notebook callback function (notebook.c) for the
 *      first "XPager" page in the "Screen" settings object.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@changed V0.9.4 (2000-07-11) [umoeller]: fixed window flashing
 *@@changed V0.9.4 (2000-07-11) [umoeller]: added window flashing delay
 *@@changed V0.9.9 (2001-03-15) [lafaix]: "window" part moved to PagerWindowInitPage
 *@@changed V0.9.19 (2002-05-07) [umoeller]: adjusted for pager rework
 *@@changed V0.9.19 (2002-05-28) [umoeller]: now using dialog formatter
 *@@changed V0.9.19 (2002-06-02) [umoeller]: added "desktop follows focus" setting
 */

STATIC VOID PagerGeneralInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                                 ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        PPAGERPAGEDATA pBackup;

        // first call: create PAGERCONFIG
        // structure;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        if (pnbp->pUser = malloc(sizeof(PAGERCONFIG)))
            LoadPagerConfig(pnbp->pUser);

        // make backup for "undo"
        if (pnbp->pUser2 = pBackup = NEW(PAGERPAGEDATA))
        {
            pBackup->fEnableXPager = cmnQuerySetting(sfEnableXPager);
            memcpy(&pBackup->PgrConfig, pnbp->pUser, sizeof(PAGERCONFIG));
        }

        // insert the controls using the dialog formatter
        // V0.9.19 (2002-05-23) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      G_dlgPagerGeneral,
                      ARRAYITEMCOUNT(G_dlgPagerGeneral));

        winhSetSliderTicks(WinWindowFromID(pnbp->hwndDlgPage, ID_SCDI_PGR1_X_SLIDER),
                           (MPARAM)0, 3,
                           (MPARAM)-1, -1);
        winhSetSliderTicks(WinWindowFromID(pnbp->hwndDlgPage, ID_SCDI_PGR1_Y_SLIDER),
                           (MPARAM)0, 3,
                           (MPARAM)-1, -1);
    }

    if (flFlags & CBI_SET)
    {
        PAGERCONFIG* pPagerCfg = (PAGERCONFIG*)pnbp->pUser;

        // moved enable pager here from XWPSetup
        // V0.9.19 (2002-05-28) [umoeller]
        winhSetDlgItemChecked(pnbp->hwndDlgPage,
                              ID_SCDI_PGR1_ENABLE,
                              cmnQuerySetting(sfEnableXPager));

        // sliders
        winhSetSliderArmPosition(WinWindowFromID(pnbp->hwndDlgPage, ID_SCDI_PGR1_X_SLIDER),
                                 SMA_INCREMENTVALUE,
                                 pPagerCfg->cDesktopsX - 1);
        // Y slider has 10 positions (0-9) where 0 is top;
        // if we have position 0, we should have 10 desktops,
        // if we have position 9, we should have 1 desktop
        // V0.9.19 (2002-05-07) [umoeller]
        winhSetSliderArmPosition(WinWindowFromID(pnbp->hwndDlgPage, ID_SCDI_PGR1_Y_SLIDER),
                                 SMA_INCREMENTVALUE,
                                 10 - pPagerCfg->cDesktopsY);

        // valueset
        UpdateValueSet(pnbp->hwndDlgPage,
                       pPagerCfg);

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_FOLLOWFOCUS,
                              // this one is reverse
                              !(pPagerCfg->flPager & PGRFL_NOFOLLOWFOCUS));

        // hotkeys
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_ARROWHOTKEYS,
                              !!(pPagerCfg->flPager & PGRFL_HOTKEYS));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_HOTKEYS_CTRL,
                              !!(pPagerCfg->flKeyShift & KC_CTRL));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_HOTKEYS_SHIFT,
                              !!(pPagerCfg->flKeyShift & KC_SHIFT));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_HOTKEYS_ALT,
                              !!(pPagerCfg->flKeyShift & KC_ALT));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_WINDOWS_KEYS, // V1.0.3 (2004-10-14) [bird]
                             !!(pPagerCfg->flPager & PGRFL_WINDOWS_KEYS));

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_WRAPAROUND,
                              !!(pPagerCfg->flPager & PGRFL_WRAPAROUND));

    }

    if (flFlags & CBI_ENABLE)
    {
        static const ULONG
        aulIDsPager[] =
            {
                ID_SCDI_PGR1_GROUP,
                ID_SCDI_PGR1_X_SLIDER,
                ID_SCDI_PGR1_X_TEXT2,
                ID_SCDI_PGR1_Y_SLIDER,
                ID_SCDI_PGR1_Y_TEXT2,
                ID_SCDI_PGR1_VALUESET,

                ID_SCDI_PGR1_FOLLOWFOCUS,
                ID_SCDI_PGR1_ARROWHOTKEYS,
                ID_SCDI_PGR1_WINDOWS_KEYS,
                ID_SCDI_PGR1_WRAPAROUND
            },
        aulIDsHotkeys[] =
            {
                ID_SCDI_PGR1_HOTKEYS_CTRL,
                ID_SCDI_PGR1_HOTKEYS_SHIFT,
                ID_SCDI_PGR1_HOTKEYS_ALT,
            };

        BOOL    fPager = cmnQuerySetting(sfEnableXPager);

        EnableNotebookButtons(pnbp,
                              fPager);

        winhEnableControls2(pnbp->hwndDlgPage,
                            aulIDsPager,
                            ARRAYITEMCOUNT(aulIDsPager),
                            fPager);
        winhEnableControls2(pnbp->hwndDlgPage,
                            aulIDsHotkeys,
                            ARRAYITEMCOUNT(aulIDsHotkeys),
                               fPager
                            && winhIsDlgItemChecked(pnbp->hwndDlgPage,
                                                    ID_SCDI_PGR1_ARROWHOTKEYS));
    }
}

/*
 *@@ PagerGeneralItemChanged:
 *      notebook callback function (notebook.c) for the
 *      first "XPager" page in the "Screen" settings object.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@changed V0.9.4 (2000-07-11) [umoeller]: fixed window flashing
 *@@changed V0.9.4 (2000-07-11) [umoeller]: added window flashing delay
 *@@changed V0.9.9 (2001-03-15) [lafaix]: "window" part moved to PagerWindowItemChanged
 *@@changed V0.9.9 (2001-03-15) [lafaix]: fixed odd undo/default behavior
 */

STATIC MRESULT PagerGeneralItemChanged(PNOTEBOOKPAGE pnbp,
                                       ULONG ulItemID, USHORT usNotifyCode,
                                       ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = 0;
    BOOL    fSave = TRUE;      // save settings per default; this is set to FALSE if not needed
    ULONG   ulPgmgChangedFlags = 0;

    // access settings
    PAGERCONFIG* pPagerCfg = (PAGERCONFIG*)pnbp->pUser;

    switch (ulItemID)
    {
        case ID_SCDI_PGR1_ENABLE:
            hifEnableXPager(ulExtra);
            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
        break;

        case ID_SCDI_PGR1_X_SLIDER:
        {
            LONG lSliderIndex = winhQuerySliderArmPosition(pnbp->hwndControl,
                                                           SMA_INCREMENTVALUE);

            WinSetDlgItemShort(pnbp->hwndDlgPage,
                               ID_SCDI_PGR1_X_TEXT2,
                               lSliderIndex + 1,
                               FALSE);      // unsigned

            LoadPagerConfig(pnbp->pUser);
            pPagerCfg->cDesktopsX = lSliderIndex + 1;
            ulPgmgChangedFlags = PGRCFG_REPAINT | PGRCFG_REFORMAT;
            UpdateValueSet(pnbp->hwndDlgPage,
                           pPagerCfg);
        }
        break;

        case ID_SCDI_PGR1_Y_SLIDER:
        {
            // Y slider has 10 positions (0-9) where 0 is top;
            // if we have position 0, we should have 10 desktops,
            // if we have position 9, we should have 1 desktop
            // V0.9.19 (2002-05-07) [umoeller]
            LONG lSliderIndex = 10 - winhQuerySliderArmPosition(pnbp->hwndControl,
                                                                SMA_INCREMENTVALUE);

            WinSetDlgItemShort(pnbp->hwndDlgPage,
                               ID_SCDI_PGR1_Y_TEXT2,
                               lSliderIndex,
                               FALSE);      // unsigned

            LoadPagerConfig(pnbp->pUser);
            pPagerCfg->cDesktopsY = lSliderIndex;
            ulPgmgChangedFlags = PGRCFG_REPAINT | PGRCFG_REFORMAT;
            UpdateValueSet(pnbp->hwndDlgPage,
                           pPagerCfg);
        }
        break;

        case ID_SCDI_PGR1_WRAPAROUND:
            LoadPagerConfig(pnbp->pUser);
            if (ulExtra)
                pPagerCfg->flPager |= PGRFL_WRAPAROUND;
            else
                pPagerCfg->flPager &= ~PGRFL_WRAPAROUND;
        break;

        case ID_SCDI_PGR1_VALUESET:
            if (usNotifyCode == VN_ENTER)
            {
                // double-click on value set item:
                // refresh startup desktop
                LoadPagerConfig(pnbp->pUser);  // V0.9.19 (2002-04-23) [pr]
                pPagerCfg->bStartX = SHORT2FROMMP((MPARAM)ulExtra);
                // value set row 1 is on top; if we get row 1,
                // it is really the top row (cDesktopsY)
                // V0.9.19 (2002-05-07) [umoeller]
                pPagerCfg->bStartY =   pPagerCfg->cDesktopsY
                                     - SHORT1FROMMP((MPARAM)ulExtra)
                                     + 1;
                UpdateValueSet(pnbp->hwndDlgPage,
                               pPagerCfg);
            }
        break;

        case ID_SCDI_PGR1_FOLLOWFOCUS:
            LoadPagerConfig(pnbp->pUser);
            // this one is reverse
            if (!ulExtra)
                pPagerCfg->flPager |= PGRFL_NOFOLLOWFOCUS;
            else
                pPagerCfg->flPager &= ~PGRFL_NOFOLLOWFOCUS;
            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
        break;

        case ID_SCDI_PGR1_ARROWHOTKEYS:
            LoadPagerConfig(pnbp->pUser);
            if (ulExtra)
                pPagerCfg->flPager |= PGRFL_HOTKEYS;
            else
                pPagerCfg->flPager &= ~PGRFL_HOTKEYS;
            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
        break;

        // V1.0.3 (2004-10-14) [bird]
        case ID_SCDI_PGR1_WINDOWS_KEYS:
            LoadPagerConfig(pnbp->pUser);
            if (ulExtra)
                pPagerCfg->flPager |= PGRFL_WINDOWS_KEYS;
            else
                pPagerCfg->flPager &= ~PGRFL_WINDOWS_KEYS;
            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
        break;

        case ID_SCDI_PGR1_HOTKEYS_CTRL:
        case ID_SCDI_PGR1_HOTKEYS_SHIFT:
        case ID_SCDI_PGR1_HOTKEYS_ALT:
        {
            ULONG flOldKeyShift;
            LoadPagerConfig(pnbp->pUser);
            flOldKeyShift = pPagerCfg->flKeyShift;

            pPagerCfg->flKeyShift = 0;
            if (winhIsDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_HOTKEYS_CTRL))
                 pPagerCfg->flKeyShift |= KC_CTRL;
            if (winhIsDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_HOTKEYS_SHIFT))
                 pPagerCfg->flKeyShift |= KC_SHIFT;
            if (winhIsDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_HOTKEYS_ALT))
                 pPagerCfg->flKeyShift |= KC_ALT;

            if (pPagerCfg->flKeyShift == 0)
            {
                // no modifiers enabled: we really shouldn't allow this,
                // so restore the old value
                pPagerCfg->flKeyShift = flOldKeyShift;
                WinAlarm(HWND_DESKTOP, WA_ERROR);
                pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            }
        }
        break;

        /*
         * DID_DEFAULT:
         *
         *changed V0.9.9 (2001-03-15) [lafaix]: saves settings here
         */

        case DID_DEFAULT:
            LoadPagerConfig(pnbp->pUser);

            hifEnableXPager(FALSE);

            pPagerCfg->cDesktopsX = 3;
            pPagerCfg->cDesktopsY = 2;
            pPagerCfg->bStartX = 1;
            pPagerCfg->bStartY = 1;
            pPagerCfg->flKeyShift = KC_CTRL | KC_ALT;
            pPagerCfg->flPager =
                (pPagerCfg->flPager
                 & ~PGRMASK_PAGE1
                 ) | PGRFL_PAGE1_DEFAULTS;

            ulPgmgChangedFlags = PGRCFG_REPAINT | PGRCFG_REFORMAT;

            SavePagerConfig(pPagerCfg,
                               ulPgmgChangedFlags);

            // call INIT callback to reinitialize page
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);

            fSave = FALSE;
        break;

        /*
         * DID_UNDO:
         *
         *changed V0.9.9 (2001-03-15) [lafaix]: saves settings here
         */

        case DID_UNDO:
        {
            PPAGERPAGEDATA pBackup = (PPAGERPAGEDATA)pnbp->pUser2;

            LoadPagerConfig(pnbp->pUser);

            hifEnableXPager(pBackup->fEnableXPager);

            pPagerCfg->cDesktopsX = pBackup->PgrConfig.cDesktopsX;
            pPagerCfg->cDesktopsY = pBackup->PgrConfig.cDesktopsY;
            pPagerCfg->flKeyShift = pBackup->PgrConfig.flKeyShift;
            pPagerCfg->flPager =
                (pPagerCfg->flPager
                 & ~PGRMASK_PAGE1
                ) | (pBackup->PgrConfig.flPager & PGRMASK_PAGE1);

            ulPgmgChangedFlags = PGRCFG_REPAINT | PGRCFG_REFORMAT;

            SavePagerConfig(pPagerCfg,
                            ulPgmgChangedFlags);

            // call INIT callback to reinitialize page
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);

            fSave = FALSE;
        }
        break;

        default:
            fSave = FALSE;
        break;
    }

    if (    (fSave)
         && (pnbp->flPage & NBFL_PAGE_INITED) // page initialized yet?
       )
    {
        SavePagerConfig(pPagerCfg,
                        ulPgmgChangedFlags);
    }

    return mrc;
}

/* ******************************************************************
 *
 *   XPager Window page notebook functions (notebook.c)
 *
 ********************************************************************/

static const CONTROLDEF
    G_PagerDisabled = LOADDEF_TEXT_WORDBREAK(ID_SCDI_PGR2_DISABLED_INFO, PAGE_WIDTH),
            // also used by the other pages
    ControlWindowGroup = LOADDEF_GROUP(ID_SCDI_PGR1_WINDOW_GROUP, PAGE_WIDTH),
    PgrPreservePropsCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_PRESERVEPROPS),
    PgrStayOnTopCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_STAYONTOP),
    PgrFlashToTopCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_FLASHTOTOP),
    PgrDelayTxt1 = LOADDEF_TEXT(ID_SCDI_PGR1_FLASH_TXT1),
    PgrDelaySpin = CONTROLDEF_SPINBUTTON(
                            ID_SCDI_PGR1_FLASH_SPIN,
                            25,
                            STD_SPIN_HEIGHT),
    PgrDelayTxt2 = LOADDEF_TEXT(ID_SCDI_PGR1_FLASH_TXT2),
    PgrMiniWindowsCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_SHOWWINDOWS),
    PgrShowWinTitlesCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_SHOWWINTITLES),
    PgrShowWinIconsCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_SHOWWINICONS),
    PgrClick2ActivateCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_CLICK2ACTIVATE),
    PgrShowSecondaryCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_SHOWSECONDARY),
    PgrShowStickyCB = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_SHOWSTICKY);

static const DLGHITEM G_dlgPagerWindow[] =
    {
        START_TABLE,
            START_ROW(0),
                CONTROL_DEF(&G_PagerDisabled),
            START_ROW(0),
                START_GROUP_TABLE(&ControlWindowGroup),
                    START_ROW(0),
                        CONTROL_DEF(&PgrPreservePropsCB),
                    START_ROW(0),
                        CONTROL_DEF(&PgrStayOnTopCB),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&PgrFlashToTopCB),
                        CONTROL_DEF(&PgrDelayTxt1),
                        CONTROL_DEF(&PgrDelaySpin),
                        CONTROL_DEF(&PgrDelayTxt2),
                    START_ROW(0),
                        CONTROL_DEF(&PgrMiniWindowsCB),
                    START_ROW(0),
                        CONTROL_DEF(&G_Spacing),
                        CONTROL_DEF(&PgrShowWinTitlesCB),
                    START_ROW(0),
                        CONTROL_DEF(&G_Spacing),
                        CONTROL_DEF(&PgrShowWinIconsCB),
                    START_ROW(0),
                        CONTROL_DEF(&G_Spacing),
                        CONTROL_DEF(&PgrClick2ActivateCB),
                    START_ROW(0),
                        CONTROL_DEF(&G_Spacing),
                        CONTROL_DEF(&PgrShowSecondaryCB),
                    START_ROW(0),
                        CONTROL_DEF(&G_Spacing),
                        CONTROL_DEF(&PgrShowStickyCB),
                END_TABLE,
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE,
    };

/*
 *@@ PagerWindowInitPage:
 *      notebook callback function (notebook.c) for the
 *      second "XPager" page in the "Screen" settings object.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@added V0.9.9 (2001-03-15) [lafaix]
 *@@changed V0.9.19 (2002-04-11) [lafaix]: added support for MDF_INCLUDE*
 *@@changed V0.9.19 (2002-04-17) [umoeller]: now using dlg formatter
 *@@changed V0.9.19 (2002-05-28) [umoeller]: adjustments for new pager handling
 */

STATIC VOID PagerWindowInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                                ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        // first call: create PAGERCONFIG
        // structure;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        if (pnbp->pUser = malloc(sizeof(PAGERCONFIG)))
            LoadPagerConfig(pnbp->pUser);

        // make backup for "undo"
        if (pnbp->pUser2 = malloc(sizeof(PAGERCONFIG)))
            memcpy(pnbp->pUser2, pnbp->pUser, sizeof(PAGERCONFIG));

        // insert the controls using the dialog formatter
        // V0.9.19 (2002-04-17) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      G_dlgPagerWindow,
                      ARRAYITEMCOUNT(G_dlgPagerWindow));
    }

    if (flFlags & CBI_SET)
    {
        PAGERCONFIG* pPagerCfg = (PAGERCONFIG*)pnbp->pUser;
        ULONG       flPager = pPagerCfg->flPager;
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_PRESERVEPROPS,
                              !!(flPager & PGRFL_PRESERVEPROPS));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_STAYONTOP,
                              !!(flPager & PGRFL_STAYONTOP));

        // flash
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_FLASHTOTOP,
                              !!(flPager & PGRFL_FLASHTOTOP));
        winhSetDlgItemSpinData(pnbp->hwndDlgPage, ID_SCDI_PGR1_FLASH_SPIN,
                               1, 30,       // min, max
                               pPagerCfg->ulFlashDelay / 1000);  // current

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_SHOWWINDOWS,
                              !!(flPager & PGRFL_MINIWINDOWS));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_SHOWWINTITLES,
                              !!(flPager & PGRFL_MINIWIN_TITLES));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_SHOWWINICONS,
                              !!(flPager & PGRFL_MINIWIN_ICONS));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_CLICK2ACTIVATE,
                              !!(flPager & PGRFL_MINIWIN_MOUSE));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_SHOWSECONDARY,
                              !!(flPager & PGRFL_INCLUDESECONDARY));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_SHOWSTICKY,
                              !!(flPager & PGRFL_INCLUDESTICKY));

    }

    if (flFlags & CBI_ENABLE)
    {
        static const ULONG
        aulIDsPager[] =
            {
                ID_SCDI_PGR1_WINDOW_GROUP,
                ID_SCDI_PGR1_PRESERVEPROPS,
                ID_SCDI_PGR1_STAYONTOP,
                ID_SCDI_PGR1_FLASHTOTOP,
                ID_SCDI_PGR1_SHOWWINDOWS,
            },
        aulIDsFlash[] =
            {
                ID_SCDI_PGR1_FLASH_TXT1,
                ID_SCDI_PGR1_FLASH_SPIN,
                ID_SCDI_PGR1_FLASH_TXT2,
            },
        aulIDsMiniWindows[] =
            {
                ID_SCDI_PGR1_SHOWWINTITLES,
                ID_SCDI_PGR1_SHOWWINICONS,
                ID_SCDI_PGR1_CLICK2ACTIVATE,
                ID_SCDI_PGR1_SHOWSECONDARY,
                ID_SCDI_PGR1_SHOWSTICKY,
            };

        BOOL fPager = cmnQuerySetting(sfEnableXPager);

        WinShowWindow(WinWindowFromID(pnbp->hwndDlgPage, ID_SCDI_PGR2_DISABLED_INFO),
                      !fPager);

        winhEnableControls2(pnbp->hwndDlgPage,
                            aulIDsPager,
                            ARRAYITEMCOUNT(aulIDsPager),
                            fPager);
        EnableNotebookButtons(pnbp,
                              fPager);

        winhEnableControls2(pnbp->hwndDlgPage,
                            aulIDsFlash,
                            ARRAYITEMCOUNT(aulIDsFlash),
                               fPager
                            && winhIsDlgItemChecked(pnbp->hwndDlgPage,
                                                    ID_SCDI_PGR1_FLASHTOTOP));
        winhEnableControls2(pnbp->hwndDlgPage,
                            aulIDsMiniWindows,
                            ARRAYITEMCOUNT(aulIDsMiniWindows),
                               fPager
                            && winhIsDlgItemChecked(pnbp->hwndDlgPage,
                                                    ID_SCDI_PGR1_SHOWWINDOWS));
    }
}

/*
 *@@ PagerWindowItemChanged:
 *      notebook callback function (notebook.c) for the
 *      second "XPager" page in the "Screen" settings object.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@added V0.9.9 (2001-03-15) [lafaix]
 *@@changed V0.9.9 (2001-03-15) [lafaix]: fixed odd undo/default behavior
 *@@changed V0.9.19 (2002-04-11) [lafaix]: added support for MDF_INCLUDE*
 */

STATIC MRESULT PagerWindowItemChanged(PNOTEBOOKPAGE pnbp,
                                      ULONG ulItemID, USHORT usNotifyCode,
                                      ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = 0;
    BOOL    fSave = TRUE;      // save settings per default; this is set to FALSE if not needed
    ULONG   ulPgmgChangedFlags = 0;

    // access settings
    PAGERCONFIG* pPagerCfg = (PAGERCONFIG*)pnbp->pUser;
    ULONG   flPagerChanged = 0;

    switch (ulItemID)
    {
        case ID_SCDI_PGR1_PRESERVEPROPS:
            flPagerChanged = PGRFL_PRESERVEPROPS;
            ulPgmgChangedFlags = PGRCFG_REFORMAT;
        break;

        case ID_SCDI_PGR1_STAYONTOP:
            flPagerChanged = PGRFL_STAYONTOP;
            ulPgmgChangedFlags = PGRCFG_REFORMAT;
        break;

        case ID_SCDI_PGR1_FLASHTOTOP:
            flPagerChanged = PGRFL_FLASHTOTOP;
            ulPgmgChangedFlags = PGRCFG_REFORMAT;
        break;

        case ID_SCDI_PGR1_FLASH_SPIN:
            // delay spinbutton
            LoadPagerConfig(pnbp->pUser);
            pPagerCfg->ulFlashDelay = ulExtra * 1000;
            ulPgmgChangedFlags = PGRCFG_REFORMAT;
        break;

        case ID_SCDI_PGR1_SHOWWINDOWS:
            flPagerChanged = PGRFL_MINIWINDOWS;
            ulPgmgChangedFlags = PGRCFG_REPAINT;
        break;

        case ID_SCDI_PGR1_SHOWWINTITLES:
            flPagerChanged = PGRFL_MINIWIN_TITLES;
            ulPgmgChangedFlags = PGRCFG_REPAINT;
        break;

        case ID_SCDI_PGR1_SHOWWINICONS:
            flPagerChanged = PGRFL_MINIWIN_ICONS;
            ulPgmgChangedFlags = PGRCFG_REPAINT;
        break;

        case ID_SCDI_PGR1_SHOWSECONDARY:
            flPagerChanged = PGRFL_INCLUDESECONDARY;
            ulPgmgChangedFlags = PGRCFG_REPAINT;
        break;

        case ID_SCDI_PGR1_SHOWSTICKY:
            flPagerChanged = PGRFL_INCLUDESTICKY;
            ulPgmgChangedFlags = PGRCFG_REPAINT;
        break;

        case ID_SCDI_PGR1_CLICK2ACTIVATE:
            flPagerChanged = PGRFL_MINIWIN_MOUSE;
        break;

        /*
         * DID_DEFAULT:
         *
         */

        case DID_DEFAULT:
            LoadPagerConfig(pnbp->pUser);
            pPagerCfg->flPager =
                (pPagerCfg->flPager
                 & ~PGRMASK_PAGE2
                 ) | PGRFL_PAGE2_DEFAULTS;

            pPagerCfg->ulFlashDelay = 2000;

            SavePagerConfig(pPagerCfg,
                            PGRCFG_REPAINT | PGRCFG_REFORMAT);
            fSave = FALSE;      // V0.9.9 (2001-03-27) [umoeller]

            // call INIT callback to reinitialize page
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;

        /*
         * DID_UNDO:
         *
         *changed V0.9.9 (2001-03-15) [lafaix]: save settings here
         */

        case DID_UNDO:
        {
            PAGERCONFIG* pBackup = (PAGERCONFIG*)pnbp->pUser2;

            LoadPagerConfig(pnbp->pUser);
            pPagerCfg->ulFlashDelay = pBackup->ulFlashDelay;
            pPagerCfg->flPager =
                (pPagerCfg->flPager
                 & ~PGRMASK_PAGE2
                ) | (pBackup->flPager & PGRMASK_PAGE2);

            SavePagerConfig(pPagerCfg,
                            PGRCFG_REPAINT | PGRCFG_REFORMAT);
                                        // fixed V0.9.9 (2001-03-27) [umoeller]
            fSave = FALSE;

            // call INIT callback to reinitialize page
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        }
        break;

        default:
            fSave = FALSE;
        break;
    }

    if (flPagerChanged)
    {
        LoadPagerConfig(pnbp->pUser);
        if (ulExtra)
            pPagerCfg->flPager |= flPagerChanged;
        else
            pPagerCfg->flPager &= ~flPagerChanged;
    }

    if (    (fSave)
         && (pnbp->flPage & NBFL_PAGE_INITED)   // page initialized yet?
       )
    {
        SavePagerConfig(pPagerCfg,
                         ulPgmgChangedFlags);
        pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
    }

    return mrc;
}

/* ******************************************************************
 *
 *   XPager Sticky page notebook functions (notebook.c)
 *
 ********************************************************************/

/*
 *@@ STICKYRECORD:
 *      extended record core for "Sticky windows" container.
 *
 *@@added V0.9.4 (2000-07-10) [umoeller]
 *@@changed V0.9.19 (2002-04-14) [lafaix]: added pcszCriteria, pcszAttribute and pcszOperator
 */

typedef struct _STICKYRECORD
{
    RECORDCORE  recc;
    CHAR        szSticky[STICKYLEN];
    PSZ         pcszCriteria;
    PSZ         pcszAttribute;
    PSZ         pcszOperator;
    PSZ         pcszValue;
    ULONG       ulFlags;
} STICKYRECORD, *PSTICKYRECORD;

/*
 *@@ AdjustStickyRecord:
 *      adjusts the pcsz* values in the STICKYRECORD.
 *
 *@@added V0.9.19 (2002-04-15) [lafaix]
 *@@changed V0.9.19 (2002-04-17) [umoeller]: added regexp support
 *@@changed V0.9.19 (2002-05-28) [umoeller]: some cleanup
 */

STATIC VOID AdjustStickyRecord(PSTICKYRECORD pRec)
{
    ULONG id;

    switch (pRec->ulFlags & SF_CRITERIA_MASK)
    {
        case SF_INCLUDE:
            id = ID_SCDI_STICKY_INCLUDE;
        break;

        case SF_EXCLUDE:
            id = ID_SCDI_STICKY_EXCLUDE;
        break;
    }

    pRec->pcszCriteria = (PSZ)cmnGetString(id);

    switch (pRec->ulFlags & SF_OPERATOR_MASK)
    {
        case SF_CONTAINS:
            id = ID_SCDI_STICKY_CONTAINS;
        break;
        case SF_BEGINSWITH:
            id = ID_SCDI_STICKY_BEGINSWITH;
        break;
        case SF_ENDSWITH:
            id = ID_SCDI_STICKY_ENDSWITH;
        break;
        case SF_EQUALS:
            id = ID_SCDI_STICKY_EQUALS;
        break;
        case SF_MATCHES: // V0.9.19 (2002-04-17) [umoeller]
            id = ID_SCDI_STICKY_MATCHES;
        break;
    }

    pRec->pcszOperator = (PSZ)cmnGetString(id);

    // only one attribute supported so far, SF_TITLE
    pRec->pcszAttribute = (PSZ)cmnGetString(ID_SCDI_STICKY_TITLEATTRIBUTE);

    pRec->pcszValue = pRec->szSticky;
}

/*
 *@@ AddStickyRecord:
 *      creates and inserts a STICKYRECORD for the given
 *      container with the specified title.
 *
 *@@added V0.9.4 (2000-07-10) [umoeller]
 *@@changed V0.9.19 (2002-04-14) [lafaix]: added ulFlags support
 */

STATIC VOID AddStickyRecord(HWND hwndCnr,
                            PSZ pszStickyName,     // in: window or switch list title (for XPager)
                            ULONG ulFlags,         // in: entry flags (SF_*)
                            BOOL fInvalidate)      // in: if TRUE, invalidate records
{
    PSTICKYRECORD pRec;

    if (pRec = (PSTICKYRECORD)cnrhAllocRecords(hwndCnr,
                                               sizeof(STICKYRECORD),
                                               1))
    {
        strhncpy0(pRec->szSticky, pszStickyName, STICKYLEN);
        pRec->ulFlags = ulFlags;

        AdjustStickyRecord(pRec);

        cnrhInsertRecords(hwndCnr,
                          NULL, // parent
                          (PRECORDCORE)pRec,
                          fInvalidate,
                          pRec->szSticky,
                          CRA_RECORDREADONLY,
                          1);   // count
    }
}

/*
 *@@ SaveStickies:
 *      enumerates the STICKYRECORD's in the given
 *      container and updates XPager's stickies
 *      list. This calls SavePagerConfig in turn.
 *
 *@@added V0.9.4 (2000-07-10) [umoeller]
 *@@changed V0.9.19 (2002-04-14) [lafaix]: added flags support
 */

STATIC VOID SaveStickies(HWND hwndCnr,
                         PAGERCONFIG* pPagerCfg)
{
    PSTICKYRECORD   pRec = NULL;
    USHORT          usCmd = CMA_FIRST;
    BOOL            fCont = TRUE;
    USHORT          usStickyIndex = 0;      // raised with each iteration

    LoadPagerConfig(pPagerCfg); // V0.9.19 (2002-04-23) [pr]
    do
    {
        pRec = (PSTICKYRECORD)WinSendMsg(hwndCnr,
                                         CM_QUERYRECORD,
                                         pRec, // ignored on first call
                                         MPFROM2SHORT(usCmd,     // CMA_FIRST or CMA_NEXT
                                                      CMA_ITEMORDER));
        usCmd = CMA_NEXT;

        if ((pRec) && ((ULONG)pRec != -1))
        {
            pPagerCfg->aulStickyFlags[usStickyIndex] = pRec->ulFlags;
            strlcpy(pPagerCfg->aszStickies[usStickyIndex],
                    pRec->szSticky,
                    sizeof(pPagerCfg->aszStickies[usStickyIndex]));
            usStickyIndex++;
        }
        else
            fCont = FALSE;

    } while (fCont);

    // store stickies count
    pPagerCfg->cStickies = usStickyIndex;

    SavePagerConfig(pPagerCfg,
                    PGRCFG_REPAINT
                         | PGRCFG_REFORMAT
                         | PGRCFG_STICKIES);
}

#define ATTRWIDTH       50
#define OPERWIDTH       50
#define VALUEWIDTH      100

static const CONTROLDEF
    CriteriaGroup = LOADDEF_GROUP(ID_SCDI_STICKY_CRITERIAGROUP, SZL_AUTOSIZE),
    AttrTxt = CONTROLDEF_TEXT_CENTER(
                            LOAD_STRING,
                            ID_SCDI_STICKY_ATTRIBUTE,
                            ATTRWIDTH,
                            -1),
    AttrList = CONTROLDEF_DROPDOWNLIST(
                            ID_SCDI_STICKY_ATTRIBUTE_DROP,
                            ATTRWIDTH,
                            70),        // reduced V0.9.20 (2002-07-03) [umoeller]
    OperTxt = CONTROLDEF_TEXT_CENTER(
                            LOAD_STRING,
                            ID_SCDI_STICKY_OPERATOR,
                            OPERWIDTH,
                            -1),
    OperList = CONTROLDEF_DROPDOWNLIST(
                            ID_SCDI_STICKY_OPERATOR_DROP,
                            OPERWIDTH,
                            70),        // reduced V0.9.20 (2002-07-03) [umoeller]
    ValueTxt = CONTROLDEF_TEXT_CENTER(
                            LOAD_STRING,
                            ID_SCDI_STICKY_VALUE,
                            VALUEWIDTH,
                            -1),
    ValueList = CONTROLDEF_DROPDOWN(
                            ID_SCDI_STICKY_VALUE_DROP,
                            VALUEWIDTH,
                            100),       // reduced V0.9.20 (2002-07-03) [umoeller]
    MatchingGroup = LOADDEF_GROUP(ID_SCDI_STICKY_MATCHINGGROUP, SZL_AUTOSIZE),
    IncludeRadio = LOADDEF_FIRST_AUTORADIO(ID_SCDI_STICKY_RADIO_INCLUDE),
    ExcludeRadio = LOADDEF_NEXT_AUTORADIO(ID_SCDI_STICKY_RADIO_EXCLUDE);

static const DLGHITEM dlgAddSticky[] =
    {
        START_TABLE,            // root table, required
/*             START_ROW(0),        no, not in the dialog V0.9.20 (2002-07-03) [umoeller]
                CONTROL_DEF(&G_PagerDisabled), */
            START_ROW(0),
                START_GROUP_TABLE(&CriteriaGroup),
                    START_ROW(0),
                        START_TABLE,
                            START_ROW(0),
                                CONTROL_DEF(&AttrTxt),
                            START_ROW(0),
                                CONTROL_DEF(&AttrList),
                        END_TABLE,
                        START_TABLE,
                            START_ROW(0),
                                CONTROL_DEF(&OperTxt),
                            START_ROW(0),
                                CONTROL_DEF(&OperList),
                        END_TABLE,
                        START_TABLE,
                            START_ROW(0),
                                CONTROL_DEF(&ValueTxt),
                            START_ROW(0),
                                CONTROL_DEF(&ValueList),
                        END_TABLE,
                    START_ROW(0),
                        START_GROUP_TABLE(&MatchingGroup),
                            START_ROW(0),
                                CONTROL_DEF(&IncludeRadio),
                            START_ROW(0),
                                CONTROL_DEF(&ExcludeRadio),
                        END_TABLE,
                END_TABLE,
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_OKButton),           // common.c
                CONTROL_DEF(&G_CancelButton),       // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };

/*
 *@@ fnwpEditStickyRecord:
 *
 *@@added V0.9.19 (2002-04-17) [umoeller]
 */

STATIC MRESULT EXPENTRY fnwpEditStickyRecord(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_COMMAND:
        {
            BOOL fDismiss = TRUE;
            HWND hwndCombo;

            if (    (SHORT1FROMMP(mp1) == DID_OK)
                 && (hwndCombo = WinWindowFromID(hwndDlg, ID_SCDI_STICKY_OPERATOR_DROP))
                 && (WinQueryLboxSelectedItem(hwndCombo) == 4)      // SF_MATCHES
                 // reassign for below
                 && (hwndCombo = WinWindowFromID(hwndDlg, ID_SCDI_STICKY_VALUE_DROP))
               )
            {
                // before allowing OK, check if the regexp is valid
                PSZ pszRegExp;
                if (pszRegExp = winhQueryWindowText(hwndCombo))
                {
                    ERE *ere;
                    int rc;
                    if (!(ere = rxpCompile(pszRegExp,
                                           0,
                                           &rc)))
                    {
                        cmnErrorMsgBox(hwndDlg,
                                    rc,
                                    234,
                                    MB_OK,
                                    TRUE);
                        fDismiss = FALSE;

                        WinSetFocus(HWND_DESKTOP, hwndCombo);
                    }

                    if (ere)
                        rxpFree(ere);

                    free(pszRegExp);
                }
                else
                    fDismiss = FALSE;
            }

            if (fDismiss)
                WinDismissDlg(hwndDlg, SHORT1FROMMP(mp1));
        }
        break;

        case WM_HELP:
            cmnDisplayHelp(NULL, ID_XSH_SETTINGS_PAGER_STICKY + 2);
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ EditStickyRecord:
 *      edit and possibly inserts a STICKYRECORD.
 *
 *@@added V0.9.19 (2002-04-14) [lafaix]
 *@@changed V0.9.19 (2002-04-17) [umoeller]: now using dialog formatter, added regexps
 *@@changed V0.9.20 (2002-07-03) [umoeller]: removed "pager is disabled" in sticky dialog, fixed excessive drop-downs
 */

STATIC VOID EditStickyRecord(PSTICKYRECORD pRec,
                             PNOTEBOOKPAGE pnbp,
                             HWND hwndCnr,
                             BOOL fInsert)
{
    HWND        hwndDlg;

    if (!dlghCreateDlg(&hwndDlg,
                       pnbp->hwndDlgPage,
                       FCF_FIXED_DLG,
                       fnwpEditStickyRecord,
                       cmnGetString(fInsert
                                        ? ID_SCDI_STICKY_ADDTITLE
                                        : ID_SCDI_STICKY_EDITTITLE),
                       dlgAddSticky,
                       ARRAYITEMCOUNT(dlgAddSticky),
                       NULL,
                       cmnQueryDefaultFont()))
    {
        PSWBLOCK    pSwBlock;

        winhCenterWindow(hwndDlg);

        // get all the tasklist entries into a buffer
        // V0.9.16 (2002-01-05) [umoeller]: now using winhQuerySwitchList
        if (pSwBlock = winhQuerySwitchList(WinQueryAnchorBlock(pnbp->hwndDlgPage)))
        {
            ULONG ul;
            HWND  hwndCombo = WinWindowFromID(hwndDlg, ID_SCDI_STICKY_ATTRIBUTE_DROP);

            // filling the possible attributes (just Title currently)
            WinInsertLboxItem(hwndCombo,
                              0,
                              cmnGetString(ID_SCDI_STICKY_TITLEATTRIBUTE));
            WinSendMsg(hwndCombo, LM_SELECTITEM, MPFROMSHORT(0), MPFROMSHORT(TRUE));

            // filling the possible operators
            hwndCombo = WinWindowFromID(hwndDlg, ID_SCDI_STICKY_OPERATOR_DROP);
            WinInsertLboxItem(hwndCombo, 0, cmnGetString(ID_SCDI_STICKY_CONTAINS));
            WinInsertLboxItem(hwndCombo, 1, cmnGetString(ID_SCDI_STICKY_BEGINSWITH));
            WinInsertLboxItem(hwndCombo, 2, cmnGetString(ID_SCDI_STICKY_ENDSWITH));
            WinInsertLboxItem(hwndCombo, 3, cmnGetString(ID_SCDI_STICKY_EQUALS));
            WinInsertLboxItem(hwndCombo, 4, cmnGetString(ID_SCDI_STICKY_MATCHES));

            switch (pRec->ulFlags & SF_OPERATOR_MASK)
            {
                case SF_CONTAINS:
                    WinSendMsg(hwndCombo, LM_SELECTITEM, MPFROMSHORT(0), MPFROMSHORT(TRUE));
                break;
                case SF_BEGINSWITH:
                    WinSendMsg(hwndCombo, LM_SELECTITEM, MPFROMSHORT(1), MPFROMSHORT(TRUE));
                break;
                case SF_ENDSWITH:
                    WinSendMsg(hwndCombo, LM_SELECTITEM, MPFROMSHORT(2), MPFROMSHORT(TRUE));
                break;
                case SF_EQUALS:
                    WinSendMsg(hwndCombo, LM_SELECTITEM, MPFROMSHORT(3), MPFROMSHORT(TRUE));
                break;
                case SF_MATCHES:
                    WinSendMsg(hwndCombo, LM_SELECTITEM, MPFROMSHORT(4), MPFROMSHORT(TRUE));
                break;
            }

            // loop through all the tasklist entries
            hwndCombo = WinWindowFromID(hwndDlg, ID_SCDI_STICKY_VALUE_DROP);
            for (ul = 0;
                 ul < pSwBlock->cswentry;
                 ul++)
            {
                PSWCNTRL pCtrl = &pSwBlock->aswentry[ul].swctl;
                if (    (strlen(pCtrl->szSwtitle))
                     && ((pCtrl->uchVisibility & SWL_VISIBLE) != 0) // V0.9.11 (2001-04-25) [umoeller]
                   )
                    WinInsertLboxItem(hwndCombo,
                                      LIT_SORTASCENDING,
                                      pCtrl->szSwtitle);
            }
            WinSetWindowText(hwndCombo, pRec->szSticky);

            WinCheckButton(hwndDlg,
                           ID_SCDI_STICKY_RADIO_INCLUDE,
                           (pRec->ulFlags & SF_CRITERIA_MASK) == SF_INCLUDE);
            WinCheckButton(hwndDlg,
                           ID_SCDI_STICKY_RADIO_EXCLUDE,
                           (pRec->ulFlags & SF_CRITERIA_MASK) == SF_EXCLUDE);

            if (WinProcessDlg(hwndDlg) == DID_OK)
            {
                // OK pressed:
                PSZ pszSticky;
                if (pszSticky = winhQueryWindowText(hwndCombo))
                {
                    ULONG ulFlags = 0;

                    // build flags from settings
                    if (WinQueryButtonCheckstate(hwndDlg,
                                                 ID_SCDI_STICKY_RADIO_EXCLUDE))
                        ulFlags |= SF_EXCLUDE;
                    hwndCombo = WinWindowFromID(hwndDlg, ID_SCDI_STICKY_OPERATOR_DROP);
                    switch (WinQueryLboxSelectedItem(hwndCombo))
                    {
                        case 1:
                            ulFlags |= SF_BEGINSWITH;
                        break;
                        case 2:
                            ulFlags |= SF_ENDSWITH;
                        break;
                        case 3:
                            ulFlags |= SF_EQUALS;
                        break;
                        case 4:
                            ulFlags |= SF_MATCHES;
                        break;
                    }
                    pRec->ulFlags = ulFlags;
                    strhncpy0(pRec->szSticky, pszSticky, STICKYLEN);

                    if (fInsert)
                        AddStickyRecord(hwndCnr,
                                        pszSticky,
                                        ulFlags,
                                        TRUE);          // invalidate
                    else
                    {
                        // pRec is already in container
                        AdjustStickyRecord(pRec);

                        // invalidate container to refresh view
                        WinSendMsg(hwndCnr,
                                   CM_INVALIDATERECORD,
                                   (MPARAM)&pRec,
                                   MPFROM2SHORT(1,
                                                CMA_TEXTCHANGED));
                    }

                    SaveStickies(hwndCnr,
                                 (PAGERCONFIG*)pnbp->pUser);
                    free(pszSticky);
                }
            }

            free(pSwBlock);
        }

        WinDestroyWindow(hwndDlg);
    }
}

static const CONTROLDEF
    StickiesGroup = LOADDEF_GROUP(ID_SCDI_STICKY_GROUP, SZL_AUTOSIZE),
    StickiesCnr = CONTROLDEF_CONTAINER(
                            ID_SCDI_STICKY_CNR,
                            200,        // for now, will be resized
                            100),       // for now, will be resized
    StickiesToggle = LOADDEF_AUTOCHECKBOX(ID_SCDI_PGR1_STICKIESTOGGLE);

static const DLGHITEM G_dlgPagerStickies[] =
    {
        START_TABLE,            // root table, required
            START_ROW(0),
                CONTROL_DEF(&G_PagerDisabled),
            START_ROW(0),
                START_GROUP_TABLE(&StickiesGroup),
                    START_ROW(0),
                        CONTROL_DEF(&StickiesCnr),
                    START_ROW(0),
                        CONTROL_DEF(&G_AddButton),
                        CONTROL_DEF(&G_EditButton),
                        CONTROL_DEF(&G_RemoveButton),
                END_TABLE,
            // Martin, I moved this out of the group
            // V1.0.0 (2002-09-17) [umoeller]
            START_ROW(0),
                CONTROL_DEF(&StickiesToggle),
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };

MPARAM G_ampStickies[] =
    {
        MPFROM2SHORT(ID_SCDI_PGR2_DISABLED_INFO, XAC_MOVEY),
        MPFROM2SHORT(ID_SCDI_STICKY_CNR, XAC_SIZEX | XAC_SIZEY),
        MPFROM2SHORT(ID_SCDI_STICKY_GROUP, XAC_SIZEX | XAC_SIZEY),
    };

/*
 *@@ PagerStickyInitPage:
 *      notebook callback function (notebook.c) for the
 *      "XPager Sticky Windows" page in the "Screen" settings object.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@added V0.9.4 (2000-07-10) [umoeller]
 *@@changed V0.9.19 (2002-04-14) [lafaix]: modified container view
 *@@changed V0.9.19 (2002-04-17) [umoeller]: now using dialog formatter
 *@@changed V1.0.0 (2002-09-14) [lafaix]: added support for PGRFL_ADDSTICKYTOGGLE
 */

STATIC VOID PagerStickyInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                                ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        HWND        hwndCnr;
        XFIELDINFO  xfi[4];
        int         i = 0;

        // first call: create PAGERCONFIG
        // structure;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        if (pnbp->pUser = malloc(sizeof(PAGERCONFIG)))
            LoadPagerConfig(pnbp->pUser);

        // make backup for "undo"
        if (pnbp->pUser2 = malloc(sizeof(PAGERCONFIG)))
            memcpy(pnbp->pUser2, pnbp->pUser, sizeof(PAGERCONFIG));

        // insert the controls using the dialog formatter
        // V0.9.19 (2002-04-17) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      G_dlgPagerStickies,
                      ARRAYITEMCOUNT(G_dlgPagerStickies));

        hwndCnr = WinWindowFromID(pnbp->hwndDlgPage,
                                  ID_SCDI_STICKY_CNR);

        // set up cnr details view
        xfi[i].ulFieldOffset = FIELDOFFSET(STICKYRECORD, pcszCriteria);
        xfi[i].pszColumnTitle = cmnGetString(ID_SCDI_STICKY_CRITERIA);
        xfi[i].ulDataType = CFA_STRING | CFA_HORZSEPARATOR;
        xfi[i++].ulOrientation = CFA_LEFT;

        xfi[i].ulFieldOffset = FIELDOFFSET(STICKYRECORD, pcszAttribute);
        xfi[i].pszColumnTitle = cmnGetString(ID_SCDI_STICKY_ATTRIBUTE);
        xfi[i].ulDataType = CFA_STRING | CFA_HORZSEPARATOR;
        xfi[i++].ulOrientation = CFA_LEFT;

        xfi[i].ulFieldOffset = FIELDOFFSET(STICKYRECORD, pcszOperator);
        xfi[i].pszColumnTitle = cmnGetString(ID_SCDI_STICKY_OPERATOR);
        xfi[i].ulDataType = CFA_STRING | CFA_HORZSEPARATOR;
        xfi[i++].ulOrientation = CFA_LEFT;

        xfi[i].ulFieldOffset = FIELDOFFSET(STICKYRECORD, pcszValue);
        xfi[i].pszColumnTitle = cmnGetString(ID_SCDI_STICKY_VALUE);
        xfi[i].ulDataType = CFA_STRING | CFA_HORZSEPARATOR;
        xfi[i++].ulOrientation = CFA_LEFT;

        cnrhSetFieldInfos(hwndCnr,
                          xfi,
                          i,             // array item count
                          FALSE,         // don't draw lines
                          1);            // return first column

        BEGIN_CNRINFO()
        {
            cnrhSetView(CV_DETAIL | CA_DETAILSVIEWTITLES);
        } END_CNRINFO(hwndCnr);
    }

    if (flFlags & CBI_SET)
    {
        // initialize container with currently sticky windows
        PAGERCONFIG* pPagerCfg = (PAGERCONFIG*)pnbp->pUser;
        USHORT          us;

        HWND hwndCnr = WinWindowFromID(pnbp->hwndDlgPage,
                                       ID_SCDI_STICKY_CNR);

        cnrhRemoveAll(hwndCnr);

        for (us = 0;
             us < pPagerCfg->cStickies;
             us++)
        {
            AddStickyRecord(hwndCnr,
                            pPagerCfg->aszStickies[us],
                            pPagerCfg->aulStickyFlags[us],
                            FALSE);
            cnrhInvalidateAll(hwndCnr);
        }

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SCDI_PGR1_STICKIESTOGGLE,
                              !!(pPagerCfg->flPager & PGRFL_ADDSTICKYTOGGLE));
    }

    if (flFlags & CBI_ENABLE)
    {
        BOOL fPager = cmnQuerySetting(sfEnableXPager);
        PAGERCONFIG* pPagerCfg = (PAGERCONFIG*)pnbp->pUser;

        WinShowWindow(WinWindowFromID(pnbp->hwndDlgPage, ID_SCDI_PGR2_DISABLED_INFO),
                      !fPager);

        WinEnableControl(pnbp->hwndDlgPage,
                         ID_SCDI_STICKY_CNR,
                         fPager);
        WinEnableControl(pnbp->hwndDlgPage,
                         DID_ADD,
                         fPager && pPagerCfg->cStickies < MAX_STICKIES);
        WinEnableControl(pnbp->hwndDlgPage,
                         DID_EDIT,
                         fPager && pPagerCfg->cStickies != 0);
        WinEnableControl(pnbp->hwndDlgPage,
                         DID_REMOVE,
                         fPager && pPagerCfg->cStickies != 0);

        EnableNotebookButtons(pnbp,
                              fPager);
    }
}

/*
 *@@ PagerStickyItemChanged:
 *      notebook callback function (notebook.c) for the
 *      "XPager Sticky Windows" page in the "Screen" settings object.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@added V0.9.4 (2000-07-10) [umoeller]
 *@@changed V0.9.11 (2001-04-25) [umoeller]: no longer listing invisible switchlist entries
 *@@changed V0.9.19 (2002-04-14) [lafaix]: using new sticky settings dialog
 *@@changed V0.9.19 (2002-04-16) [lafaix]: fixed popup menu font and DID_UNDO
 *@@changed V1.0.0 (2002-09-14) [lafaix]: added support for PGRFL_ADDSTICKYTOGGLE
 */

STATIC MRESULT PagerStickyItemChanged(PNOTEBOOKPAGE pnbp,
                                      ULONG ulItemID, USHORT usNotifyCode,
                                      ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = 0;

    HWND hwndCnr = WinWindowFromID(pnbp->hwndDlgPage,
                                   ID_SCDI_STICKY_CNR);

    switch (ulItemID)
    {
        case ID_SCDI_STICKY_CNR:
            switch (usNotifyCode)
            {
                /*
                 * CN_CONTEXTMENU:
                 *
                 */

                case CN_CONTEXTMENU:
                {
                    HWND    hPopupMenu = NULLHANDLE;

                    LoadPagerConfig(pnbp->pUser); // V0.9.19 (2002-04-23) [pr]
                    // in the CREATENOTEBOOKPAGE structure
                    // so that the notebook.c function can
                    // remove source emphasis later automatically
                    pnbp->hwndSourceCnr = pnbp->hwndControl;
                    if (pnbp->preccSource = (PRECORDCORE)ulExtra)
                    {
                        // popup menu on container recc:
                        // disabling "add" item if sticky array full
                        if (hPopupMenu = cmnLoadMenu(pnbp->hwndDlgPage, // V1.0.1 (2003-01-05) [umoeller]
                                                     cmnQueryNLSModuleHandle(FALSE),
                                                     ID_XFM_CNRITEM_SEL))
                            WinEnableMenuItem(hPopupMenu,
                                              ID_XFMI_CNRITEM_NEW,
                                              (((PAGERCONFIG*)pnbp->pUser)->cStickies < MAX_STICKIES));
                    }
                    else
                    {
                        // popup menu on cnr whitespace
                        // disabling "add" item if sticky array full
                        if (hPopupMenu = cmnLoadMenu(pnbp->hwndDlgPage, // V1.0.1 (2003-01-05) [umoeller]
                                                     cmnQueryNLSModuleHandle(FALSE),
                                                     ID_XFM_CNRITEM_NOSEL))
                            WinEnableMenuItem(hPopupMenu,
                                              ID_XFMI_CNRITEM_NEW,
                                              (((PAGERCONFIG*)pnbp->pUser)->cStickies < MAX_STICKIES));
                    }

                    if (hPopupMenu)
                        cnrhShowContextMenu(pnbp->hwndControl,  // cnr
                                            (PRECORDCORE)pnbp->preccSource,
                                            hPopupMenu,
                                            pnbp->hwndDlgPage);    // owner
                }
                break;

                /*
                 * CN_ENTER:
                 *      ulExtra has the record that was clicked on.
                 */

                case CN_ENTER:
                    // this crashed if the user double-clicked on
                    // cnr whitespace since the record was then NULL
                    if (ulExtra)        // V1.0.0 (2002-08-28) [umoeller]
                        EditStickyRecord((PSTICKYRECORD)ulExtra,
                                         pnbp,
                                         hwndCnr,
                                         FALSE); // do not create a new record
                break;
            }
        break;

        /*
         * ID_XFMI_CNRITEM_NEW:
         *      show "New sticky window" dialog and add
         *      a new sticky window from that dialog.
         */

        case DID_ADD:
        case ID_XFMI_CNRITEM_NEW:
        {
            STICKYRECORD rec;
            memset(&rec, 0, sizeof(rec));
            EditStickyRecord(&rec,
                             pnbp,
                             hwndCnr,
                             TRUE); // create a new record if needed
            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
        }
        break;

        /*
         * ID_XFMI_CNRITEM_EDIT:
         *      show "Edit sticky window entry" dialog and edit
         *      the entry from that dialog
         *      (menu item command).
         */

        case ID_XFMI_CNRITEM_EDIT:
            EditStickyRecord((PSTICKYRECORD)pnbp->preccSource,
                             pnbp,
                             hwndCnr,
                             FALSE); // do not create a new record
        break;

        /*
         * ID_SCDI_STICKY_EDIT
         *      show "Edit sticky window entry" dialog and edit
         *      the currently selected entry from that dialog
         *      (button command).
         */

        case DID_EDIT:
        {
            // get current selected record
            PSTICKYRECORD pRec;
            if (    (pRec = (PSTICKYRECORD)WinSendMsg(hwndCnr,
                                                      CM_QUERYRECORDEMPHASIS,
                                                      (MPARAM)CMA_FIRST,
                                                      (MPARAM)CRA_SELECTED))
                 && ((LONG)pRec != -1L)
               )
                EditStickyRecord(pRec,
                                 pnbp,
                                 hwndCnr,
                                 FALSE); // do not create a new record
        }
        break;

        /*
         * ID_XFMI_CNRITEM_DELETE:
         *      remove sticky window record
         *      (menu item command).
         */

        case ID_XFMI_CNRITEM_DELETE:
            WinSendMsg(hwndCnr,
                       CM_REMOVERECORD,
                       &(pnbp->preccSource), // double pointer...
                       MPFROM2SHORT(1, CMA_FREE | CMA_INVALIDATE));
            SaveStickies(hwndCnr,
                         (PAGERCONFIG*)pnbp->pUser);
            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
        break;

        /*
         * ID_SCDI_STICKY_REMOVE:
         *      remove the currently selected entry
         *      (button command).
         */

        case DID_REMOVE:
        {
            // get current selected record
            PSTICKYRECORD pRec;
            if (    (pRec = (PSTICKYRECORD)WinSendMsg(hwndCnr,
                                                      CM_QUERYRECORDEMPHASIS,
                                                      (MPARAM)CMA_FIRST,
                                                      (MPARAM)CRA_SELECTED))
                 && ((LONG)pRec != -1L)
               )
            {
                WinSendMsg(hwndCnr,
                           CM_REMOVERECORD,
                           &pRec, // double pointer...
                           MPFROM2SHORT(1, CMA_FREE | CMA_INVALIDATE));
                SaveStickies(hwndCnr,
                             (PAGERCONFIG*)pnbp->pUser);
                pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
            }
        }
        break;

        case ID_SCDI_PGR1_STICKIESTOGGLE:
        {
            PAGERCONFIG* pPagerCfg = (PAGERCONFIG*)pnbp->pUser;
            LoadPagerConfig(pnbp->pUser);
            if (ulExtra)
                pPagerCfg->flPager |= PGRFL_ADDSTICKYTOGGLE;
            else
                pPagerCfg->flPager &= ~PGRFL_ADDSTICKYTOGGLE;

            // SavePagerConfig is cheaper than SaveStickies
            SavePagerConfig(pPagerCfg, 0);

            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
        }
        break;

        /*
         * DID_UNDO:
         *      "Undo" button.
         */

        case DID_UNDO:
        {
            PAGERCONFIG* pPagerCfg = (PAGERCONFIG*)pnbp->pUser;
            PAGERCONFIG* pBackup = (PAGERCONFIG*)pnbp->pUser2;
            USHORT       us;

            LoadPagerConfig(pnbp->pUser); // V0.9.19 (2002-04-23) [pr]
            // overwrite entire string array with backup
            memcpy(pPagerCfg->aszStickies,
                   pBackup->aszStickies,
                   sizeof(pPagerCfg->aszStickies));
            // overwrite entire flags array
            memcpy(pPagerCfg->aulStickyFlags,
                   pBackup->aulStickyFlags,
                   sizeof(pPagerCfg->aulStickyFlags));
            // and count too
            pPagerCfg->cStickies = pBackup->cStickies;
            // and now also flags
            pPagerCfg->flPager =
                (pPagerCfg->flPager
                 & ~PGRMASK_PAGE3
                ) | (pBackup->flPager & PGRMASK_PAGE3);

            // SavePagerConfig is cheaper that SaveStickies: we don't
            // have to update the container first
            SavePagerConfig(pPagerCfg,
                            PGRCFG_REPAINT
                               | PGRCFG_REFORMAT
                               | PGRCFG_STICKIES);

            // call INIT callback to reinitialize page
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        }
        break;

        case ID_XFMI_CNRITEM_DELETEALL:
            cnrhRemoveAll(hwndCnr);
            SaveStickies(hwndCnr,
                         (PAGERCONFIG*)pnbp->pUser);
            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
        break;

        /*
         * DID_DEFAULT:
         *      "Clear" button.
         */

        case DID_DEFAULT:
        {
            PAGERCONFIG* pPagerCfg = (PAGERCONFIG*)pnbp->pUser;

            LoadPagerConfig(pnbp->pUser); // V0.9.19 (2002-04-23) [pr]
            cnrhRemoveAll(hwndCnr);

            pPagerCfg->flPager =
                (pPagerCfg->flPager
                 & ~PGRMASK_PAGE3
                 ) | PGRFL_PAGE3_DEFAULTS;

            // calls SavePagerConfig first because SaveStickies reloads
            // the config!
            SavePagerConfig(pPagerCfg, 0);

            SaveStickies(hwndCnr,
                         (PAGERCONFIG*)pnbp->pUser);
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        }
        break;
    }

    return mrc;
}

/* ******************************************************************
 *
 *   XPager Colors page notebook functions (notebook.c)
 *
 ********************************************************************/

#pragma pack(1)

static const struct
    {
        SHORT   id,         // ID of static frame
                idRow,      // ID of the static row title text
                idColumn;   // ID of the static column title text
    } G_aColorIDs[] =
    {
         ID_SCDI_PGR2_DTP_INACTIVE_1,
            ID_SCDI_PGR2_COLORS_BACKGROUND, ID_SCDI_PGR2_COLORS_INACTIVE_1,
         ID_SCDI_PGR2_DTP_INACTIVE_2,
            ID_SCDI_PGR2_COLORS_BACKGROUND, ID_SCDI_PGR2_COLORS_INACTIVE_2,
         ID_SCDI_PGR2_DTP_ACTIVE,
            ID_SCDI_PGR2_COLORS_BACKGROUND, ID_SCDI_PGR2_COLORS_ACTIVE,
         ID_SCDI_PGR2_DTP_GRID,
            ID_SCDI_PGR2_COLORS_BACKGROUND, ID_SCDI_PGR2_COLORS_BORDERS,

         ID_SCDI_PGR2_WIN_INACTIVE,
            ID_SCDI_PGR2_COLORS_MINIWINDOW, ID_SCDI_PGR2_COLORS_INACTIVE_1,
         ID_SCDI_PGR2_WIN_ACTIVE,
            ID_SCDI_PGR2_COLORS_MINIWINDOW, ID_SCDI_PGR2_COLORS_ACTIVE,
         ID_SCDI_PGR2_WIN_BORDER,
            ID_SCDI_PGR2_COLORS_MINIWINDOW, ID_SCDI_PGR2_COLORS_BORDERS,

         ID_SCDI_PGR2_TXT_INACTIVE,
            ID_SCDI_PGR2_COLORS_TITLE, ID_SCDI_PGR2_COLORS_INACTIVE_1,
         ID_SCDI_PGR2_TXT_ACTIVE,
            ID_SCDI_PGR2_COLORS_TITLE, ID_SCDI_PGR2_COLORS_ACTIVE,
    };

#pragma pack()

/*
 *@@ GetColorPointer:
 *
 *@@added V0.9.3 (2000-04-09) [umoeller]
 */

STATIC PLONG GetColorPointer(HWND hwndStatic,
                             PAGERCONFIG* pPagerCfg)
{
    switch (WinQueryWindowUShort(hwndStatic, QWS_ID))
    {
        case ID_SCDI_PGR2_DTP_INACTIVE_1:
            return &pPagerCfg->lcolDesktop1;

        case ID_SCDI_PGR2_DTP_INACTIVE_2:
            return &pPagerCfg->lcolDesktop2;

        case ID_SCDI_PGR2_DTP_ACTIVE:
            return &pPagerCfg->lcolActiveDesktop;

        case ID_SCDI_PGR2_DTP_GRID:
            return &pPagerCfg->lcolGrid;

        case ID_SCDI_PGR2_WIN_INACTIVE:
            return &pPagerCfg->lcolInactiveWindow;

        case ID_SCDI_PGR2_WIN_ACTIVE:
            return &pPagerCfg->lcolActiveWindow;

        case ID_SCDI_PGR2_WIN_BORDER:
            return &pPagerCfg->lcolWindowFrame;

        case ID_SCDI_PGR2_TXT_INACTIVE:
            return &pPagerCfg->lcolInactiveText;

        case ID_SCDI_PGR2_TXT_ACTIVE:
            return &pPagerCfg->lcolActiveText;
    }

    return NULL;
}

/*
 *@@ fnwpColorSelect:
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

STATIC MRESULT EXPENTRY fnwpColorSelect(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_INITDLG:
            WinSetWindowULong(hwndDlg, QWL_USER, (ULONG)mp2);
                    // we get the color static HWND in mp2
        break;

        case 0x0601:
        {
            HWND hwndStatic;
            if (hwndStatic = WinQueryWindowULong(hwndDlg, QWL_USER))
                winhSetPresColor(hwndStatic,
                                 PP_BACKGROUNDCOLOR,
                                 (ULONG)mp1);
        }
        break;

        // note: we do not handle WM_CLOSE so the control
        // can be reused; WM_CLOSE will simply dismiss the
        // dlg, but keep it around

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return mrc;
}

#define ID_COLORCTL         1000

/*
 *@@ SetColor:
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

STATIC VOID SetColor(HWND hwndColorDlg,
                     LONG lcol)
{
    WinSendMsg(WinWindowFromID(hwndColorDlg, ID_COLORCTL),
               0x0602,
               (MPARAM)lcol,
               0);
}

/*
 *@@ CreateColorDlg:
 *      creates a simple color selection dialog using
 *      the undocumented OS/2 color selection control.
 *
 *      This creates a dialog frame with the color
 *      selection only. The dialog has a close button
 *      and is not sizeable. It is created hidden to
 *      allow the caller to place it.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

STATIC HWND CreateColorDlg(HWND hwndOwner,
                           PFNWP pfnwpDialogProc,
                           PCSZ pcszTitle,
                           PVOID pCreateParams,
                           LONG lcolInitial)
{
    static const CONTROLDEF
        Color =
            {
                "ColorSelectClass",
                NULL,
                WS_VISIBLE,
                ID_COLORCTL,
                CTL_COMMON_FONT,
                {200, 200 * 60 / 100},
                COMMON_SPACING
            };
    static const DLGHITEM dlgColor[] =
        {
            START_TABLE,
                START_ROW(0),
                    CONTROL_DEF(&Color),
            END_TABLE
        };

    HWND hwndColor;
    if (!(dlghCreateDlg(&hwndColor,
                        hwndOwner,
                        FCF_FIXED_DLG,
                        pfnwpDialogProc,
                        pcszTitle,
                        dlgColor,
                        ARRAYITEMCOUNT(dlgColor),
                        pCreateParams,
                        cmnQueryDefaultFont())))
    {
        SetColor(hwndColor, lcolInitial);
        return hwndColor;
    }

    return NULLHANDLE;
}

/*
 *@@ fnwpSubclassedStaticRect:
 *      common window procedure for subclassed static
 *      frames representing XPager colors.
 *
 *@@added V0.9.3 (2000-04-09) [umoeller]
 *@@changed V0.9.7 (2001-01-17) [umoeller]: fixed inclusive rect bug
 *@@changed V0.9.19 (2002-05-28) [umoeller]: added color selection dlg on dblclk
 */

STATIC MRESULT EXPENTRY fnwpSubclassedStaticRect(HWND hwndStatic, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;
    // access settings; these have been stored in QWL_USER
    PNOTEBOOKPAGE pnbp = (PNOTEBOOKPAGE)WinQueryWindowPtr(hwndStatic, QWL_USER);
    // get PAGERCONFIG which has been stored in user param there
    PAGERCONFIG* pPagerCfg = (PAGERCONFIG*)pnbp->pUser;

    switch (msg)
    {
        case WM_PAINT:
        {
            PLONG   plColor;
            RECTL   rclPaint;
            BOOL    fEnabled;
            HPS hps = WinBeginPaint(hwndStatic,
                                    NULLHANDLE, // HPS
                                    NULL); // PRECTL
            gpihSwitchToRGB(hps);
            WinQueryWindowRect(hwndStatic,
                               &rclPaint);      // exclusive

            if (!(fEnabled = WinIsWindowEnabled(hwndStatic)))
                GpiSetPattern(hps, PATSYM_HALFTONE);

            if (plColor = GetColorPointer(hwndStatic, pPagerCfg))
            {
                // make rect inclusive
                rclPaint.xRight--;
                rclPaint.yTop--;

                // draw interior
                GpiSetColor(hps, *plColor);
                gpihBox(hps,
                        DRO_FILL,
                        &rclPaint);

                // draw frame
                if (fEnabled)
                {
                    GpiSetColor(hps, RGBCOL_BLACK);
                    gpihBox(hps,
                            DRO_OUTLINE,
                            &rclPaint);
                }
            }

            WinEndPaint(hps);
        }
        break;

        case WM_PRESPARAMCHANGED:
            switch ((ULONG)mp1)
            {
                case PP_BACKGROUNDCOLOR:
                {
                    PLONG plColor;
                    if (plColor = GetColorPointer(hwndStatic, pPagerCfg))
                    {
                        ULONG   ul = 0,
                                attrFound = 0;

                        LoadPagerConfig(pnbp->pUser); // V0.9.19 (2002-04-23) [pr]
                        WinQueryPresParam(hwndStatic,
                                          (ULONG)mp1,
                                          0,
                                          &attrFound,
                                          (ULONG)sizeof(ul),
                                          (PVOID)&ul,
                                          0);
                        *plColor = ul;
                        WinInvalidateRect(hwndStatic,
                                          NULL, FALSE);

                        if (pnbp->flPage & NBFL_PAGE_INITED)   // page initialized yet?
                            SavePagerConfig(pPagerCfg,
                                            PGRCFG_REPAINT);
                    }
                }
                break;
            }
        break;

        case WM_BUTTON1DBLCLK:
        {
            PLONG plColor;
            PPAGERPAGEDATA pData;
            if (    (plColor = GetColorPointer(hwndStatic, pPagerCfg))
                 && (pData = (PPAGERPAGEDATA)pnbp->pUser2)
               )
            {
                CHAR szDlgTitle[300] = "error";

                // compose title from row and column headings;
                // find the array item for this static in the
                // global array
                USHORT idThis = WinQueryWindowUShort(hwndStatic, QWS_ID);
                ULONG ul;

                for (ul = 0;
                     ul < ARRAYITEMCOUNT(G_aColorIDs);
                     ++ul)
                {
                    if (G_aColorIDs[ul].id == idThis)
                    {
                        sprintf(szDlgTitle,
                                "%s %s",
                                cmnGetString(G_aColorIDs[ul].idRow),
                                cmnGetString(G_aColorIDs[ul].idColumn));
                        break;
                    }
                }

                // if we have a color dialog already, use that
                if (pData->hwndColorDlg)
                {
                    // QWL_USER has the static which currently owns
                    // the dialog
                    WinSetWindowULong(pData->hwndColorDlg,
                                      QWL_USER,
                                      hwndStatic);
                    SetColor(pData->hwndColorDlg,
                             *plColor);
                    // refresh title
                    WinSetWindowText(pData->hwndColorDlg,
                                     szDlgTitle);
                }
                else
                    pData->hwndColorDlg = CreateColorDlg(pnbp->hwndDlgPage,
                                                         fnwpColorSelect,
                                                         szDlgTitle,
                                                         (PVOID)hwndStatic,
                                                         *plColor);
                winhPlaceBesides(pData->hwndColorDlg,
                                 hwndStatic,
                                 PLF_SMART);
                WinShowWindow(pData->hwndColorDlg, TRUE);
            }
        }
        break;

        default:
            mrc = G_pfnwpOrigStatic(hwndStatic, msg, mp1, mp2);
    }

    return mrc;
}

#define COLORBOX_HEIGHT 12

#define COLORBOX(id)    CONTROLDEF_TEXT("", id, 32, COLORBOX_HEIGHT)
#define TEXTBOX(id)     CONTROLDEF_TEXT(LOAD_STRING, id, SZL_AUTOSIZE, COLORBOX_HEIGHT)

static const CONTROLDEF
    ColorsGroup = LOADDEF_GROUP(ID_SCDI_PGR2_COLORS_GROUP, SZL_AUTOSIZE),
    ColorsInfo = LOADDEF_TEXT_WORDBREAK(ID_SCDI_PGR2_COLORS_INFO, -100),
    TextEmpty = CONTROLDEF_TEXT("", -1, 10, COLORBOX_HEIGHT),
    // colum titles
    TextInactive1 = TEXTBOX(ID_SCDI_PGR2_COLORS_INACTIVE_1),
    TextInactive2 = TEXTBOX(ID_SCDI_PGR2_COLORS_INACTIVE_2),
    TextActive = TEXTBOX(ID_SCDI_PGR2_COLORS_ACTIVE),
    TextBorders = TEXTBOX(ID_SCDI_PGR2_COLORS_BORDERS),
    // row titles
    TextBackground = TEXTBOX(ID_SCDI_PGR2_COLORS_BACKGROUND),
    TextMiniWindow = TEXTBOX(ID_SCDI_PGR2_COLORS_MINIWINDOW),
    TextTitle = TEXTBOX(ID_SCDI_PGR2_COLORS_TITLE),

    ColorDtpInactive1 = COLORBOX(ID_SCDI_PGR2_DTP_INACTIVE_1),
    ColorDtpInactive2 = COLORBOX(ID_SCDI_PGR2_DTP_INACTIVE_2),
    ColorDtpActive    = COLORBOX(ID_SCDI_PGR2_DTP_ACTIVE),
    ColorDtpGrid      = COLORBOX(ID_SCDI_PGR2_DTP_GRID),
    ColorWinInactive  = COLORBOX(ID_SCDI_PGR2_WIN_INACTIVE),
    ColorWinActive    = COLORBOX(ID_SCDI_PGR2_WIN_ACTIVE),
    ColorWinBorder    = COLORBOX(ID_SCDI_PGR2_WIN_BORDER),
    ColorTxtInactive  = COLORBOX(ID_SCDI_PGR2_TXT_INACTIVE),
    ColorTxtActive    = COLORBOX(ID_SCDI_PGR2_TXT_ACTIVE),
    ColorNull         = COLORBOX(-1);

static const DLGHITEM G_dlgXPagerColors[] =
    {
        START_TABLE,
            START_ROW(0),
                CONTROL_DEF(&G_PagerDisabled),
            START_ROW(0),
                START_GROUP_TABLE(&ColorsGroup),
                    START_ROW(0),
                        CONTROL_DEF(&ColorsInfo),
                    START_ROW(0),
                        START_TABLE,
                            START_ROW(0),
                                // first colum: row titles
                                START_TABLE,
                                    START_ROW(0),
                                        CONTROL_DEF(&TextEmpty),
                                    START_ROW(0),
                                        CONTROL_DEF(&TextBackground),
                                    START_ROW(0),
                                        CONTROL_DEF(&TextMiniWindow),
                                    START_ROW(0),
                                        CONTROL_DEF(&TextTitle),
                                END_TABLE,
                                // second column: inactive 1
                                START_TABLE,
                                    START_ROW(0),
                                        CONTROL_DEF(&TextInactive1),
                                    START_ROW(0),
                                        CONTROL_DEF(&ColorDtpInactive1),
                                    START_ROW(0),
                                        CONTROL_DEF(&ColorWinInactive),
                                    START_ROW(0),
                                        CONTROL_DEF(&ColorTxtInactive),
                                END_TABLE,
                                // third column: inactive 2
                                START_TABLE,
                                    START_ROW(0),
                                        CONTROL_DEF(&TextInactive2),
                                    START_ROW(0),
                                        CONTROL_DEF(&ColorDtpInactive2),
                                    START_ROW(0),
                                        CONTROL_DEF(&ColorNull),
                                    START_ROW(0),
                                        CONTROL_DEF(&ColorNull),
                                END_TABLE,
                                // fourth column: active
                                START_TABLE,
                                    START_ROW(0),
                                        CONTROL_DEF(&TextActive),
                                    START_ROW(0),
                                        CONTROL_DEF(&ColorDtpActive),
                                    START_ROW(0),
                                        CONTROL_DEF(&ColorWinActive),
                                    START_ROW(0),
                                        CONTROL_DEF(&ColorTxtActive),
                                END_TABLE,
                                // fifth column: borders
                                START_TABLE,
                                    START_ROW(0),
                                        CONTROL_DEF(&TextBorders),
                                    START_ROW(0),
                                        CONTROL_DEF(&ColorDtpGrid),
                                    START_ROW(0),
                                        CONTROL_DEF(&ColorWinBorder),
                                    START_ROW(0),
                                        CONTROL_DEF(&ColorNull),
                                END_TABLE,
                        END_TABLE,
                END_TABLE,
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE,
    };

/*
 *@@ PagerColorsInitPage:
 *      notebook callback function (notebook.c) for the
 *      "XPager Colors" page in the "Screen" settings object.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@changed V0.9.19 (2002-05-07) [umoeller]: now using dialog formatter; updated for new colors
 */

STATIC VOID PagerColorsInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                                     ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        ULONG ul = 0;
        PPAGERPAGEDATA pData;

        // first call: create PAGERCONFIG
        // structure;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        if (pnbp->pUser = malloc(sizeof(PAGERCONFIG)))
            LoadPagerConfig(pnbp->pUser);

        // make backup for "undo"
        if (pData = pnbp->pUser2 = NEW(PAGERPAGEDATA))
        {
            memcpy(&pData->PgrConfig, pnbp->pUser, sizeof(PAGERCONFIG));
            pData->hwndColorDlg = NULLHANDLE;
        }

        // insert the controls using the dialog formatter
        // V0.9.19 (2002-05-07) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      G_dlgXPagerColors,
                      ARRAYITEMCOUNT(G_dlgXPagerColors));

        // subclass static rectangles
        for (ul = 0;
             ul < ARRAYITEMCOUNT(G_aColorIDs);
             ++ul)
        {
            HWND    hwndFrame = WinWindowFromID(pnbp->hwndDlgPage,
                                                G_aColorIDs[ul].id);
            // store pcnbp in QWL_USER of that control
            // so the control knows about its purpose and can
            // access the PAGERCONFIG data
            WinSetWindowPtr(hwndFrame, QWL_USER, (PVOID)pnbp);
            // subclass this control
            G_pfnwpOrigStatic = WinSubclassWindow(hwndFrame,
                                                  fnwpSubclassedStaticRect);
        }
    }

    if (flFlags & CBI_SET)
    {
        ULONG ul = 0;
        // repaint all static controls
        for (ul = 0;
             ul < ARRAYITEMCOUNT(G_aColorIDs);
             ul++)
        {
            WinInvalidateRect(WinWindowFromID(pnbp->hwndDlgPage,
                                              G_aColorIDs[ul].id),
                              NULL,
                              FALSE);
        }
    }

    if (flFlags & CBI_ENABLE)
    {
        static const ULONG aulPager[] =
            {
                ID_SCDI_PGR2_COLORS_GROUP,
                ID_SCDI_PGR2_COLORS_INFO,
                ID_SCDI_PGR2_COLORS_INACTIVE_1,
                ID_SCDI_PGR2_COLORS_INACTIVE_2,
                ID_SCDI_PGR2_COLORS_ACTIVE,
                ID_SCDI_PGR2_COLORS_BORDERS,
                ID_SCDI_PGR2_COLORS_BACKGROUND,
                ID_SCDI_PGR2_COLORS_MINIWINDOW,
                ID_SCDI_PGR2_COLORS_TITLE,
                ID_SCDI_PGR2_DTP_INACTIVE_1,
                ID_SCDI_PGR2_DTP_INACTIVE_2,
                ID_SCDI_PGR2_DTP_ACTIVE,
                ID_SCDI_PGR2_DTP_GRID,
                ID_SCDI_PGR2_WIN_INACTIVE,
                ID_SCDI_PGR2_WIN_ACTIVE,
                ID_SCDI_PGR2_WIN_BORDER,
                ID_SCDI_PGR2_TXT_INACTIVE,
                ID_SCDI_PGR2_TXT_ACTIVE,
            };
        BOOL fPager = cmnQuerySetting(sfEnableXPager);
        WinShowWindow(WinWindowFromID(pnbp->hwndDlgPage, ID_SCDI_PGR2_DISABLED_INFO),
                      !fPager);
        winhEnableControls2(pnbp->hwndDlgPage,
                            aulPager,
                            ARRAYITEMCOUNT(aulPager),
                            fPager);
        EnableNotebookButtons(pnbp,
                              fPager);
    }

    if (flFlags & CBI_DESTROY)
    {
        PPAGERPAGEDATA pData;
        if (    (pData = pnbp->pUser2)
             && (pData->hwndColorDlg)
           )
        {
            WinDestroyWindow(pData->hwndColorDlg);
            pData->hwndColorDlg = NULLHANDLE;
        }
    }
}

/*
 *@@ PagerColorsItemChanged:
 *      notebook callback function (notebook.c) for the
 *      "XPager Colors" page in the "Screen" settings object.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@changed V0.9.19 (2002-05-07) [umoeller]: now using dialog formatter; updated for new colors
 */

STATIC MRESULT PagerColorsItemChanged(PNOTEBOOKPAGE pnbp,
                                      ULONG ulItemID, USHORT usNotifyCode,
                                      ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = 0;

    switch (ulItemID)
    {
        /*
         * DID_DEFAULT:
         *
         */

        case DID_DEFAULT:
        {
            PAGERCONFIG* pPagerCfg = (PAGERCONFIG*)pnbp->pUser;
            LoadPagerConfig(pnbp->pUser);
            pPagerCfg->flPaintMode = PMOD_TOPBOTTOM;
            pPagerCfg->lcolDesktop1 = RGBCOL_DARKBLUE;
            pPagerCfg->lcolDesktop2 = RGBCOL_BLACK;
            pPagerCfg->lcolActiveDesktop = RGBCOL_BLUE;
            pPagerCfg->lcolGrid = RGBCOL_GRAY;
            pPagerCfg->lcolInactiveWindow = RGBCOL_WHITE;
            pPagerCfg->lcolActiveWindow = RGBCOL_GREEN;
            pPagerCfg->lcolWindowFrame = RGBCOL_BLACK;
            pPagerCfg->lcolInactiveText = RGBCOL_BLACK;
            pPagerCfg->lcolActiveText = RGBCOL_BLACK;

            // call INIT callback to reinitialize page
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);

            SavePagerConfig(pPagerCfg,
                               PGRCFG_REPAINT | PGRCFG_REFORMAT);
        }
        break;

        /*
         * DID_UNDO:
         *
         */

        case DID_UNDO:
        {
            PAGERCONFIG* pPagerCfg = (PAGERCONFIG*)pnbp->pUser;
            PPAGERPAGEDATA pData = (PPAGERPAGEDATA)pnbp->pUser2;

            LoadPagerConfig(pnbp->pUser);
            #define RESTORELONG(l) pPagerCfg->l = pData->PgrConfig.l
            RESTORELONG(flPaintMode);
            RESTORELONG(lcolDesktop1);
            RESTORELONG(lcolDesktop2);
            RESTORELONG(lcolActiveDesktop);
            RESTORELONG(lcolGrid);
            RESTORELONG(lcolInactiveWindow);
            RESTORELONG(lcolActiveWindow);
            RESTORELONG(lcolWindowFrame);
            RESTORELONG(lcolInactiveText);
            RESTORELONG(lcolActiveText);

            // call INIT callback to reinitialize page
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);

            SavePagerConfig(pPagerCfg,
                               PGRCFG_REPAINT | PGRCFG_REFORMAT);
        }
        break;
    }

    return mrc;
}

/*
 *@@ pgmiInsertPagerPages:
 *      implementation for XWPScreen::xwpAddXWPScreenPages,
 *      as far as the pager config pages are concerned.
 *
 *@@added V0.9.19 (2002-04-17) [umoeller]
 *@@changed V0.9.19 (2002-04-19) [pr]: page wasn't showing "page 2/4" etc; fixed
 */

ULONG pgmiInsertPagerPages(WPObject *somSelf,       // in: screen object
                           HWND hwndDlg,            // in: notebook
                           HMODULE savehmod)        // in: NLS module
{
    INSERTNOTEBOOKPAGE  inbp;

    // "XPager" colors
    memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
    inbp.somSelf = somSelf;
    inbp.hwndNotebook = hwndDlg;
    inbp.hmod = savehmod;
    inbp.pfncbInitPage    = PagerColorsInitPage;
    inbp.pfncbItemChanged = PagerColorsItemChanged;
    inbp.usPageStyleFlags = BKA_MINOR;
    inbp.fEnumerate = TRUE;
    inbp.ulDlgID = ID_XFD_EMPTYDLG; // ID_SCD_PAGER_COLORS;
    inbp.ulDefaultHelpPanel  = ID_XSH_SETTINGS_PAGER_COLORS;
    // give this page a unique ID, which is
    // passed to the common config.sys callbacks
    inbp.ulPageID = SP_PAGER_COLORS;
    ntbInsertPage(&inbp);

    // "XPager" sticky windows
    memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
    inbp.somSelf = somSelf;
    inbp.hwndNotebook = hwndDlg;
    inbp.hmod = savehmod;
    inbp.pfncbInitPage    = PagerStickyInitPage;
    inbp.pfncbItemChanged = PagerStickyItemChanged;
    inbp.usPageStyleFlags = BKA_MINOR;
    inbp.fEnumerate = TRUE;
    inbp.ulDlgID = ID_XFD_EMPTYDLG; // ID_SCD_PAGER_STICKY; V0.9.19 (2002-04-17) [umoeller]
    inbp.ulDefaultHelpPanel  = ID_XSH_SETTINGS_PAGER_STICKY;
    // make this sizeable V0.9.19 (2002-04-17) [umoeller]
    inbp.pampControlFlags = G_ampStickies;
    inbp.cControlFlags = ARRAYITEMCOUNT(G_ampStickies);
    // give this page a unique ID, which is
    // passed to the common config.sys callbacks
    inbp.ulPageID = SP_PAGER_STICKY;
    ntbInsertPage(&inbp);

    // "XPager" window settings V0.9.9 (2001-03-15) [lafaix]
    memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
    inbp.somSelf = somSelf;
    inbp.hwndNotebook = hwndDlg;
    inbp.hmod = savehmod;
    inbp.pfncbInitPage    = PagerWindowInitPage;
    inbp.pfncbItemChanged = PagerWindowItemChanged;
    inbp.usPageStyleFlags = BKA_MINOR;
    inbp.fEnumerate = TRUE;
    inbp.ulDlgID = ID_XFD_EMPTYDLG; // ID_SCD_PAGER_WINDOW; V0.9.19 (2002-04-17) [umoeller]
    inbp.ulDefaultHelpPanel  = ID_XSH_SETTINGS_PAGER_WINDOW;
    // give this page a unique ID, which is
    // passed to the common config.sys callbacks
    inbp.ulPageID = SP_PAGER_WINDOW;
    ntbInsertPage(&inbp);

    // "XPager" general settings
    memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
    inbp.somSelf = somSelf;
    inbp.hwndNotebook = hwndDlg;
    inbp.hmod = savehmod;
    inbp.pfncbInitPage    = PagerGeneralInitPage;
    inbp.pfncbItemChanged = PagerGeneralItemChanged;
    inbp.usPageStyleFlags = BKA_MAJOR;
    inbp.fEnumerate = TRUE;
    inbp.pcszName = ENTITY_PAGER;
    inbp.ulDlgID = ID_XFD_EMPTYDLG; // ID_SCD_PAGER_GENERAL; V0.9.19 (2002-05-28) [umoeller]
    inbp.ulDefaultHelpPanel  = ID_XSH_SETTINGS_PAGER_GENERAL;
    // give this page a unique ID, which is
    // passed to the common config.sys callbacks
    inbp.ulPageID = SP_PAGER_MAIN;
    return ntbInsertPage(&inbp);
}

#endif // __NOPAGER__
