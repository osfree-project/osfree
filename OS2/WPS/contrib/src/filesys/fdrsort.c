
/*
 *@@sourcefile fdrsort.c:
 *      this file contains the extended folder sort code,
 *      which has been mostly rewritten with V0.9.12.
 *
 *      <B>Introduction to WPS Sorting</B>
 *
 *      The WPS sort functions are a mess, and I suppose
 *      IBM knows that, since they have hardly documented
 *      them at all. So here's what I found out.
 *
 *      Normally, the WPS relates sort criteria to object
 *      details. That is, to implement a sort criterion,
 *      you have to have a corresponding object detail,
 *      as defined in the wpclsQueryDetailsInfo and
 *      wpQueryDetailsData methods. This is superficially
 *      explained in the "WPS Programming Guide", in
 *      "Object Criteria: Details methods".
 *
 *      There are many more object details than those shown
 *      in folder Details views. I have counted 23 here
 *      (but your results may vary), and since there are
 *      only 13 columns in Details view, there should be
 *      about 10 invisible details. (Side note: You can
 *      view all of them by turning to the "Include" page
 *      of any folder and pressing the "Add..." button.)
 *
 *      Most of the details are defined in WPFileSystem's
 *      override of wpclsQueryDetailsInfo, and if
 *      CLASSFIELDINFO.flData has the SORTBY_SUPPORTED flag
 *      set (even if the details column is never visible),
 *      the WPS will make this detail available as a sort
 *      criterion -- that is, the WPS inserts the column title
 *      in the "Sort" submenu, shows it on the "Sort" page,
 *      and will use some corresponding comparison function
 *      for the folder containers automatically.
 *
 *      The details to be taken into account for sorting are
 *      determined by the folder's "sort class". For example,
 *      the trash can's sort class is set to XWPTrashObject
 *      (instead of the standard WPFileSystem), and boom! we
 *      get the trash object details in the sort menu.
 *
 *      Over the years, I have tried several approaches to
 *      replace folder sorting. I ran into the following
 *      problems:
 *
 *      1.  It is impossible to add new default details to a
 *          replacement class of WPFileSystem. Even if
 *          WPFileSystem is replaced with another class,
 *          wpQueryFldrSortClass and wpQueryFldrDetailsClass
 *          do not return the replacement class object, but
 *          the original WPFileSystem class object. I have
 *          then tried to replace these two methods too, and
 *          details/sorting would still not always work. There
 *          seem to be some ugly kludges in the WPS internally.
 *
 *      2.  Two sort criteria ("Name" and "Type") do not
 *          correspond to any details columns. Normally, sort
 *          criteria are specified by passing the corresponding
 *          details column to a sort function (see
 *          wpIsSortAttribAvailable, for example). However,
 *          these two criteria have indices of "-2" and "-1".
 *
 *          So special checks had to be added for these indices,
 *          and XWP even adds two more negative indices. See
 *          below.
 *
 *      3.  The documentation for CLASSFIELDINFO is incomplete.
 *
 *      4.  The wpQueryFldrSort and wpSetFldrSort methods are
 *          completely obscure. The prototypes are really useless,
 *          since the WPS uses an undocumented structure for
 *          sorting instead, which we have defined as IBMSORTINFO
 *          below.
 *
 *      5.  The most important limitation was however that there
 *          are no default settings for sorting. The default
 *          sort criterion is always set to "Name", and
 *          "Always sort" is always off. This is hard-coded into
 *          WPFolder and cannot be changed. There is not even an
 *          instance method for querying or setting the "Always sort"
 *          flag, let alone a class method for the default values.
 *          The only documented thing is the ALWAYSSORT setup
 *          string.
 *
 *      Besides, the whole concept is so complicated that I doubt
 *      many users even know what "sort classes" or "details classes"
 *      are about, even though they may set these in a folder's
 *      settings notebook.
 *
 *      <B>The XWorkplace Approach</B>
 *
 *      It's basically a "brute force" method. XWorkplace uses its
 *      own set of global and instance settings for both the default
 *      sort criterion and the "always sort" flag and then _always_
 *      sets its own comparison function directly on the container.
 *      As a result, XWP has to do _all_ the sorting now.
 *
 *      To be able to still get the IBMSORTINFO which sits somewhere
 *      in the WPFolder instance data, we have to hack the WPFolder
 *      instance data directly.
 *
 *      The old folder sort code (back from XFolder, used
 *      before 0.9.12) simply assumed that all items in
 *      the "sort" menu were the same for all folders and
 *      then intercepted the standard sort menu item IDs
 *      and set the corresponding comparison function
 *      from shared\cnrsort.c on the PM container directly.
 *
 *      The default WPS "Sort" menu has the following items:
 *
 *      --  Name                (0x1770)
 *      --  Type                (0x1771)
 *      --  Real name           (0x1777)
 *      --  Size                (0x1778)
 *      --  last write date     (0x1779)
 *      --  last access date    (0x177B)
 *      --  creation date       (0x177D)
 *
 *      The old code failed as soon as some folder used a
 *      non-standard sort class (that is, something other
 *      than WPFileSystem), and the menu items were then
 *      different. The most annoying example of this was
 *      the trash can (but the font folder as well).
 *
 *      I have now finally figured out how the stupid sort
 *      criteria relate to container sort comparison funcs
 *      so we can
 *
 *      1)  still implement global sort settings (default
 *          sort criterion and "always sort" flag as with
 *          XFolder),
 *
 *      2)  but implement sorting for non-standard sort
 *          classes too.
 *
 *      3)  In addition, we now support a global "folders
 *          first" flag.
 *
 *      Two things are changed by XWP extended folder sorting,
 *      and each needs different precautions.
 *
 *      1)  New sort criteria.
 *
 *          XWP introduces "sort by class" and "sort by extension".
 *          Besides, there is a flag for "sort folders first" always,
 *          in addition to the default sort criterion.
 *
 *          As a result, we need a common function for determining
 *          the container comparison func according for a folder
 *          (which will be determined from the folder instance
 *          settings, see below). This is fdrQuerySortFunc in this
 *          file. At all cost, we must set the container comparison
 *          func ourselves and not let the WPS do it, because the WPS
 *          doesn't know about the additional criteria. (This doesn't
 *          always work, but eventually we WILL set the sort func.)
 *
 *          If one of the "special" criteria is queried, we return
 *          a hard-coded sort func then (in shared\cnrsort.c).
 *          All of them now come in two flavors, depending on
 *          whether "folders first" is enabled.
 *
 *          Otherwise fnCompareDetailsColumn is used, which compares
 *          the details columns and takes the "folders first" setting
 *          into account itself.
 *
 *          When a folder is opened, the XWP sort func is then set
 *          from fdrManipulateNewView.
 *
 *      2)  Sort settings.
 *
 *          This is particularly difficult, because WPFolder stores
 *          a IBMSORTINFO structure with the instance data. This
 *          thing is very obscure because it even contains a pointer
 *          to the comparison func. While I have now found that the
 *          WPS needs this for getting the details data from the
 *          object records, this pointer even gets saved to the
 *          folders EA's then. Whatever.
 *
 *          We cannot hack the "default sort" field in that structure
 *          because the WPS will reference that internally all the
 *          time to determine the cnr comparison func. If we give
 *          unknown fields in that function, we are asking for trouble,
 *          since we CANNOT suppress the WPS sort code in all situations...
 *          much of it is hidden in undocumented methods which we cannot
 *          override.
 *
 *          As a result, we need our own XFolder instance variables
 *          for the default sort criterion and the "always sort"
 *          flag (see xfldr.idl).
 *
 *          In addition, we need to hack the "always sort" flag in the
 *          IBMSORTINFO structure to support global "always sort", but
 *          we will need to keep track of whether this flag is set
 *          explicitly for a folder or because the default flag was set.
 *
 *      Note that the object hotkeys for folder sorting still work
 *      even though they still post the hard-coded menu item IDs
 *      from the "Sort" menu as a folder WM_COMMAND. They might NOT
 *      work for non-default sort classes.
 *
 *@@header "filesys\folder.h"
 *@@added V0.9.12 (2001-05-18) [umoeller]
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

#define INCL_WINCOUNTRY
#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMENUS
#define INCL_WINBUTTONS
#define INCL_WINLISTBOXES
#define INCL_WINSTDCNR
#define INCL_WINPOINTERS
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\winh.h"               // PM helper routines

#include "xfldr.ih"                     // XFolder

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\cnrsort.h"             // container sort comparison functions
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\notebook.h"            // generic XWorkplace notebook handling

#include "filesys\folder.h"             // XFolder implementation
#include "filesys\object.h"             // XFldObject implementation

#pragma hdrstop                         // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Private definitions
 *
 ********************************************************************/

// these two are constant, no matter what sort class
#define WPMENUID_SORTBYNAME             0x1770      // "-2" sort criterion
#define WPMENUID_SORTBYTYPE             0x1771      // "-1" sort criterion

/* ******************************************************************
 *
 *   Modify-menu funcs
 *
 ********************************************************************/

/*
 *@@ CheckDefaultSortItem:
 *      called from various menu functions to
 *      check the default item in the "sort" menu.
 *
 *@@added V0.9.12 (2001-05-18) [umoeller]
 *@@changed V0.9.12 (2001-05-29) [umoeller]: fixed duplicate checks
 *@@changed V0.9.13 (2001-06-19) [umoeller]: "always sort" and "folders first" always got unchecked, fixed
 */

STATIC VOID CheckDefaultSortItem(HWND hwndSortMenu,
                                 LONG lSort)
{
    // ULONG ulVarMenuOffset = cmnQuerySetting(sulVarMenuOfs); V1.0.1 (2002-12-08) [umoeller]

    // first run thru the existing menu as composed
    // by the WPS and uncheck the default item.
    // MM_QUERYDEFAULTITEM doesn't work here for some
    // reason.
    SHORT sDefID,
          sItemCount = (SHORT)WinSendMsg(hwndSortMenu,
                                         MM_QUERYITEMCOUNT,
                                         0, 0);
    for (sDefID = 0;
         sDefID < sItemCount;
         sDefID++)
    {
        SHORT sidThis
            = (SHORT)WinSendMsg(hwndSortMenu,
                                MM_ITEMIDFROMPOSITION,
                                MPFROMSHORT(sDefID),
                                NULL);
        // stop at the first separator because below that
        // we have "always sort" and "folders first", which
        // we don't want to unset
        // V0.9.13 (2001-06-19) [umoeller]
        if (sidThis == *G_pulVarMenuOfs + ID_XFMI_OFS_SEPARATOR)
            break;

        winhSetMenuItemChecked(hwndSortMenu,
                               sidThis,
                               FALSE);                     // uncheck
    }

    // we need to differentiate here:

    // -- if it's "sort by class" or "sort by extension",
    //    which have been added above, we must set this
    //    explicitly

    switch (lSort)
    {
        case -1:
            sDefID = WPMENUID_SORTBYTYPE;
        break;

        case -2:
            sDefID = WPMENUID_SORTBYNAME;
        break;

        case -3:
            sDefID = *G_pulVarMenuOfs + ID_XFMI_OFS_SORTBYCLASS;
        break;

        case -4:
            sDefID = *G_pulVarMenuOfs + ID_XFMI_OFS_SORTBYEXT;
        break;

        default:
            sDefID =   lSort
                     + 2
                     + WPMENUID_SORTBYNAME;
        break;
    }

    WinSendMsg(hwndSortMenu,
               MM_SETDEFAULTITEMID,
               (MPARAM)sDefID,
               (MPARAM)NULL);
}

/*
 *@@ fdrModifySortMenu:
 *      adds the new sort items into the given menu.
 *
 *      This gets called from two locations:
 *
 *      -- from XFolder::wpModifyPopupMenu to hack the
 *         "Sort" menu of a folder context menu;
 *         in this case, hwndMenuWithSortSubmenu is the
 *         main context menu;
 *
 *      -- from the subclassed folder proc to hack the
 *         "Sort" submenu of a folder menu _bar_;
 *         in this case, hwndMenuWithSortSubmenu is the
 *         "View" pulldown, which has the "Sort" menu
 *         in turn.
 *
 *@@added V0.9.12 (2001-05-18) [umoeller]
 */

BOOL fdrModifySortMenu(WPFolder *somSelf,
                       HWND hwndMenuWithSortSubmenu)
{
    BOOL brc = FALSE;
#ifndef __ALWAYSEXTSORT__
    if (cmnQuerySetting(sfExtendedSorting))
#endif
    {
        // ULONG ulVarMenuOffset = cmnQuerySetting(sulVarMenuOfs); V1.0.1 (2002-12-08) [umoeller]
        MENUITEM mi;

        if (winhQueryMenuItem(hwndMenuWithSortSubmenu,
                              WPMENUID_SORT,
                              FALSE,
                              &mi))
        {
            HWND    hwndSortMenu = mi.hwndSubMenu;

            XFolderData *somThis = XFolderGetData(somSelf);

            // cast pointer to WPFolder-internal sort data
            // PIBMSORTINFO psi = (PIBMSORTINFO)_pFolderSortInfo;
            // SHORT sDefID;

            // we'll insert sort by "class" and "extension"
            // behind sort by "type", so find that item first
            if (winhQueryMenuItem(hwndSortMenu,
                                  WPMENUID_SORTBYTYPE,
                                  FALSE,
                                  &mi))
            {
                BOOL    f;

                // "sort by class"
                winhInsertMenuItem(hwndSortMenu,
                                   mi.iPosition + 1,            // behind "sort by type"
                                   *G_pulVarMenuOfs + ID_XFMI_OFS_SORTBYCLASS,
                                   cmnGetString(ID_XSSI_SV_CLASS), // pszSortByClass
                                   MIS_TEXT,
                                   0);

                // "sort by extension"
                winhInsertMenuItem(hwndSortMenu,
                                   mi.iPosition + 2,
                                   *G_pulVarMenuOfs + ID_XFMI_OFS_SORTBYEXT,
                                   cmnGetString(ID_XSSI_SV_EXT), // pszSortByExt
                                   MIS_TEXT,
                                   0);

                brc = TRUE;

                // now check the default sort item...
                CheckDefaultSortItem(hwndSortMenu,
                                     (_lDefSortCrit == SET_DEFAULT)
                                        ? cmnQuerySetting(slDefSortCrit)
                                        : _lDefSortCrit);

                // add "folders first"
                cmnInsertSeparator(hwndSortMenu, MIT_END);

                f = (_lFoldersFirst == SET_DEFAULT)
                        ? cmnQuerySetting(sfFoldersFirst)
                        : _lFoldersFirst;
                winhInsertMenuItem(hwndSortMenu,
                                   MIT_END,
                                   *G_pulVarMenuOfs + ID_XFMI_OFS_SORTFOLDERSFIRST,
                                   cmnGetString(ID_XSSI_SV_FOLDERSFIRST),
                                   MIS_TEXT,
                                   (f) ? MIA_CHECKED : 0);

                // add "always sort"
                cmnInsertSeparator(hwndSortMenu, MIT_END);

                f = (_lAlwaysSort == SET_DEFAULT)
                        ? cmnQuerySetting(sfAlwaysSort)
                        : _lAlwaysSort;
                winhInsertMenuItem(hwndSortMenu, MIT_END,
                                   *G_pulVarMenuOfs + ID_XFMI_OFS_ALWAYSSORT,
                                   cmnGetString(ID_XSSI_SV_ALWAYSSORT), // pszAlwaysSort
                                   MIS_TEXT,
                                   (f) ? MIA_CHECKED : 0);
            }
        }
    }

    return brc;
}

/*
 *@@ fdrSortMenuItemSelected:
 *      this is used to check if the selected menu item is one
 *      of the folder sort things and, if so, sets the folder
 *      sort settings accordingly, or sorts the folder once.
 *
 *      This gets called from two locations:
 *
 *      --  from mnuFolderSelectingMenuItem in the context
 *          of fnwpSubclassedWPFolderWindow to intercept
 *          the folder sort menu items even before they
 *          get passed to wpMenuItemSelected so that we
 *          can keep the menu open instead of dismissing
 *          it.
 *
 *      --  from fdrSortMenuItemSelected in the context of
 *          XFolder::wpMenuItemSelected; this processes
 *          folder hotkeys.
 *
 *      If this function returns TRUE, it is assumed that the
 *      menu item was processed. Only in that case this function
 *      may set *pbDismiss to whether the menu should be
 *      dismissed.
 *
 *      Note that pbDismiss can be passed as NULL, which means
 *      that the function got called while dealing with hotkeys
 *      instead of menu items.
 *
 *@@changed V0.9.12 (2001-05-18) [umoeller]: moved this here from fdrmenus.c, mostly rewritten
 *@@changed V0.9.13 (2001-06-19) [umoeller]: dismissing sort menu again, unless shift was pressed
 *@@changed V1.0.4 (2005-09-19) [pr]: use variables from method calls rather than instance vars. directly
 */

BOOL fdrSortMenuItemSelected(WPFolder *somSelf,
                             HWND hwndFrame,
                             HWND hwndMenu,      // may be NULLHANDLE if
                                                 // pbDismiss is NULL also
                             ULONG ulMenuId,
                             PBOOL pbDismiss)    // out: dismiss flag for fnwpSubclassedWPFolderWindow
{
    BOOL            brc = FALSE;
#ifndef __ALWAYSEXTSORT__
    if (cmnQuerySetting(sfExtendedSorting))
#endif
    {
        LONG            lMenuId2 = (LONG)ulMenuId - (LONG)*G_pulVarMenuOfs;
        LONG            lAlwaysSort,
                        lFoldersFirst,
                        lDefaultSort;
        XFolderData     *somThis = XFolderGetData(somSelf);
        BOOL            fShiftPressed = doshQueryShiftState();

        LONG            lSort = -999;           // dumb default for "not set"

        // step 1:
        // check if one of the sort criteria was selected

        PMPF_SORT(("lMenuId2 %d, fShiftPressed %d, pbDismiss 0x%lX",
                lMenuId2, fShiftPressed, pbDismiss));

        switch (lMenuId2)
        {
            // new sort items
            case ID_XFMI_OFS_SORTBYCLASS:
                lSort = -3;
            break;

            case ID_XFMI_OFS_SORTBYEXT:
                lSort = -4;
            break;

            // "Always sort"
            case ID_XFMI_OFS_ALWAYSSORT:
            {
                BOOL                fAlwaysSort;

                _xwpQueryFldrSort(somSelf,
                                  &lDefaultSort,
                                  &lFoldersFirst,
                                  &lAlwaysSort);
                fAlwaysSort = (lAlwaysSort == SET_DEFAULT)      // V1.0.4
                                  ? cmnQuerySetting(sfAlwaysSort)
                                  : lAlwaysSort;                // V1.0.4

                PMPF_SORT(("ID_XFMI_OFS_ALWAYSSORT, old fAlwaysSort: %d",
                            fAlwaysSort));

                _xwpSetFldrSort(somSelf,
                                lDefaultSort,
                                lFoldersFirst,
                                !fAlwaysSort);

                winhSetMenuItemChecked(hwndMenu,
                                       ulMenuId,
                                       !fAlwaysSort);

                if (pbDismiss)
                    // do not dismiss menu
                    *pbDismiss = FALSE;

                brc = TRUE;
            }
            break;

            // "folders first"
            case ID_XFMI_OFS_SORTFOLDERSFIRST:
            {
                BOOL fFoldersFirst;

                _xwpQueryFldrSort(somSelf,
                                  &lDefaultSort,
                                  &lFoldersFirst,
                                  &lAlwaysSort);
                fFoldersFirst = (lFoldersFirst == SET_DEFAULT)  // V1.0.4
                                    ? cmnQuerySetting(sfFoldersFirst)
                                    : lFoldersFirst;            // V1.0.4

                PMPF_SORT(("ID_XFMI_OFS_SORTFOLDERSFIRST, old fFoldersFirst: %d",
                            fFoldersFirst));

                _xwpSetFldrSort(somSelf,
                                lDefaultSort,
                                !fFoldersFirst,
                                lAlwaysSort);

                winhSetMenuItemChecked(hwndMenu,
                                       ulMenuId,
                                       !fFoldersFirst);

                if (pbDismiss)
                    // do not dismiss menu
                    *pbDismiss = FALSE;

                brc = TRUE;
            }
            break;

            default:
                if (ulMenuId == WPMENUID_SORTBYNAME)
                    lSort = -2;
                else if (ulMenuId == WPMENUID_SORTBYTYPE)
                    lSort = -1;
                else
                    // check if maybe this is one of the
                    // sort criteria from the details columns;
                    // for this, the WPS uses 6002 plus the
                    // details column index
                    if (    (ulMenuId >= 6002)
                         && (ulMenuId <= 6200)
                       )
                        // looks like it:
                        lSort = ulMenuId - 6002;
        }

        // 2) SORT if a sort criterion was selected
        if (lSort != -999)
        {
            // yes:
            _xwpQueryFldrSort(somSelf,
                              &lDefaultSort,
                              &lFoldersFirst,
                              &lAlwaysSort);

            if ((fShiftPressed) && (pbDismiss))
            {
                // shift was pressed, and not from hotkey:
                // change the folder sort settings
                PMPF_SORT(("    calling _xwpSetFldrSort(%s, hwnd 0x%lX, lSort %d)",
                        _wpQueryTitle(somSelf), hwndFrame, lSort));

                _xwpSetFldrSort(somSelf,
                                lSort,
                                lFoldersFirst,
                                lAlwaysSort);
                // update the menu
                CheckDefaultSortItem(hwndMenu,
                                     lSort);

                // only if shift was pressed, do not
                // dismiss menu V0.9.13 (2001-06-19) [umoeller]
                if (pbDismiss)
                    *pbDismiss = FALSE;
            }
            else
            {
                // shift was NOT pressed, or hotkey:
                // just sort once
                PMPF_SORT(("calling _xwpSortViewOnce(%s, hwnd 0x%lX, lSort %d)",
                        _wpQueryTitle(somSelf), hwndFrame, lSort));

                _xwpSortViewOnce(somSelf,
                                 hwndFrame,
                                 lSort);
            }

            // say "processed"
            brc = TRUE;
        }
    }

    return brc;
}

/* ******************************************************************
 *
 *   Folder sort comparison funcs
 *
 ********************************************************************/

/*
 *@@ CompareStrings:
 *
 *      NOTE: This is a WPS comparison func, NOT a cnr
 *      comparison func. This gets called from
 *      fnCompareDetailsColumn.
 *
 *@@added V0.9.12 (2001-05-18) [umoeller]
 */

STATIC LONG EXPENTRY CompareStrings(PSZ *ppsz1,     // ptr to PSZ 1
                                    PSZ *ppsz2)     // ptr to PSZ 2
{
    PSZ p1 = *ppsz1,
        p2 = *ppsz2;

    // please do not crash on NULL strings, so check this
    if (p1 && p2)
    {
      /*    WinCompareStrings returns:

            #define WCS_EQ               1
            #define WCS_LT               2
            #define WCS_GT               3

            WPS comparison needs:

            #define CMP_EQUAL            0
            #define CMP_GREATER          1
            #define CMP_LESS             2

            Now, these don't match AT ALL...
            Can't believe we have to translate
            WinCompareStrings to WPS value, which
            is then translated to cnr comparison
            value AGAIN, which must be -1, 0, or
            +1. Who came up with this crap?
      */

        ULONG ul = WinCompareStrings(G_habThread1,
                                     0,
                                     0,
                                     p1,
                                     p2,
                                     0);
        switch (ul)
        {
            case WCS_LT: return CMP_LESS;
            case WCS_GT: return CMP_GREATER;
        }
    }
    else if (p1)
        // but p2 is NULL: p1 greater than p2 then
        return CMP_GREATER;
    else if (p2)
        // but p1 is NULL: p1 less than p2 then
        return CMP_LESS;

    // return 0 if strcmp returned 0 above or both strings are NULL
    return 0;
}

/*
 *@@ CompareULongs:
 *      sorts ULONG values in ascending order.
 *
 *      NOTE: This is a WPS comparison func, NOT a cnr
 *      comparison func. This gets called from
 *      fnCompareDetailsColumn.
 *
 *@@added V0.9.12 (2001-05-18) [umoeller]
 */

STATIC LONG EXPENTRY CompareULongs(PULONG pul1,     // ptr to ul1
                                   PULONG pul2)     // ptr to ul2
{
    if (*pul1 > *pul2)
        return CMP_GREATER;
    if (*pul1 < *pul2)
        return CMP_LESS;
    return CMP_EQUAL;
}

/*
 *@@ CompareDate:
 *      sorts by date.
 *
 *      NOTE: This assumes that the field offsets point
 *      to CDATE values AND the next field offset is
 *      a CTIME value. This is the same behavior as
 *      with the WPS, which never documented this behavior
 *      though.
 *
 *@@added V0.9.12 (2001-05-20) [umoeller]
 */

STATIC LONG EXPENTRY CompareDate(PCDATE pd1,     // ptr to ul1
                                 PCDATE pd2)     // ptr to ul2
{
    /*  typedef struct _CDATE {
           UCHAR      day;
           UCHAR      month;
           USHORT     year;
         } CDATE;

        typedef struct _CTIME {
           UCHAR     hours;
           UCHAR     minutes;
           UCHAR     seconds;
           UCHAR     ucReserved;
        } CTIME; */

    // compare years
    if (pd1->year > pd2->year)
        return CMP_GREATER;
    else if (pd1->year < pd2->year)
        return CMP_LESS;
    else
    {
        // compare months
        if (pd1->month > pd2->month)
            return CMP_GREATER;
        else if (pd1->month < pd2->month)
            return CMP_LESS;
        else
        {
            // compare days
            if (pd1->day > pd2->day)
                return CMP_GREATER;
            else if (pd1->day < pd2->day)
                return CMP_LESS;
            else
            {
                PCTIME pt1 = (PCTIME)(pd1 + 1);
                PCTIME pt2 = (PCTIME)(pd2 + 1);
                // compare hours
                if (pt1->hours > pt2->hours)
                    return CMP_GREATER;
                else if (pt1->hours < pt2->hours)
                    return CMP_LESS;
                else
                {
                    // compare minutes
                    if (pt1->minutes > pt2->minutes)
                        return CMP_GREATER;
                    else if (pt1->minutes < pt2->minutes)
                        return CMP_LESS;
                    else
                    {
                        // compare seconds
                        if (pt1->seconds > pt2->seconds)
                            return CMP_GREATER;
                        else if (pt1->seconds < pt2->seconds)
                            return CMP_LESS;
                    }
                }
            }
        }
    }

    return CMP_EQUAL;
}

/*
 *@@ fnCompareDetailsColumn:
 *      special container comparison func which is
 *      set on the folder cnr if the sort is to be
 *      performed according to one of the details
 *      columns.
 *
 *      While four sort criteria are hard-coded
 *      (those with the negative values, see
 *      XFolder::xwpSetFldrSort), all positive
 *      values are assumed to refer to details
 *      columns.
 *
 *      Now, this function is quite a monster.
 *      Quite a bit of trickery is necessary to
 *      get access to the objects' details data
 *      since the stupid container control doesn't
 *      support the "pStorage" parameter when a
 *      sort function is set permanently via CNRINFO,
 *      which would have made things a bit easier.
 *
 *@@added V0.9.12 (2001-05-18) [umoeller]
 *@@changed V0.9.18 (2002-03-23) [umoeller]: speed optimizations
 */

SHORT EXPENTRY fnCompareDetailsColumn(PMINIRECORDCORE pmrc1,
                                      PMINIRECORDCORE pmrc2,
                                      PVOID pStorage)           // unused
{
    // get the object pointers
    WPObject *pobj1 = OBJECT_FROM_PREC(pmrc1),
             *pobj2 = OBJECT_FROM_PREC(pmrc2);

    // get the folder (same for both objects)
    WPFolder *pFolder = _wpQueryFolder(pobj1);
    // get folder instance data
    XFolderData *somThis = XFolderGetData(pFolder);
    // get WPFolder-internal sort struct
    // from pointer hacked in wpRestoreData
    // PIBMSORTINFO pSortInfo = (PIBMSORTINFO)_pFolderSortInfo;
    PIBMSORTINFO pSortInfo = &((PIBMFOLDERDATA)_pvWPFolderData)->SortInfo;
            // V1.0.0 (2002-08-24) [umoeller]

    BOOL    f1IsOfSortClass,
            f2IsOfSortClass;

    // check if we have "folders first" enabled
    // (either locally or globally, this flag is
    // set by fdrQuerySortFunc)
    if (_bCachedFoldersFirst)
    {
        // yes:
        // resolve shadows
        // (watch out, _wpQueryShadowedObject can return NULL)
        WPObject *pobjDeref1 = _xwpResolveIfLink(pobj1);
        WPObject *pobjDeref2 = _xwpResolveIfLink(pobj2);
        BOOL fIsFldr1 = (pobjDeref1)
                          ? objIsAFolder(pobjDeref1)
                          : FALSE;      // treat broken shadows as non-folders
        BOOL fIsFldr2 = (pobjDeref2)
                          ? objIsAFolder(pobjDeref2)
                          : FALSE;      // treat broken shadows as non-folders

        if (fIsFldr1 != fIsFldr2)
        {
            // only one of the two is a folder:

            if (fIsFldr1)
                // 1 is folder, but 2 is not:
                return -1;

            // well, then 2 is folder, but 1 is not:
            return 1;
        }

        // else: both are folders, or both are non-folders:
        // run the details comparison below...
    }

    // check if the objects are descended from the
    // folder's sort class; the "Class" field has
    // been set by fdrQuerySortFunc
    f1IsOfSortClass = _somIsA(pobj1, pSortInfo->Class),
    f2IsOfSortClass = _somIsA(pobj2, pSortInfo->Class);

    if (     (f1IsOfSortClass)
          && (f2IsOfSortClass)
          // && (_wpIsSortAttribAvailable(pFolder, pSortInfo->lDefaultSort))
                // added V0.9.12 (2001-06-03) [umoeller]
       )
    {
        // OK, we can go for the data values... this is VERY
        // kludgy: the WPS allocates the object details data
        // directly after the MINIRECORDCORE of an object.
        // The field offset has been set by fdrQuerySortFunc.
        PBYTE   pb1 =   (PBYTE)pmrc1                // object 1's MINIRECORDCORE
                      + sizeof(MINIRECORDCORE)      // skip MINIRECORDCORE --> details data
                      + pSortInfo->ulFieldOffset;   // details column field offset
        PBYTE   pb2 =   (PBYTE)pmrc2
                      + sizeof(MINIRECORDCORE)
                      + pSortInfo->ulFieldOffset;

        LONG    lResult = pSortInfo->pfnCompare(pb1, pb2);

        // WPS comparison functions return:
        // -- CMP_EQUAL        0
        // -- CMP_GREATER      1
        // -- CMP_LESS         2
        if (lResult == CMP_LESS)
            // convert to cnr comparison value
            return -1;

        return lResult;
    }
    // else: at least one object doesn't support the criterion...

    if (!f1IsOfSortClass)
        // but object 2 is:
        return 1;

    if (!f2IsOfSortClass)
        // but object 1 is:
        return -1;

    // neither is:
    return 0;
}

/*
 *@@ fdrQuerySortFunc:
 *      this returns the sort comparison function for
 *      the specified sort criterion.
 *
 *      See XFolder::xwpSetFldrSort for the possible
 *      values for lSort.
 *
 *      If one of the hard-coded criteria (with a negative
 *      value) is set, this is easy: we simply return one
 *      of the functions in shared\cnrsort.c.
 *
 *      Otherwise, the comparison function will be
 *      fnCompareDetailsColumn, and a bit of setup will
 *      be required for that function to work.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.12 (2001-05-18) [umoeller]: rewritten
 */

PFN fdrQuerySortFunc(WPFolder *somSelf,
                     LONG lSort)        // in: sort criterion
{
    XFolderData *somThis = XFolderGetData(somSelf);
    BOOL fFoldersFirst = (_lFoldersFirst == SET_DEFAULT)
                            ? cmnQuerySetting(sfFoldersFirst)
                            : _lFoldersFirst;

    // _PmpfF(("FOLDERS_FIRST = %d", fFoldersFirst));

    if (lSort == SET_DEFAULT)
        lSort = cmnQuerySetting(slDefSortCrit);

    switch (lSort)
    {
        // hard-coded criteria: return sort functions from shared\cnrsort.c
        case -1:
            if (fFoldersFirst)
            {
                // _Pmpf(("  returning compareType, folders first"));
                return (PFN)fnCompareTypeFoldersFirst;
            }
            else
            {
                // _Pmpf(("  returning compareType, NO folders first"));
                return (PFN)fnCompareType;
            }

        case -2:
            if (fFoldersFirst)
            {
                // _Pmpf(("  returning compare name, folders first"));
                return (PFN)fnCompareNameFoldersFirst;
            }
            else
            {
                // _Pmpf(("  returning compare name, NO folders first"));
                return (PFN)fnCompareName;
            }

        case -3:
            if (fFoldersFirst)
            {
                // _Pmpf(("  returning compare class, folders first"));
                return (PFN)fnCompareClassFoldersFirst;
            }
            else
            {
                // _Pmpf(("  returning compare class, NO folders first"));
                return (PFN)fnCompareClass;
            }

        case -4:
            if (fFoldersFirst)
            {
                // _Pmpf(("  returning compare extension, folders first"));
                return (PFN)fnCompareExtFoldersFirst;
            }
            else
            {
                // _Pmpf(("  returning compare class, NO folders first"));
                return (PFN)fnCompareExt;
            }

        default:
            // looks like caller wants a details column... well then.
            if (lSort >= 0)
            {
                PIBMSORTINFO psi;
                // PIBMSORTINFO psi;

                if (    (psi = &((PIBMFOLDERDATA)_pvWPFolderData)->SortInfo)
                     && (_wpIsSortAttribAvailable(somSelf,
                                                  lSort))
                     && (psi->Class = _wpQueryFldrSortClass(somSelf))
                   )
                {
                    // alright, set up the sort info for the
                    // cnr comparison func
                    PCLASSFIELDINFO pcfi;
                    ULONG ul;

                    psi->lCurrentSort = lSort;

                    // 1) calculate the field offset into the details data
                    _wpclsQueryDetailsInfo(psi->Class,
                                           &pcfi,
                                           NULL);
                    psi->ulFieldOffset = 0;
                    for (ul = 0;
                         ul < lSort;
                         ul++, pcfi = pcfi->pNextFieldInfo)
                    {
                        // skip the first two columns, they are
                        // not part of the details data
                        if (ul >= 2)
                            psi->ulFieldOffset += pcfi->ulLenFieldData;
                    } // end for

                    // pcfi points to the proper column now

                    // 2) set the WPS comparison func

                    // a) the sort class might have specified
                    //    its own WPS comparison function for
                    //    this details column (for example,
                    //    the WPS "size" column does this because
                    //    it's CFA_STRING)
                    if (!(psi->pfnCompare = pcfi->pfnSort))
                    {
                        // no special sort function specified:
                        // b) use our standard ones
                        if (pcfi->flData & CFA_STRING)
                            psi->pfnCompare = (PFNCOMPARE)CompareStrings;
                        else if (pcfi->flData & CFA_ULONG)
                            psi->pfnCompare = (PFNCOMPARE)CompareULongs;

                        else if (pcfi->flData & CFA_DATE)
                            psi->pfnCompare = (PFNCOMPARE)CompareDate;
                        // forget about CFA_TIME, CompareDate handles this
                        /* else if (pcfi->flData & CFA_TIME)
                            psi->pfnCompare = (PFNCOMPARE)CompareTime; */

                        else
                        {
                            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                                   "Unsupported sortby format %d",
                                   pcfi->flData);
                            // unsupported format: sort by name then
                            // (shouldn't happen, but better be safe
                            // than sorry)
                            return (PFN)fnCompareName;
                        }
                    }

                    // 3) set the cached "folders first" field so
                    //    that fnCompareDetailsColumn can quickly
                    //    read this
                    _bCachedFoldersFirst = fFoldersFirst;

                    /* _Pmpf(("  returning compare details, column %d, folders first: %d",
                            psi->lCurrentSort,
                            fFoldersFirst)); */

                    // 4) return the details column _cnr_ comparison func,
                    //    which will use the WPS comparison func
                    //    in turn
                    return (PFN)fnCompareDetailsColumn;
                }
            }

        // end default
    }

    cmnLog(__FILE__, __LINE__, __FUNCTION__,
           "Invalid sort criterion %d", lSort);

    // invalid lSort specified:
    return (PFN)fnCompareName;
}

/* ******************************************************************
 *
 *   Interfaces, callbacks
 *
 ********************************************************************/

/*
 *@@ fdrHasAlwaysSort:
 *      general function which returns whether the
 *      specified folder has the "always sort" flag
 *      set.
 *
 *      If extended folder sorting is enabled, this
 *      returns the instance or the global setting.
 *
 *      Otherwise, this returns the flag from
 *      the IBMSORTINFO.
 *
 *@@added V0.9.12 (2001-05-19) [umoeller]
 */

BOOL fdrHasAlwaysSort(WPFolder *somSelf)
{
    XFolderData *somThis = XFolderGetData(somSelf);

#ifndef __ALWAYSEXTSORT__
    if (cmnQuerySetting(sfExtendedSorting))
#endif
        return (_lAlwaysSort == SET_DEFAULT)
                   ? cmnQuerySetting(sfAlwaysSort)
                   : _lAlwaysSort;

#ifndef __ALWAYSEXTSORT__
    // if (_pFolderSortInfo)
        // return (((PIBMSORTINFO)_pFolderSortInfo)->fAlwaysSort);
    // V1.0.0 (2002-08-24) [umoeller]
    return ((PIBMFOLDERDATA)_pvWPFolderData)->SortInfo.fAlwaysSort;
#endif
}

/*
 * fdrSortAllViews:
 *      callback function for sorting all folder views.
 *      This is called by xf(cls)ForEachOpenView, which also passes
 *      the parameters to this func.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V1.0.0 (2002-08-28) [umoeller]: adjusted to new callback prototype
 */

BOOL _Optlink fdrSortAllViews(WPFolder *somSelf,
                              HWND hwndView,
                              ULONG ulView,
                              ULONG ulSort)      // sort flag
{
    if (    (ulView == OPEN_CONTENTS)
         || (ulView == OPEN_TREE)
         || (ulView == OPEN_DETAILS)
       )
    {
        _xwpSortViewOnce(somSelf,
                         hwndView,
                         ulSort);
        return TRUE;
    }

    return FALSE;
}

/*
 *@@ fdrSortViewOnce:
 *      implementation for XFolder::xwpSortViewOnce.
 *
 *@@added V0.9.19 (2002-04-24) [umoeller]
 */

BOOL fdrSortViewOnce(WPFolder *somSelf,
                     HWND hwndFrame,
                     long lSort)
{
    BOOL        rc = FALSE;

#ifndef __ALWAYSEXTSORT__
    if (cmnQuerySetting(sfExtendedSorting))
#endif
    {
        WPObject *pobjLock = NULL;
        TRY_LOUD(excpt1)
        {
            if (pobjLock = cmnLockObject(somSelf))
            {
                HWND hwndCnr;

                PMPF_SORT(("[%s]{%s} hwndFrame = 0x%lX, lSort = %d",
                            _wpQueryTitle(somSelf),
                            _somGetClassName(somSelf),
                            hwndFrame, lSort));

                if (!(hwndCnr = WinWindowFromID(hwndFrame, FID_CLIENT)))
                    cmnLog(__FILE__, __LINE__, __FUNCTION__,
                           "hwndCnr is NULLHANDLE, cannot sort");
                else
                {
                    CNRINFO CnrInfo;
                    ULONG   ulStyle = 0;

                    cnrhQueryCnrInfo(hwndCnr, &CnrInfo);

                    if ((CnrInfo.flWindowAttr & (CV_ICON | CV_TREE)) == CV_ICON)
                    {
                        // for some reason, icon views need to have "auto arrange" on,
                        // or nothing will happen
                        ulStyle = winhQueryWindowStyle(hwndCnr);
                        WinSetWindowULong(hwndCnr, QWL_STYLE, ulStyle | CCS_AUTOPOSITION);
                    }

                    // send sort msg with proper sort (comparison) func
                    WinSendMsg(hwndCnr,
                               CM_SORTRECORD,
                               (MPARAM)fdrQuerySortFunc(somSelf,
                                                        lSort),
                               MPNULL);

                    if ((CnrInfo.flWindowAttr & (CV_ICON | CV_TREE)) == CV_ICON)
                        // restore old cnr style
                        WinSetWindowULong(hwndCnr, QWL_STYLE, ulStyle);

                    rc = TRUE;
                }
            } // end if (fFolderLocked)
        }
        CATCH(excpt1) {} END_CATCH();

        if (pobjLock)
            _wpReleaseObjectMutexSem(pobjLock);
    }

    return rc;
}

/*
 *@@ fdrSetFldrCnrSort:
 *      this is the most central function of XFolder's
 *      extended sorting capabilities. This is called
 *      every time the container in an open folder view
 *      needs to have its sort settings updated. In other
 *      words, this function evaluates the current folder
 *      sort settings and finds out the corresponding
 *      container sort comparison functions and other
 *      settings.
 *
 *      Parameters:
 *      --  HWND hwndCnr     cnr of open view of somSelf
 *      --  BOOL fForce      TRUE: always update the cnr
 *                           settings, even if they have
 *                           not changed
 *
 *      This function gets called:
 *
 *      1)  from our wpOpen override to set folder sort
 *          when a new folder view opens;
 *
 *      2)  from our wpSetFldrSort override (see notes
 *          there);
 *
 *      3)  after folder sort settings have been changed
 *          using xwpSetFldrSort.
 *
 *      It is usually not necessary to call this method
 *      directly. To sort folders, you should call
 *      XFolder::xwpSetFldrSort or XFolder::xwpSortViewOnce
 *      instead.
 *
 *@@changed V0.9.0 [umoeller]: this used to be an instance method
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.12 (2001-05-18) [umoeller]: now setting wait pointer
 *@@changed V0.9.12 (2001-06-03) [umoeller]: cnrs didn't pick up changes, fixed
 */

VOID fdrSetFldrCnrSort(WPFolder *somSelf,      // in: folder to sort
                       HWND hwndCnr,           // in: container of open view of somSelf
                       BOOL fForce)            // in: always invalidate container?
{
    XFolderData *somThis = XFolderGetData(somSelf);

    if (hwndCnr)
    {
        // set wait pointer V0.9.12 (2001-05-18) [umoeller]
        HPOINTER hptrOld = winhSetWaitPointer();

        WPObject *pobjLock = NULL;
        TRY_LOUD(excpt1)
        {
            if (pobjLock = cmnLockObject(somSelf))
            {
                // this is TRUE if "Always sort" is on either locally or globally
                BOOL            AlwaysSort = (_lAlwaysSort == SET_DEFAULT)
                                                ? cmnQuerySetting(sfAlwaysSort)
                                                : _lAlwaysSort;

                // get our sort comparison func
                PFN             pfnSort =  (AlwaysSort)
                                               ? fdrQuerySortFunc(somSelf,
                                                                  (_lDefSortCrit == SET_DEFAULT)
                                                                     ? cmnQuerySetting(slDefSortCrit)
                                                                     : _lDefSortCrit)
                                               : NULL;
                CNRINFO         CnrInfo = {0};

                cnrhQueryCnrInfo(hwndCnr, &CnrInfo);

                PMPF_SORT(("%s with hwndCnr = 0x%lX",
                            _wpQueryTitle(somSelf), hwndCnr ));
                PMPF_SORT(("  _Always: %d, Global->Always: %d",
                            _lAlwaysSort, cmnQuerySetting(sfAlwaysSort) ));
                PMPF_SORT(("  ALWAYS_SORT returned %d", AlwaysSort ));
                PMPF_SORT(("  _Default: %d, Global->Default: %d",
                            _lDefSortCrit, cmnQuerySetting(slDefSortCrit) ));
                PMPF_SORT(("  pfnSort is 0x%lX", pfnSort ));

                // for icon views, we need extra precautions
                if ((CnrInfo.flWindowAttr & (CV_ICON | CV_TREE)) == CV_ICON)
                {
                    // for some reason, cnr icon views need to have "auto arrange" on
                    // when sorting, or the cnr will allow to drag'n'drop icons freely
                    // within the same cnr, which is not useful when auto-sort is on

                    ULONG       ulStyle = winhQueryWindowStyle(hwndCnr);

                    if (AlwaysSort)
                    {
                        // always sort: we need to set CCS_AUTOPOSITION, if not set
                        if ((ulStyle & CCS_AUTOPOSITION) == 0)
                        {
                            PMPF_SORT(("  Setting CCS_AUTOPOSITION"));

                            WinSetWindowULong(hwndCnr, QWL_STYLE, (ulStyle | CCS_AUTOPOSITION));
                            // Update = TRUE;
                        }
                    }
                    else
                    {
                        // NO always sort: we need to unset CCS_AUTOPOSITION, if set
                        if ((ulStyle & CCS_AUTOPOSITION) != 0)
                        {
                            PMPF_SORT(("  Clearing CCS_AUTOPOSITION"));

                            WinSetWindowULong(hwndCnr, QWL_STYLE, (ulStyle & (~CCS_AUTOPOSITION)));
                            // Update = TRUE;
                        }
                    }
                }

                // now also update the internal WPFolder sort info, because otherwise
                // the WPS will keep reverting the cnr attrs; we have obtained the pointer
                // to this structure in wpRestoreData
                if (objIsObjectInitialized(somSelf))
                    ((PIBMFOLDERDATA)_pvWPFolderData)->SortInfo.fAlwaysSort = AlwaysSort;
                                // V1.0.0 (2002-08-24) [umoeller]
                    // if (_pFolderSortInfo)
                       //  ((PIBMSORTINFO)_pFolderSortInfo)->fAlwaysSort = AlwaysSort;

                // finally, set the cnr sort function: we perform these checks
                // to avoid cnr flickering
                // V0.9.12 (2001-06-03) [umoeller]: no, we must do this always
                // because otherwise changes won't get picked up by the cnr
                /* if (    // sort function changed?
                        (CnrInfo.pSortRecord != (PVOID)pfnSort)
                        // ^^^ disabled this check, because we can now have
                        // the same sort proc for several criteria
                        // V0.9.12 (2001-05-18) [umoeller]
                     ||
                        // CCS_AUTOPOSITION flag changed above?
                        (Update)
                     || (fForce)
                   ) */
                {
                    PMPF_SORT(("  Resetting pSortRecord to %lX", pfnSort ));

                    CnrInfo.pSortRecord = NULL;
                    WinSendMsg(hwndCnr,
                               CM_SETCNRINFO,
                               (MPARAM)&CnrInfo,
                               (MPARAM)CMA_PSORTRECORD);

                    // set the cnr sort function; if this is != NULL, the
                    // container will always sort the records. If auto-sort
                    // is off, pfnSort has been set to NULL above.
                    // now update the CnrInfo, which will sort the
                    // contents and repaint the cnr also;
                    // this might take a long time
                    CnrInfo.pSortRecord = (PVOID)pfnSort;
                    WinSendMsg(hwndCnr,
                               CM_SETCNRINFO,
                               (MPARAM)&CnrInfo,
                               (MPARAM)CMA_PSORTRECORD);
                }
            }
        }
        CATCH(excpt1) {} END_CATCH();

        if (pobjLock)
            _wpReleaseObjectMutexSem(pobjLock);

        WinSetPointer(HWND_DESKTOP, hptrOld);

    } // end if (hwndCnr)
}

/*
 * fdrUpdateFolderSorts:
 *      callback function for updating all folder sorts.
 *      This is called by xf(cls)ForEachOpenView, which also passes
 *      the parameters to this func.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.7 (2000-12-18) [umoeller]: fixed wrong window handle
 *@@changed V1.0.0 (2002-08-28) [umoeller]: adjusted to new callback prototype
 */

BOOL _Optlink fdrUpdateFolderSorts(WPFolder *somSelf,
                                   HWND hwndView,
                                   ULONG ulView,
                                   ULONG fForce)
{
    PMPF_SORT(("%s", _wpQueryTitle(somSelf) ));

    if (    (ulView == OPEN_CONTENTS)
         || (ulView == OPEN_TREE)
         || (ulView == OPEN_DETAILS)
       )
    {
        HWND hwndCnr = WinWindowFromID(hwndView, FID_CLIENT);

        PMPF_SORT(("  hwndView 0x%lX, hwndCnr 0x%lX", hwndView, hwndCnr));

        fdrSetFldrCnrSort(somSelf,
                          hwndCnr, // hwndView, // wrong!! V0.9.7 (2000-12-18) [umoeller]
                          fForce);

        return TRUE;
    }

    return FALSE;
}

/* ******************************************************************
 *
 *   Notebook callbacks (notebook.c) for "Sort" pages
 *
 ********************************************************************/

/*
 *@@ InsertSortItem:
 *      quick helper for getting a sort criterion
 *      into the criteria list box.
 *
 *      We use the details column index as the
 *      list box item handle.
 *
 *@@added V0.9.12 (2001-05-18) [umoeller]
 */

STATIC VOID InsertSortItem(HWND hwndListbox,       // in: sort criteria list box
                           PULONG pulIndex,        // in/out: current index (raised by one here)
                           const char *pcsz,       // in: criterion name
                           LONG lItemHandle)       // in: details column index
{
    WinInsertLboxItem(hwndListbox,
                      *pulIndex,
                      (PSZ)pcsz);
    winhSetLboxItemHandle(hwndListbox,
                          (*pulIndex)++,
                          lItemHandle);
}

static XWPSETTING G_SortBackup[] =
    {
        slDefSortCrit,
        sfFoldersFirst,
        sfAlwaysSort
    };

/*
 * fdrSortInitPage:
 *      "Sort" page notebook callback function (notebook.c).
 *      Sets the controls on the page.
 *
 *      The "Sort" callbacks are used both for the folder settings
 *      notebook page AND the respective "Sort" page in the "Workplace
 *      Shell" object, so we need to keep instance data and the
 *      Global Settings apart.
 *
 *      We do this by examining the page ID in the notebook info struct.
 *
 *@@changed V0.9.0 [umoeller]: updated settings page
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.12 (2001-05-20) [umoeller]: reworked for new sorting features
 *@@changed V0.9.12 (2001-05-20) [umoeller]: moved this here from fdrnotebooks.c
 */

VOID fdrSortInitPage(PNOTEBOOKPAGE pnbp,
                     ULONG flFlags)
{
    HWND        hwndListbox = WinWindowFromID(pnbp->hwndDlgPage,
                                              ID_XSDI_SORTLISTBOX);

    if (flFlags & CBI_INIT)
    {
        M_WPObject *pSortClass;
        ULONG       ulIndex = 0,
                    cColumns = 0,
                    ul;
        PCLASSFIELDINFO   pcfi;

        if (pnbp->inbp.ulPageID == SP_FLDRSORT_FLDR)
        {
            // if we're being called from a folder's notebook,
            // get instance data
            XFolderData *somThis = XFolderGetData(pnbp->inbp.somSelf);

            // first call: backup instance data for "Undo" button;
            // this memory will be freed automatically by the
            // common notebook window function (notebook.c) when
            // the notebook page is destroyed
            if (pnbp->pUser = malloc(sizeof(XFolderData)))
                memcpy(pnbp->pUser, somThis, sizeof(XFolderData));

            // get folder's sort class
            pSortClass = _wpQueryFldrSortClass(pnbp->inbp.somSelf);
        }
        else
        {
            // "Workplace Shell" page:
            // first call: backup Global Settings for "Undo" button;
            // this memory will be freed automatically by the
            // common notebook window function (notebook.c) when
            // the notebook page is destroyed
            pnbp->pUser = cmnBackupSettings(G_SortBackup,
                                            ARRAYITEMCOUNT(G_SortBackup));

            // sort class: always use _WPFileSystem
            pSortClass = _WPFileSystem;
        }

        // 1) insert the hard-coded sort criteria
        InsertSortItem(hwndListbox,
                       &ulIndex,
                       cmnGetString(ID_XSSI_SV_NAME),
                       -2);
        InsertSortItem(hwndListbox,
                       &ulIndex,
                       cmnGetString(ID_XSSI_SV_TYPE),
                       -1);
        InsertSortItem(hwndListbox,
                       &ulIndex,
                       cmnGetString(ID_XSSI_SV_CLASS),
                       -3);
        InsertSortItem(hwndListbox,
                       &ulIndex,
                       cmnGetString(ID_XSSI_SV_EXT),
                       -4);

        // 2) next, insert the class-specific sort criteria
        cColumns = _wpclsQueryDetailsInfo(pSortClass,
                                          &pcfi,
                                          NULL);
        for (ul = 0;
             ul < cColumns;
             ul++, pcfi = pcfi->pNextFieldInfo)
        {
            BOOL fSortable = TRUE;

            if (pnbp->inbp.ulPageID == SP_FLDRSORT_FLDR)
                // call this method only if somSelf is really a folder!
                fSortable = _wpIsSortAttribAvailable(pnbp->inbp.somSelf, ul);

            if (    (fSortable)
                    // sortable columns only:
                 && (pcfi->flCompare & SORTBY_SUPPORTED)
                    // rule out the images:
                 && (0 == (pcfi->flTitle & CFA_BITMAPORICON))
               )
            {
                // OK, usable sort column:
                // add this to the list box
                InsertSortItem(hwndListbox,
                               &ulIndex,
                               pcfi->pTitleData,
                               ul);     // column number as handle
            }
        }
    }

    if (flFlags & CBI_SET)
    {
        LONG lDefaultSort,
             lFoldersFirst,
             lAlwaysSort;
        ULONG ulIndex;

        if (pnbp->inbp.ulPageID == SP_FLDRSORT_FLDR)
        {
            // instance notebook:
            XFolderData *somThis = XFolderGetData(pnbp->inbp.somSelf);
            lDefaultSort = (_lDefSortCrit == SET_DEFAULT)
                                ? cmnQuerySetting(slDefSortCrit)
                                : _lDefSortCrit;
            lFoldersFirst = (_lFoldersFirst == SET_DEFAULT)
                                  ? cmnQuerySetting(sfFoldersFirst)
                                  : _lFoldersFirst;
            lAlwaysSort = (_lAlwaysSort == SET_DEFAULT)
                                  ? cmnQuerySetting(sfAlwaysSort)
                                  : _lAlwaysSort;
        }
        else
        {
            // "Workplace Shell":
            lDefaultSort = cmnQuerySetting(slDefSortCrit);
            lFoldersFirst = cmnQuerySetting(sfFoldersFirst);
            lAlwaysSort = cmnQuerySetting(sfAlwaysSort);
        }

        // find the list box entry with the matching handle
        ulIndex = winhLboxFindItemFromHandle(hwndListbox,
                                             lDefaultSort);
        if (ulIndex != -1)
            winhSetLboxSelectedItem(hwndListbox,
                                    ulIndex,
                                    TRUE);

        winhSetDlgItemChecked(pnbp->hwndDlgPage,
                              ID_XSDI_SORTFOLDERSFIRST,
                              lFoldersFirst);

        winhSetDlgItemChecked(pnbp->hwndDlgPage,
                              ID_XSDI_ALWAYSSORT,
                              lAlwaysSort);
    }
}

/*
 * fdrSortItemChanged:
 *      "Sort" page notebook callback function (notebook.c).
 *      Reacts to changes of any of the dialog controls.
 *
 *      The "Sort" callbacks are used both for the folder settings notebook page
 *      AND the respective "Sort" page in the "Workplace Shell" object, so we
 *      need to keep instance data and the XFolder GLobal Settings apart.
 *
 *@@changed V0.9.0 [umoeller]: updated settings page
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.12 (2001-05-20) [umoeller]: reworked for new sorting features
 *@@changed V0.9.12 (2001-05-20) [umoeller]: moved this here from fdrnotebooks.c
 *@@changed V0.9.12 (2001-05-22) [umoeller]: added inititialized check
 *@@changed V0.9.12 (2001-05-22) [umoeller]: fixed folder refresh
 *@@changed V0.9.12 (2001-05-22) [umoeller]: fixed global "Undo", which never worked
 */

MRESULT fdrSortItemChanged(PNOTEBOOKPAGE pnbp,
                           ULONG ulItemID,
                           USHORT usNotifyCode,
                           ULONG ulExtra)      // for checkboxes: contains new state
{
    if (pnbp->flPage & NBFL_PAGE_INITED)        // V0.9.12 (2001-05-22) [umoeller]
    {
        BOOL fGlobalRefreshViews = FALSE;

        switch (ulItemID)
        {
            case ID_XSDI_ALWAYSSORT:
            case ID_XSDI_SORTFOLDERSFIRST:
            case ID_XSDI_SORTLISTBOX:
            {
                HWND        hwndListbox = WinWindowFromID(pnbp->hwndDlgPage,
                                                          ID_XSDI_SORTLISTBOX);

                ULONG ulSortIndex = (USHORT)(WinSendMsg(hwndListbox,
                                                        LM_QUERYSELECTION,
                                                        (MPARAM)LIT_CURSOR,
                                                        MPNULL));
                LONG lDefaultSort = winhQueryLboxItemHandle(hwndListbox, ulSortIndex);

                BOOL fFoldersFirst = winhIsDlgItemChecked(pnbp->hwndDlgPage,
                                                          ID_XSDI_SORTFOLDERSFIRST);

                BOOL fAlways = winhIsDlgItemChecked(pnbp->hwndDlgPage,
                                                    ID_XSDI_ALWAYSSORT);

                PMPF_SORT(("ulSortIndex = %d", ulSortIndex));
                PMPF_SORT(("  handle of selected = %d", lDefaultSort));
                PMPF_SORT(("  fFoldersFirst = %d", fFoldersFirst));
                PMPF_SORT(("  fAlways = %d", fAlways));

                if (pnbp->inbp.ulPageID == SP_FLDRSORT_FLDR)
                {
                    // change instance data
                    _xwpSetFldrSort(pnbp->inbp.somSelf,
                                    lDefaultSort,
                                    fFoldersFirst,
                                    fAlways);
                        // this refreshes the folder already
                }
                else
                {
                    // change global data:
                    // if the user enabled "always sort", check if
                    // the desktop would be sorted
                    if (fAlways)
                    {
                        LONG lDtpDefaultSort,
                             lDtpFoldersFirst,
                             lDtpAlwaysSort;
                        _xwpQueryFldrSort(cmnQueryActiveDesktop(),
                                          &lDtpDefaultSort,
                                          &lDtpFoldersFirst,
                                          &lDtpAlwaysSort);
                        if (lDtpAlwaysSort != 0)
                        {
                            // issue warning that this might also sort the Desktop
                            if (cmnMessageBoxExt(pnbp->hwndFrame,
                                                 116,
                                                 NULL, 0,
                                                 133,
                                                 MB_YESNO)
                                           == MBID_YES)
                                _xwpSetFldrSort(cmnQueryActiveDesktop(),
                                                lDtpDefaultSort,
                                                lDtpFoldersFirst,
                                                0);     // off
                        }
                    }

                    PMPF_SORT(("  updating global sort settings, new defsort: %d", lDefaultSort));

                    // moved lock down V0.9.12 (2001-05-20) [umoeller]
                    cmnSetSetting(sfFoldersFirst, fFoldersFirst);
                    cmnSetSetting(slDefSortCrit, lDefaultSort);
                    cmnSetSetting(sfAlwaysSort, fAlways);

                    fGlobalRefreshViews = TRUE;
                }
            }
            break;

            // control other than listbox:
            case DID_UNDO:
                // "Undo" button: restore backed up instance/global data
                if (pnbp->inbp.ulPageID == SP_FLDRSORT_FLDR)
                {
                    // if we're being called from a folder's notebook,
                    // restore instance data
                    if (pnbp->pUser)
                    {
                        XFolderData *Backup = (pnbp->pUser);
                        _xwpSetFldrSort(pnbp->inbp.somSelf,
                                        Backup->lDefSortCrit,
                                        Backup->lFoldersFirst,
                                        Backup->lAlwaysSort);
                    }
                }
                else
                {
                    // global sort page:
                    cmnRestoreSettings(pnbp->pUser, ARRAYITEMCOUNT(G_SortBackup));
                    fGlobalRefreshViews = TRUE;
                }

                // update the display by calling the INIT callback
                pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            break;

            case DID_DEFAULT:
                // "Default" button:
                if (pnbp->inbp.ulPageID == SP_FLDRSORT_FLDR)
                    _xwpSetFldrSort(pnbp->inbp.somSelf,
                                    SET_DEFAULT,
                                    SET_DEFAULT,
                                    SET_DEFAULT);
                else
                {
                    cmnSetDefaultSettings(SP_FLDRSORT_GLOBAL);
                    fGlobalRefreshViews = TRUE;
                }

                // update the display by calling the INIT callback
                pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            break;
        }

        if (fGlobalRefreshViews)
        {
            HPOINTER hptrOld = winhSetWaitPointer();
            // global:

            ntbUpdateVisiblePage(NULL,      // any object
                                 SP_FLDRSORT_FLDR);

            // update all open folders
            fdrForEachOpenGlobalView(fdrUpdateFolderSorts,
                                     TRUE);  // force
            WinSetPointer(HWND_DESKTOP, hptrOld);
        }
    }

    return 0;
}


