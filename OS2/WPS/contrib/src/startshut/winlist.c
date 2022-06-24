
/*
 *@@sourcefile winlist.c:
 *      debug code to show the complete window (switch) list in
 *      a PM container window.
 *
 *@@header "startshut\shutdown.h"
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
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

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINSWITCHLIST
#define INCL_WINMENUS
#define INCL_WINSTDCNR
#define INCL_WINSYS
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <stdarg.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\cnrh.h"
#include "helpers\except.h"
#include "helpers\stringh.h"
#include "helpers\winh.h"
#include "helpers\xwpsecty.h"           // XWorkplace Security base

// SOM headers which don't crash with prec. header files
#include "xfobj.ih"

// XWorkplace implementation headers
#include "shared\kernel.h"              // XWorkplace Kernel
#include "startshut\shutdown.h"         // XWorkplace eXtended Shutdown

// other SOM headers
#pragma hdrstop

/*
 *@@ WINLISTRECORD:
 *
 *@@added V0.9.4 (2000-07-15) [umoeller]
 */

typedef struct _WINLISTRECORD
{
    RECORDCORE  recc;

    ULONG       ulIndex;

    HSWITCH     hSwitch;

    CHAR        szSwTitle[200];
    PSZ         pszSwTitle;         // points to szSwTitle

    CHAR        szWinTitle[200];
    PSZ         pszWinTitle;        // points to szWinTitle

    CHAR        szObject[30];
    PSZ         pszObject;

    HWND        hwnd;
    CHAR        szHWND[20];
    PSZ         pszHWND;

    CHAR        szWinClass[30];
    PSZ         pszWinClass;

    HPROGRAM    hPgm;
    PID         pid;
    ULONG       sid;
    ULONG       ulVisibility;

    PSZ         pszObjectTitle;

    HAPP        happObject;
    CHAR        szHAPP[30];
    PSZ         pszHAPP;            // points to szHAPP

    CHAR        szClosable[30];
    PSZ         pszClosable;        // points to szClosable

    XWPSECID    uid;                // V0.9.19 (2002-04-02) [umoeller]
    CHAR        szUid[30];
    PSZ         pszUid;             // points to szUid;

} WINLISTRECORD, *PWINLISTRECORD;

#define ID_WINLISTCNR       1001

/*
 *@@ winlCreateRecords:
 *
 *@@added V0.9.4 (2000-07-15) [umoeller]
 */

ULONG winlCreateRecords(HWND hwndCnr)
{
    PSWBLOCK        pSwBlock   = NULL;         // Pointer to information returned
    ULONG           ul; /*
                    cbItems    = 0,            // Number of items in list
                    ulBufSize  = 0;            // Size of buffer for information
       */
    HAB             hab = WinQueryAnchorBlock(hwndCnr);

    PWINLISTRECORD  pFirstRec = NULL,
                    precThis = NULL;
    ULONG           ulIndex = 0;

    SHUTDOWNCONSTS  SDConsts;

    // get all the tasklist entries into a buffer
    /* cbItems = WinQuerySwitchList(hab, NULL, 0);
    ulBufSize = (cbItems * sizeof(SWENTRY)) + sizeof(HSWITCH);
    pSwBlock = (PSWBLOCK)malloc(ulBufSize);
    cbItems = WinQuerySwitchList(hab, pSwBlock, ulBufSize); */

    if (pSwBlock = winhQuerySwitchList(hab))
    {
        xsdGetShutdownConsts(&SDConsts);

        // allocate records
        pFirstRec = (PWINLISTRECORD)cnrhAllocRecords(hwndCnr,
                                                     sizeof(WINLISTRECORD),
                                                     pSwBlock->cswentry);
        if (pFirstRec)
        {
            precThis = pFirstRec;

            // loop through all the tasklist entries
            for (ul = 0;
                 (ul < pSwBlock->cswentry) && (precThis);
                 ul++)
            {
                WPObject    *pObject;
                LONG        lClosable;

                precThis->ulIndex = ulIndex++;

                precThis->hSwitch = pSwBlock->aswentry[ul].hswitch;

                precThis->hwnd = pSwBlock->aswentry[ul].swctl.hwnd;
                sprintf(precThis->szHWND, "0x%lX", pSwBlock->aswentry[ul].swctl.hwnd);
                precThis->pszHWND = precThis->szHWND;

                if (precThis->hwnd)
                    WinQueryClassName(precThis->hwnd,
                                      sizeof(precThis->szWinClass),
                                      precThis->szWinClass);
                precThis->pszWinClass = precThis->szWinClass;

                precThis->hPgm = pSwBlock->aswentry[ul].swctl.hprog;
                precThis->pid = pSwBlock->aswentry[ul].swctl.idProcess;
                precThis->sid = pSwBlock->aswentry[ul].swctl.idSession;
                precThis->ulVisibility = pSwBlock->aswentry[ul].swctl.uchVisibility;

                strhncpy0(precThis->szSwTitle,
                          pSwBlock->aswentry[ul].swctl.szSwtitle,
                          sizeof(precThis->szSwTitle));
                precThis->pszSwTitle = precThis->szSwTitle;

                WinQueryWindowText(pSwBlock->aswentry[ul].swctl.hwnd,
                                   sizeof(precThis->szWinTitle),
                                   precThis->szWinTitle);
                precThis->pszWinTitle = precThis->szWinTitle;

                lClosable = xsdIsClosable(hab,
                                          &SDConsts,
                                          &pSwBlock->aswentry[ul],
                                          &pObject,
                                          &precThis->uid);
                sprintf(precThis->szClosable, "%d", lClosable);
                precThis->pszClosable = precThis->szClosable;

                sprintf(precThis->szObject, "0x%lX", pObject);
                precThis->pszObject = precThis->szObject;

                sprintf(precThis->szUid, "%d", precThis->uid);
                precThis->pszUid = precThis->szUid;

                if (pObject)
                {
                    PVIEWITEM pvi;

                    precThis->pszObjectTitle = _wpQueryTitle(pObject);

                    if (pvi = _wpFindViewItem(pObject,
                                              VIEW_RUNNING,
                                              NULL))
                    {
                        precThis->happObject = pvi->handle;
                        sprintf(precThis->szHAPP, "%lX", pvi->handle);
                        precThis->pszHAPP = precThis->szHAPP;
                    }
                }

                precThis = (PWINLISTRECORD)precThis->recc.preccNextRecord;
            }
        }

        cnrhInsertRecords(hwndCnr,
                          NULL,         // parent
                          (PRECORDCORE)pFirstRec,
                          TRUE,
                          NULL,
                          CRA_RECORDREADONLY,
                          ulIndex);

        free(pSwBlock);
    }

    return ulIndex;
}

/*
 *@@ winl_fnwpWinList:
 *
 *@@added V0.9.4 (2000-07-15) [umoeller]
 */

MRESULT EXPENTRY winl_fnwpWinList(HWND hwndClient, ULONG msg, MPARAM mp1, MPARAM mp2)
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
                                          ID_WINLISTCNR,
                                          NULL, NULL);
                if (hwndCnr)
                {
                    XFIELDINFO      xfi[22];
                    PFIELDINFO      pfi = NULL;
                    PWINLISTRECORD  pMemRecordFirst;
                    int             i = 0;

                    ULONG           ulTotalItems = 0,
                                    ulAllocatedItems = 0,
                                    ulFreedItems = 0;
                    ULONG           ulTotalBytes = 0,
                                    ulAllocatedBytes = 0,
                                    ulFreedBytes = 0;

                    // set up cnr details view
                    xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, ulIndex);
                    xfi[i].pszColumnTitle = "i";
                    xfi[i].ulDataType = CFA_ULONG;
                    xfi[i++].ulOrientation = CFA_RIGHT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, hSwitch);
                    xfi[i].pszColumnTitle = "hSw";
                    xfi[i].ulDataType = CFA_ULONG;
                    xfi[i++].ulOrientation = CFA_RIGHT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszSwTitle);
                    xfi[i].pszColumnTitle = "Switch title";
                    xfi[i].ulDataType = CFA_STRING;
                    xfi[i++].ulOrientation = CFA_LEFT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszWinTitle);
                    xfi[i].pszColumnTitle = "Win title";
                    xfi[i].ulDataType = CFA_STRING;
                    xfi[i++].ulOrientation = CFA_LEFT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszHWND);
                    xfi[i].pszColumnTitle = "hwnd";
                    xfi[i].ulDataType = CFA_STRING;
                    xfi[i++].ulOrientation = CFA_RIGHT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszWinClass);
                    xfi[i].pszColumnTitle = "Class";
                    xfi[i].ulDataType = CFA_STRING;
                    xfi[i++].ulOrientation = CFA_LEFT;

                    /* xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, hPgm);
                    xfi[i].pszColumnTitle = "hPgm";
                    xfi[i].ulDataType = CFA_ULONG;
                    xfi[i++].ulOrientation = CFA_RIGHT; */

                    xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pid);
                    xfi[i].pszColumnTitle = "pid";
                    xfi[i].ulDataType = CFA_ULONG;
                    xfi[i++].ulOrientation = CFA_RIGHT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, sid);
                    xfi[i].pszColumnTitle = "sid";
                    xfi[i].ulDataType = CFA_ULONG;
                    xfi[i++].ulOrientation = CFA_RIGHT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, ulVisibility);
                    xfi[i].pszColumnTitle = "Vis";
                    xfi[i].ulDataType = CFA_ULONG;
                    xfi[i++].ulOrientation = CFA_RIGHT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszClosable);
                    xfi[i].pszColumnTitle = "Close";
                    xfi[i].ulDataType = CFA_STRING;
                    xfi[i++].ulOrientation = CFA_RIGHT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszObject);
                    xfi[i].pszColumnTitle = "Obj";
                    xfi[i].ulDataType = CFA_STRING;
                    xfi[i++].ulOrientation = CFA_RIGHT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszObjectTitle);
                    xfi[i].pszColumnTitle = "Obj Title";
                    xfi[i].ulDataType = CFA_STRING;
                    xfi[i++].ulOrientation = CFA_RIGHT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszUid);
                    xfi[i].pszColumnTitle = "uid";
                    xfi[i].ulDataType = CFA_STRING;
                    xfi[i++].ulOrientation = CFA_RIGHT;

                    xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszHAPP);
                    xfi[i].pszColumnTitle = "HAPP";
                    xfi[i].ulDataType = CFA_STRING;
                    xfi[i++].ulOrientation = CFA_RIGHT;

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

                    winlCreateRecords(hwndCnr);

                    BEGIN_CNRINFO()
                    {
                        cnrhSetView(CV_DETAIL | CV_MINI | CA_DETAILSVIEWTITLES
                                        | CA_DRAWICON
                                    /* | CA_CONTAINERTITLE | CA_TITLEREADONLY
                                        | CA_TITLESEPARATOR | CA_TITLELEFT*/ );
                        cnrhSetSplitBarAfter(pfi);
                        cnrhSetSplitBarPos(250);
                    } END_CNRINFO(hwndCnr);

                    WinSetFocus(HWND_DESKTOP, hwndCnr);
                }
            }
            CATCH(excpt1) {} END_CATCH();

            mrc = WinDefWindowProc(hwndClient, msg, mp1, mp2);
        }
        break;

        case WM_WINDOWPOSCHANGED:
        {
            PSWP pswp = (PSWP)mp1;
            mrc = WinDefWindowProc(hwndClient, msg, mp1, mp2);
            if (pswp->fl & SWP_SIZE)
            {
                WinSetWindowPos(WinWindowFromID(hwndClient, ID_WINLISTCNR), // cnr
                                HWND_TOP,
                                0, 0, pswp->cx, pswp->cy,
                                SWP_SIZE | SWP_MOVE | SWP_SHOW);
            }
        }
        break;

        case WM_CONTROL:
        {
            USHORT usItemID = SHORT1FROMMP(mp1),
                   usNotifyCode = SHORT2FROMMP(mp1);
            if (usItemID == ID_WINLISTCNR)       // cnr
            {
                switch (usNotifyCode)
                {
                    // V0.9.16 (2002-01-13) [umoeller]
                    case CN_ENTER:
                    {
                        PNOTIFYRECORDENTER pnre = (PNOTIFYRECORDENTER)mp2;
                        PWINLISTRECORD prec;
                        if (prec = (PWINLISTRECORD)pnre->pRecord)
                            WinSwitchToProgram(prec->hSwitch);
                    }
                    break;

                    case CN_CONTEXTMENU:
                    {
                        PWINLISTRECORD precc = (PWINLISTRECORD)mp2;
                        if (precc == NULL)
                        {
                            // whitespace
                            HWND hwndMenu = WinCreateMenu(HWND_DESKTOP,
                                                          NULL); // no menu template
                            winhInsertMenuItem(hwndMenu,
                                               MIT_END,
                                               1001,
                                               "Sort by index",
                                               MIS_TEXT, 0);
                            winhInsertMenuItem(hwndMenu,
                                               MIT_END,
                                               1002,
                                               "Sort by source file",
                                               MIS_TEXT, 0);
                            cnrhShowContextMenu(WinWindowFromID(hwndClient, ID_WINLISTCNR),
                                                NULL,       // record
                                                hwndMenu,
                                                hwndClient);
                        }
                    }
                }
            }
        }
        break;

        case WM_COMMAND:
/*             switch (SHORT1FROMMP(mp1))
            {
                case 1001:  // sort by index
                    WinSendMsg(WinWindowFromID(hwndClient, ID_WINLISTCNR),
                               CM_SORTRECORD,
                               (MPARAM)mnu_fnCompareIndex,
                               0);
                break;

                case 1002:  // sort by source file
                    WinSendMsg(WinWindowFromID(hwndClient, ID_WINLISTCNR),
                               CM_SORTRECORD,
                               (MPARAM)mnu_fnCompareSourceFile,
                               0);
                break;
            } */
        break;

        case WM_CLOSE:
            WinDestroyWindow(WinWindowFromID(hwndClient, ID_WINLISTCNR));
            WinDestroyWindow(WinQueryWindow(hwndClient, QW_PARENT));
        break;

        default:
            mrc = WinDefWindowProc(hwndClient, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ winlCreateWinListWindow:
 *
 *@@added V0.9.4 (2000-07-15) [umoeller]
 */

HWND winlCreateWinListWindow(VOID)
{
    HWND hwndFrame = NULLHANDLE;

    ULONG flStyle = FCF_TITLEBAR | FCF_SYSMENU | FCF_HIDEMAX
                    | FCF_SIZEBORDER | FCF_SHELLPOSITION
                    | FCF_NOBYTEALIGN | FCF_TASKLIST;
    if (WinRegisterClass(WinQueryAnchorBlock(HWND_DESKTOP),
                         "XWPWinList",
                         winl_fnwpWinList, 0L, 0))
    {
        HWND hwndClient;
        if (hwndFrame = WinCreateStdWindow(HWND_DESKTOP,
                                           0L,
                                           &flStyle,
                                           "XWPWinList",
                                           "Window List",
                                           0L,
                                           NULLHANDLE,     // resource
                                           0,
                                           &hwndClient))
        {
            WinSetWindowPos(hwndFrame,
                            HWND_TOP,
                            0, 0, 0, 0,
                            SWP_ZORDER | SWP_SHOW | SWP_ACTIVATE);
        }
    }

    return hwndFrame;
}

