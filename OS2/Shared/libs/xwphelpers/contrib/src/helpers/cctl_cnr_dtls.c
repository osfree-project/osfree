
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

#define INCL_DOSMISC

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINSYS
#define INCL_WININPUT
#define INCL_WINSTDCNR
#define INCL_GPIPRIMITIVES
#define INCL_GPILCIDS
#include <os2.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/comctl.h"
#include "helpers/linklist.h"
#include "helpers/gpih.h"
#include "helpers/nls.h"
#include "helpers/standards.h"
#include "helpers/stringh.h"
#include "helpers/tree.h"
#include "helpers/winh.h"

#include "private/cnr.h"

#pragma hdrstop

/*
 *@@category: Helpers\PM helpers\Window classes\Container control replacement
 *      See cctl_cnr.c.
 */

/* ******************************************************************
 *
 *   Helper funcs
 *
 ********************************************************************/

/*
 *@@ InvalidateColumn:
 *
 */

VOID InvalidateColumn(PCNRDATA pData,
                      const DETAILCOLUMN *pColumn)
{
    RECTL   rcl;
    rcl.xLeft = pColumn->xLeft - pData->scrw.ptlScrollOfs.x;
    rcl.xRight = rcl.xLeft + pColumn->cxContent + 2 * COLUMN_PADDING_X + 1;
    rcl.yBottom = 0;
    rcl.yTop = pData->dwdContent.szlWin.cy;

    WinInvalidateRect(pData->dwdContent.hwnd, &rcl, FALSE);

    // and titles too
    rcl.yTop = pData->dwdMain.szlWin.cy;
    WinInvalidateRect(pData->dwdMain.hwnd, &rcl, FALSE);
}

/*
 *@@ cdtlRecalcDetails:
 *      worker routine for refreshing all internal DETAILCOLUMN
 *      data when important stuff has changed.
 *
 *      This only gets called from CnrSem2 when WM_SEM2 comes
 *      in with sufficient update flags.
 *
 *      This sets the following flags in *pfl:
 *
 *      --  DDFL_INVALIDATECOLUMNS: columns have to be
 *          repositioned, so details window needs a full
 *          repaint.
 *
 *      --  DDFL_WINDOWSIZECHANGED: cnr titles area changed,
 *          so details window might need adjustment.
 *
 *      --  DDFL_WORKAREACHANGED: cnr workarea changed, so
 *          scroll bars need adjustment.
 */

VOID cdtlRecalcDetails(PCNRDATA pData,
                       HPS hps,
                       PULONG pfl)      // in/out: DDFL_* flags
{
    LONG        xCurrent = 0;
    PLISTNODE   pColNode;
    ULONG       cColumn = 0;

    // compute total padding for one row:
    LONG        cxPaddingRow = pData->CnrInfo.cFields * 2 * COLUMN_PADDING_X;
    LONG        cyColTitlesContent = 0,
                cyColTitlesBox = 0;

    BOOL        fRefreshAll = (*pfl & (DDFL_INVALIDATECOLUMNS | DDFL_INVALIDATERECORDS));

    LONG        yNow = 0;

    // list of records to invalidate while we're running;
    // this only gets appended to if we're not invalidating
    // all records anyway
    LINKLIST    llInvalidateRecords;
    lstInit(&llInvalidateRecords, FALSE);       // receives RECORDLISTITEM's

    GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), &pData->fm);

    // outer loop: columns (go RIGHT)
    FOR_ALL_NODES(&pData->llColumns, pColNode)
    {
        PDETAILCOLUMN pColumn = (PDETAILCOLUMN)pColNode->pItemData;
        const FIELDINFO *pfi = pColumn->pfi;
        PLISTNODE   pRecNode;

        LONG        cxContent;

        if (fRefreshAll)
            pColumn->cxWidestRecord = 0;

        // skip invisible columns
        if (!(pfi->flData & CFA_INVISIBLE))
        {
            yNow = 0;

            // inner loop: records (go UP)
            // (we start with the LAST record so we can calculate
            // the y coordinates correctly)
            pRecNode = lstQueryLastNode(&pData->llRootRecords);
            while (pRecNode)
            {
                PRECORDLISTITEM prliThis = (PRECORDLISTITEM)pRecNode->pItemData;
                const   RECORDCORE *preccThis = prliThis->precc;
                PVOID   pColumnData = (PVOID)((PBYTE)preccThis + pfi->offStruct);

                if (!(prliThis->flRecordAttr & CRA_FILTERED))
                {
                    // skip filtered records
                    if (    (*pfl & DDFL_INVALIDATERECORDS)
                         || (prliThis->flInvalidate)
                       )
                    {
                        ULONG   cLines;
                        SIZEL   szlThis = {10, 10};
                        PCSZ    pcsz = NULL;
                        CHAR    szTemp[30];

                        if (!cColumn)
                        {
                            // we're in the leftmost column:
                            prliThis->szlContent.cx
                            = prliThis->szlContent.cy
                            = prliThis->szlBox.cx
                            = prliThis->szlBox.cy
                            = 0;
                        }

                        switch (pfi->flData & (   CFA_BITMAPORICON
                                                | CFA_DATE
                                                | CFA_STRING
                                                | CFA_TIME
                                                | CFA_ULONG))
                        {
                            case CFA_BITMAPORICON:
                                // @@todo
                            break;

                            case CFA_STRING:
                                pcsz = *(PSZ*)pColumnData;
                            break;

                            case CFA_DATE:
                                nlsDate(&pData->cs,
                                        szTemp,
                                        ((PCDATE)pColumnData)->year,
                                        ((PCDATE)pColumnData)->month,
                                        ((PCDATE)pColumnData)->day);
                                pcsz = szTemp;
                            break;

                            case CFA_TIME:
                                nlsTime(&pData->cs,
                                        szTemp,
                                        ((PCTIME)pColumnData)->hours,
                                        ((PCTIME)pColumnData)->minutes,
                                        ((PCTIME)pColumnData)->seconds);
                                pcsz = szTemp;
                            break;

                            case CFA_ULONG:
                                nlsThousandsULong(szTemp,
                                                  *((PULONG)pColumnData),
                                                  pData->cs.cs.cThousands);
                                pcsz = szTemp;
                            break;
                        }

                        if (pcsz)
                        {
                            gpihCalcTextExtent(hps,
                                               pcsz,
                                               &szlThis.cx,
                                               &cLines);
                            szlThis.cy = cLines * (pData->fm.lMaxBaselineExt + pData->fm.lExternalLeading);
                        }

                        // increment record's total width
                        prliThis->szlContent.cx += szlThis.cx;
                        prliThis->szlBox.cx +=   szlThis.cx
                                               + cxPaddingRow;       // computed at top

                        // record's content height = height of tallest column of that record
                        if (szlThis.cy > prliThis->szlContent.cy)
                            prliThis->szlContent.cy = szlThis.cy;

                        // remember max width of this record's column
                        // for the entire column
                        if (szlThis.cx > pColumn->cxWidestRecord)
                            pColumn->cxWidestRecord = szlThis.cx;

                        if (!pColNode->pNext)
                        {
                            // last column of outer loop:

                            // clear refresh flags, if any
                            prliThis->flInvalidate = 0;

                            // compute box cy from content
                            prliThis->szlBox.cy =   prliThis->szlContent.cy
                                                  + pData->CnrInfo.cyLineSpacing;

                            // store record's position in workarea coords
                            prliThis->ptl.x = 0;        // we're in Details view
                            prliThis->ptl.y = yNow;

                            if (!fRefreshAll)
                                // invalidate this record's rectangle
                                lstAppendItem(&llInvalidateRecords,
                                              prliThis);
                        }
                    }

                    // go down for next record
                    yNow += prliThis->szlBox.cy;

                } // if (!(prliThis->flRecordAttr & CRA_FILTERED))

                pRecNode = pRecNode->pPrevious;

            } // while (pRecNode)

            if (!(cxContent = pfi->cxWidth))
            {
                // this is an auto-size column:

                if (pData->CnrInfo.flWindowAttr & CA_DETAILSVIEWTITLES)
                {
                    if (*pfl & DDFL_INVALIDATECOLUMNS)
                    {
                        // compute space needed for title

                        pColumn->szlTitleData.cx
                        = pColumn->szlTitleData.cy
                        = 0;

                        if (pfi->flTitle & CFA_BITMAPORICON)
                            // @@todo
                            ;
                        else
                        {
                            ULONG cLines;
                            gpihCalcTextExtent(hps,
                                               (PCSZ)pfi->pTitleData,
                                               &pColumn->szlTitleData.cx,
                                               &cLines);
                            pColumn->szlTitleData.cy = cLines * (pData->fm.lMaxBaselineExt + pData->fm.lExternalLeading);
                        }
                    }

                    cxContent = max(pColumn->cxWidestRecord, pColumn->szlTitleData.cx);

                    if (pColumn->szlTitleData.cy > cyColTitlesContent)
                        cyColTitlesContent = pColumn->szlTitleData.cy;
                }
                else
                {
                    pColumn->szlTitleData.cx
                    = 0;
                }
            }

            // check if column needs invalidating
            if (    (fRefreshAll)
                 || (pColumn->xLeft != xCurrent)
                 || (pColumn->cxContent != cxContent)
               )
            {
                pColumn->xLeft = xCurrent;
                pColumn->cxContent = cxContent;

                if (!fRefreshAll)
                    InvalidateColumn(pData,
                                     pColumn);
            }

            // go one column to the right
            xCurrent += cxContent + 2 * COLUMN_PADDING_X;

            if (    (pfi->flData & CFA_SEPARATOR)
                 && (pColNode->pNext)
               )
                xCurrent += DEFAULT_BORDER_WIDTH;

            ++cColumn;
        }
    }

    // has workarea changed?
    if (    (pData->scrw.szlWorkarea.cx != xCurrent)
         || (pData->scrw.szlWorkarea.cy != yNow)
       )
    {
        pData->scrw.szlWorkarea.cx = xCurrent;
        pData->scrw.szlWorkarea.cy = yNow;
        *pfl |= DDFL_WORKAREACHANGED;
    }

    // has details title box changed?
    if (pData->CnrInfo.flWindowAttr & CA_DETAILSVIEWTITLES)
        cyColTitlesBox =   cyColTitlesContent
                         + 2 * COLUMN_PADDING_Y
                         + pData->CnrInfo.cyLineSpacing;

    if (    (cyColTitlesContent != pData->cyColTitlesContent)
         || (cyColTitlesBox != pData->cyColTitlesBox)
       )
    {
        pData->cyColTitlesContent = cyColTitlesContent;
        pData->cyColTitlesBox = cyColTitlesBox;

        *pfl |= DDFL_WINDOWSIZECHANGED | DDFL_WORKAREACHANGED;
    }

    // now invalidate records
    if (fRefreshAll)
        WinInvalidateRect(pData->dwdMain.hwnd, NULL, TRUE);
    else
    {
        PLISTNODE pNode = lstQueryFirstNode(&llInvalidateRecords);
        while (pNode)
        {
            ctnrRepaintRecord(pData, (PRECORDLISTITEM)pNode->pItemData);
            pNode = pNode->pNext;
        }
    }

    lstClear(&llInvalidateRecords);
}

/*
 *@@ DetailsCreate:
 *      implementation for WM_CREATE in fnwpCnrDetails.
 *
 *      We receive the CNRDATA as the create param.
 */

MRESULT DetailsCreate(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    HWND    hwndParent;

    if (    (!mp1)
         || (!mp2)
         || (!(hwndParent = (((PCREATESTRUCT)mp2)->hwndParent)))
       )
        return (MRESULT)TRUE;

    WinSetWindowPtr(hwnd, QWL_USER + 1, mp1);

    // initialize DEFWINDOWDATA
    ctnrInit(hwnd,
             mp2,
             winhQueryWindowStyle(hwndParent),      // main container
             &((PCNRDATA)mp1)->dwdContent);

    // the cnr appears to always grab the focus on creation
    WinSetFocus(HWND_DESKTOP, hwnd);    // @@todo how do we do this on creation only?

    return (MRESULT)FALSE;
}

/*
 *@@ DetailsPaint:
 *      implementation for WM_PAINT in fnwpCnrDetails.
 *
 */

VOID DetailsPaint(PCNRDATA pData)
{
    HPS         hps;
    RECTL       rclClip;

    if (hps = WinBeginPaint(pData->dwdContent.hwnd, NULLHANDLE, &rclClip))
    {
        PLISTNODE pColNode,
                  pRecNodeFirst2Paint = NULL;        // first one to paint

        LONG    yPadding = pData->CnrInfo.cyLineSpacing / 2;
        ULONG   cColumn = 0;
        BOOL    fFirstCol = TRUE;

        BOOL    fHasFocus;
        HWND    hwndFocus;

        if (    (hwndFocus = WinQueryFocus(HWND_DESKTOP))
             && (hwndFocus == pData->dwdContent.hwnd)
           )
            fHasFocus = TRUE;
        else
            fHasFocus = FALSE;

        gpihSwitchToRGB(hps);

#if 0
        {
            ULONG ulTimeNow;
            DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT,
                            &ulTimeNow,
                            sizeof(ulTimeNow));
            WinFillRect(hps,
                        &rclClip,
                        ulTimeNow & 0xFFFFFF);
        }
#else
        WinFillRect(hps,
                    &rclClip,
                    ctlQueryColor(&pData->dwdMain, CTLCOL_BGND));
#endif

        // skip columns that are not in paint rectangle
        pColNode = lstQueryFirstNode(&pData->llColumns);
        while (pColNode)
        {
            PDETAILCOLUMN pCol = (PDETAILCOLUMN)pColNode->pItemData;
            const FIELDINFO *pfi = pCol->pfi;
            if (    (!(pfi->flData & CFA_INVISIBLE))
                 && (pCol->xLeft + pCol->cxContent - pData->scrw.ptlScrollOfs.x >= rclClip.xLeft)
               )
                break;

            pColNode = pColNode->pNext;
        }

        // now loop for the remaining columns (go RIGHT)
        while (pColNode)
        {
            PDETAILCOLUMN pCol = (PDETAILCOLUMN)pColNode->pItemData;
            const FIELDINFO *pfi = pCol->pfi;
            PLISTNODE   pRecNode;

            if (!(pfi->flData & CFA_INVISIBLE))
            {
                RECTL rclRecc,
                      rcl;

                rcl.xLeft = pCol->xLeft + COLUMN_PADDING_X - pData->scrw.ptlScrollOfs.x;
                rcl.xRight = rcl.xLeft + pCol->cxContent;

                if (fFirstCol)
                {
                    // first time we get here: skip all records that
                    // are outside the paint rectangle

                    pRecNode = lstQueryFirstNode(&pData->llRootRecords);
                    while (pRecNode)
                    {
                        PRECORDLISTITEM prliThis = (PRECORDLISTITEM)pRecNode->pItemData;

                        if (!(prliThis->flRecordAttr & CRA_FILTERED))
                        {
                            ctnrGetRecordRect(pData, &rclRecc, prliThis);
                            if (rclRecc.yBottom <= rclClip.yTop)
                            {
                                pRecNodeFirst2Paint = pRecNode;

                                break;
                            }
                        }

                        pRecNode = pRecNode->pNext;
                    }
                }

                // now inner loop for the remaining records (go DOWN)
                pRecNode = pRecNodeFirst2Paint;
                while (pRecNode)
                {
                    PRECORDLISTITEM prliThis = (PRECORDLISTITEM)pRecNode->pItemData;

                    if (!(prliThis->flRecordAttr & CRA_FILTERED))
                    {
                        const   RECORDCORE *preccThis = prliThis->precc;
                        PVOID   pColumnData = (PVOID)((PBYTE)preccThis + pfi->offStruct);
                        CHAR    szTemp[100];
                        PCSZ    pcsz = NULL;
                        LONG    lcolBackground,
                                lcolForeground;

                        ctnrGetRecordRect(pData, &rclRecc, prliThis);

                        PMPF_RECT("rclRecc: ", &rclRecc);

                        if (prliThis->flRecordAttr & CRA_SELECTED)
                        {
                            lcolBackground = ctlQueryColor(&pData->dwdMain, CNRCOL_HILITEBGND);
                            lcolForeground = ctlQueryColor(&pData->dwdMain, CNRCOL_HILITEFGND);

                            if (fFirstCol)
                                WinFillRect(hps,
                                            &rclRecc,
                                            lcolBackground);
                        }
                        else
                        {
                            lcolBackground = ctlQueryColor(&pData->dwdMain, CTLCOL_BGND);
                            lcolForeground = ctlQueryColor(&pData->dwdMain, CTLCOL_FGND);
                        }

                        GpiSetColor(hps, lcolForeground);
                        GpiSetBackColor(hps, lcolBackground);

                        rcl.yTop = rclRecc.yTop - yPadding;
                        rcl.yBottom = rcl.yTop - prliThis->szlContent.cy;

                        if (    (fFirstCol)
                             && (prliThis->flRecordAttr & CRA_CURSORED)
                           )
                        {
                            RECTL rcl2;
                            rcl2.xLeft = rclRecc.xLeft + 1;
                            rcl2.xRight = rclRecc.xRight - 2;
                            rcl2.yBottom = rclRecc.yBottom + 1;
                            rcl2.yTop = rclRecc.yTop - 2;
                            GpiSetLineType(hps, LINETYPE_ALTERNATE);
                            gpihBox(hps,
                                    DRO_OUTLINE,
                                    &rcl2);
                            GpiSetLineType(hps, LINETYPE_DEFAULT);
                        }

                        switch (pfi->flData & (   CFA_BITMAPORICON
                                                | CFA_DATE
                                                | CFA_STRING
                                                | CFA_TIME
                                                | CFA_ULONG))
                        {
                            case CFA_BITMAPORICON:
                                // @@todo
                            break;

                            case CFA_STRING:
                                pcsz = *(PSZ*)pColumnData;
                            break;

                            case CFA_DATE:
                                nlsDate(&pData->cs,
                                        szTemp,
                                        ((PCDATE)pColumnData)->year,
                                        ((PCDATE)pColumnData)->month,
                                        ((PCDATE)pColumnData)->day);
                                pcsz = szTemp;
                            break;

                            case CFA_TIME:
                                nlsTime(&pData->cs,
                                        szTemp,
                                        ((PCTIME)pColumnData)->hours,
                                        ((PCTIME)pColumnData)->minutes,
                                        ((PCTIME)pColumnData)->seconds);
                                pcsz = szTemp;
                            break;

                            case CFA_ULONG:
                                nlsThousandsULong(szTemp,
                                                  *((PULONG)pColumnData),
                                                  pData->cs.cs.cThousands);
                                pcsz = szTemp;
                            break;
                        }

                        if (pcsz)
                            ctnrDrawString(hps,
                                          pcsz,
                                          &rcl,
                                          pfi->flData,
                                          &pData->fm);

                        // if we're outside the paint rect now,
                        // we can quit
                        // _Pmpf(("rcl.yBottom: %d, rclClip.yBottom: %d",
                        //         rcl.yBottom,
                        //         rclClip.yBottom));
                        if (rclRecc.yBottom <= rclClip.yBottom)
                            break;

                    } // if (!(prliThis->flRecordAttr & CRA_FILTERED))

                    pRecNode = pRecNode->pNext;

                } // while (pRecNode)

                // paint vertical separators after this column?
                if (pfi->flData & CFA_SEPARATOR)
                {
                    POINTL  ptl;
                    GpiSetColor(hps, ctlQueryColor(&pData->dwdMain, CNRCOL_BORDER));
                    ptl.x = rcl.xRight + COLUMN_PADDING_X;
                    ptl.y = pData->dwdContent.szlWin.cy;
                    GpiMove(hps,
                            &ptl);
                    ptl.y = 0;
                    GpiLine(hps,
                            &ptl);
                }

                fFirstCol = FALSE;

                ++cColumn;

                // we're done if this column is outside the
                // paint rectangle
                if (    (!(pCol->pfi->flData & CFA_INVISIBLE))
                     && (pCol->xLeft + pCol->cxContent - pData->scrw.ptlScrollOfs.x >= rclClip.xRight)
                   )
                    break; // while (pColNode)

            } // if (!(pfi->flData & CFA_INVISIBLE))

            pColNode = pColNode->pNext;

        } // while (pColNode)

        WinEndPaint(hps);
    }
}

/*
 *@@ FindRecordFromMouseY:
 *
 */

PRECORDLISTITEM FindRecordFromMouseY(PCNRDATA pData,
                                     SHORT y)
{
    // convert y clickpos from window to workspace coordinates
    LONG    deltaWorkspace =   (   pData->scrw.szlWorkarea.cy
                                 - pData->dwdContent.szlWin.cy)
                             - pData->scrw.ptlScrollOfs.y;
    LONG    yWorkspace =  (LONG)y + deltaWorkspace;

    PLISTNODE pRecNode = lstQueryFirstNode(&pData->llRootRecords);
    while (pRecNode)
    {
        PRECORDLISTITEM prliThis = (PRECORDLISTITEM)pRecNode->pItemData;

        if (!(prliThis->flRecordAttr & CRA_FILTERED))
        {
            if (prliThis->ptl.y < yWorkspace)
                return prliThis;
        }

        pRecNode = pRecNode->pNext;
    }

    return NULL;
}

/*
 *@@ DeselectExcept:
 *
 */

VOID DeselectExcept(PCNRDATA pData,
                    PRECORDLISTITEM prliMouse)
{
    // deselect all selected
    PLISTNODE pRecNode;
    pRecNode = lstQueryFirstNode(&pData->llRootRecords);
    while (pRecNode)
    {
        PRECORDLISTITEM prliThis = (PRECORDLISTITEM)pRecNode->pItemData;

        if (    (prliThis != prliMouse)
             && (prliThis->flRecordAttr & CRA_SELECTED)
           )
        {
            ctnrChangeEmphasis(pData,
                               prliThis,
                               FALSE,
                               CRA_SELECTED);
        }

        pRecNode = pRecNode->pNext;
    }
}

/*
 *@@ DetailsSingleSelect:
 *      implementation for WM_SINGLESELECT in fnwpCnrDetails.
 */

MRESULT DetailsSingleSelect(PCNRDATA pData,
                            SHORT y)
{
    if (pData)
    {
        BOOL    fCtrl = WinGetKeyState(HWND_DESKTOP, VK_CTRL) & 0x8000;
        ULONG   ulSel = ctnrQuerySelMode(pData);

        // find record under mouse
        PRECORDLISTITEM prliMouse = FindRecordFromMouseY(pData, y);

        if (    (ulSel == CCS_SINGLESEL)
             || (!fCtrl)
           )
        {
            DeselectExcept(pData, prliMouse);
        }

        if (prliMouse)      // can be null with click on whitespace
        {
            // when ctrl is pressed, toggle the state of
            // the record under the mouse; otherwise select it
            ctnrChangeEmphasis(pData,
                               prliMouse,
                               (!fCtrl) || (!(prliMouse->flRecordAttr & CRA_SELECTED)),
                               CRA_SELECTED | CRA_CURSORED);
        }

        return (MRESULT)TRUE;
    }

    return (MRESULT)FALSE;
}

/*
 *@@ DetailsBeginSelect:
 *      implementation for WM_BEGINSELECT in fnwpCnrDetails.
 *
 */

MRESULT DetailsBeginSelect(PCNRDATA pData,
                           SHORT y)
{
    if (pData)
    {
        BOOL    fCtrl = WinGetKeyState(HWND_DESKTOP, VK_CTRL) & 0x8000;
        ULONG   ulSel = ctnrQuerySelMode(pData);
        PRECORDLISTITEM prliMouse;

        // find record under mouse
        prliMouse
        = pData->prliSwipingFirst
        = FindRecordFromMouseY(pData, y);

        if (    (ulSel == CCS_SINGLESEL)
             || (!fCtrl)
           )
        {
            DeselectExcept(pData, prliMouse );
        }

        if (prliMouse )
        {
            // when ctrl is pressed, toggle the state of
            // the record under the mouse; otherwise select it;
            // perform this operation on all the records that
            // user swipes over (seems to be how pm cnr does it)
            pData->fSwipeTurnOn = (!fCtrl) || (!(prliMouse->flRecordAttr & CRA_SELECTED));

            ctnrChangeEmphasis(pData,
                               prliMouse ,
                               pData->fSwipeTurnOn,
                               CRA_SELECTED | CRA_CURSORED);
            WinSetCapture(HWND_DESKTOP, pData->dwdContent.hwnd);
        }

        return (MRESULT)TRUE;
    }

    return (MRESULT)FALSE;
}

/*
 *@@ DetailsMouseMove:
 *      implementation for WM_MOUSEMOVE in fnwpCnrDetails.
 */

MRESULT DetailsMouseMove(PCNRDATA pData,
                         MPARAM mp1,
                         MPARAM mp2)
{
    if (pData)
    {
        if (pData->prliSwipingFirst)
        {
            // we're swiping:
            PRECORDLISTITEM prliThis;
            if (prliThis = FindRecordFromMouseY(pData,
                                                SHORT2FROMMP(mp1)))
                ctnrChangeEmphasis(pData,
                                   prliThis,
                                   pData->fSwipeTurnOn,
                                   CRA_SELECTED | CRA_CURSORED);
        }

        return ctlDefWindowProc(&pData->dwdContent, WM_MOUSEMOVE, mp1, mp2);
    }

    return (MRESULT)FALSE;
}

/*
 *@@ DetailsEndSelect:
 *      implementation for WM_ENDSELECT in fnwpCnrDetails.
 */

MRESULT DetailsEndSelect(PCNRDATA pData)
{
    if (pData)
    {
        if (pData->prliSwipingFirst)
        {
            WinSetCapture(HWND_DESKTOP, NULLHANDLE);
            pData->prliSwipingFirst = NULL;
        }

        return (MRESULT)TRUE;
    }

    return (MRESULT)FALSE;
}

/*
 *@@ DetailsOpen:
 *      implementation for WM_OPEN in fnwpCnrDetails.
 */

MRESULT DetailsOpen(PCNRDATA pData)
{
    if (pData)
    {
        ctnrRecordEnter(pData,
                        pData->prliCursored,
                        FALSE);     // mouse, not keyboard

        return (MRESULT)TRUE;
    }

    return (MRESULT)FALSE;
}

/*
 *@@ ScrollToRecord:
 *      scrolls the content area so that the given record becomes
 *      visible.
 *
 +      If fMakeTop == TRUE, we scroll so that the top of the
 *      record's box is scrolled to the top of the window.
 *      Otherwise we scroll so that the bottom of the record's
 *      box is at the bottom of the rectangle.
 */

VOID ScrollToRecord(PCNRDATA pData,
                    PRECORDLISTITEM prliThis,
                    BOOL fMakeTop,
                    BOOL fForce)        // in: if TRUE, we scroll even if the record is already visible
{
    RECTL   rcl;
    LONG    lNewY = -100;

    ctnrGetRecordRect(pData, &rcl, prliThis);
    if (fMakeTop)
    {
        if (    (rcl.yTop > pData->dwdContent.szlWin.cy)
             || (fForce)
           )
        {
/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿   Ä¿
³                             ³    ³ pData->scrw.ptlScrollOfs.y
+-----------------------------+   ÄÙ
+-----------------------------+
º                             º
º                             º
ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼
³                             ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
            lNewY =   pData->scrw.szlWorkarea.cy
                    - (prliThis->ptl.y + prliThis->szlBox.cy);
        }
    }
    else if (    (rcl.yBottom < 0)
              || (fForce)
            )
    {
/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿   Ä¿
³                             ³    ³ pData->scrw.ptlScrollOfs.y
ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»    ³
º                             º    ³
+-----------------------------+    ³
+-----------------------------+   ÄÙ
³                             ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
        lNewY =   pData->scrw.szlWorkarea.cy
                - pData->dwdContent.szlWin.cy
                - prliThis->ptl.y;
    }

    if (lNewY != -100)
    {
        POINTL  ptlScroll;

        if (lNewY < 0)
            lNewY = 0;
        else if (lNewY >= pData->scrw.szlWorkarea.cy)
            lNewY = pData->scrw.szlWorkarea.cy - 1;

        ptlScroll.x = 0;
        ptlScroll.y = lNewY - pData->scrw.ptlScrollOfs.y;

        winhScrollWindow(pData->dwdContent.hwnd,
                         NULL,
                         &ptlScroll);

        pData->scrw.ptlScrollOfs.y = lNewY;

        WinPostMsg(pData->dwdMain.hwnd,
                   WM_SEM2,
                   (MPARAM)DDFL_WORKAREACHANGED,
                   0);
    }
}

/*
 *@@ DetailsChar:
 *      implementation for WM_CHAR in fnwpCnrDetails.
 */

MRESULT DetailsChar(PCNRDATA pData,
                    MPARAM mp1,
                    MPARAM mp2)
{
    if (pData)
    {
        USHORT      usFlags = SHORT1FROMMP(mp1);
        USHORT      usch    = SHORT1FROMMP(mp2);
        USHORT      usvk    = SHORT2FROMMP(mp2);

        PRECORDLISTITEM prliThis;
        PLISTNODE   pNode;
        PRECORDLISTITEM prliScrollTo = NULL;
        BOOL        fMakeTop = FALSE,
                    fForce = FALSE;

        if (usFlags & KC_VIRTUALKEY)
        {
            switch (usvk)
            {
                case VK_ENTER:
                case VK_NEWLINE:
                    if (!(usFlags & KC_KEYUP))
                        if (pData->prliCursored)
                            ctnrRecordEnter(pData,
                                            pData->prliCursored,
                                            TRUE);      // keyboard

                    return (MRESULT)TRUE;

                case VK_SPACE:
                    if (!(usFlags & KC_KEYUP))
                    {
                        if (    (CCS_SINGLESEL != ctnrQuerySelMode(pData))
                             && (prliThis = pData->prliCursored)
                           )
                        {
                            if (prliThis->flRecordAttr & CRA_SELECTED)
                                DeselectExcept(pData, NULL);
                            else
                                ctnrChangeEmphasis(pData,
                                                   prliThis,
                                                   TRUE,
                                                   CRA_SELECTED);
                        }
                    }

                    return (MRESULT)TRUE;

                case VK_DOWN:
                case VK_UP:
                    if (!(usFlags & KC_KEYUP))
                    {
                        if (    (prliThis = pData->prliCursored)
                             && (pNode = ctnrFindListNodeForRecc(pData, prliThis->precc))
                           )
                        {
                            while (TRUE)
                            {
                                if (usvk == VK_UP)
                                {
                                    if (!(pNode = pNode->pPrevious))
                                        break;
                                }
                                else
                                    if (!(pNode = pNode->pNext))
                                        break;

                                prliThis = (PRECORDLISTITEM)pNode->pItemData;
                                if (!(prliThis->flRecordAttr & CRA_FILTERED))
                                {
                                    prliScrollTo = prliThis;
                                    fMakeTop = (usvk == VK_UP);
                                    break;
                                }
                            }
                        }
                    }
                break;

                case VK_HOME:
                case VK_END:
                    if (!(usFlags & KC_KEYUP))
                    {
                        if (usvk == VK_HOME)
                            pNode = lstQueryFirstNode(&pData->llRootRecords);
                        else
                            pNode = lstQueryLastNode(&pData->llRootRecords);

                        while (pNode)
                        {
                            prliThis = (PRECORDLISTITEM)pNode->pItemData;
                            if (!(prliThis->flRecordAttr & CRA_FILTERED))
                            {
                                prliScrollTo = prliThis;
                                if (usvk == VK_HOME)
                                    fMakeTop = TRUE;
                                fForce = TRUE;
                                break;
                            }

                            if (usvk == VK_HOME)
                                pNode = pNode->pNext;
                            else
                                pNode = pNode->pPrevious;
                        }
                    }
                break;

                case VK_PAGEDOWN:
                    if (!(usFlags & KC_KEYUP))
                    {
                        // find the bottommost record currently visible in the
                        // viewport and make that one the topmost
                        prliScrollTo = FindRecordFromMouseY(pData,
                                                            0);     // window y
                        fMakeTop = TRUE;
                        fForce = TRUE;
                    }
                break;

                case VK_PAGEUP:
                    if (!(usFlags & KC_KEYUP))
                    {
                        // find the topmost record currently visible in the
                        // viewport
                        prliScrollTo = FindRecordFromMouseY(pData,
                                                            pData->dwdContent.szlWin.cy);     // window y
                            // @@todo this is slightly different from the pm cnr behavior
                        fForce = TRUE;
                    }
                break;
            }

            if (prliScrollTo)
            {
                DeselectExcept(pData, prliScrollTo);

                ctnrChangeEmphasis(pData,
                                   prliScrollTo,
                                   TRUE,
                                   CRA_SELECTED | CRA_CURSORED);

                // now make sure the new record is visible
                // in the viewport
                ScrollToRecord(pData,
                               prliScrollTo,
                               fMakeTop,
                               fForce);

                return (MRESULT)TRUE;
            }
        }
    }

    return (MRESULT)FALSE;
}

/* ******************************************************************
 *
 *   Container details window proc
 *
 ********************************************************************/

/*
 *@@ fnwpCnrDetails:
 *
 */

MRESULT EXPENTRY fnwpCnrDetails(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
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
            mrc = DetailsCreate(hwnd, mp1, mp2);
        break;

        case WM_PAINT:
            DetailsPaint(pData);
        break;

        case WM_SYSCOLORCHANGE:
            ctnrPresParamChanged(hwnd, 0);
        break;

        case WM_PRESPARAMCHANGED:
            ctnrPresParamChanged(hwnd, (ULONG)mp1);
        break;

        /* ******************************************************************
         *
         *   Mouse and keyboard input
         *
         ********************************************************************/

        case WM_MOUSEMOVE:
            mrc = DetailsMouseMove(pData,
                                   mp1,
                                   mp2);
        break;

        case WM_BUTTON1DOWN:
        case WM_BUTTON2DOWN:
        case WM_BUTTON3DOWN:
            WinSetFocus(HWND_DESKTOP, pData->dwdContent.hwnd);
            mrc = (MPARAM)TRUE;
        break;

        case WM_SINGLESELECT:
            mrc = DetailsSingleSelect(pData,
                                      SHORT2FROMMP(mp1));       // we only need y in details view
        break;

        case WM_BEGINSELECT:
            mrc = DetailsBeginSelect(pData,
                                     SHORT2FROMMP(mp1));
        break;

        case WM_ENDSELECT:
            mrc = DetailsEndSelect(pData);
        break;

        case WM_OPEN:
            mrc = DetailsOpen(pData);
        break;

        case WM_CHAR:
            mrc = DetailsChar(pData, mp1, mp2);
        break;

        /* ******************************************************************
         *
         *   Direct manipulation
         *
         ********************************************************************/

        case DM_DRAGOVER:
        case DM_DRAGLEAVE:
        case DM_DROPNOTIFY:
        case DM_DROP:
        case DM_DROPHELP:

        default:
            if (pData)
                mrc = ctlDefWindowProc(&pData->dwdContent, msg, mp1, mp2);
        break;
    }

    return mrc;
}

