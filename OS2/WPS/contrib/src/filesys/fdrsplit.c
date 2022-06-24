
/*
 *@@sourcefile fdrsplit.c:
 *      folder "split view" implementation.
 *
 *
 *@@added V1.0.0 (2002-08-21) [umoeller]
 *@@header "filesys\folder.h"
 */

/*
 *      Copyright (C) 2001-2012 Ulrich M”ller.
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
#define INCL_WINPOINTERS
#define INCL_WININPUT
#define INCL_WINMENUS
#define INCL_WINSTDCNR
#define INCL_WINSHELLDATA
#define INCL_WINSCROLLBARS
#define INCL_WINSYS
#define INCL_WINTIMER

#define INCL_GPIBITMAPS
#define INCL_GPIREGIONS
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\threads.h"            // thread helpers
#include "helpers\winh.h"               // PM helper routines

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"
#include "xfdisk.ih"
// #include "xfobj.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\classtest.h"           // some cheap funcs for WPS class checks
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\contentmenus.h"        // shared menu logic
#include "shared\cnrsort.h"             // container sort comparison functions
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

#include "filesys\folder.h"             // XFolder implementation
#include "filesys\fdrsubclass.h"        // folder subclassing engine
#include "filesys\fdrviews.h"           // common code for folder views
#include "filesys\fdrsplit.h"           // folder split views
#include "filesys\object.h"             // XFldObject implementation
#include "filesys\statbars.h"           // status bar translation logic

// other SOM headers
#pragma hdrstop                         // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Private declarations
 *
 ********************************************************************/

PCSZ    WC_SPLITCONTROLLER  = "XWPSplitController",
        WC_SPLITPOPULATE   = "XWPSplitPopulate";

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// PFNWP       G_pfnwpSplitFrameOrig = NULL;

LINKLIST    G_llOpenSplitViews;
                    // linked list with pointers to the SPLITVIEWDATA
                    // structs of all currently open split views
                    // V1.0.1 (2002-11-30) [umoeller]
HMTX        G_hmtxOpenSplitViews = NULLHANDLE;

/* ******************************************************************
 *
 *   Split Populate thread
 *
 ********************************************************************/

/*
 *@@ AddFirstChild:
 *      adds the first child record for precParent
 *      to the given container if precParent represents
 *      a folder that has subfolders.
 *
 *      This gets called for every record in the drives
 *      tree so we properly add the "+" expansion signs
 *      to each record without having to fully populate
 *      each folder. This is an imitation of the standard
 *      WPS behavior in Tree views.
 *
 *      Runs on the split populate thread (fntSplitPopulate).
 *
 *@@added V0.9.18 (2002-02-06) [umoeller]
 *@@changed V1.0.0 (2002-09-09) [umoeller]: removed linklist
 */

STATIC WPObject* AddFirstChild(WPFolder *pFolder,
                               PMINIRECORDCORE precParent,     // in: folder record to insert first child for
                               HWND hwndCnr)                   // in: cnr where precParent is inserted
{
    PMINIRECORDCORE     precFirstChild;
    WPFolder            *pFirstChildFolder = NULL;

    if (!(precFirstChild = (PMINIRECORDCORE)WinSendMsg(hwndCnr,
                                                       CM_QUERYRECORD,
                                                       (MPARAM)precParent,
                                                       MPFROM2SHORT(CMA_FIRSTCHILD,
                                                                    CMA_ITEMORDER))))
    {
        // we don't have a first child already:

        // check if we have a subfolder in the folder already
        BOOL    fFolderLocked = FALSE,
                fFindLocked = FALSE;

        PMPF_SPLITVIEW(("CM_QUERYRECORD returned NULL"));

        TRY_LOUD(excpt1)
        {
            // request the find sem to make sure we won't have a populate
            // on the other thread; otherwise we get duplicate objects here
            if (fFindLocked = !_wpRequestFindMutexSem(pFolder, SEM_INDEFINITE_WAIT))
            {
                WPObject    *pObject;

                if (fFolderLocked = !_wpRequestFolderMutexSem(pFolder, SEM_INDEFINITE_WAIT))
                {
                    for (   pObject = _wpQueryContent(pFolder, NULL, QC_FIRST);
                            pObject;
                            pObject = *__get_pobjNext(pObject))
                    {
                        if (fdrvIsInsertable(pObject,
                                             INSERT_FOLDERSONLY,
                                             NULL))
                        {
                            pFirstChildFolder = pObject;
                            break;
                        }
                    }

                    _wpReleaseFolderMutexSem(pFolder);
                    fFolderLocked = FALSE;
                }

                PMPF_SPLITVIEW(("pFirstChildFolder pop is 0x%lX", pFirstChildFolder));

                if (!pFirstChildFolder)
                {
                    // no folder awake in folder yet:
                    // do a quick DosFindFirst loop to find the
                    // first subfolder in here
                    HDIR          hdir = HDIR_CREATE;
                    FILEFINDBUF3  ffb3     = {0};
                    ULONG         ulFindCount    = 1;        // look for 1 file at a time
                    APIRET        arc            = NO_ERROR;

                    CHAR          szFolder[CCHMAXPATH],
                                  szSearchMask[CCHMAXPATH];

                    _wpQueryFilename(pFolder, szFolder, TRUE);
                    sprintf(szSearchMask, "%s\\*", szFolder);

                    PMPF_SPLITVIEW(("searching %s", szSearchMask));

                    ulFindCount = 1;
                    arc = DosFindFirst(szSearchMask,
                                       &hdir,
                                       MUST_HAVE_DIRECTORY | FILE_ARCHIVED | FILE_SYSTEM | FILE_READONLY,
                                             // but exclude hidden
                                       &ffb3,
                                       sizeof(ffb3),
                                       &ulFindCount,
                                       FIL_STANDARD);

                    while ((arc == NO_ERROR))
                    {
                        PMPF_SPLITVIEW(("got %s", ffb3.achName));

                        // do not use "." and ".."
                        if (    (strcmp(ffb3.achName, ".") != 0)
                             && (strcmp(ffb3.achName, "..") != 0)
                           )
                        {
                            // this is good:
                            CHAR szFolder2[CCHMAXPATH];
                            sprintf(szFolder2, "%s\\%s", szFolder, ffb3.achName);

                            PMPF_SPLITVIEW(("awaking %s", szFolder2));

                            pObject = _wpclsQueryFolder(_WPFolder,
                                                        szFolder2,
                                                        TRUE);
                            // exclude templates
                            if (fdrvIsInsertable(pObject,
                                                 INSERT_FOLDERSONLY,
                                                 NULL))
                            {
                                pFirstChildFolder = pObject;
                                break;
                            }
                        }

                        // search next file
                        ulFindCount = 1;
                        arc = DosFindNext(hdir,
                                         &ffb3,
                                         sizeof(ffb3),
                                         &ulFindCount);

                    } // end while (rc == NO_ERROR)

                    DosFindClose(hdir);
                }
            }
        }
        CATCH(excpt1)
        {
        } END_CATCH();

        if (fFolderLocked)
            _wpReleaseFolderMutexSem(pFolder);
        if (fFindLocked)
            _wpReleaseFindMutexSem(pFolder);

        if (pFirstChildFolder)
        {
            POINTL ptl = {0, 0};
            _wpCnrInsertObject(pFirstChildFolder,
                               hwndCnr,
                               &ptl,        // without this the func fails
                               precParent,
                               NULL);
        }
    }

    return pFirstChildFolder;
}

/*
 *@@ fnwpSplitPopulate:
 *      object window for populate thread.
 *
 *      Runs on the split populate thread (fntSplitPopulate).
 *
 *@@changed V1.0.0 (2002-11-23) [umoeller]: split view stopped working when populate failed, @@fixes 192
 */

STATIC MRESULT EXPENTRY fnwpSplitPopulate(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_CREATE:
            WinSetWindowPtr(hwnd, QWL_USER, mp1);       // PSPLITCONTROLLER
        break;

        /*
         *@@ FM2_POPULATE:
         *      posted by fnwpMainControl when
         *      FM_FILLFOLDER comes in to offload
         *      populate to this second thread.
         *
         *      After populate is done, we post the
         *      following back to fnwpMainControl:
         *
         *      --  FM_POPULATED_FILLTREE always so
         *          that the drives tree can get
         *          updated;
         *
         *      --  FM_POPULATED_SCROLLTO, if the
         *          FFL_SCROLLTO flag was set;
         *
         *      --  FM_POPULATED_FILLFILES, if the
         *          FFL_FOLDERSONLY flag was _not_
         *          set.
         *
         *      This processing is all new with V0.9.18
         *      to finally synchronize the populate with
         *      the main thread better.
         *
         *      Parameters:
         *
         *      --  PMINIRECORDCORE mp1
         *
         *      --  ULONG mp2: flags, as with FM_FILLFOLDER.
         *
         *@@added V0.9.18 (2002-02-06) [umoeller]
         */

        case FM2_POPULATE:
        {
            PSPLITCONTROLLER pctl = WinQueryWindowPtr(hwnd, QWL_USER);
            WPFolder        *pFolder;
            PMINIRECORDCORE prec = (PMINIRECORDCORE)mp1;
            ULONG           fl = (ULONG)mp2,
                            fl2 = FFL_POPULATEFAILED;
            BOOL            fFoldersOnly = ((fl & FFL_FOLDERSONLY) != 0);
            ULONG           ulPopulated = 0;

            // set wait pointer
            (pctl->cThreadsRunning)++;
            WinPostMsg(pctl->hwndMainControl,
                       FM_UPDATEPOINTER,
                       0,
                       0);

            if (pFolder = fdrvGetFSFromRecord(prec, TRUE))
            {
                PMPF_SPLITVIEW(("populating %s", _wpQueryTitle(pFolder)));

                if (ulPopulated = fdrCheckIfPopulated(pFolder,
                                                      fFoldersOnly))
                {
                    // ulPopulated is 1 if we really did populate,
                    // or 2 if the folder was already populated

                    fl2 = fl;
                    // only if the folder was newly populated, then
                    // we need to set the FFL_UNLOCK bit so we
                    // can unlock objects that either do not
                    // get inserted or when the container
                    // is cleared again
                    if (ulPopulated == 1)
                        fl2 |= FFL_UNLOCKOBJECTS;
                }
            }

            // refresh the files only if we are not
            // in folders-only mode
            if (!fFoldersOnly)
                WinPostMsg(pctl->hwndMainControl,
                           FM_POPULATED_FILLFILES,
                           (MPARAM)prec,
                           (MPARAM)fl2);

            // in any case, refresh the tree
            WinPostMsg(pctl->hwndMainControl,
                       FM_POPULATED_FILLTREE,
                       (MPARAM)prec,
                       (MPARAM)(fl | fl2));
                    // fnwpMainControl will check fl again and
                    // fire "add first child" msgs accordingly

            if (ulPopulated)
                if (fl & FFL_SCROLLTO)
                    WinPostMsg(pctl->hwndMainControl,
                               FM_POPULATED_SCROLLTO,
                               (MPARAM)prec,
                               0);

            // clear wait pointer
            (pctl->cThreadsRunning)--;
            WinPostMsg(pctl->hwndMainControl,
                       FM_UPDATEPOINTER,
                       0,
                       0);
        }
        break;

        /*
         *@@ FM2_ADDFIRSTCHILD_BEGIN:
         *      posted by InsertContents before the first
         *      FM2_ADDFIRSTCHILD_NEXT is posted so we
         *      can update the "wait" ptr accordingly.
         *
         *@@added V0.9.18 (2002-02-06) [umoeller]
         */

        case FM2_ADDFIRSTCHILD_BEGIN:
        {
            PSPLITCONTROLLER pctl = WinQueryWindowPtr(hwnd, QWL_USER);
            (pctl->cThreadsRunning)++;
            WinPostMsg(pctl->hwndMainControl,
                       FM_UPDATEPOINTER,
                       0, 0);
        }
        break;

        /*
         *@@ FM2_ADDFIRSTCHILD_NEXT:
         *      fired by InsertContents for every folder that
         *      is added to the drives tree.
         *
         *      Parameters:
         *
         *      --  WPFolder* mp1: folder to add first child for.
         *          This better be in the tree.
         *
         *@@added V0.9.18 (2002-02-06) [umoeller]
         */

        case FM2_ADDFIRSTCHILD_NEXT:
            if (mp1)
            {
                PSPLITCONTROLLER pctl = WinQueryWindowPtr(hwnd, QWL_USER);
                HWND                hwndCnr = pctl->cvTree.hwndCnr;
                WPFolder            *pFolder = (WPObject*)mp1;
                PMINIRECORDCORE     precParent = _wpQueryCoreRecord(pFolder);

                PMPF_SPLITVIEW(("CM_ADDFIRSTCHILD %s", _wpQueryTitle(mp1)));

                AddFirstChild(pFolder,
                              precParent,
                              hwndCnr);
            }
        break;

        /*
         *@@ FM2_ADDFIRSTCHILD_DONE:
         *      posted by InsertContents after the last
         *      FM2_ADDFIRSTCHILD_NEXT was posted so we
         *      can reset the "wait" ptr.
         *
         *@@added V0.9.18 (2002-02-06) [umoeller]
         */

        case FM2_ADDFIRSTCHILD_DONE:
        {
            PSPLITCONTROLLER pctl = WinQueryWindowPtr(hwnd, QWL_USER);
            (pctl->cThreadsRunning)--;
            WinPostMsg(pctl->hwndMainControl,
                       FM_UPDATEPOINTER,
                       0,
                       0);
        }
        break;

        default:
            mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ fntSplitPopulate:
 *      "split populate" thread. This creates an object window
 *      so that we can easily serialize the order in which
 *      folders are populate and such.
 *
 *      This is responsible for both populating folders _and_
 *      doing the "add first child" processing. This was all
 *      new with V0.9.18's file dialog and was my second attempt
 *      at getting the thread synchronization right, which
 *      turned out to work pretty well.
 *
 *      We _need_ a second thread for "add first child" too
 *      because even adding the first child can take quite a
 *      while. For example, if a folder has 1,000 files in it
 *      and the 999th is a directory, the file system has to
 *      scan the entire contents first.
 */

STATIC VOID _Optlink fntSplitPopulate(PTHREADINFO ptiMyself)
{
    TRY_LOUD(excpt1)
    {
        QMSG qmsg;
        PSPLITCONTROLLER pctl = (PSPLITCONTROLLER)ptiMyself->ulData;

        PMPF_SPLITVIEW(("thread starting"));

        WinRegisterClass(ptiMyself->hab,
                         (PSZ)WC_SPLITPOPULATE,
                         fnwpSplitPopulate,
                         0,
                         sizeof(PVOID));
        if (!(pctl->hwndSplitPopulate = winhCreateObjectWindow(WC_SPLITPOPULATE,
                                                              pctl)))
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "Cannot create split populate object window.");

        // thread 1 is waiting for obj window to be created
        DosPostEventSem(ptiMyself->hevRunning);

        while (WinGetMsg(ptiMyself->hab, &qmsg, NULLHANDLE, 0, 0))
            WinDispatchMsg(ptiMyself->hab, &qmsg);

        winhDestroyWindow(&pctl->hwndSplitPopulate);

        PMPF_SPLITVIEW(("thread ending"));

    }
    CATCH(excpt1) {} END_CATCH();

}

/*
 *@@ fdrSplitPopulate:
 *      posts FM2_POPULATE to fnwpSplitPopulate to
 *      populate the folder represented by the
 *      given MINIRECORDCORE according to fl.
 *
 *      fnwpSplitPopulate does the following:
 *
 *      1)  Before populating, raise pctl->cThreadsRunning
 *          and post FM_UPDATEPOINTER to hwndMainControl.
 *          to have it display the "wait" pointer.
 *
 *      2)  Run _wpPopulate on the folder represented
 *          by prec.
 *
 *      3)  Post FM_POPULATED_FILLTREE back to hwndMainControl
 *          in any case to fill the tree under prec with the
 *          subfolders that were found.
 *
 *      4)  If the FFL_SCROLLTO flag is set, post
 *          FM_POPULATED_SCROLLTO back to hwndMainControl
 *          so that the tree can be scrolled properly.
 *
 *      5)  If the FFL_FOLDERSONLY flag was _not_ set,
 *          post FM_POPULATED_FILLFILES to hwndMainControl
 *          so it can insert all objects into the files
 *          container.
 *
 *      6)  Decrement pctl->cThreadsRunning and post
 *          FM_UPDATEPOINTER again to reset the wait
 *          pointer.
 */

VOID fdrSplitPopulate(PSPLITCONTROLLER pctl,
                      PMINIRECORDCORE prec,     // in: record to populate (can be disk or shadow)
                      ULONG fl)
{
    WinPostMsg(pctl->hwndSplitPopulate,
               FM2_POPULATE,
               (MPARAM)prec,
               (MPARAM)fl);
}

#ifdef __DEBUG__
VOID DumpFlags(ULONG fl)
{
    PMPF_SPLITVIEW(("  fl %s %s %s %s",
                (fl & FFL_FOLDERSONLY) ? "FFL_FOLDERSONLY " : "",
                (fl & FFL_SCROLLTO) ? "FFL_SCROLLTO " : "",
                (fl & FFL_EXPAND) ? "FFL_EXPAND " : "",
                (fl & FFL_SETBACKGROUND) ? "FFL_SETBACKGROUND " : ""
              ));
}
#else
    #define DumpFlags(fl)
#endif

/*
 *@@ fdrPostFillFolder:
 *      posts FM_FILLFOLDER to the main control
 *      window with the given parameters.
 *
 *      This gets called from the tree frame
 *      to fire populate when things get
 *      selected in the tree.
 *
 *      The main control window will then call
 *      fdrSplitPopulate to have FM2_POPULATE
 *      posted to the split populate thread.
 */

VOID fdrPostFillFolder(PSPLITCONTROLLER pctl,
                       PMINIRECORDCORE prec,       // in: record with folder to populate
                       ULONG fl)                   // in: FFL_* flags
{
    WinPostMsg(pctl->hwndMainControl,
               FM_FILLFOLDER,
               (MPARAM)prec,
               (MPARAM)fl);
}

/*
 *@@ fdrSplitQueryPointer:
 *      returns the HPOINTER that should be used
 *      according to the present thread state.
 *
 *      Returns a HPOINTER for either the wait or
 *      arrow pointer.
 */

HPOINTER fdrSplitQueryPointer(PSPLITCONTROLLER pctl)
{
    ULONG           idPtr = SPTR_ARROW;

    if (    (pctl)
         && (pctl->cThreadsRunning)
       )
        idPtr = SPTR_WAIT;

    return WinQuerySysPointer(HWND_DESKTOP,
                              idPtr,
                              FALSE);
}

/*
 *@@ fdrInsertContents:
 *      inserts the contents of the given folder into
 *      the given container.
 *
 *      It is assumed that the folder is already populated.
 *
 *      If (precParent != NULL), the contents are inserted
 *      as child records below that record. Of course that
 *      will work in Tree view only.
 *
 *      In addition, if (hwndAddFirstChild != NULLHANDLE),
 *      this will fire an CM_ADDFIRSTCHILD msg to that
 *      window for every record that was inserted.
 *
 *      flFoldersOnly is used as follows:
 *
 *      --  If the INSERT_UNLOCKFILTERED bit is set
 *          (0x10000000), we automatically unlock each
 *          object for which fdrvIsInsertable has told us
 *          that it should not be inserted. This bit must
 *          be set if you have populated the folder,
 *          because wpPopulate locks each object in the
 *          folder once. If there was no fresh populate
 *          because the folder was already populated,
 *          that bit MUST BE CLEAR or we'll run into
 *          objects that might become dormant while
 *          still being inserted.
 *
 *      --  The low byte of flFoldersOnly has the insert
 *          mode as given to fdrvIsInsertable. See remarks
 *          there.
 *
 *@@changed V1.0.0 (2002-09-09) [umoeller]: removed linklist
 */

VOID fdrInsertContents(WPFolder *pFolder,              // in: populated folder
                       HWND hwndCnr,                   // in: cnr to insert records to
                       PMINIRECORDCORE precParent,     // in: parent record or NULL
                       ULONG flInsert,                 // in: INSERT_* flags
                       HWND hwndAddFirstChild,         // in: if != 0, we post CM_ADDFIRSTCHILD for each item too
                       PCSZ pcszFileMask)              // in: file mask filter or NULL
{
    BOOL        fFolderLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        // lock the folder contents for wpQueryContent
        if (fFolderLocked = !_wpRequestFolderMutexSem(pFolder, SEM_INDEFINITE_WAIT))
        {
            // build an array of objects that should be inserted
            // (we use an array because we want to use
            // _wpclsInsertMultipleObjects);
            // we run through the folder and check if the object
            // is insertable and, if so, add it to the array,
            // which is increased in size in chunks of 1000 pointers
            WPObject    *pObject;
            WPObject    **papObjects = NULL;
            ULONG       cObjects = 0,
                        cArray = 0,
                        cAddFirstChilds = 0;

            SOMClass    *pDetailsClass = _wpQueryFldrDetailsClass(pFolder);
                    // V1.0.1 (2002-11-30) [umoeller]

            // filter out the real INSERT_* flags
            // V1.0.0 (2002-09-13) [umoeller]
            ULONG       ulFoldersOnly = flInsert & 0x0f;

            PMPF_SPLITVIEW(("file mask is \"%s\", ulFoldersOnly %d",
                                    (pcszFileMask) ? pcszFileMask : "NULL",
                                    ulFoldersOnly));

            for (pObject = _wpQueryContent(pFolder, NULL, QC_FIRST);
                 pObject;
                 pObject = *__get_pobjNext(pObject))
            {
                if (fdrvIsInsertable(pObject,
                                     ulFoldersOnly,
                                     pcszFileMask))
                {
                    // _wpclsInsertMultipleObjects fails on the
                    // entire array if only one is already in the
                    // cnr, so make sure it isn't
                    if (!fdrvIsObjectInCnr(pObject, hwndCnr))
                    {
                        PMINIRECORDCORE pRecord;

                        // create/expand array if necessary
                        if (cObjects >= cArray)     // on the first iteration,
                                                    // both are null
                        {
                            cArray += 1000;
                            papObjects = (WPObject**)realloc(papObjects, // NULL on first call
                                                             cArray * sizeof(WPObject*));
                        }

                        // check the details class of the container...
                        // we must set the half-documented CRA_IGNORE record flag
                        // if the current object does not match the container's
                        // details class, or the container will trap on any of
                        // these objects V1.0.1 (2002-11-30) [umoeller]
                        pRecord = _wpQueryCoreRecord(pObject);
                        if (    (!pDetailsClass)
                             || (_somIsA(pObject, pDetailsClass))
                           )
                            // object is of folder's details class:
                            pRecord->flRecordAttr &= ~CRA_IGNORE;
                        else
                            // object is _not_ of folder's details class:
                            pRecord->flRecordAttr |= CRA_IGNORE;

                        // store in array
                        papObjects[cObjects++] = pObject;
                    }

                    // even if the object is already in the
                    // cnr, if we are in "add first child"
                    // mode, add the first child later;
                    // this works because hwndAddFirstChild
                    // is on the same thread
                    if (    (hwndAddFirstChild)
                         && (objIsAFolder(pObject))
                       )
                    {
                        if (!cAddFirstChilds)
                        {
                            // first post: tell thread to update
                            // the wait pointer
                            WinPostMsg(hwndAddFirstChild,
                                       FM2_ADDFIRSTCHILD_BEGIN,
                                       0,
                                       0);
                            cAddFirstChilds++;
                        }

                        WinPostMsg(hwndAddFirstChild,
                                   FM2_ADDFIRSTCHILD_NEXT,
                                   (MPARAM)pObject,
                                   NULL);
                    }
                }
                else
                    // object is _not_ insertable:
                    // then we might need to unlock it
                    // V1.0.0 (2002-09-13) [umoeller]
                    if (flInsert & INSERT_UNLOCKFILTERED)
                        _wpUnlockObject(pObject);
            } // end for pobject

            PMPF_SPLITVIEW(("--> got %d objects to insert, %d to add first child",
                        cObjects, cAddFirstChilds));

            if (cObjects)
                _wpclsInsertMultipleObjects(_somGetClass(pFolder),
                                            hwndCnr,
                                            NULL,
                                            (PVOID*)papObjects,
                                            precParent,
                                            cObjects);

            if (papObjects)
                free(papObjects);

            if (cAddFirstChilds)
            {
                // we had any "add-first-child" posts:
                // post another msg which will get processed
                // after all the "add-first-child" things
                // so that the wait ptr can be reset
                WinPostMsg(hwndAddFirstChild,
                           FM2_ADDFIRSTCHILD_DONE,
                           0,
                           0);
            }
        }
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    if (fFolderLocked)
        _wpReleaseFolderMutexSem(pFolder);
}

/* ******************************************************************
 *
 *   Split view main control (main frame's client)
 *
 ********************************************************************/

STATIC MRESULT EXPENTRY fnwpFilesFrame(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2);

/*
 *@@ SplitCreate:
 *      creates all the subcontrols of the main controller
 *      window, that is, the split window with the two
 *      subframes and containers.
 *
 *      Returns NULL if no error occurred. As a result,
 *      the return value can be returned from WM_CREATE,
 *      which stops window creation if != 0 is returned.
 *
 *@@changed V1.0.1 (2002-11-30) [umoeller]: renamed, no longer exported
 */

STATIC MPARAM SplitCreate(HWND hwnd,
                          PSPLITCONTROLLER pctl)
{
    MPARAM mrc = (MPARAM)FALSE;         // return value of WM_CREATE: 0 == OK

    SPLITBARCDATA sbcd;
    HAB hab = WinQueryAnchorBlock(hwnd);
    ULONG flStyle;

    // set the window font for the main client...
    // all controls will inherit this
    winhSetWindowFont(hwnd,
                      cmnQueryDefaultFont());

    /*
     *  split window with two containers
     *
     */

    // create two subframes to be linked in split window

    // 1) left: drives tree
    pctl->hwndTreeFrame = fdrvCreateFrameWithCnr(ID_TREEFRAME,
                                                hwnd,    // main client
                                                CCS_MINIICONS | CCS_SINGLESEL,
                                                &pctl->cvTree.hwndCnr);

    // 2) right: files
    pctl->hwndFilesFrame = fdrvCreateFrameWithCnr(ID_FILESFRAME,
                                                  hwnd,    // main client
                                                  (pctl->flSplit & SPLIT_MULTIPLESEL)
                                                     ? CCS_MINIICONS | CCS_EXTENDSEL
                                                     : CCS_MINIICONS | CCS_SINGLESEL,
                                                  &pctl->cvFiles.hwndCnr);

    // create split window
    sbcd.ulSplitWindowID = 1;
        // split window becomes client of main frame
    sbcd.ulCreateFlags =   SBCF_VERTICAL
                         | SBCF_PERCENTAGE
                         | SBCF_3DEXPLORERSTYLE
                         | SBCF_MOVEABLE;
    sbcd.lPos = pctl->lSplitBarPos;   // in percent
    sbcd.ulLeftOrBottomLimit = 100;
    sbcd.ulRightOrTopLimit = 100;
    sbcd.hwndParentAndOwner = hwnd;         // client

    if (!(pctl->hwndSplitWindow = ctlCreateSplitWindow(hab,
                                                      &sbcd)))
    {
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
                "Cannot create split window.");
        // stop window creation!
        mrc = (MPARAM)TRUE;
    }
    else
    {
        // link left and right container
        WinSendMsg(pctl->hwndSplitWindow,
                   SPLM_SETLINKS,
                   (MPARAM)pctl->hwndTreeFrame,      // left
                   (MPARAM)pctl->hwndFilesFrame);    // right

        // create the "populate" thread
        thrCreate(&pctl->tiSplitPopulate,
                  fntSplitPopulate,
                  &pctl->tidSplitPopulate,
                  "SplitPopulate",
                  THRF_PMMSGQUEUE | THRF_WAIT_EXPLICIT,
                            // we MUST wait until the thread
                            // is ready; populate posts event
                            // sem when it has created its obj wnd
                  (ULONG)pctl);
        // this will wait until the object window has been created
    }

    return mrc;
}

/*
 *@@ SplitSendWMControl:
 *
 *@@added V1.0.0 (2002-09-13) [umoeller]
 */

STATIC MRESULT SplitSendWMControl(PSPLITCONTROLLER pctl,
                                  USHORT usNotifyCode,
                                  MPARAM mp2)
{
    return WinSendMsg(pctl->hwndMainFrame,
                      WM_CONTROL,
                      MPFROM2SHORT(FID_CLIENT,
                                   usNotifyCode),
                      mp2);
}

/*
 *@@ SplitFillFolder:
 *      implementation for FM_FILLFOLDER in fnwpSplitController.
 *
 *@@added V1.0.1 (2002-12-08) [umoeller]
 */

STATIC VOID SplitFillFolder(HWND hwndClient,
                            MPARAM mp1,
                            MPARAM mp2)
{
    PSPLITCONTROLLER pctl;

    if (pctl = WinQueryWindowPtr(hwndClient, QWL_USER))
    {
        PMINIRECORDCORE prec = (PMINIRECORDCORE)mp1;

        // V1.0.4 (2005-07-16) [pr]
        PMPF_SPLITVIEW(("FM_FILLFOLDER %s", prec ? prec->pszIcon : "NULL"));

        DumpFlags((ULONG)mp2);

        if (!((ULONG)mp2 & FFL_FOLDERSONLY))
        {
            // not folders-only: then we need to
            // refresh the files list

            WPFolder    *pFolder = fdrvGetFSFromRecord(prec,
                                                       TRUE); // folders only

            PMPF_SPLITVIEW(("  calling fdrvClearContainer"));

            // disable the whitespace context menu and
            // CN_EMPHASIS notifications until all objects
            // are inserted
            pctl->precFilesShowing = NULL;

            // notify frame that we're busy
            SplitSendWMControl(pctl,
                               SN_FOLDERCHANGING,
                               pFolder);        // can be NULL

            fdrvClearContainer(pctl->cvFiles.hwndCnr,
                               (pctl->fUnlockOnClear)
                                   ? CLEARFL_UNLOCKOBJECTS
                                   : 0);

            // if we had a previous view item for the
            // files cnr, remove it... since the entire
            // pctl structure was initially zeroed, this
            // check is safe
            if (pctl->pobjUseList)
                _wpDeleteFromObjUseList(pctl->pobjUseList,
                                        &pctl->uiDisplaying);

            // register a view item for the object
            // that was selected in the tree so that
            // it is marked as "open" and the refresh
            // thread can handle updates for it too;
            // use the OBJECT, not the folder derived
            // from it, since for disk objects, the
            // disks have the views, not the root folder
            if (   prec  // V1.0.4 (2005-07-16) [pr]: @@fixes 530
                &&
                (pctl->pobjUseList = OBJECT_FROM_PREC(prec))
               )
            {
                pctl->uiDisplaying.type = USAGE_OPENVIEW;
                memset(&pctl->viDisplaying, 0, sizeof(VIEWITEM));
                pctl->viDisplaying.view =
                        *G_pulVarMenuOfs + ID_XFMI_OFS_SPLIT_SUBFILES;
                pctl->viDisplaying.handle = pctl->hwndFilesFrame;
                        // do not change this! XFolder::wpUnInitData
                        // relies on this!
                // set this flag so that we can disable
                // _wpAddToContent for this view while we're
                // populating; the flag is cleared once we're done
                pctl->viDisplaying.ulViewState = VIEWSTATE_OPENING;

                _wpAddToObjUseList(pctl->pobjUseList,
                                   &pctl->uiDisplaying);

                PMPF_SPLITVIEW(("  calling fdrvSetupView for right half"));

                fdrvSetupView(&pctl->cvFiles,
                              pFolder,
                              0,
                              0); /* (pctl->flSplit & SPLIT_DETAILS) ? OPEN_DETAILS : OPEN_CONTENTS,
                              !(pctl->flSplit & SPLIT_NOMINI));      // fmini
                              */

                /*
                if ((ULONG)mp2 & FFL_SETBACKGROUND)
                    // change files container background NOW
                    // to give user immediate feedback
                    // V1.0.0 (2002-09-24) [umoeller]: use
                    // msg because we need this code from XFolder
                    // method code now too
                    WinPostMsg(pctl->hwndFilesFrame,
                               FM_SETCNRLAYOUT,
                               pFolder,
                               0);
                */
            }
        }

        // mark this folder as "populating"
        pctl->precPopulating = prec;

        // post FM2_POPULATE
        if (prec)  // V1.0.4 (2005-07-16) [pr]: @@fixes 530
            fdrSplitPopulate(pctl,
                             prec,
                             (ULONG)mp2);
    }
}

/*
 *@@ fnwpSplitController:
 *      window proc for the split view controller, which is
 *      the client of the split view's main frame and has
 *      its own window class (WC_SPLITCONTROLLER).
 *
 *      This is really the core of the split view that
 *      handles the cooperation of the two containers and
 *      manages populate. This is shared code between
 *      the "real" folder split view and the file dialog.
 *      See fdrSplitCreateFrame.
 *
 *      In addition, this gives the frame (its parent) a
 *      chance to fine-tune the behavior of the split view.
 *      As a result, the user code (the "real" folder split
 *      view, or the file dialog) can subclass the frame
 *      and thus influence how the split view works.
 *
 *      In detail:
 *
 *      --  We send WM_CONTROL with SN_FOLDERCHANGING,
 *          SN_FOLDERCHANGED, SN_OBJECTSELECTED, or
 *          SN_OBJECTENTER to the frame whenever selections
 *          change.
 *
 *      --  We make sure that the frame gets a chance
 *          to process WM_CLOSE. The problem is that
 *          with PM, only the client gets WM_CLOSE if
 *          a client exists. So we forward (post) WM_CONTROL
 *          with SN_FRAMECLOSE to the frame when we get it,
 *          and the frame can take action.
 *
 *@@changed V1.0.6 (2006-10-16) [pr]: fixed show/maximize from minimize @@fixes 865
 */

MRESULT EXPENTRY fnwpSplitController(HWND hwndClient, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT         mrc = 0;
    PSPLITCONTROLLER pctl;

    TRY_LOUD(excpt1)
    {
        switch (msg)
        {
            /*
             * WM_CREATE:
             *      we get PSPLITCONTROLLER in mp1.
             */

            case WM_CREATE:
                WinSetWindowPtr(hwndClient, QWL_USER, mp1);

                mrc = SplitCreate(hwndClient,
                                  (PSPLITCONTROLLER)mp1);
            break;

            /*
             * WM_WINDOWPOSCHANGED:
             *
             */

            case WM_WINDOWPOSCHANGED:
                if (((PSWP)mp1)->fl & SWP_SIZE)
                {
                    if (    (pctl = WinQueryWindowPtr(hwndClient, QWL_USER))
                            // file dialog handles sizing itself
                         && (!(pctl->flSplit & SPLIT_NOAUTOPOSITION))
                       )
                    {
                        // adjust size of "split window",
                        // which will rearrange all the linked
                        // windows (comctl.c)
                        WinSetWindowPos(pctl->hwndSplitWindow,
                                        HWND_TOP,
                                        0,
                                        0,
                                        ((PSWP)mp1)->cx,
                                        ((PSWP)mp1)->cy,
                                        SWP_SIZE);
                    }
                }

                // return default NULL
            break;

            /*
             * WM_MINMAXFRAME:
             *      when minimizing, we hide the "split window",
             *      because otherwise the child dialogs will
             *      display garbage
             */

            case WM_MINMAXFRAME:
                if (pctl = WinQueryWindowPtr(hwndClient, QWL_USER))
                {
                    PSWP pswp = (PSWP)mp1;
                    if (pswp->fl & SWP_MINIMIZE)
                        WinShowWindow(pctl->hwndSplitWindow, FALSE);
                    else if (pswp->fl & (SWP_MAXIMIZE | SWP_RESTORE))  // V1.0.6 (2006-10-16) [pr]: @@fixes 865
                        WinShowWindow(pctl->hwndSplitWindow, TRUE);
                }
            break;

            /*
             *@@ FM_FILLFOLDER:
             *      posted to the main control from the tree
             *      frame to fill the dialog when a new folder
             *      has been selected in the tree.
             *
             *      Use fdrPostFillFolder for posting this
             *      message, which is type-safe.
             *
             *      This automatically offloads populate
             *      to fntSplitPopulate, which will then post
             *      a bunch of messages back to us so we
             *      can update the dialog properly.
             *
             *      Parameters:
             *
             *      --  PMINIRECORDCODE mp1: record of folder
             *          (or disk or whatever) to fill with.
             *
             *      --  ULONG mp2: dialog flags.
             *
             *      mp2 can be any combination of the following:
             *
             *      --  If FFL_FOLDERSONLY is set, this operates
             *          in "folders only" mode. We will then
             *          populate the folder with subfolders only.
             *          The files list is not changed.
             *
             *          If the flag is not set, the folder is
             *          fully populated and the files list is
             *          updated as well.
             *
             *      --  If FFL_SCROLLTO is set, we will scroll
             *          the drives tree so that the given record
             *          becomes visible.
             *
             *      --  If FFL_EXPAND is set, we will also expand
             *          the record in the tree after
             *          populate and run "add first child" for
             *          each subrecord that was inserted.
             *
             *      --  If FFL_SETBACKGROUND is set, we will
             *          revamp the files container to use the
             *          view settings of the given folder.
             *
             *@@added V0.9.18 (2002-02-06) [umoeller]
             */

            case FM_FILLFOLDER:
                SplitFillFolder(hwndClient, mp1, mp2);
            break;

            /*
             *@@ FM_POPULATED_FILLTREE:
             *      posted by fntSplitPopulate after populate has been
             *      done for a folder. This gets posted in any case,
             *      if the folder was populated in folders-only mode
             *      or not.
             *
             *      Parameters:
             *
             *      --  PMINIRECORDCODE mp1: record of folder
             *          (or disk or whatever) to fill with.
             *
             *      --  ULONG mp2: FFL_* flags for whether to
             *          expand.
             *
             *      This then calls fdrInsertContents to insert
             *      the subrecords into the tree. If FFL_EXPAND
             *      was set, the tree is expanded, and we pass
             *      the controller window handle to fdrInsertContents
             *      so that we can "add first child" messages back.
             *
             *@@added V0.9.18 (2002-02-06) [umoeller]
             */

            case FM_POPULATED_FILLTREE:
            {
                PMINIRECORDCORE prec;

                PMPF_SPLITVIEW(("FM_POPULATED_FILLTREE %s",
                            mp1
                                ? ((PMINIRECORDCORE)mp1)->pszIcon
                                : "NULL"));

                if (    (pctl = WinQueryWindowPtr(hwndClient, QWL_USER))
                     && (prec = (PMINIRECORDCORE)mp1)
                   )
                {
                    // we're done populating
                    pctl->precPopulating = NULL;

                    if (!((ULONG)mp2 & FFL_POPULATEFAILED))
                    {
                        WPFolder    *pFolder = fdrvGetFSFromRecord(mp1, TRUE);
                        PLISTNODE   pNode;
                        HWND        hwndAddFirstChild = NULLHANDLE;

                        if ((ULONG)mp2 & FFL_EXPAND)
                        {
                            BOOL        fOld = pctl->fSplitViewReady;
                            // stop control notifications from messing with this
                            pctl->fSplitViewReady = FALSE;
                            cnrhExpandFromRoot(pctl->cvTree.hwndCnr,
                                               (PRECORDCORE)prec);
                            // then fire CM_ADDFIRSTCHILD too
                            hwndAddFirstChild = pctl->hwndSplitPopulate;

                            // re-enable control notifications
                            pctl->fSplitViewReady = fOld;
                        }

                        // insert subfolders into tree on the left
                        fdrInsertContents(pFolder,
                                          pctl->cvTree.hwndCnr,
                                          (PMINIRECORDCORE)mp1,
                                          INSERT_FOLDERSANDDISKS,
                                          hwndAddFirstChild,
                                          NULL);      // file mask
                    }
                }
            }
            break;

            /*
             *@@ FM_POPULATED_SCROLLTO:
             *
             *      Parameters:
             *
             *      --  PMINIRECORDCODE mp1: record of folder
             *          (or disk or whatever) that was populated
             *          and should now be scrolled to.
             *
             *@@added V0.9.18 (2002-02-06) [umoeller]
             */

            case FM_POPULATED_SCROLLTO:

                if (pctl = WinQueryWindowPtr(hwndClient, QWL_USER))
                {
                    BOOL    fOld = pctl->fSplitViewReady;
                    ULONG   ul;

                    PMPF_SPLITVIEW(("FM_POPULATED_SCROLLTO %s",
                                mp1
                                    ? ((PMINIRECORDCORE)mp1)->pszIcon
                                    : "NULL"));

                    // stop control notifications from messing with this
                    pctl->fSplitViewReady = FALSE;

                    ul = cnrhScrollToRecord(pctl->cvTree.hwndCnr,
                                            (PRECORDCORE)mp1,
                                            CMA_ICON | CMA_TEXT | CMA_TREEICON,
                                            TRUE);       // keep parent
                    cnrhSelectRecord(pctl->cvTree.hwndCnr,
                                     (PRECORDCORE)mp1,
                                     TRUE);
                    if (ul && ul != 3)
                        cmnLog(__FILE__, __LINE__, __FUNCTION__,
                                "Error: cnrhScrollToRecord returned %d", ul);

                    // re-enable control notifications
                    pctl->fSplitViewReady = fOld;
                }

            break;

            /*
             *@@ FM_POPULATED_FILLFILES:
             *      posted by fntSplitPopulate after populate has been
             *      done for the newly selected folder, if this
             *      was not in folders-only mode. We must then fill
             *      the right half of the dialog with all the objects.
             *
             *      Parameters:
             *
             *      --  PMINIRECORDCODE mp1: record of folder
             *          (or disk or whatever) to fill with.
             *          Use fdrvGetFSFromRecord(mp1, TRUE) to
             *          get the real folder since this might
             *          be a shadow or disk object.
             *
             *      --  ULONG mp2: FFL_* flags. In addition to
             *          the flags that were initially passed with
             *          FM_FILLFOLDER, this will have the
             *          FFL_UNLOCKOBJECTS bit set if the folder
             *          was freshly populated and objects should
             *          therefore be unlocked, or FFL_POPULATEFAILED
             *          if populate failed and we should simply
             *          refresh the view state without actually
             *          inserting anything.
             *
             *@@added V0.9.18 (2002-02-06) [umoeller]
             *@@changed V1.0.0 (2002-09-13) [umoeller]: changed mp2 definition
             */

            case FM_POPULATED_FILLFILES:

                if (pctl = WinQueryWindowPtr(hwndClient, QWL_USER))
                {
                    PMPF_SPLITVIEW(("FM_POPULATED_FILLFILES %s",
                                mp1
                                    ? ((PMINIRECORDCORE)mp1)->pszIcon
                                    : "NULL"));

                    if (mp1)
                    {
                        WPFolder    *pFolder = NULL;

                        pctl->fUnlockOnClear = FALSE;

                        if (    (!((ULONG)mp2 & FFL_POPULATEFAILED))
                             && (pFolder = fdrvGetFSFromRecord((PMINIRECORDCORE)mp1,
                                                               TRUE))
                           )
                        {
                            CHAR    szPathName[2*CCHMAXPATH];
                            ULONG   flInsert;

                            // if a file mask is specified, then we're
                            // working for the file dialog, and should
                            // only insert filesystem objects
                            if (pctl->pcszFileMask)
                                flInsert = INSERT_FILESYSTEMS;
                            else
                                flInsert = INSERT_ALL;

                            PMPF_SPLITVIEW(("   FFL_UNLOCKOBJECTS is 0x%lX", ((ULONG)mp2 & FFL_UNLOCKOBJECTS)));

                            if ((ULONG)mp2 & FFL_UNLOCKOBJECTS)
                            {
                                flInsert |= INSERT_UNLOCKFILTERED;
                                pctl->fUnlockOnClear = TRUE;
                            }

                            // insert all contents into list on the right
                            fdrInsertContents(pFolder,
                                              pctl->cvFiles.hwndCnr,
                                              NULL,        // parent
                                              flInsert,
                                              NULLHANDLE,  // no add first child
                                              pctl->pcszFileMask);
                                                    // ptr to file mask buffer if specified
                                                    // with fdrSplitCreateFrame; this is
                                                    // NULL unless we're working for the
                                                    // file dialog
                        }

                        // clear the "opening" flag in the VIEWITEM
                        // so that XFolder::wpAddToContent will start
                        // giving us new objects
                        pctl->viDisplaying.ulViewState &= ~VIEWSTATE_OPENING;

                        // re-enable the whitespace context menu and
                        // CN_EMPHASIS notifications
                        pctl->precFilesShowing = (PMINIRECORDCORE)mp1;

                        // update the folder pointers in the SFV
                        pctl->psfvFiles->somSelf = pFolder;
                        pctl->psfvFiles->pRealObject = OBJECT_FROM_PREC((PMINIRECORDCORE)mp1);

                        // notify frame that we're done being busy
                        SplitSendWMControl(pctl,
                                           SN_FOLDERCHANGED,
                                           pFolder);
                    }
                }

            break;

            /*
             * CM_UPDATEPOINTER:
             *      posted when threads exit etc. to update
             *      the current pointer.
             */

            case FM_UPDATEPOINTER:

                if (pctl = WinQueryWindowPtr(hwndClient, QWL_USER))
                {
                    WinSetPointer(HWND_DESKTOP,
                                  fdrSplitQueryPointer(pctl));
                }

            break;

            /*
             * WM_PAINT:
             *      file dialog needs us to paint the background
             *      because in the file dialog, we are not entirely
             *      covered by the split window. This code should
             *      never get called for the real split view.
             */

            case WM_PAINT:
            {
                HPS hps;
                if (hps = WinBeginPaint(hwndClient, NULLHANDLE, NULL))
                {
                    RECTL rcl;
                    WinQueryWindowRect(hwndClient, &rcl);
                    gpihSwitchToRGB(hps);
                    WinFillRect(hps,
                                &rcl,
                                winhQueryPresColor2(hwndClient,
                                                    PP_BACKGROUNDCOLOR,
                                                    PP_BACKGROUNDCOLORINDEX,
                                                    FALSE,
                                                    SYSCLR_DIALOGBACKGROUND));


                    WinEndPaint(hps);
                }
            }
            break;

            /*
             * WM_CLOSE:
             *      posts (!) the SN_FRAMECLOSE notification to
             *      the frame.
             *
             *      See SN_FRAMECLOSE in fdrsplit.h for
             *      details.
             */

            case WM_CLOSE:
                if (pctl = WinQueryWindowPtr(hwndClient, QWL_USER))
                {
                    PMPF_SPLITVIEW(("WM_CLOSE --> sending SN_FRAMECLOSE"));

                    // post, do not send, or the file dialog
                    // will never see this in the WinGetMsg loop
                    WinPostMsg(pctl->hwndMainFrame,
                               WM_CONTROL,
                               MPFROM2SHORT(FID_CLIENT,
                                            SN_FRAMECLOSE),
                               NULL);
                }

                // never pass this on, because the default
                // window proc posts WM_QUIT
            break;

            default:
                mrc = WinDefWindowProc(hwndClient, msg, mp1, mp2);
        }
    }
    CATCH(excpt1) {} END_CATCH();

    return mrc;
}

/* ******************************************************************
 *
 *   Left tree frame and client
 *
 ********************************************************************/

/*
 *@@ HandleWMChar:
 *      common WM_CHAR handler for both the tree and
 *      the files frame.
 *
 *      Returns TRUE if the msg was processed.
 *
 *@@added V1.0.0 (2002-11-23) [umoeller]
 *@@changed V1.0.1 (2003-01-05) [umoeller]: improved backspace (proper scrolling, shift support)
 */

STATIC BOOL HandleWMChar(HWND hwndFrame,
                         MPARAM mp1,
                         MPARAM mp2)
{
    HWND    hwndMainControl;
    USHORT  usFlags    = SHORT1FROMMP(mp1);
    USHORT  usvk       = SHORT2FROMMP(mp2);
    PSPLITCONTROLLER pctl;
    BOOL    fProcessed = FALSE;

    if (    (!(usFlags & KC_KEYUP))
         && (usFlags & KC_VIRTUALKEY)
         && (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
         && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
       )
    {
        if (!(fProcessed = (BOOL)SplitSendWMControl(pctl,
                                                    SN_VKEY,
                                                    (MPARAM)usvk)))
        {
            switch (usvk)
            {
                case VK_BACKSPACE:
                {
                    // get current selection in drives view
                    PMINIRECORDCORE precParent;
                    if (precParent = (PMINIRECORDCORE)WinSendMsg(
                                            pctl->cvTree.hwndCnr,
                                            CM_QUERYRECORD,
                                            (MPARAM)pctl->precTreeSelected,
                                            MPFROM2SHORT(CMA_PARENT,
                                                         CMA_ITEMORDER)))
                    {
                        // not at root already:

                        // if shift is pressed also, collapse the current record
                        // V1.0.1 (2003-01-05) [umoeller]
                        if (usFlags & KC_SHIFT)
                            WinSendMsg(pctl->cvTree.hwndCnr,
                                       CM_COLLAPSETREE,
                                       (MPARAM)pctl->precTreeSelected,
                                       0);

                        // scroll to record first V1.0.1 (2003-01-05) [umoeller]
                        cnrhScrollToRecord(pctl->cvTree.hwndCnr,
                                           (PRECORDCORE)precParent,
                                           CMA_ICON | CMA_TEXT | CMA_TREEICON,
                                           TRUE);       // keep parent
                        cnrhSelectRecord(pctl->cvTree.hwndCnr,
                                         precParent,
                                         TRUE);
                        fProcessed = TRUE;
                    }
                }
                break;
            }
        }
    }

    return fProcessed;
}

/*
 *@@ RefreshToolbarButtons:
 *      enables/sets tool bar buttons when view settings
 *      change.
 *
 *@@added V1.0.1 (2002-12-08) [umoeller]
 */

STATIC VOID RefreshToolbarButtons(PSPLITCONTROLLER pctl,
                                  ULONG ulView)
{
    HWND hwndMiniButton;

    if (    (pctl->xfc.hwndToolBar)
         && (hwndMiniButton = WinWindowFromID(pctl->xfc.hwndToolBar,
                                              *G_pulVarMenuOfs + ID_XFMI_OFS_SMALLICONS))
       )
    {
        // enable or disable the "mini icons" button
        _Pmpf(("hwndMiniButton is 0x%lX", hwndMiniButton));

        WinEnableWindow(hwndMiniButton,
                        (ulView != OPEN_DETAILS));
        WinSendMsg(hwndMiniButton,
                   TBBM_CHECK,
                   (MPARAM)((ulView == OPEN_DETAILS)
                        ? TRUE
                        : !(_xwpQueryXFolderStyle(pctl->pRootsFolder) & XFFL_SPLIT_NOMINI)),
                   0);
    }
}

/*
 *@@ TreeFrameControl:
 *      implementation for WM_CONTROL for FID_CLIENT
 *      in fnwpTreeFrame.
 *
 *      Set *pfCallDefault to TRUE if you want the
 *      parent window proc to be called.
 *
 *@@added V1.0.0 (2002-08-26) [umoeller]
 *@@changed V1.0.5 (2006-04-13) [pr]: Fix status bar update @@fixes 326
 *@@changed V1.0.6 (2006-12-06) [pr]: added CN_SETFOCUS @@fixes 326
 */

STATIC MRESULT TreeFrameControl(HWND hwndFrame,
                                MPARAM mp1,
                                MPARAM mp2,
                                PBOOL pfCallDefault)
{
    MRESULT mrc = 0;
    HWND                hwndMainControl;
    PSPLITCONTROLLER pctl;
    PMINIRECORDCORE     prec;

    switch (SHORT2FROMMP(mp1))
    {
        /*
         * CN_EMPHASIS:
         *      selection changed:
         */

        case CN_EMPHASIS:
        {
            PNOTIFYRECORDEMPHASIS pnre = (PNOTIFYRECORDEMPHASIS)mp2;

            // V1.0.5 (2006-04-13) [pr]: Fix status bar update @@fixes 326
            if (    (pnre->fEmphasisMask & CRA_SELECTED)
                 && (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                 && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                 // notifications not disabled?
                 && (pctl->fSplitViewReady)
                 && (prec = (PMINIRECORDCORE)pnre->pRecord)
                 && (prec->flRecordAttr & CRA_SELECTED)
               )
            {
                PMPF_SPLITVIEW(("CN_EMPHASIS %s",
                        prec->pszIcon));

                // record changed?
                if (prec != pctl->precTreeSelected)
                {
                    // then go refresh the files container
                    // with a little delay; see WM_TIMER below
                    // if the user goes thru a lot of records
                    // in the tree, this timer gets restarted
                    pctl->precToPopulate
                        = pctl->precTreeSelected
                        = prec;
                    WinStartTimer(pctl->habGUI,
                                  hwndFrame,        // post to tree frame
                                  1,
                                  200);
                }

                if (pctl->xfc.hwndStatusBar)
                {
                    PMPF_SPLITVIEW(("CN_EMPHASIS: posting STBM_UPDATESTATUSBAR to hwnd %lX",
                                pctl->xfc.hwndStatusBar ));

                    // have the status bar updated and make
                    // sure the status bar retrieves its info
                    // from the _left_ cnr
                    pctl->hwndFocusCnr = pctl->cvTree.hwndCnr;  // V1.0.9 (2012-02-12) [pr]
                    WinPostMsg(pctl->xfc.hwndStatusBar,
                               STBM_UPDATESTATUSBAR,
                               (MPARAM)pctl->hwndFocusCnr,
                               MPNULL);
                }
            }
        }
        break;

        /*
         * CN_SETFOCUS:
         *      V1.0.6 (2006-12-06) [pr]
         */

        case CN_SETFOCUS:
            if (    (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                 && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                 // notifications not disabled?
                 && (pctl->fSplitViewReady)
                 && (pctl->xfc.hwndStatusBar)
               )
            {
                PMPF_SPLITVIEW(("CN_SETFOCUS: posting STBM_UPDATESTATUSBAR to hwnd %lX",
                            pctl->xfc.hwndStatusBar ));

                // have the status bar updated and make
                // sure the status bar retrieves its info
                // from the _left_ cnr
                pctl->hwndFocusCnr = pctl->cvTree.hwndCnr;  // V1.0.9 (2012-02-12) [pr]
                WinPostMsg(pctl->xfc.hwndStatusBar,
                           STBM_UPDATESTATUSBAR,
                           (MPARAM)pctl->hwndFocusCnr,
                           MPNULL);
            }
        break;

        /*
         * CN_EXPANDTREE:
         *      user clicked on "+" sign next to
         *      tree item; expand that, but start
         *      "add first child" thread again.
         *
         *      Since a record is automatically selected
         *      also when it is expanded, we get both
         *      CN_EMPHASIS and CN_EXPANDTREE, where
         *      CN_EMPHASIS comes in first, from my testing.
         *
         *      We have to differentiate between two cases:
         *
         *      --  If the user expands a record that was
         *          previously _not_ selected (or double-clicks
         *          on the tree record, which has the same
         *          effect), we only mark precTreeExpanded
         *          so that the WM_TIMER that was started by
         *          the previous CN_EMPHASIS can run "add first
         *          child" as well.
         *
         *      --  If the user expands a record that is
         *          currently selected, we did not get CN_EMPHASIS
         *          previously and need to run "add first child"
         *          manually.
         *
         *      V1.0.0 (2002-09-13) [umoeller]
         */

        case CN_EXPANDTREE:
            if (    (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                 && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                 // notifications not disabled?
                 && (pctl->fSplitViewReady)
                 && (prec = (PMINIRECORDCORE)mp2)
               )
            {
                PMPF_SPLITVIEW(("CN_EXPANDTREE for %s; precToPopulate is %s",
                        prec->pszIcon,
                        (pctl->precToPopulate) ? pctl->precToPopulate->pszIcon : "NULL"));

                pctl->precTreeExpanded = prec;

                if (pctl->precToPopulate != prec)
                    // the record that is being expanded has _not_ just
                    // been selected: run "add first child"
                    // V1.0.0 (2002-11-23) [umoeller]
                    fdrPostFillFolder(pctl,
                                      prec,
                                      FFL_FOLDERSONLY | FFL_EXPAND);

                // and call default because xfolder
                // handles auto-scroll
                *pfCallDefault = TRUE;
            }
        break;

        /*
         * CN_ENTER:
         *      intercept this so that we won't open
         *      a folder view.
         *
         *      Before this, we should have gotten
         *      CN_EMPHASIS so the files list has
         *      been updated already.
         *
         *      Instead, check whether the record has
         *      been expanded or collapsed and do
         *      the reverse.
         */

        case CN_ENTER:
        {
            PNOTIFYRECORDENTER pnre;

            if (    (pnre = (PNOTIFYRECORDENTER)mp2)
                 && (prec = (PMINIRECORDCORE)pnre->pRecord)
                            // can be null for whitespace!
               )
            {
                ULONG ulmsg = CM_EXPANDTREE;
                if (prec->flRecordAttr & CRA_EXPANDED)
                    ulmsg = CM_COLLAPSETREE;

                WinPostMsg(pnre->hwndCnr,
                           ulmsg,
                           (MPARAM)prec,
                           0);
            }
        }
        break;

        /*
         * CN_CONTEXTMENU:
         *      we need to intercept this for context menus
         *      on whitespace, because the WPS won't do it.
         *      We pass all other cases on because the WPS
         *      does do things correctly for object menus.
         */

        case CN_CONTEXTMENU:
        {
            *pfCallDefault = TRUE;

            if (    (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                 && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                 && (!mp2)      // whitespace:
                    // display the menu for the root folder
               )
            {
#if 1
                POINTL  ptl;
                WinQueryPointerPos(HWND_DESKTOP, &ptl);
                // convert to cnr coordinates
                WinMapWindowPoints(HWND_DESKTOP,        // from
                                   pctl->cvTree.hwndCnr,   // to
                                   &ptl,
                                   1);
                _wpDisplayMenu(pctl->pRootObject,
                               pctl->hwndTreeFrame, // owner
                               pctl->cvTree.hwndCnr,   // parent
                               &ptl,
                               MENU_OPENVIEWPOPUP,
                               0);
#endif

                *pfCallDefault = FALSE;
            }
        }
        break;

        default:
            *pfCallDefault = TRUE;
    }

    return mrc;
}

/*
 *@@ fnwpTreeFrame:
 *      subclassed frame window on the right for the
 *      "Files" container. This has the files cnr
 *      as its FID_CLIENT.
 *
 *      We use the XFolder subclassed window proc for
 *      most messages. In addition, we intercept a
 *      couple more for extra features.
 *
 *@@changed V1.0.0 (2002-11-23) [umoeller]: brought back keyboard support
 */

STATIC MRESULT EXPENTRY fnwpTreeFrame(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT             mrc = 0;

    TRY_LOUD(excpt1)
    {
        BOOL                fCallDefault = FALSE;
        PSUBCLFOLDERVIEW    psfv = WinQueryWindowPtr(hwndFrame, QWL_USER);
        HWND                hwndMainControl;
        PSPLITCONTROLLER pctl;
        PMINIRECORDCORE     prec;

        switch (msg)
        {
            case WM_CONTROL:
                if (SHORT1FROMMP(mp1) == FID_CLIENT)     // that's the container
                    mrc = TreeFrameControl(hwndFrame,
                                           mp1,
                                           mp2,
                                           &fCallDefault);
                else
                    fCallDefault = TRUE;
            break;

            case WM_TIMER:
                if (    ((ULONG)mp1 == 1)
                     && (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                     && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                   )
                {
                    // timer 1 gets (re)started every time the user
                    // selects a record in three (see CN_EMPHASIS above);
                    // we use this to avoid crowding the populate thread
                    // with populate messages if the user is using the
                    // keyboard to run down the tree; CN_EMPHASIS only
                    // sets pctl->precFolderToPopulate and starts this
                    // timer.

                    PMPF_SPLITVIEW(("WM_TIMER 1, precFolderPopulating is 0x%lX (%s)",
                                pctl->precPopulating,
                                (pctl->precPopulating)
                                    ? pctl->precPopulating->pszIcon
                                    : "NULL"));

                    // If we're still busy populating something,
                    // keep the timer running and do nothing.
                    if (!pctl->precPopulating)
                    {
                        // then stop the timer
                        WinStopTimer(pctl->habGUI,
                                     hwndFrame,
                                     1);

                        // fire populate twice:

                        // 1) full populate for the files cnr
                        PMPF_SPLITVIEW(("posting FM_FILLFOLDER for files cnr"));
                        fdrPostFillFolder(pctl,
                                          pctl->precToPopulate,
                                          FFL_SETBACKGROUND);

                        if (pctl->precToPopulate == pctl->precTreeExpanded)
                        {
                            // 2) this record was also expanded: fire another
                            //    populate so that the grandchildren get added
                            //    to the tree
                            PMPF_SPLITVIEW(("posting second FM_FILLFOLDER for expanding tree"));
                            fdrPostFillFolder(pctl,
                                              pctl->precToPopulate,
                                              FFL_FOLDERSONLY | FFL_SCROLLTO | FFL_EXPAND);
                        }

                        pctl->precToPopulate = NULL;
                    }

                    // reset "expanded" flag
                    pctl->precTreeExpanded = NULL;
                }
            break;

            case WM_SYSCOMMAND:
                // forward to main frame
                WinPostMsg(WinQueryWindow(WinQueryWindow(hwndFrame, QW_OWNER),
                                          QW_OWNER),
                           msg,
                           mp1,
                           mp2);
            break;

            case WM_CHAR:
                fCallDefault = !HandleWMChar(hwndFrame, mp1, mp2);
            break;

            /*
             * WM_QUERYOBJECTPTR:
             *      we receive this message from the WPS somewhere
             *      when it tries to process menu items for the
             *      whitespace context menu. I guess normally this
             *      message is taken care of when a frame is registered
             *      as a folder view, but this frame is not. So we must
             *      answer by returning the folder that this frame
             *      represents.
             *
             *      Answering this message will enable all WPS whitespace
             *      magic: both displaying the whitespace context menu
             *      and making messages work for the whitespace menu items.
             */

            case WM_QUERYOBJECTPTR:
                PMPF_SPLITVIEW(("WM_QUERYOBJECTPTR"));
                if (    (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                     && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                   )
                    mrc = (MRESULT)pctl->pRootObject;
            break;

            /*
             * WM_CONTROLPOINTER:
             *      show wait pointer if we're busy.
             */

            case WM_CONTROLPOINTER:
                if (    (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                     && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                   )
                    mrc = (MPARAM)fdrSplitQueryPointer(pctl);
            break;

            default:
                fCallDefault = TRUE;
        }

        if (fCallDefault)
            mrc = fdrProcessFolderMsgs(hwndFrame,
                                       msg,
                                       mp1,
                                       mp2,
                                       psfv,
                                       psfv->pfnwpOriginal);
    }
    CATCH(excpt1) {} END_CATCH();

    return mrc;
}

/* ******************************************************************
 *
 *   Right files frame and client
 *
 ********************************************************************/

/*
 *@@ FilesFrameControl:
 *      implementation for WM_CONTROL for FID_CLIENT
 *      in fnwpFilesFrame.
 *
 *      Set *pfCallDefault to TRUE if you want the
 *      parent window proc to be called.
 *
 *@@added V1.0.0 (2002-08-26) [umoeller]
 *@@changed V1.0.5 (2006-04-13) [pr]: Fix status bar update @@fixes 326
 *@@changed V1.0.6 (2006-12-06) [pr]: added CN_SETFOCUS and delay timer @@fixes 326
 */

STATIC MRESULT FilesFrameControl(HWND hwndFrame,
                                 MPARAM mp1,
                                 MPARAM mp2,
                                 PBOOL pfCallDefault)
{
    MRESULT             mrc = 0;
    HWND                hwndMainControl;
    PSPLITCONTROLLER    pctl;

    switch (SHORT2FROMMP(mp1))
    {
        /*
         * CN_EMPHASIS:
         *      selection changed: refresh
         *      the status bar.
         */

        case CN_EMPHASIS:
        {
            PNOTIFYRECORDEMPHASIS pnre = (PNOTIFYRECORDEMPHASIS)mp2;
            PMINIRECORDCORE prec;

            // V1.0.5 (2006-04-13) [pr]: Fix status bar update @@fixes 326
            if (    (pnre->fEmphasisMask & (CRA_SELECTED | CRA_CURSORED))
                 && (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                 && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                 // notifications not disabled?
                 && (pctl->fSplitViewReady)
                 && (prec = (PMINIRECORDCORE)pnre->pRecord)
                    // and we're not currently populating?
                    // (the cnr automatically selects the first obj
                    // that gets inserted, and we'd rather not have
                    // the file dialog react to such auto-selections);
                    // precFilesShowing is TRUE only after all objects
                    // have been inserted
                 && (pctl->precFilesShowing)
               )
            {
                PMPF_SPLITVIEW(("CN_EMPHASIS [%s]",
                                         prec->pszIcon));
                if (prec->flRecordAttr & CRA_SELECTED)
                    SplitSendWMControl(pctl,
                                       SN_OBJECTSELECTED,
                                       prec);

                if (pctl->xfc.hwndStatusBar)
                {
                    PMPF_SPLITVIEW(("CN_EMPHASIS: starting Timer 1"));

                    // V1.0.6 (2006-12-06) [pr]
                    // delay status bar update otherwise we get a flash from the
                    // CN_SETFOCUS message if the selections in the container are
                    // changing
                    WinStartTimer(pctl->habGUI,
                                  hwndFrame,        // post to files frame
                                  1,
                                  50);
                }
            }
        }
        break;

        /*
         * CN_SETFOCUS:
         *      V1.0.6 (2006-12-06) [pr]
         */

        case CN_SETFOCUS:
            if (    (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                 && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                 && (pctl->xfc.hwndStatusBar)
               )
            {
                PMPF_SPLITVIEW(("CN_SETFOCUS: starting Timer 1"));

                WinStartTimer(pctl->habGUI,
                              hwndFrame,        // post to files frame
                              1,
                              50);
            }
        break;

        /*
         * CN_ENTER:
         *      double-click on tree record: intercept
         *      folders so we can influence the tree
         *      view on the right.
         */

        case CN_ENTER:
        {
            PNOTIFYRECORDENTER pnre;
            PMINIRECORDCORE prec;

            if (    (pnre = (PNOTIFYRECORDENTER)mp2)
                 && (prec = (PMINIRECORDCORE)pnre->pRecord)
                            // can be null for whitespace!
                 && (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                 && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
               )
            {
                // send SN_OBJECTENTER to the frame to see if
                // the frame wants a custom action. If that
                // returns TRUE, that means "processed", and
                // we do nothing (file dlg processes this itself).
                // Otherwise we try to be smart and do something.
                if (!SplitSendWMControl(pctl,
                                        SN_OBJECTENTER,
                                        (MPARAM)prec))
                {
                    WPObject *pobj;
                    if (    (pobj = fdrvGetFSFromRecord(prec,
                                                        TRUE))       // folders only:
                         && (pctl->precFilesShowing)
                       )
                    {
                        // double click on folder:
                        // if this is a _direct_ subfolder of the folder
                        // that we are currently displaying, that's easy
                        if (_wpQueryFolder(pobj) == OBJECT_FROM_PREC(pctl->precFilesShowing))
                        {
                            WinPostMsg(pctl->cvTree.hwndCnr,
                                       CM_EXPANDTREE,
                                       (MPARAM)prec,
                                       MPNULL);
                            WinPostMsg(pctl->cvTree.hwndCnr,
                                       CM_SETRECORDEMPHASIS,
                                       (MPARAM)prec,
                                       MPFROM2SHORT(CRA_SELECTED, CRA_SELECTED));
                        }
                        else
                            // not a direct child (shadow probably):
                            // call parent (have WPS open default view)
                            *pfCallDefault = TRUE;
                    }
                    else
                        // not folder:
                        *pfCallDefault = TRUE;
                }
                // else frame handled this message: do nothing,
                // do not even call parent
            }
        }
        break;

        /*
         * CN_CONTEXTMENU:
         *      we need to intercept this for context menus
         *      on whitespace, because the WPS won't do it.
         *      We pass all other cases on because the WPS
         *      does do things correctly for object menus.
         */

        case CN_CONTEXTMENU:
        {
            *pfCallDefault = TRUE;

            if (    (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                 && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                    // whitespace?
                 && (!mp2)
               )
            {
                // ok, this is a whitespace menu: if the view is
                // NOT populated, then just swallow for safety
                if (pctl->precFilesShowing)
                {
                    POINTL  ptl;
                    WinQueryPointerPos(HWND_DESKTOP, &ptl);
                    // convert to cnr coordinates
                    WinMapWindowPoints(HWND_DESKTOP,        // from
                                       pctl->cvFiles.hwndCnr,   // to
                                       &ptl,
                                       1);
                    _wpDisplayMenu(OBJECT_FROM_PREC(pctl->precFilesShowing),
                                   pctl->hwndFilesFrame, // owner
                                   pctl->cvFiles.hwndCnr,   // parent
                                   &ptl,
                                   MENU_OPENVIEWPOPUP,
                                   0);
                }

                *pfCallDefault = FALSE;
            }
        }
        break;

        default:
            *pfCallDefault = TRUE;
    }

    return mrc;
}

/*
 *@@ FilesFrameCommand:
 *      implementation for WM_COMMAND in fnwpFilesFrame.
 *
 *      Set *pfCallDefault to TRUE if you want the
 *      parent window proc to be called.
 *
 *@@added V1.0.1 (2002-12-08) [umoeller]
 */

STATIC VOID FilesFrameCommand(HWND hwndFrame,
                              MPARAM mp1,
                              MPARAM mp2,
                              PBOOL pfCallDefault)
{
    ULONG id = (ULONG)mp1;
    HWND                hwndMainControl;
    PSPLITCONTROLLER    pctl;

    if (    (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
         && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
       )
    {
        ULONG   ulNewView = 0;

        if (id == *G_pulVarMenuOfs + ID_XFMI_OFS_SMALLICONS)
        {
            // "small icons" button from tool bar:

            CNRINFO ci;
            cnrhQueryCnrInfo(pctl->cvFiles.hwndCnr, &ci);
            // make sure we are in icon view
            if (!(ci.flWindowAttr & CV_DETAIL))
            {
                ci.flWindowAttr ^= CV_MINI;
                _xwpModifyXFolderStyle(pctl->pRootsFolder,
                                       XFFL_SPLIT_NOMINI,
                                       (ci.flWindowAttr & CV_MINI)
                                            ? 0
                                            : XFFL_SPLIT_NOMINI,
                                       FALSE);      // no auto refresh
                WinSendMsg(pctl->cvFiles.hwndCnr,
                           CM_SETCNRINFO,
                           (MPARAM)&ci,
                           (MPARAM)CMA_FLWINDOWATTR);
            }
        }
        else if (id == WPMENUID_CHANGETOICON)
        {
            ulNewView = OPEN_CONTENTS;
        }
        else if (id == WPMENUID_CHANGETODETAILS)
        {
            ulNewView = OPEN_DETAILS;
        }
        else
            *pfCallDefault = TRUE;

        if (ulNewView)
        {
            _xwpModifyXFolderStyle(pctl->pRootsFolder,
                                   XFFL_SPLIT_DETAILS,
                                   (ulNewView == OPEN_DETAILS)
                                        ? XFFL_SPLIT_DETAILS
                                        : 0,
                                   FALSE);      // no auto refresh
            fdrvSetupView(&pctl->cvFiles,
                          NULL,     // no folder change
                          ulNewView,
                          !(_xwpQueryXFolderStyle(pctl->pRootsFolder) & XFFL_SPLIT_NOMINI));

            RefreshToolbarButtons(pctl,
                                  ulNewView);
        }
    }
}

/*
 *@@ fnwpFilesFrame:
 *      subclassed frame window on the right for the
 *      "Files" container. This has the tree cnr
 *      as its FID_CLIENT.
 *
 *      We use the XFolder subclassed window proc for
 *      most messages. In addition, we intercept a
 *      couple more for extra features.
 *
 *@@changed V1.0.0 (2002-11-23) [umoeller]: brought back keyboard support
 *@@changed V1.0.6 (2006-12-06) [pr]: added timer 1 delay @@fixes 326
 */

STATIC MRESULT EXPENTRY fnwpFilesFrame(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT             mrc = 0;

    TRY_LOUD(excpt1)
    {
        BOOL                fCallDefault = FALSE;
        PSUBCLFOLDERVIEW    psfv = WinQueryWindowPtr(hwndFrame, QWL_USER);
        HWND                hwndMainControl;
        PSPLITCONTROLLER pctl;

        switch (msg)
        {
            case WM_CONTROL:
                if (SHORT1FROMMP(mp1) == FID_CLIENT)     // that's the container
                    mrc = FilesFrameControl(hwndFrame,
                                            mp1,
                                            mp2,
                                            &fCallDefault);
                else
                    fCallDefault = TRUE;
            break;

            case WM_COMMAND:
                FilesFrameCommand(hwndFrame,
                                  mp1,
                                  mp2,
                                  &fCallDefault);
            break;

            // V1.0.6 (2006-12-06) [pr]
            case WM_TIMER:
                if (    ((ULONG)mp1 == 1)
                     && (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                     && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                   )
                {
                    PMPF_SPLITVIEW(("WM_TIMER 1: posting STBM_UPDATESTATUSBAR to hwnd %lX",
                                pctl->xfc.hwndStatusBar ));

                    WinStopTimer(pctl->habGUI,
                                 hwndFrame,
                                 1);

                    if (   // notifications not disabled?
                           (pctl->fSplitViewReady)
                        && (pctl->precFilesShowing)
                        && (pctl->xfc.hwndStatusBar)
                       )
                    {
                        // have the status bar updated and make
                        // sure the status bar retrieves its info
                        // from the _right_ cnr
                        pctl->hwndFocusCnr = pctl->cvFiles.hwndCnr;  // V1.0.9 (2012-02-12) [pr]
                        WinPostMsg(pctl->xfc.hwndStatusBar,
                                   STBM_UPDATESTATUSBAR,
                                   (MPARAM)pctl->hwndFocusCnr,
                                   MPNULL);
                    }
                }
            break;

            case WM_SYSCOMMAND:
                // forward to main frame
                WinPostMsg(WinQueryWindow(WinQueryWindow(hwndFrame,
                                                         QW_OWNER),
                                          QW_OWNER),
                           msg,
                           mp1,
                           mp2);
            break;

            case WM_CHAR:
                fCallDefault = !HandleWMChar(hwndFrame, mp1, mp2);
            break;

            /*
             * WM_QUERYOBJECTPTR:
             *      we receive this message from the WPS somewhere
             *      when it tries to process menu items for the
             *      whitespace context menu. I guess normally this
             *      message is taken care of when a frame is registered
             *      as a folder view, but this frame is not. So we must
             *      answer by returning the folder that this frame
             *      represents.
             *
             *      Answering this message will enable all WPS whitespace
             *      magic: both displaying the whitespace context menu
             *      and making messages work for the whitespace menu items.
             */

            case WM_QUERYOBJECTPTR:
                PMPF_SPLITVIEW(("WM_QUERYOBJECTPTR"));
                if (    (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                     && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                     // return the showing folder only if it is done
                     // populating
                     && (pctl->precFilesShowing)
                   )
                    mrc = (MRESULT)OBJECT_FROM_PREC(pctl->precFilesShowing);
                // else return default null
            break;

            /*
             * WM_CONTROLPOINTER:
             *      show wait pointer if we're busy.
             */

            case WM_CONTROLPOINTER:
                if (    (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                     && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                   )
                    mrc = (MPARAM)fdrSplitQueryPointer(pctl);
            break;

            /*
             *@@ FM_DELETINGFDR:
             *      this message is sent from XFolder::wpUnInitData
             *      if it finds the ID_XFMI_OFS_SPLIT_SUBFILES
             *      useitem in the folder. In other words, the
             *      folder whose contents are currently showing
             *      in the files cnr is about to go dormant.
             *      We must null the pointers in the splitviewdata
             *      that point to ourselves, or we'll have endless
             *      problems.
             *
             *@@added V1.0.0 (2002-08-28) [umoeller]
             */

            case FM_DELETINGFDR:
                PMPF_SPLITVIEW(("FM_DELETINGFDR"));
                if (    (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                     && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                   )
                {
                    pctl->pobjUseList = NULL;
                    pctl->precFilesShowing = NULL;
                }
            break;

            /*
             *@@ FM_FDRBACKGROUNDCHANGED:
             *      this message gets posted (!) when the folder
             *      view needs to have its container background
             *      set. This gets posted from two locations:
             *
             *      --  fnwpSplitController when FM_FILLFOLDER
             *          comes in to set the background before
             *          we start populate, but only for the
             *          right files cnr frame;
             *
             *      --  our XFolder::wpRedrawFolderBackground
             *          override when folder background settings
             *          change, for both frames.
             *
             *      Note: Both fnwpFilesFrame and fnwpSplitViewFrame
             *      react to this message, depending on which
             *      backgrounds needs to be updated.
             *
             *      Parameters:
             *
             *      --  WPFolder *mp1: folder that the container
             *          represents. This must be the root folder
             *          if we're displaying a disk object.
             *
             *      No return value.
             *
             *@@added V1.0.0 (2002-09-24) [umoeller]
             *@@changed V1.0.1 (2002-11-30) [umoeller]: renamed msg from FM_SETCNRLAYOUT
             */

            case FM_FDRBACKGROUNDCHANGED:
                PMPF_SPLITVIEW(("FM_FDRBACKGROUNDCHANGED"));
                if (    (hwndMainControl = WinQueryWindow(hwndFrame, QW_OWNER))
                     && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                     && (mp1)
                     && (pctl->flSplit & SPLIT_FDRSTYLES)
                   )
                {
                    PMPF_SPLITVIEW(("pctl->pobjUseList [%s]", _wpQueryTitle(pctl->pobjUseList)));

                    fdrvSetCnrLayout(pctl->cvFiles.hwndCnr,
                                     (WPFolder*)mp1,
                                     pctl->cvFiles.ulView);
                }
            break;

            default:
                fCallDefault = TRUE;
        }

        if (fCallDefault)
            mrc = fdrProcessFolderMsgs(hwndFrame,
                                       msg,
                                       mp1,
                                       mp2,
                                       psfv,
                                       psfv->pfnwpOriginal);
    }
    CATCH(excpt1) {} END_CATCH();

    return mrc;
}

/*
 *@@ fdrSplitCreateFrame:
 *      creates a WC_FRAME with the split view controller
 *      (fnwpSplitController) as its FID_CLIENT.
 *
 *      The caller is responsible for allocating a FDRSPLITVIEW
 *      structure and passing it in. The struct is zeroed here
 *      and then filled with meaningful data.
 *
 *      The caller must also subclass the frame that is
 *      returned and free that structure on WM_DESTROY.
 *
 *      The frame is created invisible with an undefined
 *      position. The handle of the frame window is
 *      stored in pctl->hwndMainFrame, if TRUE is returned.
 *
 *      With flSplit, pass in any combination of the
 *      following:
 *
 *      --  SPLIT_ANIMATE: give the frame the WS_ANIMATE
 *          style.
 *
 *      --  SPLIT_FDRSTYLES: make the two containers
 *          replicate the exact styles (backgrounds,
 *          fonts, colors) of the folders they are
 *          currently displaying. This is CPU-intensive,
 *          but pretty.
 *
 *      --  SPLIT_MULTIPLESEL: make the files cnr support
 *          multiple selections. Otherwise the files cnr
 *          will only allow one object at a time to be
 *          selected.
 *
 *      --  SPLIT_STATUSBAR: create a status bar below
 *          the split view. Note that is the responsibility
 *          of the caller then to position the status bar.
 *
 *      --  SPLIT_MENUBAR: create a menu bar for the frame.
 *
 *      --  SPLIT_NOAUTOPOSITION: force the split controller
 *          to not automatically reposition the subwindows
 *          in FDRSPLITVIEW. You must then explicitly
 *          position everything on WM_WINDOWPOSCHANGED.
 *          (File dlg uses this to control the extra buttons.)
 *
 *      --  SPLIT_NOAUTOPOPOPULATE: do not automatically
 *          populate the view with the root folder.
 *          (File dlg needs this to force populate to a
 *          subtree on startup.)
 *
 *@@added V1.0.0 (2002-08-28) [umoeller]
 *@@changed V1.0.1 (2002-12-08) [umoeller]: adjustments for new fdrvSetupView code
 *@@changed V1.0.1 (2003-01-17) [umoeller]: added pfnwpSubclass to move subclassing in here
 */

BOOL fdrSplitCreateFrame(WPObject *pRootObject,
                         WPFolder *pRootsFolder,
                         PSPLITCONTROLLER pctl,
                         ULONG flFrame,
                         PFNWP pfnwpSubclass,
                         PCSZ pcszTitle,
                         ULONG flSplit,
                         PCSZ pcszFileMask,
                         LONG lSplitBarPos)
{
    static      s_fRegistered = FALSE;

    if (!s_fRegistered)
    {
        // first call: register the controller class
        s_fRegistered = TRUE;

        WinRegisterClass(winhMyAnchorBlock(),
                         (PSZ)WC_SPLITCONTROLLER,
                         fnwpSplitController,
                         CS_SIZEREDRAW,
                         sizeof(PVOID));
    }

    ZERO(pctl);

    pctl->cbStruct = sizeof(*pctl);
    pctl->lSplitBarPos = lSplitBarPos;
    pctl->pRootObject = pRootObject;
    pctl->pRootsFolder = pRootsFolder;
    pctl->flSplit = flSplit;

    pctl->pcszFileMask = pcszFileMask;

    if (pctl->hwndMainFrame = winhCreateStdWindow(HWND_DESKTOP, // parent
                                                  NULL,         // pswp
                                                  flFrame,
                                                  (flSplit & SPLIT_ANIMATE)
                                                     ? WS_ANIMATE   // frame style, not yet visible
                                                     : 0,
                                                  pcszTitle,
                                                  0,            // resids
                                                  WC_SPLITCONTROLLER,
                                                  WS_VISIBLE,   // client style
                                                  0,            // frame ID
                                                  pctl,
                                                  &pctl->hwndMainControl))
    {
        PMINIRECORDCORE pRootRec;
        POINTL  ptlIcon = {0, 0};

        pctl->habGUI = WinQueryAnchorBlock(pctl->hwndMainFrame);

        if (flSplit & SPLIT_MENUBAR)
            WinLoadMenu(pctl->hwndMainFrame,
                        cmnQueryNLSModuleHandle(FALSE),
                        ID_XFM_SPLITVIEWBAR);

        if (flSplit & SPLIT_STATUSBAR)
        {
            pctl->xfc.hwndStatusBar = stbCreateBar(pRootsFolder,
                                                   pRootObject,
                                                   pctl->hwndMainFrame,
                                                   pctl->cvTree.hwndCnr);
            pctl->xfc.lStatusBarHeight = cmnQueryStatusBarHeight();
        }

        if (flSplit & SPLIT_TOOLBAR)  // V1.0.1 (2002-11-30) [umoeller]
            pctl->xfc.hwndToolBar = stbCreateToolBar(pRootsFolder,
                                                     pctl->hwndMainFrame,
                                                     *G_pulVarMenuOfs + ID_XFMI_OFS_SPLITVIEW,
                                                     &pctl->xfc.lToolBarHeight);

        // insert somSelf as the root of the tree
        pRootRec = _wpCnrInsertObject(pRootObject,
                                      pctl->cvTree.hwndCnr,
                                      &ptlIcon,
                                      NULL,       // parent record
                                      NULL);      // RECORDINSERT

        // _wpCnrInsertObject subclasses the container owner,
        // so subclass this with the XFolder subclass
        // proc again; otherwise the new menu items
        // won't work
        pctl->psfvTree = fdrCreateSFV(pctl->hwndTreeFrame,
                                     pctl->cvTree.hwndCnr,
                                     QWL_USER,
                                     FALSE,     // create no suppl. object window
                                     pRootsFolder,
                                     pRootObject);
        pctl->psfvTree->pfnwpOriginal = WinSubclassWindow(pctl->hwndTreeFrame,
                                                         fnwpTreeFrame);

        // same thing for files frame; however we need to
        // insert a temp object first to let the WPS subclass
        // the cnr owner first
        _wpCnrInsertObject(pRootsFolder,
                           pctl->cvFiles.hwndCnr,
                           &ptlIcon,
                           NULL,
                           NULL);
        pctl->psfvFiles = fdrCreateSFV(pctl->hwndFilesFrame,
                                      pctl->cvFiles.hwndCnr,
                                      QWL_USER,
                                      FALSE,            // create no suppl. object window
                                      pRootsFolder,
                                      pRootObject);
        pctl->psfvFiles->pfnwpOriginal = WinSubclassWindow(pctl->hwndFilesFrame,
                                                          fnwpFilesFrame);

        // remove the temp object again
        _wpCnrRemoveObject(pRootsFolder,
                           pctl->cvFiles.hwndCnr);

        if (pctl->flSplit & SPLIT_FDRSTYLES)
        {
            // subclass the containers for owner background drawing
            // (we must do after the WPS has subclassed above, or
            // it won't work)
            // V1.0.1 (2002-12-08) [umoeller]
            fdrvMakeCnrPaint(pctl->cvTree.hwndCnr);
            fdrvMakeCnrPaint(pctl->cvFiles.hwndCnr);
        }

        // now, after subclassing, switch the containers to the
        // correct view settings and set backgrounds and stuff
        // V1.0.1 (2002-12-08) [umoeller]
        PMPF_SPLITVIEW(("  calling fdrvSetupView for left half"));
        fdrvSetupView(&pctl->cvTree,
                      pctl->pRootsFolder,
                      OPEN_TREE,
                      TRUE);
        PMPF_SPLITVIEW(("  calling fdrvSetupView for right half"));
        fdrvSetupView(&pctl->cvFiles,
                      pctl->pRootsFolder,
                      (pctl->flSplit & SPLIT_DETAILS) ? OPEN_DETAILS : OPEN_CONTENTS,
                      !(pctl->flSplit & SPLIT_NOMINI));      // fmini

        if (!(flSplit & SPLIT_NOAUTOPOPOPULATE))
            // and populate this once we're running
            fdrPostFillFolder(pctl,
                              pRootRec,
                              // full populate, and expand tree on the left,
                              // and set background
                              FFL_SETBACKGROUND | FFL_EXPAND);

        if (pfnwpSubclass)
            pctl->xfc.pfnwpOrig = WinSubclassWindow(pctl->hwndMainFrame,
                                                    pfnwpSubclass);

        WinSendMsg(pctl->hwndMainFrame, WM_UPDATEFRAME, 0, 0);

        return TRUE;
    }

    return FALSE;
}

/*
 *@@ fdrSplitDestroyFrame:
 *      being the reverse to SplitCreate, this
 *      cleans up all allocated resources and destroys
 *      the windows, including the main frame.
 *
 *      Does NOT free pctl because we can't know how it was
 *      allocated. (As SplitCreate says, allocating
 *      and freeing the FDRSPLITVIEW is the job of the
 *      user code.)
 *
 *@@added V1.0.0 (2002-08-21) [umoeller]
 */

VOID fdrSplitDestroyFrame(PSPLITCONTROLLER pctl)
{
    // hide the main window, the cnr cleanup can
    // cause a lot of repaint, and for the file
    // dlg, the user should get immediate feedback
    // when the buttons are pressed
    WinShowWindow(pctl->hwndMainFrame, FALSE);

    // remove use item for right view
    if (pctl->pobjUseList)
        _wpDeleteFromObjUseList(pctl->pobjUseList,
                                &pctl->uiDisplaying);

    // stop threads; we crash if we exit
    // before these are stopped
    WinPostMsg(pctl->hwndSplitPopulate,
               WM_QUIT,
               0,
               0);
    pctl->tiSplitPopulate.fExit = TRUE;
    DosSleep(0);
    while (pctl->tidSplitPopulate)
        winhSleep(50);

    // prevent dialog updates
    pctl->fSplitViewReady = FALSE;

    fdrvClearContainer(pctl->cvTree.hwndCnr,
                       CLEARFL_TREEVIEW);
    fdrvClearContainer(pctl->cvFiles.hwndCnr,
                       // not tree view, but maybe unlock
                       (pctl->fUnlockOnClear)
                            ? CLEARFL_UNLOCKOBJECTS
                            : 0);

    // clean up
    winhDestroyWindow(&pctl->hwndSplitWindow);

    winhDestroyWindow(&pctl->hwndTreeFrame);
    winhDestroyWindow(&pctl->hwndFilesFrame);
    winhDestroyWindow(&pctl->hwndMainFrame);
}

/* ******************************************************************
 *
 *   Folder split view
 *
 ********************************************************************/

/*
 *@@ SPLITVIEWPOS:
 *
 *@@added V1.0.0 (2002-08-21) [umoeller]
 */

typedef struct _SPLITVIEWPOS
{
    LONG        x,
                y,
                cx,
                cy;

    LONG        lSplitBarPos;

} SPLITVIEWPOS, *PSPLITVIEWPOS;

/*
 *@@ SPLITVIEWDATA:
 *
 */

typedef struct _SPLITVIEWDATA
{
    USHORT              cb;

    USEITEM             ui;
    VIEWITEM            vi;

    CHAR                szFolderPosKey[10];

    SPLITCONTROLLER     ctl;        // pRootFolder == somSelf

    PSUBCLFOLDERVIEW    psfvBar;    // SUBCLFOLDERVIEW of either the three
                                    // or the files frame for menu bar support

} SPLITVIEWDATA, *PSPLITVIEWDATA;

/*
 *@@ LockSplitViewList:
 *      locks the global linklist of open split views.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

STATIC BOOL LockSplitViewList(VOID)
{
    if (G_hmtxOpenSplitViews)
        return !DosRequestMutexSem(G_hmtxOpenSplitViews, SEM_INDEFINITE_WAIT);

    // first call:
    lstInit(&G_llOpenSplitViews, FALSE);         // auto-free
    return !DosCreateMutexSem(NULL,
                              &G_hmtxOpenSplitViews,
                              0,
                              TRUE);      // request!
}

/*
 *@@ UnlockSplitViewList:
 *      unlocks the global linklist of open split views.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

STATIC VOID UnlockSplitViewList(VOID)
{
    DosReleaseMutexSem(G_hmtxOpenSplitViews);
}

/*
 *@@ SplitFrameInitMenu:
 *      implementation of WM_INITMENU in fnwpSplitViewFrame.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

STATIC MRESULT SplitFrameInitMenu(HWND hwndFrame,
                                  MPARAM mp1,
                                  MPARAM mp2)
{
    MRESULT mrc = 0;
    PSPLITVIEWDATA psvd;

    if (psvd = (PSPLITVIEWDATA)WinQueryWindowPtr(hwndFrame,
                                                 QWL_USER))
    {
        ULONG ulMenuType = 0;

        mrc = psvd->ctl.xfc.pfnwpOrig(hwndFrame, WM_INITMENU, mp1, mp2);

        // init menu doesn't recognize our new menu
        // bars, so check for these explicitly

        switch (SHORT1FROMMP(mp1))
        {
            case ID_XFM_BAR_FOLDER:
                PMPF_SPLITVIEW(("ID_XFM_BAR_FOLDER, (HWND)mp2 is 0x%lX", mp2));
                ulMenuType = MENU_FOLDERPULLDOWN;

                // this operates on the root folder
                psvd->psfvBar = psvd->ctl.psfvTree;
            break;

            case ID_XFM_BAR_EDIT:
                PMPF_SPLITVIEW(("ID_XFM_BAR_EDIT, (HWND)mp2 is 0x%lX", mp2));
                ulMenuType = MENU_EDITPULLDOWN;

                // this operates on the folder whose
                // contents are showing on the right
                psvd->psfvBar = psvd->ctl.psfvFiles;
            break;

            case ID_XFM_BAR_VIEW:
                PMPF_SPLITVIEW(("ID_XFM_BAR_VIEW, (HWND)mp2 is 0x%lX", mp2));
                ulMenuType = MENU_VIEWPULLDOWN;

                // this operates on the folder whose
                // contents are showing on the right
                psvd->psfvBar = psvd->ctl.psfvFiles;
            break;

            case ID_XFM_BAR_SELECTED:
                PMPF_SPLITVIEW(("ID_XFM_BAR_SELECTED, (HWND)mp2 is 0x%lX", mp2));
                ulMenuType = MENU_SELECTEDPULLDOWN;

                // this operates on the folder whose
                // contents are showing on the right
                psvd->psfvBar = psvd->ctl.psfvFiles;
            break;

            case ID_XFM_BAR_HELP:
                PMPF_SPLITVIEW(("ID_XFM_BAR_HELP, (HWND)mp2 is 0x%lX", mp2));
                ulMenuType = MENU_HELPPULLDOWN;

                // this operates on the folder whose
                // contents are showing on the right
                psvd->psfvBar = psvd->ctl.psfvFiles;
            break;
        }

        if (ulMenuType)
        {
            HWND        hwndMenu;
            if (hwndMenu = _wpDisplayMenu(psvd->ctl.pRootsFolder,
                                          hwndFrame,
                                          psvd->ctl.cvFiles.hwndCnr,
                                          NULL,
                                          ulMenuType | MENU_NODISPLAY,
                                          0))
            {
                // empty the old pulldown
                winhClearMenu((HWND)mp2);

                winhMergeMenus((HWND)mp2,
                               MIT_END,
                               hwndMenu,
                               0);

                // call our own routine to hack in new items
                fdrManipulatePulldown(psvd->ctl.psfvFiles,
                                      (ULONG)mp1,
                                      (HWND)mp2);

                // since we used MENU_NODISPLAY, apparently
                // we have to delete the menu ourselves,
                // or we end up with thousands of menus
                // under HWND_DESKTOP
                winhDestroyWindow(&hwndMenu);
            }

            PMPF_SPLITVIEW(("    _wpDisplayMenu returned 0x%lX", hwndMenu));
        }

        // rare case, but if the user clicked on the title
        // bar then psvd->psfvBar is not set; to be safe,
        // set it to the root folder (tree view)
        if (!psvd->psfvBar)
            psvd->psfvBar = psvd->ctl.psfvTree;

        // call init menu in fdrsubclass.c for sounds
        // and content menu items support
        fdrInitMenu(psvd->psfvBar,
                    (ULONG)mp1,
                    (HWND)mp2);

    }

    return mrc;
}

/*
 *@@ SplitFrameMenuSelect:
 *      implementation of WM_MENUSELECT in fnwpSplitViewFrame.
 *
 *@@added V1.0.1 (2002-12-11) [umoeller]
 */

STATIC MRESULT SplitFrameMenuSelect(HWND hwndFrame,
                                    MPARAM mp1,
                                    MPARAM mp2)
{
    MRESULT mrc;
    PSPLITVIEWDATA psvd;

    psvd = (PSPLITVIEWDATA)WinQueryWindowPtr(hwndFrame,
                                             QWL_USER);

    // always call the default, in case someone else
    // is subclassing folders (ObjectDesktop?!?)
    mrc = psvd->ctl.xfc.pfnwpOrig(hwndFrame, WM_MENUSELECT, mp1, mp2);

    PMPF_MENUS(("WM_MENUSELECT: mp1 = %lX/%lX, mp2 = %lX, psvd 0x%lX",
                SHORT1FROMMP(mp1),
                SHORT2FROMMP(mp1),
                mp2,
                psvd));

    if (psvd)
    {
        PMPF_MENUS(("   psvd->psfvBar is 0x%lX", psvd->psfvBar));

        if (psvd->psfvBar)
        {
            BOOL fDismiss = TRUE;

            PMPF_MENUS(("   calling fdrMenuSelect"));

            // let this operate on the cnr that is the
            // target of this operation
            if (fdrMenuSelect(psvd->psfvBar,
                              mp1,
                              mp2,
                              &fDismiss))
                // processed: return the modified flag instead
                mrc = (MRESULT)fDismiss;
        }
    }

    return mrc;
}

/*
 *@@ SplitFrameControl:
 *      implementation of WM_CONTROL in fnwpSplitViewFrame.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 *@@changed V1.0.5 (2006-04-13) [pr]: Fix status bar update @@fixes 326
 */

STATIC MRESULT SplitFrameControl(HWND hwndFrame,
                                 MPARAM mp1,
                                 MPARAM mp2)
{
    MRESULT mrc = 0;
    PSPLITVIEWDATA psvd;
    USHORT id = SHORT1FROMMP(mp1);

    if (psvd = (PSPLITVIEWDATA)WinQueryWindowPtr(hwndFrame,
                                                 QWL_USER))
    {
        if (id == FID_CLIENT)
        {
            // control messages from split view controller:

            switch (SHORT2FROMMP(mp1))
            {
                case SN_VKEY:
                    switch ((USHORT)mp2)
                    {
                        case VK_TAB:
                        case VK_BACKTAB:
                        {
                            HWND hwndFocus = WinQueryFocus(HWND_DESKTOP);
                            if (hwndFocus == psvd->ctl.cvTree.hwndCnr)
                                hwndFocus = psvd->ctl.cvFiles.hwndCnr;
                            else
                                hwndFocus = psvd->ctl.cvTree.hwndCnr;

                            psvd->ctl.hwndFocusCnr = hwndFocus;  // V1.0.9 (2012-02-12) [pr]
                            WinSetFocus(HWND_DESKTOP, hwndFocus);
                            // V1.0.5 (2006-04-13) [pr]: Fix status bar update @@fixes 326
                            if (psvd->ctl.xfc.hwndStatusBar)
                                WinPostMsg(psvd->ctl.xfc.hwndStatusBar,
                                           STBM_UPDATESTATUSBAR,
                                           (MPARAM) hwndFocus,
                                           MPNULL);

                            mrc = (MRESULT)TRUE;
                        }
                        break;
                    }
                break;

                case SN_FRAMECLOSE:
                {
                    SWP swp;
                    SPLITVIEWPOS pos;

                    PMPF_SPLITVIEW(("WM_CONTROL + SN_FRAMECLOSE"));

                    // save window position
                    WinQueryWindowPos(psvd->ctl.hwndMainFrame,
                                      &swp);
                    pos.x = swp.x;
                    pos.y = swp.y;
                    pos.cx = swp.cx;
                    pos.cy = swp.cy;

                    pos.lSplitBarPos = ctlQuerySplitPos(psvd->ctl.hwndSplitWindow);

                    PrfWriteProfileData(HINI_USER,
                                        (PSZ)INIAPP_FDRSPLITVIEWPOS,
                                        psvd->szFolderPosKey,
                                        &pos,
                                        sizeof(pos));

                    // clear all containers, stop populate thread etc.;
                    // this destroys the frame, which in turn destroys
                    // us and frees psv
                    fdrSplitDestroyFrame(&psvd->ctl);
                }
                break;

                case SN_UPDATESTATUSBAR:
                    PMPF_STATUSBARS(("got SN_UPDATESTATUSBAR, mp2 is %d", mp2));

                    switch ((ULONG)mp2)
                    {
                        case 1: // show/hide status bars according to folder/Global settings;
                        {
                            BOOL fNewVisible = stbViewHasStatusBar(psvd->ctl.pRootsFolder,
                                                                   NULLHANDLE,  // frame wnd for desktop checks
                                                                   *G_pulVarMenuOfs + ID_XFMI_OFS_SPLITVIEW);


                            PMPF_STATUSBARS(("[%s] fOldVis %d, fNewVis %d",
                                             _wpQueryTitle(psvd->ctl.pRootsFolder),
                                             !!psvd->ctl.xfc.hwndStatusBar,
                                             fNewVisible));

                            if (fNewVisible != (psvd->ctl.xfc.hwndStatusBar != NULLHANDLE))
                            {
                                if (fNewVisible)
                                {
                                    psvd->ctl.xfc.hwndStatusBar = stbCreateBar(psvd->ctl.pRootsFolder,
                                                                               psvd->ctl.pRootObject,
                                                                               psvd->ctl.hwndMainFrame,
                                                                               psvd->ctl.cvTree.hwndCnr);
                                }
                                else
                                    winhDestroyWindow(&psvd->ctl.xfc.hwndStatusBar);

                                WinPostMsg(psvd->ctl.hwndMainFrame, WM_UPDATEFRAME, 0, 0);
                            }
                        }
                        break;
                    }
                break;
            }
        }
    }

    return mrc;
}

/*
 *@@ fnwpSplitViewFrame:
 *
 *@@added V1.0.0 (2002-08-21) [umoeller]
 *@@changed V1.0.1 (2002-11-30) [umoeller]: added toolbar support
 *@@changed V1.0.1 (2002-12-11) [umoeller]: added WM_MENUSELECT support and excpt handling
 */

MRESULT EXPENTRY fnwpSplitViewFrame(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;
    PSPLITVIEWDATA psvd;

    if (psvd = (PSPLITVIEWDATA)WinQueryWindowPtr(hwndFrame, QWL_USER))
    {
        TRY_LOUD(excpt1)        // added V1.0.1 (2002-12-11) [umoeller]
        {
            switch (msg)
            {
                /*
                 * WM_QUERYFRAMECTLCOUNT:
                 *      this gets sent to the frame when PM wants to find out
                 *      how many frame controls the frame has. According to what
                 *      we return here, SWP structures are allocated for WM_FORMATFRAME.
                 *      We call the "parent" window proc and add one for the status bar.
                 */

                case WM_QUERYFRAMECTLCOUNT:
                {
                    // query the standard frame controls count
                    ULONG ulCount = (ULONG)psvd->ctl.xfc.pfnwpOrig(hwndFrame, msg, mp1, mp2);

                    if (psvd->ctl.xfc.hwndStatusBar)
                        ulCount++;
                    if (psvd->ctl.xfc.hwndToolBar)
                        ulCount++;

                    mrc = (MRESULT)ulCount;
                }
                break;

                /*
                 * WM_FORMATFRAME:
                 *    this message is sent to a frame window to calculate the sizes
                 *    and positions of all of the frame controls and the client window.
                 *
                 *    Parameters:
                 *          mp1     PSWP    pswp        structure array; for each frame
                 *                                      control which was announced with
                 *                                      WM_QUERYFRAMECTLCOUNT, PM has
                 *                                      allocated one SWP structure.
                 *          mp2     PRECTL  pprectl     pointer to client window
                 *                                      rectangle of the frame
                 *          returns USHORT  ccount      count of the number of SWP
                 *                                      arrays returned
                 *
                 *    It is the responsibility of this message code to set up the
                 *    SWP structures in the given array to position the frame controls.
                 *    We call the "parent" window proc and then set up our status bar.
                 */

                case WM_FORMATFRAME:
                    //  query the number of standard frame controls
                    // use comctl.c function now V1.0.1 (2002-11-30) [umoeller]
                    PMPF_STATUSBARS(("hwndStatusBar 0x%lX, hwndToolBar 0x%lX",
                                     psvd->ctl.xfc.hwndStatusBar, psvd->ctl.xfc.hwndToolBar));
                    mrc = ctlFormatExtFrame(hwndFrame,
                                            &psvd->ctl.xfc,
                                            mp1,
                                            mp2);
                break;

                /*
                 * WM_CALCFRAMERECT:
                 *     this message occurs when an application uses the
                 *     WinCalcFrameRect function.
                 *
                 *     Parameters:
                 *          mp1     PRECTL  pRect      rectangle structure
                 *          mp2     USHORT  usFrame    frame indicator
                 *          returns BOOL    rc         rectangle-calculated indicator
                 */

                case WM_CALCFRAMERECT:
                    ctlCalcExtFrameRect(hwndFrame,
                                        &psvd->ctl.xfc,
                                        mp1,
                                        mp2);
                break;

                /*
                 * WM_INITMENU:
                 *      fill the menu bar pulldowns.
                 */

                case WM_INITMENU:
                    mrc = SplitFrameInitMenu(hwndFrame, mp1, mp2);
                break;

                /*
                 * WM_MENUSELECT:
                 *      handle sticky menus for the frame.
                 *      V1.0.1 (2002-12-11) [umoeller]
                 */

                case WM_MENUSELECT:
                    mrc = SplitFrameMenuSelect(hwndFrame, mp1, mp2);
                break;

                /*
                 * WM_MEASUREITEM:
                 *      same processing as in fdrProcessFolderMsgs.
                 */

                case WM_MEASUREITEM:
                    if (   (SHORT)mp1
                         > *G_pulVarMenuOfs + ID_XFMI_OFS_VARIABLE
                       )
                    {
                        // call the measure-item func in fdrmenus.c
                        mrc = cmnuMeasureItem((POWNERITEM)mp2);
                    }
                    else
                        // none of our items: pass to original wnd proc
                        mrc = psvd->ctl.xfc.pfnwpOrig(hwndFrame, msg, mp1, mp2);
                break;

                /*
                 * WM_DRAWITEM:
                 *      same processing as in fdrProcessFolderMsgs,
                 *      except that we do not need cnr owner draw
                 *      for FID_CLIENT.
                 */

                case WM_DRAWITEM:
                    if (   (SHORT)mp1
                         > *G_pulVarMenuOfs + ID_XFMI_OFS_VARIABLE
                       )
                    {
                        // variable menu item: this must be a folder-content
                        // menu item, because for others no WM_DRAWITEM is sent
                        if (cmnuDrawItem(mp1, mp2))
                            mrc = (MRESULT)TRUE;
                    }
                    else
                        mrc = psvd->ctl.xfc.pfnwpOrig(hwndFrame, msg, mp1, mp2);
                break;

                /*
                 * WM_COMMAND:
                 *
                 */

                case WM_COMMAND:
                {
                    PSUBCLFOLDERVIEW psfv = psvd->psfvBar;
                    ULONG id = SHORT1FROMMP(mp1);

                    if (    (id == *G_pulVarMenuOfs + ID_XFMI_OFS_SMALLICONS)
                         || (id == WPMENUID_CHANGETOICON)
                         || (id == WPMENUID_CHANGETODETAILS)
                       )
                    {
                        // forward to files frame
                        WinPostMsg(psvd->ctl.hwndFilesFrame,
                                   WM_COMMAND,
                                   mp1,
                                   mp2);
                    }
                    else
                        // invoke the command on the folder who currently
                        // has the focus
                        if (    (!psfv)
                             || (!fdrWMCommand(psvd->psfvBar,
                                               id))
                           )
                            // not processed:
                            mrc = psvd->ctl.xfc.pfnwpOrig(hwndFrame, msg, mp1, mp2);
                }
                break;

                /*
                 *@@ WM_CONTROL:
                 *      intercept SN_FRAMECLOSE.
                 *      V1.0.0 (2002-09-13) [umoeller]
                 */

                case WM_CONTROL:
                    mrc = SplitFrameControl(hwndFrame,
                                            mp1,
                                            mp2);
                break;

                /*
                 * FM_FDRBACKGROUNDCHANGED:
                 *      this message gets posted (!) when the folder
                 *      view needs to have its container background
                 *      set. See FM_SETCNRLAYOUT in fnwpFilesFrame.
                 *
                 *      We only get this message _here_ if our
                 *      XFolder::wpRedrawFolderBackground detects
                 *      that folder background settings have
                 *      changed.
                 *
                 *      Parameters:
                 *
                 *      --  WPFolder *mp1: folder that the container
                 *          represents. This must be the root folder
                 *          if we're displaying a disk object.
                 *
                 *      No return code.
                 *
                 */

                case FM_FDRBACKGROUNDCHANGED:
                    PMPF_SPLITVIEW(("FM_FDRBACKGROUNDCHANGED"));
                    if (    (psvd->ctl.flSplit & SPLIT_FDRSTYLES)
                         && (mp1)
                       )
                    {
                        fdrvSetCnrLayout(psvd->ctl.cvTree.hwndCnr,
                                         (WPFolder*)mp1,
                                         OPEN_TREE);
                    }
                break;

                /*
                 * WM_DESTROY:
                 *
                 */

                case WM_DESTROY:
                    // maintain list of open split views V1.0.1 (2002-11-30) [umoeller]
                    if (LockSplitViewList())
                    {
                        PLISTNODE pNode;
                        FOR_ALL_NODES(&G_llOpenSplitViews, pNode)
                        {
                            if (pNode->pItemData == psvd)
                            {
                                lstRemoveNode(&G_llOpenSplitViews, pNode);
                                break;
                            }
                        }

                        UnlockSplitViewList();
                    }

                    _wpDeleteFromObjUseList(psvd->ctl.pRootObject,
                                            &psvd->ui);

                    winhDestroyWindow(&psvd->ctl.xfc.hwndStatusBar);
                    winhDestroyWindow(&psvd->ctl.xfc.hwndToolBar);  // V1.0.1 (2002-11-30) [umoeller]

                    _wpFreeMem(psvd->ctl.pRootObject,
                               (PBYTE)psvd);

                    mrc = psvd->ctl.xfc.pfnwpOrig(hwndFrame, msg, mp1, mp2);
                break;

                default:
                    mrc = psvd->ctl.xfc.pfnwpOrig(hwndFrame, msg, mp1, mp2);
            }
        }
        CATCH(excpt1)
        {
            mrc = 0;
        } END_CATCH();
    }

    return mrc;
}

/*
 *@@ fdrQueryOpenFolders:
 *      implementation for M_XFolder::xwpclsQueryOpenFolders.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

WPFolder* fdrQueryOpenFolders(WPFolder *pFind,
                              BOOL fLock)
{
    LINKLIST    ll;
    WPFolder    *pReturn = NULL,
                *pThis;
    PLISTNODE   pNode,
                pNode2;
    BOOL        fLocked = FALSE;

    lstInit(&ll, FALSE);

    // _PmpfF(("entering"));

    TRY_LOUD(excpt1)
    {
        // step 1: get the WPS list (icon, details, and tree views)
        for (pThis = _wpclsQueryOpenFolders(_WPFolder, NULL, QC_FIRST, TRUE);
             pThis;
             pThis = _wpclsQueryOpenFolders(_WPFolder, pThis, QC_NEXT, TRUE))
        {
            lstAppendItem(&ll, pThis);
        }

        // _PmpfF(("  got %d folders from WPS", lstCountItems(&ll)));

        // step 2: blend in open split views
        if (fLocked = LockSplitViewList())
        {
            FOR_ALL_NODES(&G_llOpenSplitViews, pNode)
            {
                PSPLITVIEWDATA psvd = (PSPLITVIEWDATA)pNode->pItemData;
                BOOL fFound = FALSE;

                // check if this folder is in the list already
                FOR_ALL_NODES(&ll, pNode2)
                {
                    if ((WPFolder*)pNode2->pItemData == psvd->ctl.pRootsFolder)
                    {
                        fFound = TRUE;
                        break;
                    }
                }

                if (!fFound)
                    lstAppendItem(&ll, psvd->ctl.pRootsFolder);
            }
        }

        // _PmpfF(("  got %d folders including split views", lstCountItems(&ll)));

        // step 3: find pFolder in the list
        if (!pFind)
        {
            // caller wants first:
            // _PmpfF(("  caller wants first"));

            if (pNode = lstQueryFirstNode(&ll))
                pReturn = pNode->pItemData;
        }
        else
        {
            // caller wants next:

            // _PmpfF(("  caller wants next after %s", _wpQueryTitle(pFind)));

            FOR_ALL_NODES(&ll, pNode)
            {
                if ((WPFolder*)pNode->pItemData == pFind)
                {
                    if (pNode = pNode->pNext)
                        pReturn = (WPFolder*)pNode->pItemData;

                    break;
                }
            }
        }

        // _PmpfF(("  found %s to return", (pReturn) ? _wpQueryTitle(pReturn) : "NULL"));

        FOR_ALL_NODES(&ll, pNode)
        {
            if (    (!fLock)
                 || ((WPFolder*)pNode->pItemData != pReturn)
               )
            _wpUnlockObject((WPFolder*)pNode->pItemData);
        }

        lstClear(&ll);
    }
    CATCH(excpt1) {} END_CATCH();

    if (fLocked)
        UnlockSplitViewList();

    return pReturn;
}

/*
 *@@ fdrCreateSplitView:
 *      creates a frame window with a split window and
 *      does the usual "register view and pass a zillion
 *      QWL_USER pointers everywhere" stuff. This is
 *      the implementation of the "real" folder split
 *      view opened for a given folder.
 *
 *      The "real" folder split view uses the split view
 *      controller, which is shared code between this
 *      and the file dialog (filedlg.c). See
 *
 *      Returns the frame window or NULLHANDLE on errors.
 *
 *@@changed V1.0.1 (2002-11-30) [umoeller]: status bar fix
 */

HWND fdrCreateSplitView(WPObject *pRootObject,      // in: folder or disk
                        WPFolder *pRootsFolder,     // in: folder or disk's root folder
                        ULONG ulView)
{
    HWND        hwndReturn = NULLHANDLE;

    WPFolder    *pFolder;

    TRY_LOUD(excpt1)
    {
        PSPLITVIEWDATA  psvd;
        ULONG           rc;

        // allocate our SPLITVIEWDATA, which contains the
        // FDRSPLITVIEW for the split view engine, plus
        // the useitem for this view
        if (psvd = (PSPLITVIEWDATA)_wpAllocMem(pRootObject,
                                               sizeof(SPLITVIEWDATA),
                                               &rc))
        {
            SPLITVIEWPOS pos;
            ULONG   cbPos;
            ULONG   flFrame = FCF_NOBYTEALIGN
                                  | FCF_TITLEBAR
                                  | FCF_SYSMENU
                                  | FCF_SIZEBORDER
                                  | FCF_AUTOICON
                                  | cmnQueryFCF(pRootObject);
                                        // V1.0.1 (2002-12-08) [umoeller]

            ULONG   flSplit,
                    flXFolder;

            ZERO(psvd);

            psvd->cb = sizeof(SPLITVIEWDATA);

            // try to restore window position, if present;
            // we put these in a separate XWorkplace app
            // because we're using a special format to
            // allow for saving the split position
            sprintf(psvd->szFolderPosKey,
                    "%lX",
                    _wpQueryHandle(pRootObject));
            cbPos = sizeof(pos);
            if (    (!(PrfQueryProfileData(HINI_USER,
                                           (PSZ)INIAPP_FDRSPLITVIEWPOS,
                                           psvd->szFolderPosKey,
                                           &pos,
                                           &cbPos)))
                 || (cbPos != sizeof(SPLITVIEWPOS))
               )
            {
                // no position stored yet:
                pos.x = (G_cxScreen - 600) / 2;
                pos.y = (G_cyScreen - 400) / 2;
                pos.cx = 600;
                pos.cy = 400;
                pos.lSplitBarPos = 30;
            }

            flSplit =   SPLIT_ANIMATE
                      | SPLIT_FDRSTYLES
                      | SPLIT_MULTIPLESEL;

            // if (stbFolderWantsStatusBars(pRootsFolder))
            // fixed V1.0.1 (2002-11-30) [umoeller]
            if (stbViewHasStatusBar(pRootsFolder,
                                    NULLHANDLE,
                                    *G_pulVarMenuOfs + ID_XFMI_OFS_SPLITVIEW))
                flSplit |= SPLIT_STATUSBAR;

            if (stbViewHasToolBar(pRootsFolder,
                                  NULLHANDLE,
                                  *G_pulVarMenuOfs + ID_XFMI_OFS_SPLITVIEW))
                flSplit |= SPLIT_TOOLBAR;

            if (_xwpQueryMenuBarVisibility(pRootsFolder))
                flSplit |= SPLIT_MENUBAR;

            // V1.0.1 (2002-11-30) [umoeller]
            flXFolder = _xwpQueryXFolderStyle(pRootsFolder);
            if (flXFolder & XFFL_SPLIT_DETAILS)
                flSplit |= SPLIT_DETAILS;
            if (flXFolder & XFFL_SPLIT_NOMINI)
                flSplit |= SPLIT_NOMINI;

            // create the frame and the client (shared code
            // with file dialog);
            // the client gets ctl in mp1 with WM_CREATE
            // and creates the split window and everything else
            if (fdrSplitCreateFrame(pRootObject,
                                    pRootsFolder,
                                    &psvd->ctl,
                                    flFrame,
                                    fnwpSplitViewFrame,
                                    "",
                                    flSplit,
                                    NULL,       // no file mask
                                    pos.lSplitBarPos))
            {
                // view-specific stuff:

                WinSetWindowPtr(psvd->ctl.hwndMainFrame,
                                QWL_USER,
                                psvd);

                // register the view
                cmnRegisterView(pRootObject,
                                &psvd->ui,
                                ulView,
                                psvd->ctl.hwndMainFrame,
                                cmnGetString(ID_XFSI_FDR_SPLITVIEW));

                // change the window title to full path, if allowed
                // (not for WPDisk)
                if (pRootObject == pRootsFolder)
                {
                    XFolderData *somThis = XFolderGetData(pRootObject);
                    if (    (_bFullPathInstance == 1)
                         || ((_bFullPathInstance == 2) && (cmnQuerySetting(sfFullPath)))
                       )
                        fdrSetOneFrameWndTitle(pRootObject, psvd->ctl.hwndMainFrame);
                }

                // now go show the damn thing
                WinSetWindowPos(psvd->ctl.hwndMainFrame,
                                HWND_TOP,
                                pos.x,
                                pos.y,
                                pos.cx,
                                pos.cy,
                                SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ACTIVATE | SWP_ZORDER);

                // and set focus to the tree view
                psvd->ctl.hwndFocusCnr = psvd->ctl.cvTree.hwndCnr;  // V1.0.9 (2012-02-12) [pr]
                WinSetFocus(HWND_DESKTOP,
                            psvd->ctl.hwndFocusCnr);

                // start processing notifications now
                psvd->ctl.fSplitViewReady = TRUE;

                // store this split view in the global list
                // for enumerating open folders
                // V1.0.1 (2002-11-30) [umoeller]
                if (LockSplitViewList())
                {
                    lstAppendItem(&G_llOpenSplitViews,
                                  psvd);

                    UnlockSplitViewList();
                }

                hwndReturn = psvd->ctl.hwndMainFrame;
            }
        }
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    return hwndReturn;
}

