
/*
 *@@sourcefile fdrcommand.c:
 *      this file contains the menu selection and object
 *      command logic.
 *
 *      The functions in here are called from XFolder and
 *      XFldDisk whenever object commands come in. This
 *      can be menu selections or WM_COMMAND messages that
 *      were posted in reaction to folder hotkeys.
 *
 *      fcmdMenuItemSelected, which gets called from
 *      XFolder::wpMenuItemSelected and XFldDisk::wpMenuItemSelected,
 *      reacts to folder and disk context menu items.
 *
 *      In addition, we hack the folder sort and folder view submenus
 *      to allow the context menu to stay open when a menu
 *      item was selected, with the help of the subclassed folder
 *      frame winproc (fnwpSubclWPFolderWindow).
 *
 *      Function prefix for this file:
 *      --  fcmd*
 *
 *      This file is new with V1.0.0 and contains code formerly
 *      in fdrmenus.c and fdrsubclass.c. I got tired of switching
 *      between many files to figure out where the code path
 *      actually was when something was selected somewhere.
 *
 *@@header "filesys\fdrcommand.h"
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
#define INCL_WINFRAMEMGR        // SC_CLOSE etc.
#define INCL_WININPUT
#define INCL_WINDIALOGS
#define INCL_WINBUTTONS
#define INCL_WINPOINTERS
#define INCL_WINMENUS
#define INCL_WINSTATICS
#define INCL_WINLISTBOXES
#define INCL_WINENTRYFIELDS
#define INCL_WINSTDCNR
#define INCL_WINMLE
#define INCL_WINCOUNTRY
#define INCL_WINCLIPBOARD
#define INCL_WINSYS
#define INCL_WINPROGRAMLIST     // needed for PROGDETAILS, wppgm.h
#define INCL_WINSHELLDATA       // Prf* functions

#define INCL_GPILOGCOLORTABLE
#define INCL_GPIPRIMITIVES

#define INCL_DEV
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
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\syssound.h"           // system sound helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
// #include "xfobj.ih"                     // XFldObject
#include "xwpstring.ih"                 // XWPString
#include "xfdisk.ih"                    // XFldDisk
#include "xfldr.ih"                     // XFolder

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\classtest.h"           // some cheap funcs for WPS class checks
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\contentmenus.h"        // shared menu logic
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\notebook.h"            // generic XWorkplace notebook handling
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

#include "filesys\fileops.h"            // file operations implementation
#include "filesys\folder.h"             // XFolder implementation
#include "filesys\fdrcommand.h"         // folder menu command reactions
#include "filesys\fdrmenus.h"           // shared folder menu logic
#include "filesys\object.h"             // XFldObject implementation
#include "filesys\program.h"            // program implementation; WARNING: this redefines macros
#include "filesys\statbars.h"           // status bar translation logic
#include "filesys\xthreads.h"           // extra XWorkplace threads

#include "startshut\shutdown.h"         // XWorkplace eXtended Shutdown

// other SOM headers
#pragma hdrstop                         // VAC++ keeps crashing otherwise
#include <wppgm.h>                      // WPProgram, needed for program hacks

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

extern POINTL   G_ptlMouseMenu;         // ptr position when menu was opened;
                                        // this var is in fdrmenus.c

/* ******************************************************************
 *
 *   Copy object filenames
 *
 ********************************************************************/

/*
 *@@ CopyOneObject:
 *
 *@@added V0.9.19 (2002-06-02) [umoeller]
 */

STATIC VOID CopyOneObject(PXSTRING pstr,
                          WPObject *pObject,
                          BOOL fFullPath,
                          const XSTRING *pstrSep)
{
    CHAR szRealName[CCHMAXPATH];
    if (    (pObject = _xwpResolveIfLink(pObject))
         && (_somIsA(pObject, _WPFileSystem))
         && (_wpQueryFilename(pObject, szRealName, fFullPath))
       )
    {
        if (pstr->ulLength)
            xstrcats(pstr, pstrSep);

        xstrcat(pstr, szRealName, 0);
    }
}

/*
 *@@ CopyObjectFileName:
 *      copy object filename(s) to clipboard. This method is
 *      called from several overrides of wpMenuItemSelected.
 *
 *      If somSelf does not have CRA_SELECTED emphasis in the
 *      container, its filename is copied. If it does have
 *      CRA_SELECTED emphasis, all filenames which have CRA_SELECTED
 *      emphasis are copied, separated by spaces.
 *
 *      Note that somSelf might not neccessarily be a file-system
 *      object. It can also be a shadow to one, so we might need
 *      to dereference that.
 *
 *@@changed V0.9.0 [umoeller]: fixed a minor bug when memory allocation failed
 *@@changed V0.9.19 (2002-06-02) [umoeller]: fixed buffer overflow with many objects
 *@@changed V0.9.19 (2002-06-02) [umoeller]: renamed from wpshCopyObjectFileName, moved here
 *@@changed V1.0.0 (2002-11-09) [umoeller]: moved here, added cSep for submenu support @@fixes 219
 */

STATIC BOOL CopyObjectFileName(WPObject *somSelf, // in: the object which was passed to wpMenuItemSelected
                               HWND hwndCnr, // in: the container of the hwmdFrame
                                    // of wpMenuItemSelected
                               BOOL fFullPath, // in: if TRUE, the full path will be
                                    // copied; otherwise the filename only
                               PCSZ pcszSep)    // in: separator if multiple filenames (CRLF or space)
{
    BOOL        fSuccess = FALSE,
                fSingleMode = TRUE;
    XSTRING     strFilenames;
    XSTRING     strSep;

    // get the record core of somSelf
    PMINIRECORDCORE pmrcSelf = _wpQueryCoreRecord(somSelf);

    // now we go through all the selected records in the container
    // and check if pmrcSelf is among these selected records;
    // if so, this means that we want to copy the filenames
    // of all the selected records.
    // However, if pmrcSelf is not among these, this means that
    // either the context menu of the _folder_ has been selected
    // or the menu of an object which is not selected; we will
    // then only copy somSelf's filename.

    PMINIRECORDCORE pmrcSelected = (PMINIRECORDCORE)CMA_FIRST;

    xstrInit(&strFilenames, 1000);
    xstrInitCopy(&strSep, pcszSep, 0);

    do
    {
        // get the first or the next _selected_ item
        pmrcSelected = (PMINIRECORDCORE)WinSendMsg(hwndCnr,
                                                   CM_QUERYRECORDEMPHASIS,
                                                   (MPARAM)pmrcSelected,
                                                   (MPARAM)CRA_SELECTED);

        if ((pmrcSelected != 0) && (((ULONG)pmrcSelected) != -1))
        {
            // first or next record core found:
            // copy filename to buffer
            CopyOneObject(&strFilenames,
                          OBJECT_FROM_PREC(pmrcSelected),
                          fFullPath,
                          &strSep);

            // compare the selection with pmrcSelf
            if (pmrcSelected == pmrcSelf)
                fSingleMode = FALSE;
        }
    } while ((pmrcSelected) && (((ULONG)pmrcSelected) != -1));

    if (fSingleMode)
    {
        // if somSelf's record core does NOT have the "selected"
        // emphasis: this means that the user has requested a
        // context menu for an object other than the selected
        // objects in the folder, or the folder's context menu has
        // been opened: we will only copy somSelf then.
        xstrClear(&strFilenames);
        CopyOneObject(&strFilenames, somSelf, fFullPath, &strSep);
    }

    if (strFilenames.ulLength)
    {
        // something was copied:
        // copy to clipboard
        fSuccess = winhSetClipboardText(WinQueryAnchorBlock(hwndCnr),
                                        strFilenames.psz,
                                        strFilenames.ulLength + 1);
    }

    xstrClear(&strFilenames);
    xstrClear(&strSep);

    return fSuccess;
}

/*
 *@@ CopyObjectFileName2:
 *      calls CopyObjectFileName with the parameters
 *      determined from the given menu item ID.
 *
 *@@added V1.0.0 (2002-11-09) [umoeller]
 */

STATIC BOOL CopyObjectFileName2(WPObject *somSelf, // in: the object which was passed to wpMenuItemSelected
                                HWND hwndCnr, // in: the container of the hwmdFrame
                                    // of wpMenuItemSelected
                                ULONG ulMenuId2)     // in: _OFS_ menu item ID
{
    BOOL    fFullPath;
    PCSZ    pcszSep = NULL;

    switch (ulMenuId2)
    {
        case ID_XFMI_OFS_COPYFILENAME_SHORTSP:
            fFullPath = FALSE;
            pcszSep = " ";
        break;

        case ID_XFMI_OFS_COPYFILENAME_FULLSP:
            fFullPath = TRUE;
            pcszSep = " ";
        break;

        case ID_XFMI_OFS_COPYFILENAME_SHORTNL:
            fFullPath = FALSE;
            pcszSep = "\r\n";
        break;

        case ID_XFMI_OFS_COPYFILENAME_FULLNL:
            fFullPath = TRUE;
            pcszSep = "\r\n";
        break;
    }

    if (pcszSep)
        return CopyObjectFileName(somSelf,
                                  hwndCnr,
                                  fFullPath,
                                  pcszSep);

    return FALSE;
}

/* ******************************************************************
 *
 *   "Selecting menu items" reaction
 *
 ********************************************************************/

/*
 *  The following functions are called on objects even before
 *  wpMenuItemSelected is called, i.e. right after a
 *  menu item has been selected by the user, and before
 *  the menu is dismissed.
 */

/*
 *@@ fcmdSelectingFsysMenuItem:
 *      this is called for file-system objects (folders and
 *      data files) even before wpMenuItemSelected.
 *
 *      This call is the result of a WM_MENUSELECT intercept
 *      of the subclassed frame window procedure of an open folder
 *      (fnwpSubclWPFolderWindow).
 *
 *      We can intercept certain menu item selections here so
 *      that they are not passed to wpMenuItemSelected. This is
 *      the only way we can react to a menu selection and _not_
 *      dismiss the menu (ie. keep it visible after the selection).
 *
 *      Note that somSelf might be a file-system object, but
 *      it might also be a shadow pointing to one, so we might
 *      need to dereference it.
 *
 *      Return value:
 *      -- TRUE          the menu item was handled here; in this case,
 *                       we set *pfDismiss to either TRUE or FALSE.
 *                       If TRUE, the menu will be dismissed and, if
 *                       if (fPostCommand), wpMenuItemSelected will be
 *                       called later.
 *                       If FALSE, the menu will _not_ be dismissed,
 *                       and wpMenuItemSelected will _not_ be called
 *                       later. This is what we return for menu items
 *                       that we have handled here already.
 *      -- FALSE         the menu item was _not_ handled.
 *                       We do _not_ touch *pfDismiss then. In any
 *                       case, wpMenuItemSelected will be called.
 *
 *@@changed V0.9.4 (2000-06-09) [umoeller]: added default documents
 *@@changed V1.0.0 (2002-11-09) [umoeller]: rewrote copy filename support
 */

BOOL fcmdSelectingFsysMenuItem(WPObject *somSelf,
                                  // in: file-system object on which the menu was
                                  // opened
                               USHORT usItem,
                                  // in: selected menu item
                               BOOL fPostCommand,
                                  // in: this signals whether wpMenuItemSelected
                                  // can be called afterwards
                               HWND hwndMenu,
                                  // in: current menu control
                               HWND hwndCnr,
                                  // in: cnr hwnd involved in the operation
                               ULONG ulSelection,
                                  // one of the following:
                                  // -- SEL_WHITESPACE: the context menu was opened on the
                                  //                   whitespace in an open container view
                                  //                   of somSelf (which is a folder then)
                                  // -- SEL_SINGLESEL: the context menu was opened for a
                                  //                   single selected object: somSelf can
                                  //                   be any object then, including folders
                                  // -- SEL_MULTISEL:   the context menu was opened on one
                                  //                   of a multitude of selected objects.
                                  //                   Again, somSelf can be any object
                                  // -- SEL_SINGLEOTHER: the context menu was opened for a
                                  //                   single object _other_ than the selected
                                  //                   objects
                               BOOL *pfDismiss)
                                  // out: if TRUE is returned (ie. the menu item was handled
                                  // here), this determines whether the menu should be dismissed
{
    ULONG           ulMenuId2 = usItem - *G_pulVarMenuOfs;
    BOOL            fHandled = TRUE;
    WPObject        *pObject = somSelf;
    WPFileSystem    *pFileSystem = _xwpResolveIfLink(pObject);

    PMPF_MENUS(("entering"));

    switch (ulMenuId2)
    {

        /*
         * ID_XFMI_OFS_ATTR_ARCHIVED etc.:
         *      update file attributes
         */

        case ID_XFMI_OFS_ATTR_ARCHIVED:
        case ID_XFMI_OFS_ATTR_SYSTEM:
        case ID_XFMI_OFS_ATTR_HIDDEN:
        case ID_XFMI_OFS_ATTR_READONLY:
        {
            ULONG       ulFileAttr;
            ULONG       ulMenuAttr;
            HPOINTER    hptrOld;

            ulFileAttr = _wpQueryAttr(pFileSystem);
            ulMenuAttr = (ULONG)WinSendMsg(hwndMenu,
                                           MM_QUERYITEMATTR,
                                           MPFROM2SHORT(usItem, FALSE),
                                           (MPARAM)MIA_CHECKED);
            // toggle "checked" flag in menu
            ulMenuAttr ^= MIA_CHECKED;  // XOR checked flag;
            WinSendMsg(hwndMenu,
                       MM_SETITEMATTR,
                       MPFROM2SHORT(usItem, FALSE),
                       MPFROM2SHORT(MIA_CHECKED, ulMenuAttr));

            // toggle file attribute
            ulFileAttr ^= // XOR flag depending on menu item
                      (ulMenuId2 == ID_XFMI_OFS_ATTR_ARCHIVED) ? FILE_ARCHIVED
                    : (ulMenuId2 == ID_XFMI_OFS_ATTR_SYSTEM  ) ? FILE_SYSTEM
                    : (ulMenuId2 == ID_XFMI_OFS_ATTR_HIDDEN  ) ? FILE_HIDDEN
                    : FILE_READONLY;

            // loop thru the selected objects
            // change the mouse pointer to "wait" state
            hptrOld = winhSetWaitPointer();

            while (pObject)
            {
                if (pFileSystem)
                {
                    PMPF_MENUS(("  Settings attrs for %s", _wpQueryTitle(pFileSystem)));

                    _wpSetAttr(pFileSystem, ulFileAttr);

                    // update open "File" notebook pages for this object
                    ntbUpdateVisiblePage(pFileSystem, SP_FILE1);
                }

                if (ulSelection == SEL_MULTISEL)
                    pObject = wpshQueryNextSourceObject(hwndCnr, pObject);
                        // note that we're passing pObject, which might
                        // be the shadow
                else
                    pObject = NULL;

                // dereference shadows again
                pFileSystem = _xwpResolveIfLink(pObject);
            }

            WinSetPointer(HWND_DESKTOP, hptrOld);

            // prevent dismissal of menu
            *pfDismiss = FALSE;
        }
        break;  // file attributes

        /*
         * ID_XFMI_OFS_COPYFILENAME_X:
         *      rewritten V1.0.0 (2002-11-09) [umoeller]
         */

        case ID_XFMI_OFS_COPYFILENAME_SHORTSP:
        case ID_XFMI_OFS_COPYFILENAME_FULLSP:
        case ID_XFMI_OFS_COPYFILENAME_SHORTNL:
        case ID_XFMI_OFS_COPYFILENAME_FULLNL:
            CopyObjectFileName2(pObject,
                                hwndCnr,
                                ulMenuId2);
                // note again that we're passing pObject instead
                // of pFileSystem, so that this routine can
                // query all selected objects from shadows too

            // dismiss menu
            *pfDismiss = TRUE;
        break;

        /*
         * ID_XFMI_OFS_FDRDEFAULTDOC:
         *      V0.9.4 (2000-06-09) [umoeller]
         */

        case ID_XFMI_OFS_FDRDEFAULTDOC:
        {
            WPFolder *pMyFolder = _wpQueryFolder(somSelf);
            if (_xwpQueryDefaultDocument(pMyFolder) == somSelf)
                // we are already the default document:
                // unset
                _xwpSetDefaultDocument(pMyFolder, NULL);
            else
                _xwpSetDefaultDocument(pMyFolder, somSelf);
        }
        break;

        default:
            fHandled = FALSE;
    }

    PMPF_MENUS(("leaving"));

    return fHandled;
}

/*
 *@@ fcmdSelectingFdrMenuItem:
 *      this is called for folders before wpMenuItemSelected.
 *      See fcmdSelectingFsysMenuItem for details.
 *
 *      Note that somSelf here will never be a shadow pointing
 *      to a folder. It will always be a folder.
 *
 *@@changed V0.9.0 [umoeller]: added support for "Menu bar" item
 *@@changed V0.9.19 (2002-06-18) [umoeller]: ID_XFMI_OFS_SELECTSOME never worked right from edit pulldown, fixed
 *@@changed V0.9.19 (2002-06-18) [umoeller]: added "batch rename"
 *@@changed V0.9.20 (2002-08-08) [umoeller]: added replacement "Paste"
 *@@changed V1.0.1 (2002-11-30) [umoeller]: removed Warp 3 compat code
 */

BOOL fcmdSelectingFdrMenuItem(WPFolder *somSelf,
                              USHORT usItem,
                              BOOL fPostCommand,
                              HWND hwndMenu,
                              HWND hwndCnr,
                              ULONG ulSelection,
                              BOOL *pfDismiss)
{
    ULONG       ulMenuId2 = usItem - *G_pulVarMenuOfs;
    BOOL        fHandled;
    HWND        hwndFrame = WinQueryWindow(hwndCnr, QW_PARENT);

    PMPF_MENUS(("entering"));

    // first check if it's one of the "Sort" menu items
    if (!(fHandled = fdrSortMenuItemSelected(somSelf,
                                             hwndFrame,
                                             hwndMenu,
                                             usItem,
                                             pfDismiss))) // dismiss flag == return value
    {
        fHandled = TRUE;

        // no "sort" menu item:
        switch (ulMenuId2)
        {
            /*
             * ID_XFMI_OFS_SMALLICONS:
             *
             */

            case ID_XFMI_OFS_SMALLICONS:
            {
                // toggle small icons for folder; this menu item
                // only exists for open Icon and Tree views
                CNRINFO CnrInfo;
                ULONG ulViewAttr, ulCnrView;
                ULONG ulMenuAttr = (ULONG)WinSendMsg(hwndMenu,
                                                     MM_QUERYITEMATTR,
                                                     MPFROM2SHORT(usItem,
                                                                  FALSE),
                                                     (MPARAM)MIA_CHECKED);
                // toggle "checked" flag in menu
                ulMenuAttr ^= MIA_CHECKED;  // XOR checked flag;
                WinSendMsg(hwndMenu,
                           MM_SETITEMATTR,
                           MPFROM2SHORT(usItem, FALSE),
                           MPFROM2SHORT(MIA_CHECKED, ulMenuAttr));

                // toggle cnr flags
                cnrhQueryCnrInfo(hwndCnr, &CnrInfo);
                ulCnrView = (CnrInfo.flWindowAttr & CV_TREE) ? OPEN_TREE : OPEN_CONTENTS;
                ulViewAttr = _wpQueryFldrAttr(somSelf, ulCnrView);
                ulViewAttr ^= CV_MINI;      // XOR mini-icons flag
                _wpSetFldrAttr(somSelf,
                               ulViewAttr,
                               ulCnrView);

                *pfDismiss = FALSE;
            }
            break;

            case ID_XFMI_OFS_FLOWED:
            case ID_XFMI_OFS_NONFLOWED:
            case ID_XFMI_OFS_NOGRID:
            {
                // these items exist for icon views only
                ULONG ulViewAttr = _wpQueryFldrAttr(somSelf, OPEN_CONTENTS);
                switch (ulMenuId2)
                {
                    case ID_XFMI_OFS_FLOWED:
                        // == CV_NAME | CV_FLOW; not CV_ICON
                        ulViewAttr = (ulViewAttr & ~CV_ICON) | CV_NAME | CV_FLOW;
                    break;

                    case ID_XFMI_OFS_NONFLOWED:
                        // == CV_NAME only; not CV_ICON
                        ulViewAttr = (ulViewAttr & ~(CV_ICON | CV_FLOW)) | CV_NAME;
                    break;

                    case ID_XFMI_OFS_NOGRID:
                        ulViewAttr = (ulViewAttr & ~(CV_NAME | CV_FLOW)) | CV_ICON;
                    break;
                }

                _wpSetFldrAttr(somSelf,
                               ulViewAttr,
                               OPEN_CONTENTS);

                winhSetMenuItemChecked(hwndMenu,
                                       *G_pulVarMenuOfs + ID_XFMI_OFS_FLOWED,
                                       (ulMenuId2 == ID_XFMI_OFS_FLOWED));
                winhSetMenuItemChecked(hwndMenu,
                                       *G_pulVarMenuOfs + ID_XFMI_OFS_NONFLOWED,
                                       (ulMenuId2 == ID_XFMI_OFS_NONFLOWED));
                winhSetMenuItemChecked(hwndMenu,
                                       *G_pulVarMenuOfs + ID_XFMI_OFS_NOGRID,
                                       (ulMenuId2 == ID_XFMI_OFS_NOGRID));

                // do not dismiss menu
                *pfDismiss = FALSE;
            }
            break;

            case ID_XFMI_OFS_SHOWSTATUSBAR:
            {
                // toggle status bar for folder
                ULONG ulMenuAttr = (ULONG)WinSendMsg(hwndMenu,
                                                     MM_QUERYITEMATTR,
                                                     MPFROM2SHORT(usItem,
                                                                  FALSE),
                                                     (MPARAM)MIA_CHECKED);

                _xwpSetStatusBarVisibility(somSelf,
                                           (ulMenuAttr & MIA_CHECKED)
                                              ? STATUSBAR_OFF
                                              : STATUSBAR_ON,
                                           TRUE);  // update open folder views

                // toggle "checked" flag in menu
                ulMenuAttr ^= MIA_CHECKED;  // XOR checked flag;
                WinSendMsg(hwndMenu,
                           MM_SETITEMATTR,
                           MPFROM2SHORT(usItem, FALSE),
                           MPFROM2SHORT(MIA_CHECKED, ulMenuAttr));

                // do not dismiss menu
                *pfDismiss = FALSE;
            }
            break;

            /*
             * ID_XFMI_OFS_WARP4MENUBAR (added V0.9.0):
             *      "Menu bar" item in "View" context submenu.
             *      This is only inserted if Warp 4 is running,
             *      so we do no additional checks here.
             */

            case ID_XFMI_OFS_WARP4MENUBAR:
            {
                BOOL fMenuVisible = _xwpQueryMenuBarVisibility(somSelf);
                // in order to change the menu bar visibility,
                // we need to resolve the method by name, since
                // we don't have the Warp 4 toolkit headers
                /*
                xfTD_wpSetMenuBarVisibility pwpSetMenuBarVisibility
                    = (xfTD_wpSetMenuBarVisibility)somResolveByName(
                                                    somSelf,
                                                    "wpSetMenuBarVisibility");

                if (pwpSetMenuBarVisibility)
                    pwpSetMenuBarVisibility(somSelf, !fMenuVisible);
                else
                    cmnLog(__FILE__, __LINE__, __FUNCTION__,
                        "Unable to resolve wpSetMenuBarVisibility.");
                */

                // we do have the headers now V1.0.1 (2002-11-30) [umoeller]
                _wpSetMenuBarVisibility(somSelf, !fMenuVisible);

                WinSendMsg(hwndMenu,
                           MM_SETITEMATTR,
                           MPFROM2SHORT(usItem, FALSE),
                           MPFROM2SHORT(MIA_CHECKED,
                                        (fMenuVisible)
                                            ? 0
                                            : MIA_CHECKED));
                // do not dismiss menu
                *pfDismiss = FALSE;
            }
            break;

            default:
                // moved edit items to fcmdProcessViewCommand
                // V1.0.0 (2002-08-26) [umoeller]
                fHandled = FALSE;
        }
    }

    return fHandled;
}

/* ******************************************************************
 *
 *   "Menu item selected" reaction
 *
 ********************************************************************/

/*
 *@@ fcmdProcessViewCommand:
 *      implementation for XFolder::xwpProcessViewCommand.
 *      See remarks there.
 *
 *      Yes, it looks strange that ProcessFolderMsgs calls
 *      xwpProcessViewCommand, which in turn calls this
 *      function... but this gives folder subclasses such
 *      as the trash can a chance to override that method
 *      to implement their own processing.
 *
 *      We also process some _view_-specific commands here
 *      which should always behave the same no matter what
 *      objects are selected. This applies to "select some"
 *      and "batch rename", which have always been broken
 *      some way or the other before 1.0.0. This now works
 *
 *      --  from the "view" submenu of the folder whitespace
 *          context menu;
 *
 *      --  from the "edit" pulldown;
 *
 *      --  from folder hotkeys.
 *
 *@@added V0.9.7 (2001-01-13) [umoeller]
 *@@changed V0.9.9 (2001-02-18) [pr]: fix delete folder from menu bar
 *@@changed V1.0.0 (2002-08-26) [umoeller]: func renamed
 *@@changed V1.0.0 (2002-08-26) [umoeller]: moved "select some" and "batch rename" here to fix duplicate popups
 *@@changed V1.0.2 (2003-02-16) [pr]: "delete original" deleted wrong object, fixed @@fixes 8
 */

BOOL fcmdProcessViewCommand(WPFolder *somSelf,
                            USHORT usCommand,
                            HWND hwndCnr,
                            WPObject *pFirstObject,
                            ULONG ulSelectionFlags)
{
    BOOL brc = FALSE;       // default: not processed, call parent

    PMPF_MENUS(("[%s] entering, usCommand 0x%lX, pFirstObject 0x%lX [%s], ulSelectionFlags %u",
                _wpQueryTitle(somSelf),
                usCommand,
                pFirstObject,
                (pFirstObject)
                    ? _wpQueryTitle(pFirstObject)
                    : NULL,
                ulSelectionFlags));

    switch (usCommand)
    {
        /*
         * WPMENUID_DELETE:
         *
         */

        case WPMENUID_DELETE:

#ifndef __ALWAYSTRASHANDTRUEDELETE__
            if (cmnQuerySetting(sfReplaceDelete))
#endif
            {
                APIRET  frc;
                BOOL    fTrueDelete;

                // use true delete if the user doesn't want the
                // trash can or if the shift key is pressed
                if (!(fTrueDelete = cmnQuerySetting(sfAlwaysTrueDelete)))
                    fTrueDelete = doshQueryShiftState();

                // V1.0.2 (2003-02-16) [pr]: @@fixes 8
                if (!pFirstObject)
                {
                    pFirstObject = wpshQuerySourceObject(somSelf,
                                                         hwndCnr,
                                                         TRUE, // selected mode
                                                         &ulSelectionFlags);

                    PMPF_MENUS(("[%s] modifying, pFirstObject 0x%lX [%s], ulSelectionFlags %u",
                                _wpQueryTitle(somSelf),
                                pFirstObject,
                                (pFirstObject)
                                    ? _wpQueryTitle(pFirstObject)
                                    : NULL,
                                ulSelectionFlags));
                }

                // collect objects from container and start deleting
                frc = fopsStartDeleteFromCnr(NULLHANDLE,
                                                // no anchor block,
                                                // ansynchronously
                                             pFirstObject,
                                                // first source object
                                             ulSelectionFlags,
                                             hwndCnr,
                                             fTrueDelete);

                PMPF_MENUS(("WPMENUID_DELETE: got APIRET %d", frc));

                // return "processed", skip default processing
                brc = TRUE;
            }
        break;  // WPMENUID_DELETE

        /*
         * WPMENUID_SELALL:
         * WPMENUID_DESELALL
         *      these two are totally broken with split views.
         *      Replace them too.
         */

        case WPMENUID_SELALL:
        case WPMENUID_DESELALL:
            cnrhSelectAll(hwndCnr,
                          (usCommand == WPMENUID_SELALL));
        break;

        /*
         * WPMENUID_REFRESH:
         *
         * added V1.0.0 (2002-08-26) [umoeller]
         */

        case WPMENUID_REFRESH:
            fdrForceRefresh(somSelf);

            // invalidate the cnr too
            WinInvalidateRect(hwndCnr, NULL, TRUE);

            brc = TRUE;
        break;

        default:
        {
            // check our own items
            ULONG       ulMenuId2 = usCommand - *G_pulVarMenuOfs;

            switch (ulMenuId2)
            {
                /*
                 * ID_XFMI_OFS_SELECTSOME:
                 *      show "Select by name" dialog.
                 */

                case ID_XFMI_OFS_SELECTSOME:
                    fdrShowSelectSome(WinQueryWindow(hwndCnr, QW_PARENT));
                            // V0.9.19 (2002-04-17) [umoeller]
                    brc = TRUE;
                break;

                /*
                 * ID_XFMI_OFS_BATCHRENAME:
                 *      show "batch rename" dialog.
                 *      V0.9.19 (2002-06-18) [umoeller]
                 */

                case ID_XFMI_OFS_BATCHRENAME:
                    fdrShowBatchRename(WinQueryWindow(hwndCnr, QW_PARENT));
                    brc = TRUE;
                break;
            }
        }
    } // switch (usCommand)

    return brc;
}

/*
 *@@ ProgramObjectSelected:
 *      this subroutine is called by fcmdMenuItemSelected whenever a
 *      program object from the config folders is to be handled;
 *      it does all the necessary fuddling with the program object's
 *      data before opening it, ie. changing the working dir to the
 *      folder's, inserting clipboard data and so on.
 *
 *@@changed V0.9.19 (2002-04-24) [umoeller]: replaced embarassing dialog from XFolder days
 *@@changed V1.0.1 (2003-01-30) [umoeller]: replaced some really embarassing buffer overflows from XFolder days
 */

STATIC BOOL ProgramObjectSelected(WPObject *pFolder,        // in: folder or disk object
                                  WPProgram *pProgram)
{
    PPROGDETAILS    pDetails;
    ULONG           ulSize;

    CHAR            szRealName[CCHMAXPATH];    // Buffer for wpQueryFilename()

    BOOL            ValidRealName = FALSE,
                    StartupChanged = FALSE,
                    ParamsChanged = FALSE,
                    TitleChanged = FALSE,
                    brc = FALSE;

    PSZ             pszOldParams = NULL,
                    pszOldTitle = NULL;
    CHAR            szNewTitle[1024] = "";

    HAB             hab;

    // get program object data
    if ((pDetails = progQueryDetails(pProgram)))
    {
        XSTRING         strNewParams;       // V0.9.16 (2001-10-06)
        ULONG           lenRealName;
        CHAR            szPassRealName[CCHMAXPATH + 2];

        xstrInit(&strNewParams, 0);

        brc = TRUE;

        // dereference disk objects
        if (_somIsA(pFolder, _WPDisk))
            pFolder = _XFldDisk    // V1.0.5 (2006-06-10) [pr]: fix crash
                      ? _xwpSafeQueryRootFolder(pFolder, FALSE, NULL)
                      : _wpQueryRootFolder(pFolder);

        if (pFolder)
            ValidRealName = (_wpQueryFilename(pFolder, szRealName, TRUE) != NULL);
        // now we have the folder's full path

        // there seems to be a bug in wpQueryFilename for
        // root folders, so we might need to append a "\"
        lenRealName = strlen(szRealName);
        if (lenRealName == 2)
        {
            szRealName[2] =  '\\';
            szRealName[3] =  '\0';
            ++lenRealName;
        }

        // *** first trick:
        // if the program object's startup dir has not been
        // set, we will set it to szRealName
        // temporarily; this will start the
        // program object in the directory
        // of the folder whose context menu was selected
        if (    (ValidRealName)
             && (!pDetails->pszStartupDir))
        {
            StartupChanged = TRUE;
            pDetails->pszStartupDir = szRealName;
        }

        // start playing with the object's parameter list,
        // if the global settings allow it
        if (cmnQuerySetting(sfAppdParam))
        {
            ULONG   lenParams;

            // if the folder's real name contains spaces,
            // we need to enclose it in quotes
            if (strchr(szRealName, ' '))
            {
                // turn     name
                // into     "name"
                szPassRealName[0] = '\"';
                memcpy(szPassRealName + 1,
                       szRealName,
                       lenRealName);
                szPassRealName[lenRealName + 1] = '\"';
                szPassRealName[lenRealName + 2] = '\0';
            }
            else
                memcpy(szPassRealName,
                       szRealName,
                       lenRealName + 1);

            // backup prog data for later restore
            if (    (pszOldParams = pDetails->pszParameters)
                 && (lenParams = strlen(pDetails->pszParameters))
               )
            {
                // parameter list not empty:

                // *** second trick:
                // we will append the current folder path to the parameters
                // if the program object's parameter list does not
                // end in "%" ("Netscape support")
                if (    (ValidRealName)
                     && (pszOldParams[lenParams - 1] != '%')
                   )
                {
                    ParamsChanged = TRUE;

                    xstrcpy(&strNewParams, pszOldParams, lenParams);
                    xstrcatc(&strNewParams, ' ');
                    xstrcat(&strNewParams, szPassRealName, 0);
                }

                // *** third trick:
                // replace an existing "%**C" in the parameters
                // with the contents of the clipboard */
                if (strstr(pszOldParams, CLIPBOARDKEY))
                {
                    hab = WinQueryAnchorBlock(HWND_DESKTOP);
                    if (WinOpenClipbrd(hab))
                    {
                        PSZ pszClipText;

                        if (pszClipText = (PSZ)WinQueryClipbrdData(hab, CF_TEXT))
                        {
                            CHAR            szClipBuf[CCHMAXPATH];
                            ULONG           ulOfs = 0;

                            PSZ pszPos = NULL;
                            // copy clipboard text from shared memory,
                            // but limit to 256 chars
                            strlcpy(szClipBuf,
                                    pszClipText,
                                    sizeof(szClipBuf));
                            WinCloseClipbrd(hab);

                            if (!ParamsChanged) // did we copy already?
                                xstrcpy(&strNewParams, pszOldParams, 0);

                            while (xstrFindReplaceC(&strNewParams,
                                                    &ulOfs,
                                                    CLIPBOARDKEY,
                                                    szClipBuf))
                                ;

                            ParamsChanged = TRUE;
                        }
                        else
                        {
                            // no text data in clipboard:
                            WinCloseClipbrd(hab);
                            cmnSetDlgHelpPanel(ID_XFH_NOTEXTCLIP);
                            if (cmnMessageBoxExt(NULLHANDLE,
                                                 116,   // warning
                                                 NULL, 0,
                                                 235,   // no text in clipboard
                                                 MB_YESNO)
                                    != MBID_YES)
                                brc = FALSE;

                            /* replaced this hideously ugly dialog too
                                V0.9.19 (2002-04-24) [umoeller]

                            if (WinDlgBox(HWND_DESKTOP,         // parent is desktop
                                          HWND_DESKTOP,             // owner is desktop
                                          (PFNWP)cmn_fnwpDlgWithHelp, // dialog procedure (common.c)
                                          cmnQueryNLSModuleHandle(FALSE),  // from resource file
                                          ID_XFD_NOTEXTCLIP,        // dialog resource id
                                          (PVOID)NULL)             // no dialog parameters
                                    == DID_CANCEL)
                                brc = FALSE;
                            */
                        }
                    }
                    else
                    {
                        cmnLog(__FILE__, __LINE__, __FUNCTION__,
                               "Unable to open clipboard.");
                        brc = FALSE;
                    }
                }

                if (ParamsChanged)
                    pDetails->pszParameters = strNewParams.psz;
            }
            else
                // parameter list is empty: simply set params
                if (ValidRealName)
                {
                    ParamsChanged = TRUE;
                    // set parameter list to folder name
                    pDetails->pszParameters = szPassRealName;
                    // since parameter list is empty, we need not
                    // search for the clipboard key ("%**C") */
                }
        } // end if (cmnQuerySetting(sfAppdParam))

        // now remove "~" from title, if allowed
        if (    (pszOldTitle = pDetails->pszTitle)
             && (cmnQuerySetting(sfRemoveX))
           )
        {
            PSZ     pszPos;
            ULONG   lenTitle = strlcpy(szNewTitle,
                                       pszOldTitle,
                                       sizeof(szNewTitle));
            if (strhKillChar(szNewTitle,
                             '~',
                             &lenTitle))
            {
                TitleChanged = TRUE;
                pDetails->pszTitle = szNewTitle;
            }
        }

        // now apply new settings, if necessary
        if (StartupChanged || ParamsChanged || TitleChanged)
            if (!_wpSetProgDetails(pProgram, pDetails))
            {
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "Unable to set new startup directory.");
                brc = FALSE;
            }

        if (brc)
            // open the object with new settings
            if (!_wpViewObject(pProgram, NULLHANDLE, OPEN_DEFAULT, 0))
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "wpViewObject failed.");

        // now restore the old settings, if necessary
        if (StartupChanged)
            pDetails->pszStartupDir = NULL;
        if (ParamsChanged)
            pDetails->pszParameters = pszOldParams;
        if (TitleChanged)
            pDetails->pszTitle = pszOldTitle;
        if (StartupChanged || ParamsChanged || TitleChanged)
            _wpSetProgDetails(pProgram, pDetails);

        free(pDetails);
        xstrClear(&strNewParams);
    }
    else
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "wpQueryProgDetails failed.");

    return brc;
}

/*
 *@@ CheckForVariableMenuItems:
 *      called from fcmdMenuItemSelected for the default
 *      case, i.e. checks if one of the variable menu
 *      items (from config folder or folder content
 *      menus) was selected.
 *
 *      Must return TRUE if the menu item was processed.
 *
 *@@added V0.9.14 (2001-07-14) [umoeller]
 */

STATIC BOOL CheckForVariableMenuItems(WPFolder *somSelf,  // in: folder or root folder
                                      HWND hwndFrame,    // in: as in wpMenuItemSelected
                                      ULONG ulMenuId)    // in: selected menu item
{
    BOOL brc = FALSE;

    PVARMENULISTITEM    pItem;
    WPObject            *pObject = NULL;

    ULONG ulFirstVarMenuId = *G_pulVarMenuOfs + ID_XFMI_OFS_VARIABLE;

    if (     (ulMenuId >= ulFirstVarMenuId)
          && (ulMenuId <  ulFirstVarMenuId + G_ulVarItemCount)
       )
    {
        // yes, variable menu item selected:
        // get corresponding menu list item from the list that
        // was created by mnuModifyFolderMenu
        if (pItem = cmnuGetVarItem(ulMenuId - ulFirstVarMenuId))
            pObject = pItem->pObject;

        if (pObject)    // defaults to NULL
        {
            // OK, we've found the corresponding object
            switch (pItem->ulObjType)
            {
                // this data has previously been saved by InsertObjectsFromList when
                // the context menu was created; it contains a flag telling us
                // what kind of menu item we're dealing with

                case OC_TEMPLATE:
                    // create a new object from this template
                    wpshCreateFromTemplate(pObject,  // template
                                           somSelf,    // folder
                                           hwndFrame, // view frame
                                           cmnQuerySetting(sulTemplatesOpenSettings),
                                                    // 0: do nothing after creation
                                                    // 1: open settings notebook
                                                    // 2: make title editable
                                           cmnQuerySetting(sfTemplatesReposition),
                                           &G_ptlMouseMenu); // V0.9.16 (2001-10-23) [umoeller]
                break;  // end OC_TEMPLATE

                case OC_PROGRAM:
                    // WPPrograms are handled separately, for we will perform
                    // tricks on the startup directory and parameters
                    ProgramObjectSelected(somSelf, pObject);
                break;  // end OC_PROGRAM

                case OC_XWPSTRING:      // V0.9.14 (2001-08-25) [umoeller]
                    if (    (krnIsClassReady(G_pcszXWPString))
                         && (_somIsA(pObject, _XWPString))
                       )
                        _xwpInvokeString(pObject,       // string object
                                         1,             // one target
                                         &somSelf);      // target: the folder
                break;

                default:
                    // objects other than WPProgram and WPFolder (which is handled by
                    // the OS/2 menu handling) will simply be opened without further
                    // discussion.
                    // This includes folder content menu items,
                    // which are marked as OC_CONTENT; MB2 clicks into
                    // content menus are handled by the subclassed folder wnd proc
                    _wpViewObject(pObject, NULLHANDLE, OPEN_DEFAULT, 0);

            } // end switch
        } //end else (pObject == NULL)
        brc = TRUE;
    } // end if ((ulMenuId >= ID_XFM_VARIABLE) && (ulMenuId < ID_XFM_VARIABLE+varItemCount))
    // else none of our variable menu items: brc still false

    return brc;
}

/*
 *@@ fcmdMenuItemSelected:
 *      this gets called by XFolder::wpMenuItemSelected and
 *      XFldDisk::wpMenuItemSelected. Since both classes have
 *      most menu items in common, the handling of the
 *      selections can be done for both in one routine.
 *
 *      This routine now checks if one of XFolder's menu items
 *      was selected; if so, it executes corresponding action
 *      and returns TRUE, otherwise it does nothing and
 *      returns FALSE, upon which the caller should
 *      call its parent method to process the menu item.
 *
 *      Note that when called from XFldDisk, somSelf points
 *      to the "root folder" of the disk object instead of
 *      the disk object itself (wpQueryRootFolder).
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist functions
 *@@changed V0.9.0 [umoeller]: "Refresh" item now moved to File thread
 *@@changed V0.9.1 (99-11-29) [umoeller]: "Open parent and close" closed even the Desktop; fixed
 *@@changed V0.9.1 (99-12-01) [umoeller]: "Open parent" crashed for root folders; fixed
 *@@changed V0.9.4 (2000-06-09) [umoeller]: added default document
 *@@changed V0.9.6 (2000-10-16) [umoeller]: fixed "Refresh now"
 *@@changed V0.9.9 (2001-03-27) [umoeller]: removed SOM_CREATEFROMTEMPLATE crap, now calling wpshCreateFromTemplate directly
 *@@changed V0.9.12 (2001-05-03) [umoeller]: removed "Partitions" for WPDrives
 *@@changed V0.9.20 (2002-08-08) [umoeller]: Ctrl+S hotkey was broken, fixed
 */

BOOL fcmdMenuItemSelected(WPFolder *somSelf,  // in: folder or root folder
                          HWND hwndFrame,    // in: as in wpMenuItemSelected
                          ULONG ulMenuId)    // in: selected menu item
{
    BOOL    brc = FALSE;     // "not processed" flag

    if (!somSelf)
        return FALSE;

    TRY_LOUD(excpt1)
    {
        ULONG   ulMenuId2 = ulMenuId - *G_pulVarMenuOfs;

        BOOL        fDummy;
        WPFolder    *pFolder = NULL;

        /*
         *  "Sort" menu items:
         *
         */

        if (fdrSortMenuItemSelected(somSelf,
                                    hwndFrame,
                                    NULLHANDLE,     // we don't know the menu hwnd
                                    ulMenuId,
                                    &fDummy))
            brc = TRUE;

        // no sort menu item:
        // check other variable IDs
        else switch (ulMenuId2)
        {
            /*
             * ID_XFMI_OFS_FDRDEFAULTDOC:
             *      open folder's default document
             *
             *  V0.9.4 (2000-06-09) [umoeller]
             */

            case ID_XFMI_OFS_FDRDEFAULTDOC:
            {
                WPFileSystem *pDefaultDoc;
                if (pDefaultDoc = _xwpQueryDefaultDocument(somSelf))
                {
                    _wpViewObject(pDefaultDoc, NULLHANDLE, OPEN_DEFAULT, 0);
                    _xwpHandleSelfClose(pDefaultDoc, hwndFrame, ulMenuId);
                }
            }
            break;

#ifndef __XWPLITE__
            /*
             * ID_XFMI_OFS_PRODINFO:
             *      "Product Information"
             */

            case ID_XFMI_OFS_PRODINFO:
                cmnShowProductInfo(NULLHANDLE,      // owner
                                   MMSOUND_SYSTEMSTARTUP);
                brc = TRUE;
            break;
#endif

            /*
             * ID_XFMI_OFS_COPYFILENAME_X:
             *      we have to handle these again in the command
             *      routine because these can come in from folder
             *      hotkeys as well
             *
             *      rewritten V1.0.0 (2002-11-09) [umoeller]
             */

            case ID_XFMI_OFS_COPYFILENAME_SHORTSP:
            case ID_XFMI_OFS_COPYFILENAME_FULLSP:
            case ID_XFMI_OFS_COPYFILENAME_SHORTNL:
            case ID_XFMI_OFS_COPYFILENAME_FULLNL:
            {
                // if the user presses hotkeys for "copy filename",
                // we don't want the filename of the folder
                // (which somSelf points to here...), but of the
                // selected objects, so we repost the msg to
                // the first selected object, which will handle
                // the rest
                HWND hwndCnr;
                if (hwndCnr = WinWindowFromID(hwndFrame, FID_CLIENT))
                {
                    PMINIRECORDCORE pmrc = WinSendMsg(hwndCnr,
                                                      CM_QUERYRECORDEMPHASIS,
                                                      (MPARAM)CMA_FIRST, // query first
                                                      (MPARAM)CRA_SELECTED);
                    if ((pmrc != NULL) && ((ULONG)pmrc != -1))
                    {
                        // get object from record core
                        WPObject *pObject2;
                        if (pObject2 = OBJECT_FROM_PREC(pmrc))
                            CopyObjectFileName2(pObject2,
                                                hwndFrame,
                                                ulMenuId2);
                    }
                }
            }
            break;

#ifndef __NOSNAPTOGRID__
            /*
             * ID_XFMI_OFS_SNAPTOGRID:
             *      "Snap to grid"
             */

            case ID_XFMI_OFS_SNAPTOGRID:
                fdrSnapToGrid(somSelf, TRUE);
                brc = TRUE;
            break;
#endif

            /*
             * ID_XFMI_OFS_OPENPARENT:
             *      "Open parent folder":
             *      only used by folder hotkeys also
             *
             * ID_XFMI_OFS_OPENPARENTANDCLOSE:
             *      "open parent, close current"
             *      only used by folder hotkeys also
             */

            case ID_XFMI_OFS_OPENPARENT:
            case ID_XFMI_OFS_OPENPARENTANDCLOSE:
                if (pFolder = _wpQueryFolder(somSelf))
                    _wpViewObject(pFolder, NULLHANDLE, OPEN_DEFAULT, 0);
                else
                    WinAlarm(HWND_DESKTOP, WA_WARNING);

                if (    (ulMenuId2 == ID_XFMI_OFS_OPENPARENTANDCLOSE)
                     && (somSelf != cmnQueryActiveDesktop())
                   )
                    // fixed V0.9.0 (UM 99-11-29); before it was
                    // possible to close the Desktop...
                    _wpClose(somSelf);
                brc = TRUE;
            break;

            /*
             * ID_XFMI_OFS_CONTEXTMENU:
             *      "Show context menu":
             *      only used by folder hotkeys also
             */

            case ID_XFMI_OFS_CONTEXTMENU:
            {
                HWND hwndCnr = WinWindowFromID(hwndFrame, FID_CLIENT);
                POINTS pts = {0, 0};
                WinPostMsg(hwndCnr,
                           WM_CONTEXTMENU,
                           (MPARAM)&pts,
                           MPFROM2SHORT(0, TRUE));
                brc = TRUE;
            }
            break;

            /*
             * ID_XFMI_OFS_REFRESH:
             *      "Refresh now"
             */

            case ID_XFMI_OFS_REFRESH:
                // we used to call _wpRefresh ourselves...
                // apparently this wasn't such a good idea,
                // because the WPS is doing a lot more things
                // than just calling "Refresh". We get messed
                // up container record cores if we just call
                // _wpRefresh this way, so instead we post the
                // WPS the command as if the item from the
                // "View" submenu was selected...

                WinPostMsg(hwndFrame,
                           WM_COMMAND,
                           MPFROMSHORT(WPMENUID_REFRESH),
                           MPFROM2SHORT(CMDSRC_MENU,
                                        FALSE));     // keyboard

                brc = TRUE; // V0.9.11 (2001-04-22) [umoeller]
            break;

            /*
             * ID_XFMI_OFS_CLOSE:
             *      this is only used for the "close window"
             *      folder hotkey;
             *      repost sys command
             */

            case ID_XFMI_OFS_CLOSE:
                WinPostMsg(hwndFrame,
                           WM_SYSCOMMAND,
                           (MPARAM)SC_CLOSE,
                           MPFROM2SHORT(CMDSRC_MENU,
                                        FALSE));        // keyboard
                brc = TRUE;
            break;

            /*
             * ID_XFMI_OFS_BORED:
             *      "[Config folder empty]" menu item...
             *      show a msg box
             */

            case ID_XFMI_OFS_BORED:
                // explain how to configure XFolder
                cmnMessageBoxExt(HWND_DESKTOP,
                                 116,
                                 NULL, 0,
                                 135,
                                 MB_OK);
                brc = TRUE;
            break;


            /*
             * ID_XFMI_OFS_RUN:
             *      open Run dialog
             *
             *  V0.9.14 (2001-08-07) [pr]
             */

            case ID_XFMI_OFS_RUN:
                cmnRunCommandLine(NULLHANDLE, NULL);
                brc = TRUE;
            break;

            /*
             * ID_XFMI_OFS_SPLITVIEW:
             *      "Open" -> "split view".
             *
             * V1.0.0 (2002-08-21) [umoeller]
             */

            case ID_XFMI_OFS_SPLITVIEW:
                _wpViewObject(somSelf,
                              WinWindowFromID(hwndFrame, FID_CLIENT),
                                            // hwndCnr
                              ulMenuId,     // varmenuofs + ID_XFMI_OFS_SPLITVIEW
                              NULLHANDLE);
                _xwpHandleSelfClose(somSelf, hwndFrame, ulMenuId); // V1.0.1 (2002-12-08) [umoeller]
            break;

            /*
             * default:
             *      check for variable menu items
             *      (ie. from config folder or folder
             *      content menus)
             */

            default:
                // moved edit items to fcmdProcessViewCommand
                // V1.0.0 (2002-08-26) [umoeller]

                switch (ulMenuId)
                {
                    case WPMENUID_PASTE:    // V0.9.20 (2002-08-08) [umoeller]
#ifndef __ALWAYSREPLACEPASTE__
                        if (cmnQuerySetting(sfReplacePaste))
#endif
                        {
                            fdrShowPasteDlg(somSelf, hwndFrame);
                            brc = TRUE;
                        }
                    break;

                    default:
                        // anything else: check if it's one of our variable menu items
                        brc = CheckForVariableMenuItems(somSelf,
                                                        hwndFrame,
                                                        ulMenuId);
                } // end switch
        } // end switch;
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    return brc;
    // this flag is FALSE by default; it signals to the caller (which
    // is wpMenuItemSelected of either XFolder or XFldDisk) whether the
    // parent method still needs to be called. If TRUE, we have processed
    // something, if FALSE, we haven't, then call the parent.
}

/*
 *@@ fcmdMenuItemHelpSelected:
 *           display help for a context menu item; this routine is
 *           shared by all XFolder classes also, so you'll find
 *           XFldDesktop items in here too.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist functions
 *@@changed V0.9.4 (2000-08-03) [umoeller]: "View" submenu items never worked; fixed
 *@@changed V0.9.19 (2002-04-17) [umoeller]: replacing help for folder view items now
 */

BOOL fcmdMenuItemHelpSelected(WPObject *somSelf,
                              ULONG MenuId)
{
    ULONG   ulFirstVarMenuId;
    ULONG   ulPanel = 0;
    ULONG   ulMenuId2 = MenuId - *G_pulVarMenuOfs;

    // first check for variable menu item IDs
    switch(ulMenuId2)
    {
        case ID_XFMI_OFS_RESTARTWPS:
            ulPanel = ID_XMH_RESTARTWPS;
        break;

        case ID_XFMI_OFS_SNAPTOGRID:
            ulPanel = ID_XMH_SNAPTOGRID;
        break;

        case ID_XFMI_OFS_REFRESH:
            ulPanel = ID_XMH_REFRESH;
        break;

        case ID_XFMI_OFS_SELECTSOME:
            ulPanel = ID_XFH_SELECTSOME;
        break;

        case ID_XFMI_OFS_BATCHRENAME:       // V0.9.19 (2002-06-18) [umoeller]
            ulPanel = ID_XFH_BATCHRENAME;
        break;

        // items in "View" submenu
        case ID_XFMI_OFS_SMALLICONS:
        case ID_XFMI_OFS_FLOWED:
        case ID_XFMI_OFS_NONFLOWED:
        case ID_XFMI_OFS_NOGRID:
        case ID_XFMI_OFS_WARP4MENUBAR:
        case ID_XFMI_OFS_SHOWSTATUSBAR:
            ulPanel = ID_XFH_VIEW_MENU_ITEMS;
        break;

        // the following adjusted V1.0.0 (2002-11-09) [umoeller]
        case ID_XFM_OFS_COPYFILENAME:
        case ID_XFMI_OFS_COPYFILENAME_SHORTSP:
        case ID_XFMI_OFS_COPYFILENAME_FULLSP:
        case ID_XFMI_OFS_COPYFILENAME_SHORTNL:
        case ID_XFMI_OFS_COPYFILENAME_FULLNL:
            ulPanel = ID_XMH_COPYFILENAME;
        break;

        case ID_XFMI_OFS_SORTBYEXT:
        case ID_XFMI_OFS_SORTBYCLASS:
        case ID_XFMI_OFS_ALWAYSSORT:
        case ID_XFMI_OFS_SORTFOLDERSFIRST:
            ulPanel = ID_XSH_SETTINGS_FLDRSORT;
        break;

        default:
            // none of the variable item ids:
            if (
#ifndef __ALWAYSEXTSORT__
                    (cmnQuerySetting(sfExtendedSorting))
                 &&
#endif
                    (    (MenuId == ID_WPMI_SORTBYNAME)
                      || (MenuId == ID_WPMI_SORTBYREALNAME)
                      // or one of the details columns:
                      || (    (MenuId >= 6002)
                           && (MenuId <= 6200)
                         )
                    )
               )
                ulPanel = ID_XSH_SETTINGS_FLDRSORT;
            else switch (MenuId)
            {
                // replacing help for icon, tree, details views
#ifndef __NOXSHUTDOWN__
                case WPMENUID_SHUTDOWN:
                    if (cmnQuerySetting(sfXShutdown))
                        ulPanel = ID_XMH_XSHUTDOWN;
                break;
#endif

                // replace help for icon, tree, details views
                // V0.9.19 (2002-04-17) [umoeller]
                case WPMENUID_TREE:
                case WPMENUID_ICON:
                case WPMENUID_DETAILS:
                case WPMENUID_CHANGETOICON:
                case WPMENUID_CHANGETOTREE:
                case WPMENUID_CHANGETODETAILS:
                    ulPanel = ID_XSH_FOLDER_VIEWS;
                break;

                default:
                    // if F1 was pressed over one of the variable menu items,
                    // open a help panel with generic help on XFolder
                    ulFirstVarMenuId = (*G_pulVarMenuOfs + ID_XFMI_OFS_VARIABLE);
                    if ( (MenuId >= ulFirstVarMenuId)
                            && (MenuId < ulFirstVarMenuId + G_ulVarItemCount)
                         )
                    {
                        PVARMENULISTITEM pItem;
                        if (pItem = cmnuGetVarItem(MenuId - ulFirstVarMenuId))
                        {
                            // OK, we've found the corresponding object
                            switch (pItem->ulObjType)
                            {
                                // this data has previously been saved by InsertObjectsFromList when
                                // the context menu was created; it contains a flag telling us
                                // what kind of menu item we're dealing with

                                case OC_CONTENTFOLDER:
                                case OC_CONTENT:
                                    ulPanel = ID_XMH_FOLDERCONTENT;
                                break;

                                default:
                                    ulPanel = ID_XMH_VARIABLE;
                                break;
                            }
                        }
                    }
                break;
            }
        break;
    }

    if (ulPanel)
    {
        // now open the help panel we've set above
        cmnDisplayHelp(somSelf,
                       ulPanel);
        return TRUE;
    }

    // none of our items: pass on to parent
    return FALSE;
}


