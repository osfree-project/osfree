
/*
 *@@sourcefile fileops.c:
 *      this has the XWorkplace file operations engine.
 *      This gets called from various method overrides to
 *      implement certain file-operations features.
 *
 *      Current features:
 *
 *      --  Replacement "File exists" dialog implementation
 *          (fopsConfirmObjectTitle).
 *
 *      --  Expanding objects to hold all sub-objects if the
 *          object is a folder. See fopsExpandObjectDeep
 *          and fopsExpandObjectFlat for details.
 *
 *      --  Generic file-operations framework to process
 *          files (actually, any objects) on the XWorkplace
 *          File thread. The common functionality of this code
 *          is to build a list of objects which need to be processed
 *          and pass that list to the XWorkplace File thread, which
 *          will then do the actual processing.
 *
 *          The framework is separated into two layers:
 *
 *          --  the bottom layer (see fops_bottom.c);
 *          --  the top layer (see fops_top.c).
 *
 *          The framework uses the special APIRET error return type,
 *          which is NO_ERROR (0) if no error occurred.
 *
 *      This file is ALL new with V0.9.0.
 *
 *      Function prefix for this file:
 *      --  fops*
 *
 *@@added V0.9.0 [umoeller]
 *@@header "filesys\fileops.h"
 */

/*
 *      Copyright (C) 1997-2010 Ulrich M”ller.
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

#define INCL_DOSERRORS
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_WINWINDOWMGR
#define INCL_WININPUT
#define INCL_WINTIMER
#define INCL_WINPOINTERS
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINSTDCNR
#define INCL_WINSHELLDATA       // Prf* functions
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
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
// #include "xfobj.ih"
#include "xfldr.ih"
#include "xtrash.ih"
#include "xtrashobj.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)
#include "shared\xsetup.h"              // XWPSetup implementation

#include "filesys\fileops.h"            // file operations implementation
#include "filesys\folder.h"             // XFolder implementation
// #include "filesys\object.h"             // XFldObject implementation
#include "filesys\trash.h"              // trash can implementation
#include "filesys\xthreads.h"           // extra XWorkplace threads

// other SOM headers
#pragma hdrstop                         // VAC++ keeps crashing otherwise

/*
 *@@ Add64:
 *      Adds two 64 bit numbers using 32 bit maths.
 *
 *@@added V1.0.9 (2010-07-17) [pr]: added large file support @@fixes 586
 */

STATIC VOID Add64(PLONGLONG pll1, PLONGLONG pll2, PLONGLONG pllResult)
{
    LONGLONG llTemp;
    ULONG ulCarry;

    ulCarry = (pll1->ulLo > (0xFFFFFFFF - pll2->ulLo)) ||
              (pll2->ulLo > (0xFFFFFFFF - pll1->ulLo)) ? 1 : 0;
    llTemp.ulLo = pll1->ulLo + pll2->ulLo;
    llTemp.ulHi = pll1->ulHi + pll2->ulHi + ulCarry;
    *pllResult = llTemp;
}

/* ******************************************************************
 *
 *   Expanded object lists
 *
 ********************************************************************/

/*
 *@@ fopsLoopSneaky:
 *      goes thru the contents of pFolder and
 *      adds the size of all non-instantiated
 *      files using Dos* functions. This operates
 *      on pFolder only and does not recurse into
 *      subfolders.
 *
 *      This ignores all subfolders in *pFolder.
 *
 *      This _raises_ the size of the contents
 *      specified with pdSizeContents by the
 *      size of all files found.
 *
 *      Preconditions: The caller should have locked
 *      pFolder before calling this to make sure the
 *      contents are not modified while we are working
 *      here.
 *
 *@@added V0.9.6 (2000-10-25) [umoeller]
 *@@changed V1.0.9 (2010-07-17) [pr]: added large file support @@fixes 586
 */

APIRET fopsLoopSneaky(WPFolder *pFolder,       // in: folder
                      PULONG pulFilesCount,    // out: no. of dormant files found (raised!)
                      PLONGLONG pllSizeContents)  // out: total size of dormant files found (raised!)
{
    APIRET  frc = NO_ERROR;

    CHAR    szFolderPath[CCHMAXPATH];

    // if we have a "folders only" populate, we
    // need to go thru the contents of the folder
    // and for all DOS files which have not been
    // added yet, we need to add the file size...
    // V0.9.6 (2000-10-25) [umoeller]

    // get folder name
    if (!_wpQueryFilename(pFolder, szFolderPath, TRUE))
        frc = FOPSERR_WPQUERYFILENAME_FAILED;
    else
    {
        M_WPFileSystem  *pWPFileSystem = _WPFileSystem;

        CHAR            szSearchMask[CCHMAXPATH];
        HDIR            hdirFindHandle = HDIR_CREATE;
        FILEFINDBUF3L   ffb3 = {0};      // returned from FindFirst/Next
        ULONG           cbFFB3 = sizeof(ffb3);
        ULONG           ulFindCount = 1;  // look for 1 file at a time

        // _PmpfF(("doing DosFindFirst for %s", szFolderPath));

        // now go find...
        sprintf(szSearchMask, "%s\\*", szFolderPath);
        frc = DosFindFirst(szSearchMask,
                           &hdirFindHandle,
                           // find everything except directories
                           FILE_ARCHIVED | FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
                           &ffb3,
                           cbFFB3,
                           &ulFindCount,
                           FIL_STANDARDL);
        // and start looping...
        while (frc == NO_ERROR)
        {
            // alright... we got the file's name in ffb3.achName
            CHAR    szFullPath[2*CCHMAXPATH];
            sprintf(szFullPath, "%s\\%s", szFolderPath, ffb3.achName);

            // _Pmpf(("    got file %s", szFullPath));

            if (!_wpclsQueryAwakeObject(pWPFileSystem,
                                        szFullPath))
            {
                // object not awake yet: this means that
                // the object was not added to the list above...
                // add the file's size
                // _Pmpf(("        not already instantiated"));
                (*pulFilesCount)++;
                Add64(pllSizeContents, &ffb3.cbFile, pllSizeContents);  // V1.0.9
            }

            ulFindCount = 1;
            frc = DosFindNext(hdirFindHandle,
                              &ffb3,
                              cbFFB3,
                              &ulFindCount);
        } // while (arc == NO_ERROR)

        if (frc == ERROR_NO_MORE_FILES)
            frc = NO_ERROR;

        DosFindClose(hdirFindHandle);
    }

    return frc;
}

/*
 *@@ fopsFolder2ExpandedList:
 *      creates a LINKLIST of EXPANDEDOBJECT items
 *      for the given folder's contents.
 *
 *      Gets called by fopsExpandObjectDeep if that
 *      function has been invoked on a folder.
 *      This calls fopsExpandObjectDeep in turn for
 *      each object found, so this may be called
 *      recursively.
 *
 *      See fopsExpandObjectDeep for an introduction
 *      what these things are good for.
 *
 *@@added V0.9.2 (2000-02-28) [umoeller]
 *@@changed V0.9.3 (2000-04-28) [umoeller]: now pre-resolving wpQueryContent for speed
 *@@changed V0.9.6 (2000-10-25) [umoeller]: added fFoldersOnly
 *@@changed V1.0.9 (2010-01-17) [pr]: added large file support @@fixes 586
 */

PLINKLIST fopsFolder2ExpandedList(WPFolder *pFolder,
                                  PLONGLONG pllSizeContents, // out: size of all objects on list
                                  BOOL fFoldersOnly)
{
    PLINKLIST   pll = lstCreate(FALSE);       // do not free the items
    LONGLONG    llSizeContents = {0,0};

    BOOL        fFolderLocked = FALSE;

    PMPF_TRASHCAN(("Object \"%s\" is a folder, creating SFL", _wpQueryTitle(pFolder) ));

    // lock folder for querying content
    TRY_LOUD(excpt1)
    {
        // populate (either fully or with folders only)
        if (fdrCheckIfPopulated(pFolder,
                                fFoldersOnly))
        {
            if (fFolderLocked = !_wpRequestFolderMutexSem(pFolder, 5000))
            {
                WPObject *pObject;
                // now collect all objects in folder;
                // -- if we have a full populate, we add all objects
                //    to the list;
                // -- if we have a "folders only" populate, we add
                //    all objects we have (which is at least all the
                //    folders, but can contain additional objects)
                // V0.9.20 (2002-07-31) [umoeller]: now using get_pobjNext SOM attribute
                for (pObject = _wpQueryContent(pFolder, NULL, QC_FIRST);
                     pObject;
                     pObject = *__get_pobjNext(pObject))
                {
                    PEXPANDEDOBJECT fSOI = NULL;

                    PMPF_TRASHCAN(("creating SOI for \"%s\" in folder \"%d\"",
                                _wpQueryTitle(pObject),
                                _wpQueryTitle(pFolder) ));

                    // create a list item for this object;
                    // if pObject is a folder, that function will
                    // call ourselves again...
                    fSOI = fopsExpandObjectDeep(pObject,
                                                fFoldersOnly);
                    Add64(&llSizeContents, &fSOI->llSizeThis, &llSizeContents);  // V1.0.9
                    lstAppendItem(pll, fSOI);
                }

                if (fFoldersOnly)
                {
                    ULONG ulFilesCount = 0;
                    fopsLoopSneaky(pFolder,
                                   &ulFilesCount,       // not needed
                                   &llSizeContents);
                }
            }
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (fFolderLocked)
        _wpReleaseFolderMutexSem(pFolder);

    *pllSizeContents = llSizeContents;

    return pll;
}

/*
 *@@ fopsExpandObjectDeep:
 *      creates a EXPANDEDOBJECT for the given
 *      object. If the object is a folder, the
 *      member list is automatically filled with
 *      the folder's contents (recursively, if
 *      the folder contains subfolders), by calling
 *      fopsFolder2ExpandedList.
 *
 *      --  If (fFoldersOnly == FALSE), this does a full
 *          populate on folder which was encountered.
 *          This can take a long time and for many subfolders
 *          can even hang the entire WPS.
 *
 *      --  If (fFoldersOnly == TRUE), this populates
 *          folders with subfolders only. Still, we add
 *          all objects which have been instantiated
 *          on the folder contents list already, but
 *          calculate the folder size using Dos* functions
 *          only.
 *
 *      Call fopsFreeExpandedObject to free the item
 *      returned by this function.
 *
 *      This function is useful for having a uniform
 *      interface which represents an object and all
 *      subobjects even if that object represents a folder.
 *      After calling this function,
 *      EXPANDEDOBJECT.ulSizeThis has the total size
 *      of pObject and all objects which reside in
 *      subfolders (if applicable).
 *
 *@@added V0.9.2 (2000-02-28) [umoeller]
 *@@changed V0.9.6 (2000-10-25) [umoeller]: added fFoldersOnly
 *@@changed V1.0.9 (2010-07-17) [pr]: added large file support
 */

PEXPANDEDOBJECT fopsExpandObjectDeep(WPObject *pObject,
                                     BOOL fFoldersOnly)
{
    PEXPANDEDOBJECT pSOI = NULL;
    if (wpshCheckObject(pObject))
    {
        // create object item
        if (pSOI = (PEXPANDEDOBJECT)malloc(sizeof(EXPANDEDOBJECT)))
        {
            PMPF_TRASHCAN(("SOI for object %s", _wpQueryTitle(pObject) ));

            pSOI->pObject = pObject;
            if (_somIsA(pObject, _WPFolder))
            {
                // object is a folder:
                // fill list (this calls us again for every object found)
                pSOI->pllContentsSFL = fopsFolder2ExpandedList(pObject,
                                                               &pSOI->llSizeThis,  // V1.0.9
                                                                // out: size of files on list
                                                               fFoldersOnly);
            }
            else
            {
                // non-folder:
                pSOI->pllContentsSFL = NULL;
                if (_somIsA(pObject, _WPFileSystem))
                    // is a file system object:
                    _wpQueryFileSizeL(pObject, &pSOI->llSizeThis);  // V1.0.9
                else
                    // abstract object:
                    pSOI->llSizeThis.ulHi = pSOI->llSizeThis.ulLo = 0;
            }

            PMPF_TRASHCAN(("End of SOI for object %s", _wpQueryTitle(pObject) ));
        }
    }

    return pSOI;
}

/*
 *@@ fopsFreeExpandedList:
 *      frees a LINKLIST of EXPANDEDOBJECT items
 *      created by fopsFolder2ExpandedList.
 *      This recurses.
 *
 *@@added V0.9.2 (2000-02-28) [umoeller]
 */

VOID fopsFreeExpandedList(PLINKLIST pllSFL)
{
    if (pllSFL)
    {
        PLISTNODE pNode = lstQueryFirstNode(pllSFL);
        while (pNode)
        {
            PEXPANDEDOBJECT pSOI = (PEXPANDEDOBJECT)pNode->pItemData;
            fopsFreeExpandedObject(pSOI);
                // after this, pSOI->pllContentsSFL is NULL

            pNode = pNode->pNext;
        }
        lstFree(&pllSFL);
    }
}

/*
 *@@ fopsFreeExpandedObject:
 *      this frees a EXPANDEDOBJECT previously
 *      created by fopsExpandObjectDeep. This calls
 *      fopsFreeExpandedList, possibly recursively,
 *      if the object contains a list (ie. is a
 *      folder).
 *
 *@@added V0.9.2 (2000-02-28) [umoeller]
 */

VOID fopsFreeExpandedObject(PEXPANDEDOBJECT pSOI)
{
    if (pSOI)
    {
        if (pSOI->pllContentsSFL)
            // object has a list:
            fopsFreeExpandedList(pSOI->pllContentsSFL);
                // this sets pllContentsSFL to NULL

        free(pSOI);
    }
}

/*
 *@@ fopsExpandObjectFlat:
 *      similar to fopsExpandObjectDeep, this builds
 *      a list of pObject and, if pObject is a folder,
 *      of all contained folders and objects. However,
 *      this creates a simple linked list containing
 *      the objects, so the hierarchy is lost.
 *
 *      This operates in two modes:
 *
 *      --  If (fFoldersOnly == FALSE), this does a
 *          full populate.
 *
 *          After the call, pllObjects contains all the
 *          plain WPObject* pointers, so the list should
 *          have been created with lstCreate(FALSE) in
 *          order not to invoke free() on it.
 *
 *          If folders are found, contained objects are
 *          appended to the list before the folder itself.
 *          Otherwise, objects are returned in the order
 *          in which wpQueryContent returns them, which
 *          is dependent on the underlying file system.
 *          and should not be relied upon.
 *
 *          For example, see the following folder/file hierarchy:
 +
 +              folder1
 +                 +--- folder2
 +                 |      +---- text1.txt
 +                 |      +---- text2.txt
 +                 +--- text3.txt
 *
 *          If fopsExpandObjectFlat(folder1) is invoked,
 *          you get the following order in the list:
 *
 +          text1.txt, text2.txt, folder2, text3.txt, folder1.
 *
 *          In this mode, pulDormantFilesCount is ignored because
 *          all files are awakened by this function.
 *
 *      --  If (fFoldersOnly == TRUE), this populates
 *          subfolders with folders only. As a result, on
 *          the list, you will receive all folders. Before
 *          the subfolder, you _may_ get regular objects
 *          with have already been awakened, but this might
 *          not be the case.
 *
 *          In this mode, pulObjectCount contains only the
 *          object count of awake objects (folders which
 *          have been awakened plus other objects which
 *          were already awake). In addition, pulDormantFilesCount
 *          receives the no. of dormant files encountered.
 *
 *          Again, contained (and awake) objects are
 *          appended to the list before the folder itself.
 *
 *      pllObjects is never cleared by this function,
 *      so you can call this several times with the
 *      same list. If you call this only once, make
 *      sure pllObjects is empty before calling.
 *
 *      This is most useful for (and used with) deleting
 *      a folder and all its contents.
 *
 *@@added V0.9.3 (2000-04-27) [umoeller]
 *@@changed V0.9.3 (2000-04-28) [umoeller]: now pre-resolving wpQueryContent for speed
 *@@changed V0.9.6 (2000-10-25) [umoeller]: added fFoldersOnly
 *@@changed V1.0.1 (2002-12-15) [umoeller]: now using XWPObjList, changed prototype
 *@@changed V1.0.9 (2010-01-17) [pr]: added large file support @@fixes 586
 */

APIRET fopsExpandObjectFlat(XWPObjList *pllObjects, // in: object list to append to
                            WPObject *pObject,      // in: object to start with
                            BOOL fFoldersOnly,
                            PULONG pulObjectCount,  // out: objects count on list;
                                                    // the ULONG must be set to 0 before calling this!
                            PULONG pulDormantFilesCount) // out: count of dormant files;
                                                    // the ULONG must be set to 0 before calling this!
{
    APIRET frc = NO_ERROR;

    if (_somIsA(pObject, _WPFolder))
    {
        // it's a folder:
        // lock it and build a list from its contents
        BOOL    fFolderLocked = FALSE;

        TRY_LOUD(excpt1)
        {
            if (!fdrCheckIfPopulated(pObject,
                                     fFoldersOnly))
                frc = FOPSERR_POPULATE_FAILED;
            else
            {
                if (fFolderLocked = !_wpRequestFolderMutexSem(pObject, 5000))
                {
                    WPObject *pSubObject;
                    // now collect all objects in folder;
                    // -- if we have a full populate, we add all objects
                    //    to the list;
                    // -- if we have a "folders only" populate, we add
                    //    all objects we have (which is at least all the
                    //    folders, but can contain additional objects)
                    // V0.9.20 (2002-07-31) [umoeller]: now using get_pobjNext SOM attribute
                    for (pSubObject = _wpQueryContent(pObject, NULL, QC_FIRST);
                         pSubObject;
                         pSubObject = *__get_pobjNext(pSubObject))
                    {
                        // recurse!
                        // this will add pSubObject to pllObjects
                        frc = fopsExpandObjectFlat(pllObjects,
                                                   pSubObject,
                                                   fFoldersOnly,
                                                   pulObjectCount,
                                                   pulDormantFilesCount);
                        if (frc != NO_ERROR)
                            // error:
                            break;
                    } // end for

                    if (frc == NO_ERROR)
                        if (fFoldersOnly)
                        {
                            LONGLONG llSizeContents = {0,0};
                            // _PmpfF(("calling fopsLoopSneaky; count pre: %d",
                               //                   *pulDormantFilesCount));
                            frc = fopsLoopSneaky(pObject,
                                                 pulDormantFilesCount,
                                                 &llSizeContents);
                            // _Pmpf(("    count post: %d", *pulDormantFilesCount));
                        }
                }
                else
                    frc = FOPSERR_LOCK_FAILED;
            }
        }
        CATCH(excpt1)
        {
            frc = ERROR_PROTECTION_VIOLATION;
        } END_CATCH();

        if (fFolderLocked)
            _wpReleaseFolderMutexSem(pObject);

    } // end if (_somIsA(pObject, _WPFolder))

    // append this object to the list;
    // since we have recursed first, the folder
    // contents come before the folder in the list
    if (frc == NO_ERROR)
    {
        // _PmpfF(("appending %s", _wpQueryTitle(pObject) ));
        _Append(pllObjects, pObject);       // V1.0.1 (2002-12-15) [umoeller]
        if (pulObjectCount)
            (*pulObjectCount)++;
    }
        // _PmpfF(("error %d for %s", _wpQueryTitle(pObject) ));

    return frc;
}

/********************************************************************
 *
 *   "File exists" (title clash) dialog
 *
 ********************************************************************/

/*
 *@@ fnwpTitleClashDlg:
 *      dlg proc for the "File exists" dialog.
 *      Most of the logic for that dialog is in the
 *      fopsConfirmObjectTitle function actually; this
 *      function is only responsible for adjusting the
 *      dialog items' keyboard focus and such.
 */

STATIC MRESULT EXPENTRY fnwpTitleClashDlg(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    #define WM_DELAYEDFOCUS         (WM_USER+1)

    MRESULT mrc = (MRESULT)0;

    switch (msg)
    {
        /*
         * WM_CONTROL:
         *      intercept focus messages and
         *      set focus to what we really want
         */

        case WM_CONTROL:
            switch (SHORT2FROMMP(mp1)) // usNotifyCode
            {
                case EN_SETFOCUS: // == BN_CLICKED
                {
                    if (SHORT1FROMMP(mp1) == ID_XFDI_CLASH_RENAMENEWTXT)
                        winhSetDlgItemChecked(hwndDlg, ID_XFDI_CLASH_RENAMENEW, TRUE);
                    else if (SHORT1FROMMP(mp1) == ID_XFDI_CLASH_RENAMEOLDTXT)
                        winhSetDlgItemChecked(hwndDlg, ID_XFDI_CLASH_RENAMEOLD, TRUE);
                    else if (SHORT1FROMMP(mp1) == ID_XFDI_CLASH_RENAMENEW)
                        WinPostMsg(hwndDlg, WM_DELAYEDFOCUS,
                                   (MPARAM)ID_XFDI_CLASH_RENAMENEWTXT, MPNULL);
                    else if (SHORT1FROMMP(mp1) == ID_XFDI_CLASH_RENAMEOLD)
                        WinPostMsg(hwndDlg, WM_DELAYEDFOCUS,
                                   (MPARAM)ID_XFDI_CLASH_RENAMEOLDTXT, MPNULL);
                }
            }
        break;

        case WM_DELAYEDFOCUS:
            winhSetDlgItemFocus(hwndDlg, (HWND)mp1);
        break;

        /*
         * WM_COMMAND:
         *      intercept "OK"/"Cancel" buttons
         */

        case WM_COMMAND:
            switch ((ULONG)mp1)
            {
                case DID_OK:
                {
                    ULONG ulSelection = DID_CANCEL,
                          ulLastFocusID = 0;
                    CHAR szLastFocusID[20] = "";
                    if (winhIsDlgItemChecked(hwndDlg, ID_XFDI_CLASH_RENAMENEW))
                    {
                        ulSelection = ID_XFDI_CLASH_RENAMENEW;
                        ulLastFocusID = ID_XFDI_CLASH_RENAMENEWTXT;
                    }
                    else if (winhIsDlgItemChecked(hwndDlg, ID_XFDI_CLASH_RENAMEOLD))
                    {
                        ulSelection = ID_XFDI_CLASH_RENAMEOLD;
                        ulLastFocusID = ID_XFDI_CLASH_RENAMEOLDTXT;
                    }
                    else
                    {
                        ulSelection = ID_XFDI_CLASH_REPLACE;
                        ulLastFocusID = ID_XFDI_CLASH_REPLACE;
                    }

                    // store focus
                    sprintf(szLastFocusID, "%d",
                            ulLastFocusID);
                    PrfWriteProfileString(HINI_USER,
                                          (PSZ)INIAPP_XWORKPLACE,
                                          (PSZ)INIKEY_NAMECLASHFOCUS,
                                          szLastFocusID);
                    // store window pos
                    winhSaveWindowPos(hwndDlg,
                                HINI_USER,
                                INIAPP_XWORKPLACE, INIKEY_WNDPOSNAMECLASH);
                    WinDismissDlg(hwndDlg, ulSelection);
                }
                break;

                case DID_CANCEL:
                    WinDismissDlg(hwndDlg, DID_CANCEL);
                break;
            }
        break;

        case WM_HELP:
            cmnDisplayHelp(NULL,    // active Desktop
                           ID_XFH_TITLECLASH);
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ fopsFindObjectWithSameTitle:
 *      checks pFolder's contents for an object with the
 *      specified title.
 *
 *      If such a thing is found, it is returned and the
 *      real name is written into pszName2Check.
 *
 *      If pIgnore != NULL, that object is ignored during
 *      the check, which is useful for "rename".
 *
 *      Used by fopsConfirmObjectTitle.
 *
 *@@added V0.9.16 (2001-12-31) [umoeller]
 */

WPFileSystem* fopsFindObjectWithSameTitle(WPFolder *pFolder,    // in: folder to check
                                          PSZ pszFind,          // in: title to check for
                                          WPObject *pIgnore)    // in: object to ignore or NULL
{
    WPFileSystem *pFSReturn = NULL;
    BOOL    fFolderLocked = FALSE;

    if (!pszFind)
        return NULL;

    PMPF_TITLECLASH(("checking %s for %s",
            _wpQueryTitle(pFolder),
            pszFind));

    TRY_LOUD(excpt1)
    {
        WPObject    *pobj = 0;

        if (fdrCheckIfPopulated(pFolder, FALSE)) // V0.9.12 (2001-04-29) [umoeller]
        {
            if (fFolderLocked = !_wpRequestFolderMutexSem(pFolder, 5000))
            {
                for (   pobj = _wpQueryContent(pFolder, NULL, (ULONG)QC_FIRST);
                        (pobj);
                        pobj = *__get_pobjNext(pobj)
                    )
                {
                    PSZ pszThis;
                    if (    (pIgnore)
                         && (pobj == pIgnore)
                       )
                    {
                        PMPF_TITLECLASH(("    found, but is self, so continue"));
                        continue;
                    }

                    // V1.0.3 (2004-06-19) [pr]: @@fixes 563
                    if (    _somIsA(pobj, _WPFileSystem)
                         && (pszThis = _wpQueryTitle(pobj))
                         && (!stricmp(pszThis, pszFind))
                       )
                    {
                        PMPF_TITLECLASH(("    returning 0x%lX", pobj));
                        pFSReturn = pobj;
                        break;
                    }
                }
            }
        }
    }
    CATCH(excpt1)
    {
        pFSReturn = NULL;
    } END_CATCH();

    if (fFolderLocked)
        _wpReleaseFolderMutexSem(pFolder);

    return pFSReturn;
}

/*
 *@@ fopsProposeNewTitle:
 *      proposes a new title for the object named
 *      pszTitle in the given folder. We attempt
 *      to find a title with a decimal number
 *      between the filestem and the extension which
 *      doesn't exist yet.
 *
 *      The proposed title is written into pszProposeTitle,
 *      which should be CCHMAXPATH in size.
 *
 *      For example, if you pass "demo.txt" and
 *      "demo.txt" and "demo:1.txt" already exist
 *      in pFolder, this would return "demo:2.txt".
 *
 *      Returns FALSE upon errors, e.g. if the internal
 *      exception handler crashed or a folder semaphore
 *      could not be requested within five seconds.
 *
 *      Used by fopsConfirmObjectTitle.
 *
 *@@added V0.9.1 (2000-01-30) [umoeller]
 *@@changed V0.9.1 (2000-01-08) [umoeller]: added mutex protection for folder queries
 *@@changed V0.9.3 (2000-04-28) [umoeller]: now pre-resolving wpQueryContent for speed
 *@@changed V0.9.9 (2001-03-27) [umoeller]: fixed hangs with .hidden filenames
 *@@changed V0.9.16 (2001-12-31) [umoeller]: removed duplicate code
 */

BOOL fopsProposeNewTitle(const char *pcszTitle,          // in: title to modify
                         WPFolder *pFolder,     // in: folder to check for existing titles
                         PSZ pszProposeTitle)   // out: buffer for new title
{
    CHAR    szFilenameWithoutCount[500];
    PSZ     p = 0,
            p2 = 0;
    BOOL    fFileExists = FALSE,
            fExit = FALSE;
    LONG    lFileCount = -1;

    BOOL    brc = TRUE;

    // propose new name; we will use the title and
    // add a ":num" before the extension and then
    // transform that one into a real name and keep
    // increasing "num" until no file with that
    // real name exists (as the WPS does it too)
    strlcpy(szFilenameWithoutCount, pcszTitle, sizeof(szFilenameWithoutCount));

    // check if title contains a ':num' already
    if (p2 = strchr(szFilenameWithoutCount, ':'))
    {
        // if we have a valid number following ":", remove it
        if (lFileCount = atoi(p2 + 1))
        {
            if (p = strchr(p2, '.'))
                strcpy(p2, p);
            else
                *p2 = '\0';
        }
        else
            *p2 = '\0';     // V0.9.9 (2001-03-27) [umoeller]
    }

    // insert new number
    p = strrchr(szFilenameWithoutCount + 1, '.');       // last dot == extension
                // fixed V0.9.9 (2001-03-27) [umoeller]: start with
                // second char to handle ".unixhidden" files
    lFileCount = 1;
    do
    {
        BOOL            fFolderLocked = FALSE;
        CHAR            szFileCount[20];

        fExit = FALSE;

        sprintf(szFileCount, ":%d", lFileCount);

        if (p)
        {
            // has extension: insert
            ULONG ulBeforeDot = (p - szFilenameWithoutCount);
            // we don't care about FAT, because
            // we propose titles, not real names
            // (V0.84)
            strncpy(pszProposeTitle, szFilenameWithoutCount, ulBeforeDot);
            pszProposeTitle[ulBeforeDot] = '\0';
            strcat(pszProposeTitle, szFileCount);
            strcat(pszProposeTitle, p);
        }
        else
        {
            // has no extension: append
            strncpy(pszProposeTitle, szFilenameWithoutCount, 200);
            // we don't care about FAT, because
            // we propose titles, not real names
            // (V0.84)
            strcat(pszProposeTitle, szFileCount);
        }

        // check if the file exists using this routine
        // V0.9.16 (2001-12-31) [umoeller]
        fFileExists = (fopsFindObjectWithSameTitle(pFolder,
                                                   pszProposeTitle,
                                                   NULL)
                        != NULL);

        lFileCount++;

        if (lFileCount > 99)
            // avoid endless loops
            break;

    } while (   (fFileExists)
              && (!fExit)
            );

    return brc;
}

#define CX_COL1             60
#define CX_COLDATE          40
#define CX_COLTIME          40
#define CX_COLSIZE          50

#define CX_INTRO   (CX_COL1 + CX_COLDATE + CX_COLTIME + CX_COLSIZE + 6 * COMMON_SPACING)

static CONTROLDEF
    ClashIntro = CONTROLDEF_TEXT_WORDBREAK(NULL, ID_XFDI_CLASH_TXT1, -100),
    ClashOldObject = CONTROLDEF_TEXT(LOAD_STRING, ID_XFDI_CLASH_OLDOBJECT, CX_COL1, SZL_AUTOSIZE),
    ClashDateOld = CONTROLDEF_TEXT("99.99.9999 ", ID_XFDI_CLASH_DATEOLD, SZL_AUTOSIZE, SZL_AUTOSIZE),
    ClashTimeOld = CONTROLDEF_TEXT("99:99:99 ", ID_XFDI_CLASH_TIMEOLD, SZL_AUTOSIZE, SZL_AUTOSIZE),
    ClashSizeOld = CONTROLDEF_TEXT_RIGHT(NULL, ID_XFDI_CLASH_SIZEOLD, CX_COLSIZE, SZL_AUTOSIZE),
    ClashNewObject = CONTROLDEF_TEXT(LOAD_STRING, ID_XFDI_CLASH_NEWOBJECT, CX_COL1, SZL_AUTOSIZE),
    ClashDateNew = CONTROLDEF_TEXT("99.99.9999 ", ID_XFDI_CLASH_DATENEW, SZL_AUTOSIZE, SZL_AUTOSIZE),
    ClashTimeNew = CONTROLDEF_TEXT("99:99:99 ", ID_XFDI_CLASH_TIMENEW, SZL_AUTOSIZE, SZL_AUTOSIZE),
    ClashSizeNew = CONTROLDEF_TEXT_RIGHT(NULL, ID_XFDI_CLASH_SIZENEW, CX_COLSIZE, SZL_AUTOSIZE),

    ClashReplaceRadio = CONTROLDEF_FIRST_AUTORADIO(LOAD_STRING, ID_XFDI_CLASH_REPLACE, -60, SZL_AUTOSIZE),
    ClashRenameNewRadio = CONTROLDEF_NEXT_AUTORADIO(LOAD_STRING, ID_XFDI_CLASH_RENAMENEW, -60, SZL_AUTOSIZE),
    ClashRenameNewEF = CONTROLDEF_ENTRYFIELD("M", ID_XFDI_CLASH_RENAMENEWTXT, -40, SZL_AUTOSIZE),
    ClashRenameOldRadio = CONTROLDEF_NEXT_AUTORADIO(LOAD_STRING, ID_XFDI_CLASH_RENAMEOLD, -60, SZL_AUTOSIZE),
    ClashRenameOldEF = CONTROLDEF_ENTRYFIELD("M", ID_XFDI_CLASH_RENAMEOLDTXT, -40, SZL_AUTOSIZE);

static const DLGHITEM G_dlgFileExists[] =
    {
        START_TABLE,
            START_ROW(0),
                CONTROL_DEF(&ClashIntro),
            START_ROW(0),
                CONTROL_DEF(&ClashOldObject),
                CONTROL_DEF(&ClashDateOld),
                CONTROL_DEF(&ClashTimeOld),
                CONTROL_DEF(&ClashSizeOld),
            START_ROW(0),
                CONTROL_DEF(&ClashNewObject),
                CONTROL_DEF(&ClashDateNew),
                CONTROL_DEF(&ClashTimeNew),
                CONTROL_DEF(&ClashSizeNew),
            START_ROW(0),
                CONTROL_DEF(&ClashReplaceRadio),
            START_ROW(0),
                CONTROL_DEF(&ClashRenameNewRadio),
                CONTROL_DEF(&ClashRenameNewEF),
            START_ROW(0),
                CONTROL_DEF(&ClashRenameOldRadio),
                CONTROL_DEF(&ClashRenameOldEF),
            START_ROW(0),
                CONTROL_DEF(&G_OKButton),
                CONTROL_DEF(&G_CancelButton),
                CONTROL_DEF(&G_HelpButton),
        END_TABLE
    };

/*
 *@@ PrepareFileExistsDlg:
 *      prepares the "file exists" dialog by setting
 *      up all the controls.
 *
 *      Note that with V0.9.16, we now pass in flOptions
 *      which _must_ have the NO_NAMECLASH_* flags already
 *      set to disable controls properly.
 *
 *@@added V0.9.3 (2000-05-03) [umoeller]
 *@@changed V0.9.16 (2001-12-31) [umoeller]: largely rewritten
 *@@changed V0.9.20 (2002-08-08) [umoeller]: now using dialog formatter
 *@@changed V1.0.1 (2002-11-30) [umoeller]: fixed radio box selection problem @@fixes 229
 *@@changed V1.0.9 (2010-07-17) [pr]: added large file support @@fixes 586
 */

STATIC HWND PrepareFileExistsDlg(WPObject *somSelf,
                                 WPFolder *Folder,
                                 WPObject *pExisting,
                                 PSZ pszTitle,
                                 ULONG flOptions)
{
    HWND        hwndConfirm = NULLHANDLE;       // V1.0.0 (2002-08-18) [umoeller]

    // load confirmation dialog
    // V0.9.20 (2002-08-08) [umoeller]

    APIRET      arc;
    PSZ         pszIntro;
    CHAR        szTemp[CCHMAXPATH];

    // replace placeholders in introductory strings
    if (pszIntro = strdup(cmnGetString(ID_XFDI_CLASH_TXT1)))
    {
        PCSZ    pcszBytes = cmnGetString(ID_XSSI_BYTES);

        ULONG   ulOfs;
        ulOfs = 0;
        strhFindReplace(&pszIntro, &ulOfs, "%1", _wpQueryTitle(pExisting));
        _wpQueryFilename(Folder, szTemp, TRUE);
        ulOfs = 0;
        strhFindReplace(&pszIntro, &ulOfs, "%2", szTemp);

        ClashIntro.pcszText = pszIntro;

        sprintf(szTemp, "9.999.999.999%s", pcszBytes);
        ClashSizeOld.pcszText = szTemp;
        ClashSizeNew.pcszText = szTemp;

        if (!(arc = dlghCreateDlg(&hwndConfirm,
                                  NULLHANDLE,
                                  FCF_FIXED_DLG,
                                  fnwpTitleClashDlg,
                                  cmnGetString(ID_XFD_TITLECLASH),
                                  G_dlgFileExists,
                                  ARRAYITEMCOUNT(G_dlgFileExists), // same as before
                                  NULL,
                                  cmnQueryDefaultFont())))
        {
            CHAR    szProposeTitle[CCHMAXPATH] = "????",
                    szExistingFilename[CCHMAXPATH];
            ULONG   ulLastFocusID = 0,
                    ulLastDot = 0;
            PSZ     pTemp;

            // prepare file date/time etc. for
            // display in window
            FILESTATUS3L        fs3;
            PCOUNTRYSETTINGS2   pcs = cmnQueryCountrySettings(FALSE);

            // disable window updates for the following changes
            WinEnableWindowUpdate(hwndConfirm, FALSE);

            // set object information fields
            if (_somIsA(pExisting, _WPFileSystem))
            {
                _wpQueryFilename(pExisting, szExistingFilename, TRUE);
                DosQueryPathInfo(szExistingFilename,
                                 FIL_STANDARDL,
                                 &fs3, sizeof(fs3));
                nlsFileDate(szTemp,
                            &fs3.fdateLastWrite,
                            pcs);
                WinSetDlgItemText(hwndConfirm, ID_XFDI_CLASH_DATEOLD, szTemp);
                nlsFileTime(szTemp,
                            &fs3.ftimeLastWrite,
                            pcs);
                WinSetDlgItemText(hwndConfirm, ID_XFDI_CLASH_TIMEOLD, szTemp);
                // V1.0.9
                nlsThousandsDouble(szTemp,
                                   65536.0 * 65536.0 * fs3.cbFile.ulHi + fs3.cbFile.ulLo,
                                   pcs->cs.cThousands);
                strcat(szTemp, pcszBytes);
                WinSetDlgItemText(hwndConfirm, ID_XFDI_CLASH_SIZEOLD, szTemp);
            }
            else
            {
                WinSetDlgItemText(hwndConfirm, ID_XFDI_CLASH_DATEOLD, "");
                WinSetDlgItemText(hwndConfirm, ID_XFDI_CLASH_TIMEOLD, "");
                WinSetDlgItemText(hwndConfirm, ID_XFDI_CLASH_SIZEOLD, "0");
            }

            // if we're not copying within the same folder,
            // i.e. if the two objects are different,
            // give info on ourselves too
            if (    (pExisting != somSelf)
                 && (_somIsA(somSelf, _WPFileSystem))
               )
            {
                CHAR szSelfFilename[CCHMAXPATH];
                _wpQueryFilename(somSelf, szSelfFilename, TRUE);
                DosQueryPathInfo(szSelfFilename,
                                 FIL_STANDARDL,
                                 &fs3, sizeof(fs3));
                nlsFileDate(szTemp,
                             &fs3.fdateLastWrite,
                             pcs);
                WinSetDlgItemText(hwndConfirm, ID_XFDI_CLASH_DATENEW, szTemp);
                nlsFileTime(szTemp,
                            &fs3.ftimeLastWrite,
                            pcs);
                WinSetDlgItemText(hwndConfirm, ID_XFDI_CLASH_TIMENEW, szTemp);
                // V1.0.9
                nlsThousandsDouble(szTemp,
                                   65536.0 * 65536.0 * fs3.cbFile.ulHi + fs3.cbFile.ulLo,
                                   pcs->cs.cThousands);
                strcat(szTemp, pcszBytes);
                WinSetDlgItemText(hwndConfirm, ID_XFDI_CLASH_SIZENEW, szTemp);
            }
            else
            {
                // if we're copying within the same folder,
                // set the "new object" fields empty
                WinSetDlgItemText(hwndConfirm, ID_XFDI_CLASH_DATENEW, "");
                WinSetDlgItemText(hwndConfirm, ID_XFDI_CLASH_TIMENEW, "");
                WinSetDlgItemText(hwndConfirm, ID_XFDI_CLASH_SIZENEW, "");
            }

            // get new title which doesn't exist in the folder yet
            fopsProposeNewTitle(pszTitle,
                                Folder,
                                szProposeTitle);

            // OK, we've found a new filename: set dlg items
            WinSendDlgItemMsg(hwndConfirm, ID_XFDI_CLASH_RENAMENEWTXT,
                              EM_SETTEXTLIMIT,
                              (MPARAM)(250), MPNULL);
            WinSetDlgItemText(hwndConfirm, ID_XFDI_CLASH_RENAMENEWTXT,
                              szProposeTitle);
            WinSendDlgItemMsg(hwndConfirm, ID_XFDI_CLASH_RENAMEOLDTXT,
                              EM_SETTEXTLIMIT,
                              (MPARAM)(250), MPNULL);
            WinSetDlgItemText(hwndConfirm, ID_XFDI_CLASH_RENAMEOLDTXT,
                              szProposeTitle);

            // select the first characters up to the extension
            // in the edit field
            if (pTemp = strrchr(szProposeTitle, '.'))       // last dot == extension
                ulLastDot = (pTemp - szProposeTitle);
            else
                ulLastDot = 300;                    // too large == select all

            WinSendDlgItemMsg(hwndConfirm, ID_XFDI_CLASH_RENAMENEWTXT,
                              EM_SETSEL,
                              MPFROM2SHORT(0, ulLastDot),
                              MPNULL);
            WinSendDlgItemMsg(hwndConfirm, ID_XFDI_CLASH_RENAMEOLDTXT,
                              EM_SETSEL,
                              MPFROM2SHORT(0, ulLastDot),
                              MPNULL);

            // find the selection the user has made last time;
            // this INI key item is maintained by fnwpTitleClashDlg above
            ulLastFocusID = PrfQueryProfileInt(HINI_USER,
                                               (PSZ)INIAPP_XWORKPLACE,
                                               (PSZ)INIKEY_NAMECLASHFOCUS,
                                               ID_XFDI_CLASH_RENAMENEWTXT);
                                                    // default value if not set

            if (flOptions & NO_NAMECLASH_RENAMEOLD)
            {
                WinEnableControl(hwndConfirm, ID_XFDI_CLASH_RENAMEOLD, FALSE);
                WinEnableControl(hwndConfirm, ID_XFDI_CLASH_RENAMEOLDTXT, FALSE);
                // if the last focus is one of the disabled items,
                // change it
                if (ulLastFocusID == ID_XFDI_CLASH_RENAMEOLDTXT)
                    ulLastFocusID = ID_XFDI_CLASH_RENAMENEWTXT;
            }

            if (flOptions & NO_NAMECLASH_REPLACE)
            {
                // disable "Replace" for "Rename" mode; this is
                // not allowed
                WinEnableControl(hwndConfirm, ID_XFDI_CLASH_REPLACE, FALSE);
                // if the last focus is one of the disabled items,
                // change it
                if (ulLastFocusID == ID_XFDI_CLASH_REPLACE)
                    ulLastFocusID = ID_XFDI_CLASH_RENAMENEWTXT;
            }

            // set focus to that item
            winhSetDlgItemFocus(hwndConfirm, ulLastFocusID);
                // this will automatically select the corresponding
                // radio button, see fnwpTitleClashDlg above
                // V1.0.1 (2002-11-30) [umoeller] no not really,
                // so force checking this
            winhSetDlgItemChecked(hwndConfirm, ulLastFocusID, TRUE);

            // *** go!
            winhRestoreWindowPos(hwndConfirm,
                                 HINI_USER,
                                 INIAPP_XWORKPLACE, INIKEY_WNDPOSNAMECLASH,
                                 SWP_MOVE | SWP_SHOW | SWP_ACTIVATE);
                                        // move only, no resize
        }

        free(pszIntro);
    }

    return hwndConfirm;
}

/*
 *@@ ConfirmObjectTitle:
 *      worker for fopsConfirmObjectTitle.
 *
 *      This is mostly a rewrite of fopsConfirmObjectTitle
 *      for V0.9.16 which fixes most of the problems we
 *      used to have with "create another", "create from
 *      template" and the like. Besides we used to handle
 *      only file-system objects, which wasn't entirely
 *      correct, and we had lots of special-casing for
 *      FAT drives from the old XFolder code, which wasn't
 *      needed either and caused hangs at times.
 *
 *      There's a few special situations to be considered:
 *
 *      --  When an object is copied within the same folder,
 *          we must perform additional checks in order not
 *          to allow "rename old" and "replace".
 *
 *      --  When an object is renamed, pszTitle has the title
 *          of the current object, and flOptions has been set
 *          explicitly by fopsConfirmObjectTitle so that we
 *          won't get a confirmation for *ppExistingObject, but
 *          only for another object with the same title.
 *
 *      --  For "Create from template", pszTitle has the template
 *          title, and flOptions has NO_NAMECLASH_DIALOG set.
 *
 *      --  For "Create another", pszTitle has the template
 *          title.
 *
 *@@added V0.9.16 (2001-12-31) [umoeller]
 *@@changed V0.9.16 (2002-01-01) [umoeller]: added auto-rename for trash can
 *@@changed V0.9.20 (2002-08-08) [umoeller]: this never respected the auto-rename etc. "Title" settings in "Workplcae Shell", fixed
 */

STATIC ULONG ConfirmObjectTitle(WPFolder *Folder,          // in: target folder to check
                                WPObject **ppExistingObject, // in: current object,
                                                             // out: object to append to or to replace
                                PSZ pszTitle,              // in: title to check for,
                                                           // out: new proposed title for object
                                ULONG cbTitle,
                                ULONG flOptions)
{
    ULONG           ulrc = NAMECLASH_NONE;

    WPFileSystem    *pFSExisting = NULL;
    CHAR            szTemp[CCHMAXPATH];

    _wpQueryFilename(Folder, szTemp, TRUE);
    PMPF_TITLECLASH(("entering for folder %s, pszTitle %s",
                szTemp, pszTitle));

    TRY_LOUD(excpt1)
    {
        // check if an object with the same title
        // exists in the folder; note, pszTitle depends
        // on the context... for copy it it will be the
        // source title, but for "create another" et al,
        // it will be the template's title
        if (pFSExisting = fopsFindObjectWithSameTitle(Folder,
                                                      pszTitle,
                                                      // object to ignore:
                                                      (flOptions & NAMECLASH_RENAMING)
                                                            ? *ppExistingObject
                                                            : NULL))
        {
            // another object with the same title exists:
            // then we probably need to display the dialog...

            ulrc = NAMECLASH_PROMPT;

            // if the existing object has "deletion" set, then it is
            // somewhere in the hidden \trash directories, so do an
            // auto-rename in that case
            if (_xwpQueryDeletion(pFSExisting, NULL, NULL))
                ulrc = NAMECLASH_RENAME;
            else
            {
                // not in trash:
                // check the "System" ("Workplace Shell") setting for
                // what user specified in case of title clashs
                // V0.9.20 (2002-08-08) [umoeller]

                CHAR    szOpt[20];

                if (PrfQueryProfileString(HINI_USER,
                                          "PM_ControlPanel",
                                          "NameClash",
                                          NULL,
                                          szOpt,
                                          sizeof(szOpt)))
                    ulrc = atoi(szOpt);

                PMPF_TITLECLASH(("_xwpQueryDeletion(pFSExisting) returned FALSE, ulrc is %d (%s)",
                            ulrc,
                            (ulrc == NAMECLASH_PROMPT) ? "NAMECLASH_PROMPT"
                            : (ulrc == NAMECLASH_RENAME) ? "NAMECLASH_RENAME"
                            : (ulrc == NAMECLASH_REPLACE) ? "NAMECLASH_REPLACE"
                            : "unknown value?!?"
                     ));
            }

            // disable "Replace" and "Rename old"
            // if we're copying within the same folder
            // or if we're copying a folder to its parent
            if (    (pFSExisting == *ppExistingObject)
                 || (pFSExisting == _wpQueryFolder(*ppExistingObject))
               )
            {
                flOptions |= NO_NAMECLASH_REPLACE | NO_NAMECLASH_RENAMEOLD;
            }

            PMPF_TITLECLASH(("  options: %s%s%s%s%s",
                        (flOptions & NO_NAMECLASH_RENAME) ? "NO_NAMECLASH_RENAME " : "",
                        (flOptions & NO_NAMECLASH_RENAMEOLD) ? "NO_NAMECLASH_RENAMEOLD " : "",
                        (flOptions & NO_NAMECLASH_APPEND) ? "NO_NAMECLASH_APPEND " : "",
                        (flOptions & NO_NAMECLASH_REPLACE) ? "NO_NAMECLASH_REPLACE " : "",
                        (flOptions & NO_NAMECLASH_DIALOG) ? "NO_NAMECLASH_DIALOG " : "" ));

            // FINALLY check if we are to display the dialog...

            if (    (ulrc == NAMECLASH_PROMPT)
                 && (flOptions & NO_NAMECLASH_DIALOG)
               )
            {
                // default action is to prompt, but in this case,
                // the caller does NOT want a confirmation dialog:
                // then do auto-rename
                ulrc = NAMECLASH_RENAME;
            }

            if (    (ulrc == NAMECLASH_RENAME)
                 && (flOptions & NO_NAMECLASH_RENAME)
               )
                // user has specified "auto rename", but rename is not allowed:
                ulrc = NAMECLASH_PROMPT;
            else if (    (ulrc == NAMECLASH_REPLACE)
                      && (flOptions & NO_NAMECLASH_REPLACE)
                    )
                // user has specified "auto replace", but replace is not allowed:
                ulrc = NAMECLASH_PROMPT;

            if (ulrc == NAMECLASH_RENAME)
            {
                // auto-rename was enabled, either by user on "Title"
                // page or above because NO_NAMECLASH_DIALOG was specified:
                // then produce a new name automatically
                fopsProposeNewTitle(pszTitle,
                                    Folder,
                                    szTemp);
                strhncpy0(pszTitle,
                          szTemp,
                          cbTitle);
                PMPF_TITLECLASH(("  performed auto-rename to %s", pszTitle));
            }
            else if (ulrc == NAMECLASH_PROMPT)
            {
                // alright, show the damn dialog
                HWND hwndConfirm = PrepareFileExistsDlg(*ppExistingObject,
                                                        Folder,
                                                        pFSExisting,
                                                        pszTitle,
                                                        flOptions);

                // go!
                ULONG ulDlgReturn = WinProcessDlg(hwndConfirm);

                // check return value
                switch (ulDlgReturn)
                {
                    case ID_XFDI_CLASH_RENAMENEW:
                        // rename new: copy user's entry to buffer
                        // provided by the method
                        WinQueryDlgItemText(hwndConfirm,
                                            ID_XFDI_CLASH_RENAMENEWTXT,
                                            cbTitle - 1,
                                            pszTitle);
                        ulrc = NAMECLASH_RENAME;
                    break;

                    case ID_XFDI_CLASH_RENAMEOLD:
                        // rename old: use wpSetTitle on existing object
                        WinQueryDlgItemText(hwndConfirm,
                                            ID_XFDI_CLASH_RENAMEOLDTXT,
                                            sizeof(szTemp) - 1,
                                            szTemp);
                        _wpSetTitleAndRenameFile(pFSExisting, szTemp, 0);

                        ulrc = NAMECLASH_NONE;
                    break;

                    case ID_XFDI_CLASH_REPLACE:
                        *ppExistingObject = pFSExisting;
                        ulrc = NAMECLASH_REPLACE;
                    break;

                    default: // case DID_CANCEL:
                        ulrc = NAMECLASH_CANCEL;
                    break;
                }

                winhDestroyWindow(&hwndConfirm);
            }
        }
    }
    CATCH(excpt1)
    {
        PMPF_TITLECLASH(("crash"));
        ulrc = NAMECLASH_CANCEL;
    }
    END_CATCH();

    return ulrc;
}

/*
 *@@ fopsConfirmObjectTitle:
 *      this gets called from XFldObject::wpConfirmObjectTitle
 *      to actually implement the "File exists" dialog, if the
 *      replacement has been enabled.
 *
 *      This performs a number of  checks and loads a dialog from
 *      the NLS resources, if necessary, using fnwpTitleClashDlg.
 *
 *      See XFldObject::wpConfirmObjectTitle for a detailed
 *      description of the highly confusing parameters.
 *
 *@@changed V0.9.1 (2000-01-30) [umoeller]: extracted some functions for clarity; this was a total mess
 *@@changed V0.9.3 (2000-04-08) [umoeller]: fixed "Create another" problem
 *@@changed V0.9.3 (2000-05-03) [umoeller]: extracted ProcessFileExistsDlg
 *@@changed V0.9.4 (2000-07-15) [umoeller]: fixed "Create another" problem for good
 *@@changed V0.9.16 (2001-12-31) [umoeller]: largely rewritten to finally fix the various special-case problems
 *@@changed V0.9.19 (2002-04-02) [umoeller]: fixed prompt for "Create shadow" which should never appear
 */

ULONG fopsConfirmObjectTitle(WPObject *somSelf,
                             WPFolder *Folder,
                             WPObject **ppDuplicate,
                             PSZ pszTitle,
                             ULONG cbTitle,
                             ULONG menuID)
{
    ULONG   ulrc = NAMECLASH_NONE;
    ULONG   flOptions = 0;

    // these flags are returned by WPShadow::wpQueryNameClashOptions;
    // for shadows we thus do nothing at all
    #define NOTHING_TO_DO (NO_NAMECLASH_RENAME | NO_NAMECLASH_APPEND | NO_NAMECLASH_REPLACE)

    TRY_LOUD(excpt1)
    {

        PMPF_TITLECLASH(("menuID is 0x%lX", menuID));

        if (    (!Folder)
             || (!_somIsA(Folder, _WPFolder))
             || (!pszTitle)
             || (!strlen(pszTitle))
             || (!cbTitle)
           )
        {
            PMPF_TITLECLASH(("   error, returning NAMECLASH_CANCEL"));
            ulrc = NAMECLASH_CANCEL;
        }
        else switch (menuID)
        {
            case WPMENUID_CREATESHADOW:
                PMPF_TITLECLASH(("   WPMENUID_CREATESHADOW"));
                // return NAMECLASH_NONE always
                flOptions = NOTHING_TO_DO;      // V0.9.19 (2002-04-02) [umoeller]
            break;

            case 110:           // comes in when objects are renamed
                PMPF_TITLECLASH(("   rename"));
                flOptions =    NAMECLASH_RENAMING
                             | NO_NAMECLASH_REPLACE
                             | NO_NAMECLASH_APPEND;
            break;

            case 183:           // comes in for "create from template"
                PMPF_TITLECLASH(("   create from template"));
                flOptions = NO_NAMECLASH_DIALOG;
            break;

            default:
                PMPF_TITLECLASH(("   calling _wpQueryNameClashOptions"));
                flOptions = _wpQueryNameClashOptions(somSelf, menuID);
            break;
        }

        if ((flOptions & NOTHING_TO_DO) != NOTHING_TO_DO)
        {
            // first set the duplicate
            *ppDuplicate = somSelf;

            // then go do some work
            ulrc = ConfirmObjectTitle(Folder,
                                      ppDuplicate,
                                      pszTitle,
                                      cbTitle,
                                      flOptions);

            PMPF_TITLECLASH(("   ConfirmObjectTitle returned %s",
                        (ulrc == NAMECLASH_CANCEL) ? "NAMECLASH_CANCEL"
                        : (ulrc == NAMECLASH_NONE) ? "NAMECLASH_NONE"
                        : (ulrc == NAMECLASH_RENAME) ? "NAMECLASH_RENAME"
                        : (ulrc == 0x04) ? "NAMECLASH_APPEND"
                        : (ulrc == NAMECLASH_REPLACE) ? "NAMECLASH_REPLACE"
                        : (ulrc == 0x20) ? "NAMECLASH_RENAME_KEEPASSOCS"
                        : (ulrc == 0x30) ? "NAMECLASH_NONE_KEEPASSOCS"
                        : "unknown"));

        }
    }
    CATCH(excpt1)
    {
        PMPF_TITLECLASH(("crash"));
        ulrc = NAMECLASH_CANCEL;
    } END_CATCH();

    return ulrc;
}

/*
 *@@ fopsMoveObjectConfirmed:
 *      moves an object to the specified target folder.
 *
 *      This calls _wpConfirmObjectTitle before to
 *      check whether that object already exists in the
 *      target folder. If "Replace file exists" has been
 *      enabled, this will call the XWorkplace replacement
 *      method (XFldObject::wpConfirmObjectTitle).
 *      This handles the return codes correctly.
 *
 *      Returns TRUE if the object has been moved.
 *
 *@@added V0.9.2 (2000-03-04) [umoeller]
 *@@changed V0.9.3 (2000-04-28) [umoeller]: "replace" destroyed pObject; fixed
 *@@changed V0.9.19 (2002-04-02) [umoeller]: fixed crash with trash can
 */

BOOL fopsMoveObjectConfirmed(WPObject *pObject,
                             WPFolder *pTargetFolder)
{
    // check if object exists in that folder already
    // (this might call the XFldObject replacement)
    WPObject    *pReplaceThis = pObject; // NULL; fixed V0.9.19 (2002-04-02) [umoeller]
    CHAR        szNewTitle[CCHMAXPATH] = "";
    BOOL        fDoMove = TRUE,
                fDidMove = FALSE;
    ULONG       ulAction;

    strlcpy(szNewTitle, _wpQueryTitle(pObject), sizeof(szNewTitle));
    ulAction = _wpConfirmObjectTitle(pObject,      // object
                                     pTargetFolder, // folder
                                     &pReplaceThis,  // object to replace (if NAMECLASH_REPLACE)
                                     szNewTitle,     // in/out: object title
                                     sizeof(szNewTitle),
                                     WPMENUID_MOVE);

    PMPF_TITLECLASH(("    _wpConfirmObjectTitle returned %d", ulAction));

    switch (ulAction)
    {
        case NAMECLASH_CANCEL:
            fDoMove = FALSE;
        break;

        case NAMECLASH_RENAME:
            _wpSetTitle(pObject,
                        szNewTitle);      // set by wpConfirmObjectTitle
        break;

        case NAMECLASH_REPLACE:
            // delete the object to be replaced;
            // if this was successfull, move pObject there
            fDoMove = _wpFree(pReplaceThis);
            /* fDoMove = FALSE;
            fDidMove = _wpReplaceObject(pObject,
                                        pReplaceThis,       // set by wpConfirmObjectTitle
                                        TRUE);              // move and replace
               */
                    // after this pObject is deleted, so
                    // this cannot be used any more
        break;

        // NAMECLASH_NONE: just go on
    }

    if (fDoMove)
        fDidMove = _wpMoveObject(pObject, pTargetFolder);

    return fDidMove;
}

/*
 *@@ fopsRenameObjectConfirmed:
 *      renames an object.
 *
 *      This calls _wpConfirmObjectTitle before to
 *      check whether the title can be used.
 *      If "Replace file exists" has been enabled,
 *      this will call the XWorkplace replacement
 *      method (XFldObject::wpConfirmObjectTitle).
 *      This handles the return codes correctly.
 *
 *      Returns TRUE if the object has been renamed.
 *
 *@@added V0.9.19 (2002-06-18) [umoeller]
 */

BOOL fopsRenameObjectConfirmed(WPObject *pObject,
                               PCSZ pcszNewTitle)
{
    // check if object exists in that folder already
    // (this might call the XFldObject replacement)
    WPObject    *pReplaceThis = pObject;
    CHAR        szNewTitle[CCHMAXPATH];
    BOOL        fDoRename = TRUE,
                fDidRename = FALSE;
    ULONG       ulAction;

    strlcpy(szNewTitle, pcszNewTitle, sizeof(szNewTitle));
    ulAction = _wpConfirmObjectTitle(pObject,      // object
                                     _wpQueryFolder(pObject), // folder
                                     &pReplaceThis,  // object to replace (if NAMECLASH_REPLACE)
                                     szNewTitle,     // in/out: object title
                                     sizeof(szNewTitle),
                                     110);          // rename code, not in toolkit

    PMPF_TITLECLASH(("    _wpConfirmObjectTitle returned %d", ulAction));

    switch (ulAction)
    {
        case NAMECLASH_CANCEL:
        case NAMECLASH_REPLACE:     // shouldn't happen
            fDoRename = FALSE;
        break;

        // case NAMECLASH_RENAME:
        // then we have
    }

    if (fDoRename)
        fDidRename = _wpSetTitle(pObject, szNewTitle);

    return fDidRename;
}


