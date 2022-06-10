
/*
 *@@sourcefile cctl_checkcnr.c:
 *      implementation for the checkbox container common control.
 *      See comctl.c for an overview.
 *
 *      This has been extracted from comctl.c with V0.9.3 (2000-05-21) [umoeller].
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\comctl.h"
 *@@added V0.9.3 (2000-05-21) [umoeller].
 */

/*
 *      Copyright (C) 1997-2002 Ulrich M”ller.
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

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WININPUT
#define INCL_WINPOINTERS
#define INCL_WINTRACKRECT
#define INCL_WINTIMER
#define INCL_WINSYS

#define INCL_WINRECTANGLES      /// xxx temporary

#define INCL_WINMENUS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINSTDCNR

#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#define INCL_GPILCIDS
#define INCL_GPIPATHS
#define INCL_GPIREGIONS
#define INCL_GPIBITMAPS             // added V0.9.1 (2000-01-04) [umoeller]: needed for EMX headers
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

#include "setup.h"                      // code generation and debugging options

#include "helpers/cnrh.h"
#include "helpers/except.h"             // exception handling
#include "helpers/gpih.h"
#include "helpers/linklist.h"
#include "helpers/winh.h"

#include "helpers/comctl.h"

#pragma hdrstop

/*
 *@@category: Helpers\PM helpers\Window classes\Checkbox containers
 *      See cctl_checkcnr.c.
 */

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// linked list of CHEXKBOXCNROWNER struct pointers
HMTX        G_hmtxCnrOwnersList = 0;
PLINKLIST   G_pllCnrOwners = 0;

/* ******************************************************************
 *
 *   Checkbox container record cores
 *
 ********************************************************************/

HBITMAP  G_hbmCheckboxes = NULLHANDLE;
ULONG    G_cxCheckbox = 0;

/*
 *@@ ctlDrawCheckbox:
 *
 *      The system checkbox bitmap (G_hbmCheckboxes)
 *      is organized into 4 columns and three rows as
 *      follows:
 *
 +                         not depressed     depressed
 +                       ÚÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄ¿
 +         checkbox      ³       ³  chk   ³       ³  chk  ³
 +                       ÃÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄ´
 +         radio button  ³       ³  chk   ³       ³  chk  ³
 +                       ÃÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄ´
 +         chbx indeterm.³       ³  ind   ³       ³  ind  ³
 +                       ÀÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÙ
 *
 *@@added V0.9.0 (99-11-28) [umoeller]
 */

BOOL ctlDrawCheckbox(HPS hps,               // in: paint PS
                     LONG x,                // in: target lower left corner x
                     LONG y,                // in: target lower left corner y
                     USHORT usRow,          // in: 0 - 2
                     USHORT usColumn,       // in: 0 - 3
                     BOOL fHalftoned)       // in: if TRUE, the checkbox will be halftoned (grayed)
{
    POINTL  aptl[4];
    BOOL    brc = FALSE;

    if (G_hbmCheckboxes)
    {
        // paint checkbox;
        LONG lHits;

        memset(aptl, 0, sizeof(POINTL) * 4);

        // aptl[0]: target bottom-left
        aptl[0].x = x;
        aptl[0].y = y;

        // aptl[1]: target top-right (inclusive!)
        aptl[1].x = aptl[0].x + G_cxCheckbox - 1;
        aptl[1].y = aptl[0].y + G_cxCheckbox - 1;

        // aptl[2]: source bottom-left:
        // depends on record selection
        aptl[2].x = usColumn * G_cxCheckbox;
        aptl[2].y = usRow * G_cxCheckbox;       // top checkbox row

        // aptl[3]: source top-right (non-inclusive!)
        aptl[3].x = aptl[2].x + G_cxCheckbox;
        aptl[3].y = aptl[2].y + G_cxCheckbox;

        if (fHalftoned)
        {
            HDC     hdc = GpiQueryDevice(hps);
            HWND    hwnd = WinWindowFromDC(hdc);
            HAB     hab = WinQueryAnchorBlock(hwnd);
            HBITMAP hbmHalftoned = gpihCreateHalftonedBitmap(hab,
                                                             G_hbmCheckboxes,
                                                             CLR_WHITE);
            lHits = GpiWCBitBlt(hps,           // hpsTarget
                                hbmHalftoned,
                                4,
                                &aptl[0],
                                ROP_SRCCOPY,        // mix
                                0);                 // options
            GpiDeleteBitmap(hbmHalftoned);
        }
        else
            lHits = GpiWCBitBlt(hps,           // hpsTarget
                                G_hbmCheckboxes,
                                4,
                                &aptl[0],
                                ROP_SRCCOPY,        // mix
                                0);                 // options
        if (lHits == GPI_OK)
            brc = TRUE;
    }

    return brc;
}

/*
 *@@ CnrCheckboxClicked:
 *
 *
 *@@added V0.9.0 (99-11-28) [umoeller]
 */

STATIC VOID CnrCheckboxClicked(PCHECKBOXCNROWNER pcbco,
                               PCHECKBOXRECORDCORE precc,
                               BOOL fToggleAndNotify) // if TRUE, toggle state and notify owner (CN_RECORDCHECKED)
{
    if (precc->ulStyle & WS_VISIBLE)
    {
        if (precc->recc.flRecordAttr & CRA_DISABLED)
            // disabled:
            WinAlarm(HWND_DESKTOP, WA_WARNING);
        else
        {
            if (precc->ulStyle & BS_AUTOCHECKBOX)
            {
                if (fToggleAndNotify)
                    precc->usCheckState = 1 - precc->usCheckState;

                WinSendMsg(pcbco->hwndCnr,
                           CM_INVALIDATERECORD,
                           (MPARAM)&precc,
                           MPFROM2SHORT(1,
                                        CMA_NOREPOSITION));
            }

            if (fToggleAndNotify)
                WinPostMsg(pcbco->hwndOwner,        // was: WinSendMsg
                           WM_CONTROL,
                           MPFROM2SHORT(pcbco->usCnrID,
                                        CN_RECORDCHECKED),
                           (MPARAM)precc);
        }
    }
}

/*
 *@@ ctlDrawCheckBoxRecord:
 *
 *@@added V0.9.18 (2002-03-03) [umoeller]
 */

MRESULT ctlDrawCheckBoxRecord(MPARAM mp2)
{
    MRESULT mrc = 0;

    // get generic DRAWITEM structure
    POWNERITEM poi = (POWNERITEM)mp2;

    // _Pmpf(("WM_DRAWITEM poi->idItem %d", poi->idItem));

    // check if we're to draw the icon
    // (and not the text)
    if (poi->idItem == CMA_ICON)
    {
        PCNRDRAWITEMINFO pcdi = (PCNRDRAWITEMINFO)poi->hItem;
        PCHECKBOXRECORDCORE precc = (PCHECKBOXRECORDCORE)pcdi->pRecord;

        if (precc->ulStyle & WS_VISIBLE)
        {
            USHORT usRow,
                   usColumn;

            switch (precc->usCheckState)
            {
                case 0: // unchecked
                    usRow = 2;
                    usColumn = 0;
                break;

                case 1: // checked
                    usRow = 2;
                    usColumn = 1;
                break;

                case 2: // indeterminate
                    usRow = 0;
                    usColumn = 1;
                break;
            }

            if (precc->ulStyle & BS_BITMAP)
                // button currently depressed:
                // add two to column
                usColumn += 2;

            ctlDrawCheckbox(poi->hps,
                            poi->rclItem.xLeft,
                            poi->rclItem.yBottom,
                            usRow,
                            usColumn,
                            // halftoned?
                            ((precc->recc.flRecordAttr & CRA_DISABLED) != 0));
            mrc = (MPARAM)FALSE;
                        // we still need the cnr to draw the
                        // emphasis
        }
        else
            mrc = (MPARAM)TRUE; // tell cnr that we've drawn the item;
                    // don't even draw emphasis
    }
    else if (poi->idItem == CMA_TEXT)
    {
        // for text, buttons etc.:
        PCNRDRAWITEMINFO pcdi = (PCNRDRAWITEMINFO)poi->hItem;
        PCHECKBOXRECORDCORE precc = (PCHECKBOXRECORDCORE)pcdi->pRecord;
        if (precc->recc.flRecordAttr & CRA_DISABLED)
        {
            RECTL rcl2;
            LONG lBackground, lForeground;
            ULONG flCmd = DT_LEFT | DT_TOP | DT_ERASERECT;
            if ((pcdi->pRecord->flRecordAttr) & CRA_SELECTED)
            {
                // disabled and selected:
                lBackground = WinQuerySysColor(HWND_DESKTOP,
                                               SYSCLR_SHADOWTEXT, 0);
                lForeground = winhQueryPresColor(poi->hwnd,
                                                 PP_BACKGROUNDCOLOR,
                                                 FALSE, // no inherit
                                                 SYSCLR_WINDOW);
            }
            else
            {
                // disabled and not selected:
                lBackground = winhQueryPresColor(poi->hwnd,
                                                 PP_BACKGROUNDCOLOR,
                                                 FALSE,
                                                 SYSCLR_WINDOW);
                lForeground = winhQueryPresColor(poi->hwnd,
                                                 PP_FOREGROUNDCOLOR,
                                                 FALSE, // no inherit
                                                 SYSCLR_WINDOWTEXT);
                flCmd |= DT_HALFTONE;
            }

            // _Pmpf(("back: 0x%lX, fore: 0x%lX", lBackground, lForeground));

            GpiCreateLogColorTable(poi->hps, 0, LCOLF_RGB, 0, 0, NULL);
            GpiSetBackColor(poi->hps, lBackground);
            GpiSetColor(poi->hps, lForeground);

            memcpy(&rcl2, &(poi->rclItem), sizeof(rcl2));

            winhDrawFormattedText(poi->hps,
                                  &rcl2,
                                  precc->recc.pszTree,
                                  flCmd);
            mrc = (MPARAM)TRUE;
        }
        else
            // tell cnr to draw the item
            mrc = (MPARAM)FALSE;
    }
    else
        // tell cnr to draw the item
        mrc = (MPARAM)FALSE;

    return mrc;
}

/*
 *@@ fnwpSubclCheckboxCnr:
 *      window proc for subclassed containers.
 *      See ctlMakeCheckboxContainer for details.
 *
 *@@added V0.9.0 (99-11-29) [umoeller]
 *@@changed V0.9.18 (2002-03-03) [umoeller]: fixed bad orig win msg, other optimizations
 *@@changed V1.0.1 (2003-01-22) [umoeller]: fixed wrong mouse pointers on WM_MOUSEMOVE
 */

STATIC MRESULT EXPENTRY fnwpSubclCheckboxCnr(HWND hwndCnr, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT             mrc = 0;
    PCHECKBOXCNROWNER   pcbco = 0;
    PFNWP               pfnwpOrig = 0;

    if (!DosRequestMutexSem(G_hmtxCnrOwnersList, 5000))
    {
        PLISTNODE   pNode = lstQueryFirstNode(G_pllCnrOwners);
        while (pNode)
        {
            pcbco = (PCHECKBOXCNROWNER)pNode->pItemData;
            if (pcbco->hwndCnr == hwndCnr)
            {
                pfnwpOrig = pcbco->pfnwpCnrOrig; // fixed V0.9.18 (2002-03-03) [umoeller]
                break;
            }
            pNode = pNode->pNext;
        }
        DosReleaseMutexSem(G_hmtxCnrOwnersList);
    }

    if (pfnwpOrig)
    {
        switch (msg)
        {
            case WM_BUTTON1DOWN:
            {
                POINTL ptlClick;
                ptlClick.x = SHORT1FROMMP(mp1);
                ptlClick.y = SHORT2FROMMP(mp1);

                // find record whose icon has this point
                pcbco->preccClicked
                    = (PCHECKBOXRECORDCORE)cnrhFindRecordFromPoint(hwndCnr,
                                                                   &ptlClick,
                                                                   &pcbco->rclReccClicked,
                                                                   CMA_ICON,
                                                                   0);

                if (pcbco->preccClicked)
                {
                    if (pcbco->preccClicked->ulStyle & WS_VISIBLE)
                    {
                        // add "depressed" style
                        pcbco->preccClicked->ulStyle |= BS_BITMAP;
                        // mouse was clicked on icon (checkbox):
                        CnrCheckboxClicked(pcbco,
                                           pcbco->preccClicked,
                                           FALSE);
                    }
                    else
                        // not visible:
                        pcbco->preccClicked = NULL;
                }

                mrc = pfnwpOrig(hwndCnr, msg, mp1, mp2);
                        // apperently, the cnr captures the mouse
            }
            break;

            case WM_MOUSEMOVE:
                if (pcbco->preccClicked)
                {
                    // mouse moved while checkbox is depressed:
                    POINTL ptlMove;
                    ptlMove.x = SHORT1FROMMP(mp1);
                    ptlMove.y = SHORT2FROMMP(mp1);

                    if (WinPtInRect(pcbco->habCnr,
                                    &pcbco->rclReccClicked,
                                    &ptlMove))
                    {
                        // mouse is in checkbox:
                        if ((pcbco->preccClicked->ulStyle & BS_BITMAP) == 0)
                        {
                            // checkbox is not drawn depressed:
                            // add "depressed" style
                            pcbco->preccClicked->ulStyle |= BS_BITMAP;
                            CnrCheckboxClicked(pcbco,
                                               pcbco->preccClicked,
                                               FALSE);
                        }
                    }
                    else
                    {
                        // mouse is outside of checkbox:
                        if (pcbco->preccClicked->ulStyle & BS_BITMAP)
                        {
                            // checkbox is drawn depressed:
                            // remove "depressed" style
                            pcbco->preccClicked->ulStyle &= ~BS_BITMAP;
                            CnrCheckboxClicked(pcbco,
                                               pcbco->preccClicked,
                                               FALSE);
                        }
                    }
                }

                // always call parent cos container changes mouse pointers
                // V1.0.1 (2003-01-22) [umoeller]
                mrc = pfnwpOrig(hwndCnr, msg, mp1, mp2);
            break;

            case WM_BUTTON1UP:
                if (pcbco->preccClicked)
                {
                    if (pcbco->preccClicked->ulStyle & BS_BITMAP)
                    {
                        pcbco->preccClicked->ulStyle &= ~BS_BITMAP;
                        CnrCheckboxClicked(pcbco,
                                           pcbco->preccClicked,
                                           TRUE);
                    }

                    pcbco->preccClicked = NULL;
                }
                mrc = pfnwpOrig(hwndCnr, msg, mp1, mp2);
            break;

            /*
             * WM_CHAR:
             *
             */

            case WM_CHAR:
            {
                USHORT fsFlags = SHORT1FROMMP(mp1);
                CHAR   ch = CHAR1FROMMP(mp2);
                            // never use the USHORT, because for
                            // the "key-up" message, the hi-char
                            // is different (duh...)

                // _Pmpf(("WM_CHAR fsFlags %lX, usch %d", fsFlags, ch));

                if (ch == ' ')
                {
                    // space: toggle checkbox
                    if ((fsFlags & KC_KEYUP) == 0)
                    {
                        // space down:
                        // filter up repetitive key msgs
                        if (pcbco->preccSpace == NULL)
                        {
                            PCHECKBOXRECORDCORE precc = (PCHECKBOXRECORDCORE)WinSendMsg(
                                                                   hwndCnr,
                                                                   CM_QUERYRECORDEMPHASIS,
                                                                   (MPARAM)CMA_FIRST,
                                                                   (MPARAM)CRA_CURSORED);
                            if ((precc) && (precc != (PCHECKBOXRECORDCORE)-1))
                            {
                                if (precc->ulStyle & WS_VISIBLE)
                                {
                                    pcbco->preccSpace = precc;

                                    // add "depressed" style
                                    pcbco->preccSpace->ulStyle |= BS_BITMAP;
                                    CnrCheckboxClicked(pcbco,
                                                       pcbco->preccSpace,
                                                       FALSE);
                                }
                            }
                        }
                    }
                    else
                    {
                        // space up:
                        if (pcbco->preccSpace)
                        {
                            if (pcbco->preccSpace->ulStyle & BS_BITMAP)
                            {
                                pcbco->preccSpace->ulStyle &= ~BS_BITMAP;
                                CnrCheckboxClicked(pcbco,
                                                   pcbco->preccSpace,
                                                   TRUE);
                            }

                            pcbco->preccSpace = NULL;
                        }
                    }

                    // do not pass spaces on
                    mrc = (MPARAM)TRUE;     // processed
                    break;
                }

                mrc = pfnwpOrig(hwndCnr, msg, mp1, mp2);
            }
            break;

            /*
             * WM_DESTROY:
             *      remove list item; this gets sent
             *      to the parent first, then the children,
             *      so we remove this in the container
             *      (this way the list item is still valid
             *      in ctl_fnwpSubclCheckboxCnrOwner)
             */

            case WM_DESTROY:
                if (!DosRequestMutexSem(G_hmtxCnrOwnersList, 5000))
                {
                    if (WinIsWindow(pcbco->habCnr,
                                    pcbco->hwndOwner))
                        // un-subclass the owner
                        WinSubclassWindow(pcbco->hwndOwner,
                                          pcbco->pfnwpOwnerOrig);

                    lstRemoveItem(G_pllCnrOwners, pcbco);

                    if (lstCountItems(G_pllCnrOwners) == 0)
                        lstFree(&G_pllCnrOwners);

                    DosReleaseMutexSem(G_hmtxCnrOwnersList);
                }

                mrc = pfnwpOrig(hwndCnr, msg, mp1, mp2);
            break;

            default:
                mrc = pfnwpOrig(hwndCnr, msg, mp1, mp2);
        }
    }
    else
        mrc = WinDefWindowProc(hwndCnr, msg, mp1, mp2);

    return mrc;
}


/*
 *@@ fnwpSubclCheckboxCnrOwner:
 *      window proc for subclassed container owners.
 *      See ctlMakeCheckboxContainer for details.
 *
 *@@added V0.9.0 (99-11-28) [umoeller]
 */

STATIC MRESULT EXPENTRY fnwpSubclCheckboxCnrOwner(HWND hwndOwner, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT             mrc = 0;
    PCHECKBOXCNROWNER   pcbco = 0;
    PFNWP               pfnwpOrig = 0;

    if (!DosRequestMutexSem(G_hmtxCnrOwnersList, 5000))
    {
        PLISTNODE   pNode = lstQueryFirstNode(G_pllCnrOwners);
        while (pNode)
        {
            pcbco = (PCHECKBOXCNROWNER)pNode->pItemData;
            if (pcbco->hwndOwner == hwndOwner)
            {
                pfnwpOrig = pcbco->pfnwpOwnerOrig;
                break;
            }
            pNode = pNode->pNext;
        }
        DosReleaseMutexSem(G_hmtxCnrOwnersList);
    }

    if (pfnwpOrig)
    {
        switch (msg)
        {
            /*
             * WM_CONTROL:
             *
             */

            case WM_CONTROL:
            {
                if (SHORT1FROMMP(mp1) == pcbco->usCnrID)
                {
                    switch (SHORT2FROMMP(mp1))
                    {
                        case CN_ENTER:
                        {
                            PNOTIFYRECORDENTER pnre = (PNOTIFYRECORDENTER)mp2;
                            PCHECKBOXRECORDCORE precc = (PCHECKBOXRECORDCORE)pnre->pRecord;

                            if (precc)
                                CnrCheckboxClicked(pcbco,
                                                   precc,
                                                   TRUE);
                        }
                        break;

                        default:
                            mrc = pfnwpOrig(hwndOwner, msg, mp1, mp2);
                    }
                }
                else
                    mrc = pfnwpOrig(hwndOwner, msg, mp1, mp2);
            }
            break;

            /*
             * WM_DRAWITEM:
             *      cnr owner draw; this is where we draw
             *      the checkboxes
             */

            case WM_DRAWITEM:
            {
                if (SHORT1FROMMP(mp1) == pcbco->usCnrID)
                    mrc = ctlDrawCheckBoxRecord(mp2);
                else
                    mrc = pfnwpOrig(hwndOwner, msg, mp1, mp2);
            }
            break;

            default:
                mrc = pfnwpOrig(hwndOwner, msg, mp1, mp2);
        }
    }
    else
        mrc = WinDefWindowProc(hwndOwner, msg, mp1, mp2);

    return mrc;
}

/*
 *@@ ctlQueryCheckboxSize:
 *
 *@@added V0.9.19 (2002-04-24) [umoeller]
 */

ULONG ctlQueryCheckboxSize(VOID)
{
    if (G_hbmCheckboxes == NULLHANDLE)
    {
        // first call:
        BITMAPINFOHEADER bmih;
        // load checkboxes bitmap
        G_hbmCheckboxes = WinGetSysBitmap(HWND_DESKTOP,
                                          SBMP_CHECKBOXES);

        // and compute size of one checkbox
        // (4 columns, 3 rows)
        bmih.cbFix = sizeof(bmih);      // V0.9.19 (2002-04-24) [umoeller]
        GpiQueryBitmapParameters(G_hbmCheckboxes,
                                 &bmih);
        G_cxCheckbox = bmih.cx / 4;
    }

    return G_cxCheckbox;
}

/*
 *@@ ctlInitCheckboxContainer:
 *
 *@@added V0.9.18 (2002-03-03) [umoeller]
 */

VOID ctlInitCheckboxContainer(HWND hwndCnr)
{
    ctlQueryCheckboxSize();

    BEGIN_CNRINFO()
    {
        cnrhSetView(CV_TREE | CV_ICON | CA_TREELINE | CA_OWNERDRAW | CV_MINI);
        cnrhSetTreeIndent(20);
        cnrhSetBmpOrIconSize(G_cxCheckbox, G_cxCheckbox);
    } END_CNRINFO(hwndCnr);
}

/*
 *@@ ctlSubclassCheckboxContainer:
 *
 *@@added V0.9.18 (2002-03-03) [umoeller]
 */

PCHECKBOXCNROWNER ctlSubclassCheckboxContainer(HWND hwndCnr)
{
    PFNWP pfnwpCnrOrig;
    if (pfnwpCnrOrig = WinSubclassWindow(hwndCnr, fnwpSubclCheckboxCnr))
    {
        // cnr successfully subclassed:
        // create storage for both subclassed cnr and owner
        PCHECKBOXCNROWNER pcbco;
        if (pcbco = (PCHECKBOXCNROWNER)malloc(sizeof(CHECKBOXCNROWNER)))
        {
            memset(pcbco, 0, sizeof(CHECKBOXCNROWNER));
            pcbco->hwndCnr = hwndCnr;
            pcbco->usCnrID = WinQueryWindowUShort(hwndCnr, QWS_ID);
            pcbco->hwndOwner = WinQueryWindow(hwndCnr, QW_OWNER);
            pcbco->pfnwpCnrOrig = pfnwpCnrOrig;

            pcbco->habCnr = WinQueryAnchorBlock(hwndCnr);

            return pcbco;
        }
    }

    return NULL;
}

/*
 *@@ ctlMakeCheckboxContainer:
 *      this turns a regular container into a "checkbox"
 *      container. This means that the container record
 *      icons are painted as checkboxes, whose status
 *      is determined using an extended record core
 *      structure (CHECKBOXRECORDCORE).
 *
 *      This function assumes that all records in the
 *      container use this special extended record core
 *      ONLY. You cannot use regular RECORDCORE's when
 *      using this function. Of course, you can extend
 *      the CHECKBOXRECORDCORE structure to the bottom
 *      if you need more fields.
 *
 *      This subclasses BOTH the container and the container
 *      owner to intercept a number of messages and to
 *      implement owner draw. This is totally transparent
 *      to the caller; QWL_USER is not used for this, so
 *      you can still store your own stuff in there.
 *
 *      Returns FALSE upon errors.
 *
 *      To set up a checkbox container, call this function
 *      (which switches the container to Tree view automatically).
 *      Then, allocate and insert CHECKBOXRECORDCORE's as usual.
 *
 *      To set and query a record's check state easily, use
 *      ctlSetRecordChecked and ctlQueryRecordChecked.
 *
 *      The checkboxes work almost identically to regular
 *      checkboxes. The user can interact with the checkbox
 *      by clicking on the checkbox itself or by double-clicking
 *      on the record text or by pressing the Enter or Space keys.
 *
 *      Set CHECKBOXRECORDCORE.ulStyle to one of the regular
 *      checkbox button style bits; if one of the "auto" flags
 *      is set, the checkbox is toggled automatically.
 *
 *      Even if "auto" is off, when the user changes the check
 *      box selection, hwndCnrOwner gets sent a WM_CONTROL message
 *      with the following parameters:
 *
 *      -- mp1: USHORT id: usCnrID, as passed to this func
 *      -- mp1: USHORT usNotifyCode: CN_RECORDCHECKED (999)
 *      -- mp2: PCHECKRECORDCORE precc: the record which was (un)checked.
 *
 *      Note that the subclassed container owner proc filters out
 *      CN_ENTER, so you'll never receive that (but CN_RECORDCHECKED
 *      instead).
 *
 *      If the "auto" style bits have not been set, you must call
 *      ctlSetRecordChecked yourself then.
 *
 *@@added V0.9.0 (99-11-28) [umoeller]
 */

BOOL ctlMakeCheckboxContainer(HWND hwndCnrOwner,    // in: owner (and parent) of container
                              USHORT usCnrID)       // in: ID of container in hwndCnrOwner
{
    BOOL    brc = FALSE;

    HWND    hwndCnr = WinWindowFromID(hwndCnrOwner, usCnrID);

    // create mutex-protected list of container owners
    if (G_hmtxCnrOwnersList == 0)
        if (DosCreateMutexSem(NULL,
                              &G_hmtxCnrOwnersList, 0, FALSE) != NO_ERROR)
            return FALSE;

    if (G_pllCnrOwners == NULL)
        G_pllCnrOwners = lstCreate(TRUE);

    if (hwndCnr)
    {
        ctlInitCheckboxContainer(hwndCnr);

        if (    (hwndCnrOwner)
             && (G_hmtxCnrOwnersList)
           )
        {
            // subclass container owner
            PFNWP pfnwpOwnerOrig = WinSubclassWindow(hwndCnrOwner, fnwpSubclCheckboxCnrOwner);
            /* _Pmpf(("Subclassed hwnd 0x%lX: orig 0x%lX",
                    hwndCnrOwner, pfnwpOwnerOrig)); */
            if (pfnwpOwnerOrig)
            {
                // owner successfully subclassed:
                // subclass container too
                PCHECKBOXCNROWNER pcbco;
                if (pcbco = ctlSubclassCheckboxContainer(hwndCnr))
                {
                    if (!DosRequestMutexSem(G_hmtxCnrOwnersList, 5000))
                    {
                        lstAppendItem(G_pllCnrOwners, pcbco);
                        DosReleaseMutexSem(G_hmtxCnrOwnersList);
                        brc = TRUE;

                        pcbco->pfnwpOwnerOrig = pfnwpOwnerOrig;
                    }
                }
            }
        }
    }

    return brc;
}

/*
 *@@ FINDCHECKRECORD:
 *
 *@@added V1.0.0 (2002-09-09) [umoeller]
 */

typedef struct _FINDCHECKRECORD
{
    ULONG               ulItemID;
    PCHECKBOXRECORDCORE precFound;
} FINDCHECKRECORD, *PFINDCHECKRECORD;

/*
 *@@ fncbFindCheckRecord:
 *      helper callback for finding a checkbox
 *      record according to an item ID. Used
 *      with cnrhForAllRecords.
 *
 *      Returns 1 if found to make
 *      cnrhForAllRecords stop searching.
 *
 *@@added V0.9.0 (99-11-28) [umoeller]
 *@@changed V1.0.0 (2002-09-09) [umoeller]: adjusted for cnrhForAllRecords updates
 */

STATIC ULONG XWPENTRY fncbFindCheckRecord(HWND hwndCnr,             // in: container
                                          PRECORDCORE preccThis,    // in: current record (from cnrhForAllRecords)
                                          ULONG ulUser)
{
    if (((PCHECKBOXRECORDCORE)preccThis)->ulItemID == ((PFINDCHECKRECORD)ulUser)->ulItemID)
    {
        // found: store found record
        ((PFINDCHECKRECORD)ulUser)->precFound = (PCHECKBOXRECORDCORE)preccThis;
        // and stop
        return 1;
    }

    return 0;
}

/*
 *@@ ctlFindCheckRecord:
 *      this searches the given checkbox container
 *      for the record which matches the given item
 *      ID. Returns NULL if not found.
 *
 *@@added V0.9.1 (99-12-03) [umoeller]
 *@@changed V1.0.0 (2002-09-09) [umoeller]: adjusted for cnrhForAllRecords updates
 */

PCHECKBOXRECORDCORE ctlFindCheckRecord(HWND hwndCnr,
                                       ULONG ulItemID)
{
    FINDCHECKRECORD fcr;

    fcr.ulItemID = ulItemID;
    fcr.precFound = NULL;

    cnrhForAllRecords(hwndCnr,
                      NULL,         // start with root
                      fncbFindCheckRecord,
                      (ULONG)&fcr);

    return fcr.precFound;
}

/*
 *@@ ctlSetRecordChecked:
 *      this searches the given checkbox container
 *      for the record which matches the given item
 *      ID and updates that record check state.
 *
 *      The check state may be:
 *      -- 0: not checked (as with regular checkboxes)
 *      -- 1: checked (as with regular checkboxes)
 *      -- 2: indeterminate (as with regular checkboxes)
 *
 *      Returns FALSE if the record could not be found.
 *
 *@@added V0.9.0 (99-11-28) [umoeller]
 */

BOOL ctlSetRecordChecked(HWND hwndCnr,          // in: container prepared with
                                                // ctlMakeCheckboxContainer
                         ULONG ulItemID,        // in: record item ID
                         USHORT usCheckState)   // in: 0, 1, 2, 3
{
    PCHECKBOXRECORDCORE precc;
    if (precc = ctlFindCheckRecord(hwndCnr, ulItemID))
    {
        precc->usCheckState = usCheckState;
        WinSendMsg(hwndCnr,
                   CM_INVALIDATERECORD,
                   (MPARAM)&precc,
                   MPFROM2SHORT(1,
                                CMA_NOREPOSITION));
        return TRUE;
    }

    return FALSE;
}

/*
 *@@ ctlQueryRecordChecked:
 *      this searches the given checkbox container
 *      for the record which matches the given item
 *      ID and returns that record's check state.
 *
 *      Returns:
 *      -- 0: not checked (as with regular checkboxes)
 *      -- 1: checked (as with regular checkboxes)
 *      -- 2: indeterminate (as with regular checkboxes)
 *      -- -1: record not found.
 *
 *@@added V0.9.0 (99-11-28) [umoeller]
 */

ULONG ctlQueryRecordChecked(HWND hwndCnr,          // in: container prepared with
                                                   // ctlMakeCheckboxContainer
                            ULONG ulItemID,        // in: record item ID
                            USHORT usCheckState)   // in: 0, 1, 2, 3
{
    PCHECKBOXRECORDCORE precc;
    if (precc = ctlFindCheckRecord(hwndCnr, ulItemID))
        return precc->usCheckState;

    return -1;
}

/*
 *@@ ctlEnableRecord:
 *      this searches the given checkbox container
 *      for the record which matches the given item
 *      ID and updates that record enablement. If
 *      the record is disabled, it's painted halftoned
 *      by fnwpSubclCheckboxCnr.
 *
 *@@added V0.9.1 (99-12-03) [umoeller]
 */

BOOL ctlEnableRecord(HWND hwndCnr,
                     ULONG ulItemID,
                     BOOL fEnable)
{
    PCHECKBOXRECORDCORE precc;

    if (precc = ctlFindCheckRecord(hwndCnr, ulItemID))
    {
        ULONG ulAttr = precc->recc.flRecordAttr;
        if (fEnable)
            precc->recc.flRecordAttr &= ~CRA_DISABLED;
        else
            precc->recc.flRecordAttr |= CRA_DISABLED;

        if (precc->recc.flRecordAttr != ulAttr)
            // attrs changed: repaint
            WinSendMsg(hwndCnr,
                       CM_INVALIDATERECORD,
                       (MPARAM)&precc,
                       MPFROM2SHORT(1,
                                    CMA_NOREPOSITION));

        return TRUE;
    }

    return FALSE;
}


