
/*
 *@@sourcefile ctr_notebook.c:
 *      XCenter notebook pages.
 *
 *      Function prefix for this file:
 *      --  ctrp* also.
 *
 *      This is all new with V0.9.7. With V1.0.0,
 *      instance setup has been moved to the new
 *      ctr_setup.c.
 *
 *@@added V0.9.7 (2000-11-27) [umoeller]
 *@@header "shared\center.h"
 */

/*
 *      Copyright (C) 2000-2006 Ulrich M”ller.
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
#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES      // V0.9.20 (2002-07-23) [lafaix] needed for plugins.h
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMENUS
#define INCL_WINDIALOGS
#define INCL_WINBUTTONS
#define INCL_WINSTATICS
#define INCL_WINSTDCNR
#define INCL_WINSTDSLIDER
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\apmh.h"               // Advanced Power Management helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
#include "xcenter.ih"
// #include "xfobj.ih"                     // XFldObject

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\notebook.h"            // generic XWorkplace notebook handling
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)
#include "shared\plugins.h"             // generic plugins support

#include "shared\center.h"              // public XCenter interfaces
#include "xcenter\centerp.h"            // private XCenter implementation

#include "hook\xwphook.h"
#include "config\hookintf.h"            // daemon/hook interface

#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   "View" page notebook callbacks (notebook.c)
 *
 ********************************************************************/

// two arrays to specify the affected settings with
// each "view" page; used with cmnSetupSetDefaults etc.

static ULONG    G_aulView1SetupOffsets[]
    = {
            FIELDOFFSET(XCenterData, fReduceDesktopWorkarea),
            FIELDOFFSET(XCenterData, ulPosition),
            FIELDOFFSET(XCenterData, ulWindowStyle),
            FIELDOFFSET(XCenterData, ulAutoHide),
            FIELDOFFSET(XCenterData, lPriorityDelta),   // V0.9.9 (2001-02-28) [pr]: added
            FIELDOFFSET(XCenterData, fAutoScreenBorder),   // V1.0.6 (2006-09-30) [pr]: added
            FIELDOFFSET(XCenterData, fHideOnClick)   // V1.0.6 (2006-09-30) [pr]: added
      };

static ULONG    G_aulView2SetupOffsets[]
    = {
            FIELDOFFSET(XCenterData, flDisplayStyle),
            FIELDOFFSET(XCenterData, ul3DBorderWidth),
            FIELDOFFSET(XCenterData, ulBorderSpacing),
            FIELDOFFSET(XCenterData, ulWidgetSpacing)
      };

static BOOL     G_fSetting = FALSE;

static SLDCDATA
        CtrDelaySliderCData =
             {
                     sizeof(SLDCDATA),
            // usScale1Increments:
                     60,        // scale 1 increments
                     0,         // scale 1 spacing
                     1,         // scale 2 increments
                     0          // scale 2 spacing
             },
        PrioritySliderCData =
             {
                     sizeof(SLDCDATA),
            // usScale1Increments:
                     32,        // scale 1 increments
                     0,         // scale 1 spacing
                     1,         // scale 2 increments
                     0          // scale 2 spacing
             };

#define VIEW_TABLE_WIDTH    200
#define HALF_TABLE_WIDTH    ((VIEW_TABLE_WIDTH / 2) - COMMON_SPACING - GROUP_INNER_SPACING_X)
#define DESCRTXT_WIDTH      15
#define SLIDER_WIDTH        (HALF_TABLE_WIDTH - 4 * COMMON_SPACING - DESCRTXT_WIDTH )

static const CONTROLDEF
    FrameGroup = LOADDEF_GROUP(ID_XRDI_VIEW_FRAMEGROUP, VIEW_TABLE_WIDTH),
    ReduceDesktopCB = LOADDEF_AUTOCHECKBOX(ID_CRDI_VIEW_REDUCEWORKAREA),
    AlwaysOnTopCB = LOADDEF_AUTOCHECKBOX(ID_CRDI_VIEW_ALWAYSONTOP),
    AnimateCB = LOADDEF_AUTOCHECKBOX(ID_CRDI_VIEW_ANIMATE),
    AutoHideCB = LOADDEF_AUTOCHECKBOX(ID_CRDI_VIEW_AUTOHIDE),
    CtrDelayTxt1 = LOADDEF_TEXT(ID_CRDI_VIEW_AUTOHIDE_TXT1),
    CtrDelaySlider = CONTROLDEF_SLIDER(ID_CRDI_VIEW_AUTOHIDE_SLIDER, 108, 14, &CtrDelaySliderCData),
    CtrDelayTxt2 = CONTROLDEF_TEXT_CENTER("TBR", ID_CRDI_VIEW_AUTOHIDE_TXT2, DESCRTXT_WIDTH, SZL_AUTOSIZE),
    AutoHideClickCB = LOADDEF_AUTOCHECKBOX(ID_CRDI_VIEW_AUTOHIDE_CLICK),
    AutoScreenBorderCB = LOADDEF_AUTOCHECKBOX(ID_CRDI_VIEW_AUTOSCREENBORDER),
    PriorityGroup = LOADDEF_GROUP(ID_CRDI_VIEW_PRTY_GROUP, HALF_TABLE_WIDTH),
    PrioritySlider = CONTROLDEF_SLIDER(ID_CRDI_VIEW_PRTY_SLIDER, SLIDER_WIDTH, 16, &PrioritySliderCData),
    PriorityTxt2 = CONTROLDEF_TEXT_CENTER("TBR", ID_CRDI_VIEW_PRTY_TEXT, DESCRTXT_WIDTH, SZL_AUTOSIZE),
    PositionGroup = LOADDEF_GROUP(ID_CRDI_VIEW_POSITION_GROUP, HALF_TABLE_WIDTH),
    TopOfScreenRadio = LOADDEF_FIRST_AUTORADIO(ID_CRDI_VIEW_TOPOFSCREEN),
    BottomOfScreenRadio = LOADDEF_NEXT_AUTORADIO(ID_CRDI_VIEW_BOTTOMOFSCREEN);

static const DLGHITEM G_dlgXCenterView[] =
    {
        START_TABLE,            // root table, required
            START_ROW(0),
                START_GROUP_TABLE(&FrameGroup),
                    START_ROW(0),
                        CONTROL_DEF(&ReduceDesktopCB),
                    START_ROW(0),
                        CONTROL_DEF(&AlwaysOnTopCB),
                    START_ROW(0),
                        CONTROL_DEF(&AnimateCB),
                    START_ROW(0),
                        CONTROL_DEF(&AutoHideCB),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&G_Spacing),
                        CONTROL_DEF(&CtrDelayTxt1),
                        CONTROL_DEF(&CtrDelaySlider),
                        CONTROL_DEF(&CtrDelayTxt2),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&G_Spacing),
                        CONTROL_DEF(&AutoHideClickCB),
                    START_ROW(0),
                        CONTROL_DEF(&AutoScreenBorderCB),
                END_TABLE,
            START_ROW(0),
                START_GROUP_TABLE(&PositionGroup),
                    START_ROW(0),
                        CONTROL_DEF(&TopOfScreenRadio),
                    START_ROW(0),
                        CONTROL_DEF(&BottomOfScreenRadio),
                END_TABLE,
                START_GROUP_TABLE(&PriorityGroup),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&PrioritySlider),
                        CONTROL_DEF(&PriorityTxt2),
                END_TABLE,
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };

/*
 *@@ ctrpView1InitPage:
 *      notebook callback function (notebook.c) for the
 *      first XCenter "View" instance settings page.
 *      Sets the controls on the page according to the
 *      instance settings.
 *
 *@@added V0.9.7 (2000-12-05) [umoeller]
 *@@changed V0.9.9 (2001-01-29) [umoeller]: "Undo" data wasn't working
 *@@changed V0.9.9 (2001-03-09) [umoeller]: added auto-hide delay slider
 *@@changed V0.9.14 (2001-08-21) [umoeller]: added "hide on click"
 *@@changed V0.9.19 (2002-05-07) [umoeller]: now using dlg formatter
 *@@changed V0.9.19 (2002-05-07) [umoeller]: added auto screen border
 *@@changed V0.9.19 (2002-06-08) [umoeller]: fixed wrong slider ticks for priority
 */

VOID ctrpView1InitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                       ULONG flFlags)        // CBI_* flags (notebook.h)
{
    XCenterData *somThis = XCenterGetData(pnbp->inbp.somSelf);

    if (flFlags & CBI_INIT)
    {
        // make backup of instance data for "Undo"
        XCenterData *pBackup;
        if (pBackup = (XCenterData*)malloc(sizeof(*somThis)))
            memcpy(pBackup, somThis, sizeof(*somThis));
            // be careful about using the copy... we have some pointers in there!
        // store in notebook struct
        pnbp->pUser = pBackup;

        // insert the controls using the dialog formatter
        // V0.9.19 (2002-05-07) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      G_dlgXCenterView,
                      ARRAYITEMCOUNT(G_dlgXCenterView));

        winhSetSliderTicks(WinWindowFromID(pnbp->hwndDlgPage,
                                           ID_CRDI_VIEW_AUTOHIDE_SLIDER),
                           MPFROM2SHORT(9, 10), 6,
                           (MPARAM)-1, -1);

        winhSetSliderTicks(WinWindowFromID(pnbp->hwndDlgPage,
                                           ID_CRDI_VIEW_PRTY_SLIDER),
                           (MPARAM)0, 3,
                           MPFROM2SHORT(0, 10), 6);
                                // fixed V0.9.19 (2002-06-08) [umoeller]
    }

    if (flFlags & CBI_SET)
    {
        LONG lSliderIndex = 0;
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_CRDI_VIEW_REDUCEWORKAREA,
                              _fReduceDesktopWorkarea);

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_CRDI_VIEW_ALWAYSONTOP,
                              ((_ulWindowStyle & WS_TOPMOST) != 0));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_CRDI_VIEW_ANIMATE,
                              ((_ulWindowStyle & WS_ANIMATE) != 0));

        // autohide
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_CRDI_VIEW_AUTOHIDE,
                              (_ulAutoHide > 0));
        if (_ulAutoHide)
            lSliderIndex = (_ulAutoHide / 1000) - 1;

        // prevent the stupid slider control from interfering
        G_fSetting = TRUE;
        winhSetSliderArmPosition(WinWindowFromID(pnbp->hwndDlgPage,
                                                 ID_CRDI_VIEW_AUTOHIDE_SLIDER),
                                 SMA_INCREMENTVALUE,
                                 lSliderIndex);
        G_fSetting = FALSE;

        WinSetDlgItemShort(pnbp->hwndDlgPage,
                           ID_CRDI_VIEW_AUTOHIDE_TXT2,
                           _ulAutoHide / 1000,
                           FALSE);      // unsigned

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_CRDI_VIEW_AUTOHIDE_CLICK,
                              _fHideOnClick);

        // V0.9.19 (2002-05-07) [umoeller]
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_CRDI_VIEW_AUTOSCREENBORDER,
                              _fAutoScreenBorder);

        // priority
        winhSetSliderArmPosition(WinWindowFromID(pnbp->hwndDlgPage,
                                                 ID_CRDI_VIEW_PRTY_SLIDER),
                                 SMA_INCREMENTVALUE,
                                 _lPriorityDelta);
        WinSetDlgItemShort(pnbp->hwndDlgPage,
                           ID_CRDI_VIEW_PRTY_TEXT,
                           _lPriorityDelta,
                           FALSE);      // unsigned

        if (_ulPosition == XCENTER_TOP)
            winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_CRDI_VIEW_TOPOFSCREEN, TRUE);
        else
            winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_CRDI_VIEW_BOTTOMOFSCREEN, TRUE);

    }

    if (flFlags & CBI_ENABLE)
    {
        static const ULONG aulIDs[] =
            {
                ID_CRDI_VIEW_AUTOHIDE_TXT1,
                ID_CRDI_VIEW_AUTOHIDE_SLIDER,
                ID_CRDI_VIEW_AUTOHIDE_TXT2,
                ID_CRDI_VIEW_AUTOHIDE_CLICK,
            };

        // disable auto-hide if workarea is to be reduced
        WinEnableControl(pnbp->hwndDlgPage, ID_CRDI_VIEW_AUTOHIDE,
                          (_fReduceDesktopWorkarea == FALSE));

        // replaced call here V1.0.0 (2002-08-12) [umoeller]
        winhEnableControls2(pnbp->hwndDlgPage,
                            aulIDs,
                            ARRAYITEMCOUNT(aulIDs),
                               (_fReduceDesktopWorkarea == FALSE)
                            && (_ulAutoHide != 0)
                           );
    }
}

/*
 *@@ ctrpView1ItemChanged:
 *      notebook callback function (notebook.c) for the
 *      XCenter "View"  instance settings page.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@added V0.9.7 (2000-12-05) [umoeller]
 *@@changed V0.9.9 (2001-01-29) [umoeller]: now using cmnSetup* funcs
 *@@changed V0.9.9 (2001-03-09) [umoeller]: added auto-hide delay slider
 *@@changed V0.9.14 (2001-08-21) [umoeller]: added "hide on click"
 *@@changed V0.9.19 (2002-04-17) [umoeller]: now automatically making XCenter screen border object
 *@@changed V1.0.1 (2002-11-30) [umoeller]: refresh screen border object only if the func is actually enabled @@fixes 252
 *@@changed V1.0.6 (2006-09-30) [pr]: adjust screen border settings @@fixes 224
 */

MRESULT ctrpView1ItemChanged(PNOTEBOOKPAGE pnbp,
                             ULONG ulItemID,
                             USHORT usNotifyCode,
                             ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT     mrc = 0;
    XCenterData *somThis = XCenterGetData(pnbp->inbp.somSelf);
    BOOL        fSave = TRUE;
    ULONG       ulCallInitCallback = 0;

    ULONG       ulUpdateFlags = XFMF_DISPLAYSTYLECHANGED;

    switch (ulItemID)
    {
        case ID_CRDI_VIEW_REDUCEWORKAREA:
            _fReduceDesktopWorkarea = ulExtra;
            if (_ulAutoHide)
                // this conflicts with auto-hide...
                // disable that.
                _ulAutoHide = 0;
            // renable items
            ulCallInitCallback = CBI_SET | CBI_ENABLE;
        break;

        case ID_CRDI_VIEW_TOPOFSCREEN:
            _ulPosition = XCENTER_TOP;

            // make sure the XCenter is the current screen
            // border object for the matching border
            // V0.9.19 (2002-04-17) [umoeller]
            // but only if _fAutoScreenBorder is enabled
            // V1.0.1 (2002-11-30) [umoeller]
            if (_fAutoScreenBorder)
                hifSetScreenBorderObjectUnique(SCREENCORNER_TOP,
                                               _wpQueryHandle(pnbp->inbp.somSelf));
        break;

        case ID_CRDI_VIEW_BOTTOMOFSCREEN:
            _ulPosition = XCENTER_BOTTOM;

            // make sure the XCenter is the current screen
            // border object for the matching border
            // V0.9.19 (2002-04-17) [umoeller]
            // but only if _fAutoScreenBorder is enabled
            // V1.0.1 (2002-11-30) [umoeller]
            if (_fAutoScreenBorder)
                hifSetScreenBorderObjectUnique(SCREENCORNER_BOTTOM,
                                               _wpQueryHandle(pnbp->inbp.somSelf));
        break;

        case ID_CRDI_VIEW_ALWAYSONTOP:
            if (ulExtra)
                _ulWindowStyle |= WS_TOPMOST;
            else
                _ulWindowStyle &= ~WS_TOPMOST;
        break;

        case ID_CRDI_VIEW_ANIMATE:
            if (ulExtra)
                _ulWindowStyle |= WS_ANIMATE;
            else
                _ulWindowStyle &= ~WS_ANIMATE;
        break;

        case ID_CRDI_VIEW_AUTOHIDE:
            if (ulExtra)
                _ulAutoHide = 4000;
            else
                _ulAutoHide = 0;
            ulCallInitCallback = CBI_SET | CBI_ENABLE;
            ulUpdateFlags = 0;      // no complete reformat
        break;

        case ID_CRDI_VIEW_AUTOHIDE_SLIDER:
            // we get this message even if the init callback is
            // setting this... stupid, stupid slider control
            if (!G_fSetting)
            {
                LONG lSliderIndex = winhQuerySliderArmPosition(pnbp->hwndControl,
                                                               SMA_INCREMENTVALUE);
                WinSetDlgItemShort(pnbp->hwndDlgPage,
                                   ID_CRDI_VIEW_AUTOHIDE_TXT2,
                                   lSliderIndex + 1,
                                   FALSE);      // unsigned
                _ulAutoHide = (lSliderIndex + 1) * 1000;
                            // range is 0 thru 59
                ulUpdateFlags = 0;      // no complete reformat
            }
        break;

        case ID_CRDI_VIEW_AUTOHIDE_CLICK:
            _fHideOnClick = ulExtra;
        break;

        // V0.9.19 (2002-05-07) [umoeller]
        case ID_CRDI_VIEW_AUTOSCREENBORDER:
            _fAutoScreenBorder = ulExtra;
            // V1.0.6 (2006-09-30) [pr]: adjust screen border setting @@fixes 224
            hifSetScreenBorderObjectUnique((_ulPosition == XCENTER_TOP)
                                             ? SCREENCORNER_TOP
                                             : SCREENCORNER_BOTTOM,
                                           _fAutoScreenBorder
                                             ? _wpQueryHandle(pnbp->inbp.somSelf)
                                             : NULLHANDLE);
        break;

        case ID_CRDI_VIEW_PRTY_SLIDER:
        {
            LONG lSliderIndex = winhQuerySliderArmPosition(pnbp->hwndControl,
                                                           SMA_INCREMENTVALUE);
            WinSetDlgItemShort(pnbp->hwndDlgPage,
                               ID_CRDI_VIEW_PRTY_TEXT,
                               lSliderIndex,
                               FALSE);      // unsigned
            // _lPriorityDelta = lSliderIndex;
            _xwpSetPriority(pnbp->inbp.somSelf,
                            lSliderIndex);
            ulUpdateFlags = 0;
        }
        break;

        case DID_DEFAULT:
            cmnSetupSetDefaults(G_XCenterSetupSet,
                                G_cXCenterSetupSetEntries, // ARRAYITEMCOUNT(G_XCenterSetupSet),
                                G_aulView1SetupOffsets,
                                ARRAYITEMCOUNT(G_aulView1SetupOffsets),
                                somThis);
            // V1.0.6 (2006-09-30) [pr]: adjust screen border setting @@fixes 224
            hifSetScreenBorderObjectUnique((_ulPosition == XCENTER_TOP)
                                             ? SCREENCORNER_TOP
                                             : SCREENCORNER_BOTTOM,
                                           _fAutoScreenBorder
                                             ? _wpQueryHandle(pnbp->inbp.somSelf)
                                             : NULLHANDLE);
            ulCallInitCallback = CBI_SET | CBI_ENABLE;
        break;

        case DID_UNDO:
        {
            XCenterData *pBackup = (XCenterData*)pnbp->pUser;
            cmnSetupRestoreBackup(G_aulView1SetupOffsets,
                                  ARRAYITEMCOUNT(G_aulView1SetupOffsets),
                                  somThis,
                                  pBackup);
            // V1.0.6 (2006-09-30) [pr]: adjust screen border setting @@fixes 224
            hifSetScreenBorderObjectUnique((_ulPosition == XCENTER_TOP)
                                             ? SCREENCORNER_TOP
                                             : SCREENCORNER_BOTTOM,
                                           _fAutoScreenBorder
                                             ? _wpQueryHandle(pnbp->inbp.somSelf)
                                             : NULLHANDLE);
            ulCallInitCallback = CBI_SET | CBI_ENABLE;
        }
        break;

        default:
            fSave = FALSE;
    }

    if (ulCallInitCallback)
        // call the init callback to refresh the page controls
        pnbp->inbp.pfncbInitPage(pnbp, ulCallInitCallback);

    if (fSave)
    {
        _wpSaveDeferred(pnbp->inbp.somSelf);

        if (_pvOpenView)
        {
            // view is currently open:
            PXCENTERWINDATA pXCenterData = (PXCENTERWINDATA)_pvOpenView;
            // this can be on a different thread, so post msg
            WinPostMsg(pXCenterData->Globals.hwndClient,
                       XCM_REFORMAT,
                       (MPARAM)ulUpdateFlags,
                       0);
        }
    }

    return mrc;
}

/* ******************************************************************
 *
 *   "View 2" ("Style") page notebook callbacks (notebook.c)
 *
 ********************************************************************/

static SLDCDATA
        BorderWidthSliderCData =
             {
                     sizeof(SLDCDATA),
            // usScale1Increments:
                     11,          // scale 1 increments
                     0,         // scale 1 spacing
                     1,          // scale 2 increments
                     0           // scale 2 spacing
             },
        BorderSpacingSliderCData =
             {
                     sizeof(SLDCDATA),
            // usScale1Increments:
                     11,          // scale 1 increments
                     0,         // scale 1 spacing
                     1,          // scale 2 increments
                     0           // scale 2 spacing
             },
        WidgetSpacingSliderCData =
             {
                     sizeof(SLDCDATA),
            // usScale1Increments:
                     10,          // scale 1 increments
                     0,         // scale 1 spacing
                     1,          // scale 2 increments
                     0           // scale 2 spacing
             };

#define STYLE_SLIDERS_WIDTH         100
#define STYLE_SLIDERS_HEIGHT        15
#define STYLE_SLIDERTEXT_WIDTH      DESCRTXT_WIDTH      // from above

static const CONTROLDEF
    BorderWidthGroup = LOADDEF_GROUP(ID_CRDI_VIEW2_3DBORDER_GROUP, HALF_TABLE_WIDTH),
    BorderWidthSlider = CONTROLDEF_SLIDER(
                            ID_CRDI_VIEW2_3DBORDER_SLIDER,
                            STYLE_SLIDERS_WIDTH,
                            STYLE_SLIDERS_HEIGHT,
                            &BorderWidthSliderCData),
    BorderWidthText = CONTROLDEF_TEXT_CENTER(
                            "M",           // to be replaced
                            ID_CRDI_VIEW2_3DBORDER_TEXT,
                            STYLE_SLIDERTEXT_WIDTH,
                            -1),
    DrawAll3DBordersCB = LOADDEF_AUTOCHECKBOX(ID_CRDI_VIEW2_ALL3DBORDERS),
    BorderSpacingGroup = LOADDEF_GROUP(ID_CRDI_VIEW2_BDRSPACE_GROUP, HALF_TABLE_WIDTH),
    BorderSpacingSlider = CONTROLDEF_SLIDER(
                            ID_CRDI_VIEW2_BDRSPACE_SLIDER,
                            STYLE_SLIDERS_WIDTH,
                            STYLE_SLIDERS_HEIGHT,
                            &BorderSpacingSliderCData),
    BorderSpacingText = CONTROLDEF_TEXT_CENTER(
                            "M",           // to be replaced
                            ID_CRDI_VIEW2_BDRSPACE_TEXT,
                            STYLE_SLIDERTEXT_WIDTH,
                            -1),
    WidgetSpacingGroup = LOADDEF_GROUP(ID_CRDI_VIEW2_WGTSPACE_GROUP, HALF_TABLE_WIDTH),
    WidgetSpacingSlider = CONTROLDEF_SLIDER(
                            ID_CRDI_VIEW2_WGTSPACE_SLIDER,
                            STYLE_SLIDERS_WIDTH,
                            STYLE_SLIDERS_HEIGHT,
                            &WidgetSpacingSliderCData),
    WidgetSpacingText = CONTROLDEF_TEXT_CENTER(
                            "M",           // to be replaced
                            ID_CRDI_VIEW2_WGTSPACE_TEXT,
                            STYLE_SLIDERTEXT_WIDTH,
                            -1),
    SizingBarsCB = LOADDEF_AUTOCHECKBOX(ID_CRDI_VIEW2_SIZINGBARS),
    SpacingLinesCB = LOADDEF_AUTOCHECKBOX(ID_CRDI_VIEW2_SPACINGLINES),
    DefWidgetStylesGroup = LOADDEF_GROUP(ID_CRDI_VIEW2_DEFSTYLES_GROUP, HALF_TABLE_WIDTH),

    FlatButtonsCB = LOADDEF_AUTOCHECKBOX(ID_CRDI_VIEW2_FLATBUTTONS),
    SunkBordersCB = LOADDEF_AUTOCHECKBOX(ID_CRDI_VIEW2_SUNKBORDERS),
    HatchInUseCB = LOADDEF_AUTOCHECKBOX(ID_CRDI_VIEW2_HATCHINUSE);

static const DLGHITEM G_dlgXCenterStyle[] =
    {
        START_TABLE,            // root table, required
            START_ROW(0),
                START_GROUP_TABLE(&BorderWidthGroup),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&BorderWidthSlider),
                        CONTROL_DEF(&BorderWidthText),
                    START_ROW(0),
                        CONTROL_DEF(&DrawAll3DBordersCB),
                END_TABLE,
            // START_ROW(0),
                START_GROUP_TABLE(&BorderSpacingGroup),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&BorderSpacingSlider),
                        CONTROL_DEF(&BorderSpacingText),
                END_TABLE,
            START_ROW(0),
                START_GROUP_TABLE(&WidgetSpacingGroup),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&WidgetSpacingSlider),
                        CONTROL_DEF(&WidgetSpacingText),
                    START_ROW(0),
                        CONTROL_DEF(&SizingBarsCB),
                    START_ROW(0),
                        CONTROL_DEF(&SpacingLinesCB),
                END_TABLE,
            // START_ROW(0),
                START_GROUP_TABLE(&DefWidgetStylesGroup),
                    START_ROW(0),
                        CONTROL_DEF(&FlatButtonsCB),
                    START_ROW(0),
                        CONTROL_DEF(&SunkBordersCB),
                    START_ROW(0),
                        CONTROL_DEF(&HatchInUseCB),
                END_TABLE,
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };


/*
 *@@ ctrpView2InitPage:
 *      notebook callback function (notebook.c) for the
 *      first XCenter "View" instance settings page.
 *      Sets the controls on the page according to the
 *      instance settings.
 *
 *@@added V0.9.7 (2000-12-05) [umoeller]
 *@@changed V0.9.9 (2001-01-29) [umoeller]: "Undo" data wasn't working
 *@@changed V0.9.13 (2001-06-19) [umoeller]: added spacing lines setting
 *@@changed V0.9.16 (2001-10-24) [umoeller]: now using dialog formatter
 *@@changed V0.9.16 (2001-10-24) [umoeller]: added hatch-in-use setting
 */

VOID ctrpView2InitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                       ULONG flFlags)        // CBI_* flags (notebook.h)
{
    XCenterData *somThis = XCenterGetData(pnbp->inbp.somSelf);

    if (flFlags & CBI_INIT)
    {
        // make backup of instance data for "Undo"
        XCenterData *pBackup;
        if (pBackup = (XCenterData*)malloc(sizeof(*somThis)))
            memcpy(pBackup, somThis, sizeof(*somThis));
            // be careful about using the copy... we have some pointers in there!
        // store in notebook struct
        pnbp->pUser = pBackup;

        // insert the controls using the dialog formatter
        // V0.9.16 (2001-10-24) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      G_dlgXCenterStyle,
                      ARRAYITEMCOUNT(G_dlgXCenterStyle));

        winhSetSliderTicks(WinWindowFromID(pnbp->hwndDlgPage,
                                           ID_CRDI_VIEW2_3DBORDER_SLIDER),
                           (MPARAM)0, 3,
                           MPFROM2SHORT(4, 5), 6);

        winhSetSliderTicks(WinWindowFromID(pnbp->hwndDlgPage,
                                           ID_CRDI_VIEW2_BDRSPACE_SLIDER),
                           (MPARAM)0, 3,
                           MPFROM2SHORT(4, 5), 6);

        winhSetSliderTicks(WinWindowFromID(pnbp->hwndDlgPage,
                                           ID_CRDI_VIEW2_WGTSPACE_SLIDER),
                           (MPARAM)0, 3,
                           MPFROM2SHORT(4, 5), 6);
    }

    if (flFlags & CBI_SET)
    {
        HWND hwndSlider;

        // 3D borders
        hwndSlider = WinWindowFromID(pnbp->hwndDlgPage, ID_CRDI_VIEW2_3DBORDER_SLIDER);
        winhSetSliderArmPosition(hwndSlider,
                                 SMA_INCREMENTVALUE,
                                 _ul3DBorderWidth);     // slider scale is from 0 to 10
        WinSetDlgItemShort(pnbp->hwndDlgPage,
                           ID_CRDI_VIEW_PRTY_TEXT,
                           _ul3DBorderWidth,
                           FALSE);      // unsigned
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_CRDI_VIEW2_ALL3DBORDERS,
                              ((_flDisplayStyle & XCS_ALL3DBORDERS) != 0));

        // border spacing
        hwndSlider = WinWindowFromID(pnbp->hwndDlgPage, ID_CRDI_VIEW2_BDRSPACE_SLIDER);
        winhSetSliderArmPosition(hwndSlider,
                                 SMA_INCREMENTVALUE,
                                 _ulBorderSpacing);     // slider scale is from 0 to 10
        WinSetDlgItemShort(pnbp->hwndDlgPage,
                           ID_CRDI_VIEW_PRTY_TEXT,
                           _ulBorderSpacing,
                           FALSE);      // unsigned

        // widget spacing
        hwndSlider = WinWindowFromID(pnbp->hwndDlgPage, ID_CRDI_VIEW2_WGTSPACE_SLIDER);
        winhSetSliderArmPosition(hwndSlider,
                                 SMA_INCREMENTVALUE,
                                 _ulWidgetSpacing - 1);     // slider scale is from 0 to 9
        WinSetDlgItemShort(pnbp->hwndDlgPage,
                           ID_CRDI_VIEW_PRTY_TEXT,
                           _ulWidgetSpacing,
                           FALSE);      // unsigned
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_CRDI_VIEW2_SIZINGBARS,
                              ((_flDisplayStyle & XCS_SIZINGBARS) != 0));

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_CRDI_VIEW2_SPACINGLINES,
                              ((_flDisplayStyle & XCS_SPACINGLINES) != 0));
                    // added V0.9.13 (2001-06-19) [umoeller]

        // default widget styles
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_CRDI_VIEW2_FLATBUTTONS,
                            ((_flDisplayStyle & XCS_FLATBUTTONS) != 0));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_CRDI_VIEW2_SUNKBORDERS,
                            ((_flDisplayStyle & XCS_SUNKBORDERS) != 0));
        // V0.9.16 (2001-10-24) [umoeller]
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_CRDI_VIEW2_HATCHINUSE,
                            ((_flDisplayStyle & XCS_NOHATCHINUSE) == 0));
    }
}

/*
 *@@ ctrpView2ItemChanged:
 *      notebook callback function (notebook.c) for the
 *      XCenter "View"  instance settings page.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@added V0.9.7 (2000-12-05) [umoeller]
 *@@changed V0.9.9 (2001-01-29) [umoeller]: now using cmnSetup* funcs
 *@@changed V0.9.13 (2001-06-19) [umoeller]: added spacing lines setting
 *@@changed V0.9.16 (2001-10-24) [umoeller]: added hatch-in-use setting
 */

MRESULT ctrpView2ItemChanged(PNOTEBOOKPAGE pnbp,
                             ULONG ulItemID,
                             USHORT usNotifyCode,
                             ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT     mrc = 0;
    XCenterData *somThis = XCenterGetData(pnbp->inbp.somSelf);
    BOOL        fSave = TRUE;
                // fDisplayStyleChanged = FALSE;
    LONG        lSliderIndex;
    ULONG       ulDisplayFlagChanged = 0;

    switch (ulItemID)
    {
        case ID_CRDI_VIEW2_3DBORDER_SLIDER:
            lSliderIndex = winhQuerySliderArmPosition(pnbp->hwndControl,
                                                      SMA_INCREMENTVALUE);
            WinSetDlgItemShort(pnbp->hwndDlgPage,
                               ID_CRDI_VIEW2_3DBORDER_TEXT,
                               lSliderIndex,
                               FALSE);      // unsigned
            _ul3DBorderWidth = lSliderIndex;
        break;

        case ID_CRDI_VIEW2_ALL3DBORDERS:
            ulDisplayFlagChanged = XCS_ALL3DBORDERS;
        break;

        case ID_CRDI_VIEW2_BDRSPACE_SLIDER:
            lSliderIndex = winhQuerySliderArmPosition(pnbp->hwndControl,
                                                      SMA_INCREMENTVALUE);
            WinSetDlgItemShort(pnbp->hwndDlgPage,
                               ID_CRDI_VIEW2_BDRSPACE_TEXT,
                               lSliderIndex,
                               FALSE);      // unsigned
            _ulBorderSpacing = lSliderIndex;
        break;

        case ID_CRDI_VIEW2_WGTSPACE_SLIDER:
            lSliderIndex = winhQuerySliderArmPosition(pnbp->hwndControl,
                                                      SMA_INCREMENTVALUE);
            WinSetDlgItemShort(pnbp->hwndDlgPage,
                               ID_CRDI_VIEW2_WGTSPACE_TEXT,
                               lSliderIndex + 1,
                               FALSE);      // unsigned
            _ulWidgetSpacing = lSliderIndex + 1;
        break;

        case ID_CRDI_VIEW2_SIZINGBARS:
            ulDisplayFlagChanged = XCS_SIZINGBARS;
        break;

        case ID_CRDI_VIEW2_SPACINGLINES:
            ulDisplayFlagChanged = XCS_SPACINGLINES;
        break;

        case ID_CRDI_VIEW2_FLATBUTTONS:
            ulDisplayFlagChanged = XCS_FLATBUTTONS;
        break;

        case ID_CRDI_VIEW2_SUNKBORDERS:
            ulDisplayFlagChanged = XCS_SUNKBORDERS;
        break;

        case ID_CRDI_VIEW2_HATCHINUSE:              // V0.9.16 (2001-10-24) [umoeller]
            // note, this one is reversed
            if (!ulExtra)
                _flDisplayStyle |= XCS_NOHATCHINUSE;
            else
                _flDisplayStyle &= ~XCS_NOHATCHINUSE;
        break;

        case DID_DEFAULT:
            cmnSetupSetDefaults(G_XCenterSetupSet,
                                G_cXCenterSetupSetEntries, // ARRAYITEMCOUNT(G_XCenterSetupSet),
                                G_aulView2SetupOffsets,
                                ARRAYITEMCOUNT(G_aulView2SetupOffsets),
                                somThis);
            // call the init callback to refresh the page controls
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;

        case DID_UNDO:
        {
            XCenterData *pBackup = (XCenterData*)pnbp->pUser;
            cmnSetupRestoreBackup(G_aulView2SetupOffsets,
                                  ARRAYITEMCOUNT(G_aulView2SetupOffsets),
                                  somThis,
                                  pBackup);
            // call the init callback to refresh the page controls
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        }
        break;

        default:
            fSave = FALSE;
    }

    if (ulDisplayFlagChanged)
    {
        if (ulExtra)
            _flDisplayStyle |= ulDisplayFlagChanged;
        else
            _flDisplayStyle &= ~ulDisplayFlagChanged;
    }

    if (fSave)
    {
        _wpSaveDeferred(pnbp->inbp.somSelf);

        if (_pvOpenView)
        {
            // view is currently open:
            PXCENTERWINDATA pXCenterData = (PXCENTERWINDATA)_pvOpenView;
            // this can be on a different thread, so post msg
            WinPostMsg(pXCenterData->Globals.hwndClient,
                       XCM_REFORMAT,
                       (MPARAM)XFMF_DISPLAYSTYLECHANGED,
                       0);
        }
    }

    return mrc;
}

/* ******************************************************************
 *
 *   "Widgets" page notebook callbacks (notebook.c)
 *
 ********************************************************************/

/*
 *@@ WIDGETRECORD:
 *      extended record core structure for "Widgets" container.
 *
 *@@added V0.9.7 (2000-12-09) [umoeller]
 */

typedef struct _WIDGETRECORD
{
    RECORDCORE      recc;
            // pszIcon contains the widget's class name

    WIDGETPOSITION  Position;           // V0.9.16 (2001-12-31) [umoeller]

    PSZ             pszIndex;           // points to szIndex
    CHAR            szIndex[40];

    const char      *pcszSetupString;
            // widget's setup string; points into instance data, do not free!

} WIDGETRECORD, *PWIDGETRECORD;

PFNWP G_pfnwpWidgetsCnr = NULL;

/*
 *@@ fnwpWidgetsCnr:
 *      subclassed window proc for the widgets container.  Handles
 *      DM_RENDER and DM_DISCARDOBJECT messages.
 *
 *@@added V0.9.14 (2001-07-29) [lafaix]
 *@@changed V1.0.6 (2006-08-22) [erdmann]: use correct function for DM_RENDERCOMPLETE @@fixes 829
 */

STATIC MRESULT EXPENTRY fnwpWidgetsCnr(HWND hwndCnr,
                                       ULONG msg,
                                       MPARAM mp1,
                                       MPARAM mp2)
{
    switch (msg)
    {
        case DM_RENDER:
        {
            PDRAGTRANSFER pdt = (PDRAGTRANSFER)mp1;
            MRESULT       mrc = (MRESULT)FALSE;

            if (DrgVerifyRMF(pdt->pditem, "DRM_OS2FILE", NULL))
            {
                CHAR          ach[CCHMAXPATH];
                PWIDGETRECORD pwr = (PWIDGETRECORD)pdt->pditem->ulItemID;
                BOOL          bSuccess;

                DrgQueryStrName(pdt->hstrRenderToName,
                                CCHMAXPATH,
                                ach);

                bSuccess = ctrpSaveToFile(ach,
                                          pwr->recc.pszIcon,
                                          pwr->pcszSetupString);

                // V1.0.6 (2006-08-22) [erdmann]: use correct function @@fixes 829
                DrgPostTransferMsg(pdt->hwndClient,
                                   DM_RENDERCOMPLETE,
                                   pdt,
                                   (bSuccess) ? DMFL_RENDEROK
                                              : DMFL_RENDERFAIL,
                                   0,
                                   TRUE);

                mrc = (MRESULT)TRUE;
            }

            DrgFreeDragtransfer(pdt);

            return mrc;
        }

    /* ??? This part does not work.  DM_DISCARDOBJECT is only received
       ??? ONCE per session.  I have absolutely no idea why.  Will check
       ??? another day. // @@todo
       ??? V0.9.14 (2001-07-30) [lafaix]
    */

        case DM_DISCARDOBJECT:
        {
            PDRAGINFO pdi;
_Pmpf(("DM_DISCARDOBJECT"));
DosBeep(100, 100);
/*            PDRAGINFO pdi;
            PWIDGETRECORD prec;
            PCREATENOTEBOOKPAGE pcnbp;

            if (    (pdi = (PDRAGINFO)mp1)
                 && (pditem = DrgQueryDragitemPtr(pdi, 0))
                 && (prec = (PWIDGETRECORD)pditem->ulItemID)
                 && (pcnbp = (PCREATENOTEBOOKPAGE)WinQueryWindowULong(hwndCnr, QWL_USER))
               )
            {
                if (prec->ulRootIndex != -1)        // @@todo
                    _xwpDeleteWidget(pnbp->inbp.somSelf,
                                     prec->ulRootIndex);
                          // this saves the instance data
                          // and updates the view
                          // and also calls the init callback
                          // to update the settings page!

                return (MRESULT)DRR_SOURCE;
            }
            else
                _Pmpf(("DM_DISCARDOBJECT NULL"));
*/        }
    }

    return G_pfnwpWidgetsCnr(hwndCnr, msg, mp1, mp2);
}

/*
 *@@ InsertWidgetSetting:
 *
 *@@added V0.9.13 (2001-06-23) [umoeller]
 */

STATIC PWIDGETRECORD InsertWidgetSetting(HWND hwndCnr,
                                         PPRIVATEWIDGETSETTING pSetting,
                                         const WIDGETPOSITION *pPosition)
                                  /*
                                  ULONG ulRootIndex,    // in: if -1, this is a subwidget;
                                                        // otherwise the widget index
                                  ULONG ulParentIndex,
                                  ULONG ulTrayIndex,
                                  ULONG ulSubwidgetIndex) */
{
    PWIDGETRECORD preccThis;

    if (preccThis = (PWIDGETRECORD)cnrhAllocRecords(hwndCnr,
                                                    sizeof(WIDGETRECORD),
                                                    1))
    {
        memcpy(&preccThis->Position, pPosition, sizeof(WIDGETPOSITION));

        if (pPosition->ulTrayWidgetIndex != -1)
            // subwidget:
            sprintf(preccThis->szIndex,
                    "  %d.%d.%d",
                    pPosition->ulTrayWidgetIndex,
                    pPosition->ulTrayIndex,
                    pPosition->ulWidgetIndex);
        else
            sprintf(preccThis->szIndex,
                    "%d",
                    pPosition->ulWidgetIndex);

        preccThis->pszIndex = preccThis->szIndex;
        preccThis->recc.pszIcon = pSetting->Public.pszWidgetClass;
        preccThis->pcszSetupString = pSetting->Public.pszSetupString;

        cnrhInsertRecords(hwndCnr,
                          NULL,         // parent
                          (PRECORDCORE)preccThis,
                          TRUE,         // invalidate
                          NULL,
                          CRA_RECORDREADONLY,
                          1);
    }

    return preccThis;
}

/*
 *@@ ctrpWidgetsInitPage:
 *      notebook callback function (notebook.c) for the
 *      XCenter "Widgets" instance settings page.
 *      Sets the controls on the page according to the
 *      instance settings.
 *
 *      CREATENOTEBOOKPAGE.pUser is used for the widget
 *      context menu HWND.
 *
 *@@added V0.9.9 (2001-03-09) [umoeller]
 *@@changed V0.9.12 (2001-05-08) [lafaix]: forced class loading/unloading
 */

VOID ctrpWidgetsInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                         ULONG flFlags)        // CBI_* flags (notebook.h)
{
    // XCenterData *somThis = XCenterGetData(pnbp->inbp.somSelf);

    if (flFlags & CBI_INIT)
    {
        // PNLSSTRINGS     pNLSStrings = cmnQueryNLSStrings();
        HWND hwndCnr = WinWindowFromID(pnbp->hwndDlgPage, ID_XFDI_CNR_CNR);
        XFIELDINFO      xfi[5];
        PFIELDINFO      pfi = NULL;
        int             i = 0;

        // set group cnr title
        WinSetDlgItemText(pnbp->hwndDlgPage, ID_XFDI_CNR_GROUPTITLE,
                          cmnGetString(ID_XSSI_WIDGETSPAGE)) ; // pszWidgetsPage

        // set up cnr details view
        xfi[i].ulFieldOffset = FIELDOFFSET(WIDGETRECORD, pszIndex);
        xfi[i].pszColumnTitle = "";
        xfi[i].ulDataType = CFA_STRING;
        xfi[i++].ulOrientation = CFA_LEFT;

        xfi[i].ulFieldOffset = FIELDOFFSET(RECORDCORE, pszIcon);
        xfi[i].pszColumnTitle = cmnGetString(ID_XSSI_WIDGETCLASS);  // "Class"; // pszWidgetClass
        xfi[i].ulDataType = CFA_STRING;
        xfi[i++].ulOrientation = CFA_LEFT;

        xfi[i].ulFieldOffset = FIELDOFFSET(WIDGETRECORD, pcszSetupString);
        xfi[i].pszColumnTitle = cmnGetString(ID_XSSI_WIDGETSETUP);  // "Setup"; // pszWidgetSetup
        xfi[i].ulDataType = CFA_STRING;
        xfi[i++].ulOrientation = CFA_LEFT;

        pfi = cnrhSetFieldInfos(hwndCnr,
                                xfi,
                                i,             // array item count
                                TRUE,          // draw lines
                                1);            // return second column

        BEGIN_CNRINFO()
        {
            cnrhSetView(CV_DETAIL | CA_DETAILSVIEWTITLES
                             | CA_ORDEREDTARGETEMPH); // target emphasis between records
            cnrhSetSplitBarAfter(pfi);
            cnrhSetSplitBarPos(100);
        } END_CNRINFO(hwndCnr);

        ctrpLoadClasses();

        // we must subclass the widgets container, so that we can handle
        // the DM_DISCARDOBJECT and DM_RENDER msgs
        // V0.9.14 (2001-07-29) [lafaix]
        G_pfnwpWidgetsCnr = WinSubclassWindow(hwndCnr, fnwpWidgetsCnr);

        // we also need a copy of pcnbp, so that we can easily handle
        // DM_DISCARDOBJECT without having to climb up the window
        // hierarchy
        // V0.9.14 (2001-07-30) [lafaix]
        WinSetWindowULong(hwndCnr, QWL_USER, (ULONG)pnbp);
    }

    if (flFlags & CBI_SET)
    {
        HWND        hwndCnr = WinWindowFromID(pnbp->hwndDlgPage, ID_XFDI_CNR_CNR);
        PLINKLIST   pllWidgets = ctrpQuerySettingsList(pnbp->inbp.somSelf);
        PLISTNODE   pNode = lstQueryFirstNode(pllWidgets);
        ULONG       ulIndex = 0;

        cnrhRemoveAll(hwndCnr);

        while (pNode)
        {
            PPRIVATEWIDGETSETTING pSetting = (PPRIVATEWIDGETSETTING)pNode->pItemData;
            WIDGETPOSITION Pos;
            Pos.ulTrayWidgetIndex = -1;
            Pos.ulTrayIndex = -1;
            Pos.ulWidgetIndex = ulIndex;
            InsertWidgetSetting(hwndCnr,
                                pSetting,
                                &Pos);

            if (pSetting->pllTraySettings)
            {
                PLISTNODE pTrayNode = lstQueryFirstNode(pSetting->pllTraySettings);
                // ULONG ulTray = 0;

                Pos.ulTrayWidgetIndex = ulIndex;
                Pos.ulTrayIndex = 0;

                while (pTrayNode)
                {
                    PTRAYSETTING pTray = (PTRAYSETTING)pTrayNode->pItemData;
                    // ULONG ulSubwidget = 0;
                    PLISTNODE pSubwidgetNode = lstQueryFirstNode(&pTray->llSubwidgetSettings);

                    Pos.ulWidgetIndex = 0;

                    while (pSubwidgetNode)
                    {
                        PPRIVATEWIDGETSETTING pSubwidget = (PPRIVATEWIDGETSETTING)pSubwidgetNode->pItemData;

                        InsertWidgetSetting(hwndCnr,
                                            pSubwidget,
                                            &Pos);
                                            /* -1,     // non-root
                                            ulIndex,
                                            ulTray,
                                            ulSubwidget); */

                        // ulSubwidget++;
                        (Pos.ulWidgetIndex)++;
                        pSubwidgetNode = pSubwidgetNode->pNext;
                    }

                    pTrayNode = pTrayNode->pNext;
                    // ulTray++;
                    (Pos.ulTrayIndex)++;
                }
            }

            pNode = pNode->pNext;
            ulIndex++;
        }
    }

    if (flFlags & CBI_DESTROY)
    {
        if (pnbp->pUser)
            // unload menu
            WinDestroyWindow((HWND)pnbp->pUser);

        pnbp->pUser = NULL;

        ctrpFreeClasses();
    }
}

// define a new rendering mechanism, which only
// our own container supports (this will make
// sure that we can only do d'n'd within this
// one container)
#define WIDGET_RMF_MECH     "DRM_XWPXCENTERWIDGET"
#define WIDGET_RMF_FORMAT   "DRF_XWPWIDGETRECORD"
// #define WIDGET_PRIVATE_RMF  "(" WIDGET_RMF_MECH ")x(" WIDGET_RMF_FORMAT ")"
#define WIDGET_PRIVATE_RMF  "(" WIDGET_RMF_MECH ",DRM_OS2FILE,DRM_DISCARD)x(" WIDGET_RMF_FORMAT ")"

static PWIDGETRECORD G_precDragged = NULL,
                     G_precAfter = NULL;

/*
 *@@ ctrpWidgetsItemChanged:
 *      notebook callback function (notebook.c) for the
 *      XCenter "Widgets" instance settings page.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@added V0.9.9 (2001-03-09) [umoeller]
 *@@changed V0.9.12 (2001-05-08) [lafaix]: fixed problems if widget class not found
 *@@changed V0.9.14 (2001-07-29) [lafaix]: now handles widget settings files dnd
 *@@changed V1.0.0 (2002-08-12) [umoeller]: fixed highly broken popup menu
 */

MRESULT ctrpWidgetsItemChanged(PNOTEBOOKPAGE pnbp,
                               ULONG ulItemID, USHORT usNotifyCode,
                               ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT     mrc = 0;
    // XCenterData *somThis = XCenterGetData(pnbp->inbp.somSelf);

    switch (ulItemID)
    {
        case ID_XFDI_CNR_CNR:
            switch (usNotifyCode)
            {
                /*
                 * CN_INITDRAG:
                 *      user begins dragging a widget
                 */

                case CN_INITDRAG:
                {
                    PCNRDRAGINIT pcdi = (PCNRDRAGINIT)ulExtra;
                    if (DrgQueryDragStatus())
                        // (lazy) drag currently in progress: stop
                        break;

                    if (pcdi)
                        // filter out whitespace
                        if (pcdi->pRecord)
                        {
                            // for now, allow only dragging of root widgets
                            // @@todo
                            if (((PWIDGETRECORD)pcdi->pRecord)->Position.ulTrayWidgetIndex == -1)
                                cnrhInitDrag(pcdi->hwndCnr,
                                             pcdi->pRecord,
                                             usNotifyCode,
                                             WIDGET_PRIVATE_RMF,
                                             DO_COPYABLE | DO_MOVEABLE); // DO_MOVEABLE);
                        }
                }
                break;

                /*
                 * CN_DRAGAFTER:
                 *      something's being dragged over the widgets cnr;
                 *      we allow dropping only for widget records being
                 *      dragged _within_ the cnr.
                 *
                 *      Note that since we have set CA_ORDEREDTARGETEMPH
                 *      for the "Assocs" cnr, we do not get CN_DRAGOVER,
                 *      but CN_DRAGAFTER only.
                 *
                 *      PMREF doesn't say which record is really in
                 *      in the CNRDRAGINFO. From my testing, that field
                 *      is set to CMA_FIRST if the source record is
                 *      dragged before the first record; it is set to
                 *      the record pointer _before_ the record being
                 *      dragged for any other record.
                 *
                 *      In other words, if the draggee is after the
                 *      first record, we get the first record in
                 *      CNRDRAGINFO; if it's after the second, we
                 *      get the second, and so on.
                 */

                case CN_DRAGAFTER:
                {
                    PCNRDRAGINFO pcdi = (PCNRDRAGINFO)ulExtra;
                    USHORT      usIndicator = DOR_NODROP,
                                    // cannot be dropped, but send
                                    // DM_DRAGOVER again
                                usOp = DO_UNKNOWN;
                                    // target-defined drop operation:
                                    // user operation (we don't want
                                    // the WPS to copy anything)

                    // reset global variable
                    G_precDragged = NULL;

                    // get access to the drag'n'drop structures
                    if (DrgAccessDraginfo(pcdi->pDragInfo))
                    {
                        if (pcdi->pDragInfo->cditem != 1)
                            usIndicator = DOR_NEVERDROP;
                        else
                        {
                            // we must accept the drop if (1) this is a file
                            // whose type is DRT_WIDGET, or (2) this a widget
                            // from ourselves
                            PDRAGITEM pdrgItem = NULL;

                            if (    // do not allow drag upon whitespace,
                                    // but only between records
                                    (pcdi->pRecord)
                                 && (pdrgItem = DrgQueryDragitemPtr(pcdi->pDragInfo, 0))
                               )
                            {
                                if (    (DrgVerifyRMF(pdrgItem, "DRM_OS2FILE", NULL))
                                     && (ctrpVerifyType(pdrgItem, DRT_WIDGET))
                                   )
                                {
                                    // this is case (1)
                                    G_precAfter = NULL;
                                    // check target record...
                                    if (    (pcdi->pRecord == (PRECORDCORE)CMA_FIRST)
                                         || (pcdi->pRecord == (PRECORDCORE)CMA_LAST)
                                       )
                                    {
                                        // store record after which to insert
                                        G_precAfter = (PWIDGETRECORD)pcdi->pRecord;
                                    }
                                    // do not allow dropping after
                                    // disabled records
                                    else if (!(pcdi->pRecord->flRecordAttr & CRA_DISABLED))
                                        // store record after which to insert
                                        G_precAfter = (PWIDGETRECORD)pcdi->pRecord;

                                    if (G_precAfter)
                                    {
                                        usIndicator = DOR_DROP;
                                        usOp = DO_COPY;
                                    }
                                }
                                else
                                if (pcdi->pDragInfo->hwndSource != pnbp->hwndControl)
                                    // neither case (1) nor (2)
                                    usIndicator = DOR_NEVERDROP;
                                else
                                {
                                    // this is case (2)
                                    if (    (pcdi->pDragInfo->usOperation == DO_DEFAULT)
                                         || (pcdi->pDragInfo->usOperation == DO_MOVE)
                                       )
                                    {
                                        PWIDGETRECORD precDragged = (PWIDGETRECORD)pdrgItem->ulItemID;

                                        G_precAfter = NULL;
                                        // check target record...
                                        if (   (pcdi->pRecord == (PRECORDCORE)CMA_FIRST)
                                            || (pcdi->pRecord == (PRECORDCORE)CMA_LAST)
                                           )
                                            // store record after which to insert
                                            G_precAfter = (PWIDGETRECORD)pcdi->pRecord;
                                        // do not allow dropping after
                                        // disabled records
                                        else if ((pcdi->pRecord->flRecordAttr & CRA_DISABLED) == 0)
                                            // store record after which to insert
                                            G_precAfter = (PWIDGETRECORD)pcdi->pRecord;

                                        if (G_precAfter)
                                        {
                                            if (    ((PWIDGETRECORD)pcdi->pRecord  // target recc
                                                      != precDragged) // source recc
                                                 && (DrgVerifyRMF(pdrgItem,
                                                                  WIDGET_RMF_MECH,
                                                                  WIDGET_RMF_FORMAT))
                                               )
                                            {
                                                // allow drop:
                                                // store record being dragged
                                                G_precDragged = precDragged;
                                                // G_precAfter already set
                                                usIndicator = DOR_DROP;
                                                usOp = DO_MOVE;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        DrgFreeDraginfo(pcdi->pDragInfo);
                    }

                    // and return the drop flags
                    mrc = (MRFROM2SHORT(usIndicator, usOp));
                }
                break;

                /*
                 * CN_DROP:
                 *      something _has_ now been dropped on the cnr.
                 */

                case CN_DROP:
                {
                    // check the global variable which has been set
                    // by CN_DRAGAFTER above:
                    if (G_precDragged)
                    {
                        // CN_DRAGOVER above has considered this valid:
                        if (G_precAfter)
                        {
                            ULONG ulIndex = -2;

                            if (G_precAfter == (PWIDGETRECORD)CMA_FIRST)
                            {
                                // move before index 0
                                ulIndex = 0;
                            }
                            else if (G_precAfter == (PWIDGETRECORD)CMA_LAST)
                            {
                                // shouldn't happen
                                DosBeep(100, 100);
                            }
                            else
                            {
                                // we get the record _before_ the draggee
                                // (CN_DRAGAFTER), but xwpMoveWidget wants
                                // the index of the widget _before_ which the
                                // widget should be inserted:
                                ulIndex = G_precAfter->Position.ulWidgetIndex + 1; // ulRootIndex + 1;
                            }
                            if (ulIndex != -2)
                                // OK... move the widgets around:
                                _xwpMoveWidget(pnbp->inbp.somSelf,
                                               G_precDragged->Position.ulWidgetIndex, // ulRootIndex,  // from
                                               ulIndex);                // to
                                    // this saves the instance data
                                    // and updates the view
                                    // and also calls the init callback
                                    // to update the settings page!
                        }
                        G_precDragged = NULL;
                    }
                    else
                    // it was not an internal widget drag; it could be a
                    // widget settings file drop, though.  Lets check.
                    {
                        PCNRDRAGINFO pcdi = (PCNRDRAGINFO)ulExtra;

                        if (DrgAccessDraginfo(pcdi->pDragInfo))
                        {
                            PDRAGITEM pdrgItem = DrgQueryDragitemPtr(pcdi->pDragInfo, 0);

                            if (    (pdrgItem)
                                 && (G_precAfter)
                                 && (DrgVerifyRMF(pdrgItem, "DRM_OS2FILE", NULL))
                                 && (ctrpVerifyType(pdrgItem, DRT_WIDGET))
                               )
                            {
                                // that was it.  We must create a new widget
                                // here.
                                CHAR        achCnr[CCHMAXPATH],
                                            achSrc[CCHMAXPATH];

                                // the dnd part guaranties the following
                                // will not overflow
                                DrgQueryStrName(pdrgItem->hstrContainerName,
                                                CCHMAXPATH,
                                                achCnr);
                                DrgQueryStrName(pdrgItem->hstrSourceName,
                                                CCHMAXPATH,
                                                achSrc);

                                if ((strlen(achCnr)+strlen(achSrc)) < (CCHMAXPATH-1))
                                {
                                    CHAR achAll[CCHMAXPATH];
                                    PSZ pszClass, pszSetup;
                                    APIRET arc;

                                    strcpy(achAll, achCnr);
                                    strcat(achAll, achSrc);

                                    if (!(arc = ctrpReadFromFile(achAll,
                                                                 &pszClass,
                                                                 &pszSetup)))
                                    {
                                        // it looks like it is a valid
                                        // widget settings data
                                        ULONG ulIndex = -2;

                                        if (G_precAfter == (PWIDGETRECORD)CMA_FIRST)
                                        {
                                            // copy before index 0
                                            ulIndex = 0;
                                        }
                                        else if (G_precAfter == (PWIDGETRECORD)CMA_LAST)
                                        {
                                            // shouldn't happen
                                            DosBeep(100, 100);
                                        }
                                        else
                                        {
                                            // we get the record _before_ the draggee
                                            // (CN_DRAGAFTER), but xwpMoveWidget wants
                                            // the index of the widget _before_ which the
                                            // widget should be inserted:
                                            ulIndex = G_precAfter->Position.ulWidgetIndex + 1; // ulRootIndex + 1;
                                        }
                                        if (ulIndex != -2)
                                        {
                                            WIDGETPOSITION pos2;
                                            pos2.ulTrayWidgetIndex = -1;
                                            pos2.ulTrayIndex = -1;
                                            pos2.ulWidgetIndex = ulIndex;

                                            _xwpCreateWidget(pnbp->inbp.somSelf,
                                                             pszClass,
                                                             pszSetup,
                                                             &pos2);
                                        }

                                        free(pszClass);
                                        free(pszSetup);
                                    }
                                }
                            }

                            DrgFreeDraginfo(pcdi->pDragInfo);
                        }
                    }
                }
                break;

                /*
                 * CN_CONTEXTMENU:
                 *      cnr context menu requested
                 *      for widget
                 */

                case CN_CONTEXTMENU:
                {
                    // we store the container and recc.
                    // in the CREATENOTEBOOKPAGE structure
                    // so that the notebook.c function can
                    // remove source emphasis later automatically
                    pnbp->hwndSourceCnr = pnbp->hwndControl;

                    if (pnbp->preccSource = (PRECORDCORE)ulExtra)
                    {
                        PCXCENTERWIDGETCLASS pClass;

                        // popup menu on container recc (not whitespace):

                        if (pnbp->pUser)
                        {
                            // reload every time now
                            // V1.0.0 (2002-08-12) [umoeller]
                            WinDestroyWindow((HWND)pnbp->pUser);
                            pnbp->pUser = NULL;
                        }

                        if (    (!ctrpFindClass(pnbp->preccSource->pszIcon,  // class name
                                                FALSE,       // fMustBeTrayable
                                                &pClass))
                                // using new func now V1.0.0 (2002-08-12) [umoeller]
                             && (pnbp->pUser = (PVOID)ctrpLoadWidgetPopupMenu(pnbp->hwndControl,
                                                                              pClass,
                                                                              WPOPFL_NOTEBOOKMENU))
                           )
                        {
                            cnrhShowContextMenu(pnbp->hwndControl,  // cnr
                                                (PRECORDCORE)pnbp->preccSource,
                                                (HWND)pnbp->pUser,
                                                pnbp->hwndDlgPage);    // owner
                        }
                    }
                }
                break;  // CN_CONTEXTMENU

            } // end switch (usNotifyCode)
        break;

        /*
         * ID_CRMI_PROPERTIES:
         *      command from widget context menu.
         */

        case ID_CRMI_PROPERTIES:
        {
            PWIDGETRECORD           prec;
            PCXCENTERWIDGETCLASS    pClass;

            if (    (prec = (PWIDGETRECORD)pnbp->preccSource)
                 && (!ctrpFindClass(pnbp->preccSource->pszIcon,  // class name
                                    FALSE,
                                    &pClass))
                 && (pClass->pShowSettingsDlg != 0)
               )
            {
                ctrpShowSettingsDlg(pnbp->inbp.somSelf,
                                    pnbp->hwndDlgPage, // owner
                                    &prec->Position);
            }
        }
        break;

        /*
         * ID_CRMI_REMOVEWGT:
         *      command from widget context menu.
         */

        case ID_CRMI_REMOVEWGT:
        {
            PWIDGETRECORD prec;
            if (    (prec = (PWIDGETRECORD)pnbp->preccSource)
                 // && (prec->Position.ulTrayWidgetIndex == -1)    // @@todo
               )
                _xwpDeleteWidget(pnbp->inbp.somSelf,
                                 &prec->Position);
                      // this saves the instance data
                      // and updates the view
                      // and also calls the init callback
                      // to update the settings page!
        }
        break;
    }

    return mrc;
}

/* ******************************************************************
 *
 *   "Classes" page notebook callbacks (notebook.c)
 *
 ********************************************************************/

/*
 *@@ XCLASSRECORD:
 *      extended record core structure for "Classes" container.
 *
 *@@added V0.9.9 (2001-03-09) [umoeller]
 */

typedef struct _XCLASSRECORD
{
    RECORDCORE      recc;

    PSZ             pszDLL;         // points to szDLL
    CHAR            szDLL[CCHMAXPATH];
    PSZ             pszClass;
    PSZ             pszClassTitle;
    PSZ             pszVersion;     // points to szVersion
    CHAR            szVersion[40];
} XCLASSRECORD, *PXCLASSRECORD;

/*
 *@@ ctrpClassesInitPage:
 *      notebook callback function (notebook.c) for the
 *      XCenter "Widgets" instance settings page.
 *      Sets the controls on the page according to the
 *      instance settings.
 *
 *@@added V0.9.7 (2000-12-05) [umoeller]
 *@@changed V0.9.20 (2002-07-23) [lafaix]: uses generic plugin support now
 */

VOID ctrpClassesInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                         ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        // PNLSSTRINGS     pNLSStrings = cmnQueryNLSStrings();
        HWND hwndCnr = WinWindowFromID(pnbp->hwndDlgPage, ID_XFDI_CNR_CNR);
        XFIELDINFO      xfi[5];
        PFIELDINFO      pfi = NULL;
        int             i = 0;

        // set group cnr title
        WinSetDlgItemText(pnbp->hwndDlgPage, ID_XFDI_CNR_GROUPTITLE,
                          cmnGetString(ID_XSSI_CLASSESPAGE)) ; // pszClassesPage

        // set up cnr details view
        xfi[i].ulFieldOffset = FIELDOFFSET(XCLASSRECORD, pszDLL);
        xfi[i].pszColumnTitle = "DLL";
        xfi[i].ulDataType = CFA_STRING;
        xfi[i++].ulOrientation = CFA_LEFT;

        xfi[i].ulFieldOffset = FIELDOFFSET(XCLASSRECORD, pszClass);
        xfi[i].pszColumnTitle = cmnGetString(ID_XSSI_WIDGETCLASS);  // "Class"; // pszWidgetClass
        xfi[i].ulDataType = CFA_STRING;
        xfi[i++].ulOrientation = CFA_LEFT;

        xfi[i].ulFieldOffset = FIELDOFFSET(XCLASSRECORD, pszClassTitle);
        xfi[i].pszColumnTitle = cmnGetString(ID_XSSI_XC_CLASSTITLE);
        xfi[i].ulDataType = CFA_STRING;
        xfi[i++].ulOrientation = CFA_LEFT;

        xfi[i].ulFieldOffset = FIELDOFFSET(XCLASSRECORD, pszVersion);
        xfi[i].pszColumnTitle = cmnGetString(ID_XSSI_XC_VERSION);
        xfi[i].ulDataType = CFA_STRING;
        xfi[i++].ulOrientation = CFA_LEFT;

        pfi = cnrhSetFieldInfos(hwndCnr,
                                xfi,
                                i,             // array item count
                                TRUE,          // draw lines
                                1);            // return second column

        BEGIN_CNRINFO()
        {
            cnrhSetView(CV_DETAIL | CA_DETAILSVIEWTITLES);
            cnrhSetSplitBarAfter(pfi);
            cnrhSetSplitBarPos(200);
        } END_CNRINFO(hwndCnr);

        ctrpLoadClasses();
    }

    if (flFlags & CBI_SET)
    {
        PLINKLIST   pllClasses;
        ULONG       cClasses;
        if (    (pllClasses = ctrpQueryClasses())
             && (cClasses = lstCountItems(pllClasses))
           )
        {
            HWND hwndCnr = WinWindowFromID(pnbp->hwndDlgPage, ID_XFDI_CNR_CNR);
            PXCLASSRECORD precFirst;

            if (precFirst = (PXCLASSRECORD)cnrhAllocRecords(hwndCnr,
                                                            sizeof(XCLASSRECORD),
                                                            cClasses))
            {
                PXCLASSRECORD precThis = precFirst;
                PLISTNODE pNode = lstQueryFirstNode(pllClasses);
                while (pNode)
                {
                    PPLUGINCLASS pClass = (PPLUGINCLASS)pNode->pItemData;

                    if (!plgIsClassBuiltIn(pClass))
                    {
                        PSZ p = NULL;
                        CHAR sz[CCHMAXPATH];
                        ULONG ulMajor,
                              ulMinor,
                              ulRevision;

                        if (!plgQueryClassModuleName(pClass,
                                                     sizeof(sz),
                                                     sz))
                        {
                            if (p = strrchr(sz, '\\'))
                            {
                                strcpy(precThis->szDLL, p + 1);
                                precThis->pszDLL = precThis->szDLL;
                            }
                        }

                        if (!p)
                            precThis->pszDLL = "Error";

                        if (!plgQueryClassVersion(pClass,
                                                  &ulMajor,
                                                  &ulMinor,
                                                  &ulRevision))
                        {
                            sprintf(precThis->szVersion,
                                    "%d.%d.%d",
                                    ulMajor,
                                    ulMinor,
                                    ulRevision);
                        }

                        precThis->pszVersion = precThis->szVersion;
                    }
                    else
                        precThis->pszDLL = (PSZ)cmnGetString(ID_CRSI_BUILTINCLASS);

                    precThis->pszClass = (PSZ)pClass->pcszClass;

                    precThis->pszClassTitle = (PSZ)pClass->pcszClassTitle;

                    precThis = (PXCLASSRECORD)precThis->recc.preccNextRecord;
                    pNode = pNode->pNext;
                }

                // kah-wump
                cnrhInsertRecords(hwndCnr,
                                  NULL,         // parent
                                  (PRECORDCORE)precFirst,
                                  TRUE,         // invalidate
                                  NULL,
                                  CRA_RECORDREADONLY,
                                  cClasses);
            }
        }
    }

    if (flFlags & CBI_ENABLE)
    {
    }

    if (flFlags & CBI_DESTROY)
    {
        ctrpFreeClasses();
    }
}

/*
 *@@ ctrpClassesItemChanged:
 *      notebook callback function (notebook.c) for the
 *      XCenter "Widgets" instance settings page.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@added V0.9.7 (2000-12-05) [umoeller]
 */

MRESULT ctrpClassesItemChanged(PNOTEBOOKPAGE pnbp,
                               ULONG ulItemID,
                               USHORT usNotifyCode,
                               ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT     mrc = 0;

    return mrc;
}

