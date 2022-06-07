
/*
 *@@sourcefile cctl_cnr.c:
 *      implementation for the replacement container control.
 *
 *@@header "helpers\comctl.h"
 *@@added V1.0.1 (2003-01-17) [umoeller]
 */

/*
 *      Copyright (C) 2003 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WININPUT
#define INCL_WINSYS
#define INCL_WINSCROLLBARS
#define INCL_WINSTDCNR

#define INCL_GPIPRIMITIVES
#define INCL_GPILCIDS
#include <os2.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\comctl.h"
#include "helpers\linklist.h"
#include "helpers\gpih.h"
#include "helpers\nls.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\tree.h"
#include "helpers\winh.h"

#include "private\cnr.h"

#pragma hdrstop

/*
 *@@category: Helpers\PM helpers\Window classes\Container control replacement
 *      See cctl_cnr.c.
 */

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

extern const CCTLCOLOR G_scsCnr[] =
    {
            TRUE, PP_BACKGROUNDCOLOR, SYSCLR_WINDOW,
            TRUE, PP_FOREGROUNDCOLOR, SYSCLR_WINDOWTEXT,
            TRUE, PP_HILITEBACKGROUNDCOLOR, SYSCLR_HILITEBACKGROUND,
            TRUE, PP_HILITEFOREGROUNDCOLOR, SYSCLR_HILITEFOREGROUND,
            TRUE, PP_BORDERCOLOR, SYSCLR_WINDOWFRAME,
            TRUE, PP_PAGEBACKGROUNDCOLOR, RGBCOL_WHITE,
            TRUE, PP_PAGEFOREGROUNDCOLOR, RGBCOL_BLACK,
            TRUE, PP_FIELDBACKGROUNDCOLOR, SYSCLR_SCROLLBAR
    };

/* ******************************************************************
 *
 *   Helper funcs
 *
 ********************************************************************/

/*
 *@@ ctnrInit:
 *
 */

VOID ctnrInit(HWND hwnd,
              MPARAM mp2,
              ULONG flMainCnrStyle,     // in: window style bits of main cnr
              PDEFWINDATA pdwd)
{
    ctlInitDWD(hwnd,
               mp2,
               pdwd,
               WinDefWindowProc,
               (flMainCnrStyle & CCS_NOCONTROLPTR)
                    ? CCS_NOSENDCTLPTR
                    : 0,
               G_scsCnr,
               ARRAYITEMCOUNT(G_scsCnr));
}

/*
 *@@ ctnrDrawString:
 *
 */

VOID ctnrDrawString(HPS hps,
                   PCSZ pcsz,              // in: string to test
                   PRECTL prcl,            // in: clipping rectangle (inclusive!)
                   ULONG fl,               // in: alignment flags
                   PFONTMETRICS pfm)
{
    ULONG fl2 = 0;
/*
   #define CFA_LEFT            0x00000001L
   #define CFA_RIGHT           0x00000002L
   #define CFA_CENTER          0x00000004L
   #define CFA_TOP             0x00000008L
   #define CFA_VCENTER         0x00000010L
   #define CFA_BOTTOM          0x00000020L

 *      --  DT_LEFT                    0x00000000
 *      --  DT_CENTER                  0x00000100
 *      --  DT_RIGHT                   0x00000200
 *      --  DT_TOP                     0x00000000
 *      --  DT_VCENTER                 0x00000400
 *      --  DT_BOTTOM                  0x00000800
*/
    if (fl & CFA_RIGHT)
        fl2 = DT_RIGHT;
    else if (fl & CFA_CENTER)
        fl2 = DT_CENTER;

    if (fl & CFA_BOTTOM)
        fl2 = DT_BOTTOM;
    else if (fl & CFA_VCENTER)
        fl2 = DT_VCENTER;

    gpihDrawString(hps,
                   pcsz,
                   prcl,
                   fl2,
                   pfm);
}

/*
 *@@ ctnrGetRecordRect:
 *      returns the rectangle of the given record
 *      converted to window coordinates. Works for
 *      all views.
 */

VOID ctnrGetRecordRect(PCNRDATA pData,
                       PRECTL prcl,
                       const RECORDLISTITEM *prli)
{
    LONG    deltaWorkspace =   (   pData->scrw.szlWorkarea.cy
                                 - pData->dwdContent.szlWin.cy)
                             - pData->scrw.ptlScrollOfs.y;
    prcl->xLeft = 0;
    prcl->xRight = pData->dwdContent.szlWin.cx;
    prcl->yBottom = prli->ptl.y - deltaWorkspace;
    prcl->yTop = prcl->yBottom + prli->szlBox.cy;
}

/*
 *@@ ctnrRepaintRecord:
 *
 */

BOOL ctnrRepaintRecord(PCNRDATA pData,
                         const RECORDLISTITEM *prli)
{
    RECTL   rcl;
    ctnrGetRecordRect(pData, &rcl, prli);
    ++rcl.xRight;       // for separators, if any
    return WinInvalidateRect(pData->dwdContent.hwnd, &rcl, FALSE);
}

/*
 *@@ ctnrQuerySelMode:
 *      returns one of CCS_EXTENDSEL, CCS_MULTIPLESEL, or
 *      CCS_SINGLESEL, depending on the cnr's current view
 *      and window style bits.
 */

ULONG ctnrQuerySelMode(PCNRDATA pData)
{
    // in tree view, there is always only single-sel mode
    if (!(pData->CnrInfo.flWindowAttr & CV_TREE))
    {
        // not tree view: then check window style bits
        ULONG   flStyle = winhQueryWindowStyle(pData->dwdMain.hwnd);
        if (flStyle & CCS_EXTENDSEL)
            return CCS_EXTENDSEL;
        else if (flStyle & CCS_MULTIPLESEL)
            return CCS_MULTIPLESEL;

        // this appears to be what the pm cnr does...
        // the CCS_SINGLESEL is totally superfluous cos
        // if none of the "selection style" bits are
        // set, the cnr operates in "single sel" mode
    }

    return CCS_SINGLESEL;
}

/*
 *@@ ctnrChangeEmphasis:
 *
 */

BOOL ctnrChangeEmphasis(PCNRDATA pData,
                        PRECORDLISTITEM prliSet,
                        BOOL fTurnOn,
                        ULONG fsEmphasis)
{
    PRECORDLISTITEM prliOld;
    ULONG   flRecordAttrOld = prliSet->flRecordAttr;

/* #define CCS_EXTENDSEL             0x00000001L
#define CCS_MULTIPLESEL           0x00000002L
#define CCS_SINGLESEL             0x00000004L */

    ULONG ulSel = ctnrQuerySelMode(pData);

    if (fTurnOn)
    {
        ULONG   flMask;
        if (ulSel == CCS_SINGLESEL)
            flMask = CRA_CURSORED | CRA_SELECTED;
        else
            flMask = CRA_CURSORED;

        if (fsEmphasis & flMask)
        {
            if (prliOld = pData->prliCursored)
            {
                prliOld->flRecordAttr &= ~flMask;
                ctnrRepaintRecord(pData, prliOld);
            }

            pData->prliCursored = prliSet;
        }

        prliSet->flRecordAttr |= fsEmphasis;
    }
    else
        prliSet->flRecordAttr &= ~fsEmphasis;

    if (flRecordAttrOld != prliSet->flRecordAttr)
        ctnrRepaintRecord(pData, prliSet);

    return TRUE;
}

/*
 *@@ ctnrRecordEnter:
 *      helper function when a record gets double-clicked
 *      upon or when "Enter" key gets pressed.
 */

VOID ctnrRecordEnter(PCNRDATA pData,
                     const RECORDLISTITEM *prli,
                     BOOL fKeyboard)
{
    NOTIFYRECORDENTER nre;
    nre.hwndCnr = pData->dwdMain.hwnd;
    nre.fKey = fKeyboard;
    nre.pRecord = (prli) ? (PRECORDCORE)prli->precc : NULL;

    ctlSendWmControl(nre.hwndCnr,
                     CN_ENTER,
                     &nre);
}

/*
 *@@ CreateChild:
 *      creates a container child window.
 */

STATIC HWND CreateChild(PCNRDATA pData,
                        PCSZ pcszClass,
                        ULONG id)
{
    return WinCreateWindow(pData->dwdMain.hwnd,
                           (PSZ)pcszClass,
                           NULL,
                           WS_VISIBLE,
                           0,
                           0,
                           0,
                           0,
                           pData->dwdMain.hwnd,
                           HWND_TOP,
                           id,
                           pData,
                           NULL);
}

/*
 *@@ FindColumnFromFI:
 *
 */

STATIC PDETAILCOLUMN FindColumnFromFI(PCNRDATA pData,
                                      const FIELDINFO *pfi,
                                      PLISTNODE *ppNode)       // out: listnode of column
{
    PLISTNODE pNode;
    FOR_ALL_NODES(&pData->llColumns, pNode)
    {
        PDETAILCOLUMN pCol = (PDETAILCOLUMN)pNode->pItemData;
        if (pCol->pfi == pfi)
        {
            if (ppNode)
                *ppNode = pNode;

            return pCol;
        }
    }

    return NULL;
}

/*
 *@@ ctnrFindListNodeForRecc:
 *
 */

PLISTNODE ctnrFindListNodeForRecc(PCNRDATA pData,
                                  const RECORDCORE *precc)
{
    RECORDTREEITEM  *pti;
    if (pti = (PRECORDTREEITEM)treeFind(pData->RecordsTree,
                                        (ULONG)precc,
                                        treeCompareKeys))
        return pti->pListNode;

    return NULL;
}

VOID SendViewportChanged(PCNRDATA pData)
{
    CNRVIEWPORT cvp;
    cvp.hwndCnr = pData->dwdMain.hwnd;
    cvp.szlWorkarea = pData->scrw.szlWorkarea;
    cvp.szlWin = pData->dwdContent.szlWin;
    cvp.ptlScroll = pData->scrw.ptlScrollOfs;

    ctlSendWmControl(pData->dwdMain.hwnd,
                     CN_VIEWPORTCHANGED,
                     &cvp);
}

/*
 *@@ ctnrUpdateScrollbars:
 *
 */

VOID ctnrUpdateScrollbars(PCNRDATA pData,
                          LONG cx,
                          LONG cy)
{
    BOOL    fNotify = FALSE;

    if (cx && pData->scrw.hwndHScroll)
    {
        winhUpdateScrollBar(pData->scrw.hwndHScroll,
                            cx,
                            pData->scrw.szlWorkarea.cx,
                            pData->scrw.ptlScrollOfs.x,
                            FALSE);
        fNotify = TRUE;
    }

    if (cy && pData->scrw.hwndVScroll)
    {
        winhUpdateScrollBar(pData->scrw.hwndVScroll,
                            cy,
                            pData->scrw.szlWorkarea.cy,
                            pData->scrw.ptlScrollOfs.y,
                            FALSE);
        fNotify = TRUE;
    }

    if (fNotify)
        SendViewportChanged(pData);
}

/* ******************************************************************
 *
 *   Standard window messages
 *
 ********************************************************************/

/*
 *@@ CnrCreate:
 *      implementation for WM_CREATE in fnwpCnr.
 */

STATIC MRESULT CnrCreate(HWND hwnd,
                         MPARAM mp1,
                         MPARAM mp2)
{
    PCNRDATA    pData;

    if (!(pData = NEW(CNRDATA)))
        return (MRESULT)TRUE;       // stop window creation

    WinSetWindowPtr(hwnd, QWL_USER + 1, pData);
    ZERO(pData);

    // initialize DEFWINDOWDATA
    ctnrInit(hwnd,
             mp2,
             ((PCREATESTRUCT)mp2)->flStyle,
             &pData->dwdMain);

    if (((PCREATESTRUCT)mp2)->flStyle & CCS_MINIRECORDCORE)
        pData->fMiniRecords = TRUE;

    // set up non-zero default values in cnrinfo
    pData->CnrInfo.cb = sizeof(CNRINFO);
    pData->CnrInfo.xVertSplitbar = -1;

    lstInit(&pData->llAllocatedFIs,
            TRUE);
    lstInit(&pData->llColumns,
            TRUE);
    lstInit(&pData->llAllocatedRecs,
            TRUE);
    lstInit(&pData->llRootRecords,
            TRUE);

    treeInit(&pData->RecordsTree,
             (PLONG)&pData->CnrInfo.cRecords);

    nlsQueryCountrySettings(&pData->cs);

    winhCreateScroller(hwnd,
                       &pData->scrw,
                       CID_VSCROLL,
                       CID_HSCROLL);

    return (MRESULT)FALSE;
}

/*
 *@@ CnrSem2:
 *      implementation for WM_SEM2 in fnwpCnr.
 *
 *      The DDFL_* semaphore bits get set whenever the
 *      view needs to recompute something. In the worst
 *      case, we resize and/or invalidate the window.
 */

STATIC VOID CnrSem2(PCNRDATA pData,
                    ULONG fl)
{
    HWND    hwnd = pData->dwdMain.hwnd;

    if (pData->CnrInfo.flWindowAttr & CV_DETAIL)
    {
        if (fl & (DDFL_INVALIDATECOLUMNS | DDFL_INVALIDATERECORDS | DDFL_INVALIDATESOME))
        {
            HPS hps = WinGetPS(hwnd);
            cdtlRecalcDetails(pData, hps, &fl);
            WinReleasePS(hps);
        }

        if (fl & (DDFL_WINDOWSIZECHANGED | DDFL_WORKAREACHANGED))
        {
            LONG    y = 0,
                    cx = pData->dwdMain.szlWin.cx,
                    cy = pData->dwdMain.szlWin.cy - pData->cyColTitlesBox;

            if (pData->scrw.hwndVScroll)
                cx -= pData->scrw.cxScrollBar;
            if (pData->scrw.hwndHScroll)
            {
                y += pData->scrw.cyScrollBar;
                cy -= pData->scrw.cyScrollBar;
            }

            if (fl & DDFL_WINDOWSIZECHANGED)
                WinSetWindowPos(pData->dwdContent.hwnd,
                                HWND_TOP,
                                0,
                                y,
                                cx,
                                cy,
                                SWP_MOVE | SWP_SIZE);
                                        // SWP_MOVE is required or PM will move our
                                        // subwindow to some adjustment position

            if (pData->scrw.hwndVScroll)
            {
                WinSetWindowPos(pData->scrw.hwndVScroll,
                                HWND_TOP,
                                cx,
                                y,
                                pData->scrw.cxScrollBar,
                                cy,
                                SWP_MOVE | SWP_SIZE);
            }

            if (pData->scrw.hwndHScroll)
            {
                WinSetWindowPos(pData->scrw.hwndHScroll,
                                HWND_TOP,
                                0,
                                0,
                                cx,
                                pData->scrw.cyScrollBar,
                                SWP_MOVE | SWP_SIZE);
            }

            ctnrUpdateScrollbars(pData,
                                 cx,
                                 cy);
        }
    }
}

/*
 *@@ CnrPaint:
 *      implementation for WM_PAINT in fnwpCnr.
 *
 *      This paints only the part of the container that belongs
 *      to the main container window, which is the title area
 *      and/or the details column headings.
 */

STATIC VOID CnrPaint(PCNRDATA pData)
{
    HPS     hps;
    RECTL   rclPaint;

    if (    (pData)
         && (hps = WinBeginPaint(pData->dwdMain.hwnd, NULLHANDLE, &rclPaint))
       )
    {
        gpihSwitchToRGB(hps);

        if (    (pData->CnrInfo.flWindowAttr & (CV_DETAIL | CA_DETAILSVIEWTITLES))
             == (CV_DETAIL | CA_DETAILSVIEWTITLES)
           )
        {
            PLISTNODE pColNode;

            POINTL  ptl;
            RECTL   rcl;
            LONG    yPadding = pData->CnrInfo.cyLineSpacing / 2;

            // lowest y that we are allowed to paint at
            LONG    yLowest = pData->dwdMain.szlWin.cy - pData->cyColTitlesBox;

            if (rclPaint.yTop > yLowest)
            {
                // clip paint rect so we don't paint into details wnd
                if (rclPaint.yBottom < yLowest)
                    rclPaint.yBottom = yLowest;

                WinFillRect(hps,
                            &rclPaint,
                            ctlQueryColor(&pData->dwdMain, CTLCOL_BGND));

                FOR_ALL_NODES(&pData->llColumns, pColNode)
                {
                    RECTL       rcl2;
                    PDETAILCOLUMN pCol = (PDETAILCOLUMN)pColNode->pItemData;
                    const FIELDINFO *pfi = pCol->pfi;
                    PLISTNODE   pRecNode;
                    ULONG       cRow;

                    rcl.xLeft = pCol->xLeft + COLUMN_PADDING_X - pData->scrw.ptlScrollOfs.x;
                    rcl.xRight = rcl.xLeft + pCol->cxContent;

                    // we start out at the top and work our way down,
                    // decrementing rcl.yTop with every item we painted
                    rcl.yTop = pData->dwdMain.szlWin.cy;

                    rcl.yTop -= COLUMN_PADDING_Y + yPadding;
                    rcl.yBottom =   rcl.yTop
                                  - pData->cyColTitlesContent;

                    if (pfi->flTitle & CFA_BITMAPORICON)
                        // @@todo
                        ;
                    else
                    {
                        ctnrDrawString(hps,
                                      (PCSZ)pfi->pTitleData,
                                      &rcl,
                                      pfi->flTitle,
                                      &pData->fm);
                    }

                    rcl.yBottom -= COLUMN_PADDING_Y + yPadding;

                    if (pfi->flData & CFA_HORZSEPARATOR)
                    {
                        ptl.x = rcl.xLeft;
                        ptl.y = rcl.yBottom;

                        GpiMove(hps,
                                &ptl);
                        ptl.x = rcl.xRight;
                        GpiLine(hps,
                                &ptl);
                    }

                    rcl.yTop = rcl.yBottom;

                } // FOR_ALL_NODES(&pData->llColumns, pColNode)
            }
        }

        WinEndPaint(hps);
    }
}

/*
 *@@ CnrWindowPosChanged:
 *      implementation for WM_WINDOWPOSCHANGED in fnwpCnr.
 */

STATIC MRESULT CnrWindowPosChanged(PCNRDATA pData,
                                   MPARAM mp1,
                                   MPARAM mp2)
{
    MRESULT mrc = 0;

    if (pData)
    {
        mrc = ctlDefWindowProc(&pData->dwdMain, WM_WINDOWPOSCHANGED, mp1, mp2);

        if (((PSWP)mp1)->fl & SWP_SIZE)
        {
            WinPostMsg(pData->dwdMain.hwnd,
                       WM_SEM2,
                       (MPARAM)DDFL_WINDOWSIZECHANGED,
                       0);
        }
    }

    return mrc;
}

/*
 *@@ ctnrPresParamChanged:
 *      implementation for WM_PRESPARAMCHANGED for both
 *      fnwpCnr and fnwpCnrDetails.
 */

VOID ctnrPresParamChanged(HWND hwnd,         // in: either main cnr or content subwindow
                         ULONG ulpp)
{
    PCNRDATA    pData;
    if (pData = (PCNRDATA)WinQueryWindowPtr(hwnd, QWL_USER + 1))
    {
        // note, no matter what hwnd is passed in,
        // we use the dwdMain buffer here cos we share presparams
        // between all cnr child windows
        ctlRefreshColors(&pData->dwdMain);

        switch (ulpp)
        {
            case 0:     // layout palette thing dropped
            case PP_FONTNAMESIZE:
                if (!pData->fSettingPP)
                {
                    pData->fSettingPP = TRUE;

                    if (pData->CnrInfo.flWindowAttr & CV_DETAIL)
                    {
                        // if we got this on the contents window,
                        // set it on the main cnr as well, and
                        // vice versa
                        PSZ pszFont;
                        if (pszFont = winhQueryWindowFont(hwnd))
                        {
                            HWND hwndOther;
                            if (hwnd == pData->dwdMain.hwnd)
                                hwndOther = pData->dwdContent.hwnd;
                            else
                                hwndOther = pData->dwdMain.hwnd;

                            winhSetWindowFont(hwndOther, pszFont);
                            free(pszFont);
                        }

                        WinPostMsg(pData->dwdMain.hwnd,
                                   WM_SEM2,
                                   (MPARAM)(DDFL_INVALIDATECOLUMNS | DDFL_INVALIDATERECORDS),
                                   0);
                    }

                    pData->fSettingPP = FALSE;
                }
            break;

            default:
                // just repaint everything
                WinInvalidateRect(pData->dwdMain.hwnd, NULL, TRUE);
        }
    }
}

/*
 *@@ CnrScroll:
 *      implementation for WM_HSCROLL and WM_VSCROLL in fnwpCnr.
 */

STATIC VOID CnrScroll(PCNRDATA pData,
                      ULONG msg,
                      MPARAM mp1,
                      MPARAM mp2)
{
    if (pData)
    {
        BOOL    fNotify = FALSE;

        if (pData->CnrInfo.flWindowAttr & CV_DETAIL)
        {
            POINTL          ptlScroll;
            NOTIFYSCROLL    ns;
            ns.hwndCnr = pData->dwdMain.hwnd;

            if (msg == WM_HSCROLL)
            {
                ptlScroll.y = 0;
                if (ptlScroll.x = winhHandleScrollMsg(pData->scrw.hwndHScroll,
                                                      &pData->scrw.ptlScrollOfs.x,
                                                      pData->dwdContent.szlWin.cx,
                                                      pData->scrw.szlWorkarea.cx,
                                                      5,
                                                      msg,
                                                      mp2))
                {
                    winhScrollWindow(pData->dwdContent.hwnd,
                                     NULL,
                                     &ptlScroll);

                    // scroll main cnr with detail titles too
                    if (pData->CnrInfo.flWindowAttr & CA_DETAILSVIEWTITLES)
                    {
                        RECTL rclClip;
                        rclClip.xLeft = 0;
                        rclClip.xRight = pData->dwdMain.szlWin.cx;
                        rclClip.yBottom = pData->dwdMain.szlWin.cy - pData->cyColTitlesBox;
                        rclClip.yTop = pData->dwdMain.szlWin.cy;
                        winhScrollWindow(pData->dwdMain.hwnd,
                                         &rclClip,
                                         &ptlScroll);
                    }

                    ns.lScrollInc = ptlScroll.y;
                    ns.fScroll = CMA_HORIZONTAL;        // @@todo details flags
                    ctlSendWmControl(pData->dwdMain.hwnd,
                                     CN_SCROLL,
                                     &ns);

                    fNotify = TRUE;
                }
            }
            else
            {
                ptlScroll.x = 0;
                if (ptlScroll.y = winhHandleScrollMsg(pData->scrw.hwndVScroll,
                                                      &pData->scrw.ptlScrollOfs.y,
                                                      pData->dwdContent.szlWin.cy,
                                                      pData->scrw.szlWorkarea.cy,
                                                      5,
                                                      msg,
                                                      mp2))
                {
                    winhScrollWindow(pData->dwdContent.hwnd,
                                     NULL,
                                     &ptlScroll);

                    ns.lScrollInc = ptlScroll.x;
                    ns.fScroll = CMA_VERTICAL;        // @@todo details flags
                    ctlSendWmControl(pData->dwdMain.hwnd,
                                     CN_SCROLL,
                                     &ns);

                    fNotify = TRUE;
                }
            }
        }

        if (fNotify)
            SendViewportChanged(pData);
    }
}

/*
 *@@ CnrDestroy:
 *      implementation for WM_DESTROY in fnwpCnr.
 */

STATIC VOID CnrDestroy(PCNRDATA pData)
{
    if (pData)
    {
        // free all data that we ever allocated;
        // all these lists are auto-free
        lstClear(&pData->llColumns);
        lstClear(&pData->llAllocatedFIs);
        lstClear(&pData->llAllocatedRecs);
        lstClear(&pData->llRootRecords);

        free(pData);
    }
}

/* ******************************************************************
 *
 *   General container messages
 *
 ********************************************************************/

/*
 *@@ CnrQueryCnrInfo:
 *      implementation for CM_QUERYCNRINFO in fnwpCnr.
 *
 *      Returns no. of bytes copied.
 */

STATIC USHORT CnrQueryCnrInfo(PCNRDATA pData,
                              PCNRINFO pci,        // out: target buffer (mp1 of CM_QUERYCNRINFO)
                              USHORT cb)           // in: mp2 of CM_QUERYCNRINFO
{
    if (pData)
    {
        USHORT cbCopied = max(cb, sizeof(CNRINFO));
        memcpy(pci,
               &pData->CnrInfo,
               cbCopied);
        return cbCopied;
    }

    return 0;
}

/*
 *@@ CnrSetCnrInfo:
 *      implementation for CM_SETCNRINFO in fnwpCnr.
 *
 *      Returns no. of bytes copied.
 */

STATIC BOOL CnrSetCnrInfo(PCNRDATA pData,
                          PCNRINFO pci,        // in: mp1 of CM_SETCNRINFO
                          ULONG flCI)          // in: CMA_* flags in mp2 of CM_SETCNRINFO
{
    if (pData)
    {
        ULONG   flDirty = 0;

        if (flCI & CMA_PSORTRECORD)
            /* Pointer to the comparison function for sorting container records. If NULL,
            which is the default condition, no sorting is performed. Sorting only occurs
            during record insertion and when changing the value of this field. The third
            parameter of the comparison function, pStorage, must be NULL. See
            CM_SORTRECORD for a further description of the comparison function. */
            pData->CnrInfo.pSortRecord = pci->pSortRecord;

        if (flCI & CMA_PFIELDINFOLAST)
            /* Pointer to the last column in the left window of the split details view. The
            default is NULL, causing all columns to be positioned in the left window. */
            pData->CnrInfo.pFieldInfoLast = pci->pFieldInfoLast;

        if (flCI & CMA_PFIELDINFOOBJECT)
            /* Pointer to a column that represents an object in the details view. This
            FIELDINFO structure must contain icons or bit maps. In-use emphasis is applied
            to this column of icons or bit maps only. The default is the leftmost column
            in the unsplit details view, or the leftmost column in the left window of the
            split details view. */
            pData->CnrInfo.pFieldInfoObject = pci->pFieldInfoObject;

        if (flCI & CMA_CNRTITLE)
        {
            // Text for the container title. The default is NULL.
            pData->CnrInfo.pszCnrTitle = pci->pszCnrTitle;

            // @@todo recalc window components, repaint
        }

        if (flCI & CMA_FLWINDOWATTR)
        {
            // Container window attributes.
            if (pData->CnrInfo.flWindowAttr != pci->flWindowAttr)
            {
                HWND    hwndSwitchOwner = NULLHANDLE;

                pData->CnrInfo.flWindowAttr = pci->flWindowAttr;

                // if switching to details view, then create
                // details subwindow
                if (pData->CnrInfo.flWindowAttr & CV_DETAIL)
                {
                    if (!pData->dwdContent.hwnd)
                    {
                        if (pData->dwdContent.hwnd = CreateChild(pData,
                                                                 WC_CCTL_CNR_DETAILS,
                                                                 CID_LEFTDVWND))
                        {
                            flDirty = DDFL_ALL;
                        }
                    }
                }
                else
                {
                    winhDestroyWindow(&pData->dwdContent.hwnd);
                }
            }
        }

        if (flCI & CMA_PTLORIGIN)
        {
            // Lower-left origin of the container window in virtual workspace coordinates,
            // used in the icon view. The default origin is (0,0).
            memcpy(&pData->CnrInfo.ptlOrigin,
                   &pci->ptlOrigin,
                   sizeof(POINTL));

            // @@todo recalc window components, repaint
        }

        if (flCI & CMA_DELTA)
        {
            // An application-defined threshold, or number of records, from either end of
            // the list of available records. Used when a container needs to handle large
            // amounts of data. The default is 0. Refer to the description of the container
            // control in the OS/2 Programming Guide for more information about specifying
            // deltas.
            pData->CnrInfo.cDelta = pci->cDelta;
        }

        if (flCI & CMA_SLBITMAPORICON)
        {
            // The size (in pels) of icons or bit maps. The default is the system size.
            memcpy(&pData->CnrInfo.slBitmapOrIcon,
                   &pci->slBitmapOrIcon,
                   sizeof(SIZEL));

            // @@todo recalc window components, repaint
        }

        if (flCI & CMA_SLTREEBITMAPORICON)
        {
            // The size (in pels) of the expanded and collapsed icons or bit maps in the
            // tree icon and tree text views.
            memcpy(&pData->CnrInfo.slTreeBitmapOrIcon,
                   &pci->slTreeBitmapOrIcon,
                   sizeof(SIZEL));

            // @@todo recalc window components, repaint
        }

        if (flCI & CMA_TREEBITMAP)
        {
            // Expanded and collapsed bit maps in the tree icon and tree text views.
            pData->CnrInfo.hbmExpanded = pci->hbmExpanded;
            pData->CnrInfo.hbmCollapsed = pci->hbmCollapsed;

            // @@todo recalc window components, repaint
        }

        if (flCI & CMA_TREEICON)
        {
            // Expanded and collapsed icons in the tree icon and tree text views.
            pData->CnrInfo.hptrExpanded = pci->hptrExpanded;
            pData->CnrInfo.hptrCollapsed = pci->hptrCollapsed;

            // @@todo recalc window components, repaint
        }

        if (flCI & CMA_LINESPACING)
        {
            // The amount of vertical space (in pels) between the records. If this value is
            // less than 0, a default value is used.
            pData->CnrInfo.cyLineSpacing = pci->cyLineSpacing;

            // @@todo recalc window components, repaint
        }

        if (flCI & CMA_CXTREEINDENT)
        {
            // Horizontal distance (in pels) between levels in the tree view. If this value is
            // less than 0, a default value is used.
            pData->CnrInfo.cxTreeIndent = pci->cxTreeIndent;

            // @@todo recalc window components, repaint
        }

        if (flCI & CMA_CXTREELINE)
        {
            // Width of the lines (in pels) that show the relationship between items in the
            // tree view. If this value is less than 0, a default value is used. Also, if the
            // CA_TREELINE container attribute of the CNRINFO data structure's
            // flWindowAttr field is not specified, these lines are not drawn.
            pData->CnrInfo.cxTreeLine = pci->cxTreeLine;

            // @@todo recalc window components, repaint
        }

        if (flCI & CMA_XVERTSPLITBAR)
        {
            // The initial position of the split bar relative to the container, used in the
            // details view. If this value is less than 0, the split bar is not used. The default
            // value is negative one (-1).
            pData->CnrInfo.xVertSplitbar = pci->xVertSplitbar;

            // @@todo recalc window components, repaint
        }

        if (flDirty)
            // post semaphore to force resize of details wnd
            WinPostMsg(pData->dwdMain.hwnd,
                       WM_SEM2,
                       (MPARAM)flDirty,
                       0);

        return TRUE;
    }

    return FALSE;
}

/*
 *@@ CnrQueryViewportRect:
 *      implementation for CM_QUERYVIEWPORTRECT in fnwpCnr.
 *
 *      From my testing, this simply returns the extensions
 *      of the container content window, which the cnr docs
 *      dub the "viewport". This never returns the workarea
 *      size, that is, the total size of the container's
 *      viewable content.
 */

STATIC BOOL CnrQueryViewportRect(PCNRDATA pData,
                                 PRECTL prcl,
                                 USHORT usIndicator,
                                 BOOL fRightSplitView)      // @@todo
{
    if (pData && prcl)
    {
        prcl->xLeft = 0;
        prcl->yBottom = 0;
        prcl->xRight = pData->dwdContent.szlWin.cx;
        prcl->yTop = pData->dwdContent.szlWin.cy;

        switch (usIndicator)
        {
            case CMA_WORKSPACE:
                // for CMA_WORKSPACE, the PM cnr returns a 0 yBottom when
                // the cnr is scrolled to the very top and negative y
                // values when it has been scrolled down to some extent;
                // wonder what the use for this would be
                prcl->xLeft += pData->scrw.ptlScrollOfs.x;
                prcl->xRight += pData->scrw.ptlScrollOfs.x;
                prcl->yBottom -= pData->scrw.ptlScrollOfs.y;
                prcl->yTop -= pData->scrw.ptlScrollOfs.y;
            break;

            case CMA_WINDOW:
                // for CMA_WINDOW, the PM cnr returns a constant
                // rectangle without scrolling taken into account
                WinMapWindowPoints(pData->dwdContent.hwnd,
                                   pData->dwdMain.hwnd,
                                   (PPOINTL)prcl,
                                   2);
            break;
        }

        return TRUE;
    }

    return FALSE;
}

/* ******************************************************************
 *
 *   FIELDINFO-related messages
 *
 ********************************************************************/

/*
 *@@ CnrAllocDetailFieldInfo:
 *      implementation for CM_ALLOCDETAILFIELDINFO in fnwpCnr.
 *
 *      Returns: PFIELDINFO linked list or NULL.
 */

STATIC PFIELDINFO CnrAllocDetailFieldInfo(PCNRDATA pData,
                                          USHORT cFieldInfos)      // in: no. of fieldinfos to allocate (> 0)
{
    PFIELDINFO  pfiFirst = NULL,
                pfiPrev = NULL;

    if (pData)
    {
        ULONG       ul;
        for (ul = 0;
             ul < cFieldInfos;
             ++ul)
        {
            // we must allocate each one separately or we cannot
            // free them separately with CM_FREEDETAILFIELDINFO
            PFIELDINFO  pfiThis;
            if (pfiThis = NEW(FIELDINFO))
            {
                ZERO(pfiThis);

                // link into list
                if (!pfiPrev)
                    // first round:
                    pfiFirst = pfiThis;
                else
                    pfiPrev->pNextFieldInfo = pfiThis;

                // put into private linklist
                lstAppendItem(&pData->llAllocatedFIs,
                              pfiThis);

                pfiPrev = pfiThis;
            }
        }
    }

    return pfiFirst;
}

/*
 *@@ CnrInsertDetailFieldInfo:
 *      implementation for CM_INSERTDETAILFIELDINFO in fnwpCnr.
 */

STATIC USHORT CnrInsertDetailFieldInfo(PCNRDATA pData,
                                       PFIELDINFO pfiFirst,
                                       PFIELDINFOINSERT pfii)
{
    USHORT      usrc = 0;

    if (    (pData)
         && (pfiFirst)
         && (pfii)
         && (pfii->cb = sizeof(FIELDINFOINSERT))
       )
    {
        ULONG       ul;
        PFIELDINFO  pfiThis = pfiFirst;
        PLISTNODE   pNodeInsertAfter;

        usrc = lstCountItems(&pData->llColumns);

        switch ((ULONG)pfii->pFieldInfoOrder)
        {
            case CMA_FIRST:
                pNodeInsertAfter = NULL;        // first
            break;

            case CMA_END:
                pNodeInsertAfter = lstQueryLastNode(&pData->llColumns);
                                            // can return NULL also
            break;

            default:
                if (!FindColumnFromFI(pData,
                                      pfii->pFieldInfoOrder,
                                      &pNodeInsertAfter))
                    pNodeInsertAfter = NULL;
        }

        for (ul = 0;
             ul < pfii->cFieldInfoInsert;
             ++ul)
        {
            PDETAILCOLUMN pdc;
            if (pdc = NEW(DETAILCOLUMN))
            {
                ZERO(pdc);

                pdc->pfi = pfiThis;

                if (pNodeInsertAfter = lstInsertItemAfterNode(&pData->llColumns,
                                                              pdc,
                                                              pNodeInsertAfter))
                {
                    ++usrc;
                    ++pData->CnrInfo.cFields;
                    pfiThis = pfiThis->pNextFieldInfo;
                    continue;
                }
            }

            free(pdc);

            usrc = 0;
            break;

        } // for (ul = 0; ul < pfii->cFieldInfoInsert; ...

        if (    (usrc)
             && (pfii->fInvalidateFieldInfo)
           )
        {
            // post semaphore to force resize of details wnd
            WinPostMsg(pData->dwdMain.hwnd,
                       WM_SEM2,
                       (MPARAM)DDFL_ALL,
                       0);
        }
    }

    return usrc;
}

/*
 *@@ CnrInvalidateDetailFieldInfo:
 *      implementation for CM_INVALIDATEDETAILFIELDINFO in fnwpCnr.
 */

STATIC BOOL CnrInvalidateDetailFieldInfo(PCNRDATA pData)
{
    if (pData)
        return WinPostMsg(pData->dwdMain.hwnd,
                          WM_SEM2,
                          (MPARAM)DDFL_INVALIDATECOLUMNS,
                          0);

    return FALSE;
}

/*
 *@@ CnrQueryDetailFieldInfo:
 *      implementation for CM_QUERYDETAILFIELDINFO in fnwpCnr.
 */

STATIC const FIELDINFO* CnrQueryDetailFieldInfo(PCNRDATA pData,
                                                PFIELDINFO pfiIn,
                                                USHORT cmd)        // in: mp2 (CMA_FIRST, CMA_LAST, CMA_NEXT, CMA_PREV)
{
    const FIELDINFO *pfiReturn = NULL;

    if (pData)
    {
        PLISTNODE pNode;

        switch (cmd)
        {
            case CMA_FIRST:
                pNode = lstQueryFirstNode(&pData->llColumns);
            break;

            case CMA_LAST:
                pNode = lstQueryLastNode(&pData->llColumns);
            break;

            case CMA_NEXT:
            case CMA_PREV:
                if (FindColumnFromFI(pData,
                                     pfiIn,
                                     &pNode))
                    if (cmd == CMA_NEXT)
                        pNode = pNode->pNext;
                    else
                        pNode = pNode->pPrevious;
            break;
        }

        if (pNode)
            pfiReturn = ((PDETAILCOLUMN)pNode->pItemData)->pfi;
    }

    return pfiReturn;
}

/*
 *@@ CnrRemoveDetailFieldInfo:
 *      implementation for CM_REMOVEDETAILFIELDINFO in fnwpCnr.
 */

STATIC SHORT CnrRemoveDetailFieldInfo(PCNRDATA pData,
                                      PFIELDINFO* ppafi,
                                      USHORT cfi,
                                      USHORT fl)
{
    if (pData)
    {
        SHORT   rc = lstCountItems(&pData->llColumns);
        ULONG   fAnythingFound = FALSE,
                ul;

        for (ul = 0;
             ul < cfi;
             ++ul)
        {
            PDETAILCOLUMN pCol;
            PLISTNODE pNodeCol;
            if (!(pCol = FindColumnFromFI(pData,
                                          ppafi[ul],
                                          &pNodeCol)))
            {
                rc = -1;
                break;
            }

            // found:
            lstRemoveNode(&pData->llColumns,
                          pNodeCol);            // auto-free, so this frees the DETAILCOLUMN

            if (fl & CMA_FREE)
                lstRemoveItem(&pData->llAllocatedFIs,
                              ppafi[ul]);       // auto-free, so this frees the FIELDINFO

            fAnythingFound = TRUE;

            --rc;
            --pData->CnrInfo.cFields;
        }

        if (    (fAnythingFound)
             && (fl & CMA_INVALIDATE)
           )
        {
            WinPostMsg(pData->dwdMain.hwnd,
                       WM_SEM2,
                       (MPARAM)DDFL_INVALIDATECOLUMNS,
                       0);
        }

        return rc;
    }

    return -1;
}

/*
 *@@ CnrFreeDetailFieldInfo:
 *      implementation for CM_FREEDETAILFIELDINFO in fnwpCnr.
 */

STATIC BOOL CnrFreeDetailFieldInfo(PCNRDATA pData,
                                   PFIELDINFO *ppafi,      // in: mp1 of CM_FREEDETAILFIELDINFO
                                   USHORT cFieldInfos)     // in: no. of items in array
{
    BOOL        brc = FALSE;

    if (pData)
    {
        ULONG   ul;

        // @@todo return FALSE if the FI is currently inserted

        if (1)
        {
            for (ul = 0;
                 ul < cFieldInfos;
                 ++ul)
            {
                PFIELDINFO pfiThis = ppafi[ul];
                lstRemoveItem(&pData->llAllocatedFIs,
                              pfiThis);
            }

            brc = TRUE;
        }
    }

    return brc;
}

/* ******************************************************************
 *
 *   Record insertion/removal
 *
 ********************************************************************/

/*
 *@@ CnrAllocRecord:
 *      implementation for CM_ALLOCRECORD in fnwpCnr.
 */

STATIC PRECORDCORE CnrAllocRecord(PCNRDATA pData,
                                  ULONG cbExtra,
                                  USHORT cRecords)
{
    PRECORDCORE preccFirst = NULL;

    if (pData)
    {
        ULONG   ul;
        ULONG   cbAlloc = (   (pData->fMiniRecords)
                            ? sizeof(MINIRECORDCORE)
                            : sizeof(RECORDCORE)
                          ) + cbExtra;

        PRECORDCORE preccPrev = NULL;

        for (ul = 0;
             ul < cRecords;
             ++ul)
        {
            PRECORDCORE preccThis;
            if (!(preccThis = (PRECORDCORE)malloc(cbAlloc)))
            {
                preccFirst = NULL;
                break;
            }

            memset(preccThis, 0, cbAlloc);

            preccThis->cb = cbAlloc;

            // link into list
            if (!preccPrev)
                // first round:
                preccFirst = preccThis;
            else
                preccPrev->preccNextRecord = preccThis;

            // put into private linklist
            lstAppendItem(&pData->llAllocatedRecs,
                          preccThis);

            preccPrev = preccThis;
        }
    }

    return preccFirst;
}

/*
 *@@ CnrInsertRecord:
 *      implementation for CM_INSERTRECORD in fnwpCnr.
 */

STATIC ULONG CnrInsertRecord(PCNRDATA pData,
                             PRECORDCORE preccFirst,
                             PRECORDINSERT pri)
{
    ULONG       cReturn = 0;

    if (    (pData)
         && (preccFirst)
         && (pri)
         && (pri->cb = sizeof(RECORDINSERT))
       )
    {
        PRECORDCORE preccThis = preccFirst;
        ULONG ul;
        PLINKLIST   pll;
        PLISTNODE   pNodeInsertAfter;

        cReturn = lstCountItems(&pData->llRootRecords);

        if (pri->pRecordParent)
        {
            // @@todo
        }
        else
            // insert at root:
            pll = &pData->llRootRecords;

        switch ((ULONG)pri->pRecordOrder)
        {
            case CMA_FIRST:
                pNodeInsertAfter = NULL;        // first
            break;

            case CMA_END:
                pNodeInsertAfter = lstQueryLastNode(pll);
                                            // can return NULL also
            break;

            default:
                pNodeInsertAfter = ctnrFindListNodeForRecc(pData,
                                                           pri->pRecordOrder);
        }

        for (ul = 0;
             ul < pri->cRecordsInsert;
             ++ul)
        {
            PRECORDLISTITEM prli;

            if (prli = NEW(RECORDLISTITEM))
            {
                ZERO(prli);

                prli->precc = preccThis;
                prli->preccParent = pri->pRecordParent;

                // make private copy of record attributes
                prli->flRecordAttr = preccThis->flRecordAttr;

                // PM container gives the first record in the cnr
                // "cursored" and "selected" emphasis, so that's
                // what we'll do too
                if (    (!cReturn)       // @@todo filtered records
                     && (!(prli->flRecordAttr & CRA_FILTERED))
                   )
                {
                    prli->flRecordAttr |= CRA_CURSORED | CRA_SELECTED;

                    pData->prliCursored = prli;
                }

                if (pNodeInsertAfter = lstInsertItemAfterNode(pll,
                                                              prli,
                                                              pNodeInsertAfter))
                {
                    PRECORDTREEITEM pTreeItem;

                    if (pTreeItem = NEW(RECORDTREEITEM))
                    {
                        ZERO(pTreeItem);

                        pTreeItem->Tree.ulKey = (ULONG)preccThis;
                        pTreeItem->pListNode = pNodeInsertAfter;    // newly created list node

                        // the following will fail if the record
                        // is already inserted!
                        if (!treeInsert(&pData->RecordsTree,
                                        (PLONG)&pData->CnrInfo.cRecords,
                                        (TREE*)pTreeItem,
                                        treeCompareKeys))
                        {
                            ++cReturn;
                            preccThis = preccThis->preccNextRecord;
                            continue;
                        }

                        free(pTreeItem);
                    }

                    lstRemoveNode(pll,
                                  pNodeInsertAfter);
                }

                free(prli);
            }

            free(prli);

            cReturn = 0;
            break;      // for
        } // for (ul = 0; ul < pri->cRecordsInsert; ...

        if (    (cReturn)
             && (pri->fInvalidateRecord)
           )
        {
            WinPostMsg(pData->dwdMain.hwnd,
                       WM_SEM2,
                       (MPARAM)DDFL_INVALIDATERECORDS,
                       0);
        }
    }

    return cReturn;
}

/*
 *@@ CnrInsertRecordArray:
 *      implementation for CM_INSERTRECORDARRAY in fnwpCnr.
 */

STATIC ULONG CnrInsertRecordArray(PCNRDATA pData,
                                  PRECORDCORE *papRecords,
                                  PRECORDINSERT pri)
{
    ULONG       cReturn = 0;

    if (    (pData)
         && (papRecords)
         && (pri)
         && (pri->cb = sizeof(RECORDINSERT))
       )
    {
        // produce a linked list off the array and call
        // the CM_INSERTRECORD implementation
        ULONG   ul;
        for (ul = 0;
             ul < (pri->cRecordsInsert - 1);
             ++ul)
        {
            papRecords[ul]->preccNextRecord = papRecords[ul + 1];
        }

        papRecords[pri->cRecordsInsert - 1]->preccNextRecord = (PRECORDCORE)NULL;

        cReturn = CnrInsertRecord(pData,
                                  papRecords[0],
                                  pri);
    }

    return cReturn;
}

/*
 *@@ CnrSetRecordEmphasis:
 *      implementation for CM_SETRECORDEMPHASIS in fnwpCnr.
 */

STATIC BOOL CnrSetRecordEmphasis(PCNRDATA pData,
                                 PRECORDCORE precc,
                                 BOOL fTurnOn,
                                 USHORT fsEmphasis)
{
    BOOL        brc = FALSE;

    if (pData)
    {
        if (precc)
        {
            PLISTNODE   pNode;
            if (pNode = ctnrFindListNodeForRecc(pData,
                                                precc))
            {
                PRECORDLISTITEM prli = (PRECORDLISTITEM)pNode->pItemData;

                ctnrChangeEmphasis(pData,
                                   prli,
                                   fTurnOn,
                                   fsEmphasis);

                // update caller's buffer too
                precc->flRecordAttr = prli->flRecordAttr;

                brc = TRUE;
            }
        } // if (precc)
        // @@todo else set emphasis on entire cnr
    }

    return brc;
}

/*
 *@@ CnrQueryRecordEmphasis:
 *      implementation for CM_QUERYRECORDEMPHASIS in fnwpCnr.
 *
 *      Note, if several flags are set in fsEmphasis, all
 *      of them must be set in the record to match.
 */

PRECORDCORE CnrQueryRecordEmphasis(PCNRDATA pData,
                                   PRECORDCORE preccSearchAfter,
                                   USHORT fsEmphasis)
{
    if (pData)
    {
        PLISTNODE pNode = NULL;
        if (preccSearchAfter == (PRECORDCORE)CMA_FIRST)
            pNode = lstQueryFirstNode(&pData->llRootRecords);
        else
            if (pNode = ctnrFindListNodeForRecc(pData, preccSearchAfter))
                pNode = pNode->pNext;
            else
                return (PRECORDCORE)-1;
        // @@todo how does this search tree subrecords?

        while (pNode)
        {
            PRECORDLISTITEM prli = (PRECORDLISTITEM)pNode->pItemData;
            if ((prli->flRecordAttr & fsEmphasis) == fsEmphasis)
                return (PRECORDCORE)prli->precc;

            pNode = pNode->pNext;
        }
    }

    return NULL;
}

/*
 *@@ CnrInvalidateRecord:
 *      implementation for CM_INVALIDATERECORD in fnwpCnr.
 *
 */

STATIC BOOL CnrInvalidateRecord(PCNRDATA pData,
                                PRECORDCORE *papRecs,
                                USHORT cRecs,
                                USHORT fsInvalidate)
{
    BOOL    brc = TRUE;

    if (pData)
    {
        if (    (!papRecs)
             || (!cRecs)
           )
            // invalidate all:
            CnrSem2(pData, DDFL_INVALIDATERECORDS);
        else
        {
            ULONG   ul;
            for (ul = 0;
                 ul < cRecs;
                 ++cRecs)
            {
                PRECORDCORE precc = papRecs[ul];
                PLISTNODE pRecNode;
                if (!(pRecNode = ctnrFindListNodeForRecc(pData,
                                                         precc)))
                {
                    brc = FALSE;
                    break;
                }

                // set special flag for recompute
                ((PRECORDLISTITEM)pRecNode->pItemData)->flInvalidate = fsInvalidate;
            }

            if (brc)
                CnrSem2(pData, DDFL_INVALIDATESOME);
                // @@todo optimize: post sem only if a column size has
                // actually changed
        }
    }

    return brc;
}

/* ******************************************************************
 *
 *   Container window proc
 *
 ********************************************************************/

/*
 *@@ fnwpCnr:
 *
 */

MRESULT EXPENTRY fnwpCnr(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT     mrc = 0;

    PCNRDATA    pData = (PCNRDATA)WinQueryWindowPtr(hwnd, QWL_USER + 1);

    switch (msg)
    {
        /* ******************************************************************
         *
         *   Standard window messages
         *
         ********************************************************************/

        case WM_CREATE:
            mrc = CnrCreate(hwnd, mp1, mp2);
        break;

        case WM_SEM2:
            CnrSem2(pData, (ULONG)mp1);
        break;

        case WM_PAINT:
            CnrPaint(pData);
        break;

        case WM_WINDOWPOSCHANGED:
            mrc = CnrWindowPosChanged(pData, mp1, mp2);
        break;

        case WM_SYSCOLORCHANGE:
            ctnrPresParamChanged(hwnd, 0);
        break;

        case WM_PRESPARAMCHANGED:
            ctnrPresParamChanged(hwnd, (ULONG)mp1);
        break;

        case WM_HSCROLL:
        case WM_VSCROLL:
            CnrScroll(pData, msg, mp1, mp2);
        break;

        case WM_DESTROY:
            CnrDestroy(pData);
        break;

        /* ******************************************************************
         *
         *   Mouse and keyboard input
         *
         ********************************************************************/

        /*
         * WM_OPEN:
         *      when the user double-clicks on the _main_ cnr
         *      (i.e. details titles), we notify the owner
         *      of a "whitespace" enter event.
         */

        case WM_OPEN:
            ctnrRecordEnter(pData,
                            NULL,
                            FALSE);     // mouse, not keyboard
            mrc = (MRESULT)TRUE;
        break;

        /* ******************************************************************
         *
         *   General container messages
         *
         ********************************************************************/

        /*
         * CM_QUERYCNRINFO:
         *
         *      Parameters:
         *
         *      --  PCNRINFO mp1: buffer to copy to.
         *      --  USHORT mp2: size of buffer.
         *
         *      Returns no. of bytes copied or 0 on errors.
         */

        case CM_QUERYCNRINFO:       // done
            mrc = (MRESULT)CnrQueryCnrInfo(pData,
                                           (PCNRINFO)mp1,
                                           SHORT1FROMMP(mp2));
        break;

        /*
         * CM_SETCNRINFO:
         *
         *      Parameters:
         *
         *      --  PCNRINFO mp1: buffer to copy fields from.
         *
         *      --  ULONG fl: CMA_* flags for fields that changed.
         *
         *      Returns BOOL.
         */

        case CM_SETCNRINFO:         // parlty done
            mrc = (MRESULT)CnrSetCnrInfo(pData,
                                         (PCNRINFO)mp1,
                                         (ULONG)mp2);
        break;

        case CM_PAINTBACKGROUND:        // @@todo
        break;

        case CM_SCROLLWINDOW:           // @@todo
        break;

        /*
         * CM_QUERYVIEWPORTRECT:
         *
         *      Parameters:
         *
         *      --  PRECTL mp1
         *
         *      --  SHORT1FROMMP(mp2): -- CMA_WINDOW: return window coordinates
         *                             -- CMA_WORKSPACE: return workspace coordinates
         *
         *      --  BOOL SHORT2FROMMP(mp2): if TRUE, return right split details view
         */

        case CM_QUERYVIEWPORTRECT:          // done
            mrc = (MRESULT)CnrQueryViewportRect(pData,
                                                (PRECTL)mp1,
                                                SHORT1FROMMP(mp2),
                                                SHORT2FROMMP(mp2));
        break;

        case CM_SETTEXTVISIBILITY:      // @@todo
        break;

        /* ******************************************************************
         *
         *   Record allocation/insertion/removal
         *
         ********************************************************************/

        /*
         * CM_ALLOCRECORD:
         *
         *      Parameters:
         *
         *      --  ULONG mp1: record size in addition to (MINI)RECORDCORE size.
         *
         *      --  USHORT mp2: no. of records to allocate.
         *
         *      Returns linked list of RECORDCORE's or NULL on errors.
         */

        case CM_ALLOCRECORD:          // done
            mrc = (MRESULT)CnrAllocRecord(pData,
                                          (ULONG)mp1,
                                          SHORT1FROMMP(mp2));
        break;

        /*
         * CM_INSERTRECORD:
         *      inserts one or more records. If there's more
         *      than one record, we assume it's a linked list.
         *
         *      Parameters:
         *
         *      --  PRECORDCORE mp1: first record
         *
         *      --  PRECORDINSERT pri
         *
         *      Returns the no. of records in the container or 0 on errors.
         */

        case CM_INSERTRECORD:           // done
            mrc = (MRESULT)CnrInsertRecord(pData,
                                           (PRECORDCORE)mp1,
                                           (PRECORDINSERT)mp2);
        break;

        /*
         * CM_INSERTRECORDARRAY:
         *      inserts one or more records. As opposed to with
         *      CM_INSERTRECORD, mp1 points to an array of
         *      record pointers instead of to a linked list
         *      of records.
         *
         *      Parameters:
         *
         *      --  PRECORDCORE mp1: first record
         *
         *      --  PRECORDINSERT pri
         *
         *      Returns the no. of records in the container or 0 on errors.
         */

        case CM_INSERTRECORDARRAY:      // done
            mrc = (MRESULT)CnrInsertRecordArray(pData,
                                               (PRECORDCORE*)mp1,
                                               (PRECORDINSERT)mp2);
        break;

        /*
         * CM_QUERYRECORD:
         *
         *      Parameters:
         *
         *      --  PRECORDCORE mp1: preccSearch
         *
         *      --  SHORT1FROMMP(mp1): CMA_FIRST, CMA_LAST, CMA_NEXT, CMA_PREV
         *
         *          or for tree views: CMA_FIRSTCHILD, CMA_LASTCHILD, CMA_PARENT
         *
         *      --  SHORT2FROMMP(mp1): CMA_ITEMORDER or CMA_ZORDER
         */

        case CM_QUERYRECORD:            // @@todo
        break;

        /*
         * CM_SETRECORDEMPHASIS:
         *
         *      Parameters:
         *
         *      --  PRECORDCORE mp1: record to change emphasis for.
         *
         *      --  SHORT1FROMMP(mp2): TRUE == turn flags on, FALSE == turn flags off.
         *
         *      --  SHORT2FROMMP(mp2): any combination of CRA_CURSORED, CRA_DISABLED,
         *          CRA_INUSE, CRA_PICKED, CRA_SELECTED, CRA_SOURCE
         *
         *      Returns BOOL.
         */

        case CM_SETRECORDEMPHASIS:
            mrc = (MRESULT)CnrSetRecordEmphasis(pData,
                                                (PRECORDCORE)mp1,
                                                SHORT1FROMMP(mp2),
                                                SHORT2FROMMP(mp2));
        break;

        /*
         * CM_QUERYRECORDEMPHASIS:
         *
         *      Parameters:
         *
         *      --  PRECORDCORE mp1: record after which to start search
         *          or NULL to start search from beginning.
         *
         *      --  USHORT mp2: any combination of CRA_COLLAPSED, CRA_CURSORED,
         *          CRA_DISABLED, CRA_DROPONABLE, CRA_EXPANDED, CRA_FILTERED,
         *          CRA_INUSE, CRA_PICKED, CRA_SELECTED, CRA_SOURCE
         */

        case CM_QUERYRECORDEMPHASIS:    // done
            mrc = (MRESULT)CnrQueryRecordEmphasis(pData,
                                                  (PRECORDCORE)mp1,
                                                  SHORT1FROMMP(mp2));
        break;

        case CM_QUERYRECORDFROMRECT:    // @@todo
        break;

        case CM_QUERYRECORDINFO:        // @@todo
        break;

        case CM_QUERYRECORDRECT:        // @@todo
        break;

        /*
         * CM_INVALIDATERECORD:
         *
         *      Parameters:
         *
         *      --  PRECORDCORE* mp1: ptr to array of record pointers
         *
         *      --  SHORT1FROMMP(mp2): no. of records in array
         *
         *      --  SHORT2FROMMP(mp2): CMA_ERASE, CMA_REPOSITION,
         *          CMA_NOREPOSITION, CMA_TEXTCHANGED
         *
         *      Returns BOOL.
         */

        case CM_INVALIDATERECORD:       // done
            mrc = (MRESULT)CnrInvalidateRecord(pData,
                                               (PRECORDCORE*)mp1,
                                               SHORT1FROMMP(mp2),
                                               SHORT2FROMMP(mp2));
        break;

        case CM_REMOVERECORD:   // @@todo
        case CM_FREERECORD:     // @@todo
        break;

        case CM_ERASERECORD:    // @@todo
        break;

        case CM_ARRANGE:        // @@todo
        break;

        case CM_FILTER:         // @@todo
        break;

        case CM_QUERYDRAGIMAGE: // @@todo

        case CM_SEARCHSTRING:

        case CM_SORTRECORD:     // @@todo

        /* ******************************************************************
         *
         *   Details view
         *
         ********************************************************************/

        /*
         * CM_ALLOCDETAILFIELDINFO:
         *
         *      Parameters:
         *
         *      --  USHORT mp1: no. of fieldinfos to allocate
         *      --  mp2: reserved
         *
         *      Returns PFIELDINFO linked list of fieldinfos,
         *      or NULL on errors.
         */

        case CM_ALLOCDETAILFIELDINFO: // done
            mrc = (MRESULT)CnrAllocDetailFieldInfo(pData,
                                                   SHORT1FROMMP(mp1));
        break;

        /*
         * CM_INSERTDETAILFIELDINFO:
         *
         *      Parameters:
         *
         *      --  PFIELDINFO mp1
         *
         *      --  PFIELDINFOINSERT mp2
         *
         *      Returns the no. of FI's in the cnr or 0 on errors.
         */

        case CM_INSERTDETAILFIELDINFO:      // done
            mrc = (MRESULT)CnrInsertDetailFieldInfo(pData,
                                                    (PFIELDINFO)mp1,
                                                    (PFIELDINFOINSERT)mp2);
        break;

        /*
         * CM_INVALIDATEDETAILFIELDINFO:
         *      No parameters.
         *
         *      Returns BOOL.
         */

        case CM_INVALIDATEDETAILFIELDINFO:  // done
            mrc = (MRESULT)CnrInvalidateDetailFieldInfo(pData);
        break;

        /*
         * CM_QUERYDETAILFIELDINFO:
         *
         *      Parameters:
         *
         *      --  PFIELDINFO mp1
         *
         *      --  USHORT mp2: CMA_FIRST, CMA_LAST, CMA_NEXT, CMA_PREV
         */

        case CM_QUERYDETAILFIELDINFO:       // done
            mrc = (MRESULT)CnrQueryDetailFieldInfo(pData,
                                                   (PFIELDINFO)mp1,
                                                   SHORT1FROMMP(mp2));
        break;

        /*
         * CM_REMOVEDETAILFIELDINFO:
         *
         *      Parameters:
         *
         *      --  PFIELDINFO* mp1: ptr to array of PFIELDINFO's
         *
         *      --  SHORT1FROMMP(mp1): no. of fieldinfos in array
         *
         *      --  SHORT2FROMMP(mp2): flRemove (CMA_FREE, CMA_INVALIDATE)
         *
         *      Returns the no. of FI's in the cnr or -1 on errors.
         */

        case CM_REMOVEDETAILFIELDINFO:      // done
            mrc = (MRESULT)CnrRemoveDetailFieldInfo(pData,
                                                    (PFIELDINFO*)mp1,
                                                    SHORT1FROMMP(mp2),
                                                    SHORT2FROMMP(mp2));
        break;

        /*
         * CM_FREEDETAILFIELDINFO:
         *
         *      Paramters:
         *
         *      --  PFIELDINFO* mp1: ptr to array of PFIELDINFO's
         *      --  USHORT mp2: no. of ptrs in array
         *
         *      Returns BOOL.
         */

        case CM_FREEDETAILFIELDINFO:        // done
            mrc = (MRESULT)CnrFreeDetailFieldInfo(pData,
                                                  (PFIELDINFO*)mp1,
                                                  SHORT1FROMMP(mp2));
        break;

        case CM_HORZSCROLLSPLITWINDOW:
        break;

        /* ******************************************************************
         *
         *   Icon view
         *
         ********************************************************************/

        case CM_SETGRIDINFO:    // @@todo
        case CM_QUERYGRIDINFO:  // @@todo
        case CM_SNAPTOGRID:     // @@todo
        break;

        /* ******************************************************************
         *
         *   Tree management
         *
         ********************************************************************/

        case CM_COLLAPSETREE:   // @@todo
        case CM_EXPANDTREE:
        case CM_MOVETREE:       // @@todo
        break;

        /* ******************************************************************
         *
         *   Direct editing
         *
         ********************************************************************/

        case CM_OPENEDIT:       // @@todo

        case CM_CLOSEEDIT:      // @@todo
        break;


        default:
            if (pData)
                mrc = ctlDefWindowProc(&pData->dwdMain, msg, mp1, mp2);
        break;

    }

    return mrc;
}

/*
 *@@ ctlRegisterXCnr:
 *
 */

BOOL ctlRegisterXCnr(HAB hab)
{
    return (    WinRegisterClass(hab,
                                 WC_CCTL_CNR,
                                 fnwpCnr,
                                 0, // CS_SYNCPAINT, // CS_CLIPSIBLINGS CS_CLIPCHILDREN
                                 sizeof(PVOID) * 2)
             && WinRegisterClass(hab,
                                 WC_CCTL_CNR_DETAILS,
                                 fnwpCnrDetails,
                                 0, // CS_SYNCPAINT, //  | CS_PARENTCLIP, // CS_CLIPSIBLINGS CS_CLIPCHILDREN
                                 sizeof(PVOID) * 2)
           );
}

