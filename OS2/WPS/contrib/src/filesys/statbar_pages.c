
/*
 *@@sourcefile statbars.c:
 *      this file contains the notebook pages for controlling
 *      status and tool bars.
 *
 *      This file is new with V1.0.1 and contains code formerly
 *      in statbars.c, plus the new settings pages for tool bars.
 *
 *@@header "filesys\statbars.h"
 *@@added V1.0.1 (2002-12-08) [umoeller]
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
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

#define INCL_WINMENUS
#define INCL_WINPOINTERS
#define INCL_WINBUTTONS
#define INCL_WINSTATICS
#define INCL_WINENTRYFIELDS
#define INCL_WINSHELLDATA       // Prf* functions
#define INCL_WINPROGRAMLIST     // needed for PROGDETAILS, wppgm.h
#include <os2.h>

// C library headers
#include <stdio.h>
#include <string.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
#include "xfobj.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\classes.h"             // WPS class list helper functions
#include "shared\classtest.h"           // some cheap funcs for WPS class checks
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\notebook.h"            // generic XWorkplace notebook handling

#include "filesys\statbars.h"           // status bar translation logic
#include "filesys\xthreads.h"           // extra XWorkplace threads

// other SOM headers
#pragma hdrstop                         // VAC++ keeps crashing otherwise
#include <wpfsys.h>                     // WPFileSystem
#include <wpdisk.h>                     // WPDisk
#include <wppgm.h>                      // WPProgram

/* ******************************************************************
 *
 *   Notebook callbacks (notebook.c) for "Status bars" pages
 *
 ********************************************************************/

#ifndef __NOCFGSTATUSBARS__

/*
 *@@ STATUSBARPAGEDATA:
 *      data for status bar pages while they're open.
 *
 *      Finally turned this into a heap structure
 *      with V0.9.14 to get rid of the global variables.
 *
 *@@added V0.9.14 (2001-07-31) [umoeller]
 */

typedef struct _STATUSBARPAGEDATA
{
    CHAR                    szSBTextNoneSelBackup[CCHMAXMNEMONICS],
                            szSBText1SelBackup[CCHMAXMNEMONICS],
                            szSBTextMultiSelBackup[CCHMAXMNEMONICS],
                            szSBClassSelected[256];
    SOMClass                *pSBClassObjectSelected;
    // somId                   somidClassSelected;

    HWND                    hwndKeysMenu;
                // if != NULLHANDLE, the menu for the last
                // "Keys" button pressed
    ULONG                   ulLastKeysID;

} STATUSBARPAGEDATA, *PSTATUSBARPAGEDATA;

// struct passed to callbacks
typedef struct _STATUSBARSELECTCLASS
{
    HWND            hwndOKButton;
} STATUSBARSELECTCLASS, *PSTATUSBARSELECTCLASS;

/*
 *@@ fncbWPSStatusBarReturnClassAttr:
 *      this callback function is called for every single
 *      record core which represents a WPS class; we need
 *      to return the record core attributes.
 *
 *      This gets called from the class list functions in
 *      classlst.c.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfwps.c
 */

STATIC MRESULT EXPENTRY fncbWPSStatusBarReturnClassAttr(HWND hwndCnr,
                                                        ULONG ulscd,   // SELECTCLASSDATA struct
                                                        MPARAM mpwps,  // current WPSLISTITEM struct
                                                        MPARAM mpreccParent) // parent record core
{
    USHORT              usAttr = CRA_RECORDREADONLY | CRA_COLLAPSED | CRA_DISABLED;
    PWPSLISTITEM        pwps = (PWPSLISTITEM)mpwps;
    PSELECTCLASSDATA    pscd = (PSELECTCLASSDATA)ulscd;
    PRECORDCORE         preccParent = NULL;

    if (pwps)
    {

        if (pwps->pClassObject)
        {
            // now check if the class supports new sort mnemonics
            if (stbClassAddsNewMnemonics(pwps->pClassObject))
            {
                // class _does_ support mnemonics: give the
                // new recc attr the "in use" flag
                usAttr = CRA_RECORDREADONLY | CRA_COLLAPSED | CRA_INUSE;

                // and select it if the settings notebook wants it
                if (!strcmp(pwps->pszClassName, pscd->szClassSelected))
                    usAttr |= CRA_SELECTED;

                // expand all the parent records of the new record
                // so that classes with status bar mnemonics are
                // all initially visible in the container
                preccParent = (PRECORDCORE)mpreccParent;
                while (preccParent)
                {
                    WinSendMsg(hwndCnr,
                               CM_EXPANDTREE,
                               (MPARAM)preccParent,
                               0);

                    // get next higher parent
                    preccParent = WinSendMsg(hwndCnr,
                                             CM_QUERYRECORD,
                                             preccParent,
                                             MPFROM2SHORT(CMA_PARENT,
                                                          CMA_ITEMORDER));
                    if (preccParent == (PRECORDCORE)-1)
                        // none: stop
                        preccParent = NULL;
                } // end while (preccParent)
            }
        } // end if if (pwps->pClassObject)
        else
            // invalid class: hide in cnr
            usAttr = CRA_FILTERED;
    }
    return (MPARAM)usAttr;
}

/*
 *@@ fncbWPSStatusBarClassSelected:
 *      callback func for class selected;
 *      mphwndInfo has been set to the static control hwnd.
 *      Returns TRUE if the selection is valid; the dlg func
 *      will then enable the OK button.
 *
 *      This gets called from the class list functions in
 *      classlst.c.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfwps.c
 */

STATIC MRESULT EXPENTRY fncbWPSStatusBarClassSelected(HWND hwndCnr,
                                                      ULONG ulpsbsc,
                                                      MPARAM mpwps,
                                                      MPARAM mphwndInfo)
{
    PWPSLISTITEM pwps = (PWPSLISTITEM)mpwps;
    CHAR szInfo[2000];
    MRESULT mrc = (MPARAM)FALSE;
    PSZ pszClassTitle;

    strlcpy(szInfo, pwps->pszClassName, sizeof(szInfo));

    if (pwps->pClassObject)
    {
        if (pszClassTitle = _wpclsQueryTitle(pwps->pClassObject))
            sprintf(szInfo, "%s (\"%s\")\n",
                    pwps->pszClassName,
                    pszClassTitle);
    }

    if (pwps->pRecord->flRecordAttr & CRA_INUSE)
    {
        sprintf(szInfo,
                "%s\n%s",
                cmnGetString(ID_XSSI_SB_CLASSMNEMONICS),
                stbQueryClassMnemonics(pwps->pClassObject));
        mrc = (MPARAM)TRUE;
    }
    else
        cmnGetString2(szInfo,
                      ID_XSSI_SB_CLASSNOTSUPPORTED,
                      sizeof(szInfo));

    WinSetWindowText((HWND)mphwndInfo, szInfo);

    return mrc;
}

#endif

static const CONTROLDEF
    SBEnable = LOADDEF_AUTOCHECKBOX(ID_XSDI_ENABLESTATUSBAR),
    SBVisibleInGroup = LOADDEF_GROUP(ID_XSDI_VISIBLEIN_GROUP, DEFAULT_TABLE_WIDTH),
    SBVisIconCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_SBFORICONVIEWS),
    SBVisTreeCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_SBFORTREEVIEWS),
    SBVisDetailsCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_SBFORDETAILSVIEWS),
    SBVisSplitCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_SBFORSPLITVIEWS),     // V1.0.1 (2002-11-30) [umoeller]
    SBStyleGroup = LOADDEF_GROUP(ID_XSDI_STYLE_GROUP, DEFAULT_TABLE_WIDTH),
    SBRaisedRadio = LOADDEF_FIRST_AUTORADIO(ID_XSDI_SBSTYLE_3RAISED),
    SBSunkenRadio = LOADDEF_NEXT_AUTORADIO(ID_XSDI_SBSTYLE_3SUNKEN),
    SBButtonRadio = LOADDEF_NEXT_AUTORADIO(ID_XSDI_SBSTYLE_4RECT),
    SBMenuRadio = LOADDEF_NEXT_AUTORADIO(ID_XSDI_SBSTYLE_4MENU);

static const DLGHITEM G_dlgStatusBar1[] =
    {
        START_TABLE,
            START_ROW(0),
                CONTROL_DEF(&SBEnable),
            START_ROW(0),
                START_GROUP_TABLE(&SBVisibleInGroup),
                    START_ROW(0),
                        CONTROL_DEF(&SBVisIconCB),
                    START_ROW(0),
                        CONTROL_DEF(&SBVisTreeCB),
                    START_ROW(0),
                        CONTROL_DEF(&SBVisDetailsCB),
                    START_ROW(0),
                        CONTROL_DEF(&SBVisSplitCB),
                END_TABLE,
            START_ROW(0),
                START_GROUP_TABLE(&SBStyleGroup),
                    START_ROW(0),
                        CONTROL_DEF(&SBRaisedRadio),
                    START_ROW(0),
                        CONTROL_DEF(&SBSunkenRadio),
                    START_ROW(0),
                        CONTROL_DEF(&SBButtonRadio),
                    START_ROW(0),
                        CONTROL_DEF(&SBMenuRadio),
                END_TABLE,
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE,
    };

static const XWPSETTING G_StatusBar1Backup[] =
    {
        sfDefaultStatusBarVisibility,
        sflSBForViews,
        sulSBStyle
    };

/*
 *@@ SetViewsFromFlags:
 *      sets the "for views" checkboxes according to
 *      the given flags. Shared between status and
 *      tool bar pages.
 *
 *@@added V1.0.1 (2002-12-08) [umoeller]
 */

STATIC VOID SetViewsFromFlags(HWND hwndDlgPage,
                              XWPSETTING s)
{
    static const struct
        {
            ULONG idDlg,
                  flForView;
        } flags[] =
        {
            ID_XSDI_SBFORICONVIEWS, SBV_ICON,
            ID_XSDI_SBFORTREEVIEWS, SBV_TREE,
            ID_XSDI_SBFORDETAILSVIEWS, SBV_DETAILS,
            ID_XSDI_SBFORSPLITVIEWS, SBV_SPLIT // V1.0.1 (2002-11-30) [umoeller]
        };

    ULONG fl = cmnQuerySetting(s),
          ul;

    for (ul = 0;
         ul < ARRAYITEMCOUNT(flags);
         ++ul)
    {
        winhSetDlgItemChecked(hwndDlgPage, flags[ul].idDlg,
                              (fl & flags[ul].flForView) != 0);
    }
}

/*
 *@@ stbStatusBar1InitPage:
 *      notebook callback function (notebook.c) for the
 *      first "Status bars" page in the "Workplace Shell" object.
 *      Sets the controls on the page according to the
 *      global settings.
 *
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 *@@changed V0.9.0 [umoeller]: moved this func here from xfwps.c
 *@@changed V0.9.19 (2002-04-24) [umoeller]: now using dialog formatter
 *@@changed V1.0.1 (2002-11-30) [umoeller]: added split view setting
 */

VOID stbStatusBar1InitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                           ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        // first call: backup Global Settings for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = cmnBackupSettings(G_StatusBar1Backup,
                                        ARRAYITEMCOUNT(G_StatusBar1Backup));

        // insert the controls using the dialog formatter
        // V0.9.19 (2002-04-24) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      G_dlgStatusBar1,
                      ARRAYITEMCOUNT(G_dlgStatusBar1));
    }

    if (flFlags & CBI_SET)
    {
        ULONG fl;
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_ENABLESTATUSBAR,
                              cmnQuerySetting(sfDefaultStatusBarVisibility));

        SetViewsFromFlags(pnbp->hwndDlgPage,
                          sflSBForViews);

        fl = cmnQuerySetting(sulSBStyle);
        switch (fl)
        {
            case SBSTYLE_WARP3RAISED:
                winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_SBSTYLE_3RAISED, TRUE);
            break;

            case SBSTYLE_WARP3SUNKEN:
                winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_SBSTYLE_3SUNKEN, TRUE);
            break;

            case SBSTYLE_WARP4RECT:
                winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_SBSTYLE_4RECT, TRUE);
            break;

            default:
                winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_SBSTYLE_4MENU, TRUE);
        }
    }
}

/*
 *@@ stbStatusBar1ItemChanged:
 *      notebook callback function (notebook.c) for the
 *      first "Status bars" page in the "Workplace Shell" object.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 *@@changed V0.9.0 [umoeller]: moved this func here from xfwps.c
 *@@changed V1.0.1 (2002-11-30) [umoeller]: added split view setting
 */

MRESULT stbStatusBar1ItemChanged(PNOTEBOOKPAGE pnbp,
                                 ULONG ulItemID,
                                 USHORT usNotifyCode,
                                 ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = (MRESULT)0;
    BOOL    fRefreshShowHide = FALSE,
            fRefreshStyle = FALSE;

    ULONG   flViewsChanged = 0;

    switch (ulItemID)
    {
        case ID_XSDI_ENABLESTATUSBAR:
            cmnSetSetting(sfDefaultStatusBarVisibility, ulExtra);
            fRefreshShowHide = TRUE;
        break;

        case ID_XSDI_SBFORICONVIEWS:
            flViewsChanged = SBV_ICON;
        break;

        case ID_XSDI_SBFORTREEVIEWS:
            flViewsChanged = SBV_TREE;
        break;

        case ID_XSDI_SBFORDETAILSVIEWS:
            flViewsChanged = SBV_DETAILS;
        break;

        case ID_XSDI_SBFORSPLITVIEWS:       // V1.0.1 (2002-11-30) [umoeller]
            flViewsChanged = SBV_SPLIT;
        break;

        case ID_XSDI_SBSTYLE_3RAISED:
            cmnSetSetting(sulSBStyle, SBSTYLE_WARP3RAISED);
            fRefreshStyle = TRUE;
        break;

        case ID_XSDI_SBSTYLE_3SUNKEN:
            cmnSetSetting(sulSBStyle, SBSTYLE_WARP3SUNKEN);
            fRefreshStyle = TRUE;
        break;

        case ID_XSDI_SBSTYLE_4RECT:
            cmnSetSetting(sulSBStyle, SBSTYLE_WARP4RECT);
            fRefreshStyle = TRUE;
        break;

        case ID_XSDI_SBSTYLE_4MENU:
            cmnSetSetting(sulSBStyle, SBSTYLE_WARP4MENU);
            fRefreshStyle = TRUE;
        break;

        case DID_UNDO:
            // "Undo" button: get pointer to backed-up Global Settings
            cmnRestoreSettings(pnbp->pUser,
                               ARRAYITEMCOUNT(G_StatusBar1Backup));
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            fRefreshStyle = TRUE;
            fRefreshShowHide = TRUE;
        break;

        case DID_DEFAULT:
            // set the default settings for this settings page
            cmnSetDefaultSettings(pnbp->inbp.ulPageID);
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            fRefreshStyle = TRUE;
            fRefreshShowHide = TRUE;
        break;
    }

    if (flViewsChanged)
    {
        ULONG fl = cmnQuerySetting(sflSBForViews);
        if (ulExtra)
            fl |= flViewsChanged;
        else
            fl &= ~flViewsChanged;
        cmnSetSetting(sflSBForViews, fl);
        fRefreshShowHide = TRUE;
    }

    // have the Worker thread update the
    // status bars for all currently open
    // folders
    if (fRefreshStyle)
        xthrPostWorkerMsg(WOM_UPDATEALLSTATUSBARS,
                          (MPARAM)2,
                          0);

    if (fRefreshShowHide)
    {
        xthrPostWorkerMsg(WOM_UPDATEALLSTATUSBARS,
                          (MPARAM)1,
                          0);
        ntbUpdateVisiblePage(NULL,   // all somSelf's
                             SP_XFOLDER_FLDR);
    }

    return mrc;
}

#ifndef __NOCFGSTATUSBARS__

/*
 *@@ RefreshClassObject:
 *      returns the class object for szSBClassSelected.
 *      Added with V0.9.16 to fix the SOM string resource
 *      leaks finally.
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 */

STATIC VOID RefreshClassObject(PSTATUSBARPAGEDATA psbpd)
{
    somId somidClassSelected;
    if (somidClassSelected = somIdFromString(psbpd->szSBClassSelected))
    {
        // get pointer to class object (e.g. M_WPObject)
        psbpd->pSBClassObjectSelected = _somFindClass(SOMClassMgrObject,
                                                      somidClassSelected,
                                                      0,
                                                      0);
        SOMFree(somidClassSelected);        // V0.9.16 (2001-10-28) [umoeller]
    }
}

static const XWPSETTING G_StatusBar2Backup[] =
    {
        sflDereferenceShadows
    };

/*
 *@@ stbStatusBar2InitPage:
 *      notebook callback function (notebook.c) for the
 *      second "Status bars" page in the "Workplace Shell" object.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 *@@changed V0.9.0 [umoeller]: added "Dereference shadows"
 *@@changed V0.9.0 [umoeller]: moved this func here from xfwps.c
 *@@changed V0.9.5 (2000-10-07) [umoeller]: added "Dereference shadows" for multiple mode
 *@@changed V0.9.14 (2001-07-31) [umoeller]: added "Keys" buttons support
 */

VOID stbStatusBar2InitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                               ULONG flFlags)        // CBI_* flags (notebook.h)
{
    PSTATUSBARPAGEDATA psbpd = (PSTATUSBARPAGEDATA)pnbp->pUser2;

    if (flFlags & CBI_INIT)
    {
        // first call: backup Global Settings for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = cmnBackupSettings(G_StatusBar2Backup,
                                         ARRAYITEMCOUNT(G_StatusBar2Backup));

        pnbp->pUser2
        = psbpd
            = NEW(STATUSBARPAGEDATA);
        ZERO(psbpd);

        strlcpy(psbpd->szSBTextNoneSelBackup,
                cmnQueryStatusBarSetting(SBS_TEXTNONESEL),
                sizeof(psbpd->szSBTextNoneSelBackup));
        strlcpy(psbpd->szSBTextMultiSelBackup,
                cmnQueryStatusBarSetting(SBS_TEXTMULTISEL),
                sizeof(psbpd->szSBTextMultiSelBackup));
        // status bar settings page: get last selected
        // class from INIs (for single-object mode)
        // and query the SOM class object from this string
        PrfQueryProfileString(HINI_USER,
                              (PSZ)INIAPP_XWORKPLACE,
                              (PSZ)INIKEY_SB_LASTCLASS,
                              (PSZ)G_pcszXFldObject,     // "XFldObject", default
                              psbpd->szSBClassSelected,
                              sizeof(psbpd->szSBClassSelected));
        if (psbpd->pSBClassObjectSelected == NULL)
            RefreshClassObject(psbpd);

        if (psbpd->pSBClassObjectSelected)
            strlcpy(psbpd->szSBText1SelBackup,
                    stbQueryClassMnemonics(psbpd->pSBClassObjectSelected),
                    sizeof(psbpd->szSBText1SelBackup));

        ctlMakeMenuButton(WinWindowFromID(pnbp->hwndDlgPage, ID_XSDI_SBKEYSNONESEL), 0, 0);
        ctlMakeMenuButton(WinWindowFromID(pnbp->hwndDlgPage, ID_XSDI_SBKEYS1SEL), 0, 0);
        ctlMakeMenuButton(WinWindowFromID(pnbp->hwndDlgPage, ID_XSDI_SBKEYSMULTISEL), 0, 0);

    }

    if (flFlags & CBI_SET)
    {
        // current class
        WinSetDlgItemText(pnbp->hwndDlgPage,
                          ID_XSDI_SBCURCLASS,
                          psbpd->szSBClassSelected);

        // no-object mode
        WinSendDlgItemMsg(pnbp->hwndDlgPage, ID_XSDI_SBTEXTNONESEL,
                          EM_SETTEXTLIMIT,
                          (MPARAM)(CCHMAXMNEMONICS-1),
                          0);
        WinSetDlgItemText(pnbp->hwndDlgPage,
                          ID_XSDI_SBTEXTNONESEL ,
                          (PSZ)cmnQueryStatusBarSetting(SBS_TEXTNONESEL));

        // one-object mode
        WinSendDlgItemMsg(pnbp->hwndDlgPage,
                          ID_XSDI_SBTEXT1SEL,
                          EM_SETTEXTLIMIT,
                          (MPARAM)(CCHMAXMNEMONICS-1),
                          0);
        if (psbpd->pSBClassObjectSelected == NULL)
            RefreshClassObject(psbpd);

        if (psbpd->pSBClassObjectSelected)
            WinSetDlgItemText(pnbp->hwndDlgPage,
                              ID_XSDI_SBTEXT1SEL,
                              stbQueryClassMnemonics(psbpd->pSBClassObjectSelected));

        // dereference shadows
        winhSetDlgItemChecked(pnbp->hwndDlgPage,
                              ID_XSDI_DEREFSHADOWS_SINGLE,
                              (cmnQuerySetting(sflDereferenceShadows) & STBF_DEREFSHADOWS_SINGLE)
                                    != 0);

        // multiple-objects mode
        WinSendDlgItemMsg(pnbp->hwndDlgPage,
                          ID_XSDI_SBTEXTMULTISEL,
                          EM_SETTEXTLIMIT,
                          (MPARAM)(CCHMAXMNEMONICS-1),
                          0);
        WinSetDlgItemText(pnbp->hwndDlgPage,
                          ID_XSDI_SBTEXTMULTISEL,
                          (PSZ)cmnQueryStatusBarSetting(SBS_TEXTMULTISEL));

        winhSetDlgItemChecked(pnbp->hwndDlgPage,
                              ID_XSDI_DEREFSHADOWS_MULTIPLE,
                              (cmnQuerySetting(sflDereferenceShadows) & STBF_DEREFSHADOWS_MULTIPLE)
                                    != 0);
    }

    if (flFlags & CBI_DESTROY)
    {
        if (psbpd)
        {
            if (psbpd->pSBClassObjectSelected)
                psbpd->pSBClassObjectSelected = NULL;

            winhDestroyWindow(&psbpd->hwndKeysMenu),

            free(psbpd);

            pnbp->pUser2 = NULL;
        }
    }
}

/*
 *@@ KEYARRAYITEM:
 *      specifies one status bar mnemonic
 *      to be inserted into a "Keys" menu.
 *
 *      If the first character oc pcszKey is '@',
 *      we insert a submenu with the format
 *      specifiers.
 *
 *@@added V0.9.14 (2001-07-31) [umoeller]
 */

typedef struct _KEYARRAYITEM
{
    ULONG       ulItemID;               // menu item ID (hard-coded)
    const char  *pcszKey;               // actual mnemonic (e.g. "$C")
    ULONG       ulDescription;          // string ID for description
} KEYARRAYITEM, *PKEYARRAYITEM;

static const KEYARRAYITEM
    G_aFormatSubKeys[] =
    {
        1, "b", ID_XSSI_SBMNC_1,       // "in bytes"
        2, "k", ID_XSSI_SBMNC_2,       // "in kBytes"
        3, "K", ID_XSSI_SBMNC_3,       // "in KBytes"
        4, "m", ID_XSSI_SBMNC_4,       // "in mBytes"
        5, "M", ID_XSSI_SBMNC_5,       // "in MBytes"
        6, "a", ID_XSSI_SBMNC_6,       // "in bytes/kBytes/mBytes/gBytes"
        7, "A", ID_XSSI_SBMNC_7       // "in bytes/KBytes/MBytes/GBytes"
    },

    G_aAllModeKeys[] =
    {
        31000, "$c", ID_XSSI_SBMNC_000,       // "no. of selected objects"
        31001, "$C", ID_XSSI_SBMNC_001,       // "total object count"

        31010, "@$f", ID_XSSI_SBMNC_010,       // "free space on drive"
        31020, "@$z", ID_XSSI_SBMNC_020,       // "total size of drive"
        31030, "@$s", ID_XSSI_SBMNC_030,       // "size of selected objects in bytes"
        31040, "@$S", ID_XSSI_SBMNC_040       // "size of folder content in bytes"
    },

    G_a1ObjectCommonKeys[] =
    {
        31100, "$t", ID_XSSI_SBMNC_100,       // "object title"
        31110, "$w", ID_XSSI_SBMNC_110,       // "WPS class default title"
        31120, "$W", ID_XSSI_SBMNC_120       // "WPS class name"
    },

    G_a1ObjectWPDiskKeys[] =
    {
        31200, "$F", ID_XSSI_SBMNC_200,       // "file system type (HPFS, FAT, CDFS, ...)"

        31210, "$L", ID_XSSI_SBMNC_210,       // "drive label"

        // skipping "free space on drive in bytes" which is redefined

        31220, "@$z", ID_XSSI_SBMNC_220       // "total space on drive in bytes"
    },

    G_a1ObjectWPFileSystemKeys[] =
    {
        31300, "$r", ID_XSSI_SBMNC_300,       // "object's real name"

        31310, "$y", ID_XSSI_SBMNC_310,       // "object type (.TYPE EA)"
        31320, "$D", ID_XSSI_SBMNC_320,       // "object creation date"
        31330, "$T", ID_XSSI_SBMNC_330,       // "object creation time"
        31340, "$a", ID_XSSI_SBMNC_340,       // "object attributes"

        31350, "$Eb", ID_XSSI_SBMNC_350,       // "EA size in bytes"
        31360, "$Ek", ID_XSSI_SBMNC_360,       // "EA size in kBytes"
        31370, "$EK", ID_XSSI_SBMNC_370,       // "EA size in KBytes"
        31380, "$Ea", ID_XSSI_SBMNC_380,       // "EA size in bytes/kBytes"
        31390, "$EA", ID_XSSI_SBMNC_390       // "EA size in bytes/KBytes"
    },

    G_a1ObjectWPUrlKeys[] =
    {
        31400, "$U", ID_XSSI_SBMNC_400       // "URL"
    },

    G_a1ObjectWPProgramKeys[] =
    {
        31500, "$p", ID_XSSI_SBMNC_500,       // "executable program file"
        31510, "$P", ID_XSSI_SBMNC_510,       // "parameter list"
        31520, "$d", ID_XSSI_SBMNC_520       // "working directory"
    };

/*
 *@@ InsertKeysIntoMenu:
 *      helper for building the "Keys" menu button menus.
 *
 *@@added V0.9.14 (2001-07-31) [umoeller]
 */

STATIC VOID InsertKeysIntoMenu(HWND hwndMenu,
                               const KEYARRAYITEM *paKeys,
                               ULONG cKeys,
                               BOOL fSeparatorBefore)
{
    ULONG ul;
    XSTRING str;
    xstrInit(&str, 100);

    if (fSeparatorBefore)
        cmnInsertSeparator(hwndMenu, MIT_END);

    for (ul = 0;
         ul < cKeys;
         ul++)
    {
        xstrcpy(&str, cmnGetString(paKeys[ul].ulDescription), 0);

        // if the first char is '@', build a submenu
        // with the various format characters
        if (*(paKeys[ul].pcszKey) == '@')
        {
            HWND hwndSubmenu = winhInsertSubmenu(hwndMenu,
                                                 MIT_END,
                                                 paKeys[ul].ulItemID,
                                                 str.psz,
                                                 MIS_TEXT,
                                                 0, NULL, 0, 0);
            ULONG ul2;
            for (ul2 = 0;
                 ul2 < ARRAYITEMCOUNT(G_aFormatSubKeys);
                 ul2++)
            {
                xstrcpy(&str, cmnGetString(G_aFormatSubKeys[ul2].ulDescription), 0);
                xstrcatc(&str, '\t');
                xstrcat(&str, paKeys[ul].pcszKey + 1, 0);
                xstrcat(&str, G_aFormatSubKeys[ul2].pcszKey, 0);

                winhInsertMenuItem(hwndSubmenu,
                                   MIT_END,
                                   G_aFormatSubKeys[ul2].ulItemID
                                        + paKeys[ul].ulItemID,
                                   str.psz,
                                   MIS_TEXT,
                                   0);
            }
        }
        else
        {
            // first character is not '@':
            // simply insert as such
            xstrcatc(&str, '\t');
            xstrcat(&str, paKeys[ul].pcszKey, 0);

            winhInsertMenuItem(hwndMenu,
                               MIT_END,
                               paKeys[ul].ulItemID,
                               str.psz,
                               MIS_TEXT,
                               0);
        }
    }

    xstrClear(&str);
}

/*
 *@@ CreateKeysMenu:
 *
 *@@added V0.9.14 (2001-07-31) [umoeller]
 */

STATIC MRESULT CreateKeysMenu(PSTATUSBARPAGEDATA psbpd,
                              ULONG ulItemID)
{
    HPOINTER hptrOld = winhSetWaitPointer();

    winhDestroyWindow(&psbpd->hwndKeysMenu);

    // different button or first call:
    // build menu then
    psbpd->hwndKeysMenu = WinCreateMenu(HWND_DESKTOP, NULL);

    InsertKeysIntoMenu(psbpd->hwndKeysMenu,
                       G_aAllModeKeys,
                       ARRAYITEMCOUNT(G_aAllModeKeys),
                       FALSE);
    if (ulItemID == ID_XSDI_SBKEYS1SEL)
    {
        InsertKeysIntoMenu(psbpd->hwndKeysMenu,
                           G_a1ObjectCommonKeys,
                           ARRAYITEMCOUNT(G_a1ObjectCommonKeys),
                           TRUE);

        if (_somDescendedFrom(psbpd->pSBClassObjectSelected,
                              _WPFileSystem))
        {
            SOMClass *pWPUrl;

            InsertKeysIntoMenu(psbpd->hwndKeysMenu,
                               G_a1ObjectWPFileSystemKeys,
                               ARRAYITEMCOUNT(G_a1ObjectWPFileSystemKeys),
                               TRUE);

            if (    (pWPUrl = ctsResolveWPUrl())
                 && (_somDescendedFrom(psbpd->pSBClassObjectSelected,
                                       pWPUrl))
               )
                InsertKeysIntoMenu(psbpd->hwndKeysMenu,
                                   G_a1ObjectWPUrlKeys,
                                   ARRAYITEMCOUNT(G_a1ObjectWPUrlKeys),
                                   TRUE);
        }
        else if (_somDescendedFrom(psbpd->pSBClassObjectSelected,
                                   _WPProgram))
                InsertKeysIntoMenu(psbpd->hwndKeysMenu,
                                   G_a1ObjectWPProgramKeys,
                                   ARRAYITEMCOUNT(G_a1ObjectWPProgramKeys),
                                   TRUE);
        else if (_somDescendedFrom(psbpd->pSBClassObjectSelected,
                                   _WPDisk))
                InsertKeysIntoMenu(psbpd->hwndKeysMenu,
                                   G_a1ObjectWPDiskKeys,
                                   ARRAYITEMCOUNT(G_a1ObjectWPDiskKeys),
                                   TRUE);
    }

    // store what we had so we can reuse the menu
    psbpd->ulLastKeysID = ulItemID;

    WinSetPointer(HWND_DESKTOP, hptrOld);

    return (MRESULT)psbpd->hwndKeysMenu;
}

/*
 *@@ stbStatusBar2ItemChanged:
 *      notebook callback function (notebook.c) for the
 *      second "Status bars" page in the "Workplace Shell" object.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@changed V0.9.0 [umoeller]: adjusted entry field handling to new notebook.c handling
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 *@@changed V0.9.0 [umoeller]: added "Dereference shadows"
 *@@changed V0.9.0 [umoeller]: moved this func here from xfwps.c
 *@@changed V0.9.5 (2000-10-07) [umoeller]: added "Dereference shadows" for multiple mode
 *@@changed V0.9.14 (2001-07-31) [umoeller]: added "Keys" buttons support
 *@@changed V0.9.14 (2001-07-31) [umoeller]: "Undo" didn't undo everything, fixed
 *@@changed V0.9.19 (2002-06-02) [umoeller]: fixed minor memory leak
 */

MRESULT stbStatusBar2ItemChanged(PNOTEBOOKPAGE pnbp,
                                 ULONG ulItemID,
                                 USHORT usNotifyCode,
                                 ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT     mrc = (MPARAM)0;
    BOOL        fSave = TRUE,
                fReadEFs = FALSE;           // read codes from entry fields?
                                            // V0.9.14 (2001-07-31) [umoeller]
    CHAR        szDummy[CCHMAXMNEMONICS];
    PSTATUSBARPAGEDATA psbpd = (PSTATUSBARPAGEDATA)pnbp->pUser2;

    ULONG       flChanged = 0;

    switch (ulItemID)
    {
        case ID_XSDI_SBTEXTNONESEL:
            if (usNotifyCode == EN_KILLFOCUS)   // changed V0.9.0
                fReadEFs = TRUE;
            else
                fSave = FALSE;
        break;

        case ID_XSDI_SBTEXT1SEL:
            if (usNotifyCode == EN_KILLFOCUS)   // changed V0.9.0
                fReadEFs = TRUE;
            else
                fSave = FALSE;
        break;

        case ID_XSDI_DEREFSHADOWS_SINGLE:    // added V0.9.0
            flChanged = STBF_DEREFSHADOWS_SINGLE;
        break;

        case ID_XSDI_SBTEXTMULTISEL:
            if (usNotifyCode == EN_KILLFOCUS)   // changed V0.9.0
                fReadEFs = TRUE;
            else
                fSave = FALSE;
        break;

        case ID_XSDI_DEREFSHADOWS_MULTIPLE:    // added V0.9.5 (2000-10-07) [umoeller]
            flChanged = STBF_DEREFSHADOWS_MULTIPLE;
        break;

        // "Select class" on "Status Bars" page:
        // set up WPS classes dialog
        case ID_XSDI_SBSELECTCLASS:
        {
            SELECTCLASSDATA         scd;
            STATUSBARSELECTCLASS    sbsc;

            XSTRING strTitle,
                    strIntroText;
            xstrInit(&strTitle, 0);
            xstrInit(&strIntroText, 0);
            cmnGetMessage(NULL, 0, &strTitle, 112);
            cmnGetMessage(NULL, 0, &strIntroText, 113);
            scd.pszDlgTitle = strTitle.psz;
            scd.pszIntroText = strIntroText.psz;
            scd.pcszRootClass = G_pcszWPObject;
            scd.pcszOrphans = NULL;
            strlcpy(scd.szClassSelected, psbpd->szSBClassSelected, sizeof(scd.szClassSelected));

            // these callback funcs are defined way more below
            scd.pfnwpReturnAttrForClass = fncbWPSStatusBarReturnClassAttr;
            scd.pfnwpClassSelected = fncbWPSStatusBarClassSelected;
            // the folllowing data will be passed to the callbacks
            // so the callback can display NLS messages
            scd.ulUserClassSelected = (ULONG)&sbsc;

            scd.pszHelpLibrary = cmnQueryHelpLibrary();
            scd.ulHelpPanel = ID_XFH_SELECTCLASS;

            // classlst.c
            if (clsSelectWpsClassDlg(pnbp->hwndFrame, // owner
                                     cmnQueryNLSModuleHandle(FALSE),
                                     ID_XLD_SELECTCLASS,
                                     &scd)
                          == DID_OK)
            {
                strlcpy(psbpd->szSBClassSelected, scd.szClassSelected, sizeof(psbpd->szSBClassSelected));
                WinSetDlgItemText(pnbp->hwndDlgPage,
                                  ID_XSDI_SBCURCLASS,
                                  psbpd->szSBClassSelected);
                PrfWriteProfileString(HINI_USER,
                                      (PSZ)INIAPP_XWORKPLACE,
                                      (PSZ)INIKEY_SB_LASTCLASS,
                                      psbpd->szSBClassSelected);
                if (psbpd->pSBClassObjectSelected)
                    psbpd->pSBClassObjectSelected = NULL;
                    // this will provoke the following func to re-read
                    // the class's status bar mnemonics

                // update the display by calling the INIT callback
                pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);

                // refresh the "Undo" data for this
                strlcpy(psbpd->szSBText1SelBackup,
                        stbQueryClassMnemonics(psbpd->pSBClassObjectSelected),
                        sizeof(psbpd->szSBText1SelBackup));

            }

            xstrClear(&strTitle);
            xstrClear(&strIntroText);
        }
        break;

        // "Keys" buttons next to entry field
        // V0.9.14 (2001-07-31) [umoeller]
        case ID_XSDI_SBKEYSNONESEL:
        case ID_XSDI_SBKEYS1SEL:
        case ID_XSDI_SBKEYSMULTISEL:
            mrc = CreateKeysMenu(psbpd,
                                 ulItemID);
        break;

        case DID_UNDO:
        {
            // "Undo" button: get pointer to backed-up Global Settings
            cmnRestoreSettings(pnbp->pUser,
                               ARRAYITEMCOUNT(G_StatusBar2Backup));

            cmnSetStatusBarSetting(SBS_TEXTNONESEL,
                                   psbpd->szSBTextNoneSelBackup);
            cmnSetStatusBarSetting(SBS_TEXTMULTISEL,
                                   psbpd->szSBTextMultiSelBackup);

            if (!psbpd->pSBClassObjectSelected)
                RefreshClassObject(psbpd);

            if (psbpd->pSBClassObjectSelected)
                stbSetClassMnemonics(psbpd->pSBClassObjectSelected,
                                     psbpd->szSBText1SelBackup);

            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        }
        break;

        case DID_DEFAULT:
        {
            // set the default settings for this settings page
            // (this is in common.c because it's also used at
            // Desktop startup)
            cmnSetStatusBarSetting(SBS_TEXTNONESEL, NULL);  // load default
            cmnSetStatusBarSetting(SBS_TEXTMULTISEL, NULL); // load default

            cmnSetSetting(sflDereferenceShadows, STBF_DEREFSHADOWS_SINGLE);
                        // V0.9.14 (2001-07-31) [umoeller]

            if (!psbpd->pSBClassObjectSelected)
                RefreshClassObject(psbpd);

            if (psbpd->pSBClassObjectSelected)
                stbSetClassMnemonics(psbpd->pSBClassObjectSelected,
                                     NULL);  // load default

            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        }
        break;

        default:
        {
            fSave = FALSE;

            if (ulItemID >= 31000)
            {
                // one of the menu item IDs from the "Keys" menu:
                // look up the item in the menu we built then
                PSZ psz;
                if (    (psbpd->hwndKeysMenu)
                     && (psz = winhQueryMenuItemText(psbpd->hwndKeysMenu,
                                                     ulItemID))
                   )
                {
                    // alright, now we got the menu item text...
                    // find the tab character, after which is the
                    // key we need to insert
                    PCSZ p;
                    if (p = strchr(psz, '\t'))
                    {
                        // p points to the key to insert now...
                        // find the entry field to insert into,
                        // this can come in for any of the three
                        // buttons

                        ULONG ulEFID = 0;
                        switch (psbpd->ulLastKeysID)
                        {
                            case ID_XSDI_SBKEYSNONESEL:
                                ulEFID = ID_XSDI_SBTEXTNONESEL;
                            break;

                            case ID_XSDI_SBKEYS1SEL:
                                ulEFID = ID_XSDI_SBTEXT1SEL;
                            break;

                            case ID_XSDI_SBKEYSMULTISEL:
                                ulEFID = ID_XSDI_SBTEXTMULTISEL;
                            break;
                        }

                        if (ulEFID)
                        {
                            HWND hwndEF = WinWindowFromID(pnbp->hwndDlgPage,
                                                          ulEFID);
                            MRESULT mr = WinSendMsg(hwndEF,
                                                    EM_QUERYSEL,
                                                    0,
                                                    0);
                            SHORT s1 = SHORT1FROMMR(mr),
                                  s2 = SHORT2FROMMR(mr);

                            PSZ pszOld;

                            if (pszOld = winhQueryWindowText(hwndEF))
                            {
                                XSTRING strNew;
                                ULONG   ulNewLength = strlen(p + 1);

                                xstrInitSet(&strNew, pszOld);

                                xstrrpl(&strNew,
                                        // first char to replace:
                                        s1,
                                        // no. of chars to replace:
                                        s2 - s1,
                                        // string to replace chars with:
                                        p + 1,
                                        ulNewLength);

                                WinSetWindowText(hwndEF,
                                                 strNew.psz);

                                WinSendMsg(hwndEF,
                                           EM_SETSEL,
                                           MPFROM2SHORT(s1,
                                                        s1 + ulNewLength),
                                           0);

                                WinSetFocus(HWND_DESKTOP, hwndEF);

                                xstrClear(&strNew);

                                fSave = TRUE;
                                fReadEFs = TRUE;

                                free(pszOld);
                                        // was missing V0.9.19 (2002-06-02) [umoeller]
                            }
                        }
                    }

                    free(psz);
                }
            }
        }
    }

    if (flChanged)
    {
        ULONG fl = cmnQuerySetting(sflDereferenceShadows);
        if (ulExtra)
            fl |= flChanged;
        else
            fl &= ~flChanged;
        cmnSetSetting(sflDereferenceShadows, fl);
    }

    if (fSave)
    {
        if (fReadEFs)
        {
            // "none selected" codes:

            WinQueryDlgItemText(pnbp->hwndDlgPage,
                                ID_XSDI_SBTEXTNONESEL,
                                sizeof(szDummy)-1, szDummy);
            cmnSetStatusBarSetting(SBS_TEXTNONESEL, szDummy);

            if (!psbpd->pSBClassObjectSelected)
                RefreshClassObject(psbpd);

            // "one selected" codes:
            if (psbpd->pSBClassObjectSelected)
            {
                WinQueryDlgItemText(pnbp->hwndDlgPage, ID_XSDI_SBTEXT1SEL,
                                    sizeof(szDummy)-1, szDummy);
                stbSetClassMnemonics(psbpd->pSBClassObjectSelected,
                                     szDummy);
            }

            // "multiple selected" codes:
            WinQueryDlgItemText(pnbp->hwndDlgPage, ID_XSDI_SBTEXTMULTISEL,
                                sizeof(szDummy)-1, szDummy);
            cmnSetStatusBarSetting(SBS_TEXTMULTISEL, szDummy);
        }

        // have the Worker thread update the
        // status bars for all currently open
        // folders
        xthrPostWorkerMsg(WOM_UPDATEALLSTATUSBARS,
                          (MPARAM)2, // update display
                          0);
    }

    return mrc;
}

#endif // __NOCFGSTATUSBARS__

/* ******************************************************************
 *
 *   Notebook callbacks (notebook.c) for "Tool bars" pages
 *
 ********************************************************************/

static const CONTROLDEF
    TBEnable = LOADDEF_AUTOCHECKBOX(ID_XSDI_ENABLETOOLBAR),
    // note, we re-use the status bar globals from above for
    // the "visible in groups" stuff
    TBStyleTextCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_TBSTYLE_TEXT),
    TBStyleMiniCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_TBSTYLE_MINI),
    TBStyleFlatCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_TBSTYLE_FLAT),
    TBStyleHiliteCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_TBSTYLE_HILITE),
    TBStyleTooltipsCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_TBSTYLE_TOOLTIPS);

static const DLGHITEM G_dlgToolBar1[] =
    {
        START_TABLE,
            START_ROW(0),
                CONTROL_DEF(&TBEnable),
            START_ROW(0),
                START_GROUP_TABLE(&SBVisibleInGroup),
                    START_ROW(0),
                        CONTROL_DEF(&SBVisIconCB),
                    START_ROW(0),
                        CONTROL_DEF(&SBVisTreeCB),
                    START_ROW(0),
                        CONTROL_DEF(&SBVisDetailsCB),
                    START_ROW(0),
                        CONTROL_DEF(&SBVisSplitCB),
                END_TABLE,
            START_ROW(0),
                START_GROUP_TABLE(&SBStyleGroup),
                    START_ROW(0),
                        CONTROL_DEF(&TBStyleTextCB),
                    START_ROW(0),
                        CONTROL_DEF(&TBStyleMiniCB),
                    START_ROW(0),
                        CONTROL_DEF(&TBStyleFlatCB),
                    START_ROW(0),
                        CONTROL_DEF(&TBStyleHiliteCB),
                    START_ROW(0),
                        CONTROL_DEF(&TBStyleTooltipsCB),
                END_TABLE,
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE,
    };

static const XWPSETTING G_ToolBar1Backup[] =
    {
        sfDefaultToolBarVisibility,
        sflTBForViews,
        sflToolBarStyle,
        sfTBToolTips
    };

/*
 *@@ stbToolBar1InitPage:
 *      notebook callback function (notebook.c) for the
 *      first "Tool bars" page in the "Workplace Shell" object.
 *      Sets the controls on the page according to the
 *      global settings.
 *
 *@@added V1.0.1 (2002-12-08) [umoeller]
 */

VOID stbToolBar1InitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                         ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        // first call: backup Global Settings for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = cmnBackupSettings(G_ToolBar1Backup,
                                        ARRAYITEMCOUNT(G_ToolBar1Backup));

        // insert the controls using the dialog formatter
        // V0.9.19 (2002-04-24) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      G_dlgToolBar1,
                      ARRAYITEMCOUNT(G_dlgToolBar1));
    }

    if (flFlags & CBI_SET)
    {
        ULONG fl;

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_ENABLETOOLBAR,
                              cmnQuerySetting(sfDefaultToolBarVisibility));

        SetViewsFromFlags(pnbp->hwndDlgPage,
                          sflTBForViews);

        fl = cmnQuerySetting(sflToolBarStyle);

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_TBSTYLE_TEXT,
                              !!(fl & TBBS_TEXT));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_TBSTYLE_MINI,
                              !!(fl & TBBS_MINIICON));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_TBSTYLE_FLAT,
                              !!(fl & TBBS_FLAT));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_TBSTYLE_HILITE,
                              !!(fl & TBBS_HILITE));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_TBSTYLE_TOOLTIPS,
                              cmnQuerySetting(sfTBToolTips));
    }
}

/*
 *@@ stbToolBar1ItemChanged:
 *      notebook callback function (notebook.c) for the
 *      first "Tool bars" page in the "Workplace Shell" object.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@added V1.0.1 (2002-12-08) [umoeller]
 */

MRESULT stbToolBar1ItemChanged(PNOTEBOOKPAGE pnbp,
                               ULONG ulItemID,
                               USHORT usNotifyCode,
                               ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = (MRESULT)0;
    BOOL    fRefreshShowHide = FALSE,
            fRefreshStyle = FALSE;

    ULONG   flViewsChanged = 0,
            flStyleChanged = 0,
            fl;

    switch (ulItemID)
    {
        case ID_XSDI_ENABLETOOLBAR:
            cmnSetSetting(sfDefaultToolBarVisibility, ulExtra);
            fRefreshShowHide = TRUE;
        break;

        case ID_XSDI_SBFORICONVIEWS:
            flViewsChanged = SBV_ICON;
        break;

        case ID_XSDI_SBFORTREEVIEWS:
            flViewsChanged = SBV_TREE;
        break;

        case ID_XSDI_SBFORDETAILSVIEWS:
            flViewsChanged = SBV_DETAILS;
        break;

        case ID_XSDI_SBFORSPLITVIEWS:
            flViewsChanged = SBV_SPLIT;
        break;

        case ID_XSDI_TBSTYLE_TEXT:
            flStyleChanged = TBBS_TEXT;
        break;

        case ID_XSDI_TBSTYLE_MINI:
            fl = cmnQuerySetting(sflToolBarStyle);
            if (ulExtra)
                fl =   fl
                     & ~TBBS_BIGICON
                     | TBBS_MINIICON;
            else
                fl =   fl
                     & ~TBBS_MINIICON
                     | TBBS_BIGICON;
            cmnSetSetting(sflToolBarStyle, fl);
            fRefreshStyle = TRUE;
        break;

        case ID_XSDI_TBSTYLE_FLAT:
            flStyleChanged = TBBS_FLAT;
        break;

        case ID_XSDI_TBSTYLE_HILITE:
            flStyleChanged = TBBS_HILITE;
        break;

        case ID_XSDI_TBSTYLE_TOOLTIPS:
            cmnSetSetting(sfTBToolTips, ulExtra);
        break;

        case DID_UNDO:
            // "Undo" button: get pointer to backed-up Global Settings
            cmnRestoreSettings(pnbp->pUser,
                               ARRAYITEMCOUNT(G_StatusBar1Backup));
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            fRefreshStyle = TRUE;
            fRefreshShowHide = TRUE;
        break;

        case DID_DEFAULT:
            // set the default settings for this settings page
            cmnSetDefaultSettings(pnbp->inbp.ulPageID);
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            fRefreshStyle = TRUE;
            fRefreshShowHide = TRUE;
        break;
    }

    if (flViewsChanged)
    {
        fl = cmnQuerySetting(sflTBForViews);
        if (ulExtra)
            fl |= flViewsChanged;
        else
            fl &= ~flViewsChanged;
        cmnSetSetting(sflTBForViews, fl);
        fRefreshShowHide = TRUE;
    }

    if (flStyleChanged)
    {
        fl = cmnQuerySetting(sflToolBarStyle);
        if (ulExtra)
            fl |= flStyleChanged;
        else
            fl &= ~flStyleChanged;
        cmnSetSetting(sflToolBarStyle, fl);
        fRefreshStyle = TRUE;
    }

    // have the Worker thread update the
    // tool bars for all currently open
    // folders @@todo
    /*
    if (fRefreshStyle)
        xthrPostWorkerMsg(WOM_UPDATEALLSTATUSBARS,
                          (MPARAM)2,
                          0);

    if (fRefreshShowHide)
    {
        xthrPostWorkerMsg(WOM_UPDATEALLSTATUSBARS,
                          (MPARAM)1,
                          0);
        ntbUpdateVisiblePage(NULL,   // all somSelf's
                             SP_XFOLDER_FLDR);
    }
    */

    return mrc;
}
