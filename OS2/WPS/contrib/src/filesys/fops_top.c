
/*
 *@@sourcefile fops_top.c:
 *      this has the top layer of the XWorkplace file operations
 *      engine. See fileops.c for an introduction.
 *
 *      The top layer then uses bottom layer (fops_bottom.c) for
 *      the actual XWorkplace file operations. This implements a
 *      generic progress window and proper error handling (hopefully).
 *      If you need to start a file operation which is already
 *      implemented here, you need not bother with the bottom layer
 *      and can simply call one of these functions here.
 *
 *      Presently, the following interfaces have been implemented:
 *
 *      -- fopsStartDeleteFromCnr,
 *      -- fopsStartTrashRestoreFromCnr,
 *      -- fopsStartTrashDestroyFromCnr,
 *      -- fopsStartPopulate.
 *
 *      Alternatively, you can use fopsStartTaskFromCnr and
 *      fopsStartTaskFromList which are used by the above implementations.
 *
 *      All of these functions use fopsStartTask from the bottom layer
 *      in some way, so all the remarks for that function apply.
 *
 *      This has been separated from fileops.c with
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
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\stringh.h"            // string helpers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
#include "xfobj.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\classtest.h"           // some cheap funcs for WPS class checks
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\kernel.h"              // XWorkplace Kernel

#include "filesys\fileops.h"            // file operations implementation

// other SOM headers
#pragma hdrstop                         // VAC++ keeps crashing otherwise
#include <wpfolder.h>
#include "xtrash.h"
#include "xtrashobj.h"
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

/*
 *@@ GENERICPROGRESSWINDATA:
 *      window data stored in QWL_USER of
 *      fops_fnwpGenericProgress.
 *
 *@@added V0.9.1 (2000-01-31) [umoeller]
 */

typedef struct _GENERICPROGRESSWINDATA
{
    HWND        hwndProgress;               // progress window handle
    HWND        hwndProgressBar;            // progress bar control (cached for speed)
    BOOL        fCancelPressed;             // TRUE after Cancel has been
                                            // pressed; the File thread is then
                                            // suspended while a confirmation is
                                            // displayed
} GENERICPROGRESSWINDATA, *PGENERICPROGRESSWINDATA;

/*
 *@@ fopsGenericProgressCallback:
 *      progress callback which gets specified when
 *      fopsStartTaskFromCnr calls fopsCreateFileTaskList.
 *      Part of the top layer of the XWorkplace file
 *      operations engine.
 *
 *      If this returns FALSE, processing is aborted
 *      in the file thread.
 *
 *      This assumes that a progress window using
 *      fops_fnwpGenericProgress has been created and
 *      ulUser is a PGENERICPROGRESSWINDATA.
 *
 *      Warning: This runs on the File thread. That's why
 *      we send a message to the progress window so that
 *      the File thread is blocked while the progress window
 *      does its processing.
 *
 *@@added V0.9.1 (2000-01-30) [umoeller]
 */

BOOL APIENTRY fopsGenericProgressCallback(PFOPSUPDATE pfu,
                                          ULONG ulUser)
{
    PGENERICPROGRESSWINDATA ppwd = (PGENERICPROGRESSWINDATA)ulUser;

    if (ppwd->hwndProgress)     // V0.9.19 (2002-04-17) [umoeller]
        return (BOOL)WinSendMsg(ppwd->hwndProgress,
                                XM_UPDATE,
                                (MPARAM)pfu,
                                (MPARAM)0);

    return TRUE;
}

/*
 *@@ fopsGenericErrorCallback:
 *      error callback which gets specified when
 *      fopsStartTaskFromCnr calls fopsCreateFileTaskList.
 *      Part of the top layer of the XWorkplace file
 *      operations engine.
 *
 *      This can run on several threads:
 *
 *      -- During fopsValidateObjOperation, this is called on
 *         the thread of the folder view from which the file
 *         operation was started (usually thread 1).
 *
 *      -- While the File thread is processing the file task,
 *         this is called if an error occurs.
 *
 *      Normally, this will return the same APIRET that was
 *      passed in in frError. For certain APIRET codes, this may
 *      return NO_ERROR, and the bottom layer will attempt to
 *      fix the error. However, this usually only works for
 *      things like "delete non-deletable" or something, which
 *      can be resolved.
 *
 *      This thing displays a confirmation box only for errors
 *      which apply to a single trash object.
 *
 *      For other errors, the error code is simply returned to the
 *      caller, which in turn passes the error code to its caller(s).
 *
 *@@added V0.9.2 (2000-03-04) [umoeller]
 *@@changed V0.9.3 (2000-04-25) [umoeller]: reworked error management
 *@@changed V0.9.4 (2000-07-27) [umoeller]: added "yes to all" to some msg boxes
 *@@changed V0.9.12 (2001-05-17) [pr]: beautified object title
 *@@changed V0.9.16 (2001-10-28) [pr]: prevent trap on null title
 *@@changed V0.9.16 (2001-12-06) [umoeller]: added delete folders confirmations
 *@@changed V1.0.1 (2002-12-15) [pr]: fixed font uninstall message @@fixes 36
 */

APIRET APIENTRY fopsGenericErrorCallback(ULONG ulOperation,
                                         WPObject *pObject,
                                         APIRET frError, // in: error reported
                                         PULONG pulIgnoreSubsequent)
                                                            // out: ignore subsequent errors of the same type
{
    CHAR    szMsg[1000];
    PCSZ    apsz[5] = {0};
    ULONG   cpsz = 0,
            ulMsg = 0,
            flFlags = 0;
    ULONG   ulIgnoreFlag = 0;
    PSZ     pszTitle = _wpQueryTitle(pObject);

    // V0.9.16 (2001-10-28) [pr]: Prevent trap on null title
    pszTitle = strdup((pszTitle) ? pszTitle : "");
    strhBeautifyTitle(pszTitle);

    switch (frError)
    {
        case FOPSERR_MOVE2TRASH_READONLY:
            // this comes in for a "move2trash" operation
            // on read-only WPFileSystems
            ulMsg = 179;
            flFlags = MB_YES_YES2ALL_NO | MB_DEFBUTTON3;
            apsz[0] = pszTitle;
            cpsz = 1;
            ulIgnoreFlag = FOPS_ISQ_MOVE2TRASH_READONLY;
        break;

        case FOPSERR_DELETE_CONFIRM_FOLDER:
            // V0.9.16 (2001-12-06) [umoeller]
            // confirm folder deletions
            if (ulOperation == XFT_MOVE2TRASHCAN)
                ulMsg = 236;        // %1 is a folder. Move to trash?
                        // V0.9.19 (2002-05-01) [umoeller]
            else
                ulMsg = 226;        // %1 is a folder. Delete?
            flFlags = MB_YES_YES2ALL_NO | MB_DEFBUTTON3;
            apsz[0] = pszTitle;
            cpsz = 1;
            ulIgnoreFlag = FOPS_ISQ_DELETE_FOLDERS;
        break;

        case FOPSERR_DELETE_READONLY:
            // this comes in for a "delete" operation
            // on read-only WPFileSystems
            ulMsg = 184;
            flFlags = MB_YES_YES2ALL_NO | MB_DEFBUTTON3;
            apsz[0] = pszTitle;
            cpsz = 1;
                // if we return NO_ERROR after this,
                // the fileops engine unlocks the file
            ulIgnoreFlag = FOPS_ISQ_DELETE_READONLY;
        break;

        case FOPSERR_DELETE_NOT_DELETABLE:
            // object not deleteable:
            ulMsg = 185;
            flFlags = MB_CANCEL;
            apsz[0] = pszTitle;
            cpsz = 1;
        break;

        case FOPSERR_WPFREE_FAILED:
        {
            // this was added for giving the user feedback if
            // wpFree failed on an object, we should then allow
            // retry
            // V1.0.0 (2002-09-09) [umoeller]
            APIRET arc2;
            XSTRING str;
            if (!(arc2 = _wpQueryError(pObject)))
                arc2 = FOPSERR_WPFREE_FAILED;

            xstrInit(&str, 0);
            cmnDescribeError(&str, arc2, NULL, TRUE);

            apsz[0] = pszTitle;
            apsz[1] = str.psz;
            if (MBID_RETRY == cmnMessageBoxExt(NULLHANDLE,
                                               104,
                                               apsz,
                                               2,
                                               252,
                                               MB_RETRYCANCEL))
                frError = NO_ERROR;

            xstrClear(&str);
        }
        break;

        default:
            if (    (ulOperation == XFT_INSTALLFONTS)
                 || (ulOperation == XFT_DEINSTALLFONTS)
               )
            {
                // for "install fonts", we should report all errors...
                // the user should know that something went wrong.

                flFlags = MB_YES_YES2ALL_NO | MB_DEFBUTTON3;

                switch (frError)
                {
                    case FOPSERR_FONT_ALREADY_INSTALLED:
                        ulMsg = 201;
                        apsz[0] = pszTitle;
                        cpsz = 1;
                    break;

                    case FOPSERR_FONT_STILL_IN_USE:
                        ulMsg = 204;
                        apsz[0] = pszTitle;
                        cpsz = 1;
                    break;

                    // V1.0.1 (2002-12-15) [pr]: @@fixes 36
                    case FOPSERR_NOT_FONT_FILE:
                        ulMsg = 258;
                        apsz[0] = pszTitle;
                        cpsz = 1;
                    break;

                    case FOPSERR_NOT_FONT_OBJECT:
                        ulMsg = 259;
                        apsz[0] = pszTitle;
                        cpsz = 1;
                    break;

                    case FOPSERR_FONT_ALREADY_DELETED:
                        ulMsg = 260;
                        apsz[0] = pszTitle;
                        cpsz = 1;
                    break;

                    default:
                        ulMsg = 200;
                        sprintf(szMsg, "%d", frError);
                        apsz[0] = szMsg;
                        apsz[1] = pszTitle;
                        cpsz = 2;
                }

                ulIgnoreFlag = FOPS_ISQ_FONTINSTALL;
            }
    }

    if (flFlags)
    {
        ULONG ulrc = cmnMessageBoxExt(NULLHANDLE,
                                      175,
                                      apsz,
                                      cpsz,
                                      ulMsg,
                                      flFlags);
        if (    (ulrc == MBID_OK)
             || (ulrc == MBID_YES)
           )
            frError = NO_ERROR;
        else if (ulrc == MBID_YES2ALL)
        {
            // "yes to all":
            *pulIgnoreSubsequent |= ulIgnoreFlag;
            frError = NO_ERROR;
        }
    }

    free(pszTitle);

    return frError;
}

/*
 *@@ fops_fnwpGenericProgress:
 *      dialog procedure for the standard XWorkplace
 *      file operations status window. This updates
 *      the window's status bar and the informational
 *      static texts for the objects and folders.
 *
 *      Part of the top layer of the XWorkplace file
 *      operations engine.
 *
 *      This receives messages from fopsGenericProgressCallback.
 *
 *@@added V0.9.1 (2000-01-30) [umoeller]
 *@@changed V0.9.12 (2001-05-17) [pr]: beautified object title
 *@@changed V0.9.16 (2001-10-28) [pr]: prevent trap on null title
 */

MRESULT EXPENTRY fops_fnwpGenericProgress(HWND hwndProgress, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_INITDLG:
            cmnSetControlsFont(hwndProgress, 0, 5000);
            ctlProgressBarFromStatic(WinWindowFromID(hwndProgress, ID_SDDI_PROGRESSBAR),
                                     PBA_ALIGNCENTER | PBA_BUTTONSTYLE);
            mrc = WinDefDlgProc(hwndProgress, msg, mp1, mp2);
        break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case DID_CANCEL:
                {
                    PGENERICPROGRESSWINDATA ppwd = WinQueryWindowPtr(hwndProgress, QWL_USER);
                    WinEnableControl(hwndProgress, DID_CANCEL, FALSE);
                    ppwd->fCancelPressed = TRUE;
                }
                break;

                default:
                    mrc = WinDefDlgProc(hwndProgress, msg, mp1, mp2);
            }
        break;

        /*
         * XM_UPDATE:
         *      has a FOPSUPDATE structure in mp1.
         *      If this returns FALSE, processing
         *      is aborted in fopsGenericProgressCallback.
         */

        case XM_UPDATE:
        {
            PGENERICPROGRESSWINDATA ppwd = WinQueryWindowPtr(hwndProgress, QWL_USER);
            PFOPSUPDATE pfu;

            // return value: TRUE means continue
            mrc = (MRESULT)TRUE;

            if (pfu = (PFOPSUPDATE)mp1)
                if ((pfu->flChanged & FOPSPROG_DONEWITHALL) == 0)
                {
                    CHAR    szTargetFolder[CCHMAXPATH] = "";
                    PSZ     pszTargetFolder = NULL;
                    PCSZ    pcszSubObject = NULL;

                    // update source folder?
                    if (pfu->flChanged & FOPSUPD_SOURCEFOLDER_CHANGED)
                    {
                        CHAR        szSourceFolder[CCHMAXPATH] = "";
                        if (pfu->pSourceFolder)
                            _wpQueryFilename(pfu->pSourceFolder, szSourceFolder, TRUE);
                        WinSetDlgItemText(hwndProgress,
                                          ID_XSDI_SOURCEFOLDER,
                                          szSourceFolder);
                    }

                    // update target folder?
                    if (pfu->flChanged & FOPSUPD_TARGETFOLDER_CHANGED)
                    {
                        if (pfu->pTargetFolder)
                        {
                            _wpQueryFilename(pfu->pTargetFolder, szTargetFolder, TRUE);
                            pszTargetFolder = szTargetFolder;
                        }
                    }

                    if (pfu->flChanged & FOPSUPD_SOURCEOBJECT_CHANGED)
                    {
                        if (pfu->pSourceObject)
                        {
                            // V0.9.16 (2001-10-28) [pr]: Prevent trap on null title
                            PSZ pszTitle = _wpQueryTitle(pfu->pSourceObject);

                            if (    pszTitle
                                 && (pszTitle = strdup(pszTitle))
                               )
                            {
                                strhBeautifyTitle(pszTitle);
                                WinSetDlgItemText(hwndProgress,
                                                  ID_XSDI_SOURCEOBJECT,
                                                  pszTitle);
                                free(pszTitle);
                            }
                        }
                    }

                    if (pfu->flChanged & FOPSUPD_EXPANDING_SOURCEOBJECT_1ST)
                    {
                        // expanding source objects list:
                        pcszSubObject = cmnGetString(ID_XSSI_POPULATING);  // pszPopulating
                                            // "Collecting objects..."
                    }

                    if (pfu->flChanged & FOPSUPD_SUBOBJECT_CHANGED)
                    {
                        if (pfu->pcszSubObject)
                            // can be null!
                            pcszSubObject = pfu->pcszSubObject;
                        else
                            // clear:
                            pcszSubObject = "";
                    }

                    // set controls text composed above
                    if (pszTargetFolder)
                        WinSetDlgItemText(hwndProgress,
                                          ID_XSDI_TARGETFOLDER,
                                          pszTargetFolder);

                    if (pcszSubObject)
                        WinSetDlgItemText(hwndProgress,
                                          ID_XSDI_SUBOBJECT,
                                          (PSZ)pcszSubObject);
                    // update status bar?
                    if (pfu->flChanged & FOPSPROG_UPDATE_PROGRESS)
                        WinSendDlgItemMsg(hwndProgress, ID_SDDI_PROGRESSBAR,
                                          WM_UPDATEPROGRESSBAR,
                                          (MPARAM)pfu->ulProgressScalar,
                                          (MPARAM)pfu->ulProgressMax);

                    // has cancel been pressed?
                    // (flag is set from WM_COMMAND)
                    if (ppwd->fCancelPressed)
                    {
                        // display "confirm cancel" message box;
                        // this msg box suspends the File thread because
                        // we're being SENT this message...
                        // so file operations are suspended until we
                        // return TRUE from here!
                        PSZ pszTitle = winhQueryWindowText(hwndProgress);
                        XSTRING strMsg;
                        xstrInit(&strMsg, 0);
                        cmnGetMessage(NULL, 0,
                                      &strMsg,
                                      186);     // "really cancel?"
                        if (cmnMessageBox(hwndProgress, // owner
                                          pszTitle,
                                          strMsg.psz,
                                          NULLHANDLE, // no help
                                          MB_RETRYCANCEL)
                                != MBID_RETRY)
                        {
                            // cancel: return FALSE
                            mrc = (MRESULT)FALSE;
                        }
                        else
                        {
                            WinEnableControl(hwndProgress, DID_CANCEL, TRUE);
                            ppwd->fCancelPressed = FALSE;
                        }

                        if (pszTitle)
                            free(pszTitle);
                        xstrClear(&strMsg);
                    }
                } // end if (pfu)
                else
                {
                    // FOPSPROG_LASTCALL_DONE set:
                    // disable "Cancel"
                    WinEnableControl(hwndProgress, DID_CANCEL, FALSE);
                    if (!ppwd->fCancelPressed)
                        // set progress to 100%
                        WinSendMsg(ppwd->hwndProgressBar,
                                   WM_UPDATEPROGRESSBAR,
                                   (MPARAM)100,
                                   (MPARAM)100);
                    // start timer to destroy window
                    WinStartTimer(WinQueryAnchorBlock(hwndProgress),
                                  hwndProgress,
                                  1,
                                  500);
                }
        }
        break;

        case WM_TIMER:
            if ((SHORT)mp1 == 1)
            {
                // "destroy window" timer
                WinStopTimer(WinQueryAnchorBlock(hwndProgress),
                             hwndProgress,
                             1);

                // store window position
                winhSaveWindowPos(hwndProgress,
                                  HINI_USER,
                                  INIAPP_XWORKPLACE, INIKEY_FILEOPSPOS);
                // clean up
                free(WinQueryWindowPtr(hwndProgress, QWL_USER));
                winhDestroyWindow(&hwndProgress);
            }
            else
                mrc = WinDefDlgProc(hwndProgress, msg, mp1, mp2);
        break;

        default:
            mrc = WinDefDlgProc(hwndProgress, msg, mp1, mp2);
    }

    return mrc;
}

/*
 * StartWithGenericProgress:
 *      starts processing the given file task list
 *      with the generic XWorkplace file progress
 *      dialog (fops_fnwpGenericProgress).
 *      Part of the top layer of the XWorkplace file
 *      operations engine.
 *
 *      This calls fopsStartTask in turn.
 *
 *      Gets called by fopsStartTaskFromCnr.
 *
 *@@added V0.9.1 (2000-02-01) [umoeller]
 *@@changed V0.9.4 (2000-08-03) [umoeller]: added NLS
 *@@changed V0.9.19 (2002-04-17) [umoeller]: no longer displaying progress if disabled in WPSystem
 *@@changed V0.9.19 (2002-06-13) [umoeller]: added excpt handling
 *@@changed V0.9.19 (2002-06-13) [umoeller]: fixed crash if pSourceFolder was NULL (font install)
 *@@changed V1.0.1 (2002-12-15) [pr]: fixed "unknown task" message box when uninstalling fonts
 */

STATIC APIRET StartWithGenericProgress(HFILETASKLIST hftl,
                                       ULONG ulOperation,
                                       HAB hab,  // in: as with fopsStartTask
                                       WPFolder *pSourceFolder, // in: source folder, as required by fopsCreateFileTaskList
                                       PGENERICPROGRESSWINDATA ppwd)
{
    APIRET frc = NO_ERROR;

    TRY_LOUD(excpt1)
    {
        PCSZ pcszTitle = "unknown task";
        WPObject *pobjConf;

        // pSourceFolder can be NULL for install fonts, so the below
        // query confirmations crashed with every font install...
        // V0.9.19 (2002-06-13) [umoeller]
        if (!(pobjConf = pSourceFolder))
            pobjConf = cmnQueryActiveDesktop();

        memset(ppwd, 0, sizeof(GENERICPROGRESSWINDATA));

        // V0.9.19 (2002-04-17) [umoeller]
        // check if progress dialog is enabled in WPSystem
        if (_wpQueryConfirmations(pobjConf) & CONFIRM_PROGRESS)
        {
            // load progress dialog
            ppwd->hwndProgress = cmnLoadDlg(NULLHANDLE, // owner
                                            fops_fnwpGenericProgress,
                                            ID_XFD_FILEOPSSTATUS,
                                            NULL);

            ppwd->hwndProgressBar = WinWindowFromID(ppwd->hwndProgress,
                                                    ID_SDDI_PROGRESSBAR);
            // store in window words
            WinSetWindowPtr(ppwd->hwndProgress, QWL_USER, ppwd);

            // determine title for the progress window
            switch (ulOperation)
            {
                case XFT_MOVE2TRASHCAN:
                    pcszTitle = cmnGetString(ID_XSSI_FOPS_MOVE2TRASHCAN);
                break;

                case XFT_RESTOREFROMTRASHCAN:
                    pcszTitle = cmnGetString(ID_XSSI_FOPS_RESTOREFROMTRASHCAN);
                break;

                case XFT_TRUEDELETE:
                    pcszTitle = cmnGetString(ID_XSSI_FOPS_TRUEDELETE);
                break;

                case XFT_INSTALLFONTS:
                    pcszTitle = cmnGetString(ID_XSSI_INSTALLINGFONTS);
                break;

                // V1.0.1 (2002-12-15) [pr]: fix "unknown task" message box
                case XFT_DEINSTALLFONTS:
                    pcszTitle = cmnGetString(ID_XSSI_DEINSTALLINGFONTS);
                break;
            }

            // set progress window title
            WinSetWindowText(ppwd->hwndProgress, (PSZ)pcszTitle);

            winhCenterWindow(ppwd->hwndProgress);
            // get last window position from INI
            winhRestoreWindowPos(ppwd->hwndProgress,
                                 HINI_USER,
                                 INIAPP_XWORKPLACE, INIKEY_FILEOPSPOS,
                                 // move only, no resize
                                 SWP_MOVE | SWP_SHOW | SWP_ACTIVATE);
            // *** go!!!
            WinShowWindow(ppwd->hwndProgress, TRUE);
        }

        frc = fopsStartTask(hftl,
                            hab);
    }
    CATCH(excpt1)
    {
        frc = ERROR_PROTECTION_VIOLATION;       // V0.9.19 (2002-06-12) [umoeller]
    } END_CATCH();

    return frc;
}

/*
 *@@ fopsStartTaskFromCnr:
 *      this sets up the generic file operations progress
 *      dialog (fops_fnwpGenericProgress), creates a file
 *      task list for the given job (fopsCreateFileTaskList),
 *      adds objects to that list (fopsAddObjectToTask), and
 *      starts processing (fopsStartTask), all in
 *      one shot.
 *
 *      Entry point to the top layer of the XWorkplace file
 *      operations engine.
 *
 *      The actual file operation calls are then made on the
 *      XWorkplace File thread (xthreads.c), which calls
 *      fopsFileThreadProcessing in this file.
 *
 *      pSourceObject must have the first object which is to
 *      be worked on, that is, the object for which the context
 *      menu had been opened. This should be the result of a
 *      previous wpshQuerySourceObject call while the context
 *      menu was open. This can be the object under the mouse
 *      or (if ulSelection == SEL_WHITESPACE) the folder itself
 *      also.
 *
 *      If (ulSelection == SEL_MULTISEL), this func will use
 *      wpshQueryNextSourceObject to get the other objects
 *      because several objects are to be deleted.
 *
 *      If you want the operation confirmed, specify message
 *      numbers with pConfirm. Those numbers must match a
 *      message in the XWorkplace NLS text message file
 *      (XFLDRxxx.TMF).
 *
 *      -- If a single object is selected in the container,
 *         FOPSCONFIRM.ulMsgSingle will be used. The "%1" parameter in the
 *         message will be replaced with the object's title.
 *
 *      -- If multiple objects are selected in the container,
 *         FOPSCONFIRM.ulMsgMultiple will be used. The "%1" parameter in
 *         the message will then be replaced with the number
 *         of objects to be worked on.
 *
 *      If pConfirm is NULL, no confirmations are displayed.
 *
 *      This returns NO_ERROR if the task was successfully started
 *      or some other error code if not.
 *
 *@@added V0.9.1 (2000-01-31) [umoeller]
 *@@changed V0.9.3 (2000-04-10) [umoeller]: added confirmation messages
 *@@changed V0.9.3 (2000-04-11) [umoeller]: fixed memory leak when errors occurred
 *@@changed V0.9.3 (2000-04-25) [umoeller]: reworked error management
 *@@changed V0.9.3 (2000-04-28) [umoeller]: added fRelatedObject
 *@@changed V0.9.4 (2000-08-03) [umoeller]: now checking for "no objects"
 *@@changed V0.9.12 (2001-05-17) [pr]: beautified object title
 *@@changed V0.9.16 (2001-10-28) [pr]: prevent trap on null title
 *@@changed V0.9.19 (2002-04-24) [umoeller]: added help support for confirmations
 */

APIRET fopsStartTaskFromCnr(ULONG ulOperation,       // in: operation; see fopsCreateFileTaskList
                            HAB hab,                 // in: as with fopsStartTask
                            WPFolder *pSourceFolder, // in: source folder, as required by fopsCreateFileTaskList
                            WPFolder *pTargetFolder, // in: target folder, as required by fopsCreateFileTaskList
                            WPObject *pSourceObject, // in: first object with source emphasis
                            ULONG ulSelection,       // in: SEL_* flags
                            BOOL fRelatedObjects,    // in: if TRUE, then the objects must be trash objects,
                                                      // and their related objects will be collected instead
                            HWND hwndCnr,            // in: container to get more source objects from
                            PFOPSCONFIRM pConfirm)   // in: confirmations or NULL
{
    APIRET frc = NO_ERROR;

    WPObject *pObject;

    PGENERICPROGRESSWINDATA ppwd;

    if (pObject = pSourceObject)
    {
        // allocate progress window data structure
        // this is passed to fopsCreateFileTaskList as ulUser
        if (ppwd = malloc(sizeof(GENERICPROGRESSWINDATA)))
        {
            HFILETASKLIST hftl = NULLHANDLE;
            ULONG       cObjects = 0;

            // create task list for the desired task
            if (!(frc = fopsCreateFileTaskList(&hftl,
                                               ulOperation,      // as passed to us
                                               pSourceFolder,    // as passed to us
                                               pTargetFolder,    // as passed to us
                                               fopsGenericProgressCallback, // progress callback
                                               fopsGenericErrorCallback, // error callback
                                               (ULONG)ppwd)))     // ulUser
                            // this locks pSourceFolder
            {
                // now add all objects to the task list
                while (pObject)
                {
                    APIRET      frc2;
                    WPObject    *pAddObject = pObject;
                    PMPF_FOPS(("got object %s", _wpQueryTitle(pObject)));

                    if (fRelatedObjects)
                        // collect related objects instead:
                        // then this better be a trash object
                        pAddObject = _xwpQueryRelatedObject(pObject);

                    if (!(frc2 = fopsAddObjectToTask(hftl,
                                                     pAddObject)))
                    {
                        // raise objects count
                        cObjects++;

                        if (ulSelection == SEL_MULTISEL)
                            // more objects to go:
                            pObject = wpshQueryNextSourceObject(hwndCnr, pObject);
                        else
                            break;
                    }
                    else
                    {
                        // error:
                        frc = frc2;
                        break;
                    }
                }

                if (!frc)
                {
                    if (cObjects == 0)
                        // no objects and no other error reported:
                        frc = FOPSERR_NO_OBJECTS_FOUND;
                    else
                    {
                        // confirmations?
                        if (pConfirm)
                        {
                            // yes:
                            PCSZ    apsz = NULL;
                            ULONG   ulMsg;

                            PMPF_FOPS(("confirming, cObjects = %d", cObjects));

                            if (cObjects == 1)
                            {
                                // single object:
                                // V0.9.16 (2001-10-28) [pr]: Prevent trap on null title
                                apsz = _wpQueryTitle(pSourceObject);
                                apsz = strdup(apsz ? apsz : "");
                                strhBeautifyTitle((PSZ)apsz);
                                ulMsg = pConfirm->ulMsgSingle;
                            }
                            else
                            {
                                apsz = malloc(30);
                                sprintf((PSZ)apsz, "%d", cObjects);
                                ulMsg = pConfirm->ulMsgMultiple;
                            }

                            if (cmnMessageBoxHelp(pConfirm->hwndOwner,  // owner
                                                  121, // "XWorkplace"
                                                  &apsz,
                                                  1,
                                                  ulMsg,
                                                  pConfirm->ulHelpPanel,  // V0.9.19 (2002-04-24) [umoeller]
                                                  MB_YESNO)
                                        != MBID_YES)
                                frc = FOPSERR_CANCELLEDBYUSER;

                            free((PSZ)apsz);
                        }
                    }
                } // end if (frc == NO_ERROR)

                if (!frc)
                    // *** go!!!
                    frc = StartWithGenericProgress(hftl,
                                                   ulOperation,
                                                   hab,
                                                   pSourceFolder,
                                                   ppwd);
                else
                    // cancel or no success: clean up
                    fopsDeleteFileTaskList(hftl);
            } // end if (!(frc = fopsCreateFileTaskList(&hftl,

            if (frc)
                free(ppwd);     // V0.9.3 (2000-04-11) [umoeller]
        }
        else
            frc = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
        frc = FOPSERR_INVALID_OBJECT;

    PMPF_FOPS(("returning APIRET %d", frc));

    return frc;
}

/*
 *@@ fopsStartTaskFromList:
 *      similar to fopsStartTaskFromCnr, but this
 *      does not check for selected objects,
 *      but adds the objects from the specified
 *      list.
 *
 *      Entry point to the top layer of the XWorkplace file
 *      operations engine.
 *
 *      pllObjects must be a list containing
 *      simple WPObject* pointers. The list is
 *      not freed or modified by this function,
 *      but read only.
 *
 *@@added V0.9.1 (2000-02-01) [umoeller]
 *@@changed V0.9.3 (2000-04-25) [umoeller]: reworked error management
 *@@changed V0.9.4 (2000-08-03) [umoeller]: now checking for "no objects"
 */

APIRET fopsStartTaskFromList(ULONG ulOperation,
                             HAB hab,                 // in: as with fopsStartTask
                             WPFolder *pSourceFolder,
                             WPFolder *pTargetFolder,
                             PLINKLIST pllObjects)      // in: list with WPObject* pointers
{
    APIRET frc = NO_ERROR;

    // allocate progress window data structure
    // this is passed to fopsCreateFileTaskList as ulUser
    PGENERICPROGRESSWINDATA ppwd;

    PMPF_FOPS(("op: %d, source: %s, target: %s",
                ulOperation,
                (pSourceFolder) ? _wpQueryTitle(pSourceFolder) : "NULL",
                (pTargetFolder) ? _wpQueryTitle(pTargetFolder) : "NULL"
                ));

    if (ppwd = malloc(sizeof(GENERICPROGRESSWINDATA)))
    {
        HFILETASKLIST hftl = NULLHANDLE;
        PLISTNODE   pNode = NULL;
        ULONG       cObjects = 0;

        // create task list for the desired task
        if (!(frc = fopsCreateFileTaskList(&hftl,
                                           ulOperation,      // as passed to us
                                           pSourceFolder,    // as passed to us
                                           pTargetFolder,    // as passed to us
                                           fopsGenericProgressCallback, // progress callback
                                           fopsGenericErrorCallback, // error callback
                                           (ULONG)ppwd)))     // ulUser
                            // this locks pSourceFolder
        {
            // add ALL objects from the list
            pNode = lstQueryFirstNode(pllObjects);
            while (pNode)
            {
                WPObject *pObject = (WPObject*)pNode->pItemData;
                APIRET frc2;

                PMPF_FOPS(("got object %s", _wpQueryTitle(pObject) ));

                frc2 = fopsAddObjectToTask(hftl, pObject);
                PMPF_FOPS(("    fopsAddObjectToTask returned %d for that", frc2));
                if (frc2)
                {
                    frc = frc2;
                    break;
                }
                pNode = pNode->pNext;
                ++cObjects;
            }

            if (!cObjects)
                // no objects:
                frc = FOPSERR_NO_OBJECTS_FOUND;

            if (!frc)
            {
                // *** go!!!
                PMPF_TRASHCAN(("calling StartWithGenericProgress with %d objects",
                            cObjects));
                frc = StartWithGenericProgress(hftl,
                                               ulOperation,
                                               hab,
                                               pSourceFolder,
                                               ppwd);

            }
            else
                // cancel or no success: clean up
                fopsDeleteFileTaskList(hftl);

        } // end if (!(frc = fopsCreateFileTaskList(&hftl,

        if (frc)
            free(ppwd);     // V0.9.3 (2000-04-11) [umoeller]
    }
    else
        frc = ERROR_NOT_ENOUGH_MEMORY;

    PMPF_FOPS(("APIRET %d", frc));

    return frc;
}

/********************************************************************
 *
 *   Trash can file operations implementation
 *
 ********************************************************************/

/*
 *@@ fopsStartDeleteFromCnr:
 *      this gets called from fnwpSubclassedFolderFrame
 *      in two situations:
 *
 *      --  if trash can "delete" support is on and a "delete"
 *          command was intercepted to move objects to the trash
 *          can;
 *
 *      --  if "true delete" support is on to truly delete objects.
 *
 *      What this function does, depends on the fTrueDelete parameter.
 *
 *      Entry point to the top layer of the XWorkplace file
 *      operations engine.
 *
 *      This calls fopsStartTaskFromCnr with the proper parameters
 *      to start moving/deleting objects which are selected in
 *      hwndCnr on the File thread.
 *
 *      Note: The "source folder" for fopsStartTaskFromCnr is
 *      automatically determined as the folder in which pSourceObject
 *      resides. This is because if an object somewhere deep in a
 *      Tree view hierarchy gets deleted, the progress window would
 *      otherwise display the main folder being deleted, which
 *      might panic the user. The source folder for delete operations
 *      is only for display anyway; the file operations engine
 *      doesn't really need it.
 *
 *@@added V0.9.1 (2000-01-29) [umoeller]
 *@@changed V0.9.3 (2000-04-25) [umoeller]: reworked error management
 *@@changed V0.9.3 (2000-04-30) [umoeller]: removed pSourceFolder parameter
 *@@changed V0.9.5 (2000-08-11) [umoeller]: confirmations use cnr's frame as owner now
 *@@changed V0.9.9 (2001-02-18) [pr]: fix trap when pSourceObject is NULL
 *@@changed V0.9.19 (2002-04-24) [umoeller]: added help to some confirmations
 *@@changed V1.0.6 (2006-10-14) [pr]: always use true delete for single Printer/Transient objects
 *@@changed V1.0.8 (2008-01-05) [pr]: true delete untrashable objects @@fixes 1035
 */

APIRET fopsStartDeleteFromCnr(HAB hab,                 // in: as with fopsStartTask
                              WPObject *pSourceObject, // in: first object with source emphasis
                              ULONG ulSelection,       // in: SEL_* flag
                              HWND hwndCnr,            // in: container to collect objects from
                              BOOL fTrueDelete)        // in: if TRUE, perform true delete; if FALSE, move to trash can
{
    APIRET      frc = NO_ERROR;

    ULONG       ulOperation = XFT_MOVE2TRASHCAN;
    FOPSCONFIRM Confirm = {0};
    ULONG       ulConfirmations = 0;
    BOOL        fConfirm = FALSE;

    if (!pSourceObject)
        frc = FOPSERR_INVALID_OBJECT;
    else
    {
        WPFolder *pSourceFolder;
        if (!(pSourceFolder = _wpQueryFolder(pSourceObject)))
            frc = FOPSERR_INVALID_OBJECT;
        else
        {
            ulConfirmations = _wpQueryConfirmations(pSourceObject);

            PMPF_FOPS(("first obj is %s", _wpQueryTitle(pSourceObject)));
            PMPF_FOPS(("  ulConfirmations: 0x%lX", ulConfirmations));

            // specify owner for confirmations in any case...
            // we might need this below!
            Confirm.hwndOwner = WinQueryWindow(hwndCnr, QW_PARENT);

            if (ulConfirmations & CONFIRM_DELETE)
                fConfirm = TRUE;

            // V1.0.6 (2006-10-14) [pr]
            if (   (ulSelection != SEL_MULTISEL)
                && (!ctsIsTrashable(pSourceObject))  // V1.0.8 (2008-01-05) [pr]
               )
                fTrueDelete = TRUE;

            if (fTrueDelete)
            {
                ulOperation = XFT_TRUEDELETE;

                if (fConfirm)
                {
                    Confirm.ulMsgSingle = 177;
                    Confirm.ulMsgMultiple = 178;
                }
            }
            else
                // move to trash can:
                if (fConfirm)
                {
                    Confirm.ulMsgSingle = 182;
                    Confirm.ulMsgMultiple = 183;
                }

            frc = fopsStartTaskFromCnr(ulOperation,
                                       hab,
                                       pSourceFolder,
                                       NULL,         // target folder: not needed
                                       pSourceObject,
                                       ulSelection,
                                       FALSE,       // no related objects
                                       hwndCnr,
                                       (fConfirm)
                                            ? &Confirm
                                            : NULL);

            PMPF_FOPS(("fopsStartTaskFromCnr returned %d", frc));

            // if we're in "move to trashcan" mode:
            if (!fTrueDelete)      // delete into trashcan
            {
                // if the drive is not supported by the trash can,
                // do a confirmed delete instead
                switch (frc)
                {
                    case FOPSERR_TRASHDRIVENOTSUPPORTED:
                        // source folder is not supported by trash can:
                        // start a "delete" job instead, with the proper
                        // confirmation messages ("Drive not supported, delete instead?")

                        PMPF_FOPS(("FOPSERR_TRASHDRIVENOTSUPPORTED"));

                        Confirm.ulMsgSingle = 180;
                        Confirm.ulMsgMultiple = 181;
                        Confirm.ulHelpPanel = ID_XSH_TRASH_NODRIVESUPPORT;
                                // V0.9.19 (2002-04-24) [umoeller]
                        frc = fopsStartTaskFromCnr(XFT_TRUEDELETE,
                                                   hab,
                                                   pSourceFolder,
                                                   NULL,         // target folder: not needed
                                                   pSourceObject,
                                                   ulSelection,
                                                   FALSE,       // no related objects
                                                   hwndCnr,
                                                   &Confirm);
                    break;

                    case FOPSERR_NO_TRASHCAN:
                        if (cmnMessageBoxHelp(Confirm.hwndOwner,
                                              121, // "XWorkplace"
                                              NULL, 0,
                                              225,   // "trash can disappeared"
                                              ID_XSH_CANNOT_FIND_TRASHCAN,
                                                    // V0.9.19 (2002-04-24) [umoeller]
                                              MB_YESNO)
                                == MBID_YES)
                        {
                            cmnEnableTrashCan(Confirm.hwndOwner,
                                              TRUE);        // enable
                        }
                    break;
                }
            }
        }
    }

    return frc;
}

/*
 *@@ fopsStartTrashRestoreFromCnr:
 *      this gets called from trshProcessViewCommand
 *      if WM_COMMAND with ID_XFMI_OFS_TRASHRESTORE has
 *      been intercepted.
 *
 *      Entry point to the top layer of the XWorkplace file
 *      operations engine.
 *
 *      This calls fopsStartTaskFromCnr with the proper parameters
 *      to start restoring the selected trash objects on the File thread.
 *
 *@@added V0.9.1 (2000-01-31) [umoeller]
 */

APIRET fopsStartTrashRestoreFromCnr(HAB hab,                 // in: as with fopsStartTask
                                    WPFolder *pTrashSource,  // in: XWPTrashCan* to restore from
                                    WPFolder *pTargetFolder, // in: specific target folder or NULL for
                                                             // each trash object's original folder
                                    WPObject *pSourceObject, // in: first object with source emphasis
                                    ULONG ulSelection,       // in: SEL_* flag
                                    HWND hwndCnr)            // in: container to collect objects from
{
    return fopsStartTaskFromCnr(XFT_RESTOREFROMTRASHCAN,
                                hab,
                                pTrashSource,
                                pTargetFolder, // can be NULL
                                pSourceObject,
                                ulSelection,
                                FALSE,       // no related objects
                                hwndCnr,
                                NULL);
}

/*
 *@@ fopsStartTrashDestroyFromCnr:
 *      this gets called from trshProcessViewCommand
 *      if WM_COMMAND with ID_XFMI_OFS_TRASHDESTROY has
 *      been intercepted.
 *
 *      Entry point to the top layer of the XWorkplace file
 *      operations engine.
 *
 *      This calls fopsStartTaskFromCnr with the proper parameters
 *      to start destroying the selected trash objects on the File thread.
 *
 *      pSourceObject must point to the first XWPTrashObject to be
 *      destroyed. This function will then start an XFT_TRUEDELETE
 *      job for the related objects.
 *
 *@@added V0.9.1 (2000-01-31) [umoeller]
 *@@changed V0.9.3 (2000-04-28) [umoeller]: switched implementation to "true delete"
 *@@changed V0.9.4 (2000-08-03) [umoeller]: confirmations were always displayed; fixed, added fConfirm
 */

APIRET fopsStartTrashDestroyFromCnr(HAB hab,                 // in: as with fopsStartTask
                                    WPFolder *pTrashSource,  // in: XWPTrashCan* to restore from
                                    WPObject *pSourceObject, // in: first object with source emphasis
                                    ULONG ulSelection,       // in: SEL_* flag
                                    HWND hwndCnr,            // in: container to collect objects from
                                    BOOL fConfirm)           // in: display confirmation?
{
    FOPSCONFIRM Confirm = {0};

    if (fConfirm)
    {
        Confirm.hwndOwner = WinQueryWindow(hwndCnr, QW_PARENT);
        Confirm.ulMsgSingle = 177;
        Confirm.ulMsgMultiple = 178;
    }

    return fopsStartTaskFromCnr(XFT_TRUEDELETE,
                                hab,
                                pTrashSource,
                                NULL,             // no target folder
                                pSourceObject,
                                ulSelection,
                                TRUE,       // collect related objects instead
                                hwndCnr,
                                (fConfirm)
                                   ? &Confirm
                                   : NULL);
}

/*
 *@@ fopsStartPopulate:
 *      populates a folder. This can be used to populate
 *      a folder synchronously without blocking the thread's
 *      message queue.
 *
 *      This starts a file task with XFT_POPULATE. See
 *      fopsStartTask for details.
 *
 *      Entry point to the top layer of the XWorkplace file
 *      operations engine.
 *
 *@@added V0.9.4 (2000-08-03) [umoeller]
 *@@changed V0.9.7 (2001-01-17) [umoeller]: fixed crash with invalid fopsDeleteFileTaskList
 */

APIRET fopsStartPopulate(HAB hab,              // in: as with fopsStartTask
                         WPFolder *pFolder)    // in: folder to populate
{
    APIRET     frc = NO_ERROR;
    HFILETASKLIST hftl = NULLHANDLE;

    // create task list for the desired task
    if (!(frc = fopsCreateFileTaskList(&hftl,
                                       XFT_POPULATE,
                                       NULL, // pSourceFolder,
                                       NULL, // pTargetFolder,
                                       NULL, // fopsGenericProgressCallback,
                                       NULL, // fopsGenericErrorCallback,
                                       0))) // (ULONG)ppwd);     // ulUser
    {
        if (frc = fopsAddObjectToTask(hftl, pFolder))
            // cancel or no success: clean up
            fopsDeleteFileTaskList(hftl);
                        // moved this here... V0.9.7 (2001-01-17) [umoeller]
    }
    else
        frc = FOPSERR_INTEGRITY_ABORT;

    if (!frc)
        // *** go!!!
        frc = fopsStartTask(hftl,
                            hab);

    return frc;
}

/*
 *@@ fopsStartFontDeinstallFromCnr:
 *      this gets called from fonProcessViewCommand
 *      if WM_COMMAND with ID_XFMI_OFS_FONT_DEINSTALL has
 *      been intercepted.
 *
 *      Entry point to the top layer of the XWorkplace file
 *      operations engine.
 *
 *      This calls fopsStartTaskFromCnr with the proper parameters
 *      to start deinstalling the selected font objects on the File
 *      thread.
 *
 *@@added V0.9.7 (2001-01-13) [umoeller]
 */

APIRET fopsStartFontDeinstallFromCnr(HAB hab,          // in: as with fopsStartTask
                                     WPFolder *pFontFolderSource, // in: XWPFontFolder with font objects
                                     WPObject *pSourceObject,  // in: first XWPFontObject
                                     ULONG ulSelection,        // in: SEL_* flag
                                     HWND hwndCnr,             // in: container to collect objects from
                                     BOOL fConfirm)            // in: display confirmation?
{
    FOPSCONFIRM Confirm = {0};

    if (fConfirm)
    {
        Confirm.hwndOwner = WinQueryWindow(hwndCnr, QW_PARENT);
        Confirm.ulMsgSingle = 202;
        Confirm.ulMsgMultiple = 203;
    }

    return fopsStartTaskFromCnr(XFT_DEINSTALLFONTS,
                                hab,
                                pFontFolderSource,
                                NULL,             // no target folder
                                pSourceObject,
                                ulSelection,
                                FALSE,             // no related objects... don't work!
                                hwndCnr,
                                (fConfirm)
                                   ? &Confirm
                                   : NULL);
}

