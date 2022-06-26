
/*
 *@@sourcefile fdrnotebooks.c:
 *      implementation for folder-related notebook pages,
 *      both in XFolder instances and in the "Workplace Shell"
 *      object, as far as they're folder-specific.
 *
 *      This file is ALL new with V0.9.3. The code in here used
 *      to be in folder.c.
 *
 *      Function prefix for this file:
 *      --  fdr*
 *
 *@@added V0.9.3 [umoeller]
 *@@header "filesys\folder.h"
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

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINFRAMEMGR
#define INCL_WININPUT
#define INCL_WINRECTANGLES
#define INCL_WINSYS             // needed for presparams
#define INCL_WINMENUS
#define INCL_WINTIMER
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINLISTBOXES
#define INCL_WINSTDCNR
#define INCL_WINSTDSLIDER
#define INCL_WINSTDSPIN
#define INCL_WINSHELLDATA       // Prf* functions
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\standards.h"          // some standard macros
#include "helpers\threads.h"            // thread helpers
#include "helpers\winh.h"               // PM helper routines

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"
#include "xfstart.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\notebook.h"            // generic XWorkplace notebook handling

#include "filesys\folder.h"             // XFolder implementation
#include "filesys\statbars.h"           // status bar translation logic
#include "filesys\xthreads.h"           // extra XWorkplace threads

// other SOM headers
#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Notebook callbacks (notebook.c) for XFldWPS  "View" page
 *
 ********************************************************************/

static const CONTROLDEF
    FolderViewGroup = LOADDEF_GROUP(ID_XSD_FOLDERVIEWGROUP, DEFAULT_TABLE_WIDTH),
    FullPathCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_FULLPATH),
    KeepTitleCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_KEEPTITLE),
    MaxPathCharsText1 = LOADDEF_TEXT(ID_XSDI_MAXPATHCHARS_TX1),
    MaxPathCharsSpin = CONTROLDEF_SPINBUTTON(
                            ID_XSDI_MAXPATHCHARS,
                            30,
                            STD_SPIN_HEIGHT),
    MaxPathCharsText2 = LOADDEF_TEXT(ID_XSDI_MAXPATHCHARS_TX2),
    TreeViewAutoScrollCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_TREEVIEWAUTOSCROLL),
#ifndef __NOFDRDEFAULTDOCS__
    FdrDefaultDocCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_FDRDEFAULTDOC),
    FdrDefaultDocViewCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_FDRDEFAULTDOCVIEW),
#endif
    FdrAutoRefreshCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_FDRAUTOREFRESH),
    FdrLazyIconsCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_FDRVIEW_LAZYICONS), // V0.9.20 (2002-07-31) [umoeller]
    FdrThumbnailsCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_FDRVIEW_THUMBNAILS), // V0.9.20 (2002-07-31) [umoeller]
    FdrShadowOverlayCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_FDRVIEW_SHADOWOVERLAY),   // V0.9.20 (2002-08-04) [umoeller]
    FdrDefaultViewGroup = LOADDEF_GROUP(ID_XSDI_FDRVIEWDEFAULT_GROUP, DEFAULT_TABLE_WIDTH),
    FdrViewInheritCB = LOADDEF_FIRST_AUTORADIO(ID_XSDI_FDRVIEW_INHERIT),
    FdrViewIconCB = LOADDEF_NEXT_AUTORADIO(ID_XSDI_FDRVIEW_ICON),
    FdrViewTreeCB = LOADDEF_NEXT_AUTORADIO(ID_XSDI_FDRVIEW_TREE),
    FdrViewDetailsCB = LOADDEF_NEXT_AUTORADIO(ID_XSDI_FDRVIEW_DETAILS);

static const DLGHITEM dlgView[] =
    {
        START_TABLE,            // root table, required
            // ordering changed V0.9.19 (2002-04-17) [umoeller]
            START_ROW(0),
                START_GROUP_TABLE(&FdrDefaultViewGroup),
                    START_ROW(0),
                        CONTROL_DEF(&FdrViewInheritCB),
                    START_ROW(0),
                        CONTROL_DEF(&FdrViewIconCB),
                    START_ROW(0),
                        CONTROL_DEF(&FdrViewTreeCB),
                    START_ROW(0),
                        CONTROL_DEF(&FdrViewDetailsCB),
                END_TABLE,
            START_ROW(0),
                // create group on top
                START_GROUP_TABLE(&FolderViewGroup),
                    START_ROW(0),
                        CONTROL_DEF(&FullPathCB),
                    START_ROW(0),
                        CONTROL_DEF(&G_Spacing),
                        CONTROL_DEF(&KeepTitleCB),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&G_Spacing),
                        CONTROL_DEF(&MaxPathCharsText1),
                        CONTROL_DEF(&MaxPathCharsSpin),
                        CONTROL_DEF(&MaxPathCharsText2),
                    START_ROW(0),
                        CONTROL_DEF(&FdrLazyIconsCB),       // V0.9.20 (2002-07-31) [umoeller]
                    START_ROW(0),
                        CONTROL_DEF(&FdrThumbnailsCB),      // V1.0.1 (2003-01-31) [umoeller]
                    START_ROW(0),
                        CONTROL_DEF(&FdrShadowOverlayCB),   // V0.9.20 (2002-07-31) [umoeller]
                    START_ROW(0),
                        CONTROL_DEF(&TreeViewAutoScrollCB),
#ifndef __NOFDRDEFAULTDOCS__
                    START_ROW(0),
                        CONTROL_DEF(&FdrDefaultDocCB),
                    START_ROW(0),
                        CONTROL_DEF(&G_Spacing),
                        CONTROL_DEF(&FdrDefaultDocViewCB),
#endif
                    START_ROW(0),
                        CONTROL_DEF(&FdrAutoRefreshCB),
                END_TABLE,
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };

static const XWPSETTING G_ViewBackup[] =
    {
        sfFullPath,
        sfKeepTitle,
        sulMaxPathChars,
        sfTreeViewAutoScroll,
#ifndef __NOFDRDEFAULTDOCS__
        sfFdrDefaultDoc,
        sfFdrDefaultDocView,
#endif
        sfFdrAutoRefreshDisabled,
        sulDefaultFolderView,
        sflOwnerDrawIcons	// V1.0.5 (2006-06-09) [pr]
    };

/*
 *@@ fdrViewInitPage:
 *      notebook callback function (notebook.c) for the
 *      "Folder views" page in the "Workplace Shell" object.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@changed V0.9.0 [umoeller]: adjusted function prototype, renamed function
 *@@changed V0.9.0 [umoeller]: moved this func here from xfwps.c
 *@@changed V0.9.4 (2000-06-09) [umoeller]: added default documents
 *@@changed V0.9.9 (2001-02-06) [umoeller]: added folder auto-refresh
 *@@changed V0.9.12 (2001-04-30) [umoeller]: added default folder views
 *@@changed V0.9.16 (2001-10-11) [umoeller]: now using dialog formatter
 *@@changed V0.9.20 (2002-08-04) [umoeller]: added lazyload settings
 *@@changed V1.0.1 (2003-01-31) [umoeller]: added thumbnail setting
 */

VOID fdrViewInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                     ULONG flFlags)        // CBI_* flags (notebook.h)
{
    PCKERNELGLOBALS pKernelGlobals = krnQueryGlobals();

    if (flFlags & CBI_INIT)
    {
        // first call: backup Global Settings for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = cmnBackupSettings(G_ViewBackup,
                                         ARRAYITEMCOUNT(G_ViewBackup));

        // insert the controls using the dialog formatter
        // V0.9.16 (2001-10-11) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      dlgView,
                      ARRAYITEMCOUNT(dlgView));
    }

    if (flFlags & CBI_SET)
    {
        ULONG   ulid,
                flOwnerDraw;

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_FULLPATH,
                              cmnQuerySetting(sfFullPath));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_KEEPTITLE,
                              cmnQuerySetting(sfKeepTitle));
        // maximum path chars spin button
        winhSetDlgItemSpinData(pnbp->hwndDlgPage, ID_XSDI_MAXPATHCHARS,
                               11, 200,        // limits
                               cmnQuerySetting(sulMaxPathChars));  // data
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_TREEVIEWAUTOSCROLL,
                              cmnQuerySetting(sfTreeViewAutoScroll));

#ifndef __NOFDRDEFAULTDOCS__
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_FDRDEFAULTDOC,
                              cmnQuerySetting(sfFdrDefaultDoc));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_FDRDEFAULTDOCVIEW,
                              cmnQuerySetting(sfFdrDefaultDocView));
#endif

        if (pKernelGlobals->fAutoRefreshReplaced)
            winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_FDRAUTOREFRESH,
                                  !cmnQuerySetting(sfFdrAutoRefreshDisabled));

        // V0.9.20 (2002-08-04) [umoeller]
        flOwnerDraw = cmnQuerySetting(sflOwnerDrawIcons);
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_FDRVIEW_LAZYICONS,
                              !!(flOwnerDraw & OWDRFL_LAZYLOADICON));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_FDRVIEW_SHADOWOVERLAY,
                              !!(flOwnerDraw & OWDRFL_SHADOWOVERLAY));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_FDRVIEW_THUMBNAILS,
                              !!(flOwnerDraw & OWDRFL_LAZYLOADTHUMBNAIL));

        // folder default views V0.9.12 (2001-04-30) [umoeller]
        switch (cmnQuerySetting(sulDefaultFolderView))
        {
            case OPEN_CONTENTS: ulid = ID_XSDI_FDRVIEW_ICON; break;
            case OPEN_TREE:     ulid = ID_XSDI_FDRVIEW_TREE; break;
            case OPEN_DETAILS:  ulid = ID_XSDI_FDRVIEW_DETAILS; break;

            default: /* 0 */ ulid = ID_XSDI_FDRVIEW_INHERIT; break;
        }

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ulid, TRUE);
    }

    if (flFlags & CBI_ENABLE)
    {
#ifndef __NOFDRDEFAULTDOCS__
        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_FDRDEFAULTDOCVIEW,
                         cmnQuerySetting(sfFdrDefaultDoc));
#endif

        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_FDRAUTOREFRESH,
                         pKernelGlobals->fAutoRefreshReplaced);
    }
}

/*
 *@@ fdrViewItemChanged:
 *      notebook callback function (notebook.c) for the
 *      "Folder views" page in the "Workplace Shell" object.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 *@@changed V0.9.0 [umoeller]: moved this func here from xfwps.c
 *@@changed V0.9.4 (2000-06-09) [umoeller]: added default documents
 *@@changed V0.9.9 (2001-02-06) [umoeller]: added folder auto-refresh
 *@@changed V0.9.12 (2001-04-30) [umoeller]: added default folder views
 *@@changed V0.9.20 (2002-08-04) [umoeller]: added lazyload settings
 *@@changed V1.0.1 (2003-01-31) [umoeller]: added thumbnail setting
 *@@changed V1.0.5 (2006-06-09) [pr]: fixed Undo
 */

MRESULT fdrViewItemChanged(PNOTEBOOKPAGE pnbp,
                           ULONG ulItemID, USHORT usNotifyCode,
                           ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = (MRESULT)0;
    BOOL    fUpdate = FALSE, fFolderUpdate = FALSE;
    ULONG   flOwnerDrawChanged = 0;

    switch (ulItemID)
    {
        case ID_XSDI_FULLPATH:
            cmnSetSetting(sfFullPath, ulExtra);
            fUpdate = TRUE;
        break;

        case ID_XSDI_KEEPTITLE:
            cmnSetSetting(sfKeepTitle, ulExtra);
            fUpdate = TRUE;
        break;

        case ID_XSDI_TREEVIEWAUTOSCROLL:
            cmnSetSetting(sfTreeViewAutoScroll, ulExtra);
        break;

        case ID_XSDI_MAXPATHCHARS:  // spinbutton
            cmnSetSetting(sulMaxPathChars, ulExtra);
            fUpdate = TRUE;
        break;

#ifndef __NOFDRDEFAULTDOCS__
        case ID_XSDI_FDRDEFAULTDOC:
            cmnSetSetting(sfFdrDefaultDoc, ulExtra);
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;

        case ID_XSDI_FDRDEFAULTDOCVIEW:
            cmnSetSetting(sfFdrDefaultDocView, ulExtra);
        break;
#endif

        case ID_XSDI_FDRAUTOREFRESH:
            cmnSetSetting(sfFdrAutoRefreshDisabled, (ulExtra == 0));
        break;

        // V0.9.20 (2002-08-04) [umoeller]
        case ID_XSDI_FDRVIEW_LAZYICONS:
            flOwnerDrawChanged = OWDRFL_LAZYLOADICON;
        break;

        // V1.0.1 (2003-01-31) [umoeller]
        case ID_XSDI_FDRVIEW_THUMBNAILS:
            flOwnerDrawChanged = OWDRFL_LAZYLOADTHUMBNAIL;
        break;

        // V0.9.20 (2002-08-04) [umoeller]
        case ID_XSDI_FDRVIEW_SHADOWOVERLAY:
            flOwnerDrawChanged = OWDRFL_SHADOWOVERLAY;
        break;

        case ID_XSDI_FDRVIEW_ICON:
            cmnSetSetting(sulDefaultFolderView, OPEN_CONTENTS);
        break;

        case ID_XSDI_FDRVIEW_TREE:
            cmnSetSetting(sulDefaultFolderView, OPEN_TREE);
        break;

        case ID_XSDI_FDRVIEW_DETAILS:
            cmnSetSetting(sulDefaultFolderView, OPEN_DETAILS);
        break;

        case ID_XSDI_FDRVIEW_INHERIT:
            cmnSetSetting(sulDefaultFolderView, 0);
        break;


        case DID_UNDO:
            // restore the settings for this page
            cmnRestoreSettings(pnbp->pUser,
                               ARRAYITEMCOUNT(G_ViewBackup));

            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            fFolderUpdate = TRUE;
        break;

        case DID_DEFAULT:
            // set the default settings for this settings page
            // (this is in common.c because it's also used at
            // Desktop startup)
            cmnSetDefaultSettings(pnbp->inbp.ulPageID);
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            fFolderUpdate = TRUE;
        break;
    }

    if (fUpdate)
        // have Worker thread update all open folder windows
        // with the full-path-in-title settings
        xthrPostWorkerMsg(WOM_REFRESHFOLDERVIEWS,
                          (MPARAM)NULL, // update all, not just children
                          (MPARAM)FDRUPDATE_TITLE);

    if (flOwnerDrawChanged)
    {
        ULONG flOwnerDraw = cmnQuerySetting(sflOwnerDrawIcons);
        if (ulExtra)
            cmnSetSetting(sflOwnerDrawIcons, flOwnerDraw | flOwnerDrawChanged);
        else
            cmnSetSetting(sflOwnerDrawIcons, flOwnerDraw & ~flOwnerDrawChanged);

        fFolderUpdate = TRUE;
    }

    if (fFolderUpdate)
        xthrPostWorkerMsg(WOM_REFRESHFOLDERVIEWS,
                          (MPARAM)NULL, // update all, not just children
                          (MPARAM)FDRUPDATE_REPAINT);

    return mrc;
}

/* ******************************************************************
 *
 *   Notebook callbacks (notebook.c) for XFldWPS"Grid" page
 *
 ********************************************************************/

#ifndef __NOSNAPTOGRID__

static const XWPSETTING G_GridBackup[] =
    {
        sfAddSnapToGridDefault,
        sulGridX,
        sulGridY,
        sulGridCX,
        sulGridCY
    };

/*
 *@@ fdrGridInitPage:
 *      notebook callback function (notebook.c) for the
 *      "Snap to grid" page in the "Workplace Shell" object.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 */

VOID fdrGridInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                     ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        // first call: backup Global Settings for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = cmnBackupSettings(G_GridBackup,
                                         ARRAYITEMCOUNT(G_GridBackup));
    }

    if (flFlags & CBI_SET)
    {
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_SNAPTOGRID,
                              cmnQuerySetting(sfAddSnapToGridDefault));
        winhSetDlgItemSpinData(pnbp->hwndDlgPage, ID_XSDI_GRID_X,
                               0, 500,
                               cmnQuerySetting(sulGridX));
        winhSetDlgItemSpinData(pnbp->hwndDlgPage, ID_XSDI_GRID_Y,
                               0, 500,
                               cmnQuerySetting(sulGridY));
        winhSetDlgItemSpinData(pnbp->hwndDlgPage, ID_XSDI_GRID_CX,
                               1, 500,
                               cmnQuerySetting(sulGridCX));
        winhSetDlgItemSpinData(pnbp->hwndDlgPage, ID_XSDI_GRID_CY,
                               1, 500,
                               cmnQuerySetting(sulGridCY));
    }
}

/*
 *@@ fdrGridItemChanged:
 *      notebook callback function (notebook.c) for the
 *      "Snap to grid" page in the "Workplace Shell" object.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 */

MRESULT fdrGridItemChanged(PNOTEBOOKPAGE pnbp,
                           ULONG ulItemID,
                           USHORT usNotifyCode,
                           ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = (MRESULT)0;
    BOOL fSave = TRUE;

    switch (ulItemID)
    {
        case ID_XSDI_SNAPTOGRID:
            cmnSetSetting(sfAddSnapToGridDefault, ulExtra);
        break;

        case ID_XSDI_GRID_X:
            cmnSetSetting(sulGridX, ulExtra);
        break;

        case ID_XSDI_GRID_Y:
            cmnSetSetting(sulGridY, ulExtra);
        break;

        case ID_XSDI_GRID_CX:
            cmnSetSetting(sulGridCX, ulExtra);
        break;

        case ID_XSDI_GRID_CY:
            cmnSetSetting(sulGridCY, ulExtra);
        break;

        case DID_UNDO:
            // "Undo" button: get pointer to backed-up Global Settings
            // and restore the settings for this page
            cmnRestoreSettings(pnbp->pUser,
                               ARRAYITEMCOUNT(G_GridBackup));
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
            fSave = FALSE;
    }

    return mrc;
}

#endif

/* ******************************************************************
 *
 *   Notebook callbacks (notebook.c) for "XFolder" instance page
 *
 ********************************************************************/

static const CONTROLDEF
#ifndef __NOFOLDERCONTENTS__
    FavoriteFolderCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_FAVORITEFOLDER),
#endif
#ifndef __NOQUICKOPEN__
    QuickOpenCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_QUICKOPEN),
#endif
    /* FullPathCB = CONTROLDEF_AUTOCHECKBOX(
                            LOAD_STRING,
                            ID_XSDI_FULLPATH,
                            -1,
                            -1),
    KeepTitleCB = CONTROLDEF_AUTOCHECKBOX(
                            LOAD_STRING,
                            ID_XSDI_KEEPTITLE,
                            -1,
                            -1), */
            // already defined in "View" page above
#ifndef __NOSNAPTOGRID__
    SnapToGridCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_SNAPTOGRID),
#endif
    FdrHotkeysCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_ACCELERATORS),
    StatusBarCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_ENABLESTATUSBAR);


static const DLGHITEM dlgXFolder[] =
    {
        START_TABLE,            // root table, required
#ifndef __NOFOLDERCONTENTS__
            START_ROW(0),       // row 1 in the root table, required
                CONTROL_DEF(&FavoriteFolderCB),
#endif
#ifndef __NOQUICKOPEN__
            START_ROW(0),
                CONTROL_DEF(&QuickOpenCB),
#endif
            START_ROW(0),
                CONTROL_DEF(&FullPathCB),
            START_ROW(0),
                CONTROL_DEF(&G_Spacing),
                CONTROL_DEF(&KeepTitleCB),
#ifndef __NOSNAPTOGRID__
            START_ROW(0),
                CONTROL_DEF(&SnapToGridCB),
#endif
            START_ROW(0),
                CONTROL_DEF(&FdrHotkeysCB),
            START_ROW(0),
                CONTROL_DEF(&StatusBarCB),
            START_ROW(0),
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };

/*
 * fdrXFolderInitPage:
 *      "XFolder" page notebook callback function (notebook.c).
 *      Sets the controls on the page according to a folder's
 *      instance settings.
 *
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.1 (99-12-28) [umoeller]: "snap to grid" was enabled even if disabled globally; fixed
 *@@changed V0.9.4 (2000-08-02) [umoeller]: added "keep title" instance setting
 *@@changed V0.9.16 (2001-09-29) [umoeller]: now using dialog formatter
 */

VOID fdrXFolderInitPage(PNOTEBOOKPAGE pnbp,  // notebook info struct
                        ULONG flFlags)              // CBI_* flags (notebook.h)
{
    XFolderData *somThis = XFolderGetData(pnbp->inbp.somSelf);

    if (flFlags & CBI_INIT)
    {
        // first call: backup instance data for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        if (pnbp->pUser = malloc(sizeof(XFolderData)))
            memcpy(pnbp->pUser, somThis, sizeof(XFolderData));

        // insert the controls using the dialog formatter
        // V0.9.16 (2001-09-29) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      dlgXFolder,
                      ARRAYITEMCOUNT(dlgXFolder));
    }

    if (flFlags & CBI_SET)
    {
#ifndef __NOFOLDERCONTENTS__
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_FAVORITEFOLDER,
                              _xwpIsFavoriteFolder(pnbp->inbp.somSelf));
#endif

#ifndef __NOQUICKOPEN__
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_QUICKOPEN,
                              _xwpQueryQuickOpen(pnbp->inbp.somSelf));
#endif

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_FULLPATH,
                              (     ((_bFullPathInstance == 2)
                                       ? cmnQuerySetting(sfFullPath)
                                       : _bFullPathInstance )
                                 != 0));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_KEEPTITLE,
                              (     ((_bKeepTitleInstance == 2)
                                       ? cmnQuerySetting(sfKeepTitle)
                                       : _bKeepTitleInstance )
                                 != 0));
#ifndef __NOSNAPTOGRID__
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_SNAPTOGRID,
                              (     ((_bSnapToGridInstance == 2)
                                       ? cmnQuerySetting(sfAddSnapToGridDefault)
                                       : _bSnapToGridInstance )
                                 != 0));
#endif
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_ACCELERATORS,
                              (     ((_bFolderHotkeysInstance == 2)
                                       ? cmnQuerySetting(sfFolderHotkeysDefault)
                                       : _bFolderHotkeysInstance )
                                 != 0));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_ENABLESTATUSBAR,
                              (   (     ((_bStatusBarInstance == STATUSBAR_DEFAULT)
                                           ? cmnQuerySetting(sfDefaultStatusBarVisibility)
                                           : _bStatusBarInstance )
                                     != 0)
                                  // always uncheck for Desktop
                                && (stbClassCanHaveStatusBars(pnbp->inbp.somSelf))
                                        // V0.9.19 (2002-04-17) [umoeller]
                              ));
    }

    if (flFlags & CBI_ENABLE)
    {
        // disable items
        BOOL fEnable = (
                              1
#ifndef __ALWAYSFDRHOTKEYS__
                              && (cmnQuerySetting(sfFolderHotkeys))
#endif
                         );

        WinEnableControl(pnbp->hwndDlgPage,
                         ID_XSDI_ACCELERATORS,
                         fEnable);

        WinEnableControl(pnbp->hwndDlgPage,
                         ID_XSDI_KEEPTITLE,
                         ( (_bFullPathInstance == 2)
                             ? cmnQuerySetting(sfFullPath)
                             : _bFullPathInstance ));

#ifndef __NOSNAPTOGRID__
        WinEnableControl(pnbp->hwndDlgPage,
                         ID_XSDI_SNAPTOGRID,  // added V0.9.1 (99-12-28) [umoeller]
                         cmnQuerySetting(sfSnap2Grid));
#endif
                         // always disable for Desktop
        fEnable =        (   (stbClassCanHaveStatusBars(pnbp->inbp.somSelf))
                                        // V0.9.19 (2002-04-17) [umoeller]
#ifndef __NOCFGSTATUSBARS__
                          && (cmnQuerySetting(sfStatusBars))
#endif
                         );
        WinEnableControl(pnbp->hwndDlgPage,
                         ID_XSDI_ENABLESTATUSBAR,
                         fEnable);
    }
}

/*
 * fdrXFolderItemChanged:
 *      "XFolder" page notebook callback function (notebook.c).
 *      Reacts to changes of any of the dialog controls.
 *
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.4 (2000-08-02) [umoeller]: added "keep title" instance setting
 */

MRESULT fdrXFolderItemChanged(PNOTEBOOKPAGE pnbp,
                              ULONG ulItemID,
                              USHORT usNotifyCode,
                              ULONG ulExtra)      // for checkboxes: contains new state
{
    XFolderData *somThis = XFolderGetData(pnbp->inbp.somSelf);
    BOOL fUpdate = TRUE;

    switch (ulItemID)
    {
#ifndef __NOSNAPTOGRID__
        case ID_XSDI_SNAPTOGRID:
            _bSnapToGridInstance = ulExtra;
        break;
#endif

        case ID_XSDI_FULLPATH:
            _bFullPathInstance = ulExtra;
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            fdrUpdateAllFrameWindows(pnbp->inbp.somSelf, FDRUPDATE_TITLE);
        break;

        case ID_XSDI_KEEPTITLE:
            _bKeepTitleInstance = ulExtra;
            fdrUpdateAllFrameWindows(pnbp->inbp.somSelf, FDRUPDATE_TITLE);
        break;

        case ID_XSDI_ACCELERATORS:
            _bFolderHotkeysInstance = ulExtra;
        break;

        case ID_XSDI_ENABLESTATUSBAR:
            _xwpSetStatusBarVisibility(pnbp->inbp.somSelf,
                        ulExtra,
                        TRUE);  // update open folder views
        break;

#ifndef __NOFOLDERCONTENTS__
        case ID_XSDI_FAVORITEFOLDER:
            _xwpMakeFavoriteFolder(pnbp->inbp.somSelf, ulExtra);
        break;
#endif

#ifndef __NOQUICKOPEN__
        case ID_XSDI_QUICKOPEN:
            _xwpSetQuickOpen(pnbp->inbp.somSelf, ulExtra);
        break;
#endif

        case DID_UNDO:
            if (pnbp->pUser)
            {
                XFolderData *Backup = (pnbp->pUser);
                // "Undo" button: restore backed up instance data
                _bFullPathInstance = Backup->bFullPathInstance;
                _bKeepTitleInstance = Backup->bKeepTitleInstance;
                _bSnapToGridInstance = Backup->bSnapToGridInstance;
                _bFolderHotkeysInstance = Backup->bFolderHotkeysInstance;
                _xwpSetStatusBarVisibility(pnbp->inbp.somSelf,
                                           Backup->bStatusBarInstance,
                                           TRUE);  // update open folder views
                // update the display by calling the INIT callback
                pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
                fdrUpdateAllFrameWindows(pnbp->inbp.somSelf, FDRUPDATE_TITLE);
            }
        break;

        case DID_DEFAULT:
            // "Default" button:
            _bFullPathInstance = 2;
            _bKeepTitleInstance = 2;
            _bSnapToGridInstance = 2;
            _bFolderHotkeysInstance = 2;
            _xwpSetStatusBarVisibility(pnbp->inbp.somSelf,
                        STATUSBAR_DEFAULT,
                        TRUE);  // update open folder views
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            fdrUpdateAllFrameWindows(pnbp->inbp.somSelf, FDRUPDATE_TITLE);
        break;

        default:
            fUpdate = FALSE;
        break;
    }

    if (fUpdate)
        _wpSaveDeferred(pnbp->inbp.somSelf);

    return (MPARAM)0;
}

#ifndef __NOXWPSTARTUP__

/* ******************************************************************
 *
 *   XFldStartup notebook callbacks (notebook.c)
 *
 ********************************************************************/

static const XWPSETTING G_StartupFolderBackup[] =
    {
        sulStartupInitialDelay,
        sfShowStartupProgress
    };

/*
 * fdrStartupFolderInitPage:
 *      notebook callback function (notebook.c) for the
 *      "Startup" page in the startup folder's settings notebook.
 *      (This used to be page 2 in the Desktop's settings notebook
 *      before V0.9.0.)
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@changed V0.9.0 [umoeller]: moved this here from xfdesk.c
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 *@@changed V0.9.4 (2000-08-02) [umoeller]: now using sliders; added initial delay
 *@@changed V0.9.9 (2001-03-19) [pr]: multiple startup folder mods.
 */

VOID fdrStartupFolderInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                              ULONG flFlags)        // CBI_* flags (notebook.h)
{
    XFldStartupData *somThis = NULL;

    somThis = XFldStartupGetData(pnbp->inbp.somSelf);

    if (flFlags & CBI_INIT)
    {
        // first call: backup Global Settings and instance
        // variables for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = cmnBackupSettings(G_StartupFolderBackup,
                                         ARRAYITEMCOUNT(G_StartupFolderBackup));
        if (pnbp->pUser2 = malloc(sizeof(XFldStartupData)))
            memcpy(pnbp->pUser2, somThis, sizeof(XFldStartupData));

        // set up sliders
        winhSetSliderTicks(WinWindowFromID(pnbp->hwndDlgPage, ID_SDDI_STARTUP_INITDELAY_SLID),
                           (MPARAM)0, 3,
                           MPFROM2SHORT(9, 10), 6);
        winhSetSliderTicks(WinWindowFromID(pnbp->hwndDlgPage, ID_SDDI_STARTUP_OBJDELAY_SLID),
                           (MPARAM)0, 3,
                           MPFROM2SHORT(9, 10), 6);
    }

    if (flFlags & CBI_SET)
    {
        // the range of valid startup delays is
        // 500 ms to 10,000 ms in steps of 500 ms;
        // that gives us 20 - 1 = 19 positions

        // initial delay
        winhSetSliderArmPosition(WinWindowFromID(pnbp->hwndDlgPage, ID_SDDI_STARTUP_INITDELAY_SLID),
                                 SMA_INCREMENTVALUE,
                                 (cmnQuerySetting(sulStartupInitialDelay) / 500) - 1);
        // per-object delay
        winhSetSliderArmPosition(WinWindowFromID(pnbp->hwndDlgPage, ID_SDDI_STARTUP_OBJDELAY_SLID),
                                 SMA_INCREMENTVALUE,
                                 (_ulObjectDelay / 500) - 1);

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SDDI_SHOWSTARTUPPROGRESS,
                              cmnQuerySetting(sfShowStartupProgress));
        if (_ulType == XSTARTUP_REBOOTSONLY)
            winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SDDI_STARTUP_REBOOTSONLY, TRUE);

        if (_ulType == XSTARTUP_EVERYWPSRESTART)
            winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SDDI_STARTUP_EVERYWPSRESTART, TRUE);
    }

    if (flFlags & CBI_ENABLE)
    {
    }
}

/*
 * fdrStartupFolderItemChanged:
 *      notebook callback function (notebook.c) for the
 *      "Startup" page in the startup folder's settings notebook.
 *      (This used to be page 2 in the Desktop's settings notebook
 *      before V0.9.0.)
 *      Reacts to changes of any of the dialog controls.
 *
 *@@changed V0.9.0 [umoeller]: moved this here from xfdesk.c
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 *@@changed V0.9.4 (2000-08-02) [umoeller]: now using sliders; added initial delay
 *@@changed V0.9.9 (2001-03-19) [pr]: multiple startup folder mods.; add Undo & Default processing
 */

MRESULT fdrStartupFolderItemChanged(PNOTEBOOKPAGE pnbp,
                                    ULONG ulItemID, USHORT usNotifyCode,
                                    ULONG ulExtra)      // for checkboxes: contains new state
{
    ULONG   ulChange = 1;
    BOOL fUpdate = TRUE;
    XFldStartupData *somThis = XFldStartupGetData(pnbp->inbp.somSelf);

    switch (ulItemID)
    {
        case ID_SDDI_SHOWSTARTUPPROGRESS:
            cmnSetSetting(sfShowStartupProgress, ulExtra);
            fUpdate = FALSE;
        break;

        case ID_SDDI_STARTUP_INITDELAY_SLID:
        {
            LONG lSliderIndex = winhQuerySliderArmPosition(
                                            pnbp->hwndControl,
                                            SMA_INCREMENTVALUE);
            LONG lMS = (lSliderIndex + 1) * 500;
            CHAR szMS[30];
            sprintf(szMS, "%d ms", lMS);
            WinSetDlgItemText(pnbp->hwndDlgPage,
                              ID_SDDI_STARTUP_INITDELAY_TXT2,
                              szMS);

            cmnSetSetting(sulStartupInitialDelay, lMS);
            fUpdate = FALSE;
        }
        break;

        case ID_SDDI_STARTUP_OBJDELAY_SLID:
        {
            LONG lSliderIndex = winhQuerySliderArmPosition(
                                            pnbp->hwndControl,
                                            SMA_INCREMENTVALUE);
            LONG lMS = (lSliderIndex + 1) * 500;
            CHAR szMS[30];
            sprintf(szMS, "%d ms", lMS);
            WinSetDlgItemText(pnbp->hwndDlgPage,
                              ID_SDDI_STARTUP_OBJDELAY_TXT2,
                              szMS);

            _ulObjectDelay = lMS;
            ulChange = 0;
        }
        break;

        case ID_SDDI_STARTUP_REBOOTSONLY:
            _ulType = XSTARTUP_REBOOTSONLY;
            ulChange = 0;
        break;

        case ID_SDDI_STARTUP_EVERYWPSRESTART:
            _ulType = XSTARTUP_EVERYWPSRESTART;
            ulChange = 0;
        break;

        case DID_UNDO:
            if (pnbp->pUser)
            {
                XFldStartupData *Backup = pnbp->pUser2;
                // "Undo" button: restore backed up instance & global data
                _ulType = Backup->ulType;
                _ulObjectDelay = Backup->ulObjectDelay;
                cmnRestoreSettings(pnbp->pUser,
                                   ARRAYITEMCOUNT(G_StartupFolderBackup));
                // update the display by calling the INIT callback
                pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            }
        break;

        case DID_DEFAULT:
            // "Default" button:
            _ulType = XSTARTUP_REBOOTSONLY;
            _ulObjectDelay = XSTARTUP_DEFAULTOBJECTDELAY;
            cmnSetDefaultSettings(SP_STARTUPFOLDER);
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;

        default:
            ulChange = 0;
    }

    if (fUpdate)
        _wpSaveDeferred(pnbp->inbp.somSelf);

    return (MPARAM)0;
}

#endif
