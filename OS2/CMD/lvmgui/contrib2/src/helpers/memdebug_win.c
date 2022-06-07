
/*
 *@@sourcefile memdebug_win.c:
 *      implementation of the PM heap debugging window,
 *      if the __XWPMEMDEBUG__ #define is set in setup.h.
 *
 *      This has been extracted from memdebug.c to allow
 *      linking with non-PM programs.
 *
 *@@added V0.9.6 (2000-11-12) [umoeller]
 */

/*
 *      Copyright (C) 2000-2005 Ulrich M”ller.
 *      This program is part of the XWorkplace package.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINCOUNTRY
#define INCL_WINSYS
#define INCL_WINMENUS
#define INCL_WINSTDCNR
#include <os2.h>

#include <stdio.h>
#include <string.h>
// #include <malloc.h>
#include <setjmp.h>

#include "helpers\tree.h"

#define DONT_REPLACE_MALLOC             // never do debug memory for this
#define MEMDEBUG_PRIVATE
#include "setup.h"

#ifdef __XWPMEMDEBUG__

#include "helpers\cnrh.h"
#include "helpers\except.h"
// #include "helpers\memdebug.h"        // included by setup.h already
#include "helpers\nls.h"
#include "helpers\stringh.h"
#include "helpers\winh.h"

/*
 *@@category: Helpers\C helpers\Heap debugging
 */

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

PSZ             G_pszMemCnrTitle = NULL;
HWND            G_hwndMemDebugMenu = NULLHANDLE;

/* ******************************************************************
 *
 *   Heap debugging window
 *
 ********************************************************************/

/*
 *@@ MEMRECORD:
 *
 *@@added V0.9.1 (99-12-04) [umoeller]
 */

typedef struct _MEMRECORD
{
    RECORDCORE  recc;

    ULONG       ulIndex;

    CDATE       cdateAllocated;
    CTIME       ctimeAllocated;

    PSZ         pszFreed;

    ULONG       ulTID;

    PSZ         pszFunction;    // points to szFunction
    CHAR        szFunction[400];

    PSZ         pszSource;      // points to szSource
    CHAR        szSource[CCHMAXPATH];

    ULONG       ulLine;

    ULONG       ulAddress;      // has pAfterMagic, invisible, only
                                // for sorting V0.9.14 (2001-08-01) [umoeller]

    PSZ         pszAddress;     // points to szAddress
    CHAR        szAddress[20];

    ULONG       ulSize;

} MEMRECORD, *PMEMRECORD;

/* ULONG       ulHeapItemsCount1,
            ulHeapItemsCount2;
ULONG       ulTotalAllocated,
            ulTotalFreed;
PMEMRECORD  pMemRecordThis = NULL;
PSZ         pszMemCnrTitle = NULL; */

#if 0
    /*
     *@@ fncbMemHeapWalkCount:
     *      callback func for _heap_walk function used for
     *      fnwpMemDebug.
     *
     *@@added V0.9.1 (99-12-04) [umoeller]
     */

    int fncbMemHeapWalkCount(const void *pObject,
                             size_t Size,
                             int useflag,
                             int status,
                             const char *filename,
                             size_t line)
    {
        // skip all the items which seem to be
        // internal to the runtime
        if ((filename) || (useflag == _FREEENTRY))
        {
            ulHeapItemsCount1++;
            if (useflag == _FREEENTRY)
                ulTotalFreed += Size;
            else
                ulTotalAllocated += Size;
        }
        return 0;
    }

    /*
     *@@ fncbMemHeapWalkFill:
     *      callback func for _heap_walk function used for
     *      fnwpMemDebug.
     *
     *@@added V0.9.1 (99-12-04) [umoeller]
     */

    int fncbMemHeapWalkFill(const void *pObject,
                            size_t Size,
                            int useflag,
                            int status,
                            const char *filename,
                            size_t line)
    {
        // skip all the items which seem to be
        // internal to the runtime
        if ((filename) || (useflag == _FREEENTRY))
        {
            ulHeapItemsCount2++;
            if ((pMemRecordThis) && (ulHeapItemsCount2 < ulHeapItemsCount1))
            {
                pMemRecordThis->ulIndex = ulHeapItemsCount2 - 1;

                pMemRecordThis->pObject = pObject;
                pMemRecordThis->useflag = useflag;
                pMemRecordThis->status = status;
                pMemRecordThis->filename = filename;

                pMemRecordThis->pszAddress = pMemRecordThis->szAddress;

                pMemRecordThis->ulSize = Size;

                pMemRecordThis->pszSource = pMemRecordThis->szSource;

                pMemRecordThis->ulLine = line;

                pMemRecordThis = (PMEMRECORD)pMemRecordThis->recc.preccNextRecord;
            }
            else
                return 1;     // stop
        }

        return 0;
    }

    /*
     *@@ CreateRecordsVAC:
     *
     *@@added V0.9.3 (2000-04-10) [umoeller]
     */

    VOID CreateRecordsVAC(HWND hwndCnr)
    {
        // count heap items
        ulHeapItemsCount1 = 0;
        ulTotalFreed = 0;
        ulTotalAllocated = 0;
        _heap_walk(fncbMemHeapWalkCount);

        pMemRecordFirst = (PMEMRECORD)cnrhAllocRecords(hwndCnr,
                                                       sizeof(MEMRECORD),
                                                       ulHeapItemsCount1);
        if (pMemRecordFirst)
        {
            ulHeapItemsCount2 = 0;
            pMemRecordThis = pMemRecordFirst;
            _heap_walk(fncbMemHeapWalkFill);

            // the following doesn't work while _heap_walk is running
            pMemRecordThis = pMemRecordFirst;
            while (pMemRecordThis)
            {
                switch (pMemRecordThis->useflag)
                {
                    case _USEDENTRY: pMemRecordThis->pszUseFlag = "Used"; break;
                    case _FREEENTRY: pMemRecordThis->pszUseFlag = "Freed"; break;
                }

                switch (pMemRecordThis->status)
                {
                    case _HEAPBADBEGIN: pMemRecordThis->pszStatus = "heap bad begin"; break;
                    case _HEAPBADNODE: pMemRecordThis->pszStatus = "heap bad node"; break;
                    case _HEAPEMPTY: pMemRecordThis->pszStatus = "heap empty"; break;
                    case _HEAPOK: pMemRecordThis->pszStatus = "OK"; break;
                }

                pMemRecordThis->ulAddress = (ULONG)pMemRecordThis->pObject;

                sprintf(pMemRecordThis->szAddress,
                        "0x%lX",
                        pMemRecordThis->pObject);
                strhcpy(pMemRecordThis->szSource,
                        (pMemRecordThis->filename)
                            ? pMemRecordThis->filename
                            : "?");

                pMemRecordThis = (PMEMRECORD)pMemRecordThis->recc.preccNextRecord;
            }

            cnrhInsertRecords(hwndCnr,
                              NULL,         // parent
                              (PRECORDCORE)pMemRecordFirst,
                              TRUE,
                              NULL,
                              CRA_RECORDREADONLY,
                              ulHeapItemsCount2);
        }
    }

#endif

/*
 *@@ CreateRecords:
 *
 *@@added V0.9.3 (2000-04-10) [umoeller]
 */

STATIC VOID CreateRecords(HWND hwndCnr,
                          PULONG pulTotalItems,
                          PULONG pulAllocatedItems,
                          PULONG pulFreedItems,
                          PULONG pulTotalBytes,
                          PULONG pulAllocatedBytes,
                          PULONG pulFreedBytes)
{
    // count heap items
    // ULONG       ulHeapItemsCount1 = 0;
    PMEMRECORD  pMemRecordFirst;

    ULONG       cHeapItems = 0;
    volatile BOOL       fLocked = FALSE; // XWP V1.0.4 (2005-10-09) [pr]

    TRY_LOUD(excpt1)
    {
        if (fLocked = memdLock())
        {
            PHEAPITEM pHeapItem = (PHEAPITEM)treeFirst(G_pHeapItemsRoot);

            *pulTotalItems = 0;
            *pulAllocatedItems = 0;
            *pulFreedItems = 0;

            *pulTotalBytes = 0;
            *pulAllocatedBytes = 0;
            *pulFreedBytes = 0;

            *pulTotalItems = G_cHeapItems;

            if (pMemRecordFirst = (PMEMRECORD)cnrhAllocRecords(hwndCnr,
                                                               sizeof(MEMRECORD),
                                                               G_cHeapItems))
            {
                PMEMRECORD  pMemRecordThis = pMemRecordFirst;
                pHeapItem = (PHEAPITEM)treeFirst(G_pHeapItemsRoot);

                while ((pMemRecordThis) && (pHeapItem))
                {
                    if (pHeapItem->fFreed)
                    {
                        (*pulFreedItems)++;
                        (*pulFreedBytes) += pHeapItem->ulSize;
                    }
                    else
                    {
                        (*pulAllocatedItems)++;
                        (*pulAllocatedBytes) += pHeapItem->ulSize;
                    }

                    (*pulTotalBytes) += pHeapItem->ulSize;

                    pMemRecordThis->ulIndex = cHeapItems++;

                    cnrhDateTimeDos2Win(&pHeapItem->dtAllocated,
                                        &pMemRecordThis->cdateAllocated,
                                        &pMemRecordThis->ctimeAllocated);

                    if (pHeapItem->fFreed)
                        pMemRecordThis->pszFreed = "yes";

                    pMemRecordThis->ulTID = pHeapItem->ulTID;

                    strhcpy(pMemRecordThis->szSource, pHeapItem->pcszSourceFile);
                    pMemRecordThis->pszSource = pMemRecordThis->szSource;

                    pMemRecordThis->ulLine = pHeapItem->ulLine;

                    strhcpy(pMemRecordThis->szFunction, pHeapItem->pcszFunction);
                    pMemRecordThis->pszFunction = pMemRecordThis->szFunction;

                    pMemRecordThis->ulAddress = pHeapItem->Tree.ulKey;

                    sprintf(pMemRecordThis->szAddress,
                            "0x%lX",
                            pHeapItem->Tree.ulKey);
                    pMemRecordThis->pszAddress = pMemRecordThis->szAddress;

                    pMemRecordThis->ulSize = pHeapItem->ulSize;


                    /* switch (pMemRecordThis->useflag)
                    {
                        case _USEDENTRY: pMemRecordThis->pszUseFlag = "Used"; break;
                        case _FREEENTRY: pMemRecordThis->pszUseFlag = "Freed"; break;
                    }

                    switch (pMemRecordThis->status)
                    {
                        case _HEAPBADBEGIN: pMemRecordThis->pszStatus = "heap bad begin"; break;
                        case _HEAPBADNODE: pMemRecordThis->pszStatus = "heap bad node"; break;
                        case _HEAPEMPTY: pMemRecordThis->pszStatus = "heap empty"; break;
                        case _HEAPOK: pMemRecordThis->pszStatus = "OK"; break;
                    }

                    sprintf(pMemRecordThis->szAddress, "0x%lX", pMemRecordThis->pObject);
                    strhcpy(pMemRecordThis->szSource,
                            (pMemRecordThis->filename)
                                ? pMemRecordThis->filename
                                : "?"); */

                    pMemRecordThis = (PMEMRECORD)pMemRecordThis->recc.preccNextRecord;
                    pHeapItem = (PHEAPITEM)treeNext((TREE*)pHeapItem);
                }

                cnrhInsertRecords(hwndCnr,
                                  NULL,         // parent
                                  (PRECORDCORE)pMemRecordFirst,
                                  TRUE,
                                  NULL,
                                  CRA_RECORDREADONLY,
                                  cHeapItems);
            }
        }
    }
    CATCH(excpt1)
    {
        if (G_pMemdLogFunc)
        {
            CHAR        szMsg[1000];
            sprintf(szMsg,
                    "Crash occurred at object #%d out of %d",
                    cHeapItems,
                    G_cHeapItems);
            G_pMemdLogFunc(szMsg);
        }
    } END_CATCH();

    if (fLocked)
        memdUnlock();
}

/*
 *@@ mnu_fnCompareIndex:
 *
 *@@added V0.9.1 (99-12-03) [umoeller]
 */

STATIC SHORT EXPENTRY mnu_fnCompareIndex(PMEMRECORD pmrc1, PMEMRECORD  pmrc2, PVOID pStorage)
{
    pStorage = pStorage; // to keep the compiler happy
    if ((pmrc1) && (pmrc2))
        if (pmrc1->ulIndex < pmrc2->ulIndex)
            return -1;
        else if (pmrc1->ulIndex > pmrc2->ulIndex)
            return 1;

    return 0;
}

/*
 *@@ mnu_fnCompareSourceFile:
 *
 *@@added V0.9.1 (99-12-03) [umoeller]
 */

STATIC SHORT EXPENTRY mnu_fnCompareSourceFile(PMEMRECORD pmrc1, PMEMRECORD  pmrc2, PVOID pStorage)
{
    HAB habDesktop = WinQueryAnchorBlock(HWND_DESKTOP);
    pStorage = pStorage; // to keep the compiler happy
    if ((pmrc1) && (pmrc2))
            switch (WinCompareStrings(habDesktop, 0, 0,
                                      pmrc1->szSource,
                                      pmrc2->szSource,
                                      0))
            {
                case WCS_LT: return -1;
                case WCS_GT: return 1;
                default:    // equal
                    if (pmrc1->ulLine < pmrc2->ulLine)
                        return -1;
                    else if (pmrc1->ulLine > pmrc2->ulLine)
                        return 1;

            }

    return 0;
}

/*
 *@@ mnu_fnCompareSourceFile:
 *
 *@@added V0.9.6 (2000-11-12) [umoeller]
 */

STATIC SHORT EXPENTRY mnu_fnCompareSize(PMEMRECORD pmrc1, PMEMRECORD  pmrc2, PVOID pStorage)
{
    pStorage = pStorage; // to keep the compiler happy
    if ((pmrc1) && (pmrc2))
    {
        if (pmrc1->ulSize > pmrc2->ulSize)
            return 1;
        else if (pmrc1->ulSize < pmrc2->ulSize)
            return -1;
    }

    return 0;
}

/*
 *@@ mnu_fnCompareAddress:
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

STATIC SHORT EXPENTRY mnu_fnCompareAddress(PMEMRECORD pmrc1, PMEMRECORD  pmrc2, PVOID pStorage)
{
    pStorage = pStorage; // to keep the compiler happy
    if ((pmrc1) && (pmrc2))
    {
        if (pmrc1->ulAddress > pmrc2->ulAddress)
            return 1;
        else if (pmrc1->ulAddress < pmrc2->ulAddress)
            return -1;
    }

    return 0;
}

#define ID_MEMCNR   1000

/*
 *@@ memd_fnwpMemDebug:
 *      client window proc for the heap debugger window
 *      accessible from the Desktop context menu if
 *      __XWPMEMDEBUG__ is defined. Otherwise, this is not
 *      compiled.
 *
 *      Usage: this is a regular PM client window procedure
 *      to be used with WinRegisterClass and WinCreateStdWindow.
 *      See dtpMenuItemSelected, which uses this.
 *
 *      This creates a container with all the memory objects
 *      with the size of the client area in turn.
 *
 *@@added V0.9.1 (99-12-04) [umoeller]
 *@@changed V0.9.14 (2001-08-01) [umoeller]: added sort by address
 */

MRESULT EXPENTRY memd_fnwpMemDebug(HWND hwndClient, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_CREATE:
        {
            TRY_LOUD(excpt1)
            {
                // PCREATESTRUCT pcs = (PCREATESTRUCT)mp2;
                HWND hwndCnr;
                hwndCnr = WinCreateWindow(hwndClient,        // parent
                                          WC_CONTAINER,
                                          "",
                                          WS_VISIBLE | CCS_MINIICONS | CCS_READONLY | CCS_SINGLESEL,
                                          0, 0, 0, 0,
                                          hwndClient,        // owner
                                          HWND_TOP,
                                          ID_MEMCNR,
                                          NULL, NULL);
                if (hwndCnr)
                {
                    XFIELDINFO      xfi[11];
                    PFIELDINFO      pfi = NULL;
                    PMEMRECORD      pMemRecordFirst;
                    int             i = 0;

                    ULONG           ulTotalItems = 0,
                                    ulAllocatedItems = 0,
                                    ulFreedItems = 0;
                    ULONG           ulTotalBytes = 0,
                                    ulAllocatedBytes = 0,
                                    ulFreedBytes = 0;

                    // set up cnr details view
                    xfi[i].ulFieldOffset = FIELDOFFSET(MEMRECORD, ulIndex);
                    xfi[i].pszColumnTitle = "No.";
                    xfi[i].ulDataType = CFA_ULONG;
                    xfi[i++].ulOrientation = CFA_RIGHT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(MEMRECORD, cdateAllocated);
                    xfi[i].pszColumnTitle = "Date";
                    xfi[i].ulDataType = CFA_DATE;
                    xfi[i++].ulOrientation = CFA_LEFT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(MEMRECORD, ctimeAllocated);
                    xfi[i].pszColumnTitle = "Time";
                    xfi[i].ulDataType = CFA_TIME;
                    xfi[i++].ulOrientation = CFA_LEFT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(MEMRECORD, pszFreed);
                    xfi[i].pszColumnTitle = "Freed";
                    xfi[i].ulDataType = CFA_STRING;
                    xfi[i++].ulOrientation = CFA_CENTER;

                    xfi[i].ulFieldOffset = FIELDOFFSET(MEMRECORD, ulTID);
                    xfi[i].pszColumnTitle = "TID";
                    xfi[i].ulDataType = CFA_ULONG;
                    xfi[i++].ulOrientation = CFA_RIGHT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(MEMRECORD, pszFunction);
                    xfi[i].pszColumnTitle = "Function";
                    xfi[i].ulDataType = CFA_STRING;
                    xfi[i++].ulOrientation = CFA_LEFT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(MEMRECORD, pszSource);
                    xfi[i].pszColumnTitle = "Source";
                    xfi[i].ulDataType = CFA_STRING;
                    xfi[i++].ulOrientation = CFA_LEFT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(MEMRECORD, ulLine);
                    xfi[i].pszColumnTitle = "Line";
                    xfi[i].ulDataType = CFA_ULONG;
                    xfi[i++].ulOrientation = CFA_RIGHT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(MEMRECORD, ulSize);
                    xfi[i].pszColumnTitle = "Size";
                    xfi[i].ulDataType = CFA_ULONG;
                    xfi[i++].ulOrientation = CFA_RIGHT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(MEMRECORD, pszAddress);
                    xfi[i].pszColumnTitle = "Address";
                    xfi[i].ulDataType = CFA_STRING;
                    xfi[i++].ulOrientation = CFA_LEFT;

                    pfi = cnrhSetFieldInfos(hwndCnr,
                                            &xfi[0],
                                            i,             // array item count
                                            TRUE,          // no draw lines
                                            3);            // return column

                    {
                        PSZ pszFont = "9.WarpSans";
                        WinSetPresParam(hwndCnr,
                                        PP_FONTNAMESIZE,
                                        strlen(pszFont),
                                        pszFont);
                    }

                    CreateRecords(hwndCnr,
                                  &ulTotalItems,
                                  &ulAllocatedItems,
                                  &ulFreedItems,
                                  &ulTotalBytes,
                                  &ulAllocatedBytes,
                                  &ulFreedBytes);

                    BEGIN_CNRINFO()
                    {
                        CHAR    szCnrTitle[1000];
                        CHAR    szTotalItems[100],
                                szAllocatedItems[100],
                                szFreedItems[100],
                                szReleasedItems[100];
                        CHAR    szTotalBytes[100],
                                szAllocatedBytes[100],
                                szFreedBytes[100],
                                szReleasedBytes[100];
                        sprintf(szCnrTitle,
                                "Total memory log entries in use: %s items = %s bytes\n"
                                "    Total memory logs allocated: %s items = %s bytes\n"
                                "    Total memory logs freed: %s items = %s bytes\n"
                                "Total memory logs freed and discarded: %s items = %s bytes",
                                nlsThousandsDouble(szTotalItems,
                                                    ulTotalItems,
                                                    '.'),
                                nlsThousandsDouble(szTotalBytes,
                                                    ulTotalBytes,
                                                    '.'),
                                nlsThousandsDouble(szAllocatedItems,
                                                    ulAllocatedItems,
                                                    '.'),
                                nlsThousandsDouble(szAllocatedBytes,
                                                    ulAllocatedBytes,
                                                    '.'),
                                nlsThousandsDouble(szFreedItems,
                                                    ulFreedItems,
                                                    '.'),
                                nlsThousandsDouble(szFreedBytes,
                                                    ulFreedBytes,
                                                    '.'),
                                nlsThousandsDouble(szReleasedItems,
                                                    G_ulItemsReleased,
                                                    '.'),
                                nlsThousandsDouble(szReleasedBytes,
                                                    G_ulBytesReleased,
                                                    '.'));
                        G_pszMemCnrTitle = strdup(szCnrTitle);
                        cnrhSetTitle(G_pszMemCnrTitle);
                        cnrhSetView(CV_DETAIL | CV_MINI | CA_DETAILSVIEWTITLES
                                        | CA_DRAWICON
                                    | CA_CONTAINERTITLE | CA_TITLEREADONLY
                                        | CA_TITLESEPARATOR | CA_TITLELEFT);
                        cnrhSetSplitBarAfter(pfi);
                        cnrhSetSplitBarPos(250);
                    } END_CNRINFO(hwndCnr);

                    // create menu
                    G_hwndMemDebugMenu = WinCreateMenu(HWND_DESKTOP,
                                                       NULL); // no menu template

                    winhInsertMenuItem(G_hwndMemDebugMenu,
                                       MIT_END,
                                       1001,
                                       "Sort by index",
                                       MIS_TEXT, 0);
                    winhInsertMenuItem(G_hwndMemDebugMenu,
                                       MIT_END,
                                       1002,
                                       "Sort by source file",
                                       MIS_TEXT, 0);
                    winhInsertMenuItem(G_hwndMemDebugMenu,
                                       MIT_END,
                                       1003,
                                       "Sort by object size",
                                       MIS_TEXT, 0);
                    winhInsertMenuItem(G_hwndMemDebugMenu,
                                       MIT_END,
                                       1004,
                                       "Sort by address",
                                       MIS_TEXT, 0);

                    WinSetFocus(HWND_DESKTOP, hwndCnr);
                }
            }
            CATCH(excpt1) {} END_CATCH();

            mrc = WinDefWindowProc(hwndClient, msg, mp1, mp2);
        break; }

        case WM_WINDOWPOSCHANGED:
        {
            PSWP pswp = (PSWP)mp1;
            mrc = WinDefWindowProc(hwndClient, msg, mp1, mp2);
            if (pswp->fl & SWP_SIZE)
            {
                WinSetWindowPos(WinWindowFromID(hwndClient, ID_MEMCNR), // cnr
                                HWND_TOP,
                                0, 0, pswp->cx, pswp->cy,
                                SWP_SIZE | SWP_MOVE | SWP_SHOW);
            }
        break; }

        case WM_CONTROL:
        {
            USHORT usItemID = SHORT1FROMMP(mp1),
                   usNotifyCode = SHORT2FROMMP(mp1);
            if (usItemID == ID_MEMCNR)       // cnr
            {
                switch (usNotifyCode)
                {
                    case CN_CONTEXTMENU:
                    {
                        PMEMRECORD precc = (PMEMRECORD)mp2;
                        if (precc == NULL)
                        {
                            // whitespace:
                            cnrhShowContextMenu(WinWindowFromID(hwndClient, ID_MEMCNR),
                                                NULL,       // record
                                                G_hwndMemDebugMenu,
                                                hwndClient);
                        }
                    }
                }
            }
        break; }

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case 1001:  // sort by index
                    WinSendMsg(WinWindowFromID(hwndClient, ID_MEMCNR),
                               CM_SORTRECORD,
                               (MPARAM)mnu_fnCompareIndex,
                               0);
                break;

                case 1002:  // sort by source file
                    WinSendMsg(WinWindowFromID(hwndClient, ID_MEMCNR),
                               CM_SORTRECORD,
                               (MPARAM)mnu_fnCompareSourceFile,
                               0);
                break;

                case 1003: // sort by object size
                    WinSendMsg(WinWindowFromID(hwndClient, ID_MEMCNR),
                               CM_SORTRECORD,
                               (MPARAM)mnu_fnCompareSize,
                               0);
                break;

                case 1004: // sort by address V0.9.14 (2001-08-01) [umoeller]
                    WinSendMsg(WinWindowFromID(hwndClient, ID_MEMCNR),
                               CM_SORTRECORD,
                               (MPARAM)mnu_fnCompareAddress,
                               0);
                break;
            }
        break;

        case WM_CLOSE:
            WinDestroyWindow(WinWindowFromID(hwndClient, ID_MEMCNR));
            WinDestroyWindow(WinQueryWindow(hwndClient, QW_PARENT));
            free(G_pszMemCnrTitle);
            WinDestroyWindow(G_hwndMemDebugMenu);
            G_pszMemCnrTitle = NULL;
        break;

        default:
            mrc = WinDefWindowProc(hwndClient, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ memdCreateMemDebugWindow:
 *      creates a heap debugging window which
 *      is a standard frame with a container,
 *      listing all heap objects ever allocated.
 *
 *      The client of this standard frame is in
 *      memd_fnwpMemDebug.
 *
 *      This thing lists all calls to malloc()
 *      which were ever made, including the
 *      source file and source line number which
 *      made the call. Extreeeemely useful for
 *      detecting memory leaks.
 *
 *      This only works if the memory functions
 *      have been replaced with the debug versions
 *      in this file.
 */

VOID memdCreateMemDebugWindow(VOID)
{
    ULONG flStyle = FCF_TITLEBAR | FCF_SYSMENU | FCF_HIDEMAX
                    | FCF_SIZEBORDER | FCF_SHELLPOSITION
                    | FCF_NOBYTEALIGN | FCF_TASKLIST;
    if (WinRegisterClass(WinQueryAnchorBlock(HWND_DESKTOP),
                         "XWPMemDebug",
                         memd_fnwpMemDebug, 0L, 0))
    {
        HWND hwndClient;
        HWND hwndMemFrame = WinCreateStdWindow(HWND_DESKTOP,
                                               0L,
                                               &flStyle,
                                               "XWPMemDebug",
                                               "Allocated XWorkplace Memory Objects",
                                               0L,
                                               NULLHANDLE,     // resource
                                               0,
                                               &hwndClient);
        if (hwndMemFrame)
        {
            WinSetWindowPos(hwndMemFrame,
                            HWND_TOP,
                            0, 0, 0, 0,
                            SWP_ZORDER | SWP_SHOW | SWP_ACTIVATE);
        }
    }
}

#else
void memdDummy2(void)
{
    int i = 0;
    i++;
}
#endif // __XWPMEMDEBUG__

