
/*
 *@@sourcefile fops_bottom.c:
 *      this has the bottom layer of the XWorkplace file operations
 *      engine. See fileops.c for an introduction.
 *
 *      The bottom layer is independent of the GUI and uses
 *      callbacks for reporting progress and errors. This
 *      is different from the ugly WPS implementation where
 *      you get message boxes when calling a file operations
 *      method, and there's no way to suppress them. (Try
 *      _wpFree on a read-only file system object.)
 *
 *      Starting a file operation requires several steps:
 *
 *      1.  Create a "file task list" using fopsCreateFileTaskList.
 *          This tells the file operatations engine what to do.
 *          Also, you can specify progress and error callbacks.
 *
 *      2.  Add objects to the list using fopsAddObjectToTask.
 *          This already does preliminary checks whether the
 *          file operation specified with fopsCreateFileTaskList
 *          will work on the respective object. If not, your
 *          error callback gets called and can attempt to fix
 *          the error.
 *
 *      3.  Call fopsStartTask which processes the file task
 *          list on the XWorkplace File thread. You can choose
 *          to have processing done synchronously or asynchronously.
 *          During processing, your progress callback gets called.
 *          If errors occur during processing, your error callback
 *          will get called again, but this time, processing aborts.
 *
 *      The file-operations top layer (fops_top.c) is simply a
 *      wrapper around the top layer, implementing APIs for
 *      building certain file lists and also callbacks for
 *      GUI progress dialog support.
 *
 *      This file has been separated from fileops.c with
 *      V0.9.4 (2000-07-27) [umoeller].
 *
 *      Function prefix for this file:
 *      --  fops*
 *
 *@@added V0.9.4 (2000-07-27) [umoeller]
 *@@header "filesys\fileops.h"
 */

/*
 *      Copyright (C) 2000-2008 Ulrich M”ller.
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

#define INCL_WINWINDOWMGR
#define INCL_WININPUT
#define INCL_WINTIMER
#define INCL_WINPOINTERS
#define INCL_WINDIALOGS
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
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\stringh.h"            // string helper routines

// SOM headers which don't crash with prec. header files
// #include "xfobj.ih"
#include "xfldr.ih"
#include "xfont.ih"
#include "xfontfile.ih"
#include "xfontobj.ih"
#include "xtrash.ih"
#include "xtrashobj.ih"

// XWorkplace implementation headers
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)
#include "shared\classtest.h"           // some cheap funcs for WPS class checks

#include "config\fonts.h"               // font folder implementation

#include "filesys\fileops.h"            // file operations implementation
#include "filesys\folder.h"             // XFolder implementation
#include "filesys\object.h"             // XFldObject implementation
#include "filesys\trash.h"              // trash can implementation
#include "filesys\xthreads.h"           // extra XWorkplace threads

// other SOM headers
#pragma hdrstop                         // VAC++ keeps crashing otherwise

/*
 *@@ FILETASKLIST:
 *      internal file-task-list structure.
 *      This is what a HFILETASKLIST handle
 *      really points to.
 *
 *      A FILETASKLIST represents a common
 *      job to be performed on one or several
 *      objects. Such a structure is created
 *      by fopsCreateFileTaskList.
 *
 *@@added V0.9.1 (2000-01-27) [umoeller]
 *@@changed V0.9.2 (2000-03-04) [umoeller]: added error callback
 *@@changed V0.9.4 (2000-07-27) [umoeller]: added ulIgnoreSubsequent to ignore further errors
 *@@changed V1.0.1 (2002-12-15) [umoeller]: now using XWPObjList for the objects list
 */

typedef struct _FILETASKLIST
{
    // LINKLIST    llObjects;      // list of simple WPObject* pointers to be processed

    XWPObjList  *pllObjects;     // replaced V1.0.1 (2002-12-15) [umoeller]

    ULONG       ulOperation;    // operation; see fopsCreateFileTaskList.

    WPFolder    *pSourceFolder; // source folder for the operation; this gets locked
    BOOL        fSourceLocked;  // TRUE if wpRequestFolderMutexSem has been invoked
                                // on pSourceFolder

    WPFolder    *pTargetFolder; // target folder for the operation
    BOOL        fTargetLocked;  // TRUE if wpRequestFolderMutexSem has been invoked
                                // on pTargetFolder

    POINTL      ptlTarget;      // target coordinates in pTargetFolder

    FNFOPSPROGRESSCALLBACK *pfnProgressCallback;
                    // progress callback specified with fopsCreateFileTaskList
    FNFOPSERRORCALLBACK *pfnErrorCallback;
                    // error callback specified with fopsCreateFileTaskList

    ULONG       ulIgnoreSubsequent;
                    // ignore subsequent errors; this
                    // is set to one of the FOPS_ISQ_* flags
                    // if the error callback chooses to ignore
                    // subsequent errors

    ULONG       ulUser;                 // user parameter specified with fopsCreateFileTaskList
} FILETASKLIST, *PFILETASKLIST;

/*
 *@@ fopsCreateFileTaskList:
 *      creates a new task list for the given file operation
 *      to which items can be added using fopsAddFileTask.
 *      Part of the bottom layer of the XWorkplace file
 *      operations engine. Can be called on any thread.
 *
 *      To start such a file task, three steps are required:
 *
 *      -- call fopsCreateFileTaskList (this function);
 *
 *      -- add objects to that list using fopsAddObjectToTask;
 *
 *      -- pass the task to the File thread using fopsStartTask,
 *         OR delete the list using fopsDeleteFileTaskList.
 *
 *      WARNING: This function requests the folder mutex semaphore
 *      for pSourceFolder. (While this semaphore is held, no other
 *      thread can add or remove objects from the folder.) You must
 *      add objects to the returned task list (using fopsAddFileTask)
 *      as quickly as possible.
 *
 *      The mutex will only be released after all file processing
 *      has been completed (thru fopsStartTask) or if you call
 *      fopsDeleteFileTaskList explicitly. So you MUST call either
 *      one of those two functions after a task list was successfully
 *      created using this function.
 *
 *      Returns NULL upon errors, e.g. because a folder
 *      mutex could not be accessed.
 *
 *      This is usually called on the thread of the folder view
 *      from which the file operation was started (mostly thread 1).
 *      There are several easy-to-use frontends to this in the
 *      top layer (fops_top.c).
 *
 *      The following are supported for ulOperation:
 *
 *      -- XFT_MOVE2TRASHCAN: move list items into trash can.
 *          In that case, pSourceFolder has the source folder.
 *          This isn't really needed for processing, but the
 *          progress dialog should display this, so to speed
 *          up processing, pass it with this call.
 *          pTargetFolder is ignored because the default trash
 *          can is determined automatically.
 *
 *      -- XFT_RESTOREFROMTRASHCAN: restore objects from trash can.
 *          This expects trash objects (XWPTrashObject) on the list.
 *          In that case, pSourceFolder has the trash can to
 *          restore from (XWPTrashCan*).
 *          If (pTargetFolder == NULL), the objects will be restored
 *          to the respective locations where they were deleted from.
 *          If (pTargetFolder != NULL), the objects will all be moved
 *          to that folder.
 *
 *      -- XFT_DELETE: delete list items from pSourceFolder without
 *          moving them into the trash can first.
 *          In that case, pTargetFolder is ignored.
 *
 *          This is used with "true delete" (shift and "delete"
 *          context menu item). Besides, this is used to empty the
 *          trash can and to destroy trash objects; in these cases,
 *          this function assumes to be operating on the related
 *          objects, not the trash objects.
 *
 *      -- XFT_POPULATE: populate folders on list. pSourcefolder
 *          and pTargetFolder are ignored. Also you must specify
 *          both callbacks as NULL.
 *          fopsAddObjectToTask may only be used with folder objects.
 *          This operation is useful to populate folders synchronously
 *          without blocking the thread's message queue since
 *          fopsStartTask can avoid that.
 *
 *      -- XFT_INSTALLFONTS: install font files as PM fonts.
 *          pSourceFolder is ignored.
 *          pTargetFolder must be the default font folder.
 *          This expects font files (instances of XWPFontFile)
 *          on the list.
 *
 *      -- XFT_DEINSTALLFONTS: deinstall font objects.
 *          pSourceFolder must point to the XWPFontFolder containing
 *          the font objects.
 *          pTargetFolder is ignored.
 *          This expects font _objects_ (instances of XWPFontObject)
 *          on the list.
 *
 *      Move, copy, and create shadows are still missing... we can't
 *      do those until I have rewritten the ugly file handles
 *      management in the WPS.
 *
 *@@added V0.9.1 (2000-01-27) [umoeller]
 *@@changed V0.9.2 (2000-03-04) [umoeller]: added error callback
 *@@changed V0.9.4 (2000-08-03) [umoeller]: added XFT_POPULATE
 *@@changed V0.9.7 (2001-01-13) [umoeller]: added XFT_INSTALLFONTS, XFT_DEINSTALLFONTS
 *@@changed V0.9.16 (2001-11-25) [umoeller]: now returning APIRET
 *@@changed V0.9.16 (2001-11-25) [umoeller]: for XFT_MOVE2TRASHCAN, now checking if trash can exists
 */

APIRET fopsCreateFileTaskList(HFILETASKLIST *phftl,     // out: new file task list
                              ULONG ulOperation,     // in: XFT_* flag
                              WPFolder *pSourceFolder,
                              WPFolder *pTargetFolder,
                              FNFOPSPROGRESSCALLBACK *pfnProgressCallback, // in: callback procedure
                              FNFOPSERRORCALLBACK *pfnErrorCallback, // in: error callback
                              ULONG ulUser)          // in: user parameter passed to callback
{
    HFILETASKLIST   hftl = NULLHANDLE;
    APIRET          frc = NO_ERROR;

    if (    (ulOperation == XFT_MOVE2TRASHCAN)
            // check if the trash can exists
            // V0.9.16 (2001-11-10) [umoeller]
         && (    (!_XWPTrashCan)
              || (!_xwpclsQueryDefaultTrashCan(_XWPTrashCan))
            )
       )
        frc = FOPSERR_NO_TRASHCAN;
    else
    {
        BOOL            fSourceLocked = FALSE;

        if (pSourceFolder)
        {
            if (!(fSourceLocked = !_wpRequestFolderMutexSem(pSourceFolder, 5000)))
                frc = FOPSERR_LOCK_FAILED;
        }

        if (!frc)
        {
            PFILETASKLIST pftl;
            if (pftl = malloc(sizeof(FILETASKLIST)))
            {
                memset(pftl, 0, sizeof(FILETASKLIST));
                // lstInit(&pftl->llObjects, FALSE);     // no freeing, this stores WPObject pointers!
                pftl->pllObjects = _xwpclsCreateList(_XFldObject, NULL, NULL);
                            // V1.0.1 (2002-12-15) [umoeller]
                pftl->ulOperation = ulOperation;
                pftl->pSourceFolder = pSourceFolder;
                pftl->fSourceLocked = fSourceLocked;
                pftl->pTargetFolder = pTargetFolder;
                pftl->pfnProgressCallback = pfnProgressCallback;
                pftl->pfnErrorCallback = pfnErrorCallback;
                pftl->ulUser = ulUser;
                hftl = ((HFILETASKLIST)pftl);
                        // do not unlock pSourceFolder!
            }
            else
                frc = ERROR_NOT_ENOUGH_MEMORY;
        }

        if (!frc)
        {
            // no error:
            *phftl = hftl;
        }
        else
        {
            // error
            if (fSourceLocked)
                _wpReleaseFolderMutexSem(pSourceFolder);
        }
    }

    return frc;
}

/*
 *@@ fopsValidateObjOperation:
 *      returns 0 (NO_ERROR) only if ulOperation is valid
 *      on pObject.
 *      Part of the bottom layer of the XWorkplace file
 *      operations engine.
 *
 *      This must be run on the thread which called fopsCreateFileTaskList.
 *
 *      If an error has been found and (pfnErrorCallback != NULL),
 *      that callback gets called and its return value is returned.
 *      This means that pfnErrorCallback can attempt to fix the error.
 *      If the callback is NULL, some meaningfull error gets
 *      returned if the operation is invalid, but there's no chance
 *      to fix it.
 *
 *      This is usually called from fopsAddObjectToTask on the thread
 *      of the folder view from which the file operation was started
 *      (mostly thread 1) while objects are added to a file task list.
 *
 *      This can also be called at any time. For example, the trash can
 *      uses this from XWPTrashCan::wpDragOver to validate the objects
 *      being dragged.
 *
 *@@added V0.9.1 (2000-02-01) [umoeller]
 *@@changed V0.9.2 (2000-03-04) [umoeller]: added error callback
 *@@changed V0.9.3 (2000-04-25) [umoeller]: reworked error management
 *@@changed V0.9.4 (2000-07-27) [umoeller]: added pulIgnoreSubsequent to ignore further errors
 *@@changed V1.0.6 (2006-10-30) [pr]: prevent Transient/Printer objects going into Trash
 *@@changed V1.0.8 (2008-01-05) [pr]: prevent other untrashable objects going into Trash @@fixes 1035
 */

APIRET fopsValidateObjOperation(ULONG ulOperation,        // in: operation
                                FNFOPSERRORCALLBACK *pfnErrorCallback,
                                                          // in: error callback or NULL
                                WPObject *pObject,        // in: current object to check
                                PULONG pulIgnoreSubsequent)
                                    // in: ignore subsequent errors of the same type;
                                    // ULONG must be 0 on the first call; FOPS_ISQ_*
                                    // flags can be set by the error callback.
                                    // The ptr can only be NULL if pfnErrorCallback is
                                    // also NULL.
{
    APIRET  frc = NO_ERROR;
    BOOL    fPromptUser = TRUE;

    // error checking
    switch (ulOperation)
    {
        case XFT_MOVE2TRASHCAN:
            if (   !_wpIsDeleteable(pObject)
                || !ctsIsTrashable(pObject)  // V1.0.8 (2008-01-05) [pr]
               )
            {
                // abort right away
                frc = FOPSERR_MOVE2TRASH_NOT_DELETABLE;
                fPromptUser = FALSE;
            }
            else
                frc = trshIsOnSupportedDrive(pObject);
                            // V0.9.16 (2001-11-10) [umoeller]
        break;

        case XFT_TRUEDELETE:
            if (!_wpIsDeleteable(pObject))
                frc = FOPSERR_DELETE_NOT_DELETABLE;
        break;

        case XFT_POPULATE:
            // adding objects to populate jobs is not
            // supported
            if (!_somIsA(pObject, _WPFolder))
                frc = FOPSERR_POPULATE_FOLDERS_ONLY;
        break;

        case XFT_INSTALLFONTS:
            if (!_somIsA(pObject, _XWPFontFile))
                frc = FOPSERR_NOT_FONT_FILE;
        break;

        case XFT_DEINSTALLFONTS:
            if (!_somIsA(pObject, _XWPFontObject))
                frc = FOPSERR_NOT_FONT_OBJECT;
        break;
    }

    if (frc != NO_ERROR)
        if ((pfnErrorCallback) && (fPromptUser))
            // prompt user:
            // this can recover
            frc = pfnErrorCallback(ulOperation, pObject, frc, pulIgnoreSubsequent);
        // else: return error code

    return frc;
}

/*
 *@@ fopsAddFileTask:
 *      adds an object to be processed to the given
 *      file task list.
 *      Part of the bottom layer of the XWorkplace file
 *      operations engine.
 *
 *      This must be run on the thread which called fopsCreateFileTaskList.
 *
 *      Note: This function is reentrant only for the
 *      same HFILETASKLIST. Once fopsStartTask
 *      has been called for the task list, you must not
 *      modify the task list again.
 *      There's no automatic protection against this!
 *      So call this function only AFTER fopsCreateFileTaskList
 *      and BEFORE fopsStartTask.
 *
 *      This calls fopsValidateObjOperation on the object and
 *      returns the error code of fopsValidateObjOperation.
 *
 *@@added V0.9.1 (2000-01-26) [umoeller]
 *@@changed V0.9.2 (2000-03-04) [umoeller]: added error callback
 *@@changed V0.9.3 (2000-04-25) [umoeller]: reworked error management
 */

APIRET fopsAddObjectToTask(HFILETASKLIST hftl,      // in: file-task-list handle
                           WPObject *pObject)
{
    APIRET frc;
    PFILETASKLIST pftl = (PFILETASKLIST)hftl;

    if (!(frc = fopsValidateObjOperation(pftl->ulOperation,
                                         pftl->pfnErrorCallback,
                                         pObject,
                                         &pftl->ulIgnoreSubsequent)))
        // proceed:
        // if (!lstAppendItem(&pftl->llObjects, pObject))
        if (!_Append(pftl->pllObjects, pObject))
            // error:
            frc = FOPSERR_INTEGRITY_ABORT;

    return frc;
}

/*
 *@@ fopsStartTask:
 *      this starts processing the specified file task list.
 *
 *      Part of the bottom layer of the XWorkplace file
 *      operations engine.
 *
 *      This must be run on the thread which called fopsCreateFileTaskList.
 *
 *      Further processing is done on the File thread
 *      (xthreads.c) in order not to block the user
 *      interface.
 *
 *      This function can operate in two modes:
 *
 *      -- If (hab == NULLHANDLE), this function returns
 *         almost immediately while processing continues
 *         on the File thread (asynchronous mode).
 *
 *         DO NOT WORK on the task list passed to this
 *         function once this function has been called.
 *         The File thread uses it and destroys it
 *         automatically when done.
 *
 *         There's no way to get the return code of the
 *         actual file processing in asynchronous mode.
 *         This function will return 0 (NO_ERROR) if
 *         the task has started, even if it will later fail
 *         on the File thread.
 *
 *      -- By contrast, if you specify the thread's anchor block
 *         in hab, this function waits for the processing to
 *         complete and returns an error code. This still uses
 *         the File thread, but during processing the current
 *         thread's message queue is processed modally (similar to
 *         what WinMessageBox does) so the system is not blocked
 *         during the operation.
 *
 *         This only works if you specify the HAB on which the
 *         thread is running. Of course, this implies that the
 *         thread has a message queue.
 *
 *@@added V0.9.1 (2000-01-27) [umoeller]
 *@@changed V0.9.3 (2000-04-25) [umoeller]: renamed from fopsStartProcessingTasks
 *@@changed V0.9.4 (2000-08-03) [umoeller]: added synchronous processing support; prototype changed
 *@@changed V0.9.19 (2002-04-17) [umoeller]: removed special window class
 */

APIRET fopsStartTask(HFILETASKLIST hftl,
                     HAB hab)             // in: if != NULLHANDLE, synchronous operation
{
    APIRET  frc = NO_ERROR;
    PFILETASKLIST pftl = (PFILETASKLIST)hftl;
    HWND    hwndNotify = NULLHANDLE;
    // unlock the folders so the file thread
    // can start working
    if (pftl->fSourceLocked)
    {
        _wpReleaseFolderMutexSem(pftl->pSourceFolder);
        pftl->fSourceLocked = FALSE;
    }

    if (hab)
        // synchronous mode: create object window on the
        // running thread and pass that object window
        // (hwndNotify) to the File thread. We then
        // enter a modal message loop below, waiting
        // for T1M_FOPS_TASK_DONE to be posted by the
        // File thread.

        // removed this overhead V0.9.19 (2002-04-17) [umoeller]
        // if (WinRegisterClass(hab,
        //                      "XWPFileOperationsNotify",
        //                      WinDefWindowProc,
        //                      0,
        //                      0))

        if (!(hwndNotify = WinCreateWindow(HWND_OBJECT,
                                           WC_STATIC, // "XWPFileOperationsNotify",
                                           (PSZ)"",
                                           0,
                                           0,0,0,0,
                                           0,
                                           HWND_BOTTOM,
                                           0,
                                           0,
                                           NULL)))
            frc = FOPSERR_START_FAILED;

    if (!frc)
    {
        // have File thread process this list;
        // this calls fopsFileThreadProcessing below
        if (!xthrPostFileMsg(FIM_PROCESSTASKLIST,
                             (MPARAM)hftl,
                             (MPARAM)hwndNotify))
            frc = FOPSERR_START_FAILED;
        else
            if (hab)
            {
                // synchronous mode: enter modal message loop
                // for the window created above.
                // fopsFileThreadProcessing (on the File thread)
                // posts T1M_FOPS_TASK_DONE to that window, so
                // we examine each incoming message.
                QMSG qmsg;
                BOOL fQuit = FALSE;
                while (WinGetMsg(hab, &qmsg, 0, 0, 0))
                {
                    // current message for our object window?
                    if (    (qmsg.hwnd == hwndNotify)
                            // "done" message?
                         && (qmsg.msg == T1M_FOPS_TASK_DONE)
                            // check if it's our handle (for security)
                         && (qmsg.mp1 == pftl)
                       )
                    {
                        fQuit = TRUE;
                        // mp2 has APIRET return code
                        frc = (ULONG)qmsg.mp2;
                    }

                    WinDispatchMsg(hab, &qmsg);
                    if (fQuit)
                        break;
                }
            }
    }
    else
        // error:
        fopsDeleteFileTaskList(hftl);

    if (hwndNotify)
        WinDestroyWindow(hwndNotify);

    return frc;
}

/********************************************************************
 *
 *   File thread processing
 *
 ********************************************************************/

/*
 *@@ fopsCallProgressCallback:
 *      this gets called from fopsFileThreadProcessing
 *      every time the progress callback needs an update.
 *
 *      This is slightly complex. See fileops.h for the
 *      various FOPSUPD_* flags which can come in here.
 *
 *      Part of the bottom layer of the XWorkplace file
 *      operations engine.
 *
 *      Returns FALSE to abort.
 *
 *@@added V0.9.2 (2000-03-30) [cbo]:
 */

APIRET fopsCallProgressCallback(PFILETASKLIST pftl,
                                ULONG flChanged,
                                FOPSUPDATE *pfu)   // in: update structure in fopsFileThreadProcessing;
                                                    // NULL means done altogether
{
    APIRET frc = NO_ERROR;

    // store changed flags
    pfu->flChanged = flChanged;

    // set source and target folders
    // depending on operation
    switch (pftl->ulOperation)
    {
        case XFT_MOVE2TRASHCAN:
            if (pfu->fFirstCall)
            {
                // update source and target;
                // the target has been set to the trash can before
                // the first call by fopsFileThreadProcessing
                pfu->flChanged |= (FOPSUPD_SOURCEFOLDER_CHANGED
                                        | FOPSUPD_TARGETFOLDER_CHANGED);
            }
        break;

        case XFT_RESTOREFROMTRASHCAN:
            if (pfu->fFirstCall)
                // source is trash can, which is needed only for
                // the first call
                pfu->flChanged |= FOPSUPD_SOURCEFOLDER_CHANGED;

            // target folder can change with any call
            pfu->flChanged |= FOPSUPD_TARGETFOLDER_CHANGED;
        break;

        case XFT_TRUEDELETE:
            if (pfu->fFirstCall)
                // update source on first call; there is
                // no target folder
                pfu->flChanged |= FOPSUPD_SOURCEFOLDER_CHANGED;
        break;

    }

    if (pftl->pfnProgressCallback)
        if (!(pftl->pfnProgressCallback(pfu,
                                        pftl->ulUser)))
            // FALSE means abort:
            frc = FOPSERR_CANCELLEDBYUSER;

    if (pfu)
        // unset first-call flag, which
        // was initially set
        pfu->fFirstCall = FALSE;

    return frc;
}

/*
 *@@ fopsFileThreadFixNonDeletable:
 *      called during "true delete" processing when
 *      an object is encountered which is non-deletable.
 *
 *      It is the responsibility of this function to
 *      prompt the user about what to do and fix the
 *      object to be deletable, if the user wants this.
 *
 *@@added V0.9.6 (2000-10-25) [umoeller]
 */

APIRET fopsFileThreadFixNonDeletable(PFILETASKLIST pftl,
                                     WPObject *pSubObjThis, // in: obj to fix
                                     PULONG pulIgnoreSubsequent) // in: ignore subsequent errors of the same type
{
    APIRET frc = NO_ERROR;

    if (_somIsA(pSubObjThis, _WPFileSystem))
    {
        if ( 0 == ((*pulIgnoreSubsequent) & FOPS_ISQ_DELETE_READONLY) )
            // first error, or user has not selected
            // to abort subsequent:
            frc = FOPSERR_DELETE_READONLY;      // non-fatal
        else
            // user has chosen to ignore subsequent:
            // this will call wpSetAttr below
            frc = NO_ERROR;
            // _Pmpf(("      frc = %d", frc));
    }
    else
        frc = FOPSERR_DELETE_NOT_DELETABLE; // fatal

    if (frc != NO_ERROR)
        if (pftl->pfnErrorCallback)
        {
            // prompt user:
            // this can recover, but should have
            // changed the problem (read-only)
            // _Pmpf(("      calling error callback"));
            frc = pftl->pfnErrorCallback(pftl->ulOperation,
                                         pSubObjThis,
                                         frc,
                                         pulIgnoreSubsequent);
        }

    if (!frc)
        if (_somIsA(pSubObjThis, _WPFileSystem))
            if (!_wpSetAttr(pSubObjThis, FILE_NORMAL))
                frc = FOPSERR_WPSETATTR_FAILED;

    return frc;
}

/*
 *@@ fopsFileThreadConfirmDeleteFolder:
 *      gets called for every folder on the "delete"
 *      list. If "Delete folder" confirmations are
 *      enabled, this calls the error callback
 *      with the FOPSERR_DELETE_CONFIRM_FOLDER error
 *      code, which should then confirm folder deletion.
 *
 *@@added V0.9.16 (2001-12-06) [umoeller]
 */

APIRET fopsFileThreadConfirmDeleteFolder(PFILETASKLIST pftl,
                                         WPObject *pSubObjThis, // in: folder to be deleted
                                         PULONG pulIgnoreSubsequent) // in: ignore subsequent errors of the same type
{
    if (    // does caller want to have folder deletions confirmed?
            (_wpQueryConfirmations(pSubObjThis) & CONFIRM_DELETEFOLDER)
         && ( 0 == ((*pulIgnoreSubsequent) & FOPS_ISQ_DELETE_FOLDERS) )
       )
    {
        return pftl->pfnErrorCallback(pftl->ulOperation,
                                      pSubObjThis,
                                      FOPSERR_DELETE_CONFIRM_FOLDER,
                                      pulIgnoreSubsequent);
    }

    return NO_ERROR;
}

/*
 *@@ fopsUseForceDelete:
 *      returns TRUE if the given filename resides
 *      in the hidden \trash\ directory and the
 *      caller should therefore use DosForceDelete
 *      instead of DosDelete to delete the file
 *      in order not to pollute DELDIR.
 *
 *      Called directly from fopsFileThreadSneakyDeleteFolderContents,
 *      if the file is not awake, or by XFldDataFile::wpDestroyObject
 *      if it is.
 *
 *@@added V0.9.20 (2002-07-16) [umoeller]
 */

BOOL fopsUseForceDelete(PCSZ pcszFilename)
{
    return !strnicmp(pcszFilename + 1, ":\\trash\\", 8);
}

/*
 *@@ Call_wpFree:
 *      calls wpFree on the given object and returns an
 *      error code if that fails. Now that we use wpSetError
 *      in our wpDestroyObject overrides, this is a good
 *      idea to do.
 *
 *@@added V1.0.0 (2002-09-09) [umoeller]
 */

APIRET Call_wpFree(PFILETASKLIST pftl,
                   WPObject *pobj)
{
    APIRET  arc;
    BOOL    fRepeat;

    do
    {
        arc = NO_ERROR;
        fRepeat = FALSE;

        if (!_wpFree(pobj))
        {
            // check if we can get an error code from the object...
            // we now use wpSetError in wpDestroyObject, so this
            // _might_ have something meaningful
            if (!(arc = _wpQueryError(pobj)))
                // nothing set:
                arc = FOPSERR_WPFREE_FAILED;
        }

        // added error reports here; since we have disabled
        // the message boxes in wpDestroyObject, the user
        // gets no feedback at all if the deletion failed,
        // but we should give her some V1.0.0 (2002-09-09) [umoeller]
        if (    (arc)
             && (pftl->pfnErrorCallback)
           )
            if (!pftl->pfnErrorCallback(pftl->ulOperation,
                                        pobj,
                                        FOPSERR_WPFREE_FAILED,
                                        NULL))
                // NO_ERROR return code means retry:
                fRepeat = TRUE;

    } while (fRepeat);

    return arc;
}

/*
 *@@ fopsFileThreadSneakyDeleteFolderContents:
 *      this gets called from fopsFileThreadTrueDelete before
 *      an instance of WPFolder actually gets deleted.
 *
 *      Starting with V0.9.6, we no longer fully populate
 *      folders in order to do the entire processing via
 *      WPS projects. Instead, we only populate folders
 *      with subfolders and delete files using Dos* functions,
 *      which is much faster and puts less stress on the WPS
 *      object management which runs into trouble otherwise.
 *
 *      This function must delete all dormant files in the
 *      specified folder, but not the folder itself.
 *
 *      Preconditions:
 *
 *      1)  Subfolders of the folder have already been deleted.
 *
 *      2)  Files in the folder which were already awake have
 *          already been deleted using _wpFree.
 *
 *      So there should be only dormant files left, which we
 *      can safely delete using Dos* functions.
 *
 *@@added V0.9.6 (2000-10-25) [umoeller]
 *@@changed V0.9.20 (2002-07-16) [umoeller]: optimizations and adjustments for fopsUseForceDelete
 *@@changed V1.0.1 (2003-01-30) [umoeller]: more optimizations
 */

APIRET fopsFileThreadSneakyDeleteFolderContents(PFILETASKLIST pftl,
                                                FOPSUPDATE *pfu,
                                    // in: update structure in fopsFileThreadProcessing
                                                WPObject **ppObject,
                                    // in: folder whose contents are to be deleted,
                                    // out: failing object if error
                                                PCSZ pcszMainFolderPath,
                                    // in: path of main folder of which *ppObject is a subobject
                                    // somehow; just for the progress dialog
                                    // now passing in the path for speed V0.9.20 (2002-07-12) [umoeller]
                                                BOOL fUseForceDelete,
                                    // in: if TRUE, we use DosForceDelete V0.9.20 (2002-07-12) [umoeller]
                                                PULONG pulIgnoreSubsequent,
                                    // in: ignore subsequent errors of the same type
                                                ULONG ulProgressScalarFirst,
                                                ULONG cSubObjects,
                                                PULONG pulSubObjectThis)
{
    APIRET      frc = NO_ERROR;
    WPFolder    *pFolder = *ppObject;
    CHAR        szFolderPath[CCHMAXPATH] = "";
    CHAR        szFullPath[2*CCHMAXPATH];
    HDIR        hdirFindHandle = HDIR_CREATE;

    BOOL        fFolderLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (!(fFolderLocked = !_wpRequestFolderMutexSem(pFolder, 5000)))
            frc = FOPSERR_REQUESTFOLDERMUTEX_FAILED;
        else
        {
            if (    (!_wpQueryFilename(pFolder, szFolderPath, TRUE))
                 // || (!_wpQueryFilename(pMainFolder, szMainFolderPath, TRUE))
               )
                frc = FOPSERR_WPQUERYFILENAME_FAILED;
            else
            {
                ULONG           ulMainFolderPathLen = strlen(pcszMainFolderPath);

                CHAR            szSearchMask[CCHMAXPATH];
                FILEFINDBUF3    ffb3 = {0};      // returned from FindFirst/Next
                ULONG           cbFFB3 = sizeof(FILEFINDBUF3);
                ULONG           ulFindCount = 1;  // look for 1 file at a time
                ULONG           lenFullPath;

                // _PmpfF(("doing DosFindFirst for %s",
                   //          szFolderPath));

                // prepare full path and search mask
                // optimized V1.0.1 (2003-01-30) [umoeller]
                lenFullPath = strlcpy(szFullPath, szFolderPath, sizeof(szFullPath));
                memcpy(szSearchMask, szFolderPath, lenFullPath + 1);

                szSearchMask[lenFullPath] = '\\';
                szSearchMask[lenFullPath + 1] = '*';
                szSearchMask[lenFullPath + 2] = '\0';

                szFullPath[lenFullPath] = '\\';
                szFullPath[++lenFullPath] = '\0';

                frc = DosFindFirst(szSearchMask,
                                   &hdirFindHandle,
                                   // find everything except directories
                                   FILE_ARCHIVED | FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
                                   &ffb3,
                                   cbFFB3,
                                   &ulFindCount,
                                   FIL_STANDARD);

                // and start looping...
                while (!frc)
                {
                    // alright... we got the file's name in ffb3.achName
                    strcpy(szFullPath + lenFullPath, ffb3.achName);
                            // optimized V0.9.19 (2002-04-17) [umoeller]

                    // _Pmpf(("    got file %s", szFullPath));

                    // call callback for subobject;
                    // as the path, get the full path name of the file
                    // minus the full path of the main folder we are
                    // working on
                    pfu->pcszSubObject = szFullPath + ulMainFolderPathLen + 1;
                    // calc new sub-progress: this is the value we first
                    // had before working on the subobjects (which
                    // is a multiple of 100) plus a sub-progress between
                    // 0 and 100 for the subobjects
                    pfu->ulProgressScalar = ulProgressScalarFirst
                                            + (((*pulSubObjectThis) * 100 )
                                                 / cSubObjects);

                    if (!(frc = fopsCallProgressCallback(pftl,
                                                         FOPSUPD_SUBOBJECT_CHANGED
                                                          | FOPSPROG_UPDATE_PROGRESS,
                                                         pfu)))
                    {
                        // no error, not cancelled:
                        // check file's attributes
                        if (ffb3.attrFile & (FILE_SYSTEM | FILE_READONLY))
                        {
                            // system or read-only file:
                            // prompt!!

                            // 1) make object awake
                            WPFileSystem *pFSObj;
                            if (!(pFSObj = _wpclsQueryObjectFromPath(_WPFileSystem,
                                                                     szFullPath)))
                                frc = FOPSERR_INVALID_OBJECT;
                            else
                            {
                                // _Pmpf(("        calling fopsFileThreadFixNonDeletable"));
                                frc = fopsFileThreadFixNonDeletable(pftl,
                                                                    pFSObj,
                                                                    pulIgnoreSubsequent);
                            }

                            if (!frc)
                            {
                                // either no problem or problem fixed:
                                if (frc = Call_wpFree(pftl, pFSObj))
                                    *ppObject = pFSObj;
                            }
                        }
                        else
                        {
                            // sneaky delete!!
                            // use DosForceDelete if the file is in \trash
                            // V0.9.20 (2002-07-12) [umoeller]
                            if (fUseForceDelete)
                                frc = DosForceDelete(szFullPath);
                            else
                                frc = DosDelete(szFullPath);

                            PMPF_TRASHCAN(("<%s> deleted --> %d", szFullPath, frc));
                        }
                    }

                    if (!frc)
                    {
                        ulFindCount = 1;
                        frc = DosFindNext(hdirFindHandle,
                                          &ffb3,
                                          cbFFB3,
                                          &ulFindCount);

                        // raise object count for progress...
                        (*pulSubObjectThis)++;
                    }
                } // while (arc == NO_ERROR)

                if (frc == ERROR_NO_MORE_FILES)
                    frc = NO_ERROR;
            }
        }
    }
    CATCH(excpt1)
    {
        frc = FOPSERR_FILE_THREAD_CRASHED;
    } END_CATCH();

    if (hdirFindHandle != HDIR_CREATE)
        DosFindClose(hdirFindHandle);

    if (fFolderLocked)
        _wpReleaseFolderMutexSem(pFolder);

    return frc;
}

/*
 *@@ fopsFileThreadTrueDelete:
 *      gets called from fopsFileThreadProcessing with
 *      XFT_TRUEDELETE for every object on the list.
 *      Part of the bottom layer of the XWorkplace file
 *      operations engine.
 *
 *      The object to be deleted is FOPSUPDATE.pSourceObject.
 *      On output, if this returns something != NO_ERROR,
 *      it sets *ppFailingObject to the object that
 *      failed.
 *
 *      This checks if pSourceObject is a folder and will
 *      call fopsFileThreadSneakyDeleteFolderContents,
 *      if necessary.
 *
 *      Note that the progress callback has already been
 *      called with FOPSUPD_SOURCEOBJECT_CHANGED
 *      | FOPSPROG_UPDATE_PROGRESS for the source object
 *      and will only need a refresh for subobjects,
 *      if any.
 *
 *      This finally allows cancelling a "delete"
 *      operation when a subfolder is currently
 *      processed.
 *
 *@@added V0.9.3 (2000-04-27) [umoeller]
 *@@changed V0.9.3 (2000-04-30) [umoeller]: reworked progress reports
 *@@changed V0.9.4 (2000-07-27) [umoeller]: added ulIgnoreSubsequent to ignore further errors
 *@@changed V0.9.6 (2000-10-25) [umoeller]: largely rewritten to support sneaky delete (much faster)
 *@@changed V0.9.9 (2001-02-01) [umoeller]: added FOI_DELETEINPROGRESS
 *@@changed V0.9.9 (2001-04-01) [umoeller]: fixed crashes with shadows
 *@@changed V0.9.16 (2001-12-06) [umoeller]: added confirmation of folder delete, if enabled
 *@@changed V0.9.19 (2002-04-17) [umoeller]: fixed bad FOI_DELETEINPROGRESS flag which caused refresh to fail after true delet
 *@@changed V0.9.19 (2002-04-17) [umoeller]: fixed duplicate updates in progress dlg and other overhead
 *@@changed V0.9.20 (2002-07-12) [umoeller]: optimizations and adjustments for fopsUseForceDelete
 *@@changed V0.9.20 (2002-07-16) [umoeller]: fixed deleting FTP folders
 *@@changed V0.9.20 (2002-08-04) [umoeller]: no longer unsetting FOI_DELETEINPROGRESS before wpFree'ing a folder
 *@@changed V1.0.0 (2002-09-09) [umoeller]: added better error reports if wpFree failed
 *@@changed V1.0.1 (2002-12-15) [umoeller]: rewritten to use XWPObjList class
 */

APIRET fopsFileThreadTrueDelete(HFILETASKLIST hftl,
                                FOPSUPDATE *pfu,       // in: update structure in fopsFileThreadProcessing
                                PULONG pulIgnoreSubsequent, // in: ignore subsequent errors of the same type
                                WPObject **ppObjectFailed)   // out: failing object if error
{
    APIRET  frc = NO_ERROR;
    PFILETASKLIST pftl = (PFILETASKLIST)hftl;
    PCSZ    pcszClassName;

    *ppObjectFailed = pfu->pSourceObject;

    // we only need the massive overhead below
    // if this is really a folder
    // V0.9.19 (2002-04-17) [umoeller]
    if (    (!objIsAFolder(pfu->pSourceObject))
         // do a plain wpFree for FTP folders also because
         // populate always fails on them
         // V0.9.20 (2002-07-12) [umoeller]
         || (    (pcszClassName = _somGetClassName(pfu->pSourceObject))
              && (!strcmp(pcszClassName, "WPHost"))
            )
       )
    {
        // non-folder or FTP folder:
        if (!_wpIsDeleteable(pfu->pSourceObject))
            // not deletable: prompt user about
            // what to do with this
            frc = fopsFileThreadFixNonDeletable(pftl,
                                                pfu->pSourceObject,
                                                pulIgnoreSubsequent);

        if (    (frc)
             || (frc = Call_wpFree(pftl, pfu->pSourceObject))
           )
            *ppObjectFailed = pfu->pSourceObject;
    }
    else
    {
        // source object is folder:
        // all subobjects will have the same path, so
        // query this once only
        // V0.9.20 (2002-07-12) [umoeller]
        CHAR szMainFolderPath[CCHMAXPATH];
        if (!_wpQueryFilename(pfu->pSourceObject, szMainFolderPath, TRUE))
            frc = FOPSERR_WPQUERYFILENAME_FAILED;
        else
        {
            // list of WPObject* pointers
            // LINKLIST llSubObjects;
            XWPObjList  *pllSubObjects = NULL;
                        // V1.0.1 (2002-12-15) [umoeller]

            // another exception handler to allow for cleanup
            TRY_LOUD(excpt1)
            {
                ULONG   cSubObjects = 0,
                        cSubObjectsTemp = 0,
                        cSubDormantFilesTemp = 0;

                // we can use the same "force delete" setting for
                // all objects here (passed to fopsFileThreadSneakyDeleteFolderContents)
                // V0.9.20 (2002-07-12) [umoeller]
                BOOL fUseForceDelete = fopsUseForceDelete(szMainFolderPath);

                pllSubObjects = _xwpclsCreateList(_XFldObject, NULL, NULL);
                        // V1.0.1 (2002-12-15) [umoeller]

                PMPF_TRASHCAN(("expanding %s", _wpQueryTitle(pfu->pSourceObject)));

                // say "collecting objects"
                if (!(frc = fopsCallProgressCallback(pftl,
                                                     FOPSUPD_EXPANDING_SOURCEOBJECT_1ST,
                                                     pfu)))
                {
                    // confirm folder deletions?
                    if (!(frc = fopsFileThreadConfirmDeleteFolder(pftl,
                                                                  pfu->pSourceObject,
                                                                  pulIgnoreSubsequent)))
                    {
                        // build list of all objects to be deleted,
                        // but populate folders only...
                        // this will give us all objects in the
                        // folders which are already awake before
                        // the folder itself.
                        if (!(frc = fopsExpandObjectFlat(pllSubObjects,
                                                         pfu->pSourceObject,
                                                         TRUE,        // populate folders only
                                                         &cSubObjectsTemp,
                                                         &cSubDormantFilesTemp)))
                                      // now cSubObjectsTemp has the no. of awake objects,
                                      // cSubDormantFilesTemp has the no. of dormant files
                        {
                            // say "done collecting objects"
                            frc = fopsCallProgressCallback(pftl,
                                                           FOPSUPD_EXPANDING_SOURCEOBJECT_DONE,
                                                           pfu);
                            // calc total count for progress
                            cSubObjects = cSubObjectsTemp + cSubDormantFilesTemp;
                        }
                    }
                }

                if (    (!frc)
                     && (cSubObjects) // avoid division by zero below
                   )
                {
                    ULONG ulSubObjectThis = 0,
                          // save progress scalar
                          ulProgressScalarFirst = pfu->ulProgressScalar;

                    // We need to do the following:

                    // 1) If the object is a non-folder, delete it.
                    //    It is some object in a folder which was already
                    //    awake, and its parent folder will come later
                    //    on the list.
                    // 2) If we encounter a folder, do a "sneaky delete"
                    //    of all files in the folder (to avoid the WPS
                    //    popups for read-only files) and then delete
                    //    the folder object.

                    // all sub-objects collected:
                    // go thru the whole list and start deleting.
                    // Note that folder contents which are already awake
                    // come before the containing folder ("bottom-up" directory list).
                    // PLISTNODE pNode = lstQueryFirstNode(&llSubObjects);
                    WPObject *pSubObjThis = NULL;

                    // keep taking the _first_ object off the list;
                    // wpFree will automatically take it off the list
                    // thru the cleanup sequence in wpUnInitData
                    // V1.0.1 (2002-12-15) [umoeller]
                    while (pSubObjThis = _Enum(pllSubObjects, NULL))
                    {
                        // get object to delete...
                        // WPObject *pSubObjThis = (WPObject*)pNode->pItemData;

                        // delete the object: we get here for any instantiated
                        // Desktop object which must be deleted, that is
                        // -- for any awake Desktop object in any subfolder (before the folder)
                        // -- for any folder after its contents have been deleted
                        //    (either sneakily or by a previous wpFree)

                        // calc new sub-progress: this is the value we first
                        // had before working on the subobjects (which
                        // is a multiple of 100) plus a sub-progress between
                        // 0 and 100 for the subobjects
                        pfu->ulProgressScalar = ulProgressScalarFirst
                                                + ((ulSubObjectThis * 100 )
                                                     / cSubObjects);
                        if (frc = fopsCallProgressCallback(pftl,
                                                           FOPSUPD_SUBOBJECT_CHANGED
                                                            | FOPSPROG_UPDATE_PROGRESS,
                                                           pfu))
                        {
                            // error or cancelled:
                            *ppObjectFailed = pSubObjThis;
                            break;
                        }

                        PMPF_TRASHCAN(("subobj [%s] {%s}: calling _wpIsDeleteable",
                                       _wpQueryTitle(pSubObjThis),
                                       _somGetClassName(pSubObjThis)));

                        if (    (_wpIsDeleteable(pSubObjThis))
                                // not deletable: prompt user about
                                // what to do with this
                             || (!(frc = fopsFileThreadFixNonDeletable(pftl,
                                                                       pSubObjThis,
                                                                       pulIgnoreSubsequent)))
                           )
                        {
                            // now check if we have a folder...
                            // if so, we need some special processing
                            BOOL fSubIsFolder;
                            if (fSubIsFolder = objIsAFolder(pSubObjThis))
                            {
                                // folder:
                                // do sneaky delete of dormant folder contents
                                // (awake folder contents have already been freed
                                // because these came before the folder on the
                                // subobjects list)
                                WPObject *pobj2 = pSubObjThis;

                                PMPF_TRASHCAN(("Sneaky delete folder %s", _wpQueryTitle(pSubObjThis) ));

                                // before we do anything, set the "delete in progress"
                                // flag. This greatly reduces the pressure on the WPS
                                // folder auto-refresh because the WPS can then drop
                                // all notifications, I guess.
                                // V0.9.9 (2001-02-01) [umoeller]
                                // Note also that we keep the flag set all the way
                                // down to calling _wpFree on the folder, which prevents
                                // the WPS from creating handles while deleting
                                // V0.9.20 (2002-08-04) [umoeller]
                                _wpModifyFldrFlags(pSubObjThis,
                                                   FOI_DELETEINPROGRESS,
                                                   FOI_DELETEINPROGRESS);
                                if (frc = fopsFileThreadSneakyDeleteFolderContents(pftl,
                                                                                   pfu,
                                                                                   &pobj2,
                                                                                   szMainFolderPath,
                                                                                    // ^^ main folder; this must be
                                                                                    // a folder, because we have a subobject
                                                                                   fUseForceDelete,
                                                                                    // V0.9.20 (2002-07-12) [umoeller]
                                                                                   pulIgnoreSubsequent,
                                                                                   ulProgressScalarFirst,
                                                                                   cSubObjects,
                                                                                   &ulSubObjectThis))
                                    // error:
                                    *ppObjectFailed = pobj2;

                                // force the WPS to flush all pending
                                // auto-refresh information for this folder...
                                // these have all piled up for the sneaky stuff
                                // above and will cause some internal overflow
                                // if we don't give the WPS a chance to process them!
                                _wpFlushNotifications(pSubObjThis);
                            } // end if (_somIsA(pSubObjThis, _WPFolder))

                            PMPF_TRASHCAN(("calling _wpFree"));

                            if (!frc)
                                if (frc = Call_wpFree(pftl, pSubObjThis))
                                    *ppObjectFailed = pSubObjThis;

                            if (frc)
                            {
                                if (fSubIsFolder)
                                    // only if freeing the folder failed, unset
                                    // FOI_DELETEINPROGRESS
                                    _wpModifyFldrFlags(pSubObjThis,
                                                       FOI_DELETEINPROGRESS,
                                                       0);

                                break;
                            }
                        }
                        else
                            // error:
                            *ppObjectFailed = pSubObjThis;

                        // pNode = pNode->pNext;
                        ulSubObjectThis++;
                    } // end while ((pNode) && (!frc))

                    // done with subobjects: report NULL subobject
                    if (!frc)
                    {
                        pfu->pcszSubObject = NULL;
                        frc = fopsCallProgressCallback(pftl,
                                                       FOPSUPD_SUBOBJECT_CHANGED,
                                                       pfu);
                    }
                } // end if (    (!frc)
            }
            CATCH(excpt1)
            {
                frc = FOPSERR_FILE_THREAD_CRASHED;
            } END_CATCH();

            if (pllSubObjects)
                _somFree(pllSubObjects);        // V1.0.1 (2002-12-15) [umoeller]
            // lstClear(&llSubObjects);
        }
    }

    return frc;
}

/*
 *@@ FileThreadFontProcessing:
 *      gets called from fopsFileThreadProcessing with
 *      XFT_INSTALLFONTS and  XFT_DEINSTALLFONTS for every
 *      object on the list.
 *
 *      This calls the font engine (src\config.fonts.c)
 *      in turn, most notably fonInstallFont or fonDeInstallFont.
 *
 *@@added V0.9.7 (2001-01-13) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: font uninstall was missing status bar update, fixed
 *@@changed V1.0.1 (2002-12-15) [umoeller]: made func static, changed prototype
 */

STATIC APIRET FileThreadFontProcessing(HAB hab,
                                       PFILETASKLIST pftl,
                                       WPObject *pObjectThis,
                                       WPObject **ppFreeFont,       // out: set to pObjectThis if it needs freeing
                                       PULONG pulIgnoreSubsequent)
{
    APIRET frc = FOPSERR_INTEGRITY_ABORT;

    switch (pftl->ulOperation)
    {
        case XFT_INSTALLFONTS:
            frc = fonInstallFont(hab,
                                 pftl->pTargetFolder, // font folder
                                 pObjectThis,    // font file
                                 NULL);          // out: new obj
                    // this returns proper APIRET codes
        break;

        case XFT_DEINSTALLFONTS:
            frc = fonDeInstallFont(hab,
                                   pftl->pSourceFolder,
                                   pObjectThis);     // font object
                    // this returns proper APIRET
            if ((!frc) || (frc == FOPSERR_FONT_STILL_IN_USE))
                // in these two cases only, destroy the
                // font object after we've called the error
                // callback... it still needs the object!
                *ppFreeFont = pObjectThis;
        break;
    }

    if (frc != NO_ERROR)
    {
        if ( 0 == ((*pulIgnoreSubsequent) & FOPS_ISQ_FONTINSTALL) )
            // first error, or user has not selected
            // to abort subsequent:
            // now, these errors we should report;
            // if the error callback returns NO_ERROR,
            // we continue anyway
            frc = pftl->pfnErrorCallback(pftl->ulOperation,
                                         pObjectThis,
                                         frc,
                                         pulIgnoreSubsequent);
        else
            // user has chosen to ignore subsequent:
            // just go on
            frc = NO_ERROR;
    }

    return frc;
}

/*
 *@@ fopsFileThreadProcessing:
 *      this actually performs the file operations
 *      on the objects which have been added to
 *      the given file task list.
 *      Part of the bottom layer of the XWorkplace file
 *      operations engine.
 *
 *      WARNING: NEVER call this function manually.
 *      This gets called on the File thread (xthreads.c)
 *      automatically  after fopsStartTask has
 *      been called.
 *
 *      This runs on the File thread.
 *
 *@@added V0.9.1 (2000-01-29) [umoeller]
 *@@changed V0.9.3 (2000-04-25) [umoeller]: reworked error management
 *@@changed V0.9.3 (2000-04-26) [umoeller]: added "true delete" support
 *@@changed V0.9.3 (2000-04-30) [umoeller]: reworked progress reports
 *@@changed V0.9.4 (2000-08-03) [umoeller]: added synchronous processing support
 *@@changed V0.9.7 (2001-01-13) [umoeller]: added hab to prototype (needed for font install)
 *@@changed V0.9.7 (2001-01-13) [umoeller]: added XFT_INSTALLFONTS, XFT_DEINSTALLFONTS
 *@@changed V0.9.19 (2002-05-01) [umoeller]: reversed confirmations for move to trash and empty trash
 *@@changed V1.0.6 (2006-09-24) [pr]: don't move Printers and Transient objects to Trash Can
 *@@changed V1.0.8 (2008-01-05) [pr]: don't move WPNetGrp and WPServer objects to Trash Can @@fixes 1035
 */

VOID fopsFileThreadProcessing(HAB hab,              // in: file thread's anchor block
                              HFILETASKLIST hftl,
                              HWND hwndNotify)      // in: if != NULLHANDLE, post this
                                                    // window a T1M_FOPS_TASK_DONE msg
{
    BOOL frc = NO_ERROR;
    PFILETASKLIST pftl = (PFILETASKLIST)hftl;

    PMPF_FOPS(("0x%lX", hftl));

    TRY_LOUD(excpt2)
    {
        if (pftl)
        {
            /*
             * 1) preparations before collecting objects
             *
             */

            // objects count
            ULONG ulCurrentObject = 0;

            // ignore subsequent errors; this
            // is set to one of the FOPS_ISQ_* flags
            // if the error callback chooses to ignore
            // subsequent errors
            ULONG ulIgnoreSubsequent = 0;

            // get first node
            // PLISTNODE   pNode = lstQueryFirstNode(&pftl->llObjects);
            FOPSUPDATE  fu;
            memset(&fu, 0, sizeof(fu));

            switch (pftl->ulOperation)
            {
                case XFT_MOVE2TRASHCAN:
                    // move to trash can:
                    // in that case, pTargetFolder is NULL,
                    // so query the target folder first
                    pftl->pTargetFolder = _xwpclsQueryDefaultTrashCan(_XWPTrashCan);

                    PMPF_TRASHCAN(("  target trash can is %s", _wpQueryTitle(pftl->pTargetFolder) ));
                break;

                case XFT_TRUEDELETE:
                    // this can either be a real true delete,
                    // or an empty trash can command... for
                    // empty trash can, we should no longer
                    // display confirmations,
                    if (_somIsA(pftl->pSourceFolder, _XWPTrashCan))
                    {
                        // empty trash can:
                        // pretend the user has pressed "Yes to all" for
                        // folders and readonly files
                        // V0.9.19 (2002-05-01) [umoeller]
                        ulIgnoreSubsequent = FOPS_ISQ_FLAGS_EMPTYTRASH;
                    }
                break;
            }

            fu.ulOperation = pftl->ulOperation;
            fu.pSourceFolder = pftl->pSourceFolder;
            fu.pTargetFolder = pftl->pTargetFolder;
            fu.fFirstCall = TRUE;  // unset by CallProgressCallback after first call
            fu.ulProgressScalar = 0;
            // fu.ulProgressMax = lstCountItems(&pftl->llObjects) * 100;
            fu.ulProgressMax = _Count(pftl->pllObjects) * 100;
                    // V1.0.1 (2002-12-15) [umoeller]

            PMPF_FOPS(("    %d items on list", fu.ulProgressMax / 100));

            /*
             * 2) process objects on list
             *
             */

            // keep taking the first object off the list
            // V1.0.1 (2002-12-15) [umoeller]
            while (fu.pSourceObject = _Enum(pftl->pllObjects, NULL))
            {
                // WPObject    *pObjectThis = (WPObject*)pNode->pItemData;
                WPObject       *pObjectFailed = NULL;

                // per default, take object off the list; this
                // is set to FALSE if the object gets deleted
                BOOL        fTakeOff = TRUE;

                PMPF_FOPS(("    checking object 0x%lX", fu.pSourceObject));

                // check if object is still valid
                if (!wpshCheckObject(fu.pSourceObject))
                {
                    frc = FOPSERR_INVALID_OBJECT;
                    break;
                }
                else
                    // call progress callback with this object
                    if (    (pftl->pfnProgressCallback)
                         && (frc = fopsCallProgressCallback(pftl,
                                                            FOPSUPD_SOURCEOBJECT_CHANGED
                                                             | FOPSPROG_UPDATE_PROGRESS,
                                                            &fu))
                       )
                        // error or cancelled:
                        break;

                PMPF_FOPS(("    processing %s", _wpQueryTitle(fu.pSourceObject) ));
                // now, for each object, perform
                // the desired operation
                switch (pftl->ulOperation)
                {
                    /*
                     * XFT_MOVE2TRASHCAN:
                     *
                     */

                    case XFT_MOVE2TRASHCAN:
                    {
                        PMPF_FOPS(("trashmove %s",
                                        _wpQueryTitle(fu.pSourceObject) ));

                        // confirm this if the object is a folder and
                        // folder deletion confirm is on
                        // V0.9.19 (2002-05-01) [umoeller]
                        if (    (!objIsAFolder(fu.pSourceObject))
                             || (!(frc = fopsFileThreadConfirmDeleteFolder(pftl,
                                                                           fu.pSourceObject,
                                                                           &ulIgnoreSubsequent)))
                           )
                        {
                            // V1.0.8 (2008-01-05) [pr]
                            if (!ctsIsTrashable(fu.pSourceObject))
                            {
                                frc = fopsFileThreadTrueDelete(hftl,
                                                               &fu,
                                                               &ulIgnoreSubsequent,
                                                               &pObjectFailed);
                            }
                            else
                                if (!_xwpDeleteIntoTrashCan(pftl->pTargetFolder, // default trash can
                                                            fu.pSourceObject))
                                    frc = FOPSERR_NOT_HANDLED_ABORT;
                        }
                    }
                    break;

                    /*
                     * XFT_RESTOREFROMTRASHCAN:
                     *
                     */

                    case XFT_RESTOREFROMTRASHCAN:
                        PMPF_FOPS(("restoring %s",
                                    _wpQueryTitle(fu.pSourceObject) ));

                        if (!_xwpRestoreFromTrashCan(fu.pSourceObject,      // trash object
                                                     pftl->pTargetFolder))  // can be NULL
                            frc = FOPSERR_NOT_HANDLED_ABORT;
                        // after this pObjectThis is invalid!

                        // pSourceObject has been destroyed, so it's
                        // been taken off the list automatically
                        fTakeOff = FALSE;
                    break;

                    /*
                     * XFT_TRUEDELETE:
                     *
                     */

                    case XFT_TRUEDELETE:
                        PMPF_FOPS(("calling fopsFileThreadTrueDelete for %s",
                                    _wpQueryTitle(fu.pSourceObject) ));

                        frc = fopsFileThreadTrueDelete(hftl,
                                                       &fu,
                                                       &ulIgnoreSubsequent,
                                                       &pObjectFailed);
                                           // after this pObjectThis is invalid!

                        // pSourceObject has been destroyed, so it's
                        // been taken off the list automatically
                        fTakeOff = FALSE;
                    break;

                    /*
                     * XFT_POPULATE:
                     *
                     */

                    case XFT_POPULATE:
                        if (!fdrCheckIfPopulated(fu.pSourceObject,
                                                 FALSE))   // full populate
                            frc = FOPSERR_POPULATE_FAILED;
                    break;

                    /*
                     * XFT_INSTALLFONTS:
                     *
                     */

                    case XFT_INSTALLFONTS:
                    case XFT_DEINSTALLFONTS:
                    {
                        WPObject *pFreeFont = NULL;

                        frc = FileThreadFontProcessing(hab,
                                                       pftl,
                                                       fu.pSourceObject,
                                                       &pFreeFont,
                                                       &ulIgnoreSubsequent);
                        if (pFreeFont)
                        {
                            _wpFree(pFreeFont);
                            _xwpChangeFontsCount(pftl->pSourceFolder, -1);
                                    // status bar update was missing V0.9.20 (2002-07-25) [umoeller]

                            // pSourceObject has been destroyed, so it's
                            // been taken off the list automatically
                            fTakeOff = FALSE;
                        }
                    }
                    break;
                }

                #ifdef __DEBUG__
                    if (frc != NO_ERROR)
                    {
                        CHAR sz[500];
                        PSZ pszTitle = "?";
                        if (wpshCheckObject(fu.pSourceObject))
                            pszTitle = _wpQueryTitle(fu.pSourceObject);

                        sprintf(sz,
                                "Error in " __FUNCTION__ " processing [%s] {%s}:",
                                pszTitle,
                                _somGetClassName(fu.pSourceObject));

                        cmnDosErrorMsgBox(NULLHANDLE,
                                          pszTitle,  // in: string for %1 message or NULL
                                          "File Thread Error",  // in: msgbox title
                                          sz,       // in: string before error or NULL
                                          frc,
                                          NULL,
                                          MB_CANCEL,
                                          TRUE); // in: as in cmnDescribeError
                    }
                #endif

                if (frc)
                    break;

                // pNode = pNode->pNext;

                // now, if the object hasn't been destroyed already,
                // take if off the list so we can take the first
                // object again (speed)
                if (fTakeOff)
                    _Remove(pftl->pllObjects, fu.pSourceObject);

                ulCurrentObject++;
                fu.ulProgressScalar = ulCurrentObject * 100; // for progress
            } // end while (pNode)

            // call progress callback to say "done"
            fopsCallProgressCallback(pftl,
                                     FOPSPROG_DONEWITHALL | FOPSPROG_UPDATE_PROGRESS,
                                     &fu);      // NULL means done
        } // end if (pftl)

        // even with errors, delete file task list now
        fopsDeleteFileTaskList(hftl);
                // this unlocks the folders also
    }
    CATCH(excpt2)
    {
        frc == FOPSERR_FILE_THREAD_CRASHED;
    } END_CATCH();

    if (hwndNotify)
        // synchronous mode:
        // post msg to thread-1 object window;
        // this leaves the modal message loop in fopsStartTask
        WinPostMsg(hwndNotify,
                   T1M_FOPS_TASK_DONE,
                   pftl,            // ptr no longer valid, but used
                                    // as identifier
                   (MPARAM)frc);
}

/*
 *@@ fopsDeleteTaskList:
 *      frees all resources allocated with
 *      the given file-task list and unlocks
 *      the folders involved in the operation.
 *
 *      Note: You can call this manually to
 *      delete a file-task list which you have
 *      created using fopsCreateFileTaskList,
 *      but you MUST call this function on the
 *      same thread which called fopsCreateFileTaskList,
 *      or the WPS will probably hang itself up.
 *
 *      Do NOT call this function after you have
 *      called fopsStartTask because
 *      then the File thread is working on the
 *      list already. The file list will automatically
 *      get cleaned up then, so you only need to
 *      call this function yourself if you choose
 *      NOT to call fopsStartTask.
 *
 *@@added V0.9.1 (2000-01-29) [umoeller]
 */

BOOL fopsDeleteFileTaskList(HFILETASKLIST hftl)
{
    BOOL brc = TRUE;
    PFILETASKLIST pftl = (PFILETASKLIST)hftl;
    if (pftl)
    {
        if (pftl->fTargetLocked)
        {
            _wpReleaseFolderMutexSem(pftl->pTargetFolder);
            pftl->fTargetLocked = FALSE;
        }
        if (pftl->fSourceLocked)
        {
            _wpReleaseFolderMutexSem(pftl->pSourceFolder);
            pftl->fSourceLocked = FALSE;
        }

        _somFree(pftl->pllObjects);     // V1.0.1 (2002-12-15) [umoeller]

        // lstClear(&pftl->llObjects);       // frees items automatically
        free(pftl);
    }
    return brc;
}


