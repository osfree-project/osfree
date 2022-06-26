
/*
 *@@sourcefile xthreads.c:
 *      this file has the additional XWorkplace threads
 *      to offload tasks to the background.
 *
 *      Currently, there are three additional threads which
 *      are started by xthrStartThreads upon Desktop startup
 *      and are _always_ running:
 *
 *      --  XFolder "Worker" thread / object window (fntWorkerThread, fnwpWorkerObject),
 *          running with Idle priority (which is now configurable, V0.9.0);
 *      --  XFolder "File" thread / object window (fntFileThread, fnwpFileObject;
 *          new with V0.9.0), running at regular priority;
 *      --  XFolder "Bush" thread / object window (fntBushThread, fnwpBushObject),
 *          running at maximum regular priority.
 *
 *      Also, we have functions to communicate with these threads.
 *
 *      Function prefix for this file:
 *      --  xthr*
 *
 *@@header "filesys\xthreads.h"
 */

/*
 *      Copyright (C) 1997-2012 Ulrich M”ller.
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
 *  7)  headers in filesys\ (as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSSESMGR
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINDIALOGS
#define INCL_WINTIMER
#define INCL_WINSHELLDATA
#define INCL_WINSTDCNR

#define INCL_GPIBITMAPS
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <io.h>                 // access etc.
#include <umalloc.h>            // user heaps

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\animate.h"            // icon and other animations
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\eah.h"                // extended attributes helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\shapewin.h"           // shaped windows helper functions
#include "helpers\stringh.h"            // string helper routines
#include "helpers\syssound.h"           // system sound helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\tree.h"               // red-black binary trees
#include "helpers\winh.h"               // PM helper routines
#include "helpers\wphandle.h"           // file-system object handles

// SOM headers which don't crash with prec. header files
#include "xfstart.ih"
#include "xtrash.ih"                    // XWPTrashCan
#include "xtrashobj.ih"                 // XWPTrashCan
#include "xfldr.h"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\init.h"                // XWorkplace initialization
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

// headers in /hook
#include "hook\xwphook.h"

#include "filesys\fileops.h"            // file operations implementation
#include "filesys\folder.h"             // XFolder implementation
#include "filesys\object.h"             // XFldObject implementation
#include "filesys\statbars.h"           // status bar translation logic
#include "filesys\trash.h"              // trash can implementation
#include "filesys\xthreads.h"           // extra XWorkplace threads

#include "config\hookintf.h"             // daemon/hook interface

// headers in /hook
#include "hook\xwphook.h"

// other SOM headers
#pragma hdrstop                 // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// thread infos: moved these here from KERNELGLOBALS
// V0.9.9 (2001-03-07) [umoeller]
static THREADINFO   G_tiWorkerThread,
                    G_tiBushThread,
                    G_tiWimpThread,
                    G_tiFileThread;

// Worker thread -- awake objects
static HMTX         G_hmtxAwakeObjectsList = NULLHANDLE;    // V0.9.9 (2001-04-04) [umoeller]
static TREE         *G_AwakeObjectsTree;
static LONG         G_lAwakeObjectsCount = 0;           // V0.9.9 (2001-04-04) [umoeller]
static Heap_t       G_AwakeObjectsHeap;                 // user heap for _ucreate
                                                        // V0.9.9 (2001-04-04) [umoeller]
static char         G_HeapStartChunk[_HEAP_MIN_SIZE];   // first block of storage on the heap

// Worker thread -- other data
static HWND         G_hwndWorkerObject = NULLHANDLE;    // V0.9.9 (2001-04-04) [umoeller]
static HAB          G_habWorkerThread = NULLHANDLE;
static HMQ          G_hmqWorkerThread = NULLHANDLE;

// currently waiting messages for Worker thread;
// if this gets too large, its priority will be raised
static HMTX         G_hmtxWorkerThreadData = NULLHANDLE;
static ULONG        G_ulWorkerMsgCount = 0;     // V0.9.9 (2001-04-04) [umoeller]
static BOOL         G_fWorkerThreadHighPriority = FALSE; // V0.9.9 (2001-04-04) [umoeller]

// Bush thread
static HAB          G_habBushThread = NULLHANDLE;
static HMQ          G_hmqBushThread = NULLHANDLE;
static CHAR         G_szBootupStatus[256];
#ifndef __NOBOOTUPSTATUS__
static HWND         G_hwndBootupStatus = NULLHANDLE;
#endif

// File thread
static HAB          G_habFileThread = NULLHANDLE;
static HMQ          G_hmqFileThread = NULLHANDLE;
static ULONG        G_CurFileThreadMsg = 0;
            // current message that File thread is processing,
            // or null if none

/* ******************************************************************
 *
 *   Worker thread
 *
 ********************************************************************/

/*
 *@@ LockWorkerThreadData:
 *      locks G_hmtxWorkerThreadData. Creates the mutex on
 *      the first call.
 *
 *      This mutex is used to protect the global worker
 *      thread data such as the no. of msgs currently
 *      posted and the current worker thread priority.
 *      This used to be protected by the kernel mutex
 *      before V0.9.9, but I decided that a special
 *      mutex for this would probably help system
 *      performance since this is requested for every
 *      single object that gets awakened by the WPS.
 *
 *      Returns TRUE if the mutex was obtained.
 *
 *@@added V0.9.9 (2001-04-04) [umoeller]
 */

STATIC BOOL LockWorkerThreadData(VOID)
{
    if (G_hmtxWorkerThreadData)
        return !DosRequestMutexSem(G_hmtxWorkerThreadData, SEM_INDEFINITE_WAIT);

    return !DosCreateMutexSem(NULL,
                              &G_hmtxWorkerThreadData,
                              0,
                              TRUE);      // request
}

/*
 *@@ UnlockWorkerThreadData:
 *
 *@@added V0.9.9 (2001-04-04) [umoeller]
 */

STATIC VOID UnlockWorkerThreadData(VOID)
{
    DosReleaseMutexSem(G_hmtxWorkerThreadData);
}

/*
 *@@ xthrResetWorkerThreadPriority:
 *      this resets the Worker thread priority
 *      to either maximum regular (if we're
 *      currently in "high priority" mode,
 *      which only happens if too many
 *      messages have piled up) or to what
 *      was specified in the global settings
 *      for "default priority" mode (XWPConfig).
 *
 *      This gets called from RaiseWorkerThreadPriority
 *      and from fncbXWCParanoiaItemChanged (xwpsetup.c).
 *
 *@@changed V0.9.0 [umoeller]: added beeping
 *@@changed V0.9.9 (2001-04-04) [umoeller]: made mutexes more granular
 */

VOID xthrResetWorkerThreadPriority(VOID)
{
    if (LockWorkerThreadData())
    {
        ULONG   ulPrty, ulDelta;

        if (G_fWorkerThreadHighPriority)
        {
            // high priority
            ulPrty = PRTYC_REGULAR;
            ulDelta = +31;

#ifndef __NOPARANOIA__
            if (cmnQuerySetting(sfWorkerPriorityBeep))
                DosBeep(1200, 30);
#endif
        }
        else
        {
            // default priority:
#ifndef __NOPARANOIA__
            switch (cmnQuerySetting(sulDefaultWorkerThreadPriority))
            {
                case 0:     // 0: idle +/-0
#endif
                    ulPrty = PRTYC_IDLETIME;
                    ulDelta = 0;
#ifndef __NOPARANOIA__
                break;

                case 2:     // 2: regular +/-0
                    ulPrty = PRTYC_REGULAR;
                    ulDelta = 0;
                break;

                default:    // 1: idle +31
                    ulPrty = PRTYC_IDLETIME;
                    ulDelta = +31;
                    break;
            }
            if (cmnQuerySetting(sfWorkerPriorityBeep))
                DosBeep(1000, 30);
#endif
        }

        DosSetPriority(PRTYS_THREAD,
                       ulPrty,
                       ulDelta,
                       thrQueryID(&G_tiWorkerThread));

        UnlockWorkerThreadData();
    }
}

/*
 *@@ RaiseWorkerThreadPriority:
 *      depending on fRaise, raise or lower Worker
 *      thread prty.
 *
 *      This gets called automatically if too many
 *      messages have piled up for fnwpWorkerObject.
 *      This gets called from both the thread that
 *      xthrPostWorkerMsg runs on, as well as the
 *      worker thread itself.
 *
 *@@changed V0.9.0 [umoeller]: avoiding duplicate sets now
 *@@changed V0.9.9 (2001-04-04) [umoeller]: made mutexes more granular
 */

STATIC VOID RaiseWorkerThreadPriority(BOOL fRaise)
{
    static BOOL fFirstTime = TRUE;

    BOOL        fSem = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fSem = LockWorkerThreadData())
        {
            if (   (fFirstTime)
                || (fRaise != G_fWorkerThreadHighPriority)
               )
            {
                fFirstTime = FALSE;
                G_fWorkerThreadHighPriority = fRaise;
                xthrResetWorkerThreadPriority();
            }
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (fSem)
        UnlockWorkerThreadData();
}

/*
 *@@ xthrPostWorkerMsg:
 *      this posts a msg to the XFolder Worker thread object window
 *      (fntWorkerThread, fnwpWorkerObject)
 *      and controls that thread's priority at the same time; use this
 *      function instead of WinPostMsg, cos otherwise the
 *      Worker thread gets confused.
 *
 *@@changed V0.9.3 (2000-04-26) [umoeller]: changed kernel locks
 *@@changed V0.9.7 (2000-12-13) [umoeller]: fixed sync problems
 *@@changed V0.9.9 (2001-04-01) [pr]: added log message on fail
 *@@changed V0.9.9 (2001-04-04) [umoeller]: fixed msgq congestion, priority raise works now
 */

BOOL xthrPostWorkerMsg(ULONG msg, MPARAM mp1, MPARAM mp2)
{
    BOOL    brc = FALSE;

    ULONG   tidWorker;
    BOOL    fRaised = FALSE;

    if (    (tidWorker = thrQueryID(&G_tiWorkerThread))
         && (G_hwndWorkerObject)
       )
    {
        BOOL    fWorkerThreadDataLocked = FALSE;

        brc = WinPostMsg(G_hwndWorkerObject, msg, mp1, mp2);

        TRY_LOUD(excpt1)
        {
            if (fWorkerThreadDataLocked = LockWorkerThreadData())
            {
                // message successfully posted:

                G_ulWorkerMsgCount++;
                if (G_ulWorkerMsgCount > 300)
                {
                    // if the Worker thread msg queue gets congested,
                    // boost priority
                    RaiseWorkerThreadPriority(TRUE);

                    fRaised = TRUE;
                }
            }
        }
        CATCH(excpt1)
        {
            brc = FALSE;
        } END_CATCH();

        if (fWorkerThreadDataLocked)
            UnlockWorkerThreadData();

        if (fRaised)
            // if we're not running on the Worker thread,
            // sleep a bit to give the worker time to
            // process V0.9.9 (2001-04-04) [umoeller]
            if (doshMyTID() != tidWorker)
                DosSleep(10);


        if (!brc)
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "Failed to post msg = 0x%lX, mp1 = 0x%lX, mp2 = 0x%lX",
                   msg, mp1, mp2);
    }

    return brc;
}

/*
 *@@ fnwpGenericStatus:
 *      dlg func for bootup status wnd and maybe others.
 */

MRESULT EXPENTRY fnwpGenericStatus(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = NULL;

    switch (msg)
    {
        case WM_INITDLG:
            mrc = (MPARAM)TRUE; // focus change flag;
            // mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;
    }

    return mrc;
}

/*
 *@@ fnwpWorkerObject:
 *      wnd proc for Worker thread object window
 *      (see fntWorkerThread below).
 *
 *@@changed V0.9.0 [umoeller]: moved some features to File thread
 *@@changed V0.9.0 [umoeller]: adjust WOM_PROCESSORDEREDCONTENT for new XFolder::xwpBeginEnumContent functions
 *@@changed V0.9.0 [umoeller]: removed WM_INVALIDATEORDEREDCONTENT
 *@@changed V0.9.0 [umoeller]: WOM_ADDAWAKEOBJECT is now storing plain WPObject pointers (no more OBJECTLISTITEM)
 *@@changed V0.9.3 (2000-04-28) [umoeller]: now pre-resolving wpQueryContent for speed
 *@@changed V0.9.7 (2000-12-08) [umoeller]: fixed crash when kernel globals weren't returned
 *@@changed V0.9.7 (2000-12-08) [umoeller]: got rid of dtGetULongTime
 *@@changed V0.9.9 (2001-04-04) [umoeller]: made mutexes more granular
 *@@changed V0.9.12 (2001-04-28) [umoeller]: moved all the startup crap out of here
 *@@changed V0.9.18 (2002-02-23) [umoeller]: removed dull PM timer by moving code to extra Wimp thread
 */

MRESULT EXPENTRY fnwpWorkerObject(HWND hwndObject, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = NULL;

    switch (msg)
    {

        /*
         *@@ WOM_REFRESHFOLDERVIEWS:
         *      this one is posted by XFolder's overrides of wpMoveObject,
         *      wpSetTitle or wpRefresh with the calling instance's somSelf
         *      in mp1; we will now update the open frame window titles of both
         *      the caller and its open subfolders with the full folder path
         *      Parameters:
         *
         *      XFolder* mp1  folder to update; if this is NULL, all open
         *                  folders on the system will be updated
         *
         *      ULONG mp2: action to take:
         *
         *          --  FDRUPDATE_TITLE (0)
         *          --  FDRUPDATE_REPAINT (1)
         *
         *@@changed V0.9.20 (2002-08-08) [umoeller]: added mp2
         */

        case WOM_REFRESHFOLDERVIEWS:
        {
            XFolder     *pFolder = NULL,
                        *pCalling = (XFolder*)(mp1);

            for ( pFolder = _wpclsQueryOpenFolders(_WPFolder, NULL, QC_FIRST, FALSE);
                  pFolder;
                  pFolder = _wpclsQueryOpenFolders(_WPFolder, pFolder, QC_NEXT, FALSE))
            {
                if (    (wpshCheckObject(pFolder))
                     && (_somIsA(pFolder, _WPFolder))
                   )
                {
                    if (    (!pCalling)
                         || (wpshResidesBelow(pFolder, pCalling))
                       )
                    {
                        // folder applies:
                        fdrUpdateAllFrameWindows(pFolder, (ULONG)mp2);
                    }
                }
            }
        }
        break;

        /*
         * WOM_UPDATEALLSTATUSBARS:
         *      goes thru all open folder views and updates
         *      status bars if necessary.
         *      Parameters:
         *      ULONG mp1   do-what flag
         *                  1:  show or hide status bars according
         *                      to folder instance and Global settings
         *                  2:  reformat status bars (e.g. because
         *                      fonts have changed)
         */

        case WOM_UPDATEALLSTATUSBARS:
            PMPF_STATUSBARS(("WT: WOM_UPDATEALLSTATUSBARS, mp1 = %d", mp1));

            // for each open folder view, call the callback
            // which updates the status bars
            // (stb_UpdateCallback in folder.c)
            fdrForEachOpenGlobalView(stb_UpdateCallback,
                                     (ULONG)mp1);
        break;

        /*
         * WOM_DELETEICONPOSEA:
         *
         */

        case WOM_DELETEICONPOSEA:
        {
            /* this msg is posted when the .ICONPOS EAs are
               do be deleted; we better do this in the
               background thread */
            EABINDING   eab;
            XFolder     *pFolder = (XFolder*)mp1;
            CHAR        szPath[CCHMAXPATH];

            if (!wpshCheckObject(pFolder))
                break;

            _wpQueryFilename(pFolder, szPath, TRUE); // fully qualfd.
            eab.bFlags = 0;
            eab.pszName = ".ICONPOS";
            eab.bNameLength = strlen(eab.pszName);
            eab.pszValue = "";
            eab.usValueLength = 0;
            eaPathWriteOne(szPath, &eab);
        }
        break;

        /*
         * WOM_DELETEFOLDERPOS:
         *      (new with V0.82) this is posted by
         *      XFolder::wpFree (i.e. when a folder
         *      is deleted); we will now search the
         *      OS2.INI file for folderpos entries
         *      for the deleted folder and remove
         *      them. This is only posted if the
         *      corresponding setting in "Workplace
         *      Shell" is on.
         *      Parameters: mp1 contains the
         *          (hex) five-digit handle, which
         *          should always be 0x3xxxx.
         */

        case WOM_DELETEFOLDERPOS:
        {
            // according to Henk Kelder:
            // for each open folder the WPS creates an INI entry, which
            // key is the decimal value of the HOBJECT, followed by
            // some wicked "@" codes, which probably stand for the
            // various folder views. Since I don't know the codes,
            // I'm getting the keys list for PM_Workplace:FolderPos.

            APIRET arc;
            PSZ pszFolderPosKeys = NULL;

            if (!(arc = prfhQueryKeysForApp(HINI_USER,
                                            WPINIAPP_FOLDERPOS,
                                            &pszFolderPosKeys)))
            {
                PSZ     pKey2 = pszFolderPosKeys;
                CHAR    szComp[20];
                ULONG   cbComp;
                sprintf(szComp, "%d@", (ULONG)mp1);
                cbComp = strlen(szComp);

                PMPF_CNRCONTENT(("Checking for folderpos %s", szComp));

                // now walk thru all the keys in the folderpos
                // application and check if it's one for our
                // folder; if so, delete it
                while (*pKey2 != 0)
                {
                    if (!memcmp(szComp, pKey2, cbComp))
                    {
                        PrfWriteProfileData(HINI_USER,
                                            (PSZ)WPINIAPP_FOLDERPOS, pKey2,
                                            NULL, 0);

                        PMPF_CNRCONTENT(("  Deleted %s", pKey2));
                    }

                    pKey2 += strlen(pKey2)+1;
                }

                free(pszFolderPosKeys);
            }
        }
        break;

        #ifdef __DEBUG__
        case XM_CRASH:          // posted by debugging context menu of XFldDesktop
            CRASH;
        break;
        #endif

        default:
            mrc = WinDefWindowProc(hwndObject, msg, mp1, mp2);

    } // end switch

    return mrc;
}

/*
 *@@ fntWorkerThread:
 *      this is the thread function for the XFolder
 *      "Worker" (= background) thread, to which
 *      messages for tasks are passed which are too
 *      time-consuming to be processed on the
 *      Workplace thread; it creates an object window
 *      (using fnwpWorkerObject as the window proc)
 *      and stores its handle in KERNELGLOBALS.hwndWorkerObject.
 *
 *      This thread is created by initMain.
 *
 *@@changed V0.9.7 (2000-12-13) [umoeller]: made awake-objects mutex unnamed
 *@@changed V0.9.7 (2000-12-13) [umoeller]: fixed semaphore protection
 *@@changed V0.9.9 (2001-04-04) [umoeller]: cleaned up global resources, added two more granular mutexes for performance
 */

void _Optlink fntWorkerThread(PTHREADINFO pti)
{
    QMSG            qmsg;
    PSZ             pszErrMsg = NULL;
    BOOL            fTrapped = FALSE;

    if (G_habWorkerThread = WinInitialize(0))
    {
        if (G_hmqWorkerThread = WinCreateMsgQueue(G_habWorkerThread, 4000))
        {
            BOOL fExit = FALSE;

            do
            {
                BOOL    fWorkerSem = FALSE;

                WinCancelShutdown(G_hmqWorkerThread, TRUE);

                WinRegisterClass(G_habWorkerThread,
                                 (PSZ)WNDCLASS_WORKEROBJECT,    // class name
                                 (PFNWP)fnwpWorkerObject,    // Window procedure
                                 0,                  // class style
                                 0);                 // extra window words

                // create Worker object window
                G_hwndWorkerObject = winhCreateObjectWindow(WNDCLASS_WORKEROBJECT, NULL);

                if (!G_hwndWorkerObject)
                    winhDebugBox(HWND_DESKTOP,
                             "XFolder: Error",
                             "XFolder failed to create the Worker thread object window.");
                else
                {
                    // set ourselves to idle-time priority; this will
                    //   be raised to regular when the msg queue becomes congested
                    RaiseWorkerThreadPriority(FALSE);

                    TRY_LOUD(excpt1)
                    {
                        // now enter the message loop
                        while (WinGetMsg(G_habWorkerThread, &qmsg, NULLHANDLE, 0, 0))
                        {
                            if (fWorkerSem = LockWorkerThreadData())
                            {
                                if (G_ulWorkerMsgCount > 0)
                                    G_ulWorkerMsgCount--;

                                if (G_ulWorkerMsgCount < 10)
                                    RaiseWorkerThreadPriority(FALSE);

                                UnlockWorkerThreadData();
                                fWorkerSem = FALSE;
                            }

                            // dispatch the queue msg
                            WinDispatchMsg(G_habWorkerThread, &qmsg);
                        } // loop until WM_QUIT

                        // WM_QUIT:
                        fExit = TRUE;
                    }
                    CATCH(excpt1)
                    {
                        // the exception handler puts us here if an exception occurred:
                        // set flag that exception occurred
                        fTrapped = TRUE;
                    } END_CATCH();

                    if (fWorkerSem)
                    {
                        UnlockWorkerThreadData();
                        fWorkerSem = FALSE;
                    }
                }

                if (fTrapped)
                    // only report the first error, or otherwise we will
                    // jam the system with msg boxes
                    // @@todo get rid of this shit
                    if (!pszErrMsg)
                    {
                        if (pszErrMsg = strdup("An error occurred in the XWorkplace Worker thread. "
                                   "Since the error is probably not serious, "
                                   "the Worker thread will continue to run. "
                                   "However, if errors like these persist, "
                                   "you might want to disable the "
                                   "Worker thread in the \"XWorkplace Setup\" object."))
                            krnPostThread1ObjectMsg(T1M_EXCEPTIONCAUGHT, (MPARAM)pszErrMsg, MPNULL);
                    }

            } while (!fExit);
        }
    }

    // clean up
    winhDestroyWindow(&G_hwndWorkerObject);

    WinDestroyMsgQueue(G_hmqWorkerThread);
    G_hmqWorkerThread = NULLHANDLE;
    WinTerminate(G_habWorkerThread);
    G_habWorkerThread = NULLHANDLE;
}

/* ******************************************************************
 *
 *   File thread
 *
 ********************************************************************/

/*
 *@@ xthrPostFileMsg:
 *      this posts a msg to the File thread object window.
 */

BOOL xthrPostFileMsg(ULONG msg, MPARAM mp1, MPARAM mp2)
{
    BOOL rc = FALSE;
    PCKERNELGLOBALS pKernelGlobals = krnQueryGlobals();

    if (pKernelGlobals)
        if (thrQueryID(&G_tiFileThread))
        {
            if (pKernelGlobals->hwndFileObject)
            {
                rc = WinPostMsg(pKernelGlobals->hwndFileObject,
                                msg,
                                mp1,
                                mp2);
            }
        }

    if (!rc)
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "Failed to post msg = 0x%lX, mp1 = 0x%lX, mp2 = 0x%lX",
               msg, mp1, mp2);

    return rc;
}

/*
 *@@ xthrIsFileThreadBusy:
 *      this can be called to check whether the
 *      File thread is currently working.
 *      If this returns 0, the File thread is idle.
 *
 *      Otherwise this returns the FIM_* message
 *      number which the file thread is currently
 *      working on.
 *
 *@@added V0.9.2 (2000-02-28) [umoeller]
 */

ULONG xthrIsFileThreadBusy(VOID)
{
    return G_CurFileThreadMsg;
}

/*
 *@@ CollectDoubleFiles:
 *      implementation for FIM_DOUBLEFILES.
 *      This runs on the File thread.
 *
 *@@added V0.9.2 (2000-02-21) [umoeller]
 *@@changed V1.0.9 (2010-07-17) [pr]: added large file support @@fixes 586
 */

STATIC VOID CollectDoubleFiles(MPARAM mp1)
{
    PDOUBLEFILES pdf = (PDOUBLEFILES)mp1;

    // get first entry (node item is a PSZ with the directory)
    PLISTNODE pNodePath = lstQueryFirstNode(pdf->pllDirectories);

    PLINKLIST pllFilesTemp = lstCreate(TRUE);   // free items

    pdf->pllDoubleFiles = lstCreate(TRUE);   // free items

    while (pNodePath)
    {
        PSZ             pszDirThis = (PSZ)pNodePath->pItemData;
        // collect files of that directory

        HDIR          hdirFindHandle = HDIR_SYSTEM;
        FILEFINDBUF3L ffb3           = {0};      // returned from FindFirst/Next
        ULONG         ulResultBufLen = sizeof(ffb3);
        ULONG         ulFindCount    = 1;        // look for 1 file at a time
        APIRET        arc;

        // skip "." path entries
        if (strcmp(pszDirThis, ".") != 0)
        {
            CHAR    szSearchMask[CCHMAXPATH];
            sprintf(szSearchMask, "%s\\*", pszDirThis);

            arc = DosFindFirst(szSearchMask,         // file pattern
                               &hdirFindHandle,      // directory search handle
                               FILE_NORMAL,          // search attribute
                               &ffb3,                // result buffer
                               ulResultBufLen,       // result buffer length
                               &ulFindCount,         // number of entries to find
                               FIL_STANDARDL);       // return level 11 file info

            // keep finding the next file until there are no more files
            while (arc == NO_ERROR)
            {
                // file found:
                PFILELISTITEM pfliNew = malloc(sizeof(FILELISTITEM)),
                              pfliExisting = NULL;
                // search all files we've found so far
                // if we have doubles
                PLISTNODE pNodePrevious = lstQueryFirstNode(pllFilesTemp);
                while (pNodePrevious)
                {
                    PFILELISTITEM pfli = (PFILELISTITEM)pNodePrevious->pItemData;
                    if (!stricmp(pfli->szFilename, ffb3.achName))
                    {
                        pfliExisting = pfli;
                        break;
                    }
                    pNodePrevious = pNodePrevious->pNext;
                }

                // append file to temporary list
                strlcpy(pfliNew->szFilename, ffb3.achName, sizeof(pfliNew->szFilename));
                pfliNew->pszDirectory = pszDirThis;
                pfliNew->fDate = ffb3.fdateLastWrite;
                pfliNew->fTime = ffb3.ftimeLastWrite;
                pfliNew->llSize = ffb3.cbFile;
                pfliNew->fProcessed = FALSE;
                lstAppendItem(pllFilesTemp, pfliNew);

                if (pfliExisting)
                {
                    // double item: append copies to doubles list
                    // the copies will not be destroyed

                    PFILELISTITEM pfli2;

                    if (!pfliExisting->fProcessed)
                    {
                        // make copy of existing item for doubles list
                        pfli2 = malloc(sizeof(FILELISTITEM));
                        memcpy(pfli2, pfliExisting, sizeof(FILELISTITEM));
                        lstAppendItem(pdf->pllDoubleFiles, pfli2);
                        pfliExisting->fProcessed = TRUE;
                    }

                    // make copy of new item for doubles list
                    pfli2 = malloc(sizeof(FILELISTITEM));
                    memcpy(pfli2, pfliNew, sizeof(FILELISTITEM));
                    lstAppendItem(pdf->pllDoubleFiles, pfli2);
                }

                ulFindCount = 1;                    // reset find count
                arc = DosFindNext(hdirFindHandle,
                                 &ffb3,             // result buffer
                                 ulResultBufLen,
                                 &ulFindCount);     // number of entries to find

            } // endwhile
        }

        // next dir
        pNodePath = pNodePath->pNext;
    }

    // destroy the temprary list, including all files
    lstFree(&pllFilesTemp);

    WinPostMsg(pdf->hwndNotify,
               pdf->ulNotifyMsg,
               (MPARAM)pdf,
               (MPARAM)0);
}

/*
 *@@ fnwpFileObject:
 *      wnd proc for File thread object window
 *      (see fntFileThread below).
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.3 (2000-04-25) [umoeller]: startup folder was permanently disabled when panic flag was set; fixed
 *@@changed V0.9.3 (2000-04-25) [umoeller]: redid initial desktop open processing
 *@@changed V0.9.9 (2001-03-27) [umoeller]: earlier daemon notification of desktop open
 *@@changed V0.9.10 (2001-04-08) [umoeller]: no earlier daemon notification of desktop open... sigh
 *@@changed V0.9.12 (2001-04-28) [umoeller]: moved all the startup crap out of here
 */

MRESULT EXPENTRY fnwpFileObject(HWND hwndObject, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = NULL;

    G_CurFileThreadMsg = msg;

    switch (msg)
    {
        /*
         *@@ FIM_DESKTOPPOPULATED:
         *      this msg is posted by XFldDesktop::wpPopulate;
         *      we will now go for the XWorkplace startup
         *      processing.
         *
         *      Parameters:
         *      --  WPDesktop* mp1:     currently active Desktop.
         *      --  HWND mp2:           frame of Desktop just opened.
         *
         *@@added V0.9.3 (2000-04-26) [umoeller]
         *@@changed V0.9.4 (2000-08-02) [umoeller]: initial delay now configurable
         *@@changed V0.9.5 (2000-08-26) [umoeller]: reverted to populate from open...
         *@@changed V0.9.12 (2001-04-29) [umoeller]: now starting fntStartupThread
         */

        case FIM_DESKTOPPOPULATED:
            initDesktopPopulated();
        break;

        /*
         *@@ FIM_RECREATECONFIGFOLDER:
         *      this msg is posted to recreate the config
         *      folder and/or installation folders.
         *
         *      (ULONG)mp1 must be one of the following:
         *
         *      -- RCF_DEFAULTCONFIGFOLDER: produce standard
         *         config folder by executing install\crobjXXX.cmd.
         *
         *      -- RCF_MAININSTALLFOLDER: produce full install
         *         folder, including config folder, by executing
         *         install\instlXXX.cmd, which calls crobjXXX.cmd
         *         in turn.
         *
         *@@changed V0.9.16 (2001-10-11) [umoeller]: removed RCF_QUERYACTION
         *@@changed V0.9.16 (2001-10-23) [umoeller]: removed RCF_EMPTYCONFIGFOLDERONLY
         *@@changed V1.0.5 (2006-04-01) [pr]: added title parameter to doshQuickStartSession
         */

        case FIM_RECREATECONFIGFOLDER:
        {
            static BOOL fWarningOpen = FALSE;
            ULONG   ulAction = (ULONG)mp1;
            ULONG   ulReturn = ulAction;

            if (   (ulReturn == RCF_DEFAULTCONFIGFOLDER)
                || (ulReturn == RCF_MAININSTALLFOLDER)
               )
            {
                HWND    hwndCreating;
                CHAR    szPath[CCHMAXPATH], szPath2[CCHMAXPATH];
                if (cmnQueryXWPBasePath(szPath))
                {   // INI entry found: append "\" if necessary
                    PID     pid;
                    ULONG   sid;
                    sprintf(szPath2, "/c %s\\install\\%s%s.cmd",
                            szPath,
                            (ulReturn == RCF_DEFAULTCONFIGFOLDER)
                                ? "crobj"       // script for config folder only
                                : "instl",      // script for full installation folder
                                                // (used after first Desktop startup)
                            cmnQueryLanguageCode());

                    // "creating config" window
                    hwndCreating = cmnLoadDlg(HWND_DESKTOP,
                                              WinDefDlgProc,
                                              ID_XFD_CREATINGCONFIG,
                                              0);
                    WinShowWindow(hwndCreating, TRUE);
                    doshQuickStartSession("cmd.exe",
                                          szPath2,
                                          szPath2 + 3,           // title
                                          SSF_TYPE_DEFAULT,      // session type
                                          FALSE,                 // background
                                          SSF_CONTROL_INVISIBLE, // but auto-close
                                          TRUE,                  // wait
                                          &sid,
                                          &pid,
                                          NULL);

                    winhDestroyWindow(&hwndCreating);
                }
            }
        }
        break;

        /*
         *@@ FIM_PROCESSTASKLIST:
         *      this processes a file-task-list created
         *      using fopsCreateFileTaskList. This message
         *      gets posted by fopsStartProcessingTasks.
         *
         *      This calls fopsFileThreadProcessing in turn.
         *      This message only makes sure that processing
         *      runs on the File thread.
         *
         *      Note: Do not _send_ this msg, since
         *      fopsFileThreadProcessing possibly takes a
         *      long time.
         *
         *      Parameters:
         *      -- HFILETASKLIST mp1: file task list to process.
         *      -- HWND mp2: temporary window for modal processing.
         *
         *@@added V0.9.1 (2000-01-29) [umoeller]
         *@@changed V0.9.4 (2000-08-03) [umoeller]: added hwndNotify
         */

        case FIM_PROCESSTASKLIST:
            fopsFileThreadProcessing(WinQueryAnchorBlock(hwndObject),
                                     (HFILETASKLIST)mp1,
                                     (HWND)mp2);
        break;

        /*
         *@@ FIM_DOUBLEFILES:
         *      collects all files in a given list of directories
         *      and creates a new list of files which occur in
         *      several directories.
         *
         *      This gets called from the "Double files" dialog
         *      on OS/2 kernel's "System path" page.
         *
         *      Parameters:
         *          PDOUBLEFILES mp1:  info structure.
         *
         *      When done, we post DOUBLEFILES.ulNotifyMsg to
         *      the window specified in DOUBLEFILES.hwndNotify.
         *      With that message, the window gets the DOUBLEFILES
         *      structure back in mp1, with pllFiles set to the
         *      new files list of FILELISTITEM's.
         *
         *      It is the responsibility of the notify window to
         *      invoke lstFree() on DOUBLEFILES.pllFiles.
         */

        case FIM_DOUBLEFILES:
            CollectDoubleFiles(mp1);
        break;

        /*
         *@@ FIM_INSERTHOTKEYS:
         *      inserts all object hotkeys as
         *      HOTKEYRECORD records into the given
         *      container window. This is passed
         *      to us from hifKeybdHotkeysInitPage
         *      because this takes several seconds.
         *
         *      Parameters:
         *      -- HWND mp1: container window.
         *      -- PBOOL mp2: flag set to TRUE while we're
         *                    working on this.
         *
         *      Note: this locks all objects to which
         *      object hotkeys have been assigned.
         */

        case FIM_INSERTHOTKEYS:
            hifCollectHotkeys(mp1, mp2);
        break;

        /*
         *@@ FIM_CALCTRASHOBJECTSIZE:
         *      calls trshCalcTrashObjectSize. This
         *      message has only been implemented to
         *      offload this task to the File thread.
         *
         *      Parameters:
         *      -- XWPTrashObject* mp1: trash object.
         *      -- XWPTrashCan* mp2: trash can to which mp1 has been added.
         *
         *@@added V0.9.2 (2000-02-28) [umoeller]
         */

        case FIM_CALCTRASHOBJECTSIZE:
            trshCalcTrashObjectSize((XWPTrashObject*)mp1,
                                    (XWPTrashCan*)mp2);
        break;

        #ifdef __DEBUG__
        case XM_CRASH:          // posted by debugging context menu of XFldDesktop
            CRASH;
        break;
        #endif

        default:
            G_CurFileThreadMsg = 0;
            mrc = WinDefWindowProc(hwndObject, msg, mp1, mp2);
    }

    G_CurFileThreadMsg = 0;

    return mrc;
}

/*
 *@@ fntFileThread:
 *          this is the thread function for the XFolder
 *          "File" thread.
 *
 *          This thread is used for file operations mainly,
 *          but also for other tasks which should not be
 *          processed in the (idle-time) Worker thread.
 *
 *          Post the File object window (fnwpFileObject)
 *          a FIM_* message (using xthrPostFileMsg),
 *          and the corresponding file action will be executed.
 *
 *          As opposed to the "Worker" thread, this thread runs
 *          with normal regular priority (delta +/- 0).
 *
 *          This thread is also created from initMain.
 *
 *@@added V0.9.0 [umoeller]
 */

void _Optlink fntFileThread(PTHREADINFO pti)
{
    QMSG                  qmsg;
    PSZ                   pszErrMsg = NULL;

    TRY_LOUD(excpt1)
    {
        PKERNELGLOBALS pKernelGlobals;
        if (    (G_habFileThread = WinInitialize(0))
             && (G_hmqFileThread = WinCreateMsgQueue(G_habFileThread, 3000))
             && (pKernelGlobals = krnLockGlobals(__FILE__, __LINE__, __FUNCTION__))
           )
        {
            WinCancelShutdown(G_hmqFileThread, TRUE);

            WinRegisterClass(G_habFileThread,
                             (PSZ)WNDCLASS_FILEOBJECT,    // class name
                             (PFNWP)fnwpFileObject,    // Window procedure
                             0,                  // class style
                             0);                 // extra window words

            // set ourselves to regular priority
            DosSetPriority(PRTYS_THREAD,
                           PRTYC_REGULAR,
                           0, // priority delta
                           0);

            // create object window
            if (!(pKernelGlobals->hwndFileObject = winhCreateObjectWindow(WNDCLASS_FILEOBJECT,
                                                                          NULL)))
                winhDebugBox(HWND_DESKTOP,
                         "XFolder: Error",
                         "XFolder failed to create the File thread object window.");

            krnUnlockGlobals();
            pKernelGlobals = NULL;

            // now enter the message loop
            while (WinGetMsg(G_habFileThread, &qmsg, NULLHANDLE, 0, 0))
                // loop until WM_QUIT
                WinDispatchMsg(G_habFileThread, &qmsg);
        }
    }
    CATCH(excpt1)
    {
        // the thread exception handler puts us here if an exception occurred:
        // clean up

        // only report the first error, or otherwise we will
        // jam the system with msg boxes @@todo get rid of this shit
        if (!pszErrMsg)
        {
            if (pszErrMsg = strdup("An error occurred in the XFolder File thread.\n"
                                  "The File thread has been terminated. This severely limits "
                                  "XWorkplace's functionality. Please restart the WPS now "
                                  "to have the File thread restarted also. "))
                krnPostThread1ObjectMsg(T1M_EXCEPTIONCAUGHT, (MPARAM)pszErrMsg, MPNULL);
        }
        // get out of here
    } END_CATCH();

    {
        PKERNELGLOBALS pKernelGlobals = krnLockGlobals(__FILE__, __LINE__, __FUNCTION__);
        if (pKernelGlobals)
        {
            winhDestroyWindow(&pKernelGlobals->hwndFileObject);
            krnUnlockGlobals();
        }
        WinDestroyMsgQueue(G_hmqFileThread);
        G_hmqFileThread = NULLHANDLE;
        WinTerminate(G_habFileThread);
        G_habFileThread = NULLHANDLE;
    }
}

/* ******************************************************************
 *
 *   here come the Bush thread functions
 *
 ********************************************************************/

/*
 *@@ xthrPostBushMsg:
 *      this posts a msg to the XFolder Bush thread object window.
 */

BOOL xthrPostBushMsg(ULONG msg, MPARAM mp1, MPARAM mp2)
{
    BOOL rc = FALSE;
    PCKERNELGLOBALS pKernelGlobals = krnQueryGlobals();

    if (thrQueryID(&G_tiBushThread))
        if (pKernelGlobals->hwndBushObject)
            rc = WinPostMsg(pKernelGlobals->hwndBushObject,
                            msg,
                            mp1,
                            mp2);
    if (!rc)
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "Failed to post msg = 0x%lX, mp1 = 0x%lX, mp2 = 0x%lX",
               msg, mp1, mp2);

    return rc;
}

#ifndef __NOBOOTLOGO__
SHAPEFRAME sb = {0};
#endif

/*
 *@@ fnwpBushObject:
 *      wnd proc for Bush thread object window
 *      (see fntBushThread below)
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new global settings
 *@@changed V0.9.3 (2000-04-25) [umoeller]: moved all multimedia stuff to media\mmthread.c
 */

MRESULT EXPENTRY fnwpBushObject(HWND hwndObject, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = NULL;

    switch (msg)
    {

#ifndef __NOBOOTLOGO__

        /*
         * WM_CREATE:
         *      show XFolder logo at bootup
         */

        case WM_CREATE:
        {
            PCKERNELGLOBALS pKernelGlobals = krnQueryGlobals();
            // these are already initialized by xfobjM_wpclsInitData

            memset(&sb, 0, sizeof(sb));

            // boot logo disabled because Shift pressed at Desktop startup?
            if (    (!(initQueryPanicFlags() & SUF_SKIPBOOTLOGO))
                    // no: logo allowed?
                 && (cmnQuerySetting(sfBootLogo))
               )
            {
                PSZ pszBootLogoFile = cmnQueryBootLogoFile();

                if (cmnQuerySetting(sulBootLogoStyle) == 1)
                {
                    // blow-up mode:
                    HDC         hdcMem;
                    HPS         hpsMem;
                    SIZEL       szlPage = {0, 0};
                    if (gpihCreateMemPS(G_habBushThread,
                                        &szlPage,
                                        &hdcMem,
                                        &hpsMem))
                    {
                        HBITMAP hbmBootLogo;
                        if (!gpihLoadBitmapFile(&hbmBootLogo,
                                                hpsMem,
                                                pszBootLogoFile))
                        {
                            HPS     hpsScreen = WinGetScreenPS(HWND_DESKTOP);
                            anmBlowUpBitmap(hpsScreen,
                                            hbmBootLogo,
                                            1000);  // total animation time
                            WinReleasePS(hpsScreen);

                            // delete the bitmap again
                            GpiDeleteBitmap(hbmBootLogo);
                        }
                        GpiDestroyPS(hpsMem);
                        DevCloseDC(hdcMem);
                    }
                }
                else
                {
                    // transparent mode:
                    if (shpLoadBitmap(G_habBushThread,
                                      pszBootLogoFile, // from file,
                                      0, 0,     // not from resources
                                      &sb))
                        // create shape (transparent) windows
                        shpCreateWindows(&sb);
                }

                free(pszBootLogoFile);
            } // end if (cmnQuerySetting(sfBootLogo))

            mrc = WinDefWindowProc(hwndObject, msg, mp1, mp2);
        }
        break;

        /*
         * QM_DESTROYLOGO:
         *
         */

        case QM_DESTROYLOGO:
        {
            if (cmnQuerySetting(sulBootLogoStyle) == 0)
                // was bitmap window created successfully?
                if (sb.hwndShapeFrame)
                {
                    // yes: clean up
                    shpFreeBitmap(&sb);
                    winhDestroyWindow(&sb.hwndShapeFrame) ;
                    winhDestroyWindow(&sb.hwndShape);
                }
        }
        break;
#endif

#ifndef __NOBOOTUPSTATUS__

        /*
         * QM_BOOTUPSTATUS:
         *      posted by XFldObject::wpclsInitData every
         *      time a WPS class gets initialized and
         *      "Report class initialization" on the "WPS
         *      Classes" page is on. We will then display
         *      a little window for a few seconds.
         *      Parameters: mp1 points to the SOM class
         *      object being initalized; if NULL, then
         *      the bootup status wnd will be destroyed.
         */

        case QM_BOOTUPSTATUS:
        {
            if (G_hwndBootupStatus == NULLHANDLE)
            {
                // window currently not visible:
                // create it
                strcpy(G_szBootupStatus, "Loaded %s");
                G_hwndBootupStatus = cmnLoadDlg(HWND_DESKTOP,
                                                fnwpGenericStatus,
                                                ID_XFD_BOOTUPSTATUS,
                                                NULL);
                WinSetWindowPos(G_hwndBootupStatus,
                                HWND_TOP,
                                0, 0, 0, 0,
                                SWP_SHOW); // show only, do not activate;
                                    // we don't want the window to get the
                                    // focus, because this would close
                                    // open menus and such
            }

            if (G_hwndBootupStatus)
                // window still visible or created anew:
                if (mp1 == NULL)
                    winhDestroyWindow(&G_hwndBootupStatus);
                else
                {
                    CHAR szTemp[2000];
                    WPObject* pObj = (WPObject*)mp1;

                    // get class title (e.g. "WPObject")
                    PSZ pszClassTitle = _somGetName(pObj);
                    if (pszClassTitle)
                        sprintf(szTemp, G_szBootupStatus, pszClassTitle);
                    else sprintf(szTemp, G_szBootupStatus, "???");

                    WinSetDlgItemText(G_hwndBootupStatus,
                                      ID_XFDI_BOOTUPSTATUSTEXT,
                                      szTemp);

                    // show window for 2 secs
                    WinStartTimer(G_habBushThread, hwndObject, 1, 2000);
                }
        }
        break;

        /*
         * WM_TIMER:
         *      destroy bootup status wnd
         */

        case WM_TIMER:
        {
            switch ((USHORT)mp1) // timer id
            {

                // timer 1: bootup status wnd
                case 1:
                    winhDestroyWindow(&G_hwndBootupStatus);
                    WinStopTimer(G_habWorkerThread, hwndObject, 1);
                break;
            }
        }
        break;
#endif

        /*
         *@@ QM_TREEVIEWAUTOSCROLL:
         *     this msg is posted mainly by fnwpSubclassedFolderFrame
         *     (subclassed folder windows) after the "plus" sign has
         *     been clicked on (WM_CONTROL for containers with
         *     CN_EXPANDTREE notification).
         *
         *      Parameters:
         *          HWND mp1:    frame wnd handle
         *          PMINIRECORDCORE mp2:
         *                       the expanded minirecordcore
         *
         *@@changed V0.9.4 (2000-06-16) [umoeller]: moved this to Bush thread from File thread
         *@@changed V0.9.9 (2001-03-11) [umoeller]: fixed possible crash with broken shadows
         */

        case QM_TREEVIEWAUTOSCROLL:
        {
            WPObject    *pFolder = OBJECT_FROM_PREC((PMINIRECORDCORE)mp2);

            if (wpshCheckObject(pFolder))
            {
                if (pFolder = _xwpResolveIfLink(pFolder))  // V0.9.9 (2001-03-11) [umoeller]
                {
                    if (!_somIsA(pFolder, _WPFolder)) // check only folders, avoid disks
                        break;

                    // now check if the folder whose "plus" sign has been
                    // clicked on is already populated: if so, the WPS seems
                    // to insert the objects directly (i.e. in the Workplace
                    // thread), if not, the objects are inserted by some
                    // background populate thread, which we have no control
                    // over...
                    if ( (_wpQueryFldrFlags(pFolder) & FOI_POPULATEDWITHFOLDERS) == 0)
                    {
                        // NOT fully populated: sleep a while, then post
                        // the same msg again, until the "populated" folder
                        // flag has been set
                        DosSleep(100);
                        xthrPostBushMsg(QM_TREEVIEWAUTOSCROLL, mp1, mp2);
                    }
                    else
                    {
                        // otherwise: scroll the tree view properly
                        HWND                hwndCnr = WinWindowFromID((HWND)mp1, 0x8008);
                        PMINIRECORDCORE     preccLastChild;
                        preccLastChild = WinSendMsg(hwndCnr,
                                                    CM_QUERYRECORD,
                                                    mp2,   // PMINIRECORDCORE
                                                    MPFROM2SHORT(CMA_LASTCHILD,
                                                                 CMA_ITEMORDER));
                        cnrhScrollToRecord(hwndCnr,
                                           (PRECORDCORE)preccLastChild,
                                           CMA_TEXT,
                                           TRUE);
                    }
                }
            }
        }
        break;

        default:
            mrc = WinDefWindowProc(hwndObject, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ fntBushThread:
 *          this is the thread function for the XFolder
 *          "Bush" thread, which is responsible for
 *
 *          --  showing the bootup logo
 *
 *          --  displaying the notification window when classes
 *              get initialized
 *
 *          --  tree-view auto-scroll.
 *
 *          As opposed to the "Worker" thread, this thread runs
 *          with high regular priority.
 *
 *          This thread is also created from initMain.
 *
 *@@changed V0.9.3 (2000-04-25) [umoeller]: moved all multimedia stuff to media\mmthread.c
 *@@changed V0.9.18 (2002-02-23) [umoeller]: renamed from "Speedy" thread
 */

void _Optlink fntBushThread(PTHREADINFO pti)
{
    QMSG                  qmsg;
    PSZ                   pszErrMsg = NULL;

    TRY_LOUD(excpt1)
    {
        if (G_habBushThread = WinInitialize(0))
        {
            if (G_hmqBushThread = WinCreateMsgQueue(G_habBushThread, 3000))
            {
                PKERNELGLOBALS pKernelGlobals;
                if (pKernelGlobals = krnLockGlobals(__FILE__, __LINE__, __FUNCTION__))
                {
                    WinCancelShutdown(G_hmqBushThread, TRUE);

                    WinRegisterClass(G_habBushThread,
                                     (PSZ)WNDCLASS_QUICKOBJECT,    // class name
                                     (PFNWP)fnwpBushObject,    // Window procedure
                                     0,                  // class style
                                     0);                 // extra window words

                    // set ourselves to higher regular priority
                    DosSetPriority(PRTYS_THREAD,
                                   PRTYC_REGULAR,
                                   +31, // priority delta
                                   0);

                    // create object window
                    pKernelGlobals->hwndBushObject
                        = winhCreateObjectWindow(WNDCLASS_QUICKOBJECT, NULL);

                    if (!pKernelGlobals->hwndBushObject)
                        winhDebugBox(HWND_DESKTOP,
                                 "XFolder: Error",
                                 "XFolder failed to create the Bush thread object window.");

                    krnUnlockGlobals();
                    pKernelGlobals = NULL;
                }

                // now enter the message loop
                while (WinGetMsg(G_habBushThread, &qmsg, NULLHANDLE, 0, 0))
                    WinDispatchMsg(G_habBushThread, &qmsg);
                                // loop until WM_QUIT
            }
        }
    }
    CATCH(excpt1)
    {
        // the thread exception handler puts us here if an exception occurred:
        // clean up @@todo get rid of this shit
        if (!pszErrMsg)
        {
            // only report the first error, or otherwise we will
            // jam the system with msg boxes
            if (pszErrMsg = strdup("An error occurred in the XFolder Bush thread. "
                        "\n\nThe additional XFolder system sounds will be disabled for the "
                        "rest of this Workplace Shell session. You will need to restart "
                        "the WPS in order to re-enable them. "
                        "\n\nIf errors like these persist, you might want to disable the "
                        "additional XFolder system sounds again. For doing this, execute "
                        "SOUNDOFF.CMD in the BIN subdirectory of the XFolder installation "
                        "directory. "))
                krnPostThread1ObjectMsg(T1M_EXCEPTIONCAUGHT, (MPARAM)pszErrMsg, MPNULL);
        }

        // disable sounds
    } END_CATCH();

    {
        PKERNELGLOBALS pKernelGlobals = krnLockGlobals(__FILE__, __LINE__, __FUNCTION__);
        if (pKernelGlobals)
            winhDestroyWindow(&pKernelGlobals->hwndBushObject);

        WinDestroyMsgQueue(G_hmqBushThread);
        G_hmqBushThread = NULLHANDLE;
        WinTerminate(G_habBushThread);
        G_habBushThread = NULLHANDLE;

        krnUnlockGlobals();
    }
}

/* ******************************************************************
 *
 *   Wimp thread
 *
 ********************************************************************/

/*
 *@@ fntWimpThread:
 *      this thread is new with V0.9.18 and runs with
 *      idle time priority to perform various tasks
 *      every few minutes. This code was moved to this
 *      separate thread from the Worker thread to save
 *      another PM timer that was previously used for
 *      close to nothing.
 *
 *@@added V0.9.18 (2002-02-23) [umoeller]
 */

void _Optlink fntWimpThread(PTHREADINFO pti)
{
    TRY_LOUD(excpt1)
    {
        BOOL dummy = 1;

        DosSetPriority(PRTYS_THREAD,
                       PRTYC_IDLETIME,
                       0,
                       0);      // current thread

        // run forever
        while (dummy)           // avoid compiler warning
        {
            // sleep five minutes
            DosSleep(5 * 60 * 1000);

            #if (__IBMC__ >= 300)
                _heapmin();
                // _heapmin returns all unused memory from the default
                // runtime heap to the operating system;
                // this is VAC++3.0-specific
            #endif

            // reload country settings
            cmnQueryCountrySettings(TRUE);
        }
    }
    CATCH(excpt1)
    {
    } END_CATCH();
}

/* ******************************************************************
 *
 *   Startup
 *
 ********************************************************************/

/*
 *@@ xthrStartThreads:
 *      this gets called by initMain upon
 *      system startup to start the three additional
 *      XWorkplace threads.
 *
 *@@changed V0.9.3 (2000-04-25) [umoeller]: moved all multimedia stuff to media\mmthread.c
 *@@changed V0.9.18 (2002-02-23) [umoeller]: added Wimp thread
 *@@changed V0.9.18 (2002-02-23) [umoeller]: changed a few thread names for fun
 */

BOOL xthrStartThreads(VOID)
{
    BOOL brc = FALSE;

    initLog("Entering " __FUNCTION__":");

    if (thrQueryID(&G_tiWorkerThread) == NULLHANDLE)
    {
        // store the thread ID of the calling thread;
        // this should always be 1
        // ... moved this to initMain

        /*
         *  start threads
         *
         */

        // threads not disabled:
        G_ulWorkerMsgCount = 0;
        thrCreate(&G_tiWorkerThread,
                  fntWorkerThread,
                  NULL, // running flag
                  "Worker",
                  THRF_WAIT,    // no msgq, but wait V0.9.9 (2001-01-31) [umoeller]
                  0);

        initLog("  Started XWP Worker thread, TID: %d",
                          G_tiWorkerThread.tid);

        thrCreate(&G_tiBushThread,
                  fntBushThread,
                  NULL, // running flag
                  "Bush",
                  THRF_WAIT,    // no msgq, but wait V0.9.9 (2001-01-31) [umoeller]
                  0);

        initLog("  Started XWP Bush thread, TID: %d",
                          G_tiBushThread.tid);

        // start Wimp thread V0.9.18 (2002-02-23) [umoeller]
        thrCreate(&G_tiWimpThread,
                  fntWimpThread,
                  NULL, // running flag
                  "Wimp",
                  THRF_WAIT,            // no msgq
                  0);

        initLog("  Started XWP Wimp thread, TID: %d",
                          G_tiWimpThread.tid);

        thrCreate(&G_tiFileThread,
                  fntFileThread,
                  NULL, // running flag
                  "File",
                  THRF_WAIT,    // no msgq, but wait V0.9.9 (2001-01-31) [umoeller]
                  0);

        initLog("  Started XWP File thread, TID: %d",
                          G_tiFileThread.tid);
    }

    return brc;
}


