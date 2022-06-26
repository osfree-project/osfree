
/*
 *@@ pg_winlist.c:
 *      daemon window list.
 *
 *      This functionality has changed completely with
 *      V0.9.19. The window list used to be maintained
 *      by the pager, but is now a central daemon service
 *      which can be used by many "clients", one of which
 *      is the pager. Another client is the XCenter
 *      window list widget, but essentially any window
 *      can use this service.
 *
 *      To become a client of the window list service,
 *      a window (in any process) sends XDM_ADDWINLISTWATCH
 *      to fnwpDaemonObject with a message value that it
 *      wants to receive when the window list changes.
 *      It then receives that message (plus that message
 *      value + 1) for notifications. See XDM_ADDWINLISTWATCH
 *      for details.
 *
 *      To get the entire current window list in one chunk
 *      of shared memory, the window should send XDM_QUERYWINLIST
 *      first because the notifications will only report
 *      changes to the current list.
 *
 *      See pg_control.c for an introduction to XPager.
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

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINSWITCHLIST
#define INCL_WINSYS
#define INCL_WINSTDCNR

#define INCL_GPIBITMAPS                 // needed for helpers\shapewin.h
#include <os2.h>

#include <stdio.h>
#include <setjmp.h>

#define DONT_REPLACE_FOR_DBCS
#define DONT_REPLACE_MALLOC         // in case mem debug is enabled
#include "setup.h"                      // code generation and debugging options

#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\regexp.h"             // extended regular expressions
#include "helpers\shapewin.h"           // shaped windows;
                                        // only needed for the window class names
#include "helpers\standards.h"          // some standard macros
#include "helpers\threads.h"

#include "xwpapi.h"                     // public XWorkplace definitions

#include "hook\xwphook.h"
#include "hook\hook_private.h"
#include "hook\xwpdaemn.h"              // XPager and daemon declarations

#pragma hdrstop

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// XPager
LINKLIST        G_llWinInfos;
            // linked list of PXWININFO structs; this is auto-free
HMTX            G_hmtxWinInfos = 0;
            // mutex sem protecting that array
            // V0.9.12 (2001-05-31) [umoeller]: made this private
LINKLIST        G_llTransientStickies;
            // linked list of PXWINTRANSIENT structs; this is auto-free
            // V1.0.0 (2002-08-13) [lafaix]

extern HAB      G_habDaemon;
extern USHORT   G_pidDaemon;

/* ******************************************************************
 *
 *   Debug window list frame
 *
 ********************************************************************/

/*
 *  Messy code, partly copied from cnrh.c, just for
 *  the window list debug window if DEBUG_WINDOWLIST
 *  is #define'd in include\setup.h.
 *
 *  Ignore this section, it causes headaches and other
 *  bad things.
 */

#ifdef DEBUG_WINDOWLIST
HWND            G_hwndDebugFrame = NULLHANDLE,
                G_hwndDebugCnr = NULLHANDLE;

VOID cnrhSetFieldInfo(PFIELDINFO *ppFieldInfo2,  // in/out: double ptr to FIELDINFO
                      ULONG ulFieldOffset,       // in: FIELDOFFSET(YOURRECORDCORE, yourField)
                      PSZ pszColumnTitle,        // in: column title; this must be a static string!!
                      ULONG ulDataType,          // in: column data type (CFA_* flags)
                      ULONG ulOrientation,       // in: vertical and horizontal orientation (CFA_* flags)
                      BOOL fDrawLines)           // in: if TRUE, we'll draw lines around the columns
{
    PFIELDINFO pInfo;
    if (    (ppFieldInfo2)
         && (pInfo = *ppFieldInfo2)
       )
    {
        ULONG flData = ulDataType | ulOrientation;
        if (fDrawLines)
            flData |= CFA_HORZSEPARATOR | CFA_SEPARATOR;

        pInfo->cb = sizeof(FIELDINFO);
        pInfo->flData = flData;
        pInfo->flTitle = CFA_FITITLEREADONLY | ulOrientation;
        pInfo->offStruct = ulFieldOffset;
        pInfo->pTitleData = pszColumnTitle;   // strdup removed, V0.9.1 (99-12-18) [umoeller]
        pInfo->pUserData   = NULL;
        pInfo->cxWidth = 0;
        *ppFieldInfo2 = pInfo->pNextFieldInfo;
    }
}

ULONG cnrhInsertFieldInfos(HWND hwndCnr,                // in: cnr for Details view
                           PFIELDINFO pFieldInfoFirst,  // in: first field info as returned
                                                        // by cnrhAllocFieldInfos
                           ULONG ulFieldCount)          // in: no. of field infos
{
    FIELDINFOINSERT fii;
    fii.cb = sizeof(FIELDINFOINSERT);
    fii.pFieldInfoOrder = (PFIELDINFO)CMA_END;
    fii.fInvalidateFieldInfo = TRUE;
    fii.cFieldInfoInsert = ulFieldCount;

    return (ULONG)WinSendMsg(hwndCnr,
                             CM_INSERTDETAILFIELDINFO,
                             (MPARAM)pFieldInfoFirst,
                             (MPARAM)&fii);
}

    #undef FIELDOFFSET
    #define FIELDOFFSET(type, field)    ((ULONG)&(((type *)0)->field))

    ULONG cnrhClearFieldInfos(HWND hwndCnr,
                              BOOL fInvalidate);

    #define cnrhAllocFieldInfos(hwndCnr, sColumnCount)              \
        (PFIELDINFO)WinSendMsg(hwndCnr,                             \
                               CM_ALLOCDETAILFIELDINFO,             \
                               MPFROMSHORT(sColumnCount),           \
                               NULL)

    typedef struct _XFIELDINFO
    {
        ULONG   ulFieldOffset;
        PCSZ    pszColumnTitle;
        ULONG   ulDataType;
        ULONG   ulOrientation;
    } XFIELDINFO, *PXFIELDINFO;

PFIELDINFO cnrhSetFieldInfos(HWND hwndCnr,            // in: container hwnd
                             PXFIELDINFO paxfi,       // in: pointer to an array of ulFieldCount XFIELDINFO structures
                             ULONG ulFieldCount,      // in: no. of items in paxfi array (> 0)
                             BOOL fDrawLines,         // in: if TRUE, we'll draw lines around the columns
                             ULONG ulFieldReturn)     // in: the column index to return as PFIELDINFO
{
    PFIELDINFO  pFieldInfoFirst,
                pFieldInfo2,
                pFieldInfoReturn = NULL;

    if ((pFieldInfoFirst = cnrhAllocFieldInfos(hwndCnr, ulFieldCount)))
    {
        ULONG ul = 0;
        PXFIELDINFO pxfi = NULL;

        pFieldInfo2 = pFieldInfoFirst;
        pxfi = paxfi;
        for (ul = 0; ul < ulFieldCount; ul++)
        {
            if (ul == ulFieldReturn)
                // set return value
                pFieldInfoReturn = pFieldInfo2;

            // set current field info;
            // this will modify pFieldInfo to point to the next
            cnrhSetFieldInfo(&pFieldInfo2,
                             pxfi->ulFieldOffset,
                             (PSZ)pxfi->pszColumnTitle,
                             pxfi->ulDataType,
                             pxfi->ulOrientation,
                             fDrawLines);
            pxfi++;
        }

        // insert field infos
        if (cnrhInsertFieldInfos(hwndCnr,
                                 pFieldInfoFirst,
                                 ulFieldCount) == 0)
            pFieldInfoReturn = NULL;
    }

    return pFieldInfoReturn;
}

    #define BEGIN_CNRINFO()                                         \
        {                                                           \
            CNRINFO     CnrInfo_ = {0};                             \
            ULONG       ulSendFlags_ = 0;                           \
            CnrInfo_.cb = sizeof(CnrInfo_);

    #define END_CNRINFO(hwndCnr)                                    \
            WinSendMsg((hwndCnr), CM_SETCNRINFO, (MPARAM)&CnrInfo_, \
                (MPARAM)ulSendFlags_);                              \
        }

    #define cnrhSetSplitBarAfter(pFieldInfo)                        \
        CnrInfo_.pFieldInfoLast = (pFieldInfo);                     \
        ulSendFlags_ |= CMA_PFIELDINFOLAST;

    #define cnrhSetSplitBarPos(xPos)                                \
        CnrInfo_.xVertSplitbar = (xPos);                            \
        ulSendFlags_ |= CMA_XVERTSPLITBAR;

    #define cnrhSetView(flNewAttr)                                  \
        CnrInfo_.flWindowAttr = (flNewAttr);                        \
        ulSendFlags_ |= CMA_FLWINDOWATTR;

PFNWP G_pfnwpFrameOrig = NULL;

MRESULT EXPENTRY fnwpSubclDebugFrame(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_CONTROL:
            if (SHORT1FROMMP(mp1) == FID_CLIENT)
            {
                switch (SHORT2FROMMP(mp1))
                {
                    case CN_ENTER:
                    {
                        PWINLISTRECORD prec;
                        if (!(prec = (PWINLISTRECORD)((PNOTIFYRECORDENTER)mp2)->pRecord))
                            DosBeep(1000, 10);
                        else
                            WinSetActiveWindow(HWND_DESKTOP,
                                               ((PXWININFO)prec->pWinInfo)->data.swctl.hwnd);
                    }
                    break;
                }
            }
            else
                mrc = G_pfnwpFrameOrig(hwnd, msg, mp1, mp2);
        break;


        default:
            mrc = G_pfnwpFrameOrig(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

STATIC VOID FillRec(PXWININFO pWinInfo)
{
    ULONG       pid, tid, flStyle;
    PWINLISTRECORD prec;
    HWND        hwnd = pWinInfo->data.swctl.hwnd;
    SWP         swp;

    if (!(prec = pWinInfo->prec))
        return;

    prec->recc.flRecordAttr = CRA_OWNERFREE | CRA_RECORDREADONLY;

    prec->recc.hptrIcon = pWinInfo->hptrFrame;

    sprintf(prec->szHWND, "%lX", hwnd);
    prec->pszHWND = prec->szHWND;

    sprintf(prec->szHWNDParent, "%lX", WinQueryWindow(hwnd, QW_PARENT));
    prec->pszHWNDParent = prec->szHWNDParent;

    prec->szFlags[0] = '\0';
    if (pWinInfo->data.swctl.uchVisibility & SWL_VISIBLE)       // 0x04
        strcat(prec->szFlags, "vis ");
    if (pWinInfo->data.swctl.uchVisibility & SWL_INVISIBLE)     // 0x01
        strcat(prec->szFlags, "!vis ");
    if (pWinInfo->data.swctl.uchVisibility & SWL_GRAYED)        // 0x02
        strcat(prec->szFlags, "gray ");

    if (pWinInfo->data.swctl.fbJump & SWL_JUMPABLE)             // 0x02
        strcat(prec->szFlags, "jmp ");
    if (pWinInfo->data.swctl.fbJump & SWL_NOTJUMPABLE)          // 0x01
        strcat(prec->szFlags, "!jmp ");
    prec->pszFlags = prec->szFlags;

    WinQueryWindowPos(hwnd, &swp);
    sprintf(prec->szPos,
            "x %d, cx %d, y %d, cy %d",
            swp.x, swp.cx, swp.y, swp.cy);
    prec->pszPos = prec->szPos;

    prec->pszSwtitle = pWinInfo->data.swctl.szSwtitle;

    switch (pWinInfo->data.bWindowType)
    {
        #define PRINTTYPE(t) case WINDOW_ ## t: prec->pszWindowType = # t; break
        PRINTTYPE(NORMAL);
        PRINTTYPE(MAXIMIZE);
        PRINTTYPE(MINIMIZE);
        PRINTTYPE(XWPDAEMON);
        PRINTTYPE(WPSDESKTOP);
        PRINTTYPE(STICKY);
        PRINTTYPE(NIL);
        PRINTTYPE(HIDDEN);
    }

    WinQueryClassName(pWinInfo->data.swctl.hwnd, sizeof(prec->szClass), prec->szClass);
    prec->pszClass = prec->szClass;

    WinQueryWindowProcess(pWinInfo->data.swctl.hwnd, &pid, &tid);
    sprintf(prec->szPID, "%04lX", pid);
    prec->pszPID = prec->szPID;

    prec->szStyle[0] = '\0';
    prec->pszStyle = prec->szStyle;
    if (flStyle = WinQueryWindowULong(pWinInfo->data.swctl.hwnd, QWL_STYLE))
    {
        #define PRINTSTYLE(s) if (flStyle & s) strcat(prec->szStyle, # s " ")
        PRINTSTYLE(WS_VISIBLE);
        PRINTSTYLE(WS_DISABLED);
        PRINTSTYLE(WS_CLIPCHILDREN);
        PRINTSTYLE(WS_CLIPSIBLINGS);
        PRINTSTYLE(WS_PARENTCLIP);
        PRINTSTYLE(WS_SAVEBITS);
        PRINTSTYLE(WS_SYNCPAINT);
        PRINTSTYLE(WS_MINIMIZED);
        PRINTSTYLE(WS_MAXIMIZED);
        PRINTSTYLE(WS_ANIMATE);

        if (!strcmp(prec->szClass, "#1"))
        {
            PRINTSTYLE(FS_ICON);
            PRINTSTYLE(FS_ACCELTABLE);
            PRINTSTYLE(FS_SHELLPOSITION);
            PRINTSTYLE(FS_TASKLIST);
            PRINTSTYLE(FS_NOBYTEALIGN);
            PRINTSTYLE(FS_NOMOVEWITHOWNER);
            PRINTSTYLE(FS_SYSMODAL);
            PRINTSTYLE(FS_DLGBORDER);
            PRINTSTYLE(FS_BORDER);
            PRINTSTYLE(FS_SCREENALIGN);
            PRINTSTYLE(FS_MOUSEALIGN);
            PRINTSTYLE(FS_SIZEBORDER);
            PRINTSTYLE(FS_AUTOICON);
            // PRINTSTYLE(FS_DBE_APPSTAT);
        }
    }

    prec->pWinInfo = pWinInfo;
}

STATIC VOID CreateDebugFrame(VOID)
{
    ULONG fl = FCF_SYSMENU | FCF_TITLEBAR | FCF_MINBUTTON | FCF_SIZEBORDER | FCF_NOBYTEALIGN | FCF_TASKLIST;
    if (!(G_hwndDebugFrame = WinCreateStdWindow(HWND_DESKTOP,
                                                0,
                                                &fl,
                                                WC_CONTAINER,
                                                "Debug Window List",
                                                WS_VISIBLE | CCS_MINIICONS,
                                                0,
                                                0,
                                                &G_hwndDebugCnr)))
        _PmpfF(("Cannot create debug frame"));
    else
    {
        G_pfnwpFrameOrig = WinSubclassWindow(G_hwndDebugFrame, fnwpSubclDebugFrame);

        #define DEBUGSPACING    30
        #define DEBUGWIDTH      500
        #define DEBUGHEIGHT     (WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN) - 2 * DEBUGSPACING)
        _PmpfF(("got debug frame 0x%lX", G_hwndDebugFrame));
        WinSetWindowPos(G_hwndDebugFrame,
                        HWND_TOP,
                        WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN) - DEBUGWIDTH - DEBUGSPACING,
                        DEBUGSPACING,
                        DEBUGWIDTH,
                        DEBUGHEIGHT,
                        SWP_SHOW | SWP_ZORDER | SWP_MOVE | SWP_SIZE);

        {
            XFIELDINFO      xfi[10];
            PFIELDINFO      pfi = NULL;
            int             i = 0;

            CHAR            szFont[] = "9.WarpSans";
            WinSetPresParam(G_hwndDebugCnr,
                            PP_FONTNAMESIZE,
                            sizeof(szFont),
                            szFont);

            // set up cnr details view
            xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, recc.hptrIcon);
            xfi[i].pszColumnTitle = "";
            xfi[i].ulDataType = CFA_BITMAPORICON;
            xfi[i++].ulOrientation = CFA_LEFT | CFA_TOP;

            xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszHWND);
            xfi[i].pszColumnTitle = "hwnd";
            xfi[i].ulDataType = CFA_STRING;
            xfi[i++].ulOrientation = CFA_LEFT | CFA_TOP;

            xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszHWNDParent);
            xfi[i].pszColumnTitle = "pWnd";
            xfi[i].ulDataType = CFA_STRING;
            xfi[i++].ulOrientation = CFA_LEFT | CFA_TOP;

            xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszFlags);
            xfi[i].pszColumnTitle = "fl";
            xfi[i].ulDataType = CFA_STRING;
            xfi[i++].ulOrientation = CFA_LEFT | CFA_TOP;

            xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszPos);
            xfi[i].pszColumnTitle = "pos";
            xfi[i].ulDataType = CFA_STRING;
            xfi[i++].ulOrientation = CFA_LEFT | CFA_TOP;

            xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszSwtitle);
            xfi[i].pszColumnTitle = "szSwtitle";
            xfi[i].ulDataType = CFA_STRING;
            xfi[i++].ulOrientation = CFA_LEFT | CFA_TOP;

            xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszWindowType);
            xfi[i].pszColumnTitle = "type";
            xfi[i].ulDataType = CFA_STRING;
            xfi[i++].ulOrientation = CFA_LEFT | CFA_TOP;

            xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszPID);
            xfi[i].pszColumnTitle = "pid";
            xfi[i].ulDataType = CFA_STRING;
            xfi[i++].ulOrientation = CFA_LEFT | CFA_TOP;

            xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszClass);
            xfi[i].pszColumnTitle = "cls";
            xfi[i].ulDataType = CFA_STRING;
            xfi[i++].ulOrientation = CFA_LEFT | CFA_TOP;

            xfi[i].ulFieldOffset = FIELDOFFSET(WINLISTRECORD, pszStyle);
            xfi[i].pszColumnTitle = "style";
            xfi[i].ulDataType = CFA_STRING;
            xfi[i++].ulOrientation = CFA_LEFT | CFA_TOP;

            pfi = cnrhSetFieldInfos(G_hwndDebugCnr,
                                    xfi,
                                    i,             // array item count
                                    TRUE,          // draw lines
                                    4);

            BEGIN_CNRINFO()
            {
                cnrhSetView(CV_DETAIL | CA_DETAILSVIEWTITLES | CV_MINI);
                cnrhSetSplitBarAfter(pfi);
                cnrhSetSplitBarPos(DEBUGWIDTH * 2 / 5);
            } END_CNRINFO(G_hwndDebugCnr);
        }
    }
}

#endif

/* ******************************************************************
 *
 *   PAGERWININFO list maintenance
 *
 ********************************************************************/

/*
 *@@ pgrInit:
 *      initializes the winlist. Called from main().
 *
 *@@added V0.9.12 (2001-05-31) [umoeller]
 *@@changed V1.0.0 (2002-08-13) [lafaix]: initializing transient stikies list
 */

APIRET pgrInit(VOID)
{
    APIRET arc = DosCreateMutexSem(NULL,
                                   &G_hmtxWinInfos,
                                   0,
                                   FALSE);

    lstInit(&G_llWinInfos, TRUE);
            // V0.9.7 (2001-01-21) [umoeller]
    lstInit(&G_llTransientStickies, TRUE);
            // V1.0.0 (2002-08-13) [lafaix]

    #ifdef DEBUG_WINDOWLIST
        CreateDebugFrame();
    #endif

    return arc;
}

/*
 *@@ pgrLockWinlist:
 *      locks the window list.
 *
 *@@added V0.9.12 (2001-05-31) [umoeller]
 */

BOOL pgrLockWinlist(VOID)
{
    return !DosRequestMutexSem(G_hmtxWinInfos, TIMEOUT_HMTX_WINLIST);
}

/*
 *@@ pgrUnlockWinlist:
 *
 *@@added V0.9.12 (2001-05-31) [umoeller]
 */

VOID pgrUnlockWinlist(VOID)
{
    DosReleaseMutexSem(G_hmtxWinInfos);
}

/*
 *@@ pgrFindWinInfo:
 *      returns the PAGERWININFO for hwndThis from
 *      the global linked list or NULL if no item
 *      exists for that window.
 *
 *      If ppListNode is != NULL, it receives a
 *      pointer to the LISTNODE representing that
 *      item (in case you want to quickly free it).
 *
 *      Preconditions:
 *
 *      --  The caller must lock the list first.
 *
 *@@added V0.9.7 (2001-01-21) [umoeller]
 */

PXWININFO pgrFindWinInfo(HWND hwndThis,         // in: window to find
                         PVOID *ppListNode)     // out: list node (ptr can be NULL)
{
    PXWININFO pReturn = NULL;

    PLISTNODE pNode = lstQueryFirstNode(&G_llWinInfos);
    while (pNode)
    {
        PXWININFO pWinInfo = (PXWININFO)pNode->pItemData;
        if (pWinInfo->data.swctl.hwnd == hwndThis)
        {
            pReturn = pWinInfo;
            break;
        }

        pNode = pNode->pNext;
    }

    if (ppListNode)
        *ppListNode = pNode;

    return pReturn;
}

/*
 *@@ ClearWinlist:
 *      clears the global list of PAGERWININFO entries.
 *
 *      Preconditions:
 *
 *      --  The caller must lock the list first.
 *
 *@@added V0.9.7 (2001-01-21) [umoeller]
 */

STATIC VOID ClearWinlist(VOID)
{
    #ifdef DEBUG_WINDOWLIST
        WinSendMsg(G_hwndDebugCnr,
                   CM_REMOVERECORD,
                   NULL,
                   MPFROM2SHORT(0, CMA_FREE | CMA_INVALIDATE));
    #endif

    // clear the list... it's in auto-free mode,
    // so this will clear the WININFO structs as well
    lstClear(&G_llWinInfos);
}

/* ******************************************************************
 *
 *   Debugging
 *
 ********************************************************************/

#ifdef __DEBUG__

STATIC VOID DumpOneWindow(PCSZ pcszPrefix,
                          PXWININFO pEntryThis)
{
    _Pmpf(("%s hwnd 0x%lX \"%s\":\"%s\" pid 0x%lX(%d) type %d",
           pcszPrefix,
           pEntryThis->data.swctl.hwnd,
           pEntryThis->data.swctl.szSwtitle,
           pEntryThis->data.szClassName,
           pEntryThis->data.swctl.idProcess,
           pEntryThis->data.swctl.idProcess,
           pEntryThis->data.bWindowType));
}

/*
 *@@ DumpAllWindows:
 *
 */

STATIC VOID DumpAllWindows(VOID)
{
    if (pgrLockWinlist())
    {
        ULONG ul = 0;
        PLISTNODE pNode = lstQueryFirstNode(&G_llWinInfos);
        while (pNode)
        {
            PXWININFO pEntryThis = (PXWININFO)pNode->pItemData;
            CHAR szPrefix[100];
            sprintf(szPrefix, "Dump %d:", ul++);
            DumpOneWindow(szPrefix, pEntryThis);

            pNode = pNode->pNext;
        }

        pgrUnlockWinlist();
    }
}

#endif

/* ******************************************************************
 *
 *   List management
 *
 ********************************************************************/

/*
 *@@ AddInfo:
 *      adds the given wininfo to the global list.
 *
 *      Preconditions:
 *
 *      --  Caller must hold the winlist mutex.
 *
 *@@added V0.9.19 (2002-06-15) [umoeller]
 */

STATIC VOID AddInfo(PXWININFO pWinInfo)
{
    lstAppendItem(&G_llWinInfos, pWinInfo);

    #ifdef DEBUG_WINDOWLIST
    {
        RECORDINSERT    ri;
        if (pWinInfo->prec = (PWINLISTRECORD)WinSendMsg(G_hwndDebugCnr,
                                                        CM_ALLOCRECORD,
                                                        (MPARAM)(sizeof(WINLISTRECORD) - sizeof(RECORDCORE)),
                                                        (MPARAM)1))
        {
            FillRec(pWinInfo);
            ri.cb = sizeof(RECORDINSERT);
            ri.pRecordOrder = (PRECORDCORE)CMA_END;
            ri.pRecordParent = NULL;
            ri.zOrder = CMA_TOP;
            ri.fInvalidateRecord = TRUE;
            ri.cRecordsInsert = 1;
            WinSendMsg(G_hwndDebugCnr,
                       CM_INSERTRECORD,
                       (PRECORDCORE)pWinInfo->prec,
                       (MPARAM)&ri);
        }
    }
    #endif
}

/*
 *@@ RemoveInfo:
 *
 *@@added V0.9.19 (2002-06-15) [umoeller]
 */

STATIC VOID RemoveInfo(PLISTNODE pNode)
{
    #ifdef DEBUG_WINDOWLIST
    {
        PXWININFO pWinInfo = (PXWININFO)pNode->pItemData;
        WinSendMsg(G_hwndDebugCnr,
                   CM_REMOVERECORD,
                   (MPARAM)&pWinInfo->prec,
                   MPFROM2SHORT(1,
                                CMA_INVALIDATE | CMA_FREE));
        pWinInfo->prec = NULL;
    }
    #endif

    lstRemoveNode(&G_llWinInfos, pNode);
}

/* ******************************************************************
 *
 *   Window analysis, exported interfaces
 *
 ********************************************************************/

/*
 *@@ pgrGetWinData:
 *      analyzes pWinInfo->hwnd and stores the results
 *      back in *pWinInfo.
 *
 *      This does not allocate a new PAGERWININFO.
 *      Use pgrCreateWinInfo for that, which
 *      calls this function in turn.
 *
 *      fQuickCheck should only be set if the caller is
 *      refreshing an existing wininfo and has already
 *      performed a number of checks. In fact, it is a
 *      special case for the call from MoveCurrentDesktop
 *      and probably only works for that call and should
 *      therefore be FALSE for all other calls.
 *
 *      Returns TRUE if the specified window data was
 *      returned.
 *
 *      If this returns FALSE, the window either
 *      does not exist or is considered irrelevant
 *      for XPager. This function excludes a number
 *      of window classes from the window list in
 *      order not to pollute anything.
 *      In that case, the fields in pWinInfo are
 *      undefined.
 *
 *      More specifically, the following are exluded:
 *
 *      --  Windows that are not children of HWND_DESKTOP.
 *
 *      --  Windows that belong to the pager.
 *
 *      --  Windows of the classes "PM Icon title",
 *          "AltTabWindow", "menu", and "shaped" windows.
 *
 *      Preconditions:
 *
 *      --  On input, pWinInfo->swctl.hwnd must be set to
 *          the window to be examined. All other fields
 *          are ignored and reset here.
 *
 *      --  If pWinInfo is one of the items on the global
 *          linked list, the caller must lock the list
 *          before calling this.
 *
 *@@added V0.9.2 (2000-02-21) [umoeller]
 *@@changed V0.9.4 (2000-08-08) [umoeller]: removed "special" windows; now ignoring ShapeWin windows
 *@@changed V0.9.15 (2001-09-14) [umoeller]: now always checking for visibility; this fixes VX-REXX apps hanging XPager
 *@@changed V0.9.18 (2002-02-21) [lafaix]: minimized and maximized windows titles were not queryied and maximized windows couldn't be sticky
 *@@changed V0.9.20 (2002-08-04) [umoeller]: while Embellish was running, pager was broken; fixed
 *@@changed V1.0.2 (2004-03-11) [umoeller]: optimized; added fQuickCheck param
 */

BOOL pgrGetWinData(PXWINDATA pData,     // in/out: window info
                   BOOL fQuickCheck)    // in: if TRUE, we assume basic data has already been retrieved and skip some
{
    BOOL    brc = FALSE;

    // back up the window handle
    HWND    hwnd = pData->swctl.hwnd;
    // ULONG   pid, tid;

    ZERO(pData);

    if (    (fQuickCheck)
         || (    (pData->swctl.hwnd = hwnd)
              // && (WinIsWindow(G_habDaemon, hwnd))     not necessary V1.0.2 (2004-03-11) [umoeller]
              && (WinQueryWindowProcess(hwnd,
                                        &pData->swctl.idProcess,
                                        &pData->tid))
              && (pData->swctl.idProcess)
              && (WinQueryClassName(hwnd,
                                    sizeof(pData->szClassName),
                                    pData->szClassName))
              && (WinQueryWindowPos(hwnd,
                                    &pData->swp))
            )
       )
    {
        brc = TRUE;     // can be changed again

        if (pData->swctl.idProcess == G_pidDaemon)
            // belongs to XPager:
            pData->bWindowType = WINDOW_XWPDAEMON;
        else if (hwnd == G_pHookData->hwndWPSDesktop)
            // better not move the WPS Desktop window:
            pData->bWindowType = WINDOW_WPSDESKTOP;
        else if (!fQuickCheck)      // V1.0.3 (2004-03-11) [umoeller]
        {
            const char *pcszClassName = pData->szClassName;
            if (
                    // make sure this is a desktop child;
                    // we use WinSetMultWindowPos, which requires
                    // that all windows have the same parent
                    // (!WinIsChild(hwnd, HWND_DESKTOP))
                    (WinQueryWindow(hwnd, QW_PARENT) != G_pHookData->hwndPMDesktop)
                            // fixed V0.9.20 (2002-08-04) [umoeller]
                            // stupid Embellish creates its toolbar window as a standard PM frame,
                            // which has the main Embellish window as its parent... for some reason,
                            // the above code caused this to be added in the window list, breaking
                            // the pager while Embellish is running
                    // ignore PM "Icon title" class:
                 || (!strcmp(pcszClassName, "#32765"))
                    // ignore Warp 4 "Alt tab" window; this always exists,
                    // but is hidden most of the time
                 || (!strcmp(pcszClassName, "AltTabWindow"))
                    // ignore all menus:
                 || (!strcmp(pcszClassName, "#4"))
                    // ignore shaped windows (src\helpers\shapewin.c):
                 || (!strcmp(pcszClassName, WC_SHAPE_WINDOW))
                 || (!strcmp(pcszClassName, WC_SHAPE_REGION))
               )
            {
                brc = FALSE;
            }
        }

        if (brc)
        {
            if (!(pData->hsw = WinQuerySwitchHandle(hwnd, 0)))
            {
                if (!pData->bWindowType)
                    pData->bWindowType = WINDOW_NIL;
            }
            else
                // get switch entry in all cases;
                // otherwise we have an empty switch title
                // for some windows in the list, which will cause
                // the new refresh thread to fire "window changed"
                // every time
                // V0.9.19 (2002-06-08) [umoeller]
                WinQuerySwitchEntry(pData->hsw,
                                    &pData->swctl);

            if (!pData->bWindowType)
            {
                // window type not found yet:
                ULONG ulStyle = WinQueryWindowULong(hwnd, QWL_STYLE);

                if (pgrIsSticky(hwnd,
                                pData->swctl.szSwtitle))
                {
                    pData->bWindowType = WINDOW_STICKY;
                }
                else if (    // V0.9.15 (2001-09-14) [umoeller]:
                             // _always_ check for visibility, and
                             // if the window isn't visible, don't
                             // mark it as normal
                             // (this helps VX-REXX apps, which can
                             // solidly lock XPager with their hidden
                             // frame in the background, upon which
                             // WinSetMultWindowPos fails)
                             (!(ulStyle & WS_VISIBLE))
                          || (pData->swp.fl & SWP_HIDE)
                          || (ulStyle & FCF_SCREENALIGN)  // netscape dialog
                        )
                {
                    pData->bWindowType = WINDOW_HIDDEN;
                }
                else
                {
                    // the minimize attribute prevails the "sticky" attribute,
                    // "sticky" prevails maximize, and maximize prevails normal
                    // V0.9.18 (2002-02-21) [lafaix]
                    if (pData->swp.fl & SWP_MINIMIZE)
                        pData->bWindowType = WINDOW_MINIMIZE;
                    else if (pData->swp.fl & SWP_MAXIMIZE)
                        pData->bWindowType = WINDOW_MAXIMIZE;
                    else
                        pData->bWindowType = WINDOW_NORMAL;
                }
            }
        } // if (brc)
    }

    return brc;
}

/*
 *@@ pgrCreateWinInfo:
 *      adds a new window to our window list.
 *
 *      Called upon XDM_WINDOWCHANGE in fnwpDaemonObject
 *      when either the hook or fntWinlistThread have
 *      determined that a new window might need to be added.
 *
 *      Returns what pgrGetWinData returned.
 *
 *@@added V0.9.2 (2000-02-21) [umoeller]
 *@@changed V0.9.7 (2001-01-21) [umoeller]: rewritten for linked list for wininfos
 *@@changed V0.9.19 (2002-05-28) [umoeller]: rewritten
 */

BOOL pgrCreateWinInfo(HWND hwnd)
{
    BOOL brc = FALSE;

    XWINDATA dataTemp;
    dataTemp.swctl.hwnd = hwnd;
    if (pgrGetWinData(&dataTemp, FALSE))
    {
        // window is valid and usable:

        if (pgrLockWinlist())
        {
            PXWININFO pWinInfo;

            // check if we have an entry for this window already
            if (!(pWinInfo = pgrFindWinInfo(hwnd, NULL)))
            {
                // not yet in list: create a new one
                if (pWinInfo = NEW(XWININFO))
                {
                    ZERO(pWinInfo);
                    memcpy(&pWinInfo->data, &dataTemp, sizeof(XWINDATA));

                    // store in list
                    AddInfo(pWinInfo);
                    brc = TRUE;
                }
            }
            else
            {
                // already present: refresh that one then
                #ifdef DEBUG_WINDOWLIST
                    PWINLISTRECORD pOld = pWinInfo->prec;
                #endif

                memcpy(&pWinInfo->data, &dataTemp, sizeof(XWINDATA));

                #ifdef DEBUG_WINDOWLIST
                    pWinInfo->prec = pOld;
                    FillRec(pWinInfo);
                    WinSendMsg(G_hwndDebugCnr,
                               CM_INVALIDATERECORD,
                               (MPARAM)&pWinInfo->prec,
                               MPFROM2SHORT(1,
                                            CMA_TEXTCHANGED));
                #endif

                brc = TRUE;
            }

            pgrUnlockWinlist();
        }
    }

    return brc;
}

/*
 *@@ pgrBuildWinlist:
 *      (re)initializes the window list.
 *      This must get called exactly once when
 *      the window list is initialized.
 *
 *@@added V0.9.2 (2000-02-21) [umoeller]
 *@@changed V0.9.7 (2001-01-21) [umoeller]: rewritten for linked list for wininfos
 */

VOID pgrBuildWinlist(VOID)
{
    if (pgrLockWinlist())
    {
        HENUM henum;
        HWND hwndTemp;
        LINKLIST llWnds;
        PLISTNODE pNode;
        lstInit(&llWnds, FALSE);

        ClearWinlist();

        // first, build a stack of open windows because
        // we want to add them in reverse order!
        // V0.9.20 (2002-07-22) [umoeller]
        henum = WinBeginEnumWindows(HWND_DESKTOP);
        while (hwndTemp = WinGetNextWindow(henum))
        {
            lstAppendItem(&llWnds, (PVOID)hwndTemp);
        }
        WinEndEnumWindows(henum);

        // now run through the list in reverse order
        for (pNode = lstQueryLastNode(&llWnds);
             pNode;
             pNode = pNode->pPrevious)
        {
            XWINDATA dataTemp;
            hwndTemp = (HWND)pNode->pItemData;
            dataTemp.swctl.hwnd = hwndTemp;
            if (pgrGetWinData(&dataTemp, FALSE))
            {
                // window found:
                // append this thing to the list

                PXWININFO pNew;
                if (pNew = NEW(XWININFO))
                {
                    memcpy(&pNew->data, &dataTemp, sizeof(dataTemp));
                    AddInfo(pNew);
                }
            }
        }

        pgrUnlockWinlist();

        lstClear(&llWnds);
    }

    WinPostMsg(G_pHookData->hwndPagerClient,
               PGRM_REFRESHCLIENT,
               (MPARAM)FALSE,
               0);
}

/*
 *@@ pgrFreeWinInfo:
 *      removes a window from our window list which has
 *      been destroyed.
 *
 *      Called upon XDM_WINDOWCHANGE in fnwpDaemonObject
 *      when either the hook or fntWinlistThread have
 *      determined that a window has died.
 *
 *@@added V0.9.2 (2000-02-21) [umoeller]
 *@@changed V0.9.7 (2001-01-21) [umoeller]: now using linked list for wininfos
 *@@changed V1.0.0 (2002-09-05) [lafaix]: removes possible entry in transient list too
 */

VOID pgrFreeWinInfo(HWND hwnd)
{
    if (pgrLockWinlist())
    {
        PLISTNODE       pNodeFound = NULL;
        PXWININFO       pWinInfo;

        if (    (pWinInfo = pgrFindWinInfo(hwnd,
                                            (PVOID*)&pNodeFound))
             && (pNodeFound)
           )
            // we have an item for this window:
            // remove from list, which will also free pWinInfo
            RemoveInfo(pNodeFound);

        // V1.0.0 (2002-09-05) [lafaix]
        pNodeFound = lstQueryFirstNode(&G_llTransientStickies);

        while (pNodeFound)
        {
            PXWINTRANSIENT pWinTransient = (PXWINTRANSIENT)pNodeFound->pItemData;

            if (hwnd == pWinTransient->hwnd)
            {
                lstRemoveNode(&G_llTransientStickies, pNodeFound);
                break;
            }

            pNodeFound = pNodeFound->pNext;
        }

        pgrUnlockWinlist();
    }
}

/*
 *@@ pgrRefresh:
 *      attempts to refresh a window in our window
 *      list if it's already present. Will not add
 *      it as a new window though.
 *
 *@@added V0.9.19 (2002-06-02) [umoeller]
 */

BOOL pgrRefresh(HWND hwnd)
{
    BOOL brc = FALSE;
    if (pgrLockWinlist())
    {
        // check if we have an entry for this window already
        PXWININFO    pWinInfo;
        if (pWinInfo = pgrFindWinInfo(hwnd,
                                      NULL))
        {
            // we have an entry:
            if (!pgrGetWinData(&pWinInfo->data, FALSE))
                // failed:
                WinPostMsg(G_pHookData->hwndDaemonObject,
                           XDM_WINDOWCHANGE,
                           (MPARAM)hwnd,
                           (MPARAM)WM_DESTROY);

            #ifdef DEBUG_WINDOWLIST
            else
            {
                FillRec(pWinInfo);
                WinSendMsg(G_hwndDebugCnr,
                           CM_INVALIDATERECORD,
                           (MPARAM)&pWinInfo->prec,
                           MPFROM2SHORT(1,
                                        CMA_TEXTCHANGED));
            }
            #endif

            brc = TRUE;
        }

        pgrUnlockWinlist();
    }

    return brc;
}
#if 0

/*
 *@@ pgrRefreshSWP:
 *      refreshes the SWP member and the bWindowType
 *      field.
 *
 *      Caller must hold the winlist mutex.
 *
 *@@added V0.9.19 (2002-06-18) [umoeller]
 */

BOOL pgrRefreshSWP(PXWININFO pWinInfo)
{
    // update window pos in window list
    WinQueryWindowPos(pWinInfo->swctl.hwnd,
                      &pWinInfo->swp);

    // update the bWindowType status, as restoring
    // a previously minimized or hidden window may
    // change it
    // (we don't have to update "special" windows)
    // V0.9.18 (2002-02-20) [lafaix]
    if (    (flSaved != pWinInfo->swp.fl)
         && (    (pWinInfo->bWindowType == WINDOW_NORMAL)
              || (pWinInfo->bWindowType == WINDOW_MINIMIZE)
              || (pWinInfo->bWindowType == WINDOW_MAXIMIZE)
            )
       )
    {
        bTypeThis = WINDOW_NORMAL;
        if (pWinInfo->swp.fl & SWP_MINIMIZE)
            bTypeThis = WINDOW_MINIMIZE;
        else
        if (pWinInfo->swp.fl & SWP_MAXIMIZE)
            bTypeThis = WINDOW_MAXIMIZE;
    }
}

#endif

/*
 *@@ pgrToggleTransientSticky:
 *      adds or removes a given window from the transient sticky
 *      list.
 *
 *      Returns TRUE if the window was successfully added or removed.
 *
 *@@added V1.0.0 (2002-08-13) [lafaix]
 */

BOOL pgrToggleTransientSticky(HWND hwnd)
{
    BOOL brc = FALSE;

    if (pgrLockWinlist())
    {
        // add if not already in transient stickies list
        PLISTNODE pNode = lstQueryFirstNode(&G_llTransientStickies);

        while (pNode)
        {
            PXWINTRANSIENT pWinTransient = (PXWINTRANSIENT)pNode->pItemData;

            if (hwnd == pWinTransient->hwnd)
            {
                pWinTransient->bSticky = !(pWinTransient->bSticky);
                brc = TRUE;
                break;
            }

            pNode = pNode->pNext;
        }

        if (!brc)
        {
            // not already in transient list, so add it
            PXWINTRANSIENT pWinTransient;

            if (pWinTransient = NEW(XWINTRANSIENT))
            {
                ZERO(pWinTransient);
                pWinTransient->hwnd = hwnd;
                pWinTransient->bSticky = TRUE;

                lstAppendItem(&G_llTransientStickies, pWinTransient);
                brc = TRUE;
            }
        }

        pgrRefresh(hwnd);

        pgrUnlockWinlist();
    }

    return brc;
}

/*
 *@@ pgrIsWindowTransientSticky:
 *      returns TRUE if the window with the specified handle is a
 *      transient sticky window.
 *
 *@@added V1.0.0 (2002-08-13) [lafaix]
 */

BOOL pgrIsWindowTransientSticky(HWND hwnd)
{
  return pgrIsSticky(hwnd, NULL);
}

/*
 *@@ pgrIsSticky:
 *      returns TRUE if the window with the specified window
 *      and switch titles is a sticky window. A window is
 *      considered sticky if its switch list title matches
 *      an include entry in the "sticky windows" list or if
 *      it is an XCenter and it does not match an exclude
 *      entry.
 *
 *@@added V0.9.2 (2000-02-21) [umoeller]
 *@@changed V0.9.16 (2001-10-31) [umoeller]: now making system window list sticky always
 *@@changed V0.9.19 (2002-04-14) [lafaix]: now uses matching criteria
 *@@changed V0.9.19 (2002-04-17) [umoeller]: added regular expressions (SF_MATCHES)
 *@@changed V1.0.0 (2002-08-13) [lafaix]: added support for transient stickies
 */

BOOL pgrIsSticky(HWND hwnd,
                 PCSZ pcszSwtitle)
{
    HWND    hwndClient;

    // check for system window list
    if (    (G_pHookData)
         && (hwnd == G_pHookData->hwndSwitchList)
       )
        return TRUE;

    // check for transient stickies
    if (pgrLockWinlist())
    {
        BOOL bFound = FALSE;
        PXWINTRANSIENT pWinTransient = NULL;

        PLISTNODE pNode = lstQueryFirstNode(&G_llTransientStickies);
        while (pNode)
        {
            pWinTransient = (PXWINTRANSIENT)pNode->pItemData;

            if (hwnd == pWinTransient->hwnd)
            {
                bFound = TRUE;
                break;
            }

            pNode = pNode->pNext;
        }

        pgrUnlockWinlist();

        if (bFound)
            return pWinTransient->bSticky;
    }

    if (pcszSwtitle)
    {
        ULONG   ul;

        for (ul = 0;
             ul < G_pHookData->PagerConfig.cStickies;
             ul++)
        {
            ULONG   flThis = G_pHookData->PagerConfig.aulStickyFlags[ul];
            PCSZ    pcszThis = G_pHookData->PagerConfig.aszStickies[ul];

            BOOL    fInclude = (flThis & SF_CRITERIA_MASK) == SF_INCLUDE;

            if ((flThis & SF_OPERATOR_MASK) == SF_MATCHES)
            {
                // regular expression:
                // check if we have compiled this one already
                int rc;
                ERE *pERE;
                if (!(G_pHookData->paEREs[ul]))
                    // compile now
                    G_pHookData->paEREs[ul] = rxpCompile(pcszThis,
                                                         0,
                                                         &rc);

                if (pERE = G_pHookData->paEREs[ul])
                {
                    int             pos, length;
                    ERE_MATCHINFO   mi;
                    // _PmpfF(("checking %s", pcszSwtitle));
                    if (rxpMatch_fwd(pERE,
                                     0,
                                     pcszSwtitle,
                                     0,
                                     &pos,
                                     &length,
                                     &mi))
                    {
                         // make sure we don't just have a substring
                         if (    (pos == 0)
                              && (    (length >= STICKYLEN - 1)
                                   || (length == strlen(pcszSwtitle))
                                 )
                            )
                            return fInclude;
                    }
                }
            }
            else
            {
                PCSZ pResult = strstr(pcszSwtitle,
                                      pcszThis);

                switch (flThis & SF_OPERATOR_MASK)
                {
                     case SF_CONTAINS:
                         if (pResult)
                             return fInclude;
                     break;

                     case SF_BEGINSWITH:
                         if (pResult == pcszSwtitle)
                             return fInclude;
                     break;

                     case SF_ENDSWITH:
                         if (    (pResult)
                                 // as the pattern has been found, the switch name
                                 // is at least as long as the pattern, so the following
                                 // is safe
                              && (!strcmp(  pcszSwtitle
                                          + strlen(pcszSwtitle)
                                          - strlen(pcszThis),
                                          pcszThis))
                            )
                         {
                             return fInclude;
                         }
                     break;

                     case SF_EQUALS:
                         if (     (pResult)
                              &&  (!strcmp(pcszSwtitle,
                                           pcszThis))
                            )
                         {
                             return fInclude;
                         }
                     break;
                }
            }
        }
    }

    // not in sticky names list:
    // check if it's an XCenter (check client class name)
    if (hwndClient = WinWindowFromID(hwnd, FID_CLIENT))
    {
        CHAR szClass[100];
        if (    (WinQueryClassName(hwndClient, sizeof(szClass), szClass))
             && (!strcmp(szClass, WC_XCENTER_CLIENT))
           )
            // target is XCenter:
            return TRUE;
    }

    return FALSE;
}

/*
 *@@ pgrIconChange:
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

BOOL pgrIconChange(HWND hwnd,
                   HPOINTER hptr)
{
    BOOL brc = FALSE;
    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = pgrLockWinlist())
        {
            PLISTNODE pNode = lstQueryFirstNode(&G_llWinInfos);
            while (pNode)
            {
                PXWININFO pWinInfo = (PXWININFO)pNode->pItemData;

                if (pWinInfo->data.swctl.hwnd == hwnd)
                {
                    // check icons only if the switch list item
                    // is visible in the first place; otherwise,
                    // with some windows (e.g. mozilla), we fire
                    // out the "icon change" message before the
                    // xcenter winlist gets a chance of adding
                    // the switch list item to its private list
                    // because it suppresses entries without
                    // SWL_VISIBLE
                    if (    (pWinInfo->data.swctl.uchVisibility & SWL_VISIBLE)
                         && (pWinInfo->hptrFrame != hptr)
                       )
                    {
                        pWinInfo->hptrFrame = hptr;

                        #ifdef DEBUG_WINDOWLIST
                        {
                            if (pWinInfo->prec)
                            {
                                 pWinInfo->prec->recc.hptrIcon = hptr;
                                 WinSendMsg(G_hwndDebugCnr,
                                            CM_INVALIDATERECORD,
                                            (MPARAM)&pWinInfo->prec,
                                            MPFROM2SHORT(1,
                                                         CMA_TEXTCHANGED));
                            }
                        }
                        #endif

                        brc = TRUE;
                    }
                    break;
                }

                pNode = pNode->pNext;
            }
        }
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    if (fLocked)
    {
        pgrUnlockWinlist();
        fLocked = FALSE;
    }

    return brc;
}

/*
 *@@ pgrQueryWinList:
 *      implementation for XDM_QUERYWINLIST.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

PSWBLOCK pgrQueryWinList(ULONG pid)
{
    BOOL brc = FALSE;
    BOOL fLocked = FALSE;
    PSWBLOCK pSwblockReturn = NULL,
             pSwblock;

    TRY_LOUD(excpt1)
    {
        if (fLocked = pgrLockWinlist())
        {
            PLISTNODE pNode;
            ULONG cWindows, cbSwblock;
            if (    (cWindows = lstCountItems(&G_llWinInfos))
                 && (cbSwblock = (cWindows * sizeof(SWENTRY)) + sizeof(HSWITCH))
                 && (!DosAllocSharedMem((PVOID*)&pSwblock,
                                        NULL,
                                        cbSwblock,
                                        PAG_COMMIT | OBJ_GIVEABLE | PAG_READ | PAG_WRITE))
               )
            {
                ULONG ul = 0;
                pSwblock->cswentry = cWindows;
                pNode = lstQueryFirstNode(&G_llWinInfos);
                while (pNode)
                {
                    PXWININFO pWinInfo = (PXWININFO)pNode->pItemData;

                    // return switch handle
                    pSwblock->aswentry[ul].hswitch = pWinInfo->data.hsw;

                    // return SWCNTRL
                    memcpy(&pSwblock->aswentry[ul].swctl,
                           &pWinInfo->data.swctl,
                           sizeof(SWCNTRL));

                    // override hwndIcon with the frame icon if we
                    // queried one in the background thread
                    pSwblock->aswentry[ul].swctl.hwndIcon = pWinInfo->hptrFrame;

                    pNode = pNode->pNext;
                    ++ul;
                }

                if (!DosGiveSharedMem(pSwblock,
                                      pid,
                                      PAG_READ | PAG_WRITE))
                {
                    pSwblockReturn = pSwblock;
                }

                DosFreeMem(pSwblock);
            }
        }
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    if (fLocked)
    {
        pgrUnlockWinlist();
        fLocked = FALSE;
    }

    return pSwblockReturn;
}

/*
 *@@ CheckWindow:
 *      checks one window from the system switchlist
 *      against our private list.
 *
 *      We only hold the winlist mutex (for the private
 *      list) locked for a very short time in order not
 *      to block the system since we might send msgs
 *      from here.
 *
 *      This fires XDM_WINDOWCHANGE or XDM_ICONCHANGE
 *      to the daemon object window if the window changed
 *      compared to our list.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 *@@changed V0.9.19 (2002-06-18) [umoeller]: added visible/jumpable checks
 *@@changed V1.0.6 (2006-08-04) [pr]: null pointer check @@fixes 748
 */

VOID CheckWindow(HAB hab,
                 PSWCNTRL pCtrlThis)
{
    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = pgrLockWinlist())
        {
            // check if the entry is in the list already...

            // 1) rule out obvious non-windows
            if (    (pCtrlThis->hwnd)
                 && (WinIsWindow(hab, pCtrlThis->hwnd))
               )
            {
                PXWININFO pInfo;
                PLISTNODE pNode;

                if (!(pInfo = pgrFindWinInfo(pCtrlThis->hwnd,
                                             (PVOID*)&pNode)))
                {
                    // window is not in list: add it then
                    if (   (pCtrlThis->uchVisibility & SWL_VISIBLE)
                        && G_pHookData  // V1.0.6 (2006-08-04) [pr]
                       )
                        WinPostMsg(G_pHookData->hwndDaemonObject,
                                   XDM_WINDOWCHANGE,
                                   (MPARAM)pCtrlThis->hwnd,
                                   (MPARAM)WM_CREATE);
                }
                else
                {
                    HWND        hwnd;
                    HPOINTER    hptrOld;
                    BOOL        fVisible;

                    // OK, this list node is still in the switchlist:
                    // check if all the data is still valid
                    if (    // Mozilla starts up with a visible, jumpable, minimized
                            // window that is later changed to !visible and !jumpable,
                            // so fix this V0.9.19 (2002-06-18) [umoeller]
                            (pCtrlThis->uchVisibility != pInfo->data.swctl.uchVisibility)
                         || (pCtrlThis->fbJump != pInfo->data.swctl.fbJump)
                            // switch title changed?
                         || (strcmp(pCtrlThis->szSwtitle,
                                    pInfo->data.swctl.szSwtitle))
                       )
                    {
                        // change: refresh
                        if (G_pHookData)        // V1.0.6 (2006-08-04) [pr]
                            WinPostMsg(G_pHookData->hwndDaemonObject,
                                       XDM_WINDOWCHANGE,
                                       (MPARAM)pCtrlThis->hwnd,
                                       (MPARAM)WM_SETWINDOWPARAMS);
                    }

                    hwnd = pCtrlThis->hwnd;
                    hptrOld = pInfo->hptrFrame;
                    fVisible = !!(pCtrlThis->uchVisibility & SWL_VISIBLE);

                    // WinSendMsg below can block so unlock the list now
                    pgrUnlockWinlist();
                    fLocked = FALSE;

                    if (fVisible)
                    {
                        // check icon
                        HPOINTER hptrNew = (HPOINTER)WinSendMsg(hwnd,
                                                                WM_QUERYICON,
                                                                0,
                                                                0);
                        if (hptrNew != hptrOld)
                        {
                            // icon changed:
                            #ifdef DEBUG_WINDOWLIST
                            _PmpfF(("icon changed hwnd 0x%lX", pCtrlThis->hwnd));
                            #endif

                            if (G_pHookData)        // V1.0.6 (2006-08-04) [pr]
                                WinPostMsg(G_pHookData->hwndDaemonObject,
                                           XDM_ICONCHANGE,
                                           (MPARAM)hwnd,
                                           (MPARAM)hptrNew);
                        }
                    }
                }
            }
        }
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    if (fLocked)
    {
        pgrUnlockWinlist();
        fLocked = FALSE;
    }
}

/*
 *@@ fntWinlistThread:
 *      window list thread started when the first
 *      XDM_ADDWINLISTWATCH comes in.
 *
 *      This scans the system switch list and updates
 *      our private window list in the background,
 *      making sure that everything is always up
 *      to date.
 *
 *      This is required for several reasons:
 *
 *      1)  to get the icons right, since sending
 *          WM_QUERYICON to a frame can block with
 *          misbehaving apps like PMMail;
 *
 *      2)  to detect changes in the switch entries
 *          that are not reflected by messages that
 *          can be intercepted by the hook. For
 *          example, Mozilla creates a minimized
 *          window on startup that is initially
 *          marked visible and switchable (for
 *          whatever reason) and then marked
 *          invisible and not switchable again.
 *          We end up with a dead window in the
 *          XCenter window list otherwise.
 *
 *      This might look very expensive, but it does
 *      not produce a noticeable CPU load on my
 *      system.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 *@@changed V1.0.5 (2006-05-21) [pr]: WarpCenter always in Window List @@fixes 11
 */

VOID _Optlink fntWinlistThread(PTHREADINFO pti)
{
    HWND       hwndObject = WinQueryObjectWindow(HWND_DESKTOP);	// V1.0.5 (2006-05-21) [pr]

    while (!pti->fExit)
    {
        ULONG   cItems,
                cbSwblock;
        PSWBLOCK pSwBlock;
        BOOL fLocked = FALSE;

        if (    (cItems = WinQuerySwitchList(pti->hab, NULL, 0))
             && (cbSwblock = (cItems * sizeof(SWENTRY)) + sizeof(HSWITCH))
             && (pSwBlock = (PSWBLOCK)malloc(cbSwblock))
           )
        {
            if (cItems = WinQuerySwitchList(pti->hab, pSwBlock, cbSwblock))
            {
                // run through all switch list entries
                ULONG ul;
                for (ul = 0;
                     ul < pSwBlock->cswentry;
                     ++ul)
                {
                    // and compare each entry with our private list
                    CheckWindow(pti->hab,
                                &pSwBlock->aswentry[ul].swctl);
                }
            }

            free(pSwBlock);
        }

        if (pti->fExit)
            break;

        // now clean out non-existent windows
        TRY_LOUD(excpt1)
        {
            if (fLocked = pgrLockWinlist())
            {
                PLISTNODE pNode = lstQueryFirstNode(&G_llWinInfos);
                while (pNode)
                {
                    PXWININFO pWinInfo = (PXWININFO)pNode->pItemData;
                    if (    G_pHookData         // V1.0.4 (2005-03-03) [pr]: @@fixes 526
                         && (   !WinIsWindow(pti->hab,
                                             pWinInfo->data.swctl.hwnd)
                             || (WinQueryWindow(pWinInfo->data.swctl.hwnd,	// V1.0.5 (2006-05-21) [pr]
                                                QW_PARENT) == hwndObject)
                            )
                       )
                        WinPostMsg(G_pHookData->hwndDaemonObject,
                                   XDM_WINDOWCHANGE,
                                   (MPARAM)pWinInfo->data.swctl.hwnd,
                                   (MPARAM)WM_DESTROY);

                    pNode = pNode->pNext;
                }
            }
        }
        CATCH(excpt1)
        {
        } END_CATCH();

        if (fLocked)
        {
            pgrUnlockWinlist();
            fLocked = FALSE;
        }

        DosSleep(500);
    }
}


