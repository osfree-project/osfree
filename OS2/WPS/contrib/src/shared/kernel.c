
/*
 *@@sourcefile kernel.c:
 *      this file has code which I considered to be "internal"
 *      enough to be called the "XWorkplace kernel".
 *
 *      With V0.9.0, this code has been moved to shared\ to allow
 *      for extension by other programmers which need certain code
 *      to be running on thread 1 also.
 *
 *      In detail, we have:
 *
 *      -- kernel locks and exception log information;
 *
 *      -- the KERNELGLOBALS interface (krnQueryGlobals);
 *
 *      -- the thread-1 object window (fnwpThread1Object);
 *
 *      In this file, I have assembled code which you might consider
 *      useful for extensions. For example, if you need code to
 *      execute on thread 1 of PMSHELL.EXE (which is required for
 *      some WPS methods to work, unfortunately), you can add a
 *      message to be processed in fnwpThread1Object.
 *
 *      If you need stuff to be executed upon Desktop startup, you can
 *      insert a function into initMain.
 *
 *      All functions in this file have the "krn*" prefix (V0.9.0).
 *
 *      The initialization code has been moved to init.c with
 *      V0.9.16.
 *
 *@@header "shared\kernel.h"
 */

/*
 *      Copyright (C) 1997-2013 Ulrich M”ller.
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
#define INCL_DOSQUEUES
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINTIMER
#define INCL_WINSYS
#define INCL_WINPOINTERS
#define INCL_WINMENUS
#define INCL_WINDIALOGS
#define INCL_WINBUTTONS
#define INCL_WINPROGRAMLIST     // needed for PROGDETAILS, wppgm.h
#define INCL_WINSWITCHLIST
#define INCL_WINSHELLDATA
#define INCL_WINSTDFILE
#include <os2.h>
// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <io.h>                 // access etc.
#include <fcntl.h>
#include <sys\stat.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\tree.h"               // red-black binary trees
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers
#include "helpers\xwpsecty.h"           // XWorkplace Security

// SOM headers which don't crash with prec. header files
// #include "xfobj.ih"
#include "xfldr.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "xwpapi.h"                     // public XWorkplace definitions

#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\init.h"                // XWorkplace initialization
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\notebook.h"            // generic XWorkplace notebook handling

#include "filesys\filedlg.h"            // replacement file dialog implementation
#include "filesys\program.h"            // program implementation; WARNING: this redefines macros
#include "filesys\refresh.h"            // folder auto-refresh
#include "filesys\xthreads.h"           // extra XWorkplace threads

#include "startshut\shutdown.h"         // XWorkplace eXtended Shutdown

// headers in /hook
#include "hook\xwphook.h"

#include "bldlevel.h"

// other SOM headers
#pragma hdrstop

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// global lock semaphore for krnLock etc.
static HMTX             G_hmtxCommonLock = NULLHANDLE;

// "Quick open" dlg status (thread-1 object wnd)
// static ULONG            G_ulQuickOpenNow = 0,
   //                      G_ulQuickOpenMax = 0;
// static HWND             G_hwndQuickStatus = NULLHANDLE;
// static BOOL             G_fQuickOpenCancelled = FALSE;

// flags passed with mp1 of XDM_PAGERCONFIG
static ULONG            G_XPagerConfigFlags = 0;

// global structure with data needed across threads
// (see kernel.h)
KERNELGLOBALS           G_KernelGlobals = {0};

PXWPGLOBALSHARED        G_pXwpGlobalShared = NULL;
PXWPSHELLSHARED         G_pXWPShellShared = NULL;

// classes tree V0.9.16 (2001-09-29) [umoeller]
// see krnClassInitialized
static TREE             *G_ClassNamesTree;

// anchor block of WPS thread 1 (queried in initMain);
// this is exported thru kernel.h and never changed again
extern HAB              G_habThread1 = NULLHANDLE;

extern PID              G_pidWPS = 0;
extern TID              G_tidWorkplaceThread = 0;

// hiwords for abstract and file-system object handles;
// initialized in initMain, exported thru kernel.h
extern USHORT           G_usHiwordAbstract = 0;
extern USHORT           G_usHiwordFileSystem = 0;

// V0.9.11 (2001-04-25) [umoeller]
static HWND             G_hwndXPagerContextMenu = NULLHANDLE;

// resize information for ID_XFD_CONTAINERPAGE, which is used
// by many settings pages
MPARAM G_ampGenericCnrPage[] =
    {
        MPFROM2SHORT(ID_XFDI_CNR_GROUPTITLE, XAC_SIZEX | XAC_SIZEY),
        MPFROM2SHORT(ID_XFDI_CNR_CNR, XAC_SIZEX | XAC_SIZEY)
    };

extern MPARAM *G_pampGenericCnrPage = G_ampGenericCnrPage;
extern ULONG G_cGenericCnrPage = sizeof(G_ampGenericCnrPage) / sizeof(G_ampGenericCnrPage[0]);

// forward declarations
MRESULT EXPENTRY fnwpStartupDlg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY fncbStartup(HWND hwndStatus, ULONG ulObject, MPARAM mpNow, MPARAM mpMax);
MRESULT EXPENTRY fnwpQuickOpenDlg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY fncbQuickOpen(HWND hwndFolder, ULONG ulObject, MPARAM mpNow, MPARAM mpMax);

/* ******************************************************************
 *
 *   Resource protection (thread safety)
 *
 ********************************************************************/

static const char  *G_pcszReqSourceFile = NULL;
static ULONG       G_ulReqLine = 0;
static const char  *G_pcszReqFunction = NULL;

/*
 *@@ krnInit:
 *      called from initMain to initialize kernel globals.
 *      This must make no assumptions about the state of
 *      any other XWorkplace component.
 *
 *@@added V1.0.2 (2003-11-13) [umoeller]
 */

VOID krnInit(VOID)
{
    // first call:
    DosCreateMutexSem(NULL,         // unnamed
                      &G_hmtxCommonLock,
                      0,            // unshared
                      TRUE);       // request now
    treeInit(&G_ClassNamesTree,        // V0.9.16 (2001-09-29) [umoeller]
             NULL);

    // zero KERNELGLOBALS
    memset(&G_KernelGlobals, 0, sizeof(KERNELGLOBALS));

    // get PM system error windows V0.9.3 (2000-04-28) [umoeller]
    winhFindPMErrorWindows(&G_KernelGlobals.hwndHardError,
                           &G_KernelGlobals.hwndSysError);

    krnUnlock();
}

/*
 *@@ krnLock:
 *      function to request the global hmtxCommonLock
 *      semaphore to finally make the kernel functions
 *      thread-safe. While this semaphore is held,
 *      all other threads are kept from accessing
 *      XWP kernel data. Consider this XWP's "big kernel
 *      lock".
 *
 *      Returns TRUE if the semaphore could be accessed
 *      within the specified timeout.
 *
 *      As parameters to this function, pass the caller's
 *      source file, line number, and function name.
 *      This is stored internally so that the xwplog.log
 *      file can report error messages properly if the
 *      mutex is not released.
 *
 *      The string pointers must be static const strings.
 *      Use "__FILE__, __LINE__, __FUNCTION__" always.
 *
 *      Note: Make sure that your code is properly protected
 *      with exception handlers (see helpers\except.c
 *      for remarks about that).
 *
 *      Proper usage:
 *
 +          BOOL fLocked = FALSE;
 +          TRY_LOUD(excpt1)
 +          {
 +              fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__);
 +              if (fLocked)
 +              {
 +                  // ... precious code here
 +              }
 +          }
 +          CATCH(excpt1) { } END_CATCH();
 +
 +          if (fLocked)
 +              krnUnlock();        // NEVER FORGET THIS!!
 *
 *@@added V0.9.0 (99-11-14) [umoeller]
 *@@changed V0.9.3 (2000-04-08) [umoeller]: moved this here from common.c
 *@@changed V0.9.7 (2000-12-13) [umoeller]: changed prototype to trace locks
 *@@changed V0.9.16 (2001-09-29) [umoeller]: added classes tree init
 *@@changed V1.0.2 (2003-11-13) [umoeller]: exported krnInit()
 */

BOOL krnLock(PCSZ pcszSourceFile,        // in: __FILE__
             ULONG ulLine,                      // in: __LINE__
             PCSZ pcszFunction)          // in: __FUNCTION__
{
    if (!DosRequestMutexSem(G_hmtxCommonLock, 10 * 1000))
    {
        // store owner (these are const strings, this is safe)
        G_pcszReqSourceFile = pcszSourceFile;
        G_ulReqLine = ulLine;
        G_pcszReqFunction = pcszFunction;
        return TRUE;
    }

    // request failed within ten seconds:
    cmnLog(__FILE__, __LINE__, __FUNCTION__,
           "krnLock mutex request failed!!\n"
           "    First requestor: %s (%s, line %d))\n"
           "    Second (failed) requestor: %s (%s, line %d))",
           (G_pcszReqFunction) ? G_pcszReqFunction : "NULL",
           (G_pcszReqSourceFile) ? G_pcszReqSourceFile : "NULL",
           G_ulReqLine,
           pcszFunction,
           pcszSourceFile,
           ulLine);

    return FALSE;
}

/*
 *@@ krnUnlock:
 *
 *@@added V0.9.0 (99-11-14) [umoeller]
 *@@changed V0.9.3 (2000-04-08) [umoeller]: moved this here from common.c
 */

VOID krnUnlock(VOID)
{
    DosReleaseMutexSem(G_hmtxCommonLock);
}

/*
 *@@ krnQueryLock:
 *      returns the thread ID which currently owns
 *      the common lock semaphore or 0 if the semaphore
 *      is not owned (not locked).
 *
 *@@added V0.9.1 (2000-01-30) [umoeller]
 *@@changed V0.9.3 (2000-04-08) [umoeller]: moved this here from common.c
 */

ULONG krnQueryLock(VOID)
{
    PID     pid = 0;
    TID     tid = 0;
    ULONG   ulCount = 0;
    if (DosQueryMutexSem(G_hmtxCommonLock,
                         &pid,
                         &tid,
                         &ulCount)
            == NO_ERROR)
        return tid;

    return 0;
}

/********************************************************************
 *
 *   KERNELGLOBALS structure
 *
 ********************************************************************/

/*
 *@@ krnQueryGlobals:
 *
 *@@added V0.9.16 (2001-09-29) [umoeller]
 */

PCKERNELGLOBALS krnQueryGlobals(VOID)
{
    return &G_KernelGlobals;
}

/*
 *@@ krnLockGlobals:
 *      this returns the global KERNELGLOBALS structure
 *      which contains all kinds of data which need to
 *      be accessed across threads. This structure is
 *      a global structure in kernel.c.
 *
 *      This calls krnLock to lock the globals.
 *
 *@@changed V0.9.7 (2000-12-13) [umoeller]: changed prototype to trace locks
 */

PKERNELGLOBALS krnLockGlobals(PCSZ pcszSourceFile,
                              ULONG ulLine,
                              PCSZ pcszFunction)
{
    if (krnLock(pcszSourceFile, ulLine, pcszFunction))
        return &G_KernelGlobals;

    return NULL;
}

/*
 *@@ krnUnlockGlobals:
 *
 */

VOID krnUnlockGlobals(VOID)
{
    krnUnlock();
}

/* ******************************************************************
 *
 *   Class maintanance
 *
 ********************************************************************/

/*
 *@@ krnClassInitialized:
 *      registers the specified class name as
 *      "initialized" with the kernel.
 *
 *      This mechanism replaces the BOOLs in
 *      KERNELGLOBALS which had to be set to
 *      TRUE by each class's wpclsInitData.
 *      Instead, we now maintain a map of class
 *      names.
 *
 *      pcszClassName must be the simple class
 *      name, such as "XFldDataFile".
 *
 *      Returns TRUE only if the class name was
 *      added, that is, if it was not already in
 *      the list.
 *
 *@@added V0.9.16 (2001-09-29) [umoeller]
 *@@changed V1.0.6 (2006-09-24) [pr]: fixed minor memory leak
 */

BOOL krnClassInitialized(PCSZ pcszClassName)
{
    BOOL brc = FALSE;

    initLog("entering krnClassInitialized for %s", pcszClassName);

    if (krnLock(__FILE__, __LINE__, __FUNCTION__))
            // krnLock initializes the tree now
    {
        TREE *pNew;
        if (pNew = NEW(TREE))
        {
            pNew->ulKey = (ULONG)pcszClassName;
            brc = !treeInsert(&G_ClassNamesTree,
                              NULL,
                              pNew,
                              treeCompareStrings);
            // V1.0.6 (2006-09-24) [pr]
            if (!brc)
                FREE(pNew);
        }

        krnUnlock();
    }

    initLog("leaving krnClassInitialized for %s", pcszClassName);

    return brc;
}

/*
 *@@ krnIsClassReady:
 *      returns TRUE if the specified class was
 *      registered with krnClassInitialized, i.e.
 *      if the class is usable on the system.
 *
 *@@added V0.9.16 (2001-09-29) [umoeller]
 */

BOOL krnIsClassReady(PCSZ pcszClassName)
{
    BOOL brc = FALSE;

    if (krnLock(__FILE__, __LINE__, __FUNCTION__))
    {
        brc = (NULL != treeFind(G_ClassNamesTree,
                                (ULONG)pcszClassName,
                                treeCompareStrings));

        krnUnlock();
    }

    return brc;
}

/* ******************************************************************
 *
 *   Exception handlers (\helpers\except.c)
 *
 ********************************************************************/

/*
 *@@ krnExceptOpenLogFile:
 *      this opens or creates C:\XWPTRAP.LOG or C:\EWPTRAP.LOG and
 *      writes a debug header into it (date and time); returns
 *      a FILE* pointer for fprintf(), so additional data can
 *      be written. You should use fclose() to close the file.
 *
 *      This is an "exception hook" which is registered with
 *      the generic exception handlers in src\helpers\except.c.
 *      This code gets called from there whenever an exception
 *      occurs, but only with the "loud" exception handler.
 *
 *@@changed V0.9.0 [umoeller]: moved this stuff here from except.c
 *@@changed V0.9.0 [umoeller]: renamed function
 *@@changed V0.9.2 (2000-03-10) [umoeller]: switched date format to ISO
 */

FILE* _System krnExceptOpenLogFile(VOID)
{
    CHAR        szFilename[CCHMAXPATH];
    FILE        *file = NULL;

    if (    (doshCreateLogFilename(szFilename,
                                   XFOLDER_CRASHLOG,
                                   F_ALLOW_BOOTROOT_LOGFILE))
         && (file = fopen(szFilename, "a"))
       )
    {
        DATETIME    dt;
        DosGetDateTime(&dt);
        fprintf(file, "\n" XWORKPLACE_STRING " trap message -- Date: %04d-%02d-%02d, Time: %02d:%02d:%02d\n",
                dt.year, dt.month, dt.day,
                dt.hours, dt.minutes, dt.seconds);

#define LOGFILENAME         XFOLDER_CRASHLOG

        fprintf(file,
                "-----------------------------------------------------------\n"
                "\nAn internal error occurred in " XWORKPLACE_STRING " (XFLDR.DLL).\n"
                "Please send a bug report to " CONTACT_ADDRESS_USER "\n"
                "so that this error may be fixed for future " XWORKPLACE_STRING " versions.\n"
                "Please supply this file (?:\\" LOGFILENAME ") with your e-mail\n"
                "and describe as exactly as possible the conditions under which\n"
                "the error occurred.\n"
                "\nRunning XFLDR.DLL version: " BLDLEVEL_VERSION " built " __DATE__ "\n");

    }

    return file;
}

/*
 *@@ krnExceptExplainXFolder:
 *      this is the only XFolder-specific information
 *      which is written to the logfile.
 *
 *      This is an "exception hook" which is registered with
 *      the generic exception handlers in src\helpers\except.c.
 *      This code gets called from there whenever an exception
 *      occurs, but only with the "loud" exception handler.
 *
 *@@changed V0.9.0 [umoeller]: moved this stuff here from except.c
 *@@changed V0.9.0 [umoeller]: renamed function
 *@@changed V0.9.1 (99-12-28) [umoeller]: updated written information; added File thread
 *@@changed V0.9.16 (2001-12-02) [pr]: fixed thread priority display
 */

VOID _System krnExceptExplainXFolder(FILE *file,      // in: logfile from fopen()
                                     PTIB ptib,       // in: thread info block
                                     ULONG ulpri)     // in: thread priority
{
    PID         pid;
    TID         tid;
    ULONG       ulCount;
    APIRET      arc;

    ULONG       cThreadInfos = 0;
    PTHREADINFO paThreadInfos = NULL;

    // *** thread info
    if (ptib)
    {
        if (ptib->tib_ptib2)
        {
            THREADINFO ti;

            // find out which thread trapped
            tid = ptib->tib_ptib2->tib2_ultid;

            fprintf(file,
                    "Thread identification:\n    TID 0x%lX (%d) ",
                    tid,        // hex
                    tid);       // dec

            if (thrFindThread(&ti, tid))
                fprintf(file, " (%s)", ti.pcszThreadName);
            else
                if (tid == G_tidWorkplaceThread)  // V0.9.16 (2001-11-02) [pr]: Added thread 1 identification
                    fprintf(file, " (Workplace thread)");
                else
                    fprintf(file, " (unknown thread)");

            /* fprintf(file,
                    "\n    Thread priority: 0x%lX, ordinal: 0x%lX\n",
                    ulpri,
                    ptib->tib_ordinal); */ // moved this to except.c V0.9.19 (2002-03-28) [umoeller]
        }
        else
            fprintf(file, __FUNCTION__ ": ptib->tib_ptib2 is NULL.\n");
    }
    else
        fprintf(file, __FUNCTION__ ": ptib is NULL.\n");

    // running XFolder threads
    fprintf(file, "\nThe following threads could be identified:\n");

    fprintf(file,  "    PMSHELL Workplace thread ID: 0x%lX\n", G_tidWorkplaceThread);

    // V0.9.9 (2001-03-07) [umoeller]
    paThreadInfos = thrListThreads(&cThreadInfos);
    if (paThreadInfos)
    {
        ULONG ul;
        for (ul = 0;
             ul < cThreadInfos;
             ul++)
        {
            PTHREADINFO pThis = &paThreadInfos[ul];
            fprintf(file,
                    "    %s: ID 0x%lX (%d)\n",
                    pThis->pcszThreadName,
                    pThis->tid,
                    pThis->tid);
        }
        free(paThreadInfos);
    }

    if (tid = krnQueryLock())
        fprintf(file, "\nGlobal lock semaphore is currently owned by thread 0x%lX (%u).\n", tid, tid);
    else
        fprintf(file, "\nGlobal lock semaphore is currently not owned.\n", tid, tid);

    /* removed V0.9.20 (2002-07-25) [umoeller]
    arc = xthrQueryAwakeObjectsMutexOwner(&pid,
                                          &tid,
                                          &ulCount);
    if ((arc == NO_ERROR) && (tid))
        fprintf(file, "Awake-objects semaphore is currently owned by thread 0x%lX (%u) (request count: %d).\n",
                      tid, tid, ulCount);
    else
        fprintf(file, "Awake-objects semaphore is currently not owned (request count: %d).\n",
                tid, tid, ulCount);
    */
}

/*
 *@@ krnExceptError:
 *      this is an "exception hook" which is registered with
 *      the generic exception handlers in src\helpers\except.c.
 *      This code gets called whenever a TRY_* macro fails to
 *      install an exception handler.
 *
 *@@added V0.9.2 (2000-03-10) [umoeller]
 */

VOID APIENTRY krnExceptError(PCSZ pcszFile,
                             ULONG ulLine,
                             PCSZ pcszFunction,
                             APIRET arc)     // in: DosSetExceptionHandler error code
{
    cmnLog(pcszFile, ulLine, pcszFunction,
           "TRY_* macro failed to install exception handler (APIRET %d)",
           arc);
}

#ifdef __XWPMEMDEBUG__

/*
 *@@ krnMemoryError:
 *      reports memory error msgs if XWorkplace is
 *      compiled in debug mode _and_ memory debugging
 *      is enabled.
 *
 *@@added V0.9.3 (2000-04-11) [umoeller]
 */

VOID krnMemoryError(PCSZ pcszMsg)
{
    cmnLog(__FILE__, __LINE__, __FUNCTION__,
           "Memory error:\n    %s",
           pcszMsg);
}

#endif

/* ******************************************************************
 *
 *   Startup/Daemon interface
 *
 ********************************************************************/

/*
 *@@ krnEnableReplaceRefresh:
 *      enables or disables "replace auto-refresh folders".
 *      The setting does not take effect until after a
 *      Desktop restart.
 *
 *@@added V0.9.9 (2001-01-31) [umoeller]
 */

VOID krnEnableReplaceRefresh(BOOL fEnable)
{
    PrfWriteProfileData(HINI_USER,
                        (PSZ)INIAPP_XWORKPLACE,
                        (PSZ)INIAPP_REPLACEFOLDERREFRESH,
                        &fEnable,
                        sizeof(fEnable));
}

/*
 *@@ krnReplaceRefreshEnabled:
 *      returns TRUE if "replace folder refresh" has been
 *      enabled.
 *
 *      This setting is not in the global settings because
 *      it has a separate entry in OS2.INI.
 *
 *      Note that this returns the current value of the
 *      setting. If the user has just changed the setting
 *      without restarting the WPS, this does not mean
 *      that "replace refresh" is actually currently working.
 *      Use KERNELGLOBALS.fAutoRefreshReplaced instead.
 *
 *@@added V0.9.9 (2001-01-31) [umoeller]
 *@@changed V1.0.1 (2003-01-25) [umoeller]: rewritten
 */

BOOL krnReplaceRefreshEnabled(VOID)
{
    BOOL        fReplaceFolderRefresh = TRUE;
    ULONG       cb = sizeof(fReplaceFolderRefresh);

    PrfQueryProfileData(HINI_USER,
                        (PSZ)INIAPP_XWORKPLACE,
                        (PSZ)INIAPP_REPLACEFOLDERREFRESH,
                        &fReplaceFolderRefresh,
                        &cb);

    // added this environment variable
    if (getenv("XWP_NO_REPLACE_REFRESH"))
        fReplaceFolderRefresh = FALSE;

    return fReplaceFolderRefresh;
}

/*
 *@@ krnSetProcessStartupFolder:
 *      this gets called during XShutdown to set
 *      the flag in the XWPGLOBALSHARED shared-memory
 *      structure whether the XWorkplace startup
 *      folder should be re-used at the next WPS
 *      startup.
 *
 *      This is only meaningful between Desktop restarts,
 *      because this flag does not get stored anywhere.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V1.0.1 (2003-01-25) [umoeller]: rewritten
 */

VOID krnSetProcessStartupFolder(BOOL fReuse)
{
    BOOL    fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            // cast PVOID
            if (G_pXwpGlobalShared)
                G_pXwpGlobalShared->fProcessStartupFolder = fReuse;
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (fLocked)
        krnUnlock();
}

/*
 *@@ krnNeed2ProcessStartupFolder:
 *      this returns TRUE if the startup folder needs to
 *      be processed. See krnSetProcessStartupFolder.
 *
 *@@changed V0.9.0 [umoeller]: completely rewritten; now using XWPGLOBALSHARED shared memory.
 */

BOOL krnNeed2ProcessStartupFolder(VOID)
{
    BOOL    brc = TRUE;

    BOOL    fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            if (G_pXwpGlobalShared)
                brc = G_pXwpGlobalShared->fProcessStartupFolder;
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (fLocked)
        krnUnlock();

    return brc;
}

/*
 *@@ krnQueryDaemonObject:
 *      returns the window handle of the object window
 *      in XWPDAEMN.EXE or NULLHANDLE if the daemon
 *      is no longer running for some reason.
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

HWND krnQueryDaemonObject(VOID)
{
    HWND    hwnd = NULLHANDLE;

    BOOL    fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            if (!G_pXwpGlobalShared)
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "G_pXwpGlobalShared is NULL.");
            else
                // get the handle of the daemon's object window
                if (!(hwnd = G_pXwpGlobalShared->hwndDaemonObject))
                    cmnLog(__FILE__, __LINE__, __FUNCTION__,
                           "G_pXwpGlobalShared->hwndDaemonObject is NULLHANDLE.");
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (fLocked)
        krnUnlock();

    return hwnd;
}

/*
 *@@ krnStartDaemon:
 *      starts or restarts the XWP daemon.
 *
 *@@added V0.9.19 (2002-03-28) [umoeller]
 */

HAPP krnStartDaemon(VOID)
{
    CHAR    szDir[CCHMAXPATH],
            szExe[CCHMAXPATH];
    HAPP    happDaemon = NULLHANDLE;

    // we need to specify XWorkplace's "BIN"
    // subdir as the working dir, because otherwise
    // the daemon won't find XWPHOOK.DLL.

    // compose paths
    if (cmnQueryXWPBasePath(szDir))
    {
        // path found:
        PROGDETAILS pd;
        // working dir: append bin
        strcat(szDir, "\\bin");
        // exe: append bin\xwpdaemon.exe
        sprintf(szExe,
                "%s\\xwpdaemn.exe",
                szDir);
        memset(&pd, 0, sizeof(pd));
        pd.Length = sizeof(PROGDETAILS);
        pd.progt.progc = PROG_PM;
        pd.progt.fbVisible = SHE_VISIBLE;
        pd.pszTitle = "XWorkplace Daemon";
        pd.pszExecutable = szExe;
        pd.pszParameters = "-D";
        pd.pszStartupDir = szDir;
        pd.pszEnvironment = "WORKPLACE\0\0";

        happDaemon = WinStartApp(G_KernelGlobals.hwndThread1Object, // hwndNotify,
                                 &pd,
                                 "-D", // params; otherwise the daemon
                                       // displays a msg box
                                 NULL,
                                 0);// no SAF_INSTALLEDCMDLINE,
        initLog("  WinStartApp for \"%s\" returned HAPP 0x%lX",
                          pd.pszExecutable,
                          happDaemon);
    }

    return happDaemon;
}

/*
 *@@ krnPostDaemonMsg:
 *      this posts a message to the XWorkplace
 *      Daemon (XWPDAEMN.EXE). Returns TRUE if
 *      successful.
 *
 *      If FALSE is returned, the daemon is probably
 *      not running.
 *
 *@@added V0.9.0 [umoeller]
 */

BOOL krnPostDaemonMsg(ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HWND hwnd;
    if (hwnd = krnQueryDaemonObject())
        return WinPostMsg(hwnd, msg, mp1, mp2);

    return FALSE;
}

/*
 *@@ krnSendDaemonMsg:
 *      this sends a message to the XWorkplace
 *      Daemon (XWPDAEMN.EXE). If the daemon
 *      is not running, returns NULL.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

MRESULT krnSendDaemonMsg(ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HWND hwnd;
    if (hwnd = krnQueryDaemonObject())
        return WinSendMsg(hwnd, msg, mp1, mp2);

    return 0;
}

/*
 *@@ krnMultiUser:
 *      returns TRUE only if the Workplace Shell is
 *      running under control of XWPSHELL.EXE.
 *
 *@@added V1.0.1 (2003-01-25) [umoeller]
 */

BOOL krnMultiUser(VOID)
{
    return (G_pXWPShellShared != NULL);
}

/* ******************************************************************
 *
 *   Thread-1 object window
 *
 ********************************************************************/

static BOOL     G_fLimitMsgOpen = FALSE;
static HWND     G_hwndArchiveStatus = NULLHANDLE;
static PFNWP    G_pfnwpObjectStatic = NULL;

STATIC MRESULT EXPENTRY fnwpThread1Object(HWND hwndObject, ULONG msg, MPARAM mp1, MPARAM mp2);
STATIC MRESULT EXPENTRY fnwpAPIObject(HWND hwndObject, ULONG msg, MPARAM mp1, MPARAM mp2);

/*
 *@@ krnCreateObjectWindows:
 *      called from initMain to create the thread-1 and API
 *      object windows.
 *
 *@@added V0.9.18 (2002-03-27) [umoeller]
 */

VOID krnCreateObjectWindows(VOID)
{
    if (G_KernelGlobals.hwndThread1Object = WinCreateWindow(
                                                HWND_OBJECT,
                                                WC_STATIC,
                                                "",
                                                0,          // not visible
                                                0, 0, 10, 10,
                                                NULLHANDLE, // owner
                                                HWND_BOTTOM,
                                                0,
                                                NULL,
                                                NULL))
    {
        G_pfnwpObjectStatic = WinSubclassWindow(G_KernelGlobals.hwndThread1Object,
                                          fnwpThread1Object);

        // store HAB of WPS thread 1 V0.9.9 (2001-04-04) [umoeller]
        G_habThread1 = WinQueryAnchorBlock(G_KernelGlobals.hwndThread1Object);

        initLog("XWorkplace thread-1 object window created, HWND 0x%lX",
                G_KernelGlobals.hwndThread1Object);
    }

    if (G_KernelGlobals.hwndAPIObject = WinCreateWindow(
                                                HWND_OBJECT,
                                                WC_STATIC,
                                                "",
                                                0,          // not visible
                                                0, 0, 10, 10,
                                                NULLHANDLE, // owner
                                                HWND_BOTTOM,
                                                0,
                                                NULL,
                                                NULL))
    {
        G_pfnwpObjectStatic = WinSubclassWindow(G_KernelGlobals.hwndAPIObject,
                                          fnwpAPIObject);

        initLog("XWorkplace API object window created, HWND 0x%lX",
                G_KernelGlobals.hwndAPIObject);
    }
}

/*
 *@@ T1M_DaemonReady:
 *      implementation for T1M_DAEMONREADY.
 *
 *@@added V0.9.3 (2000-04-24) [umoeller]
 *@@changed V1.0.1 (2002-12-08) [umoeller]: added daemon NLS init here @@fixes bug 64
 *@@changed V1.0.4 (2005-10-17) [bvl]: Load NLS DLL before loading deamon strings @@fixes 389
 */

STATIC VOID T1M_DaemonReady(VOID)
{
    // _Pmpf(("T1M_DaemonReady"));
    // PXWPGLOBALSHARED pXwpGlobalShared;

    HWND    hwndDaemonObject;

    if (hwndDaemonObject = krnQueryDaemonObject())
    {
#ifndef __ALWAYSHOOK__
        if (cmnQuerySetting(sfXWPHook))
#endif
        {
            if (WinSendMsg(hwndDaemonObject,
                           XDM_HOOKINSTALL,
                           (MPARAM)TRUE,
                           0))
            {
                // success:
                // notify daemon of Desktop window;
                // this is still NULLHANDLE if we're
                // currently starting the WPS
                HWND hwndActiveDesktop = cmnQueryActiveDesktopHWND();
                // _Pmpf(("  Posting XDM_DESKTOPREADY (0x%lX)",
                //         hwndActiveDesktop));
                krnPostDaemonMsg(XDM_DESKTOPREADY,
                                 (MPARAM)hwndActiveDesktop,
                                 (MPARAM)0);

                // _Pmpf(("    cmnQuerySetting(sfXPagerEnabled:) %d",
                //        cmnQuerySetting(sfEnableXPager)));

#ifndef __NOPAGER__
                if (cmnQuerySetting(sfEnableXPager))
                    // XPager is enabled too:
                    WinSendMsg(hwndDaemonObject,
                               XDM_STARTSTOPPAGER,
                               (MPARAM)TRUE,
                               0);
#endif
            }
        }

        // added this call here V1.0.1 (2002-12-08) [umoeller]
        // (bug 64)
        // we need to be sure the NLS DLL is loaded V1.0.4 (2005-10-17) [bvl] @@fixes 389
        cmnQueryNLSModuleHandle(FALSE);
        cmnLoadDaemonNLSStrings();
    }
}

/*
 *@@ T1M_OpenObjectFromHandle:
 *      implementation for T1M_OPENOBJECTFROMHANDLE in
 *      fnwpThread1Object.
 *
 *      Parameters:
 *
 *      -- HOBJECT mp1: object handle to open.
 *              The following "special objects" are
 *              handled here:
 *              (definitions are in include\xwphook.h):
 *
 +                SPECIALOBJ_SHOWWINDOWLIST         0xFFFF0000
 +                SPECIALOBJ_DESKTOPCONTEXTMENU     0xFFFF0001
 *
 *         Note that there are more definitions, but those
 *         are handled in the daemon and not passed here
 *         because they affect the pager and other hook-specific
 *         things.
 *
 *      -- ULONG mp2: corner reached;
 *                  1 = lower left,
 *                  2 = top left,
 *                  3 = lower right,
 *                  4 = top right;
 *                  0 = object hotkey. We then play the respective
 *                      system sound.
 *                  -1 = other action. Perform no special processing.
 *
 *      The way this works is the following:
 *
 *      1)  If the object is "special", the respective action
 *          is performed.
 *
 *      2)  If it is not, we try to get the object from the
 *          handle. If that works (handle isn't invalid),
 *          we first play the "hotkey" system sound. We then
 *          invoke XFldObject::xwpHotkeyOrBorderAction.
 *
 *      Note that the "corner" value (mp2) is evaluated by
 *      XFldObject::xwpHotkeyOrBorderAction and its overrides,
 *      so there might be class-specific behavior attached to
 *      that. For example, XCenter::xwpHotkeyOrBorderAction
 *      will open the X-Button menu for the "0" value.
 *
 *@@added V0.9.3 (2000-04-20) [umoeller]
 *@@changed V0.9.3 (2000-04-20) [umoeller]: added system sound
 *@@changed V0.9.4 (2000-06-12) [umoeller]: fixed desktop menu position
 *@@changed V0.9.4 (2000-06-15) [umoeller]: fixed VIO windows in background
 *@@changed V0.9.7 (2000-11-29) [umoeller]: fixed memory leak
 *@@changed V0.9.13 (2001-06-23) [umoeller]: now using winhQuerySwitchList
 *@@changed V0.9.16 (2001-11-22) [umoeller]: now disallowing object open during startup and shutdown
 *@@changed V0.9.19 (2002-04-24) [umoeller]: fixed major screwups during startup and shutdown
 *@@changed V1.0.5 (2005-11-26) [pr]: changed to use method directly, and modify xwpHotkeyOrBorderAction prototype
 *@@changed V1.0.6 (2006-08-18) [pr]: put method name resolution back @@fixes 792
 *@@changed V1.0.6 (2006-10-24) [pr]: remove redundant parameter
 */

STATIC VOID T1M_OpenObjectFromHandle(MPARAM mp1,
                                     MPARAM mp2)
{
    TRY_LOUD(excpt1)
    {
        HOBJECT hobjStart = (HOBJECT)mp1;

        // make sure the desktop is already fully populated
        // V0.9.16 (2001-10-25) [umoeller]
        // if (!G_KernelGlobals.fDesktopPopulated)
           //  return;      // ouch, we have an exception handler V0.9.19 (2002-04-24) [umoeller]

        // make sure we're not shutting down,
        // but allow object hotkeys while confirmation
        // dialogs are open
        // V0.9.16 (2001-11-22) [umoeller]
        /* if (xsdIsShutdownRunning() > 1)
            return;     // ouch we have an exception handler here
                        // V0.9.19 (2002-04-24) [umoeller]
        */

        if (!G_KernelGlobals.fDesktopPopulated)
            hobjStart = NULLHANDLE;
        // now, allow objects while we're still shuttting windows
        // down, but not after all windows have been closed
        // V0.9.19 (2002-04-24) [umoeller]
        else switch (xsdQueryShutdownState())
        {
            // case XSD_IDLE: // this is OK
            case XSD_CONFIRMING: // this is not
            case XSD_INITIALIZING: // this is not
            // case XSD_CLOSINGWINDOWS: // this is OK
            // case XSD_CANCELLED: // this is OK
            case XSD_ALLCLOSED_SAVING: // this is not
            case XSD_SAVEDONE_FLUSHING: // this is not
                hobjStart = NULLHANDLE;
        }

        if (hobjStart)
        {
            if ((ULONG)hobjStart >= SPECIALOBJ_FIRST)
            {
                // special objects:
                switch ((ULONG)hobjStart)
                {
                    case SPECIALOBJ_SHOWWINDOWLIST: // 0xFFFF0000
                        // show window list
                        WinPostMsg(cmnQueryActiveDesktopHWND(),
                                   WM_COMMAND,
                                   (MPARAM)0x8011,
                                   MPFROM2SHORT(CMDSRC_MENU,
                                                TRUE));
                    break;

                    case SPECIALOBJ_DESKTOPCONTEXTMENU: // 0xFFFF0001
                    {
                        // show Desktop's context menu V0.9.1 (99-12-19) [umoeller]
                        WPObject* pActiveDesktop;
                        HWND hwndFrame;
                        if (    (pActiveDesktop = cmnQueryActiveDesktop())
                             && (hwndFrame = cmnQueryActiveDesktopHWND())
                           )
                        {
                            HWND hwndClient = WinWindowFromID(hwndFrame, FID_CLIENT);
                            POINTL ptlPopup = { 0, 0 }; // default: lower left
                            WinQueryPointerPos(HWND_DESKTOP, &ptlPopup);
                            /* switch ((ULONG)mp2)
                            {
                                // corner reached:
                                case 2: // top left
                                    ptlPopup.x = 0;
                                    ptlPopup.y = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
                                break;

                                case 3: // lower right
                                    ptlPopup.x = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
                                    ptlPopup.y = 0;
                                break;

                                case 4: // top right
                                    ptlPopup.x = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
                                    ptlPopup.y = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
                                break;
                            } */
                            _wpDisplayMenu(pActiveDesktop,
                                           hwndFrame,       // owner
                                           hwndClient,
                                           &ptlPopup,
                                           MENU_OPENVIEWPOPUP,
                                           0);      // reserved
                        }
                    }
                    break;
                }
            } // end if ((ULONG)hobjStart >= SPECIALOBJ_FIRST)
            else
            {
                // normal object handle:
                WPObject *pobjStart = _wpclsQueryObject(_WPObject,
                                                        hobjStart);

                PMPF_KEYS(("received hobj 0x%lX -> 0x%lX",
                            hobjStart,
                            pobjStart));

                if (pobjStart)
                {
                    somTD_XFldObject_xwpHotkeyOrBorderAction pfn_xwpHotkeyOrBorderAction;

                    // obtain "xwpHotkeyOrBorderAction" method pointer
                    if (pfn_xwpHotkeyOrBorderAction = (somTD_XFldObject_xwpHotkeyOrBorderAction)somResolveByName(
                                                            pobjStart,
                                                            "xwpHotkeyOrBorderAction"))
                    {
                        // method resolved:
#ifndef __NOXSYSTEMSOUNDS__
                        if ((ULONG)mp2 == 0)
                            // object hotkey, not screen corner:
                            cmnPlaySystemSound(MMSOUND_XFLD_HOTKEYPRSD);
                                        // V0.9.3 (2000-04-20) [umoeller]
#endif
                        pfn_xwpHotkeyOrBorderAction(pobjStart,
                                                    OPEN_DEFAULT,      // changed V1.0.2 (2003-03-07) [umoeller]
                                                    (ULONG)mp2);
                    }
                }
            }
        }
    }
    CATCH(excpt1) {} END_CATCH();
}

/*
 *@@ T1M_OpenObjectFromHandle2:
 *      implementation for T1M_OPENOBJECTFROMHANDLE2 in
 *      fnwpThread1Object.
 *
 *      Required for the #398 bugfix from w_monitors.c.
 *
 *@@added V1.0.2 (2003-03-07) [umoeller]
 *@@changed V1.0.6 (2006-08-18) [pr]: use method name resolution @@fixes 792
 *@@changed V1.0.6 (2006-10-24) [pr]: remove redundant parameter
 */

STATIC VOID T1M_OpenObjectFromHandle2(HOBJECT hobj,
                                      ULONG ulView)
{
    WPObject *pobjStart = _wpclsQueryObject(_WPObject,
                                            hobj);

    PMPF_KEYS(("received hobj 0x%lX -> 0x%lX",
                hobj,
                pobjStart));

    if (pobjStart)
    {
        somTD_XFldObject_xwpHotkeyOrBorderAction pfn_xwpHotkeyOrBorderAction;

        // obtain "xwpHotkeyOrBorderAction" method pointer
        if (pfn_xwpHotkeyOrBorderAction = (somTD_XFldObject_xwpHotkeyOrBorderAction)
                somResolveByName(pobjStart,
                                 "xwpHotkeyOrBorderAction"))
        {
            pfn_xwpHotkeyOrBorderAction(pobjStart,
                                        ulView,
                                        -1);
        }
    }
}

/*
 *@@ fnwpThread1Object:
 *      wnd proc for the thread-1 object window.
 *
 *      This is needed for processing messages which must be
 *      processed on thread 1. We cannot however process these
 *      messages in the subclassed folder frame wnd proc
 *      (fnwpSubclWPFolderWindow in folder.c),
 *      because adding user messages to that wnd proc could
 *      conflict with default WPFolder messages or those of
 *      some other WPS enhancer, and we can also never be
 *      sure whether the folder window proc is really running
 *      on thread 1. Sometimes it isn't. ;-)
 *
 *      We therefore create another object window, which we
 *      own all alone.
 *
 *      Even though the PM docs say that an object window should never
 *      be created on thread 1 (which we're doing here), because this
 *      would "slow down the system", this is not generally true.
 *      WRT performance, it doesn't matter if an object window or a
 *      frame window processes messages. And since we _need_ to process
 *      some messages on the Workplace thread (1), especially when
 *      manipulating WPS windows, we do it here.
 *
 *      But of course, since this is on thread 1, we must get out of
 *      here quickly (0.1 seconds rule), because while we're processing
 *      something in here, the WPS user interface is blocked.
 *
 *      Note: Another view-specific object window is created
 *      for every folder view that is opened, because sometimes
 *      folder views do _not_ run on thread 1 and manipulating
 *      frame controls from thread 1 would then hang the PM.
 *      See fdr_fnwpSupplFolderObject in filesys\folder.c for details.
 *
 *@@changed V0.9.0 [umoeller]: T1M_QUERYXFOLDERVERSION message handling
 *@@changed V0.9.0 [umoeller]: T1M_OPENOBJECTFROMHANDLE added
 *@@changed V0.9.1 (99-12-19) [umoeller]: added "show Desktop menu" to T1M_OPENOBJECTFROMHANDLE
 *@@changed V0.9.2 (2000-02-23) [umoeller]: added T1M_PAGERCLOSED
 *@@changed V0.9.3 (2000-04-09) [umoeller]: added T1M_PAGERCONFIGDELAYED
 *@@changed V0.9.3 (2000-04-09) [umoeller]: fixed timer problem, which was never stopped... this solves the "disappearing windows" problem!!
 *@@changed V0.9.3 (2000-04-25) [umoeller]: startup folder was permanently disabled when panic flag was set; fixed
 *@@changed V0.9.4 (2000-06-05) [umoeller]: added exception handling
 *@@changed V0.9.6 (2000-10-16) [umoeller]: added WM_APPTERMINATENOTIFY
 *@@changed V0.9.14 (2001-08-07) [pr]: added T1M_OPENRUNDIALOG
 *@@changed V0.9.20 (2002-08-04) [umoeller]: "properties" in pager context menu opens pager page directly now
 */

STATIC MRESULT EXPENTRY fnwpThread1Object(HWND hwndObject, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MPARAM  mrc = NULL;

    switch(msg)
    {
        /*
         * WM_TIMER:
         *
         */

        case WM_TIMER:
            switch ((USHORT)mp1)    // timer ID
            {
                case 1:
                    // archive status timer
                    winhDestroyWindow(&G_hwndArchiveStatus);
                break;

#ifndef __NOPAGER__
                case 2: // started from T1M_PAGERCONFIGDELAYED
                {
                    HWND    hwndDaemonObject;
                    if (hwndDaemonObject = krnQueryDaemonObject())
                    {
                        // cross-process send msg: this
                        // does not return until the daemon
                        // has re-read the data
                        WinSendMsg(hwndDaemonObject,
                                   XDM_PAGERCONFIG,
                                   (MPARAM)G_XPagerConfigFlags,
                                   0);

                        // reset flags
                        G_XPagerConfigFlags = 0;
                    }
                }
                break;
#endif
            }

            // stop timer; this was missing!! V0.9.3 (2000-04-09) [umoeller]
            WinStopTimer(WinQueryAnchorBlock(hwndObject),
                         hwndObject,
                         (USHORT)mp1);      // timer ID
        break;

        /*
         * WM_APPTERMINATENOTIFY:
         *      this gets posted from PM since we use
         *      this object window as the notify window
         *      to WinStartApp when we start program objects
         *      (progOpenProgram, filesys/program.c).
         *
         *      We must then remove source emphasis for
         *      the corresponding object.
         */

        case WM_APPTERMINATENOTIFY:
            progAppTerminateNotify((HAPP)mp1);
        break;

        /*
         *@@ T1M_LIMITREACHED:
         *      this is posted by cmnAppendMi2List when too
         *      many menu items are in use, i.e. the user has
         *      opened a zillion folder content menus; we
         *      will display a warning dlg, which will also
         *      destroy the open menu.
         */

        case T1M_LIMITREACHED:
            if (!G_fLimitMsgOpen)
            {
                // avoid more than one dlg window
                G_fLimitMsgOpen = TRUE;
                cmnSetDlgHelpPanel(ID_XFH_LIMITREACHED);
                WinDlgBox(HWND_DESKTOP,         // parent is desktop
                          HWND_DESKTOP,             // owner is desktop
                          (PFNWP)cmn_fnwpDlgWithHelp,    // dialog procedure, defd. at bottom
                          cmnQueryNLSModuleHandle(FALSE),  // from resource file
                          ID_XFD_LIMITREACHED,        // dialog resource id
                          (PVOID)NULL);             // no dialog parameters
                G_fLimitMsgOpen = FALSE;
            }
        break;

        /*
         *@@ T1M_EXCEPTIONCAUGHT:
         *      this is posted from the various XFolder threads
         *      when something trapped; it is assumed that
         *      mp1 is a PSZ to an error msg allocated with
         *      malloc(), and after displaying the error,
         *      (PSZ)mp1 is freed here. If mp2 != NULL, the WPS will
         *      be restarted (this is demanded by XSHutdown traps).
         *
         *@@changed V0.9.4 (2000-08-03) [umoeller]: fixed heap bug
         */

        case T1M_EXCEPTIONCAUGHT:
            if (mp1)
            {
                XSTRING strMsg;
                xstrInitSet(&strMsg, (PSZ)mp1);
                if (mp2)
                {
                    // restart Desktop: Yes/No box
                    if (WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                                      strMsg.psz,
                                      (PSZ)"XFolder: Exception caught",
                                      0,
                                      MB_YESNO | MB_ICONEXCLAMATION | MB_MOVEABLE)
                             == MBID_YES)
                        // if yes: terminate the current process,
                        // which is PMSHELL.EXE. We cannot use DosExit()
                        // directly, because this might mess up the
                        // C runtime library.
                        exit(99);
                }
                else
                {
                    // just report:
                    xstrcat(&strMsg,
                            "\n\nPlease post a bug report to "
                            CONTACT_ADDRESS_USER " and attach the file "
                            LOGFILENAME ", which you will find in the root "
                            "directory of your boot drive. ", 0);
                    winhDebugBox(HWND_DESKTOP, "XFolder: Exception caught", strMsg.psz);
                }

                xstrClear(&strMsg);
            }
        break;

        /*
         *@@ T1M_QUERYXFOLDERVERSION:
         *      this msg may be send to the XFolder object
         *      window from external programs to query the
         *      XFolder version number which is currently
         *      installed. We will return:
         *          mrc = MPFROM2SHORT(major, minor)
         *      which may be broken down by the external
         *      program using the SHORT1/2FROMMP macros.
         *      This is used by the XShutdown command-line
         *      interface (XSHUTDWN.EXE) to assert that
         *      XFolder is up and running, but can be used
         *      by other software too.
         */

        case T1M_QUERYXFOLDERVERSION:
        {
            ULONG   ulMajor = 0,
                    ulMinor = 0;
            sscanf(XFOLDER_VERSION, // e.g. 0.9.2, this is defined in dlgids.h
                    "%d.%d", &ulMajor, &ulMinor);   // V0.9.0

            mrc = MPFROM2SHORT(ulMajor, ulMinor);
        }
        break;

        /*
         * T1M_EXTERNALSHUTDOWN:
         *      this msg may be posted to the XFolder object
         *      window from external programs to initiate
         *      the eXtended shutdown. mp1 is assumed to
         *      point to a block of shared memory containing
         *      a SHUTDOWNPARAMS structure.
         * removed V0.9.18 (2002-03-27) [umoeller]
         */

        /* case T1M_EXTERNALSHUTDOWN:
        {
            PSHUTDOWNPARAMS psdpShared = (PSHUTDOWNPARAMS)mp1;

            if ((ULONG)mp2 != 1234)
            {
                // not special code:
                // that's the send-msg from XSHUTDWN.EXE;
                // copy the memory to local memory and
                // return, otherwise XSHUTDWN.EXE hangs
                APIRET arc = DosGetSharedMem(psdpShared, PAG_READ | PAG_WRITE);
                if (arc == NO_ERROR)
                {
                    // shared memory successfully accessed:
                    // the block has now two references (XSHUTDWN.EXE and
                    // PMSHELL.EXE);
                    // repost msg with that ptr to ourselves
                    WinPostMsg(hwndObject, T1M_EXTERNALSHUTDOWN, mp1, (MPARAM)1234);
                    // return TRUE to XSHUTDWN.EXE, which will then terminate
                    mrc = (MPARAM)TRUE;
                    // after XSHUTDWN.EXE terminates, the shared mem
                    // is not freed yet, because we still own it;
                    // we process that in the second msg (below)
                }
                else
                {
                    winhDebugBox(0,
                             "External XShutdown call",
                             "Error calling DosGetSharedMem.");
                    mrc = (MPARAM)FALSE;
                }
            }
            else
            {
                // mp2 == 1234: second call
                xsdInitiateShutdownExt(psdpShared);
                // finally free shared mem
                DosFreeMem(psdpShared);
            }
        }
        break; */

        /*
         *@@ T1M_DESTROYARCHIVESTATUS:
         *      this gets posted from arcCheckIfBackupNeeded,
         *      which gets called from initMain
         *      with the handle of this object wnd and this message ID.
         */

        case T1M_DESTROYARCHIVESTATUS:
            G_hwndArchiveStatus = (HWND)mp1;
            WinStartTimer(WinQueryAnchorBlock(hwndObject),
                          hwndObject,
                          1,
                          10);
        break;

        /*
         *@@ T1M_OPENOBJECTFROMHANDLE:
         *      this can be posted to the thread-1 object
         *      window from anywhere to have an object
         *      opened in its default view. As opposed to
         *      WinOpenObject, which opens the object on
         *      thread 13 (on my system), the thread-1
         *      object window will always open the object
         *      on thread 1, which leads to less problems.
         *
         *      See T1M_OpenObjectFromHandle for the
         *      parameters.
         *
         *      Most notably, this is posted from the daemon
         *      to open "screen border objects" and global
         *      hotkey objects.
         */

        case T1M_OPENOBJECTFROMHANDLE:
            T1M_OpenObjectFromHandle(mp1, mp2);
        break;

        /*
         *@@ T1M_OPENOBJECTFROMHANDLE2:
         *      like T1M_OPENOBJECTFROMHANDLE, but instead
         *      this has the following params:
         *
         *      --  HOBJECT mp1
         *
         *      --  ULONG mp2: OPEN_* flag (view to open).
         *
         *@@added V1.0.2 (2003-03-07) [umoeller]
         */

        case T1M_OPENOBJECTFROMHANDLE2:
            T1M_OpenObjectFromHandle2((HOBJECT)mp1, (ULONG)mp2);
        break;

        /*
         *@@ T1M_OPENOBJECTFROMPTR:
         *      this can be posted or sent to the thread-1
         *      object window from anywhere to have an object
         *      opened in a specific view and have the HWND
         *      returned (on send only, of course).
         *
         *      This is useful if you must make sure that
         *      an object view is running on thread 1 and
         *      nowhere else. Used with the XCenter settings
         *      view, for example.
         *
         *      Parameters:
         *
         *      -- WPObject *mp1: SOM object pointer on which
         *         wpViewObject is to be invoked.
         *
         *      -- ULONG mp2: ulView to open (OPEN_DEFAULT,
         *         OPEN_SETTINGS, ...)
         *
         *      wpViewObject will be invoked with hwndCnr
         *      and param == NULL.
         *
         *      Returns the return value of wpViewObject,
         *      which is either a HWND or a HAPP.
         *
         *@@added V0.9.9 (2001-02-06) [umoeller]
         */

        case T1M_OPENOBJECTFROMPTR:
            mrc = (MPARAM)_wpViewObject((WPObject*)mp1,
                                        NULLHANDLE,     // hwndCnr
                                        (ULONG)mp2,
                                        0);             // param
        break;

        /*
         *@@ T1M_MENUITEMSELECTED:
         *      calls wpMenuItemSelected on thread 1.
         *      This can be posted or sent, we don't care.
         *      This is used from the XCenter to make sure
         *      views won't get opened on the XCenter thread.
         *
         *      Parameters:
         *
         *      --  WPObject* mp1: object on which to
         *          invoke wpMenuItemSelected.
         *
         *      --  ULONG mp2: menu item ID to pass to
         *          wpMenuItemSelected.
         *
         *      Returns the BOOL rc of wpMenuItemSelected.
         *
         *@@added V0.9.11 (2001-04-18) [umoeller]
         */

        case T1M_MENUITEMSELECTED:
            mrc = (MPARAM)_wpMenuItemSelected((WPObject*)mp1,
                                              NULLHANDLE,       // hwndFrame
                                              (ULONG)mp2);
        break;

        /*
         *@@ T1M_DAEMONREADY:
         *      posted by the XWorkplace daemon after it has
         *      successfully created its object window.
         *      This can happen in two situations:
         *
         *      -- during Desktop startup, after initMain
         *         has started the daemon;
         *      -- any time later, if the daemon has been restarted
         *         (shouldn't happen).
         *
         *      The thread-1 object window will then send XDM_HOOKINSTALL
         *      back to the daemon if the global settings have the
         *      hook enabled.
         */

        case T1M_DAEMONREADY:
            T1M_DaemonReady();
        break;

#ifndef __NOPAGER__
        /*
         *@@ T1M_PAGERCLOSED:
         *      this gets posted by dmnKillXPager when
         *      the user has closed the XPager window.
         *      We then disable XPager in the global settings.
         *
         *      Parameters:
         *      -- BOOL mp1: if TRUE, XPager will be disabled
         *                   in the global settings.
         *
         *@@added V0.9.2 (2000-02-23) [umoeller]
         *@@changed V0.9.3 (2000-04-25) [umoeller]: added mp1 parameter
         */

        case T1M_PAGERCLOSED:
            if (mp1)
                cmnSetSetting(sfEnableXPager, FALSE);

            // update "Features" page, if open
            ntbUpdateVisiblePage(NULL, SP_SETUP_FEATURES);
        break;

        /*
         *@@ T1M_PAGERCONFIGDELAYED:
         *      posted by XWPScreen when any XPager configuration
         *      has changed. We delay sending XDM_PAGERCONFIG to
         *      the daemon for a little while in order not to overload
         *      the system, because XPager needs to reconfigure itself
         *      every time.
         *
         *      Parameters:
         *      -- ULONG mp1: same flags as with XDM_PAGERCONFIG
         *              mp1.
         *
         *@@added V0.9.3 (2000-04-09) [umoeller]
         */

        case T1M_PAGERCONFIGDELAYED:
            // add flags to global variable which will be
            // passed (and reset) when timer elapses
            G_XPagerConfigFlags |= (ULONG)mp1;
            // start timer 2
            WinStartTimer(WinQueryAnchorBlock(hwndObject),
                          hwndObject,
                          2,
                          500);     // half a second delay
        break;
#endif

#ifndef __XWPLITE__
        /*
         *@@ T1M_WELCOME:
         *      posted if XWorkplace has just been installed.
         *
         *      This post comes from the Startup thread after
         *      all other startup processing (startup folders,
         *      quick open, etc.) has completed, but only if
         *      the "just installed" flag was set in OS2.INI
         *      (which has then been removed).
         *
         *      Starting with V0.9.9, we now allow the user
         *      to create the XWorkplace standard objects
         *      here. We no longer do this from WarpIn because
         *      we also defer class registration into the OS2.INI
         *      file to avoid the frequent error messages that
         *      WarpIN produces otherwise.
         *
         *@@added V0.9.7 (2001-01-07) [umoeller]
         *@@changed V0.9.9 (2001-03-27) [umoeller]: added obj creation here
         */

        case T1M_WELCOME:
            if (cmnMessageBoxExt(NULLHANDLE,
                                 121,
                                 NULL, 0,
                                 211,       // create objects?
                                 MB_OKCANCEL)
                    == MBID_OK)
            {
                // produce objects NOW
                xthrPostFileMsg(FIM_RECREATECONFIGFOLDER,
                                (MPARAM)RCF_MAININSTALLFOLDER,
                                0);
            }
        break;
#endif

        /*
         *@@ T1M_PAGERCTXTMENU:
         *      gets posted from XPager if the user
         *      right-clicked onto an empty space in the pager
         *      window (and not on a mini-window).
         *
         *      We should then display the XPager context
         *      menu here because
         *
         *      1)  XPager cannot handle the commands in
         *          the first place (such as open settings)
         *
         *      2)  we don't want NLS stuff in the daemon.
         *
         *      Parameters:
         *
         *      SHORT1FROMMP(mp1): desktop x coordinate of
         *                         mouse click.
         *      SHORT2FROMMP(mp1): desktop y coordinate of
         *                         mouse click.
         *
         *@@added V0.9.11 (2001-04-25) [umoeller]
         */

        case T1M_PAGERCTXTMENU:
            if (!G_hwndXPagerContextMenu)
                G_hwndXPagerContextMenu = WinLoadMenu(hwndObject,
                                                      cmnQueryNLSModuleHandle(FALSE),
                                                      ID_XSM_PAGERCTXTMENU);

            WinPopupMenu(HWND_DESKTOP,      // parent
                         hwndObject,        // owner
                         G_hwndXPagerContextMenu,
                         SHORT1FROMMP(mp1),
                         SHORT2FROMMP(mp1),
                         0,
                         PU_HCONSTRAIN | PU_VCONSTRAIN | PU_MOUSEBUTTON1
                            | PU_MOUSEBUTTON2 | PU_KEYBOARD);
                                // WM_COMMAND is handled below
        break;

        /*
         * WM_COMMAND:
         *      handle commands from the XPager context menu
         *      here (thread-1 object window was specified as
         *      menu's owner above).
         * added V0.9.11 (2001-04-25) [umoeller]
         */

        case WM_COMMAND:
            switch ((USHORT)mp1)
            {
                case ID_CRMI_PROPERTIES:
                    // changed this to open the proper page directly
                    // V0.9.20 (2002-08-04) [umoeller]
                    ntbOpenSettingsPage(XFOLDER_SCREENID,
                                        SP_PAGER_MAIN);
                break;

                case ID_CRMI_HELP:
                    cmnDisplayHelp(NULL,        // active desktop
                                   ID_XSH_PAGER_INTRO);
                break;
            }
        break;

        /*
         *@@ T1M_INITIATEXSHUTDOWN:
         *      posted from the XCenter X-button widget
         *      to have XShutdown initiated with the
         *      current settings.
         *
         *      (ULONG)mp1 must be one of the following:
         *      --  ID_CRMI_LOGOFF: logoff.
         *      --  ID_CRMI_RESTARTWPS: restart Desktop.
         *      --  ID_CRMI_SHUTDOWN: "real" shutdown.
         *
         *      These are the menu item IDs from the
         *      X-button menu.
         *
         *      We have this message here now because
         *      initiating XShutdown from an XCenter
         *      thread means asking for trouble.
         *
         *@@added V0.9.12 (2001-04-28) [umoeller]
         */

        case T1M_INITIATEXSHUTDOWN:
            switch ((ULONG)mp1)
            {
                case ID_CRMI_LOGOFF:
                    xsdInitiateRestartWPS(TRUE);    // logoff
                break;

                case ID_CRMI_RESTARTWPS:
                    xsdInitiateRestartWPS(FALSE);   // restart Desktop, no logoff
                break;

                case ID_CRMI_SHUTDOWN:
                    WinPostMsg(cmnQueryActiveDesktopHWND(),
                               WM_COMMAND,
                               MPFROMSHORT(WPMENUID_SHUTDOWN),
                               MPFROM2SHORT(CMDSRC_MENU,
                                            FALSE));
                break;
            }
        break;

        /*
         *@@ T1M_OPENRUNDIALOG:
         *      this gets posted from the XCenter thread
         *      to open the Run dialog on thread 1.
         *
         *@@added V0.9.14 (2001-08-07) [pr]
         */

        case T1M_OPENRUNDIALOG:
            cmnRunCommandLine((HWND)mp1,
                              (PCSZ)mp2);
        break;

        /*
         *@@ T1M_NOTIFYWAKEUP:
         *      posted ONLY from the replacement refresh
         *      (PumpAgedNotification) when an object has
         *      been added to a folder and needs to be
         *      made awake. We shouldn't do that on the
         *      pump thread in order to avoid system
         *      deadlocks. See the remarks in
         *      PumpAgedNotification for details.
         *
         *      Parameters:
         *
         *      --  PSZ mp1: full path of the new object.
         *          To be free()'d here.
         *
         *@@added V0.9.20 (2002-07-25) [umoeller]
         */

        case T1M_NOTIFYWAKEUP:
            if (mp1)
            {
                _wpclsQueryObjectFromPath(_WPFileSystem,
                                          (PSZ)mp1);
                free(mp1);
            }
        break;

#ifdef __DEBUG__
        case XM_CRASH:          // posted by debugging context menu of XFldDesktop
            CRASH;
        break;
#endif

        default:
            mrc = G_pfnwpObjectStatic(hwndObject, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ krnPostThread1ObjectMsg:
 *      post msg to thread-1 object window (fnwpThread1Object).
 *      See include\shared\kernel.h for the supported T1M_*
 *      messages.
 *      This is used from all kinds of places and different threads.
 */

BOOL krnPostThread1ObjectMsg(ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PCKERNELGLOBALS pKernelGlobals = krnQueryGlobals();
    if (pKernelGlobals->hwndThread1Object)
        return WinPostMsg(pKernelGlobals->hwndThread1Object, msg, mp1, mp2);

    return FALSE;
}

/*
 *@@ krnSendThread1ObjectMsg:
 *      send msg to thread-1 object window (fnwpThread1Object).
 *      See include\shared\kernel.h for the supported T1M_*
 *      messages.
 *      Note that, as usual, sending a message from another
 *      thread will block that thread until we return.
 */

MRESULT krnSendThread1ObjectMsg(ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PCKERNELGLOBALS pKernelGlobals = krnQueryGlobals();
    if (pKernelGlobals->hwndThread1Object)
        return WinSendMsg(pKernelGlobals->hwndThread1Object, msg, mp1, mp2);

    return (MRESULT)NULL;
}

/* ******************************************************************
 *
 *   API object window
 *
 ********************************************************************/

/*
 *@@ fnwpAPIObject:
 *      window proc for the XWorkplace API object window.
 *
 *      This API object window is quite similar to the thread-1
 *      object window (fnwpThread1Object), except that its
 *      messages are defined in include\xwpapi.h. As a result,
 *      this thing handles public messages to allow external
 *      processes to communicate with XWorkplace in the WPS
 *      process.
 *
 *      Like the thread-1 object window, this is created on
 *      Desktop startup by initMain and runs on
 *      thread-1 of the WPS always.
 *
 *@@added V0.9.9 (2001-03-23) [umoeller]
 */

STATIC MRESULT EXPENTRY fnwpAPIObject(HWND hwndObject, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        /*
         *@@ APIM_FILEDLG:
         *      opens the XWorkplace file dialog.
         *
         *      Parameters:
         *
         *      -- PXWPFILEDLG mp1: file dialog parameters. NOTE:
         *         If this message comes from another process,
         *         this better point to shared memory, and all
         *         member pointers must point to shared memory
         *         as well. This shared memory must also be
         *         given to the WPS process.
         *
         *      -- mp2: not used, always 0.
         *
         *      This message must be POSTED to the API object
         *      window, and the posting thread should then enter
         *      a modal message loop until XWPFILEDLG.hwndNotify
         *      receives WM_USER.
         *
         *      Returns: HWND like WinFileDlg.
         *
         *@@added V0.9.9 (2001-03-23) [umoeller]
         */

        case APIM_FILEDLG:
        {
            PXWPFILEDLG pfd;
            if (pfd = (PXWPFILEDLG)mp1)
            {
                // open the (modal) file dialog; this does
                // not return until the dialog is dismissed
                pfd->hwndReturn = fdlgFileDlg(pfd->hwndOwner,
                                              pfd->szCurrentDir,
                                              &pfd->fd);
            }

            // now post WM_USER back to the notify window
            // given to us
            WinPostMsg(pfd->hwndNotify,
                       WM_USER,
                       0, 0);
        }
        break;

        /*
         *@@ APIM_SHOWHELPPANEL:
         *      displays a help panel. Used by NetscapeDDE
         *      and Treesize.
         *
         *      Parameters:
         *
         *      --  ULONG mp1: the help panel from the XWP
         *          online help to be displayed.
         *
         *@@added V0.9.16 (2001-10-02) [umoeller]
         */

        case APIM_SHOWHELPPANEL:
            cmnDisplayHelp(NULL, (ULONG)mp1);
        break;

        default:
            mrc = WinDefWindowProc(hwndObject, msg, mp1, mp2);
    }

    return mrc;
}


