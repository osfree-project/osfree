
/*
 *@@sourcefile notebook.c:
 *      this file is new with V0.82 and contains very useful code for
 *      WPS Settings notebooks pages. All XWorkplace notebook pages
 *      are implemented using these routines.
 *
 *      All the functions in this file have the ntb* prefix.
 *
 *      The concept of this is that when inserting a notebook page
 *      by overriding the proper WPS methods for an object, you call
 *      ntbInsertPage here instead of calling wpInsertSettingsPage.
 *      This function will always use the same window procedure
 *      (fnwpPageCommon) and call CALLBACKS for certain notebook
 *      events which you can specify in your call to ntbInsertPage.
 *
 *      See the declaration of CREATENOTEBOOKPAGE in notebook.h for
 *      details about the callbacks.
 *
 *      Advantages of using this code:
 *
 *      1)  Less typing, less code. Writing the same stupid PM
 *          code to react to the typical "checkbox pressed"
 *          notification is really tiresome. fnwpPageCommon has
 *          callback support for the very large majority of PM
 *          events that will come into a dialog proc.
 *
 *      2)  The separation between "init" and "item changed"
 *          callbacks allows for less code also when items need
 *          to be disabled because of certain settings.
 *
 *      3)  Easier support for "Undo" and "Default". The NOTEBOOKPAGE
 *          struct that is maintained for each page has space for
 *          backing up data so you won't have to deal with QWL_USER
 *          and that crap yourself. In conjunction with the new
 *          global settings support (see cmnBackupSettings), this
 *          can make life quite a bit easier.
 *
 *      4)  Maintenance of a global list of all currently open
 *          notebook pages on the system. This allows for refreshing
 *          the display if settings change from some other place.
 *          See ntbQueryOpenPages and ntbUpdateVisiblePage.
 *          For example, if status bars are disabled globally in
 *          "Workplace Shell", the status bar checkboxes should
 *          be disabled in open folder instance notebooks.
 *
 *      5)  Thread safety and exception handling. fnwpPageCommon
 *          installs an exception handler, so all the callbacks are
 *          protected by that handler too.
 *
 *      6)  Dialog formatter support. See ntbFormatPage.
 *
 *@@header "shared\notebook.h"
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

#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#define INCL_WIN
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\winh.h"               // PM helper routines

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file

// other SOM headers
#pragma hdrstop

// finally, our own header file
#include "shared\notebook.h"            // generic XWorkplace notebook handling

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// root of linked list of opened notebook pages
// (this holds NOTEBOOKPAGELISTITEM's)
static LINKLIST        G_llOpenPages;   // this is auto-free

// root of linked list of subclassed notebooks
// (this holds
static LINKLIST        G_llSubclNotebooks; // this is auto-free

// mutex semaphore for both lists
static HMTX            G_hmtxNotebooks = NULLHANDLE;

MRESULT EXPENTRY fnwpSubclNotebook(HWND hwndNotebook, ULONG msg, MPARAM mp1, MPARAM mp2);

/* ******************************************************************
 *
 *   Notebook helpers
 *
 ********************************************************************/

/*
 *@@ LockNotebooks:
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

STATIC BOOL LockNotebooks(VOID)
{
    if (G_hmtxNotebooks)
        return !DosRequestMutexSem(G_hmtxNotebooks, SEM_INDEFINITE_WAIT);

    if (!DosCreateMutexSem(NULL,         // unnamed
                           &G_hmtxNotebooks,
                           0,            // unshared
                           TRUE))        // request!
    {
        lstInit(&G_llOpenPages, TRUE); // NOTEBOOKPAGELISTITEMs are freeable
        lstInit(&G_llSubclNotebooks, TRUE); // SUBCLNOTEBOOKLISTITEM are freeable

        PMPF_NOTEBOOKS(("Created NOTEBOOKPAGELISTITEM list and mutex"));

        return TRUE;
    }

    return FALSE;
}

/*
 *@@ UnlockNotebooks:
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

STATIC VOID UnlockNotebooks(VOID)
{
    DosReleaseMutexSem(G_hmtxNotebooks);
}

/* ******************************************************************
 *
 *   Notebook page dialog function
 *
 ********************************************************************/

/*
 *@@ PageInit:
 *      implementation for WM_INITDLG in
 *      fnwpPageCommon.
 *
 *@@added V0.9.1 (99-12-31) [umoeller]
 */

STATIC VOID PageInit(PNOTEBOOKPAGE pnbp,
                     HWND hwndDlg)
{
    PMPF_NOTEBOOKS(("WM_INITDLG"));

    if (!(pnbp->flPage & NBFL_PAGE_INITED))        // V0.9.19 (2002-04-17) [umoeller]
    {
        // store the dlg hwnd in notebook structure
        pnbp->hwndDlgPage = hwndDlg;

        // store the WM_INITDLG parameter in the
        // window words; the CREATENOTEBOOKPAGE
        // structure is passed to us by ntbInsertPage
        // as a creation parameter in mp2
        WinSetWindowULong(hwndDlg, QWL_USER, (ULONG)pnbp);

        // make Warp 4 notebook buttons and move controls
        winhAssertWarp4Notebook(hwndDlg,
                                100);         // ID threshold
                                // 14);

        // set controls font to 8.Helv, if global settings
        // want this (paranoia page, V0.9.0)
#ifndef __NOPARANOIA__
        if (cmnQuerySetting(sfUse8HelvFont))
            winhSetControlsFont(hwndDlg,
                                0,
                                8000,
                                "8.Helv");
#endif

        // initialize the other fields
        pnbp->preccSource = (PRECORDCORE)-1;
        pnbp->hwndSourceCnr = NULLHANDLE;

        // call "initialize" callback
        if (pnbp->inbp.pfncbInitPage)
            pnbp->inbp.pfncbInitPage(pnbp, CBI_INIT | CBI_SET | CBI_ENABLE);

        // timer desired?
        if (pnbp->inbp.ulTimer)
        {
            WinStartTimer(WinQueryAnchorBlock(hwndDlg),
                          hwndDlg,
                          1,
                          pnbp->inbp.ulTimer);
            // call timer callback already now;
            // let's not wait until the first downrun
            if (pnbp->inbp.pfncbTimer)
                pnbp->inbp.pfncbTimer(pnbp, 1);
        }

        // winhAdjustControls desired?
        if (    (pnbp->inbp.pampControlFlags)
//     #ifndef __ALWAYSRESIZESETTINGSPAGES__            // setting removed V1.0.1 (2002-12-14) [umoeller]
//              && (cmnQuerySetting(sfResizeSettingsPages))
//     #endif
           )
        {
            // yes: allocate and zero
            if (pnbp->pxac = NEW(XADJUSTCTRLS))
            {
                memset(pnbp->pxac, 0, sizeof(XADJUSTCTRLS));
                winhAdjustControls(hwndDlg,
                                   pnbp->inbp.pampControlFlags,
                                   pnbp->inbp.cControlFlags,
                                   NULL,    // INIT
                                   pnbp->pxac);
            }
        }

        pnbp->flPage |= NBFL_PAGE_INITED; // V0.9.19 (2002-04-24) [umoeller]
    }
}

/*
 *@@ PageDestroy:
 *      implementation for WM_DESTROY in
 *      fnwpPageCommon.
 *
 *@@added V0.9.1 (99-12-31) [umoeller]
 *@@changed V0.9.7 (2000-12-10) [umoeller]: fixed mutex problems
 */

STATIC VOID PageDestroy(PNOTEBOOKPAGE pnbp)
{
    PMPF_NOTEBOOKS(("WM_DESTROY"));

    if (pnbp)
    {
        PMPF_NOTEBOOKS(("  found pcnbp"));

        // stop timer, if started
        if (pnbp->inbp.ulTimer)
        {
            PMPF_NOTEBOOKS(("  stopping timer"));

            WinStopTimer(WinQueryAnchorBlock(pnbp->hwndDlgPage),
                         pnbp->hwndDlgPage,
                         1);
        }

        // call INIT callback with CBI_DESTROY
        if (pnbp->inbp.pfncbInitPage)
            pnbp->inbp.pfncbInitPage(pnbp, CBI_DESTROY);

        // tooltip to be destroyed?
        winhDestroyWindow(&pnbp->hwndTooltip);

        // destroy context menus V1.0.1 (2003-01-05) [umoeller]
        winhDestroyWindow(&pnbp->hmenuSel);
        winhDestroyWindow(&pnbp->hmenuWhitespace);

        // winhAdjustControls prepared?
        if (pnbp->pxac)
        {
            // yes: clean up
            winhAdjustControls(pnbp->hwndDlgPage,
                               NULL,    // cleanup
                               0,       // cleanup
                               NULL,    // cleanup
                               pnbp->pxac);
            free(pnbp->pxac);
            pnbp->pxac = NULL;
        }

        // remove the NOTEBOOKPAGELISTITEM from the
        // linked list of open notebook pages
        // V0.9.7 (2000-12-09) [umoeller]: more mutex protection

        PMPF_NOTEBOOKS(("  trying to remove page ID %d from list",
                         pnbp->inbp.ulPageID));

        if (pnbp->pnbli)
        {
            BOOL fLocked = FALSE;
            TRY_LOUD(excpt1)
            {
                if (fLocked = LockNotebooks())
                    lstRemoveItem(&G_llOpenPages,
                                  pnbp->pnbli);  // this is auto-free!
                                // this free's the pnbli
            }
            CATCH(excpt1) {} END_CATCH();

            if (fLocked)
                UnlockNotebooks();
        }

        // free allocated user memory
        if (pnbp->pUser)
            free(pnbp->pUser);
        if (pnbp->pUser2)
            free(pnbp->pUser2);
        _wpFreeMem(pnbp->inbp.somSelf, (PBYTE)pnbp);
    }
}

/*
 *@@ PageWmControl:
 *      implementation for WM_CONTROL in fnwpPageCommon.
 *
 *      hwndDlg is not passed because this can be retrieved
 *      thru pnbp->hwndDlgPage.
 *
 *@@added V0.9.1 (99-12-31) [umoeller]
 *@@changed V0.9.4 (2000-07-11) [umoeller]: added CN_HELP and fPassCnrHelp handling
 *@@changed V0.9.9 (2001-02-06) [umoeller]: added support for direct editing
 *@@changed V0.9.9 (2001-03-15) [lafaix]: added support for valuesets
 *@@changed V0.9.9 (2001-03-27) [umoeller]: changed ulExtra for CN_RECORDCHECKED
 *@@changed V0.9.19 (2002-04-15) [lafaix]: added support for CN_ENTER
 *@@changed V0.9.19 (2002-06-02) [umoeller]: optimizations
 *@@changed V1.0.1 (2003-01-05) [umoeller]: added automatic cnr context menu support for add/edit/remove
 */

STATIC MRESULT EXPENTRY PageWmControl(PNOTEBOOKPAGE pnbp,
                                      ULONG msg,
                                      MPARAM mp1,
                                      MPARAM mp2) // in: as in WM_CONTROL
{
    // code returned to fnwpPageCommon
    MRESULT mrc = 0;

    // identify the source of the msg
    ULONG   ulItemID = SHORT1FROMMP(mp1),
            usNotifyCode = SHORT2FROMMP(mp1);

    BOOL    fCallItemChanged = FALSE;
            // if this becomes TRUE, we'll call the "item changed" callback

    CHAR    szClassName[20];
    ULONG   ulClassCode = 0;
    ULONG   ulExtra = -1;

    PMPF_NOTEBOOKS(("WM_CONTROL"));

    // "item changed" callback defined?
    if (    (pnbp->inbp.pfncbItemChanged)
         && (pnbp->hwndControl = WinWindowFromID(pnbp->hwndDlgPage, ulItemID))
            // we identify the control by querying its class.
            // The standard PM classes have those wicked "#xxxx" classnames;
            // when we find a supported control, we filter out messages
            // which are not of interest, and call the
            // callbacks only for these messages by setting fCallItemChanged to TRUE
         && (WinQueryClassName(pnbp->hwndControl,
                               sizeof(szClassName),
                               szClassName))
            // system class?
         && (szClassName[0] == '#')
        )
    {
        // now translate the class name into a ULONG
        ulClassCode = atoi(szClassName + 1);
        // sscanf(&szClassName[1], "%d", &ulClassCode);

        switch (ulClassCode)
        {
            // checkbox? radio button?
            case 3:
                switch (usNotifyCode)
                {
                    case BN_CLICKED:
                    case BN_DBLCLICKED: // added V0.9.0
                        // code for WC_BUTTON...
                        if (winhQueryWindowStyle(pnbp->hwndControl) & BS_PRIMARYSTYLES)
                                // == 0x000F; BS_PUSHBUTTON has 0x0,
                                // so we exclude pushbuttons here
                        {
                            // for checkboxes and radiobuttons, pass
                            // the new check state to the callback
                            ulExtra = (ULONG)WinSendMsg(pnbp->hwndControl,
                                                        BM_QUERYCHECK,
                                                        MPNULL,
                                                        MPNULL);
                            fCallItemChanged = TRUE;
                        }
                }
            break;

            // spinbutton?
            case 32:
                switch (usNotifyCode)
                {
                    case SPBN_UPARROW:
                    case SPBN_DOWNARROW:
                    case SPBN_CHANGE:   // manual input
                        // for spinbuttons, pass the new spbn
                        // value in ulExtra
                        WinSendMsg(pnbp->hwndControl,
                                   SPBM_QUERYVALUE,
                                   (MPARAM)&ulExtra,
                                   MPFROM2SHORT(0, SPBQ_UPDATEIFVALID));
                        fCallItemChanged = TRUE;
                }
            break;

            // listbox?
            case 7:
            // combobox?
            case 2:
                if (usNotifyCode == LN_SELECT)
                    fCallItemChanged = TRUE;
            break;

            // entry field?
            case 6:
                switch (usNotifyCode)
                {
                    case EN_CHANGE:         // 0x0004
                    case EN_SETFOCUS:       // 0x0001
                    case EN_KILLFOCUS:      // 0x0002
                        fCallItemChanged = TRUE;
                    break;

                    case EN_HOTKEY:
                        // from hotkey entry field (comctl.c):
                        fCallItemChanged = TRUE;
                        ulExtra = (ULONG)mp2;
                            // HOTKEYNOTIFY struct pointer
                    break;
                }
            break;

            // multi-line entry field?
            case 10:
                switch (usNotifyCode)
                {
                    case MLN_CHANGE:        // 0x0007
                    case MLN_SETFOCUS:      // 0x0008
                    case MLN_KILLFOCUS:     // 0x0009
                        fCallItemChanged = TRUE;
                }
            break;

            // container?
            case 37:
                switch (usNotifyCode)
                {
                    case CN_EMPHASIS:
                    {
                        // get cnr notification struct
                        PNOTIFYRECORDEMPHASIS pnre;
                        if (    (pnre = (PNOTIFYRECORDEMPHASIS)mp2)
                             && (pnre->fEmphasisMask & CRA_SELECTED)
                             && (pnre->pRecord)
                             && (pnre->pRecord != pnbp->preccLastSelected)
                           )
                        {
                            fCallItemChanged = TRUE;
                            ulExtra = (ULONG)(pnre->pRecord);
                            pnbp->preccLastSelected = pnre->pRecord;
                        }
                    }
                    break;

                    case CN_CONTEXTMENU:
                        WinQueryPointerPos(HWND_DESKTOP,
                                           &pnbp->ptlMenuMousePos);

                        // add/edit/remove support V1.0.1 (2003-01-05) [umoeller]
                        if (    (pnbp->inbp.ulEditCnrID)
                             && (ulItemID == pnbp->inbp.ulEditCnrID)
                           )
                        {
                            HWND hMenu;
                            pnbp->hwndSourceCnr = pnbp->hwndControl;
                            if (pnbp->preccSource = (PRECORDCORE)mp2)
                            {
                                // popup menu on container recc:
                                if (!(hMenu = pnbp->hmenuSel))
                                {
                                    hMenu
                                    = pnbp->hmenuSel
                                    = cmnLoadMenu(pnbp->hwndDlgPage,
                                                  cmnQueryNLSModuleHandle(FALSE),
                                                  ID_XFM_CNRITEM_SEL);
                                }
                            }
                            else
                            {
                                // popup menu on cnr whitespace:
                                if (!(hMenu = pnbp->hmenuWhitespace))
                                {
                                    hMenu
                                    = pnbp->hmenuWhitespace
                                    = cmnLoadMenu(pnbp->hwndDlgPage,
                                                  cmnQueryNLSModuleHandle(FALSE),
                                                  ID_XFM_CNRITEM_NOSEL);
                                }
                            }

                            if (hMenu)
                                cnrhShowContextMenu(pnbp->hwndControl,  // cnr
                                                    (PRECORDCORE)mp2,
                                                    hMenu,
                                                    pnbp->hwndDlgPage);    // owner
                        }
                        else
                        {
                            fCallItemChanged = TRUE;
                            ulExtra = (ULONG)mp2;
                                // record core for context menu
                                // or NULL for cnr whitespace
                        }
                    break;

                    case CN_ENTER:
                    {
                        PNOTIFYRECORDENTER pnre;
                        if (pnre = (PNOTIFYRECORDENTER)mp2)
                        {
                            // add/edit/remove support V1.0.1 (2003-01-05) [umoeller]
                            if (    (pnbp->inbp.ulEditCnrID)
                                 && (ulItemID == pnbp->inbp.ulEditCnrID)
                               )
                            {
                                pnbp->preccSource = pnre->pRecord;
                                // simulate "edit"
                                WinPostMsg(pnbp->hwndDlgPage,
                                           WM_COMMAND,
                                           (MPARAM)DID_EDIT,
                                           0);
                            }
                            else
                            {
                                fCallItemChanged = TRUE;
                                ulExtra = (ULONG)pnre->pRecord;
                            }
                        }
                    }
                    break;

                    case CN_PICKUP:
                    case CN_INITDRAG:
                    {
                        // get cnr notification struct (mp2)
                        PCNRDRAGINIT pcdi;
                        if (pcdi = (PCNRDRAGINIT)mp2)
                        {
                            fCallItemChanged = TRUE;
                            ulExtra = (ULONG)pcdi;
                        }
                    }
                    break;

                    case CN_DRAGAFTER:
                    case CN_DRAGOVER:
                    case CN_DROP:
                    {
                        // get cnr notification struct (mp2)
                        PCNRDRAGINFO pcdi;
                        if (pcdi = (PCNRDRAGINFO)mp2)
                        {
                            fCallItemChanged = TRUE;
                            ulExtra = (ULONG)pcdi;
                        }
                    }
                    break;

                    case CN_DROPNOTIFY:
                    {
                        // get cnr notification struct (mp2)
                        PCNRLAZYDRAGINFO pcldi;
                        if (pcldi = (PCNRLAZYDRAGINFO)mp2)
                        {
                            fCallItemChanged = TRUE;
                            ulExtra = (ULONG)pcldi;
                        }
                    }
                    break;

                    case CN_RECORDCHECKED:
                    {
                        // extra check-box cnr notification code
                        // (cctl_checkcnr.c)
                        PCHECKBOXRECORDCORE precc;
                        if (precc = (PCHECKBOXRECORDCORE)mp2)
                        {
                            ulExtra = (ULONG)precc;
                                    // changed V0.9.9 (2001-03-27) [umoeller]
                            fCallItemChanged = TRUE;
                        }
                    }
                    break;

                    /*
                     * CN_EXPANDTREE:
                     *      do tree-view auto scroll
                     *      (added V0.9.1)
                     */

                    case CN_EXPANDTREE:
                        mrc = WinDefDlgProc(pnbp->hwndDlgPage, msg, mp1, mp2);
                        if (cmnQuerySetting(sfTreeViewAutoScroll))
                        {
                            // store record for WM_TIMER later
                            pnbp->preccExpanded = (PRECORDCORE)mp2;
                            // and container also
                            pnbp->hwndExpandedCnr = pnbp->hwndControl;
                            WinStartTimer(WinQueryAnchorBlock(pnbp->hwndDlgPage),
                                          pnbp->hwndDlgPage,
                                          999,      // ID
                                          100);
                        }
                    break;

                    case CN_HELP:
                        // does caller want CN_HELP passed?
                        if (pnbp->inbp.fPassCnrHelp)
                            // yes:
                            // call "item changed"
                            fCallItemChanged = TRUE;
                        else
                            // no (default), don't pass:
                            // same handling as with WM_HELP
                            cmnDisplayHelp(pnbp->inbp.somSelf,
                                           pnbp->inbp.ulDefaultHelpPanel);
                                // V0.9.16 (2001-10-23) [umoeller]
                    break;

                    case CN_BEGINEDIT:
                    case CN_REALLOCPSZ:
                    case CN_ENDEDIT:
                        // support for these has been added with
                        // V0.9.9 (2001-02-06) [umoeller]
                        ulExtra = (ULONG)mp2;
                                    // PCNREDITDATA
                        fCallItemChanged = TRUE;
                    break;
                } // end switch (usNotifyCode)
            break;    // container

            // linear slider?
            case 38:
                switch (usNotifyCode)
                {
                    case SLN_CHANGE:
                    case SLN_SLIDERTRACK:
                        fCallItemChanged = TRUE;
                }
            break;

            // circular slider?
            case 65:
                switch (usNotifyCode)
                {
                    case CSN_SETFOCUS:
                                // mp2 is TRUE or FALSE
                    case CSN_CHANGED:
                                // mp2 has new slider value
                    case CSN_TRACKING:
                                // mp2 has new slider value
                        fCallItemChanged = TRUE;
                        ulExtra = (ULONG)mp2;
                }
            break;

            // value set? (added V0.9.9 (2001-03-15) [lafaix])
            case 39:
                switch (usNotifyCode)
                {
                    case VN_ENTER:
                                // mp2 is selected row/col
                    case VN_SELECT:
                                // mp2 is selected row/col
                        fCallItemChanged = TRUE;
                        ulExtra = (ULONG)mp2;
                }
            break;

        } // end switch (ulClassCode)

        if (fCallItemChanged)
            // "important" message found:
            // call "item changed" callback
            mrc = pnbp->inbp.pfncbItemChanged(pnbp,
                                              ulItemID,
                                              usNotifyCode,
                                              ulExtra);
    } // end if (szClassName[0] == '#')

    return mrc;
}

/*
 *@@ PageWmCommand:
 *      implementation for WM_COMMAND in fnwpPageCommon.
 *
 *      hwndDlg is not passed because this can be retrieved
 *      thru pnbp->hwndDlgPage.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

STATIC MRESULT EXPENTRY PageWmCommand(PNOTEBOOKPAGE pnbp,
                                      ULONG msg,
                                      MPARAM mp1,
                                      MPARAM mp2) // in: as in WM_CONTROL
{
    MRESULT mrc = 0;

    PMPF_NOTEBOOKS(("WM_COMMAND"));

    // call "item changed" callback
    if (    (pnbp)
         && (pnbp->inbp.pfncbItemChanged)
       )
    {
        USHORT  cmd = SHORT1FROMMP(mp1);

        // add/edit/remove support V1.0.1 (2003-01-05) [umoeller]
        if (pnbp->inbp.ulEditCnrID)
        {
            // translate menu items
            switch (cmd)
            {
                case ID_XFMI_CNRITEM_NEW:
                    cmd = DID_ADD;
                break;

                case ID_XFMI_CNRITEM_EDIT:
                    cmd = DID_EDIT;
                break;

                case ID_XFMI_CNRITEM_DELETE:
                    cmd = DID_REMOVE;
                break;

                case DID_EDIT:
                case DID_REMOVE:
                    if (    (!(pnbp->preccSource = (PRECORDCORE)WinSendMsg(WinWindowFromID(pnbp->hwndDlgPage,
                                                                                           pnbp->inbp.ulEditCnrID),
                                                                           CM_QUERYRECORDEMPHASIS,
                                                                           (MPARAM)CMA_FIRST,
                                                                           (MPARAM)CRA_SELECTED)))
                         || ((LONG)pnbp->preccSource == -1L)
                       )
                    cmd = 0;
                break;
            }
        }

        if (cmd)
            mrc = pnbp->inbp.pfncbItemChanged(pnbp,
                                              cmd,
                                              0,
                                              (ULONG)mp2);
    }

    return mrc;
}

/*
 *@@ PageWindowPosChanged:
 *      implementation for WM_WINDOWPOSCHANGED in fnwpPageCommon.
 *
 *@@added V0.9.7 (2000-12-10) [umoeller]
 */

STATIC VOID PageWindowPosChanged(PNOTEBOOKPAGE pnbp,
                                 MPARAM mp1)
{
    PSWP pswp = (PSWP)mp1;

    PMPF_NOTEBOOKS(("WM_WINDOWPOSCHANGED"));

    if (!pnbp)
        return;

    if (pnbp->inbp.pfncbInitPage)
    {
        if (pswp->fl & SWP_SHOW)
        {
            // notebook page is being shown:
            // call "initialize" callback
            WinEnableWindowUpdate(pnbp->hwndDlgPage, FALSE);
            pnbp->flPage |= NBFL_PAGE_SHOWING;
            pnbp->inbp.pfncbInitPage(pnbp,
                                     CBI_SHOW | CBI_ENABLE);
                    // we also set the ENABLE flag so
                    // that the callback can re-enable
                    // controls when the page is being
                    // turned to

            WinEnableWindowUpdate(pnbp->hwndDlgPage, TRUE);
        }
        else if (pswp->fl & SWP_HIDE)
        {
            // notebook page is being hidden:
            // call "initialize" callback
            pnbp->flPage &= ~NBFL_PAGE_SHOWING;
            pnbp->inbp.pfncbInitPage(pnbp, CBI_HIDE);
        }
    }

    if (pswp->fl & SWP_SIZE)
    {
        // notebook is being resized:
        // was winhAdjustControls prepared?
        if (pnbp->pxac)
            // yes:
            winhAdjustControls(pnbp->hwndDlgPage,
                               pnbp->inbp.pampControlFlags,
                               pnbp->inbp.cControlFlags,
                               pswp,
                               pnbp->pxac);
    }
}

/*
 *@@ PageTimer:
 *      implementation for WM_TIMER in fnwpPageCommon.
 *
 *@@added V0.9.7 (2000-12-10) [umoeller]
 */

STATIC VOID PageTimer(PNOTEBOOKPAGE pnbp,
                      MPARAM mp1)
{
    PMPF_NOTEBOOKS(("WM_TIMER"));

    switch ((USHORT)mp1)    // timer ID
    {
        case 1:
            // timer for caller: call callback
            if (pnbp)
                if (pnbp->inbp.pfncbTimer)
                    pnbp->inbp.pfncbTimer(pnbp, 1);
        break;

        case 999:
            // tree view auto-scroll timer:
            // CN_EXPANDTREE has set up the data we
            // need in pcnbp
            if (pnbp->preccExpanded->flRecordAttr & CRA_EXPANDED)
            {
                PRECORDCORE     preccLastChild;
                WinStopTimer(WinQueryAnchorBlock(pnbp->hwndDlgPage),
                             pnbp->hwndDlgPage,
                             999);
                // scroll the tree view properly
                preccLastChild = WinSendMsg(pnbp->hwndExpandedCnr,
                                            CM_QUERYRECORD,
                                            pnbp->preccExpanded,
                                               // expanded PRECORDCORE from CN_EXPANDTREE
                                            MPFROM2SHORT(CMA_LASTCHILD,
                                                         CMA_ITEMORDER));
                if (    (preccLastChild)
                     && (preccLastChild != (PRECORDCORE)-1)
                   )
                {
                    // ULONG ulrc;
                    cnrhScrollToRecord(pnbp->hwndExpandedCnr,
                                       (PRECORDCORE)preccLastChild,
                                       CMA_TEXT,   // record text rectangle only
                                       TRUE);      // keep parent visible
                }
            }
        break;
    }
}

/*
 *@@ fnwpPageCommon:
 *      this is the common notebook window procedure which is
 *      always set if you use ntbInsertPage to insert notebook
 *      pages. This function will analyze all incoming messages
 *      and call the corresponding callback functions which you
 *      passed to ntbInsertPage. Also, for quite a number of
 *      messages, a predefined behavior will take place so you
 *      don't have to recode the same stuff for each notebook page.
 *
 *      ntbInsertPage has stored the CREATENOTEBOOKPAGE structure
 *      in QWL_USER of the page.
 *
 *      This function installs exception handling during message
 *      processing, i.e. including your callbacks, using
 *      excHandlerLoud in except.c. This will automatically write
 *      trap logs if a dialog page window proc traps. Of course,
 *      that doesn't save you from protecting your own mutex
 *      semaphores and stuff yourself; see except.c for details.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist functions
 *@@changed V0.9.0 [umoeller]: sped up window class analysis
 *@@changed V0.9.0 [umoeller]: added support for containers, sliders, and MLEs
 *@@changed V0.9.0 [umoeller]: changed entryfield support
 *@@changed V0.9.0 [umoeller]: fixed the age-old button double-click bug
 *@@changed V0.9.0 [umoeller]: added cnr drag'n'drop support
 *@@changed V0.9.0 [umoeller]: added cnr ownerdraw support
 *@@changed V0.9.0 [umoeller]: added CREATENOTEBOOKPAGE.fPageInitialized flag support
 *@@changed V0.9.0 [umoeller]: added 8.Helv controls font support
 *@@changed V0.9.0 [umoeller]: added support for WM_COMMAND return value
 *@@changed V0.9.1 (99-11-29) [umoeller]: added checkbox container support (ctlMakeCheckboxContainer)
 *@@changed V0.9.1 (99-11-29) [umoeller]: added container auto-scroll
 *@@changed V0.9.1 (99-11-29) [umoeller]: reworked message flow
 *@@changed V0.9.1 (99-12-06) [umoeller]: added notebook subclassing
 *@@changed V0.9.1 (99-12-19) [umoeller]: added EN_HOTKEY support (ctlMakeHotkeyEntryField)
 *@@changed V0.9.1 (99-12-31) [umoeller]: extracted PageInit, PageDestroy, PageWmControl
 *@@changed V0.9.3 (2000-05-01) [umoeller]: added WM_MOUSEMOVE pointer changing
 *@@changed V0.9.7 (2000-12-10) [umoeller]: fixed mutex problems
 *@@changed V0.9.7 (2000-12-10) [umoeller]: extracted PageWindowPosChanged, PageTimer
 *@@changed V1.0.1 (2003-01-05) [umoeller]: added automatic cnr context menu support for add/edit/remove
 */

STATIC MRESULT EXPENTRY fnwpPageCommon(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT             mrc = NULL;
    BOOL                fProcessed = FALSE;

    // protect ALL the processing with the
    // loud exception handler; this includes
    // all message processing, including the
    // callbacks defined by the implementor
    TRY_LOUD(excpt1)
    {
        PNOTEBOOKPAGE pnbp = NULL;

        /*
         * WM_INITDLG:
         *
         */

        if (msg == WM_INITDLG)
        {
            pnbp = (PNOTEBOOKPAGE)mp2;
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            PageInit(pnbp, hwndDlg);
            fProcessed = TRUE;
        }
        else
        {
            MRESULT     mrc2 = 0;

            // get the notebook creation struct, which was passed
            // to ntbInsertPage, from the window words
            if (    (pnbp = (PNOTEBOOKPAGE)WinQueryWindowPtr(hwndDlg, QWL_USER))
                 // run message callback defined by caller, if any
                 && (pnbp->inbp.pfncbMessage)
                 && (pnbp->inbp.pfncbMessage(pnbp, msg, mp1, mp2, &mrc2))
               )
            {
                // TRUE returned == msg processed:
                // return the return value
                mrc = mrc2;
                fProcessed = TRUE;
            }
        }

        if (    (pnbp)
             && (!fProcessed)
           )
        {
            fProcessed = TRUE;

            switch(msg)
            {
                /*
                 * WM_CONTROL:
                 *
                 */

                case WM_CONTROL:
                    mrc = PageWmControl(pnbp, msg, mp1, mp2);
                break;

                /*
                 * WM_DRAWITEM:
                 *      container owner draw
                 */

                case WM_DRAWITEM:
                {
                    HWND    hwndControl;
                    CHAR    szClassName[5];

                    if (    (hwndControl = WinWindowFromID(hwndDlg,
                                                           (USHORT)mp1)) // has the control ID
                         && (WinQueryClassName(hwndControl,
                                               sizeof(szClassName),
                                               szClassName))
                       )
                    {
                        if (!memcmp(szClassName, "#37", 4))
                            // container:
                            mrc = cnrhOwnerDrawRecord(mp2,
                                                      // V0.9.16 (2001-09-29) [umoeller]
                                                      pnbp->inbp.ulCnrOwnerDraw);
                    }
                    // else: return default FALSE
                }
                break;  // WM_DRAWITEM

                /*
                 * WM_MENUEND:
                 *      this is received when a menu is just
                 *      about to be destroyed. This might come
                 *      in if the notebook page has some context
                 *      menu defined by the caller.
                 *
                 *      For the purpose of container context menus,
                 *      per definition, if the caller sets
                 *      preccSource to some record core or NULL, we
                 *      will remove source emphasis from that recc
                 *      (or, if NULL, from the whole container)
                 *      in this situation.
                 *
                 *      Normally, hwndCnr is 0 and preccSource is -1.
                 */

                case WM_MENUEND:
                {
                    PMPF_NOTEBOOKS(("WM_MENUEND"));

                    if (    (pnbp->preccSource != (PRECORDCORE)-1)
                         && (pnbp->hwndSourceCnr)
                       )
                    {
                        WinSendMsg(pnbp->hwndSourceCnr,
                                   CM_SETRECORDEMPHASIS,
                                   (MPARAM)(pnbp->preccSource),
                                   MPFROM2SHORT(FALSE, CRA_SOURCE));
                        // reset hwndCnr to make sure we won't
                        // do this again
                        pnbp->hwndSourceCnr = 0;
                        // but leave preccSource as it is, because
                        // WM_MENUEND is posted before WM_COMMAND,
                        // and the caller might still need this
                    }

                    mrc = (MRESULT)0;
                }
                break;  // WM_MENUEND

                /*
                 * WM_COMMAND:
                 *      for buttons, we also use the callback
                 *      for "item changed"; the difference
                 *      between WM_CONTROL and WM_COMMAND has
                 *      never made sense to me
                 */

                case WM_COMMAND:
                    mrc = PageWmCommand(pnbp, msg, mp1, mp2);
                break;  // WM_COMMAND

                /*
                 * WM_HELP:
                 *      results from the "Help" button or
                 *      from pressing F1; we display help
                 *      depending on the control which has
                 *      the focus and depending on the data
                 *      which has been passed to us
                 */

                case WM_HELP:
                    cmnDisplayHelp(pnbp->inbp.somSelf,
                                   pnbp->inbp.ulDefaultHelpPanel);
                            // V0.9.16 (2001-10-23) [umoeller]
                break; // WM_HELP

                /*
                 * WM_WINDOWPOSCHANGED:
                 *      cheap trick: this msg is posted when
                 *      the user switches to a different notebook
                 *      page. Since every notebook page is really
                 *      a separate dialog window, PM simulates
                 *      switching notebook pages by showing or
                 *      hiding the various dialog windows.
                 *      We will call the INIT callback with a
                 *      show/hide flag then.
                 */

                case WM_WINDOWPOSCHANGED:
                    PageWindowPosChanged(pnbp, mp1);
                    // call default
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                break;  // WM_WINDOWPOSCHANGED

                /*
                 * WM_MOUSEMOVE:
                 *      set mouse pointer to "wait" if the
                 *      corresponding flag in pcnbp is on.
                 */

                case WM_MOUSEMOVE:
                    if (    (pnbp)
                         && (pnbp->fShowWaitPointer)
                       )
                    {
                        WinSetPointer(HWND_DESKTOP,
                                      winhQueryWaitPointer());
                        break;
                    }

                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                break;

                /*
                 * WM_CONTROLPOINTER:
                 *      set mouse pointer to "wait" if the
                 *      corresponding flag in pcnbp is on.
                 */

                case WM_CONTROLPOINTER:
                    if (    (pnbp)
                         && (pnbp->fShowWaitPointer)
                       )
                    {
                        mrc = (MRESULT)winhQueryWaitPointer();
                        break;
                    }

                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                break;

                /*
                 * WM_TIMER:
                 *      call timer callback, if defined.
                 */

                case WM_TIMER:
                    PageTimer(pnbp, mp1);
                break;

                /*
                 *@@ XNTBM_UPDATE:
                 *      message posted by ntbUpdateVisiblePage
                 *      to update the page by calling the INIT
                 *      callback.
                 *
                 *      Parameters:
                 *
                 *      ULONG mp1: CBI_* flags to pass to INIT callback.
                 *
                 *@@added V0.9.3 (2000-04-24) [umoeller]
                 */

                case XNTBM_UPDATE:
                    if (    (pnbp)
                         && (pnbp->inbp.pfncbInitPage)
                       )
                        pnbp->inbp.pfncbInitPage(pnbp, (ULONG)mp1);
                break;

                /*
                 * WM_DESTROY:
                 *      clean up the allocated structures.
                 */

                case WM_DESTROY:
                    PageDestroy(pnbp);
                    fProcessed = FALSE;
                break;

                default:
                    fProcessed = FALSE;

            } // end switch (msg)
        } // end if (!fProcessed)
    }
    CATCH(excpt1) {} END_CATCH();

    if (!fProcessed)
        mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);

    return mrc;
}

/*
 *@@ CreateNBLI:
 *      creates a new SUBCLNOTEBOOKLISTITEM for hwndNotebook.
 *
 *      This has been extracted from ntbInsertPage because it
 *      wasn't such a good idea to put the entire function in
 *      a mutex block.
 *
 *@@added V0.9.7 (2000-12-09) [umoeller]
 *@@changed V0.9.19 (2002-06-02) [umoeller]: optimized
 */

STATIC PNOTEBOOKPAGELISTITEM CreateNBLI(PNOTEBOOKPAGE pnbp) // in: new struct from ntbInsertPage
{
    BOOL        fLocked = FALSE;

    // create NOTEBOOKPAGELISTITEM to be stored in list
    PNOTEBOOKPAGELISTITEM pnbliNew = NULL;

    TRY_LOUD(excpt1)
    {
        HWND        hwndDesktop = NULLHANDLE,
                    hwndCurrent = pnbp->inbp.hwndNotebook;
        PLISTNODE   pNode;
        BOOL        fNotebookAlreadySubclassed = FALSE;

        // get frame to which this window belongs
        hwndDesktop = WinQueryDesktopWindow(WinQueryAnchorBlock(pnbp->inbp.hwndNotebook),
                                            NULLHANDLE);

        // find frame window handle of "Workplace Shell" window
        while ( (hwndCurrent) && (hwndCurrent != hwndDesktop))
        {
            pnbp->hwndFrame = hwndCurrent;
            hwndCurrent = WinQueryWindow(hwndCurrent, QW_PARENT);
        }

        if (!hwndCurrent)
            pnbp->hwndFrame = NULLHANDLE;

        // store new page in linked list
        if (fLocked = LockNotebooks())
        {
            pnbliNew = malloc(sizeof(NOTEBOOKPAGELISTITEM));

            pnbliNew->pnbp = pnbp;

            // store new list item in structure, so we can easily
            // find it upon WM_DESTROY
            pnbp->pnbli = (PVOID)pnbliNew;

            lstAppendItem(&G_llOpenPages,
                          pnbliNew);

            PMPF_NOTEBOOKS(("appended NOTEBOOKPAGELISTITEM to pages list"));

            // now search the list of notebook list items
            // for whether a page has already been inserted
            // into this notebook; if not, subclass the
            // notebook (otherwise it has already been subclassed
            // by this func)
            pNode = lstQueryFirstNode(&G_llSubclNotebooks);
            while (pNode)
            {
                PSUBCLNOTEBOOKLISTITEM psnbliThis;
                if (    (psnbliThis = (PSUBCLNOTEBOOKLISTITEM)pNode->pItemData)
                     && (psnbliThis->hwndNotebook == pnbp->inbp.hwndNotebook)
                   )
                {
                    fNotebookAlreadySubclassed = TRUE;
                    break;
                }

                pNode = pNode->pNext;
            }

            if (!fNotebookAlreadySubclassed)
            {
                // notebook not yet subclassed:
                // do it now
                PSUBCLNOTEBOOKLISTITEM pSubclNBLINew;
                if (pSubclNBLINew = (PSUBCLNOTEBOOKLISTITEM)malloc(sizeof(SUBCLNOTEBOOKLISTITEM)))
                {
                    pSubclNBLINew->hwndNotebook = pnbp->inbp.hwndNotebook;
                    lstAppendItem(&G_llSubclNotebooks,
                                  pSubclNBLINew);
                    pSubclNBLINew->pfnwpNotebookOrig
                        = WinSubclassWindow(pnbp->inbp.hwndNotebook,
                                            fnwpSubclNotebook);
                }
            }
        } // end if (fLocked)
        else
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "hmtxNotebookLists mutex request failed");
    }
    CATCH(excpt1) {} END_CATCH();

    if (fLocked)
        UnlockNotebooks();

    return pnbliNew;
}

/*
 *@@ FindNBLI:
 *      finds the SUBCLNOTEBOOKLISTITEM for hwndNotebook.
 *
 *      This has been extracted from fnwpSubclNotebook
 *      because it wasn't such a good idea to put the entire
 *      window proc in a mutex block.
 *
 *@@added V0.9.7 (2000-12-09) [umoeller]
 */

STATIC PSUBCLNOTEBOOKLISTITEM FindNBLI(HWND hwndNotebook)
{
    PSUBCLNOTEBOOKLISTITEM pSubclNBLI = NULL;
    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = LockNotebooks())
        {
            PLISTNODE   pNode = lstQueryFirstNode(&G_llSubclNotebooks);
            while (pNode)
            {
                PSUBCLNOTEBOOKLISTITEM psnbliThis;
                if (    (psnbliThis = (PSUBCLNOTEBOOKLISTITEM)pNode->pItemData)
                     && (psnbliThis->hwndNotebook == hwndNotebook)
                   )
                {
                    pSubclNBLI = psnbliThis;
                    break;
                }

                pNode = pNode->pNext;
            }
        } // end if (fLocked)
    }
    CATCH(excpt1) {} END_CATCH();

    if (fLocked)
        UnlockNotebooks();

    return pSubclNBLI;
}

/*
 *@@ DestroyNBLI:
 *      implementation for WM_DESTROY in fnwpSubclNotebook.
 *
 *      This has been extracted from fnwpSubclNotebook
 *      because it wasn't such a terrific idea to put
 *      the entire window proc in a mutex block.
 *
 *@@added V0.9.7 (2000-12-09) [umoeller]
 *@@changed V0.9.14 (2001-08-23) [umoeller]: fixed bad pointer on list node remove
 */

STATIC VOID DestroyNBLI(HWND hwndNotebook,
                        PSUBCLNOTEBOOKLISTITEM pSubclNBLI)
{
    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = LockNotebooks())
        {
            PLISTNODE pPageNode = lstQueryFirstNode(&G_llOpenPages);


            PMPF_NOTEBOOKS(("WM_DESTROY"));

            while (pPageNode)
            {
                PLISTNODE pNext = pPageNode->pNext; // V0.9.14 (2001-08-23) [umoeller]
                PNOTEBOOKPAGELISTITEM pPageLI;
                PNOTEBOOKPAGE pnbp;
                if (    (pPageLI = (PNOTEBOOKPAGELISTITEM)pPageNode->pItemData)
                     && (pnbp = pPageLI->pnbp)
                     && (pnbp->inbp.hwndNotebook == hwndNotebook) // our page?
                     && (!(pnbp->flPage  & NBFL_PAGE_INITED))
                            // page has NOT been initialized
                            // (this flag is set by fnwpPageCommon):
                   )
                {
                    // remove it from list
                    PMPF_NOTEBOOKS(("  removed page ID %d", pnbp->inbp.ulPageID));

                    _wpFreeMem(pnbp->inbp.somSelf,
                               (PBYTE)pnbp);
                    lstRemoveNode(&G_llOpenPages,
                                  pPageNode);
                }

                pPageNode = pNext;
            }

            // remove notebook control from list
            lstRemoveItem(&G_llSubclNotebooks,
                          pSubclNBLI);      // this frees the pSubclNBLI

            PMPF_NOTEBOOKS(("  removed pSubclNBLI"));

        } // end if (fLocked)
    }
    CATCH(excpt1) {} END_CATCH();

    if (fLocked)
        UnlockNotebooks();
}

/*
 *@@ fnwpSubclNotebook:
 *      window procedure for notebook controls subclassed
 *      by ntbInsertPage.
 *
 *      The notebook control itself is automatically subclassed
 *      by ntbInsertPage if it hasn't been subclassed yet. Subclassing
 *      is necessary because, with V0.9.1, I finally realized
 *      that each call to ntbInsertPage allocates memory for
 *      the CREATENOTEBOOKPAGE items, but this is only
 *      released upon WM_DESTROY with pages that have actually
 *      been switched to. Pages which were inserted but never
 *      switched to will never get WM_DESTROY, so for those pages
 *      this subclassed notebook proc does the cleanup.
 *
 *      Er, these memory leaks must have been in XFolder for
 *      ages. Quite embarassing.
 *
 *@@added V0.9.1 (99-12-06) [umoeller]
 *@@changed V0.9.7 (2000-12-10) [umoeller]: fixed mutex problems
 */

STATIC MRESULT EXPENTRY fnwpSubclNotebook(HWND hwndNotebook, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    PSUBCLNOTEBOOKLISTITEM pSubclNBLI;
    if (pSubclNBLI = FindNBLI(hwndNotebook))
    {
        switch (msg)
        {
            /*
             * WM_DESTROY:
             *      notebook is being destroyed.
             *      Enumerate notebook pages inserted
             *      by ntbInsertPage and destroy those
             *      which haven't been initialized yet,
             *      because those won't get WM_DESTROY
             *      in fnwpPageCommon.
             */

            case WM_DESTROY:
            {
                PFNWP pfnwpNotebookOrig = pSubclNBLI->pfnwpNotebookOrig;
                DestroyNBLI(hwndNotebook, pSubclNBLI);
                            // after this, pSubclNBLI is invalid!!
                mrc = pfnwpNotebookOrig(hwndNotebook, msg, mp1, mp2);
            }
            break;

            default:
                mrc = pSubclNBLI->pfnwpNotebookOrig(hwndNotebook, msg, mp1, mp2);
        }
    } // end if (pSubclNBLI)
    else
        mrc = WinDefWindowProc(hwndNotebook, msg, mp1, mp2);

    return mrc;
}

/*
 *@@ ntbInsertPage:
 *      this function inserts the specified notebook page
 *      using the wpInsertSettingsPage function. However,
 *      this always uses fnwpPageCommon for the notebook's
 *      window procedure, which then calls the callbacks which
 *      you may specify in the INSERTNOTEBOOKPAGE structure.
 *
 *      This function returns the return code of wpInsertSettingsPages.
 *
 *      A linked list of currently open pages is maintained by this
 *      function, which can be accessed from ntbQueryOpenPages and
 *      ntbUpdateVisiblePage.
 *
 *      All the notebook functions are thread-safe.
 *
 *      Note that the usage of this func changed with V0.9.18.
 *      The caller is no longer required to allocate memory for
 *      this function; instead, he fills an INSERTNOTEBOOKPAGE
 *      struct on the stack of which we make a heap copy here.
 *
 *      <B>Example usage</B> from some WPS "add notebook page" method:
 *
 +          INSERTNOTEBOOKPAGE inbp;
 +          // always zero all fields, because we don't use all of them
 +          memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
 +          inbp.somSelf = somSelf;
 +          inbp.hwndNotebook = hwndNotebook;  // from WPS method header
 +          inbp.hmod = cmnQueryNLSModuleHandle(FALSE); // resource module handle
 +          inbp.ulDlgID = ...;       // dialog ID in inbp.hmod
 +          inbp.fMajorTab = TRUE;
 +          inbp.pcszName = "~Test page";
 +          inbp.ulDefaultHelpPanel  = ...;
 +          inbp.ulPageID = ...;      // unique ID
 +          inbp.pfncbInitPage    = fncbYourPageInitPage;     // init callback
 +          inbp.pfncbItemChanged = fncbYourPageItemChanged;  // item-changed callback
 +          ntbInsertPage(&inbp);
 *
 *      The <B>ulPageID</B> is not required, but strongly recommended
 *      to tell the different notebook pages apart (esp. when using
 *      ntbUpdateVisiblePage). This can be used to easily update all
 *      open settings pages with ntbUpdateVisiblePage. For example,
 *      if a global setting changes which affects instance notebook
 *      pages as well, all of these can be easily updated. Use any
 *      ULONG you like, this has nothing to do with the dialog resources.
 *      XWorkplace uses the SP_* IDs from common.h.
 *
 *      <B>The "init" callback</B>
 *
 *      The "init" callback receives the following parameters:
 *
 *      --  PNOTEBOOKPAGE pnbp:     notebook info struct, which contains
 *                                  the INSERTNOTEBOOKPAGE which was passed
 *                                  to this function
 *
 *      --  ULONG flFlags:           CBI_* flags (notebook.h), which determine
 *                                   the context of the call.
 *
 *      fnwpPageCommon will call this init callback itself
 *      in the following situations:
 *
 *      -- When the page is initialized (WM_INITDLG), flFlags is
 *         CBI_INIT | CBI_SET | CBI_ENABLE.
 *
 *      -- When the page is later turned away from, flFlags is
 *         CBI_HIDE.
 *
 *      -- When the page is later turned to, flFlags is
 *         CBI_SHOW | CBI_ENABLE.
 *
 *      -- When the page is destroyed (when the notebook is closed),
 *         flFlags is CBI_DESTROY.
 *
 *      CBI_INIT is guaranteed to come in only _once_ in the lifetime
 *      of the notebook page.
 *
 *      It is recommended to have up to six blocks in your "init" callback
 *      (but you probably won't need all of them):
 *
 +      VOID fncbWhateverInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
 +                                ULONG flFlags)        // CBI_* flags (notebook.h)
 +      {
 +          if (flFlags & CBI_INIT) // initialize page; this gets called exactly once
 +          {
 +              pnbp->pUser = malloc(...);  // allocate memory for "Undo";
 +                                          // this will be free()'d automatically
 +                                          // on destroy
 +              memcpy((PVOID)pnbp->pUser, ...)  // backup data for "Undo"
 +
 +                   ...        // initialize controls (set styles, subclass controls, etc.)
 +          }
 +
 +          if (flFlags & CBI_SET)
 +                   ... // set controls' data; this gets called only once
 +                       // from fnwpPageCommon, but you can call this yourself
 +                       // several times
 +          if (flFlags & CBI_ENABLE)
 +                   ... // enable/disable controls; this can get called several times
 +          if (flFlags & CBI_HIDE)
 +                   ..  // page is turned away from: hide additional frame windows, if any
 +          if (flFlags & CBI_SHOW)
 +                   ..  // page is turned back to a second time: show windows again
 +          if (flFlags & CBI_DESTROY)
 +                   ... // clean up on exit: only once
 +      }
 *
 *      The "flFlags" approach allows you to call the init callback
 *      from your own code also if your page should need updating.
 *      For example, if a checkbox is unchecked, you can call the init
 *      callback with CBI_ENABLE only from the "item changed" callback (below)
 *      to disable other controls if needed.
 *
 *      This is especially useful with the ntbUpdateVisiblePage function,
 *      which iterates over all open pages.
 *
 *      <B>Using the dialog formatter</B>
 *
 *      You can use the xwphelpers dialog formatter (src\helpers\dialog.c)
 *      instead of fixed dialog resources with these functions too.
 *      For this, do the following:
 *
 *      1)  For INSERTNOTEBOOKPAGE.hwndDlg, use ID_XFD_EMPTYDLG.
 *          That is an empty dialog frame. Unfortunately this is
 *          necessary because wpInsertSettingsPages always need
 *          a dialog resource.
 *
 *      2)  When CBI_INIT comes into your "init" callback, call
 *          ntbFormatPage. See remarks there for details.
 *
 *      <B>The "item changed" callback</B>
 *
 *      This gets called from fnwpPageCommon when either
 *      WM_CONTROL or WM_COMMAND comes in. Note that fnwpPageCommon
 *      _filters_ these messages and calls the "item changed" callback
 *      only for notifications which I have considered useful so far.
 *      If this is not sufficient for you, you must use the pfncbMessage
 *      callback (which gets really all the messages).
 *
 *      This callback is not required, but your page won't react to
 *      anything if you don't install this (which might be OK if your
 *      page is "read-only").
 *
 *      The "item changed" callback receives the following parameters:
 *
 *      --  PNOTEBOOKPAGE pnbp:     notebook info struct (as with init
 *                                  callback)
 *
 *      --  ULONG ulItemId:         ID of the changing item
 *
 *      --  USHORT usNotifyCode:    as in WM_CONTROL; NULL for WM_COMMAND
 *
 *      --  ULONG  ulExtra:         additional control data.
 *
 *      <B>ulExtra</B> has the following:
 *
 *      -- For checkboxes (BN_CLICKED), this is the new selection state
 *         of the control (0 or 1, or, for tri-state checkboxes, possibly 2).
 *
 *      -- For radio buttons (BN_CLICKED), only the radio button that
 *         got selected gets a callback with ulExtra == 1. The other radio
 *         buttons in the group (which got unchecked) do not produce
 *         messages.
 *
 *      -- For pushbuttons (WM_COMMAND message), this contains
 *         the mp2 of WM_COMMAND (usSourceType, usPointer).
 *
 *      -- For spinbuttons, this contains the new LONG value if
 *         a value has changed. This works for numerical spinbuttons only.
 *
 *      -- Container CN_EMPHASIS: has the new selected record
 *                    (only if CRA_SELECTED changed).
 *
 *      -- Container CN_CONTEXTMENU: has the record core (mp2);
 *                    rclMenuMousePos has the mouse position.
 *
 *      -- Container CN_INITDRAG or
 *                   CN_PICKUP:   has the PCNRDRAGINIT (mp2).
 *
 *      -- Container CN_DRAGOVER,
 *                   CN_DROP:     has the PCNRDRAGINFO (mp2).
 *
 *      -- Container CN_DROPNOTIFY: has the PCNRLAZYDRAGINFO (mp2).
 *
 *      -- Container CN_BEGINEDIT, CN_REALLOCPSZ, CN_ENDEDIT: has the
 *              PCNREDITDATA (mp2) (added with V0.9.9 (2001-02-06) [umoeller]).
 *
 *      -- Check-box container CN_RECORDCHECKED (see cctl_checkcnr.c):
 *              has the CHECKBOXRECORDCORE pointer that was clicked.
 *
 *      -- For circular and linear sliders (CSN_CHANGED or CSN_TRACKING),
 *                this has the new slider value.
 *
 *      -- For value sets, this contains the coordinates of the
 *         selected value (mp2).
 *
 *      -- For all other controls/messages, this is always -1.
 *
 *      Whatever the "item changed" callback returns will be the
 *      return value of fnwpPageCommon. Normally, you should
 *      return 0, except for the container d'n'd messages.
 *
 *      A couple of remarks about using this on radio buttons...
 *      If radio buttons just won't work in your dialog:
 *
 *      --  Make sure that the first radio button has the WS_GROUP style,
 *          and the first item _after_ the radio buttons has the WS_GROUP
 *          style as well.
 *
 *      --  The group of radio buttons should not appear at the top of the
 *          dialog. PM just keeps sending the wrong BN_CLICKED things if
 *          radio buttons are on top. I think this is a PM bug.
 *
 *      <B>Implementing an "Undo" button</B>
 *
 *      NOTEBOOKPAGE has the pUser and pUser2 fields, which are PVOIDs
 *      to user data. Upon CBI_INIT in the "init" callback, you
 *      can allocate memory (using malloc()) and copy undo data into
 *      that buffer. In the "item changed" callback, check for the ID
 *      of your "Undo" button, copy that backed-up data back and call
 *      the "init" callback with CBI_SET | CBI_ENABLE to have the page
 *      updated with the backed-up data.
 *
 *      If pUser or pUser2 are != NULL, free() will automatically be
 *      invoked on them by fnwpPageCommon when the page is destroyed.
 *      This is done _after_ the INIT callback has been called which
 *      CBI_DESTROY, so if you store something in there which should
 *      not be free()'d, set those pointers to NULL upon CBI_DESTROY.
 *
 *      <B>Using timers</B>
 *
 *      Using timers can be helpful if your page should update itself
 *      periodically. This is extremely easy: just set
 *      INSERTNOTEBOOKPAGE.ulTimer to the frequency (in ms) and
 *      install a timer callback in INSERTNOTEBOOKPAGE.pfncbTimer,
 *      which then gets called with that frequency. The timer is
 *      automatically started and stopped when the page is destroyed.
 *
 *      In your "timer" callback, you can then call the "init" callback
 *      with CBI_SET to have your controls updated.
 *
 *      The "timer" callback gets these parameters:
 *      --  PNOTEBOOKPAGE pcnbp:    notebook info struct
 *      --  ULONG ulTimer:          timer id (always 1)
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist functions
 *@@changed V0.9.1 (99-12-06) [umoeller]: added notebook subclassing
 *@@changed V0.9.1 (2000-02-14) [umoeller]: reversed order of functions; now subclassing is last
 *@@changed V0.9.7 (2000-12-10) [umoeller]: fixed mutex problems
 *@@changed V0.9.9 (2001-03-27) [umoeller]: changed usItemID to ULONG in "item changed" callback
 *@@changed V0.9.16 (2001-10-23) [umoeller]: finally found out how to set minor tab titles
 *@@changed V0.9.18 (2002-02-23) [umoeller]: finally adding proper memory management; adjusted prototype for new notebook structs
 */

ULONG ntbInsertPage(PINSERTNOTEBOOKPAGE pinbp)
{
    PNOTEBOOKPAGE   pnbpNew;
    ULONG           ulrc = 0;
    ULONG           dummy;

    // added memory management V0.9.18 (2002-02-23) [umoeller]
    if (pnbpNew = (PNOTEBOOKPAGE)_wpAllocMem(pinbp->somSelf,
                                             sizeof(NOTEBOOKPAGE),
                                             &dummy))
    {
        PAGEINFO        pi;

        ZERO(pnbpNew);

        // copy input data V0.9.18 (2002-02-23) [umoeller]
        memcpy(&pnbpNew->inbp,
               pinbp,
               sizeof(INSERTNOTEBOOKPAGE));

        memset(&pi, 0, sizeof(PAGEINFO));

        pi.cb                  = sizeof(PAGEINFO);
        pi.pfnwp               = fnwpPageCommon;
        pi.resid               = pinbp->hmod;
        pi.dlgid               = pinbp->ulDlgID;
        // pass new NOTEBOOKPAGE as create param V0.9.18 (2002-02-23) [umoeller]
        pi.pCreateParams       = pnbpNew;
        pi.usPageStyleFlags    = BKA_STATUSTEXTON | pinbp->usPageStyleFlags;
        pi.usPageInsertFlags   = BKA_FIRST;
        pi.usSettingsFlags     = (pinbp->fEnumerate) ? SETTINGS_PAGE_NUMBERS : 0;
                                        // enumerate in status line
        pi.pszName             = (PSZ)pinbp->pcszName;

        pi.pszHelpLibraryName  = (PSZ)cmnQueryHelpLibrary();
        pi.idDefaultHelpPanel  = pinbp->ulDefaultHelpPanel;

        // insert page
        if (ulrc = _wpInsertSettingsPage(pinbp->somSelf,
                                         pinbp->hwndNotebook,
                                         &pi))
                // this returns the notebook page ID
        {
            // successfully inserted:

            // if this is a minor tab, hack the "page info"
            // to display the minor tab title in the notebook
            // context menu V0.9.16 (2001-10-23) [umoeller]
            if (pinbp->pcszMinorName)
            {
                BOOKPAGEINFO bpi;
                memset(&bpi, 0, sizeof(&bpi));
                bpi.cb = sizeof(BOOKPAGEINFO);
                bpi.fl = BFA_MINORTABTEXT;
                bpi.pszMinorTab = (PSZ)pinbp->pcszMinorName;
                bpi.cbMinorTab = strlen(pinbp->pcszMinorName);
                WinSendMsg(pinbp->hwndNotebook,
                           BKM_SETPAGEINFO,
                           (MPARAM)ulrc,
                           (MPARAM)&bpi);
            }

            // store PM notebook page ID
            pnbpNew->ulNotebookPageID = ulrc;

            // create SUBCLNOTEBOOKLISTITEM
            CreateNBLI(pnbpNew);
        }
    }

    return ulrc;
}

/*
 *@@ ntbFormatPage:
 *      wrapper around dlghFormatDialog for dynamically
 *      formatting notebook pages. This allows XWorkplace
 *      to use the dialog formatter (src\helpers\dialog.c)
 *      with notebook pages too.
 *
 *      Usage:
 *
 *      1)  With ntbInsertPage, specify ID_XFD_EMPTYDLG for
 *          the dialog ID to be loaded (which is an empty
 *          dialog frame). This is required, unfortunately,
 *          because the WPS expects us to use PM resources.
 *
 *      2)  In your "init page" callback, when CBI_INIT
 *          comes in, call this function on the array of
 *          DLGHITEM which represents the controls to
 *          be added. Example:
 *
 +          static VOID blahInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
 +                                   ULONG flFlags)        // CBI_* flags (notebook.h)
 +          {
 +              if (flFlags & CBI_INIT)
 +              {
 +                  ntbFormatPage(pnbp->hwndDlgPage,
 +                                dlgBlah,
 +                                ARRAYITEMCOUNT(dlgBlah);
 +              }
 +          }
 *
 *      3)  For dynamic string loading, set each dialog
 *          item's pcszText to LOADSTRING, which will
 *          cause this function to load the string
 *          resources for each dialog item from the NLS
 *          DLL. It is assumed that the NLS string has
 *          the same ID as the control.
 *
 *@@added V0.9.16 (2001-09-29) [umoeller]
 *@@changed V0.9.19 (2002-04-02) [umoeller]: adjusted for new cmnLoadDialogStrings to avoid NLS blowups
 */

APIRET ntbFormatPage(HWND hwndDlg,              // in: dialog frame to work on
                     PCDLGHITEM paDlgItems,     // in: definition array
                     ULONG cDlgItems)           // in: array item count (NOT array size)
{
    APIRET arc;
    SIZEL szlClient;

    // go create the controls
    if (!(arc = dlghFormatDlg(hwndDlg,
                              paDlgItems,
                              cDlgItems,
                              cmnQueryDefaultFont(),
                              DFFL_CREATECONTROLS,
                              &szlClient,
                              NULL)))
    {
        // resize the frame
        dlghResizeFrame(hwndDlg,
                        &szlClient);

        // make Warp 4 notebook buttons and move controls
        // (this was already called in PageInit on WM_INITDLG,
        // but at that point the init callback wasn't called yet...)
        winhAssertWarp4Notebook(hwndDlg,
                                100);         // ID threshold
                                // 14);
    }
    else
    {
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "dlghFormatDlg returned %d", arc);
        cmnErrorMsgBox(hwndDlg,
                       arc,
                       0,
                       MB_OK,
                       TRUE);
    }

    return arc;
}

/*
 *@@ ntbQueryOpenPages:
 *      this function returns the NOTEBOOKPAGE
 *      structures for currently open notebook pages, which
 *      are maintained by ntbInsertPage and fnwpPageCommon.
 *      This way you can iterate over all open pages and call
 *      the callbacks of certain pages to have pages updated,
 *      if necessary.
 *
 *      If (pcnpb == NULL), the first open page is returned;
 *      otherwise, the page which follows after pcnbp in
 *      our internal list.
 *
 *      In order to identify pages properly, you should always
 *      set unique ulPageID identifiers when inserting notebook
 *      pages and evaluate somSelf, if these are instance pages.
 *
 *      Warning: This function returns all pages which have
 *      been inserted using ntbInsertPage. This does not
 *      necessarily mean that the INIT callback has been
 *      invoked on the page yet. You should check inbp.fPageVisible
 *      for the current visibility of the page returned by
 *      this function.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist functions
 *@@changed V0.9.0 [umoeller]: fixed an endless loop problem
 *@@changed V0.9.7 (2000-12-10) [umoeller]: fixed mutex problems
 *@@changed V0.9.18 (2002-02-23) [umoeller]: adjusted prototype for new notebook structs
 */

PNOTEBOOKPAGE ntbQueryOpenPages(PNOTEBOOKPAGE pnbp)
{
    PLISTNODE               pNode = 0;
    PNOTEBOOKPAGELISTITEM   pItemReturn = 0;
    BOOL                    fLocked = FALSE;

    TRY_QUIET(excpt1)
    {
        if (fLocked = LockNotebooks())
        {
            pNode = lstQueryFirstNode(&G_llOpenPages);

            if (pnbp == NULL)
            {
                // pcnbp == NULL: return first item
                if (pNode)
                    pItemReturn = (PNOTEBOOKPAGELISTITEM)pNode->pItemData;
            }
            else
            {
                // pcnbp given: search for that page
                while (pNode)
                {
                    PNOTEBOOKPAGELISTITEM pItem = (PNOTEBOOKPAGELISTITEM)pNode->pItemData;
                    if (pItem->pnbp == pnbp)
                    {
                        // page found: return next
                        if (pNode = pNode->pNext)
                            pItemReturn = (PNOTEBOOKPAGELISTITEM)pNode->pItemData;
                        break;
                    }

                    pNode = pNode->pNext;
                }
            }
        } // end if (fLocked)
        else
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "hmtxNotebookLists mutex request failed");
    }
    CATCH(excpt1) { } END_CATCH();

    if (fLocked)
        UnlockNotebooks();

    if (pItemReturn)
        return pItemReturn->pnbp;

    return NULL;
}

/*
 *@@ ntbUpdateVisiblePage:
 *      this will go thru all currently open notebook
 *      pages (which are maintained by fnwpPageCommon
 *      in a linked list) and update a page (by calling its
 *      "init" callback with CBI_SET | CBI_ENABLE) if it
 *      matches the specified criteria.
 *
 *      These criteria are:
 *      --  somSelf         must match CREATENOTEBOOKPAGE.somSelf
 *      --  ulPageID        must match CREATENOTEBOOKPAGE.ulPageID
 *
 *      If any of these criteria is NULL, it's considered
 *      a "don't care", i.e. it is not checked for.
 *      A page is only updated if it's currently visible,
 *      i.e. turned to in an open settings notebook.
 *
 *      <B>Example:</B> If a certain XWorkplace feature is disabled
 *      in "XWorkplace Setup", we will need to disable controls
 *      in all open folder instance settings notebooks which are
 *      related to that feature. So we can call ntbUpdateVisiblePage
 *      with somSelf == NULL (all objects) and ulPageID == the
 *      folder page ID which has these controls, and there we go.
 *
 *      Returns the number of pages that were updated.
 *
 *@@changed V0.9.3 (2000-04-24) [umoeller]: finally made this thread-safe; now sending XNTBM_UPDATE
 *@@changed V0.9.19 (2002-04-24) [umoeller]: this only refreshed currently visible pages, fixed
 */

ULONG ntbUpdateVisiblePage(WPObject *somSelf,
                           ULONG ulPageID)
{
    ULONG ulrc = 0;
    PNOTEBOOKPAGE pnbp = NULL;

    while (pnbp = ntbQueryOpenPages(pnbp))
    {
        if (pnbp->flPage  & NBFL_PAGE_INITED)       // fixed V0.9.19 (2002-04-24) [umoeller]
        {
            if (    (    (ulPageID == 0)     // don't care?
                      || (pnbp->inbp.ulPageID == ulPageID)
                    )
                 && (    (somSelf == NULL)   // don't care?
                      || (pnbp->inbp.somSelf == somSelf)
                    )
               )
            {
                WinSendMsg(pnbp->hwndDlgPage,
                           XNTBM_UPDATE,
                           (MPARAM)(CBI_SET | CBI_ENABLE),
                           0);
                ulrc++;
            }
        }
    } // while (pcnbp = ntbQueryOpenPages(pcnbp))

    return ulrc;
}

/*
 *@@ ntbTurnToPage:
 *      switches to the page specified by the given
 *      XWorkplace SP_* page ID.
 *
 *@@added V0.9.20 (2002-08-04) [umoeller]
 */

BOOL ntbTurnToPage(HWND hwndNotebook,       // in: notebook control (FID_CLIENT of settings view frame)
                   ULONG ulPageID)          // in: XWorkplace SP_* page ID
{
    PNOTEBOOKPAGE pnbp = NULL;

    while (pnbp = ntbQueryOpenPages(pnbp))
    {
        if (    (hwndNotebook == pnbp->inbp.hwndNotebook)
             && (pnbp->inbp.ulPageID == ulPageID)
           )
        {
            return (BOOL)WinSendMsg(hwndNotebook,
                                    BKM_TURNTOPAGE,
                                    (MPARAM)pnbp->ulNotebookPageID,
                                    0);
        }
    }

    return FALSE;
}

/*
 *@@ ntbOpenSettingsPage:
 *      little helper func to open the settings notebook
 *      of an object specified by object ID and to then
 *      switch to the page specified by the given SP_*
 *      page ID.
 *
 *      Naturally, this will only work for pages inserted
 *      by XWorkplace.
 *
 *@@added V0.9.20 (2002-08-04) [umoeller]
 */

BOOL ntbOpenSettingsPage(PCSZ pcszObjectID,         // in: object ID
                         ULONG ulPageID)            // in: SP_* page ID
{
    WPObject *pobj;
    HWND    hwndFrame,
            hwndNotebook;

    if (pobj = cmnQueryObjectFromID(pcszObjectID))
    {
        PVIEWITEM pvi;

        // Wow. I thought this would be a quick'n'easy hack to
        // implement, but here's another bunch of entries for
        // the IBM weirdo programming hall of fame.

        // What I wanted is to use wpViewObject to get the frame
        // of the settings notebook, either a new one or the
        // existing view, if any. PMTREE tells me that the notebook
        // control is the FID_CLIENT of the notebook frame.

        // What I found out is this however:

        // 1) We can't simply use wpViewObject to give us back the handle
        //    of an open view if it called wpSwitchTo, because
        //    the dumbass then returns NULLHANDLE.

        // 2) If wpViewObject _does_ create a new OPEN_SETTINGS,
        //    it does not return the frame, but the notebook
        //    control. WHAT?

        // Hence the code below.

        if (    (    (pvi = _wpFindViewItem(pobj, VIEW_SETTINGS, NULL))
                  && (hwndFrame = pvi->handle)
                  && (hwndNotebook = WinWindowFromID(hwndFrame, FID_CLIENT))
                )
             || (hwndNotebook = _wpViewObject(pobj,
                                              NULLHANDLE,
                                              OPEN_SETTINGS,
                                              0))
           )
        {
            _PmpfF(("got hwndNotebook 0x%lX", hwndNotebook));
            return ntbTurnToPage(hwndNotebook,
                                 ulPageID);
        }
    }

    return FALSE;
}

/*
 * ntbDisplayFocusHelp:
 *      this is used from all kinds of settings dlg procs to
 *      display help panels.
 *
 *      This will either display a sub-panel for the dialog
 *      control which currently has the keyboard focus or a
 *      "global" help page for the whole dialog.
 *
 *      This works as follows:
 *
 *      1)  First, the dialog item which currently has the
 *          keyboard focus is queried.
 *
 *      2)  We then attempt to open a help panel which
 *          corresponds to that ID, depending on the
 *          usFirstControlID and ulFirstSubpanel parameters:
 *          the ID of the focused control is queried,
 *          usFirstControlID is subtracted, and the result
 *          is added to ulFirstSubpanel (to get the help panel
 *          which is to be opened).
 *
 *          For example, if the control with the ID
 *          (usFirstControlID + 1) currently has the focus,
 *          (usFirstSubpanel + 1) will be displayed.
 *
 *          You should therefore assign the control IDs on
 *          the page in ascending order and order your help
 *          panels accordingly.
 *
 *      3)  If that fails (or if ulFirstSubpanel == 0),
 *          ulPanelIfNotFound will be displayed instead, which
 *          should be the help panel for the whole dialog.
 *
 *      This returns FALSE if step 3) failed also.
 *
 *changed V0.9.0 [umoeller]: functionality altered completely
 *
 *      This code was no longer really used and thus removed.
 *      V0.9.16 (2001-10-23) [umoeller]
 */

/* BOOL ntbDisplayFocusHelp(WPObject *somSelf,         // in: input for wpDisplayHelp
                         USHORT usFirstControlID,   // in: first dialog item ID
                         ULONG ulFirstSubpanel,     // in: help panel ID which corresponds to usFirstControlID
                         ULONG ulPanelIfNotFound)   // in: subsidiary help panel ID
{
    BOOL brc = TRUE;
    PCSZ pszHelpLibrary = cmnQueryHelpLibrary();

    HWND hwndFocus = WinQueryFocus(HWND_DESKTOP);
    if ((hwndFocus) && (somSelf))
    {

        USHORT  idItem = WinQueryWindowUShort(hwndFocus, QWS_ID);
        BOOL    fOpenPagePanel = FALSE;

        // attempt to display subpanel

        if (ulFirstSubpanel != 0)
        {
            if (!_wpDisplayHelp(somSelf,
                                (idItem-usFirstControlID) + ulFirstSubpanel,
                                (PSZ)pszHelpLibrary))
                fOpenPagePanel = TRUE;
        }
        else
            fOpenPagePanel = TRUE;

        if (fOpenPagePanel)
        {
            PMPF_NOTEBOOKS(("not found, displaying superpanel %d", ulPanelIfNotFound ));

            // didn't work: display page panel
            if (!_wpDisplayHelp(somSelf,
                                ulPanelIfNotFound,
                                (PSZ)pszHelpLibrary))
            {
                // still errors: complain
                cmnHelpNotFound(ulPanelIfNotFound);
                brc = FALSE;
            }
        }
    }
    return brc;
} */


