
/*
 * treesize.c:
 *      this is the main (and only) .C file for treesize.exe.
 *
 *      Treesize is new with XFolder V0.81.
 *
 *      This thing is multi-threaded. The second (Collect) thread
 *      (fntCollect) is started by WM_INITDLG of the main window (fnwpMain)
 *      and keeps sending messages to the main window which will then
 *      insert the directory records into the container.
 *
 *      Initially, this was mostly a quick hack done in one day and a half.
 *      Some improvements have been done with XWorkplace V0.9.0 though,
 *      e.g. Treesize is now NLS-enabled.
 *
 *      Still, this code might be interesting if you'd like to know more about
 *      the following:
 *          --  multithreading PM programs and inter-thread communication,
 *          --  using a dlg template as a main window which can be resized,
 *              maximized and minimized,
 *          --  container programming: tree views, context menus, drag-n-drop.
 *
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

#define INCL_DOSPROCESS
#define INCL_DOSMODULEMGR
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINDIALOGS
#define INCL_WINCOUNTRY
#define INCL_WINSHELLDATA
#define INCL_WINPROGRAMLIST
#define INCL_WINPOINTERS
#define INCL_WINTIMER
#define INCL_WINSTATICS
#define INCL_WINMENUS
#define INCL_WINMLE
#define INCL_WINSTDCNR
#define INCL_WINWORKPLACE
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define DONT_REPLACE_FOR_DBCS       // do not replace strchr with DBCS version
#include "setup.h"

#include "helpers\pmprintf.h"
#include "helpers\comctl.h"
#include "helpers\cnrh.h"
#include "helpers\eah.h"
#include "helpers\dosh.h"
#include "helpers\nls.h"
#include "helpers\nlscache.h"       // added V1.0.4 (2005-02-24) [chennecke]: load NLS strings from resource DLL
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\tree.h"
#include "helpers\winh.h"

#include "bldlevel.h"
#include "dlgids.h"

#include "treesize.h"

#include "xwpapi.h"
#include "shared\helppanels.h"

/*
 * Global variables
 *
 */

// the directory to start with
CHAR        G_szRootDir[CCHMAXPATH];
ULONG       G_cbRootDir = sizeof(G_szRootDir)-3;

// NLS Resource DLL
HMODULE     G_hmodNLS = NULLHANDLE;

// main wnd handle
HWND        G_hwndMain = NULLHANDLE;

// Collect thread ID: always 0 if not running
TID         G_tidCollect = 0;
HAB         G_habCollect;
HMQ         G_hmqCollect;
// setting this to TRUE will stop the Collect thread
BOOL        G_fStopThread = FALSE;

// DIRINFO struct for the directory to start with
PDIRINFO    G_pdiRoot;

// and the settings read from/ stored to OS2.INI
struct {
    ULONG       ulSort;
    BOOL        CollectEAs;
    ULONG       ulSizeDisplay;
    BOOL        LowPriority;
} G_Settings;

TREE    *G_LargestFilesTree;
LONG    G_cLargestFilesTree = 0;

const char      *G_pcszXFldTreesize = "XFldTreesize";

// shortcuts to dlg controls
HWND        G_hwndCnr,
            G_hwndText,
            G_hwndIcon,
            G_hwndClose,
            G_hwndClear,
            G_hwndRefresh;

// thousands separator from "Country" object
CHAR        G_szThousand[10];

// for container scrolling
PRECORDCORE G_preccScrollTo = NULL;

// some storage for between dragging and dropping
BOOL        G_fDnDValid = FALSE;
CHAR        G_szDir[CCHMAXPATH];
CHAR        G_szFile[CCHMAXPATH];

// is window minimized?
BOOL        G_fMinimized = FALSE;

// the parent tree entry of the 100 largest files
// V0.9.15 (2001-08-25) [rbri] sort the file entries by size
PRECORDCORE G_precParentOf100Largest = NULL;


/********************************************************************
 *
 *   Collect thread
 *
 ********************************************************************/

/*
 * CollectDirectory:
 *      initially called by the Collect thread (fntCollect)
 *      and then calls itself recursively for each
 *      directory it finds.
 *      Takes a DIRINFO structure, which is also initially
 *      created by the Collect thread.
 *      This sends two messages to the main window (fnwpMain):
 *      WM_BEGINDIRECTORY, which causes the record core
 *      to be created, and WM_ENDDIRECTORY, which causes
 *      the record core to be updated with the directory size,
 *      when processing of this directory (and all the subdirs)
 *      is done with.
 *
 *@@changed V0.9.14 (2001-07-28) [umoeller]: added largest files collect
 *@@changed V0.9.14 (2001-07-28) [umoeller]: now using WinPostMsg, which is speedier
 *@@changed V1.0.4 (2005-02-24) [chennecke]: replaced hard-coded strings with corresponding nlsGetString() calls
 *@@changed V1.0.9 (2010-07-17) [pr]: added large file support @@fixes 586
 *@@changed V1.0.9 (2010-07-22) [pr]: fix missing files caused by duplicate tree keys
 */

VOID CollectDirectory(PDIRINFO pdiThis)
{
    HDIR          hdirFindHandle = HDIR_CREATE;
    FILEFINDBUF3L ffb3     = {0};      /* Returned from FindFirst/Next */
    ULONG         cbFFB3 = sizeof(ffb3);
    ULONG         ulFindCount    = 1;        /* Look for 1 file at a time    */
    APIRET        rc             = NO_ERROR; /* Return code                  */
    CHAR          szCurrentDir[CCHMAXPATH],
                  szSearchMask[CCHMAXPATH];

    ULONG         ulFilesThisDir = 0;
    double        dSizeThisDir = 0,
                  dEASizeThisDir = 0;

    // add a "\" to the directory, if we're not in root
    strcpy(szCurrentDir, pdiThis->szFullPath);
    if (szCurrentDir[strlen(szCurrentDir)-1] != '\\')
        strcat(szCurrentDir, "\\");

    // have record core inserted; since we're SENDing this
    // msg (and not posting), this also takes care of
    // thread synchronization so that the record cores don't
    // get messed up. Sending messages across threads blocks
    // the calling thread until the msg has been processed.
    // Ah yes, and this is why we may only send this if our
    // thread hasn't been requested to terminate. In that case,
    // the main window might just be being destroyed, so this
    // would block PM. Sigh.
    if (!G_fStopThread)
        WinSendMsg(G_hwndMain, TSM_BEGINDIRECTORY, (MPARAM)pdiThis, NULL);

    // now go for the first directory entry in our directory (szCurrentDir):
    strcpy(szSearchMask, szCurrentDir);
    strcat(szSearchMask, "*");
    rc = DosFindFirst( szSearchMask,
                       &hdirFindHandle,
                       // find eeeeverything
                       FILE_ARCHIVED | FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY | FILE_DIRECTORY,
                       &ffb3,
                       cbFFB3,
                       &ulFindCount,
                       FIL_STANDARDL);

    // and start looping
    while (     (rc == NO_ERROR)
            &&  (!G_fStopThread)      // if our thread hasn't been stopped
          )
    {
        CHAR szFile[CCHMAXPATH];
        ULONG ulEASize;

        sprintf(szFile, "%s%s", szCurrentDir, // always has trailing "\"
                                ffb3.achName);

        // get EA size
        if (    (G_Settings.CollectEAs)
             && (strlen(szCurrentDir) > 3)
           )
        {
            ulEASize = eaPathQueryTotalSize(szFile);
            _Pmpf(("EAs for %s: %d", ffb3.achName, ulEASize));
            dEASizeThisDir += ulEASize;
        }

        if (ffb3.attrFile & FILE_DIRECTORY)
        {
            if (    (strcmp(ffb3.achName, ".") != 0)
                 && (strcmp(ffb3.achName, "..") != 0)
               )
            {
                // subdirectory found: recurse!

                // create new DIRINFO for the recursive call
                PDIRINFO pdiRecurse = malloc(sizeof(DIRINFO));
                pdiRecurse->ulFiles = 0;
                pdiRecurse->dTotalSize0 = 0;
                pdiRecurse->dTotalEASize = 0;
                pdiRecurse->pParent = pdiThis;
                strcpy(pdiRecurse->szThis, ffb3.achName);
                strcpy(pdiRecurse->szFullPath, szFile);
                pdiRecurse->ulRecursionLevel = pdiThis->ulRecursionLevel + 1;
                // and go!
                CollectDirectory(pdiRecurse);
            }
        }
        else
        {
            // regular file:
            ulFilesThisDir++;
            // add to total size
            dSizeThisDir += 65536.0 * 65536.0 * ffb3.cbFile.ulHi
                            + ffb3.cbFile.ulLo;  // V1.0.9

            if (ffb3.cbFile.ulLo > 1024)
            {
                // create entry for largest files tree
                PFILEENTRY pFileEntry = NEW(FILEENTRY);
                if (pFileEntry)
                {
                    // V1.0.9
                    PCHAR pszSize = malloc(40);

                    pFileEntry->pDir = pdiThis;
                    pFileEntry->pszFilename = strdup(ffb3.achName);
                    pFileEntry->dSize = 65536.0 * 65536.0 * ffb3.cbFile.ulHi +
                        ffb3.cbFile.ulLo;
                    sprintf(pszSize, "%020.0f", floor(pFileEntry->dSize + 0.5));
                    pFileEntry->Tree.ulKey = (ULONG) pszSize;

                    treeInsert(&G_LargestFilesTree,
                               &G_cLargestFilesTree,
                               (TREE*)pFileEntry,
                               treeCompareStrings);
                }
            }
        }

        rc = DosFindNext(hdirFindHandle,
                         &ffb3,
                         cbFFB3,
                         &ulFindCount);

    } /* endwhile */

    DosFindClose(hdirFindHandle);

    // now that we're done with this directory:
    // store the sizes in the current DIRINFO
    pdiThis->ulFiles += ulFilesThisDir;
    pdiThis->dTotalSize0 += dSizeThisDir;
    pdiThis->dTotalEASize += dEASizeThisDir;

    // add the size of the subdirector(ies) to
    // the parent's size

    if (!G_fStopThread)
    {
        PDIRINFO      pdiParent = pdiThis->pParent;

        while (pdiParent)
        {
            pdiParent->ulFiles += ulFilesThisDir;
            pdiParent->dTotalSize0 += dSizeThisDir;
            pdiParent->dTotalEASize += dEASizeThisDir;
            pdiParent = pdiParent->pParent;
            // pdiThis2 = pdiThis2->pParent;
        }

        // have record core updated with new total size
        WinPostMsg(G_hwndMain, TSM_DONEDIRECTORY, (MPARAM)pdiThis, NULL);
    }

    // end of function: if this was a recursive call,
    // this will continue collecting files in the parent
    // directory. If this was the "root" call from fntCollect,
    // the Collect thread will then terminate.
};

/*
 * fntCollect:
 *      Collect thread, started by fnwpMain below.
 *      The main window can always determine whether
 *      the Collect thread is running by checking
 *      whether (tidCollect != 0). There is never
 *      more than one "instance" of this thread running.
 *
 *@@changed V1.0.9 (2010-07-22) [pr]: fix memory leaks
 */

void _System fntCollect(ULONG ulDummy)
{
    // we need a msg queue for WinSendMsg
    if (!(G_habCollect = WinInitialize(0)))
        return;
    if (!(G_hmqCollect = WinCreateMsgQueue(G_habCollect, 0)))
        return;

    // low priority?
    DosSetPriority(PRTYS_THREAD,
                   (G_Settings.LowPriority)
                        ? PRTYC_IDLETIME
                        : PRTYC_REGULAR,
                   0,       // delta
                   0);      // current thread

    treeInit(&G_LargestFilesTree, &G_cLargestFilesTree);

    // prepare "root" DIRINFO structure for CollectDirectory()
    G_pdiRoot = (PDIRINFO)malloc(sizeof(DIRINFO));
    if (G_pdiRoot)
    {
        G_pdiRoot->pParent = NULL;
        G_pdiRoot->ulFiles = 0;
        G_pdiRoot->dTotalSize0 = 0;
        G_pdiRoot->dTotalEASize = 0;
        strcpy(G_pdiRoot->szThis, G_szRootDir);
        strcpy(G_pdiRoot->szFullPath, G_szRootDir);
        G_pdiRoot->ulRecursionLevel = 1;
        CollectDirectory(G_pdiRoot);
    }

    // report that we're done completely
    if (!G_fStopThread)
        WinPostMsg(G_hwndMain, TSM_DONEWITHALL, NULL, NULL);

    WinDestroyMsgQueue(G_hmqCollect);
    WinTerminate(G_habCollect);
    G_tidCollect = 0;
}

/********************************************************************
 *
 *   Main thread
 *
 ********************************************************************/

/*
 *@@ SaveSettings:
 *
 *@@added V0.9.16 (2001-12-02) [umoeller]
 */

VOID SaveSettings(VOID)
{
    PrfWriteProfileData(HINI_USER,
                        (PSZ)G_pcszXFldTreesize,
                        "Settings",
                        &G_Settings,
                        sizeof(G_Settings));
}

/*
 * Cleanup:
 *      deallocates all memory and cleans up record cores.
 *      Also recurses; for initial call, preccParent
 *      must be NULL.
 *
 *@@changed V0.9.15 (2001-08-26) [rbri] free for file entries added
 *@@changed V1.0.9 (2010-07-22) [pr]: fix memory leaks
 */

VOID Cleanup(PSIZERECORD preccParent)
{
    PSIZERECORD precc2 = preccParent;

    do {
        precc2 =
            (PSIZERECORD)WinSendMsg(G_hwndCnr,
                                        CM_QUERYRECORD,
                                        // the following ugly code does the following:
                                        // if this is the "root" call: get first child recc.
                                        // For recursive calls: get next child recc.
                                        (MPARAM)((preccParent) // "root" call?
                                                    ? preccParent
                                                    : precc2), // don't care
                                        MPFROM2SHORT(
                                                ((preccParent) // "root" call?
                                                    ? CMA_FIRSTCHILD
                                                    : CMA_FIRST
                                            ),
                                            CMA_ITEMORDER)
                                        );
        if ((precc2) && ((ULONG)precc2 != -1))
        {
            Cleanup(precc2);
            // _Pmpf(("Removing %s", precc2->pdi->szRecordText));
            if (precc2->pdi)
                free(precc2->pdi);

            if (precc2->pFileEntry)
            {
                free((void *) precc2->pFileEntry->Tree.ulKey);
                free(precc2->pFileEntry);
            }

            WinSendMsg(G_hwndCnr,
                       CM_REMOVERECORD,
                       &precc2,
                       MPFROM2SHORT(1,   // remove one recc
                           CMA_FREE));
        }
    } while ((precc2) && ((ULONG)precc2 != -1));

    if (preccParent == NULL)
        WinSendMsg(G_hwndCnr,
                   CM_INVALIDATERECORD,
                   NULL,
                   MPFROM2SHORT(0, CMA_REPOSITION));
}

/*
 * fnCompareName:
 *      comparison func for container sort by name
 */

SHORT EXPENTRY fnCompareName(PRECORDCORE pmrc1, PRECORDCORE pmrc2, PVOID pStorage)
{
    HAB habDesktop = WinQueryAnchorBlock(HWND_DESKTOP);
    // pStorage = pStorage; // to keep the compiler happy
    if ((pmrc1) && (pmrc2))
        if ((pmrc1->pszIcon) && (pmrc2->pszIcon))
            switch (WinCompareStrings(habDesktop, 0, 0,
                pmrc1->pszIcon, pmrc2->pszIcon, 0))
            {
                case WCS_LT: return -1;
                case WCS_GT: return 1;
            }

    return 0;
}

/*
 * fnCompareSize:
 *      comparison func for container sort by file size
 *
 *@@changed V0.9.15 (2001-08-25) [rbri] also compare the file entries
 */

SHORT EXPENTRY fnCompareSize(PSIZERECORD pmrc1, PSIZERECORD pmrc2, PVOID pStorage)
{
    // pStorage = pStorage; // to keep the compiler happy
    if ((pmrc1) && (pmrc2))
    {
        if (    ((pmrc1->pdi) && (pmrc2->pdi))
             || ((pmrc1->pFileEntry) && (pmrc2->pFileEntry))
           )
        {
            if (pmrc1->dTotalSize > pmrc2->dTotalSize)
                return -1;
            else if (pmrc1->dTotalSize < pmrc2->dTotalSize)
                return 1;
        }
    }

    return 0;
}

/*
 * fnCompareFilesCount:
 *      comparison func for container sort by files count
 *
 *@@added V0.9.1 [umoeller]
 *@@changed V0.9.15 (2001-08-25) [rbri] sort the file entries by size
 */

SHORT EXPENTRY fnCompareFilesCount(PSIZERECORD pmrc1, PSIZERECORD pmrc2, PVOID pStorage)
{
    pStorage = pStorage; // to keep the compiler happy
    if ((pmrc1) && (pmrc2))
        if ((pmrc1->pdi) && (pmrc2->pdi))
        {
            if (pmrc1->pdi->ulFiles > pmrc2->pdi->ulFiles)
                return -1;
            else if (pmrc1->pdi->ulFiles < pmrc2->pdi->ulFiles)
                return 1;
        }
        else if ((pmrc1->pFileEntry) && (pmrc2->pFileEntry))
        {
            // sort the file entries by size
            if (pmrc1->dTotalSize > pmrc2->dTotalSize)
                return -1;
            else if (pmrc1->dTotalSize < pmrc2->dTotalSize)
                return 1;
        }

    return 0;
}

/*
 * fnCompareEASize:
 *      comparison func for container sort by ea size
 *
 *@@changed V0.9.15 (2001-08-25) [rbri] sort the file entries by size
 */

SHORT EXPENTRY fnCompareEASize(PSIZERECORD pmrc1, PSIZERECORD pmrc2, PVOID pStorage)
{
    pStorage = pStorage; // to keep the compiler happy
    if ((pmrc1) && (pmrc2))
        if ((pmrc1->pdi) && (pmrc2->pdi))
        {
            if (pmrc1->pdi->dTotalEASize > pmrc2->pdi->dTotalEASize)
                return -1;
            else if (pmrc1->pdi->dTotalEASize < pmrc2->pdi->dTotalEASize)
                return 1;
        }
        else if ((pmrc1->pFileEntry) && (pmrc2->pFileEntry))
        {
            // sort the file entries by size
            if (pmrc1->dTotalSize > pmrc2->dTotalSize)
                return -1;
            else if (pmrc1->dTotalSize < pmrc2->dTotalSize)
                return 1;
        }

    return 0;
}

/*
 *@@ ComposeFilename:
 *
 *@@added V0.9.14 (2001-07-28) [umoeller]
 */

PSZ ComposeFilename(PSZ pszBuf,
                    PFILEENTRY pFileEntry)
{
    sprintf(pszBuf,
            "%s\\%s",
            pFileEntry->pDir->szFullPath,
            pFileEntry->pszFilename);

    return pszBuf;
}

/*
 *@@ Insert100LargestFiles:
 *
 *@@added V0.9.14 (2001-07-28) [umoeller]
 *@@changed V0.9.15 (2001-08-25) [rbri] the 100 largest entries are available only once after D&D
 *@@changed V1.0.4 (2005-02-24) [chennecke]: replaced hard-coded strings with corresponding nlsGetString() calls
 *@@changed V1.0.9 (2010-07-21) [pr]: fixed memory leaks and missing largest files on Refresh
 */

VOID Insert100LargestFiles(VOID)
{
    ULONG cFiles;
    PSIZERECORD precFirst;
    cFiles = G_cLargestFilesTree;
    if (cFiles > 100)
        cFiles = 100;
    if (precFirst = (PSIZERECORD)cnrhAllocRecords(G_hwndCnr,
                                                  sizeof(SIZERECORD),
                                                  cFiles))
    {
        PSIZERECORD precThis = precFirst;

        PFILEENTRY pEntry = (PFILEENTRY)treeLast(G_LargestFilesTree);

        CHAR szSize[200];
        CHAR szFilename[400],
             szTemp[1000];

        sprintf(szSize, nlsGetString(ID_TSSI_LARGESTFILES), cFiles);

        // create the entry only if it is not already there
        // V1.0.9
        if (G_precParentOf100Largest == NULL)
            G_precParentOf100Largest = cnrhAllocRecords(G_hwndCnr,
                                                      sizeof(SIZERECORD),
                                                      1);
        else
            free(G_precParentOf100Largest->pszIcon);

        cnrhInsertRecords(G_hwndCnr,
                          NULL,      // parent
                          G_precParentOf100Largest,
                          TRUE,
                          strdup(szSize),
                          CRA_RECORDREADONLY | CRA_COLLAPSED,
                          1);
        // build & insert the new subnodes
        while (pEntry && precThis)
        {
            precThis->dTotalSize = pEntry->dSize;  // V1.0.9

            szFilename[0] = '\0';

            sprintf(szTemp,
                    "%s (%s %s)",
                    ComposeFilename(szFilename, pEntry),
                    nlsThousandsDouble(szSize,
                                        (G_Settings.ulSizeDisplay == SD_BYTES)
                                                ? precThis->dTotalSize
                                            : (G_Settings.ulSizeDisplay == SD_KBYTES)
                                                ? ((precThis->dTotalSize + 512) / 1024)
                                            : ((precThis->dTotalSize + (512*1024)) / 1024 / 1024),
                                        G_szThousand[0]),
                    // "bytes" string:
                    (G_Settings.ulSizeDisplay == SD_BYTES)
                            ? nlsGetString(ID_TSSI_BYTES)
                        : (G_Settings.ulSizeDisplay == SD_KBYTES)
                            ? nlsGetString(ID_TSSI_KBYTES)
                        : nlsGetString(ID_TSSI_MBYTES));

            precThis->pFileEntry = pEntry;

            precThis->recc.pszIcon
            = precThis->recc.pszName
            = precThis->recc.pszText
            = precThis->recc.pszTree
                = strdup(szTemp);

            precThis = (PSIZERECORD)precThis->recc.preccNextRecord;
            pEntry = (PFILEENTRY)treePrev((TREE*)pEntry);
        }

        cnrhInsertRecords(G_hwndCnr,
                          G_precParentOf100Largest,
                          (PRECORDCORE)precFirst,
                          TRUE,
                          NULL,
                          CRA_RECORDREADONLY,
                          cFiles);
    }
}

/*
 * fnwpMain:
 *      dlg func for main window. This does all the
 *      processing after main() has called WinProcessDlg.
 *      This mainly does the following:
 *      --  update the container by reacting to msgs from
 *          the Collect thread
 *      --  handle cnr context menus
 *      --  handle cnr drag'n'drop
 *      --  handle dialog resizing/minimizing etc.
 *
 *@@changed V0.9.14 (2001-08-09) [umoeller]: fixed bad sort by size that broke with 100 largest files
 *@@changed V1.0.0 (2002-11-23) [bvl]: added "Refresh" button @@fixes 172
 *@@changed V1.0.4 (2005-02-24) [chennecke]: replaced hard-coded strings with corresponding nlsGetString() calls
 */

MRESULT EXPENTRY fnwpMain(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        /*
         * WM_INITDLG:
         *      Dialog wnds don't get WM_CREATE, they get
         *      WM_INITDLG instead.
         *      setup all the controls, mainly the cnr.
         */

        case WM_INITDLG:
        {
            CNRINFO CnrInfo;

            // remember wnd handles for later
            G_hwndIcon = WinWindowFromID(hwndDlg, ID_TSDI_ICON);
            G_hwndText = WinWindowFromID(hwndDlg, ID_TSDI_TEXT1);
            G_hwndCnr = WinWindowFromID(hwndDlg, ID_TSDI_CNR);
            G_hwndClose = WinWindowFromID(hwndDlg, DID_OK);
            G_hwndClear = WinWindowFromID(hwndDlg, DID_CLEAR);
            G_hwndRefresh = WinWindowFromID(hwndDlg, DID_REFRESH);

            // set font V0.9.16 (2001-12-02) [umoeller]
            if (doshIsWarp4())
            {
                PCSZ pcszWS = "9.WarpSans";

                winhSetWindowFont(G_hwndText,
                                  "9.WarpSans Bold");
                winhSetWindowFont(G_hwndCnr,
                                  pcszWS);
                winhSetWindowFont(G_hwndClose,
                                  pcszWS);
                winhSetWindowFont(G_hwndClear,
                                  pcszWS);
                winhSetWindowFont(G_hwndRefresh,
                                  pcszWS);
            }

            // set up the container
            WinSendMsg(G_hwndCnr, CM_QUERYCNRINFO, &CnrInfo, (MPARAM)sizeof(CnrInfo));
            // don't sort initially, because sort-by-size doesn't
            // work until we have all the directory sizes. Instead,
            // we add all the containers to the end of the list
            // initially, which also reduces cnr repainting. The
            // list is only sorted after processing is done completely.
            CnrInfo.pSortRecord = NULL;
            // switch to tree view
            CnrInfo.flWindowAttr = CV_TREE | CV_TEXT | CA_TREELINE;
            CnrInfo.cxTreeIndent = 25;
            WinSendMsg(G_hwndCnr, CM_SETCNRINFO,
                       &CnrInfo,
                       (MPARAM)( /* CMA_PSORTRECORD | */ CMA_FLWINDOWATTR | CMA_CXTREEINDENT));

            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);

            // get the thousands separator for later; this
            // is what is specified in the "Country" object
            PrfQueryProfileString(HINI_USER,
                                  "PM_Default_National",
                                  "sThousand",
                                  ",",
                                  G_szThousand,
                                  sizeof(G_szThousand)-1);

            // initiate processing (below)
            WinPostMsg(hwndDlg, TSM_START, 0, 0);
        }
        break;

        /*
         * TSM_START:
         *      start collect thread. This is posted
         *      both by WM_INITDLG above and after folders have
         *      been dropped on the container (CN_DROP below)
         */

        case TSM_START:
            WinSetWindowText(G_hwndText, G_szRootDir);
            WinEnableWindow(G_hwndClear, FALSE);
            WinEnableWindow(G_hwndRefresh, FALSE);
            G_fStopThread = FALSE;
            DosCreateThread(&G_tidCollect, fntCollect, 0,
                    CREATE_READY, 4*65536);
            WinSetActiveWindow(HWND_DESKTOP, hwndDlg);
                // needed when something has been dropped
        break;

        /*
         * TSM_BEGINDIRECTORY:
         *      posted by CollectDirectory (in the Collect
         *      thread) when it's starting to work on a directory.
         *      We will insert the record core then, which
         *      initially displays "Working..."
         */

        case TSM_BEGINDIRECTORY:
        {
            // the DIRINFO struct is passed in mp1 by
            // the Collect thread
            PDIRINFO pdi = (PDIRINFO)mp1;

            // allocate memory for record core
            pdi->precc = cnrhAllocRecords(G_hwndCnr,
                                          sizeof(SIZERECORD),
                                          1);

            // recc text
            sprintf(pdi->szRecordText, nlsGetString(ID_TSSI_WORKING), pdi->szThis);

             _Pmpf(("  WM_BEGINDIRECTORY %s", pdi->szRecordText));
             _Pmpf(("    Level: %d", pdi->ulRecursionLevel));

            if (pdi->precc)
            {
                PSIZERECORD preccParent = NULL;

                // store reverse pointer; DIRINFO points
                // to the record core, the recc points
                // to DIRINFO (check treesize.h)
                ((PSIZERECORD)(pdi->precc))->pdi = pdi;
                ((PSIZERECORD)(pdi->precc))->fDisplayValid = FALSE;
                ((PSIZERECORD)(pdi->precc))->dTotalSize = pdi->dTotalSize0;

                // store parent directory
                if (pdi->pParent)
                    preccParent = (PSIZERECORD)pdi->pParent->precc;

                // insert recc into container
                cnrhInsertRecords(G_hwndCnr,
                                  (PRECORDCORE)preccParent,
                                      // parent for tree view;
                                      // NULL for root level
                                  (PRECORDCORE)pdi->precc,
                                      // recc to insert
                                  TRUE,
                                  pdi->szRecordText,
                                  // recc attributes:
                                  // don't make editable
                                  CRA_RECORDREADONLY
                                      // initially, expand tree two levels deep
                                      | ((pdi->ulRecursionLevel < 3)
                                          ? CRA_EXPANDED
                                          : CRA_COLLAPSED),
                                      // invalidate (repaint) only if parent is expanded;
                                      // this avoids excessive cnr repaint
                                 /* (preccParent)
                                     ? (preccParent->recc.flRecordAttr & CRA_EXPANDED)
                                     : TRUE,  // invalidate for root level
                                  */
                                  1);

                // select recc if it's the root recc;
                // in Tree view, this automatically deselects
                // the previous selection
                if (pdi->ulRecursionLevel == 1)
                    WinSendMsg(G_hwndCnr,
                               CM_SETRECORDEMPHASIS,
                               pdi->precc,
                               MPFROM2SHORT(
                                   // select or deselect flag
                                   TRUE,
                                   CRA_SELECTED
                               ));
            }
        }
        break;

        /*
         * TSM_DONEDIRECTORY:
         *      posted by CollectDirectory when processing
         *      of a directory and all of its subdirectories
         *      is done. We will update the recc to show
         *      the dir's size.
         *
         *      Parameters:
         *      PDIRINFO mp1: directory information.
         *
         *      When a recc text changes, the record core
         *      _always_ needs to be invalidated, or the
         *      cnr displays garbage. To avoid excessive
         *      cnr repainting, we try to do this economically.
         */

        case TSM_DONEDIRECTORY:
        {
            CHAR szSize[30];
            CHAR szEASize[30];
            CHAR szFiles[30];

            // the DIRINFO struct is passed in mp1 by
            // the Collect thread
            PDIRINFO    pdiStart = (PDIRINFO)mp1,
                        pdiThis = pdiStart;

            // now work on this DIRINFO and climb up
            // the parent tree
            while (pdiThis)
            {
                // recc's parent record
                PRECORDCORE preccParent =
                                (pdiThis->pParent)
                                    ? pdiThis->pParent->precc
                                    : NULL;

                // invalidate (repaint) record only if the
                // parent recc is expanded; for collapsed trees,
                // this repaint is done only when the tree is
                // expanded manually (CN_EXPAND below) to avoid
                // excessive cnr repaint
                BOOL fInvalidate = (preccParent)
                                    ? (preccParent->flRecordAttr & CRA_EXPANDED)
                                    : TRUE;

                // new recc title:
                sprintf(pdiThis->szRecordText, "%s (%s %s, %s %s",
                        // title:
                        pdiThis->szThis,
                        // value:
                        nlsThousandsDouble(szSize,
                                            (G_Settings.ulSizeDisplay == SD_BYTES)
                                                    ? pdiThis->dTotalSize0
                                                : (G_Settings.ulSizeDisplay == SD_KBYTES)
                                                    ? ((pdiThis->dTotalSize0 + 512) / 1024)
                                                : ((pdiThis->dTotalSize0 + (512*1024)) / 1024 / 1024),
                                            G_szThousand[0]),
                        // "bytes" string:
                        (G_Settings.ulSizeDisplay == SD_BYTES)
                                ? nlsGetString(ID_TSSI_BYTES)
                            : (G_Settings.ulSizeDisplay == SD_KBYTES)
                                ? nlsGetString(ID_TSSI_KBYTES)
                            : nlsGetString(ID_TSSI_MBYTES),
                        nlsThousandsULong(szFiles,
                                           pdiThis->ulFiles,
                                           G_szThousand[0]),
                        nlsGetString(ID_TSSI_FILES));

                if (G_Settings.CollectEAs)
                    sprintf(pdiThis->szRecordText + strlen(pdiThis->szRecordText),
                            ", %s %s EAs",
                            nlsThousandsDouble(szSize,
                                                (G_Settings.ulSizeDisplay == SD_BYTES)
                                                        ? pdiThis->dTotalEASize
                                                    : (G_Settings.ulSizeDisplay == SD_KBYTES)
                                                        ? ((pdiThis->dTotalEASize + 512) / 1024)
                                                    : ((pdiThis->dTotalEASize + (512*1024)) / 1024 / 1024),
                                                G_szThousand[0]),
                           (G_Settings.ulSizeDisplay == SD_BYTES)
                                   ? nlsGetString(ID_TSSI_BYTES)
                               : (G_Settings.ulSizeDisplay == SD_KBYTES)
                                   ? nlsGetString(ID_TSSI_KBYTES)
                               : nlsGetString(ID_TSSI_MBYTES));

                strcat(pdiThis->szRecordText, ")");
                // _Pmpf(("  WM_DONEDIRECTORY %s", pdi->szRecordText));

                // refresh the current dir size
                // (sort funcs use the RECORDCORE field, not the
                // DIRINFO any more V0.9.14 (2001-08-18) [umoeller])
                ((PSIZERECORD)(pdiThis->precc))->dTotalSize = pdiThis->dTotalSize0;

                if (fInvalidate)
                {
                    WinSendMsg(G_hwndCnr,
                               CM_INVALIDATERECORD,
                               (MPARAM)&(pdiThis->precc),
                               MPFROM2SHORT(1,     // one record only
                                       CMA_TEXTCHANGED));
                    ((PSIZERECORD)pdiThis->precc)->fDisplayValid = TRUE;
                }

                pdiThis = pdiThis->pParent;
            }
        }
        break;

        /*
         * TSM_DONEWITHALL:
         *      posted by the Collect thread right before
         *      it is ending. We will now sort the whole cnr.
         */

        case TSM_DONEWITHALL:
        {
            WinEnableWindow(G_hwndClear, TRUE);
            WinEnableWindow(G_hwndRefresh, TRUE);

            // insert 100 largest files
            Insert100LargestFiles();

            // sort cnr; ulSort is a global var containing
            // either SV_SIZE or SV_EASIZE or SV_NAME or SV_FILESCOUNT
            WinSendMsg(G_hwndCnr,
                        CM_SORTRECORD,
                        ((G_Settings.ulSort == SV_SIZE) ? (MPARAM)fnCompareSize
                        : (G_Settings.ulSort == SV_EASIZE) ? (MPARAM)fnCompareEASize
                        : (G_Settings.ulSort == SV_FILESCOUNT) ? (MPARAM)fnCompareFilesCount
                        : (MPARAM)fnCompareName),
                        NULL);

            // scroll to make root record visible
            G_preccScrollTo = G_pdiRoot->precc;
            WinStartTimer(WinQueryAnchorBlock(hwndDlg),
                          hwndDlg,
                          2,
                          100);
        }
        break;

        /*
         * WM_CONTROL:
         *
         */

        case WM_CONTROL:
        {
            switch (SHORT2FROMMP(mp1)) // notify code
            {

                /*
                 * CN_ENTER:
                 *      double-click or "Enter" on
                 *      record core: open WPS folder
                 */

                case CN_ENTER:
                {
                    PNOTIFYRECORDENTER pnre = (PVOID)mp2;
                    if (pnre)
                    {
                        PSIZERECORD precc = (PVOID)pnre->pRecord;
                        if (precc)
                        {
                            HOBJECT hobj = 0;
                            if (precc->pdi)
                                hobj = WinQueryObject(precc->pdi->szFullPath);
                            else if (precc->pFileEntry)
                            {
                                // entry in 100 largest files:
                                hobj = WinQueryObject(precc->pFileEntry->pDir->szFullPath);
                            }

                            if (hobj)
                                WinOpenObject(hobj,
                                              0,          // == OPEN_DEFAULT
                                              TRUE);
                        }
                        // else double click on whitespace, which we'll ignore
                    }
                }
                break;

                /*
                 * CN_EXPANDTREE:
                 *      tree view has been expanded:
                 *      do cnr auto-scroll if we're
                 *      not initially filling the cnr,
                 *      and invalidate all the sub-records,
                 *      if they have not been invalidated
                 *      before
                 */

                case CN_EXPANDTREE:
                {
                    PRECORDCORE precc2 = NULL;
                    BOOL fFirstRun = TRUE,
                         fWindowDisabled = FALSE;
                    G_preccScrollTo = mp2;

                    // loop thru all the child records
                    // and invalidate them
                    do
                    {
                        precc2 =
                            (PRECORDCORE)WinSendMsg(G_hwndCnr,
                                                    CM_QUERYRECORD,
                                                    // the following gets either the
                                                    // first child recc of the recc
                                                    // that has been expanded or the
                                                    // next child for consecutive loops
                                                    (MPARAM)((fFirstRun)
                                                            ? G_preccScrollTo   // first loop
                                                            : precc2),
                                                    MPFROM2SHORT(
                                                            ((fFirstRun)
                                                            ? CMA_FIRSTCHILD  // first loop
                                                            : CMA_NEXT
                                                        ),
                                                        CMA_ITEMORDER)
                                                    );
                        fFirstRun = FALSE;
                        if ((precc2) && ((ULONG)precc2 != -1))
                            if (!((PSIZERECORD)precc2)->fDisplayValid)
                            {
                                if (!fWindowDisabled)
                                {
                                    WinEnableWindowUpdate(G_hwndCnr, FALSE);
                                    fWindowDisabled = TRUE;
                                }
                                WinSendMsg(G_hwndCnr,
                                           CM_INVALIDATERECORD,
                                           (MPARAM)&precc2,
                                           MPFROM2SHORT(1,     // one record only
                                                   CMA_TEXTCHANGED));

                                ((PSIZERECORD)precc2)->fDisplayValid = TRUE;
                            }
                    } while ((precc2) && ((ULONG)precc2 != -1));

                    if (fWindowDisabled)
                        WinEnableWindowUpdate(G_hwndCnr, TRUE);

                    // do cnr auto-scroll
                    WinStartTimer(WinQueryAnchorBlock(hwndDlg),
                                  hwndDlg,
                                  1,
                                  100);
                }
                break;

                /*
                 * CN_EMPHASIS:
                 *      selection changed: update title
                 */

                case CN_EMPHASIS:
                {
                    // get cnr notification struct
                    PNOTIFYRECORDEMPHASIS pnre = (PNOTIFYRECORDEMPHASIS)mp2;

                    if (pnre)
                        if (    (pnre->fEmphasisMask & CRA_SELECTED)
                             && (pnre->pRecord)
                             && (((PSIZERECORD)(pnre->pRecord))->pdi)
                           )
                        {
                            WinSetWindowText(G_hwndText,
                                             ((PSIZERECORD)(pnre->pRecord))->pdi->szFullPath);
                        }
                }
                break;

                /*
                 * CN_CONTEXTMENU:
                 *      context menu requested
                 */

                case CN_CONTEXTMENU:
                {
                    POINTL ptl;
                    HWND   hPopupMenu;
                    ULONG ulFlagsSelected = 0;
                    CHAR szDummy[2000];

                    // mp2 contains the recc upon which the context
                    // menu was requested or NULL for cnr whitespace
                    PSIZERECORD preccSelected = (PSIZERECORD)mp2;

                    // open context menu only on whitespace
                    // of container
                    if (!preccSelected)
                    {
                        // whitespace:
                        hPopupMenu = WinLoadMenu(hwndDlg,
                                                 G_hmodNLS, ID_TSM_CONTEXT);

                        WinQueryPointerPos(HWND_DESKTOP, &ptl);
                        WinPopupMenu(HWND_DESKTOP, hwndDlg,
                                     hPopupMenu,
                                     (SHORT)ptl.x,
                                     (SHORT)ptl.y,
                                     0,
                                     PU_NONE | PU_MOUSEBUTTON1 | PU_KEYBOARD
                                            | PU_HCONSTRAIN | PU_VCONSTRAIN);

                        // check current sort item
                        WinSendMsg(hPopupMenu, MM_SETITEMATTR,
                                   MPFROM2SHORT((G_Settings.ulSort == SV_NAME) ? ID_TSMI_SORTBYNAME
                                                   : (G_Settings.ulSort == SV_EASIZE) ? ID_TSMI_SORTBYEASIZE
                                                   : (G_Settings.ulSort == SV_FILESCOUNT) ? ID_TSMI_SORTBYFILESCOUNT
                                                   : ID_TSMI_SORTBYSIZE
                                                , TRUE),
                                   MPFROM2SHORT(MIA_CHECKED,
                                                MIA_CHECKED));

                        // check size display
                        WinSendMsg(hPopupMenu, MM_SETITEMATTR,
                                MPFROM2SHORT((G_Settings.ulSizeDisplay == SD_BYTES) ? ID_TSMI_SIZE_BYTES
                                                : (G_Settings.ulSizeDisplay == SD_KBYTES) ? ID_TSMI_SIZE_KBYTES
                                                : ID_TSMI_SIZE_MBYTES
                                             , TRUE),
                                MPFROM2SHORT(MIA_CHECKED,
                                             MIA_CHECKED));

                        // check "Collect EAs"?
                        if (G_Settings.CollectEAs)
                            WinSendMsg(hPopupMenu, MM_SETITEMATTR,
                                       MPFROM2SHORT(ID_TSMI_COLLECTEAS,
                                           TRUE),
                                       MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
                        // check "Low priority"?
                        if (G_Settings.LowPriority)
                            WinSendMsg(hPopupMenu, MM_SETITEMATTR,
                                       MPFROM2SHORT(ID_TSMI_LOWPRTY,
                                           TRUE),
                                       MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
                    }
                }
                break;

                /**********************************************
                *                                             *
                *  Drag'n'drop                                *
                *                                             *
                **********************************************/

                /*
                    The container control translates the
                    PM drag'n'drop messages into container
                    notifications. This is barely documented,
                    but still helpful, because the cnr already
                    gives us container-specific structures
                    with these notification codes and draws
                    target emphasis automatically.

                    For us, two nofifications are important:

                    --  CN_DRAGOVER is sent while the user
                        is dragging stuff over the container;
                        the return value must indicate whether
                        this is a valid object that may be
                        dropped;
                    --  CN_DROP is sent after the user has
                        released the dragged object (i.e.
                        when it's dropped). We must then
                        react to read in the new directory.
                 */

                /*
                 * CN_DRAGOVER:
                 *      something's being dragged over the container.
                 *      Depending on the return value of this message,
                 *      PM will show the "not allowed" pointer or not.
                 *      Also, the cnr can automatically give itself
                 *      target emphasis.
                 *      Let's check if we're getting a WPS folder and,
                 *      if so, allow this.
                 */

                case CN_DRAGOVER:
                {
                    // container drag info structures
                    PCNRDRAGINFO pcdi = (PCNRDRAGINFO)mp2;
                    PDRAGITEM pdi;

                    // reset global variable for CN_DROP later
                    G_fDnDValid = FALSE;

                    // default return value: do _not_ allow dropping.
                    // This will result in the "forbidden" pointer
                    // being drawn by PM. NEVERDROP means that we
                    // will _never_ accept this object, and PM will
                    // stop sending messages for this object then.
                    mrc = MRFROM2SHORT(DOR_NEVERDROP,
                                       DO_DEFAULT);

                    if (G_tidCollect)
                        // accept nothing while Collect thread is working
                        break;
                    if (pcdi->pDragInfo->cditem != 1)
                        // accept no more than one single item at a time;
                        // we cannot handle more than one directory
                        break;
                    if (pcdi->pDragInfo->usOperation != DO_DEFAULT)
                    {
                        // we don't accept modifier keys (Shift, Ctrl, ...).
                        // But instead of NEVERDROP, return NODROP,
                        // which will keep sending msgs instead
                        // of stopping that
                        mrc = MRFROM2SHORT(DOR_NODROP, DO_DEFAULT);
                        break;
                    }
                    if (pcdi->pRecord)
                    {
                        // mouse is over a record core: no,
                        // we only accept drag on whitespace.
                        mrc = MRFROM2SHORT(DOR_NODROP, DO_DEFAULT);
                        break;
                    }

                    // Now that we've ruled out the most apparent
                    // errors, we need to check further:

                    // get access to the drag'n'drop structures
                    DrgAccessDraginfo(pcdi->pDragInfo);
                    // get the item being dragged (PDRAGITEM)
                    pdi = DrgQueryDragitemPtr(pcdi->pDragInfo, 0);

                    /*  From PMREF:

                        typedef struct _DRAGITEM {
                            HWND     hwndItem;
                              Window handle of the source of the drag operation.
                            ULONG    ulItemID;
                              Information used by the source to identify the object being dragged.
                            HSTR     hstrType;
                              String handle of the object type.
                            HSTR     hstrRMF;
                              String handle of the rendering mechanism and format.
                            HSTR     hstrContainerName;
                              String handle of the name of the container holding the source object.
                            HSTR     hstrSourceName;
                              String handle of the name of the source object.
                            HSTR     hstrTargetName;
                              String handle of the suggested name of the object at the target.
                            SHORT    cxOffset;
                              X-offset from the pointer hot spot to the origin of the image that represents this object.
                            SHORT    cyOffset;
                              Y-offset from the pointer hot spot to the origin of the image that represents this object.
                            USHORT   fsControl;
                              Source-object control flags.
                            USHORT   fsSupportedOps;
                              Direct manipulation operations supported by the source object.
                        } DRAGITEM;        */

                    if (pdi)
                    {
                        // now check render mechanism and format.
                        // The WPS gives us DRM_OS2FILE for "real" files only,
                        // but DRM_OBJECT for abstract objects.
                        // Moreover, the format is DRF_TEXT for all files,
                        // but folders have DRF_UNKNOWN instead.
                        // So by checking for DRM_OS2FILE and DRF_UNKNOWN,
                        // we should get folders only.
                        // (Unfortunately, this will _not_ work for root folders
                        // (drive objects), because these are rendered as
                        // DRM_OBJECT, and since we're not running in the WPS
                        // process, we cannot get the corresponding SOM object).
                        if (DrgVerifyRMF(pdi, "DRM_OS2FILE", "DRF_UNKNOWN"))
                        {
                            // it's a folder (directory):
                            // the following gives us the full path of where the
                            // dragged folder resides in. The PM d'n'd functions always
                            // use these strange "string handles", which we
                            // need to translate now.
                            DrgQueryStrName(pdi->hstrContainerName,
                                        sizeof(G_szDir), G_szDir);
                                // e.g. for "F:\OS2\APPS" this would be "F:\OS2\"

                            // this gives us the filename of the dragged folder
                            DrgQueryStrName(pdi->hstrSourceName,
                                        sizeof(G_szFile), G_szFile);
                                // e.g. for "F:\OS2\APPS" this would be "APPS"

                            // store flag for CN_DROP below
                            G_fDnDValid = TRUE;
                            // return "allow drop" code
                            mrc = MRFROM2SHORT(DOR_DROP,
                                               DO_UNKNOWN);
                                        // means application-specific;
                                        // that's cool for us, because we don't
                                        // really want the WPS to start copying
                                        // or moving anything
                        }
                    }

                    // clean up
                    DrgFreeDraginfo(pcdi->pDragInfo);
                }
                break;

                /*
                 * CN_DROP:
                 *      something _has_ now been dropped on the cnr.
                 */

                case CN_DROP:
                    // check the global variable which has been set
                    // by CN_DRAGOVER above:
                    if (G_fDnDValid)
                    {
                        // CN_DRAGOVER above has considered this valid:
                        // restart the whole gathering process
                        sprintf(G_szRootDir, "%s%s", G_szDir, G_szFile);
                        WinPostMsg(hwndDlg, TSM_START, NULL, NULL);
                        G_fDnDValid = FALSE;
                    }
                break;
            }
        }
        break;

        /*
         * WM_TIMER:
         *      timer for tree view auto-scroll
         */

        case WM_TIMER:
        {
            WinStopTimer(WinQueryAnchorBlock(hwndDlg),
                    hwndDlg,
                    (ULONG)mp1);
            if ( ( G_preccScrollTo->flRecordAttr & CRA_EXPANDED) != 0 )
            {
                PRECORDCORE     preccLastChild;
                // scroll the tree view properly
                preccLastChild = WinSendMsg(G_hwndCnr,
                                            CM_QUERYRECORD,
                                            G_preccScrollTo,   // expanded PRECORDCORE from CN_EXPANDTREE
                                            MPFROM2SHORT(
                                                 ( ((ULONG)mp1 == 1)
                                                    ? CMA_LASTCHILD : CMA_FIRSTCHILD),
                                                 CMA_ITEMORDER));
                if (preccLastChild)
                {
                    // ULONG ulrc;
                    cnrhScrollToRecord(G_hwndCnr,
                                       (PRECORDCORE)preccLastChild,
                                       CMA_TEXT,   // record text rectangle only
                                       TRUE);      // keep parent visible
                }
            }
        }
        break;

        /*
         * WM_ADJUSTWINDOWPOS:
         *      sent _while_ a window is being moved
         *      or resized.
         *      set minimum wnd size when resizing
         */

        case WM_ADJUSTWINDOWPOS:
        {
            PSWP pswp = PVOIDFROMMP(mp1);
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);

            // only if window is not minimized, or this
            // will produce really funny minimized windows
            if (    ((pswp->fl & (SWP_SIZE | SWP_MINIMIZE)) == SWP_SIZE)
                 && (!G_fMinimized)
               )
            {
                // resizing and not minimize: assert minimum cx/cy
                if (pswp->cx < 300)
                    pswp->cx = 300;
                if (pswp->cy < 300)
                    pswp->cy = 300;
            }
        }
        break;

        /*
         * WM_WINDOWPOSCHANGED:
         *      posted _after_ the window has been moved
         *      or resized.
         *      since we have a sizeable dlg, we need to
         *      update the controls' sizes also, or PM
         *      will display garbage. This is the trick
         *      how to use sizeable dlgs, because the do
         *      _not_ get sent WM_SIZE messages.
         */

        case WM_WINDOWPOSCHANGED:
        {
            // this msg is passed two SWP structs:
            // one for the old, one for the new data
            // (from PM docs)
            PSWP pswpNew = PVOIDFROMMP(mp1);
            PSWP pswpOld = pswpNew + 1;
            BOOL fWasMinimized = G_fMinimized;

            if (pswpNew->fl & SWP_MINIMIZE)
            {
                // minimizing: pmref says, we should
                // hide all controls, because otherwise
                // these get painted upon the minimized icon
                G_fMinimized = TRUE;
                WinShowWindow(G_hwndCnr, FALSE);
                WinShowWindow(G_hwndClose, FALSE);
                WinShowWindow(G_hwndIcon, FALSE);
                WinShowWindow(G_hwndClear, FALSE);
                WinShowWindow(G_hwndRefresh, FALSE);
                WinShowWindow(G_hwndText, FALSE);
            }
            else if (pswpNew->fl & SWP_RESTORE)
            {
                // un-minimize: do the reverse
                G_fMinimized = FALSE;
                WinShowWindow(G_hwndCnr, TRUE);
                WinShowWindow(G_hwndClose, TRUE);
                WinShowWindow(G_hwndIcon, TRUE);
                WinShowWindow(G_hwndClear, TRUE);
                WinShowWindow(G_hwndRefresh, TRUE);
                WinShowWindow(G_hwndText, TRUE);
            }

            // resizing and not minimize?
            if (    (pswpNew->fl & SWP_SIZE)
                &&  (!G_fMinimized)
                &&  (!fWasMinimized)
               )
            {
                SWP swpControl;

                // for each control in the dlg, we now adjust
                // size and/or position by determining the
                // difference between the old and new SWP structs.
                LONG        ldcx = (pswpNew->cx - pswpOld->cx),
                                    // width delta
                            ldcy = (pswpNew->cy - pswpOld->cy);
                                    // height delta

                // container: adjust width and height
                WinQueryWindowPos(G_hwndCnr,
                                  &swpControl);
                WinSetWindowPos(G_hwndCnr,
                                NULLHANDLE, 0, 0, // don't care
                                swpControl.cx + ldcx,
                                swpControl.cy + ldcy,
                                SWP_SIZE);

                // top text: adjust width and ypos
                WinQueryWindowPos(G_hwndText,
                                  &swpControl);
                WinSetWindowPos(G_hwndText,
                                NULLHANDLE,
                                swpControl.x,
                                swpControl.y + ldcy,
                                swpControl.cx + ldcx,
                                swpControl.cy,
                                SWP_MOVE | SWP_SIZE);

                // icon: adjust ypos only
                WinQueryWindowPos(G_hwndIcon,
                                  &swpControl);
                WinSetWindowPos(G_hwndIcon,
                                NULLHANDLE,
                                swpControl.x,
                                swpControl.y + ldcy,
                                0, 0,
                                SWP_MOVE);

                // "Clear" button:
                // adjust xpos only
                /* WinQueryWindowPos(G_hwndClear,
                                  &swpControl);
                WinSetWindowPos(G_hwndClear,
                                NULLHANDLE,
                                swpControl.x + ldcx,
                                swpControl.y,
                                0, 0,
                                SWP_MOVE); */
            }
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        }
        break;

        /*
         * WM_COMMAND:
         *      button or menu commands.
         */

        case WM_COMMAND:
        {
            switch ((ULONG)mp1)
            {
                case DID_CLEAR:
                    Cleanup(NULL);
                break;

                case DID_REFRESH:
                    Cleanup(NULL);
                    WinPostMsg(hwndDlg, TSM_START, 0, 0);
                break;

                case ID_TSMI_SORTBYNAME:
                    WinSendMsg(G_hwndCnr,
                               CM_SORTRECORD,
                               (MPARAM)fnCompareName,
                               NULL);
                    G_Settings.ulSort = SV_NAME;
                    SaveSettings();
                break;

                case ID_TSMI_SORTBYSIZE:
                    WinSendMsg(G_hwndCnr,
                                CM_SORTRECORD,
                                (MPARAM)fnCompareSize,
                                NULL);
                    G_Settings.ulSort = SV_SIZE;
                    SaveSettings();
                break;

                case ID_TSMI_SORTBYFILESCOUNT: // added V0.9.1
                    WinSendMsg(G_hwndCnr,
                               CM_SORTRECORD,
                               (MPARAM)fnCompareFilesCount,
                               NULL);
                    G_Settings.ulSort = SV_FILESCOUNT;
                    SaveSettings();
                break;

                case ID_TSMI_SORTBYEASIZE:
                    WinSendMsg(G_hwndCnr,
                               CM_SORTRECORD,
                               (MPARAM)fnCompareEASize,
                               NULL);
                    G_Settings.ulSort = SV_EASIZE;
                    SaveSettings();
                break;

                case ID_TSMI_SIZE_BYTES:
                    G_Settings.ulSizeDisplay = SD_BYTES;
                    SaveSettings();
                break;

                case ID_TSMI_SIZE_KBYTES:
                    G_Settings.ulSizeDisplay = SD_KBYTES;
                    SaveSettings();
                break;

                case ID_TSMI_SIZE_MBYTES:
                    G_Settings.ulSizeDisplay = SD_MBYTES;
                    SaveSettings();
                break;

                case ID_TSMI_LOWPRTY:
                    G_Settings.LowPriority = !(G_Settings.LowPriority);
                    SaveSettings();

                    if (G_tidCollect)
                        // collect thread running:
                        DosSetPriority(PRTYS_THREAD,
                                       (G_Settings.LowPriority) ? PRTYC_IDLETIME : PRTYC_REGULAR,
                                       0, G_tidCollect);
                break;

                case ID_TSMI_COLLECTEAS:
                    G_Settings.CollectEAs = !(G_Settings.CollectEAs);
                    SaveSettings();
                break;

                /*
                 * ID_TSMI_HELP:
                 *      contact XWP to display the help panel.
                 *      Finally added this with V0.9.16.
                 */

                case ID_TSMI_HELP:
                {
                     // tell xwp to display the help
                     PXWPGLOBALSHARED   pXwpGlobalShared = NULL;
                     if (!(DosGetNamedSharedMem((PVOID*)&pXwpGlobalShared,
                                                SHMEM_XWPGLOBAL,
                                                PAG_READ | PAG_WRITE)))
                     {
                         WinPostMsg(pXwpGlobalShared->hwndAPIObject,
                                    APIM_SHOWHELPPANEL,
                                    (MPARAM)ID_XSH_TREESIZE,
                                    0);
                         DosFreeMem(pXwpGlobalShared);
                     }
                }
                break;

                default:
                {
                    // this includes the "OK" button and
                    // the "Close" menu item:
                    // store window params (size, presparams, ...)
                    // Note that this function only works while the
                    // window is still visible, so we do this here.
                    G_fStopThread = TRUE;
                    WinStoreWindowPos((PSZ)G_pcszXFldTreesize, "WindowPos", hwndDlg);
                    Cleanup(NULL);
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                }
                break;

            }
        }
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }
    return mrc;
}

/*
 *@@ LoadNLS:
 *      Treesize NLS interface.
 *
 *@@added V0.9.0 [umoeller]
 */

BOOL LoadNLS(VOID)
{
    CHAR        szNLSDLL[2*CCHMAXPATH];
    BOOL Proceed = TRUE;

    if (PrfQueryProfileString(HINI_USER,
                              "XWorkplace",
                              "XFolderPath",
                              "",
                              szNLSDLL, sizeof(szNLSDLL)) < 3)

    {
        WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                      "Treesize was unable to determine the location of the "
                      "XWorkplace National Language Support DLL, which is "
                      "required for operation. The OS2.INI file does not contain "
                      "this information. "
                      "Treesize cannot proceed. Please re-install XWorkplace.",
                      "Treesize: Fatal Error",
                      0, MB_OK | MB_MOVEABLE);
        Proceed = FALSE;
    }
    else
    {
        CHAR    szLanguageCode[50] = "";

        // now compose module name from language code
        PrfQueryProfileString(HINI_USERPROFILE,
                              "XWorkplace", "Language",
                              "001",
                              (PVOID)szLanguageCode,
                              sizeof(szLanguageCode));

        // allow '?:\' for boot drive
        // V0.9.19 (2002-06-08) [umoeller]
        if (szNLSDLL[0] == '?')
        {
            ULONG ulBootDrive;
            DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                            &ulBootDrive,
                            sizeof(ulBootDrive));
            szNLSDLL[0] = (CHAR)ulBootDrive + 'A' - 1;
        }

        strcat(szNLSDLL, "\\bin\\xfldr");
        strcat(szNLSDLL, szLanguageCode);
        strcat(szNLSDLL, ".dll");

        // try to load the module
        if (DosLoadModule(NULL,
                          0,
                          szNLSDLL,
                          &G_hmodNLS))
        {
            CHAR    szMessage[2000];
            sprintf(szMessage,
                    "Treesize was unable to load \"%s\", "
                    "the National Language DLL which "
                    "is specified for XWorkplace in OS2.INI.",
                    szNLSDLL);
            WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                          szMessage,
                          "Treesize: Fatal Error",
                          0, MB_OK | MB_MOVEABLE);
            Proceed = FALSE;
        }

        _Pmpf(("DosLoadModule: 0x%lX", G_hmodNLS));
    }
    return Proceed;
}

/*
 * main:
 *      Program entry point. This loads the main wnd
 *      as a dlg resource. All the processing is done
 *      in fnwpMain above, which is running after this
 *      func has called WinProcessDlg.
 *
 *@@changed V1.0.4 (2005-02-24) [chennecke]: added nlsInitStrings() call to initialize the NLS string cache
 */

int main(int argc, char *argv[])
{
    HAB         hab;
    HMQ         hmq;
    FRAMECDATA  fcd;
    QMSG        qmsg;
    BOOL        Proceed = TRUE;
    ULONG       ul;
    CHAR        szTitle[400];

    if (!(hab = WinInitialize(0)))
        return FALSE;

    if (!(hmq = WinCreateMsgQueue(hab, 0)))
        return FALSE;

    winhInitGlobals();          // V1.0.1 (2002-11-30) [umoeller]

    // check command line parameters: in C,
    // the first parameter is always the full
    // path of the TREESIZE executable.
    if (argc == 1)
    {
        // one param only: no directory given,
        // use current dir then.
        ULONG ulCurrentDisk = 0, ulDriveMap = 0;
        DosQueryCurrentDisk(&ulCurrentDisk, &ulDriveMap);
        G_szRootDir[0] = 'A'+ulCurrentDisk-1;
        G_szRootDir[1] = ':';
        G_szRootDir[2] = '\\';
        DosQueryCurrentDir(0, G_szRootDir+3, &G_cbRootDir);
    }
    else
        // param given: use this
        strcpy(G_szRootDir, argv[1]);

    // now attempt to find the XWorkplace NLS resource DLL,
    // which we need for all resources (new with XWP 0.9.0)
    Proceed = LoadNLS();

    if (Proceed)
    {
        ULONG       cbSettings = sizeof(G_Settings);
        SWCNTRL     swctl;
        PID         pid;
        HSWITCH     hswitch;
        HPOINTER    hIcon = WinLoadPointer(HWND_DESKTOP,
                                           G_hmodNLS,
                                           ID_TS_ICON);

        // this loads the main window. All the processing
        // is done in fnwpMain.
        G_hwndMain = WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP,
                                fnwpMain,
                                G_hmodNLS, ID_TSD_MAIN,
                                G_szRootDir);

        if (!G_hwndMain)
            WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                          "The main window could not be found in the XWorkplace NLS DLL. "
                          "Probably the DLL is outdated. Terminating.",
                          "Treesize: Fatal Error",
                          0, MB_OK | MB_MOVEABLE);

        // set clip children flag
        WinSetWindowBits(G_hwndMain,
                         QWL_STYLE,
                         WS_CLIPCHILDREN,         // unset bit
                         WS_CLIPCHILDREN);

        // append path to title
        WinQueryWindowText(G_hwndMain, sizeof(szTitle), szTitle);
        // strcat(szTitle, " V" BLDLEVEL_VERSION);
                // removed version number V0.9.19 (2002-05-04) [umoeller]
        sprintf(szTitle + strlen(szTitle), " - %s", G_szRootDir);
        WinSetWindowText(G_hwndMain, szTitle);

        // give the dlg an icon
        WinSendMsg(G_hwndMain,
                   WM_SETICON,
                   (MPARAM)hIcon,
                   NULL);

        // fix the icon V0.9.16 (2001-12-02) [umoeller]
        ctlPrepareStaticIcon(G_hwndIcon,
                             1);
        WinSendMsg(G_hwndIcon,
                   SM_SETHANDLE,
                   (MPARAM)hIcon,
                   NULL);

        // add ourselves to the tasklist
        swctl.hwnd = G_hwndMain;                 // window handle
        swctl.hwndIcon = hIcon;                // icon handle
        swctl.hprog = NULLHANDLE;              // program handle
        WinQueryWindowProcess(G_hwndMain, &(swctl.idProcess), NULL);
                                               // process identifier
        swctl.idSession = 0;                   // session identifier ?
        swctl.uchVisibility = SWL_VISIBLE;     // visibility
        swctl.fbJump = SWL_JUMPABLE;           // jump indicator
        WinQueryWindowText(G_hwndMain, sizeof(swctl.szSwtitle), (PSZ)&swctl.szSwtitle);
        swctl.bProgType = PROG_DEFAULT;        // program type

        hswitch = WinAddSwitchEntry(&swctl);

        // get settings in OS2.INI
        WinRestoreWindowPos((PSZ)G_pcszXFldTreesize,
                            "WindowPos",
                            G_hwndMain);
        G_Settings.ulSort = SV_SIZE;
        G_Settings.CollectEAs = TRUE;
        G_Settings.ulSizeDisplay = SD_KBYTES;
        G_Settings.LowPriority = TRUE;
        PrfQueryProfileData(HINI_USER,
                            (PSZ)G_pcszXFldTreesize,
                            "Settings",
                            &G_Settings,
                            &cbSettings);

        // initialize NLS cache
        nlsInitStrings(hab, G_hmodNLS, NULL, 0);

        // go!
        WinShowWindow(G_hwndMain, TRUE);
        WinProcessDlg(G_hwndMain);
        winhDestroyWindow(&G_hwndMain);
    } // end if (proceed)

    if (G_tidCollect)
    {
        // give the Collect thread 5 seconds to exit;
        // normally, this shouldn't be a problem
        G_fStopThread = TRUE;
        for (ul = 0; ul < 50; ul++)
            if (G_tidCollect == 0)
                break;
            else
                DosSleep(100);  // 50*100 ms = 5 seconds
    }

    // clean up on the way out
    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);

    return TRUE;
}

